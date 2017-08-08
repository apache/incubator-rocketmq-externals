/********************************************************************
author:  qiwei.wqw@alibaba-inc.com
*********************************************************************/
#ifndef __MQADMIN_H__
#define __MQADMIN_H__
#include <boost/asio.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include "MQMessageExt.h"
#include "MQMessageQueue.h"
#include "QueryResult.h"
#include "RocketMQClient.h"
#include "SessionCredentials.h"

namespace metaq {
class MQClientFactory;
//<!***************************************************************************

enum elogLevel {
  eLOG_LEVEL_DISABLE = 0,
  eLOG_LEVEL_FATAL = 1,
  eLOG_LEVEL_ERROR = 2,
  eLOG_LEVEL_WARN = 3,
  eLOG_LEVEL_INFO = 4,
  eLOG_LEVEL_DEBUG = 5,
  eLOG_LEVEL_TRACE = 6,
  eLOG_LEVEL_LEVEL_NUM = 7
};

class ROCKETMQCLIENT_API MQClient {
 public:
  MQClient();
  virtual ~MQClient();

 public:
  // clientid=processId-ipAddr@instanceName;
  string getMQClientId() const;
  const string& getNamesrvAddr() const;
  void setNamesrvAddr(const string& namesrvAddr);
  const string& getNamesrvDomain() const;
  void setNamesrvDomain(const string& namesrvDomain);
  const string& getInstanceName() const;
  void setInstanceName(const string& instanceName);
  //<!groupName;
  const string& getGroupName() const;
  void setGroupName(const string& groupname);
  
  /**
  * no realization
  */
  void createTopic(const string& key, const string& newTopic, int queueNum);
  /**
  * search earliest msg store time for specified queue
  *
  * @param mq
  *            message queue
  * @return earliest store time, ms
  */
  int64 earliestMsgStoreTime(const MQMessageQueue& mq);
  /**
  * search maxOffset of queue
  *
  * @param mq
  *            message queue
  * @return minOffset of queue
  */
  int64 minOffset(const MQMessageQueue& mq);
  /**
  * search maxOffset of queue
  * Note: maxOffset-1 is max offset that could get msg
  * @param mq
  *            message queue
  * @return maxOffset of queue
  */
  int64 maxOffset(const MQMessageQueue& mq);
  /**
  * get queue offset by timestamp
  *
  * @param mq
  *            mq queue
  * @param timestamp
  *            timestamp with ms unit
  * @return queue offset according to timestamp
  */
  int64 searchOffset(const MQMessageQueue& mq, uint64_t timestamp);
  /**
  * get whole msg info from broker by msgId
  *
  * @param msgId
  * @return MQMessageExt
  */
  MQMessageExt* viewMessage(const string& msgId);
  /**
  * query message by topic and key
  *
  * @param topic
  *            topic name
  * @param key
  *            topic key
  * @param maxNum
  *            query num
  * @param begin
  *            begin timestamp
  * @param end
  *            end timestamp
  * @return
  *            according to QueryResult
  */
  QueryResult queryMessage(const string& topic, const string& key, int maxNum,
                           int64 begin, int64 end);

  vector<MQMessageQueue> getTopicMessageQueueInfo(const string& topic);

  // log configuration interface, default LOG_LEVEL is LOG_LEVEL_INFO, default
  // log file num is 3, each log size is 100M
  void setMetaqLogLevel(elogLevel inputLevel);
  void setMetaqLogFileSizeAndNum(int fileNum,
                                 long perFileSize);  // perFileSize is MB unit

  /** set TcpTransport pull thread num, which dermine the num of threads to
 distribute network data,
     1. its default value is CPU num, it must be setted before producer/consumer
 start, minimum value is CPU num;
     2. this pullThread num must be tested on your environment to find the best
 value for RT of sendMsg or delay time of consume msg before you change it;
     3. producer and consumer need different pullThread num, if set this num,
 producer and consumer must set different instanceName.
     4. configuration suggestion:
         1>. minimum RT of sendMsg:
                 pullThreadNum = brokerNum*2
 **/
  void setTcpTransportPullThreadNum(int num);
  const int getTcpTransportPullThreadNum() const;

  /** timeout of tcp connect, it is same meaning for both producer and consumer;
      1. default value is 3000ms
      2. input parameter could only be milliSecond, suggestion value is
  1000-3000ms;
  **/
  void setTcpTransportConnectTimeout(uint64_t timeout);  // ms
  const uint64_t getTcpTransportConnectTimeout() const;

  /** timeout of tryLock tcpTransport before sendMsg/pullMsg, if timeout,
  returns NULL
      1. paremeter unit is ms, default value is 3000ms, the minimun value is
  1000ms
          suggestion value is 3000ms;
      2. if configured with value smaller than 1000ms, the tryLockTimeout value
  will be setted to 1000ms
  **/
  void setTcpTransportTryLockTimeout(uint64_t timeout);  // ms
  const uint64_t getTcpTransportTryLockTimeout() const;

  void setUnitName(string unitName);
  const string& getUnitName();

  void setSessionCredentials(const string& input_accessKey,
                             const string& input_secretKey,
                             const string& input_onsChannel);
  const SessionCredentials& getSessionCredentials() const;

 protected:
  virtual void start();
  virtual void shutdown();
  MQClientFactory* getFactory() const;
  virtual bool isServiceStateOk();

 protected:
  string m_namesrvAddr;
  string m_namesrvDomain;
  string m_instanceName;
  //<!  the name is globle only
  string m_GroupName;
  //<!factory;
  MQClientFactory* m_clientFactory;
  int m_serviceState;
  int m_pullThreadNum;
  uint64_t m_tcpConnectTimeout;           // ms
  uint64_t m_tcpTransportTryLockTimeout;  // s

  string m_unitName;
  SessionCredentials m_SessionCredentials;
};
//<!***************************************************************************
}  //<!end namespace;
#endif
