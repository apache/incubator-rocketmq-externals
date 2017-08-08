/********************************************************************
author:  qiwei.wqw@alibaba-inc.com
*********************************************************************/
#include "MessageQueue.h"
#include "UtilAll.h"

namespace metaq {
//<!************************************************************************
MessageQueue::MessageQueue() {
  m_queueId = -1;  // invalide mq
  m_topic.clear();
  m_brokerName.clear();
}

MessageQueue::MessageQueue(const string& topic, const string& brokerName,
                           int queueId)
    : m_topic(topic), m_brokerName(brokerName), m_queueId(queueId) {}

MessageQueue::MessageQueue(const MessageQueue& other)
    : m_topic(other.m_topic),
      m_brokerName(other.m_brokerName),
      m_queueId(other.m_queueId) {}

MessageQueue& MessageQueue::operator=(const MessageQueue& other) {
  if (this != &other) {
    m_brokerName = other.m_brokerName;
    m_topic = other.m_topic;
    m_queueId = other.m_queueId;
  }
  return *this;
}

string MessageQueue::getTopic() const { return m_topic; }

void MessageQueue::setTopic(const string& topic) { m_topic = topic; }

string MessageQueue::getBrokerName() const { return m_brokerName; }

void MessageQueue::setBrokerName(const string& brokerName) {
  m_brokerName = brokerName;
}

int MessageQueue::getQueueId() const { return m_queueId; }

void MessageQueue::setQueueId(int queueId) { m_queueId = queueId; }

bool MessageQueue::operator==(const MessageQueue& mq) const {
  if (this == &mq) {
    return true;
  }

  if (m_brokerName != mq.m_brokerName) {
    return false;
  }

  if (m_queueId != mq.m_queueId) {
    return false;
  }

  if (m_topic != mq.m_topic) {
    return false;
  }

  return true;
}

int MessageQueue::compareTo(const MessageQueue& mq) const {
  int result = m_topic.compare(mq.m_topic);
  if (result != 0) {
    return result;
  }

  result = m_brokerName.compare(mq.m_brokerName);
  if (result != 0) {
    return result;
  }

  return m_queueId - mq.m_queueId;
}

bool MessageQueue::operator<(const MessageQueue& mq) const {
  return compareTo(mq) < 0;
}

MetaqJson::Value MessageQueue::toJson() const {
  MetaqJson::Value outJson;
  outJson["topic"] = m_topic;
  outJson["brokerName"] = m_brokerName;
  outJson["queueId"] = m_queueId;
  return outJson;
}

//<!***************************************************************************
}  //<!end namespace;