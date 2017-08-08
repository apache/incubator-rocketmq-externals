/********************************************************************
author:  qiwei.wqw@alibaba-inc.com
*********************************************************************/
#include <sys/prctl.h>
#include "ConsumeMsgService.h"
#include "DefaultMQPushConsumer.h"
#include "Logging.h"
#include "Rebalance.h"
#include "UtilAll.h"

namespace metaq {

//<!***************************************************************************
ConsumeMessageOrderlyService::ConsumeMessageOrderlyService(
    MQConsumer* consumer, int threadCount, MQMessageListener* msgListener)
    : m_pConsumer(consumer),
      m_shutdownInprogress(false),
      m_pMessageListener(msgListener),
      m_MaxTimeConsumeContinuously(60 * 1000),
      m_ioServiceWork(m_ioService),
      m_async_service_thread(NULL) {
  string taskName = UtilAll::getProcessName();
  prctl(PR_SET_NAME, "oderlyConsumeTP", 0, 0, 0);
  for (int i = 0; i != threadCount; ++i) {
    m_threadpool.create_thread(
        boost::bind(&boost::asio::io_service::run, &m_ioService));
  }
  prctl(PR_SET_NAME, taskName.c_str(), 0, 0, 0);

}

void ConsumeMessageOrderlyService::boost_asio_work() {
  LOG_INFO("ConsumeMessageOrderlyService::boost asio async service runing");
  boost::asio::io_service::work work(m_async_ioService);  // avoid async io
                                                          // service stops after
                                                          // first timer timeout
                                                          // callback
  boost::system::error_code ec;
  boost::asio::deadline_timer t(m_async_ioService,
                                boost::posix_time::milliseconds(PullRequest::RebalanceLockInterval));
  t.async_wait(
      boost::bind(&ConsumeMessageOrderlyService::lockMQPeriodically, this, ec, &t));
  
  m_async_ioService.run();
}

ConsumeMessageOrderlyService::~ConsumeMessageOrderlyService(void) {
  m_pConsumer = NULL;
  m_pMessageListener = NULL;
}

void ConsumeMessageOrderlyService::start() {
    m_async_service_thread.reset(new boost::thread(
      boost::bind(&ConsumeMessageOrderlyService::boost_asio_work, this)));
}

void ConsumeMessageOrderlyService::shutdown() {
  stopThreadPool();
  unlockAllMQ();
}

void ConsumeMessageOrderlyService::lockMQPeriodically(
    boost::system::error_code& ec, boost::asio::deadline_timer* t) {
  m_pConsumer->getRebalance()->lockAll();

  boost::system::error_code e;
  t->expires_at(t->expires_at() + boost::posix_time::milliseconds(PullRequest::RebalanceLockInterval), e);
  t->async_wait(boost::bind(&ConsumeMessageOrderlyService::lockMQPeriodically,
                             this, ec, t));
}

void ConsumeMessageOrderlyService::unlockAllMQ() {
  m_pConsumer->getRebalance()->unlockAll(false);
}

bool ConsumeMessageOrderlyService::lockOneMQ(const MQMessageQueue& mq) {
  return m_pConsumer->getRebalance()->lock(mq);
}

void ConsumeMessageOrderlyService::stopThreadPool() {
  m_shutdownInprogress = true;
  m_ioService.stop();
  m_async_ioService.stop();
  m_async_service_thread->join();
  m_threadpool.join_all();
}

MessageListenerType
ConsumeMessageOrderlyService::getConsumeMsgSerivceListenerType() {
  return m_pMessageListener->getMessageListenerType();
}

void ConsumeMessageOrderlyService::submitConsumeRequest(
    PullRequest* request, vector<MQMessageExt>& msgs) {
  m_ioService.post(boost::bind(&ConsumeMessageOrderlyService::ConsumeRequest,
                               this, request));
}

void ConsumeMessageOrderlyService::static_submitConsumeRequestLater(
    void* context, PullRequest* request, bool tryLockMQ,
    boost::asio::deadline_timer* t) {
  LOG_INFO("submit consumeRequest later for mq:%s",
           request->m_messageQueue.toString().c_str());
  vector<MQMessageExt> msgs;
  ConsumeMessageOrderlyService* orderlyService =
      (ConsumeMessageOrderlyService*)context;
  orderlyService->submitConsumeRequest(request, msgs);
  if (tryLockMQ) {
    orderlyService->lockOneMQ(request->m_messageQueue);
  }
  if (t) deleteAndZero(t);
}

void ConsumeMessageOrderlyService::ConsumeRequest(PullRequest* request) {
  bool bGetMutex = false;
  boost::unique_lock<boost::timed_mutex> lock(
      request->getPullRequestCriticalSection(), boost::try_to_lock);
  if (!lock.owns_lock()) {
    if (!lock.timed_lock(boost::get_system_time() +
                         boost::posix_time::seconds(1))) {
      LOG_ERROR("ConsumeRequest of:%s get timed_mutex timeout",
                request->m_messageQueue.toString().c_str());
      return;
    } else {
      bGetMutex = true;
    }
  } else {
    bGetMutex = true;
  }
  if (!bGetMutex) {
    // LOG_INFO("pullrequest of mq:%s consume inprogress",
    // request->m_messageQueue.toString().c_str());
    return;
  }
  if (!request || request->isDroped()) {
    LOG_WARN("the pull result is NULL or Had been dropped");
    request->clearAllMsgs();  // add clear operation to avoid bad state when
                              // dropped pullRequest returns normal
    return;
  }

  if (m_pMessageListener) {
    if ((request->isLocked() && !request->isLockExpired()) ||
        m_pConsumer->getMessageModel() == BROADCASTING) {
      DefaultMQPushConsumer* pConsumer = (DefaultMQPushConsumer*)m_pConsumer;
      uint64_t beginTime = UtilAll::currentTimeMillis();
      bool continueConsume = true;
      while (continueConsume) {
        if ((UtilAll::currentTimeMillis() - beginTime) >
            m_MaxTimeConsumeContinuously) {
          LOG_INFO(
              "continuely consume message queue:%s more than 60s, consume it "
              "later",
              request->m_messageQueue.toString().c_str());
          tryLockLaterAndReconsume(request, false);
          break;
        }
        vector<MQMessageExt> msgs;
        request->takeMessages(msgs, pConsumer->getConsumeMessageBatchMaxSize());
        if (!msgs.empty()) {
          request->setLastConsumeTimestamp(UtilAll::currentTimeMillis());
          ConsumeStatus consumeStatus =
              m_pMessageListener->consumeMessage(msgs);
          if (consumeStatus == RECONSUME_LATER) {
            request->makeMessageToCosumeAgain(msgs);
            continueConsume = false;
            tryLockLaterAndReconsume(request, false);
          } else {
            m_pConsumer->updateConsumeOffset(request->m_messageQueue,
                                             request->commit());
          }
        } else {
          continueConsume = false;
        }
        msgs.clear();
        if (m_shutdownInprogress) {
          LOG_INFO("shutdown inprogress, break the consuming");
          return;
        }
      }
      LOG_DEBUG("consume once exit of mq:%s",
                request->m_messageQueue.toString().c_str());
    } else {
      LOG_ERROR("message queue:%s was not locked",
                request->m_messageQueue.toString().c_str());
      tryLockLaterAndReconsume(request, true);
    }
  }
}
void ConsumeMessageOrderlyService::tryLockLaterAndReconsume(
    PullRequest* request, bool tryLockMQ) {
  int retryTimer = tryLockMQ ? 500 : 100;
  boost::asio::deadline_timer* t = new boost::asio::deadline_timer(
      m_async_ioService, boost::posix_time::milliseconds(retryTimer));
  t->async_wait(boost::bind(
      &(ConsumeMessageOrderlyService::static_submitConsumeRequestLater), this,
      request, tryLockMQ, t));
}
}