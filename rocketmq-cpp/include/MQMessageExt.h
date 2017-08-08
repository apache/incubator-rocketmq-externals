/********************************************************************
author:  qiwei.wqw@alibaba-inc.com
*********************************************************************/
#ifndef __MESSAGEEXT_H__
#define __MESSAGEEXT_H__

#ifdef WIN32
#include <Windows.h>
#include <Winsock2.h>
#else
#include <sys/socket.h>
#endif

#include "MQMessage.h"
#include "RocketMQClient.h"

namespace metaq {
//<!message extend class, which was generated on broker;
//<!***************************************************************************
class ROCKETMQCLIENT_API MQMessageExt : public MQMessage {
 public:
  MQMessageExt();
  MQMessageExt(int queueId, int64 bornTimestamp, sockaddr bornHost,
               int64 storeTimestamp, sockaddr storeHost, string msgId);

  virtual ~MQMessageExt();

  static int parseTopicFilterType(int sysFlag);

  int getQueueId() const;
  void setQueueId(int queueId);

  int64 getBornTimestamp() const;
  void setBornTimestamp(int64 bornTimestamp);

  sockaddr getBornHost() const;
  string getBornHostString() const;
  string getBornHostNameString() const;
  void setBornHost(const sockaddr& bornHost);

  int64 getStoreTimestamp() const;
  void setStoreTimestamp(int64 storeTimestamp);

  sockaddr getStoreHost() const;
  string getStoreHostString() const;
  void setStoreHost(const sockaddr& storeHost);

  const string& getMsgId() const;
  void setMsgId(const string& msgId);

  int getSysFlag() const;
  void setSysFlag(int sysFlag);

  int getBodyCRC() const;
  void setBodyCRC(int bodyCRC);

  int64 getQueueOffset() const;
  void setQueueOffset(int64 queueOffset);

  int64 getCommitLogOffset() const;
  void setCommitLogOffset(int64 physicOffset);

  int getStoreSize() const;
  void setStoreSize(int storeSize);

  int getReconsumeTimes() const;
  void setReconsumeTimes(int reconsumeTimes);

  int64 getPreparedTransactionOffset() const;
  void setPreparedTransactionOffset(int64 preparedTransactionOffset);

  string toString() const {
    stringstream ss;
    ss << "MessageExt [queueId=" << m_queueId << ", storeSize=" << m_storeSize
       << ", queueOffset=" << m_queueOffset << ", sysFlag=" << m_sysFlag
       << ", bornTimestamp=" << m_bornTimestamp
       << ", bornHost=" << getBornHostString()
       << ", storeTimestamp=" << m_storeTimestamp
       << ", storeHost=" << getStoreHostString() << ", msgId=" << m_msgId
       << ", commitLogOffset=" << m_commitLogOffset << ", bodyCRC=" << m_bodyCRC
       << ", reconsumeTimes=" << m_reconsumeTimes
       << ", preparedTransactionOffset=" << m_preparedTransactionOffset << ",  "
       << MQMessage::toString() << "]";
    return ss.str();
  }

 private:
  int64 m_queueOffset;
  int64 m_commitLogOffset;
  int64 m_bornTimestamp;
  int64 m_storeTimestamp;
  int64 m_preparedTransactionOffset;
  int m_queueId;
  int m_storeSize;
  int m_sysFlag;
  int m_bodyCRC;
  int m_reconsumeTimes;
  sockaddr m_bornHost;
  sockaddr m_storeHost;
  string m_msgId;
};
//<!***************************************************************************
}  //<!end namespace;
#endif
