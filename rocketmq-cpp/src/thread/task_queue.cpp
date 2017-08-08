/********************************************************************
author:  qiwei.wqw@alibaba-inc.com
*********************************************************************/
#include "task_queue.h"
#include <sys/prctl.h>
#include "UtilAll.h"
#include "disruptorLFQ.h"

namespace metaq {
//<!***************************************************************************
Task* taskEventFactory::NewInstance(const int& size) const {
  return new Task[size];
}

taskBatchHandler::taskBatchHandler(int pullMsgThreadPoolNum)
    : m_ioServiceWork(m_ioService) {
  string taskName = UtilAll::getProcessName();
  prctl(PR_SET_NAME, "PullMsgTP", 0, 0, 0);
  for (int i = 0; i != pullMsgThreadPoolNum; ++i) {
    m_threadpool.create_thread(
        boost::bind(&boost::asio::io_service::run, &m_ioService));
  }
  prctl(PR_SET_NAME, taskName.c_str(), 0, 0, 0);
}

void taskBatchHandler::OnEvent(const int64_t& sequence,
                               const bool& end_of_batch, Task* event) {
   //cp Task event out, avoid publish event override current Task event                               
  Task currentTask(*event);
  m_ioService.post(boost::bind(&taskBatchHandler::runTaskEvent, this,
                               currentTask, sequence));
}

void taskBatchHandler::runTaskEvent(Task event, int64_t sequence) {
  // LOG_INFO("processor event sequence:%lld",  sequence);
  event.run();
}

void taskBatchHandler::stopIOService() {
  m_ioService.stop();
  m_threadpool.join_all();
}

taskEventTranslator::taskEventTranslator(Task* event) : m_taskEvent(event) {}

Task* taskEventTranslator::TranslateTo(const int64_t& sequence, Task* event) {
  // LOG_INFO("publish sequence:%lld, event:%x", sequence, event);
  *event = *m_taskEvent;
  return event;
};

//******************************************************************************************8
TaskQueue::TaskQueue(int threadCount) {
  m_flag.store(true, boost::memory_order_release);
  m_disruptorLFQ = new disruptorLFQ(threadCount);
}

TaskQueue::~TaskQueue() {
  delete m_disruptorLFQ;
  m_disruptorLFQ = NULL;
}

void TaskQueue::close() {
  m_flag.store(false, boost::memory_order_release);
  m_disruptorLFQ->m_task_handler->stopIOService();
  m_disruptorLFQ->m_processor->Halt();
}

bool TaskQueue::bTaskQueueStatusOK() {
  return m_flag.load(boost::memory_order_acquire) == true;
}

void TaskQueue::produce(const Task& task) {
  boost::mutex::scoped_lock lock(m_publishLock);
  taskEventTranslator pTranslator(const_cast<Task*>(&task));
  m_disruptorLFQ->m_publisher->PublishEvent(&pTranslator);
}

int TaskQueue::run() {
  while (true) {
    m_disruptorLFQ->m_processor->Run();
    if (m_flag.load(boost::memory_order_acquire) == false) {
      break;
    }
  }
  return 0;
}

//<!***************************************************************************
}  //<! end namespace;
