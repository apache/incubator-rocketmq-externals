/********************************************************************
author:  qiwei.wqw@alibaba-inc.com
*********************************************************************/
#ifndef __REBALANCEIMPL_H__
#define __REBALANCEIMPL_H__

#include "AllocateMQStrategy.h"
#include "ConsumeType.h"
#include "MQConsumer.h"
#include "MQMessageQueue.h"
#include "PullRequest.h"
#include "SubscriptionData.h"

#include <boost/thread/mutex.hpp>

namespace metaq {
class MQClientFactory;
//<!************************************************************************
class Rebalance {
 public:
  Rebalance(MQConsumer*, MQClientFactory*);
  virtual ~Rebalance();

  virtual void messageQueueChanged(const string& topic,
                                   vector<MQMessageQueue>& mqAll,
                                   vector<MQMessageQueue>& mqDivided) = 0;

  virtual void removeUnnecessaryMessageQueue(const MQMessageQueue& mq) = 0;

  virtual int64 computePullFromWhere(const MQMessageQueue& mq) = 0;

  virtual bool updateRequestTableInRebalance(
      const string& topic, vector<MQMessageQueue>& mqsSelf) = 0;

 public:
  void doRebalance();
  void persistConsumerOffset();
  void persistConsumerOffsetByResetOffset();
  //<!m_subscriptionInner;
  SubscriptionData* getSubscriptionData(const string& topic);
  void setSubscriptionData(const string& topic, SubscriptionData* pdata);

  map<string, SubscriptionData*>& getSubscriptionInner();

  //<!m_topicSubscribeInfoTable;
  void setTopicSubscribeInfo(const string& topic, vector<MQMessageQueue>& mqs);
  bool getTopicSubscribeInfo(const string& topic, vector<MQMessageQueue>& mqs);

  void addPullRequest(MQMessageQueue mq, PullRequest* pPullRequest);
  PullRequest* getPullRequest(MQMessageQueue mq);
  map<MQMessageQueue, PullRequest*> getPullRequestTable();
  void lockAll();
  bool lock(MQMessageQueue mq);
  void unlockAll(bool oneway = false);
  void unlock(MQMessageQueue mq);

 protected:
  map<string, SubscriptionData*> m_subscriptionData;

  boost::mutex m_topicSubscribeInfoTableMutex;
  map<string, vector<MQMessageQueue>> m_topicSubscribeInfoTable;
  typedef map<MQMessageQueue, PullRequest*> MQ2PULLREQ;
  MQ2PULLREQ m_requestQueueTable;
  boost::mutex m_requestTableMutex;

  AllocateMQStrategy* m_pAllocateMQStrategy;
  MQConsumer* m_pConsumer;
  MQClientFactory* m_pClientFactory;
};

//<!************************************************************************
class RebalancePull : public Rebalance {
 public:
  RebalancePull(MQConsumer*, MQClientFactory*);
  virtual ~RebalancePull(){};

  virtual void messageQueueChanged(const string& topic,
                                   vector<MQMessageQueue>& mqAll,
                                   vector<MQMessageQueue>& mqDivided);

  virtual void removeUnnecessaryMessageQueue(const MQMessageQueue& mq);

  virtual int64 computePullFromWhere(const MQMessageQueue& mq);

  virtual bool updateRequestTableInRebalance(const string& topic,
                                             vector<MQMessageQueue>& mqsSelf);
};

//<!***************************************************************************
class RebalancePush : public Rebalance {
 public:
  RebalancePush(MQConsumer*, MQClientFactory*);
  virtual ~RebalancePush(){};

  virtual void messageQueueChanged(const string& topic,
                                   vector<MQMessageQueue>& mqAll,
                                   vector<MQMessageQueue>& mqDivided);

  virtual void removeUnnecessaryMessageQueue(const MQMessageQueue& mq);

  virtual int64 computePullFromWhere(const MQMessageQueue& mq);

  virtual bool updateRequestTableInRebalance(const string& topic,
                                             vector<MQMessageQueue>& mqsSelf);
};

//<!************************************************************************
}  //<!end namespace;

#endif
