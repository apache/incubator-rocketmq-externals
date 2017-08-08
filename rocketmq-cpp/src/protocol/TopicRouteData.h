/********************************************************************
author:  qiwei.wqw@alibaba-inc.com
*********************************************************************/
#ifndef __TOPICROUTEDATA_H__
#define __TOPICROUTEDATA_H__
#include <algorithm>
#include "Logging.h"
#include "UtilAll.h"
#include "dataBlock.h"
#include "json/json.h"

namespace metaq {
//<!***************************************************************************
struct QueueData {
  string brokerName;
  int readQueueNums;
  int writeQueueNums;
  int perm;

  bool operator<(const QueueData& other) const {
    return brokerName < other.brokerName;
  }

  bool operator==(const QueueData& other) const {
    if (brokerName == other.brokerName &&
        readQueueNums == other.readQueueNums &&
        writeQueueNums == other.writeQueueNums && perm == other.perm) {
      return true;
    }
    return false;
  }
};

//<!***************************************************************************
struct BrokerData {
  string brokerName;
  map<int, string> brokerAddrs;  //<!0:master,1,2.. slave

  bool operator<(const BrokerData& other) const {
    return brokerName < other.brokerName;
  }

  bool operator==(const BrokerData& other) const {
    if (brokerName == other.brokerName && brokerAddrs == other.brokerAddrs) {
      return true;
    }
    return false;
  }
};

//<!************************************************************************/
class TopicRouteData {
 public:
  virtual ~TopicRouteData() {
    m_brokerDatas.clear();
    m_queueDatas.clear();
  }
  static TopicRouteData* Decode(const MemoryBlock* mem) {
    //<!see doc/TopicRouteData.json;
    const char* const pData = static_cast<const char*>(mem->getData());

    MetaqJson::Reader reader;
    MetaqJson::Value root;
    const char* begin = pData;
    const char* end = pData + mem->getSize();
    if (!reader.parse(begin, end, root)) {
      return NULL;
    }

    TopicRouteData* trd = new TopicRouteData();
    trd->setOrderTopicConf(root["orderTopicConf"].asString());

    MetaqJson::Value qds = root["queueDatas"];
    for (size_t i = 0; i < qds.size(); i++) {
      QueueData d;
      MetaqJson::Value qd = qds[i];
      d.brokerName = qd["brokerName"].asString();
      d.readQueueNums = qd["readQueueNums"].asInt();
      d.writeQueueNums = qd["writeQueueNums"].asInt();
      d.perm = qd["perm"].asInt();

      trd->getQueueDatas().push_back(d);
    }

    sort(trd->getQueueDatas().begin(), trd->getQueueDatas().end());

    MetaqJson::Value bds = root["brokerDatas"];
    for (size_t i = 0; i < bds.size(); i++) {
      BrokerData d;
      MetaqJson::Value bd = bds[i];
      d.brokerName = bd["brokerName"].asString();

      LOG_DEBUG("brokerName:%s", d.brokerName.c_str());

      MetaqJson::Value bas = bd["brokerAddrs"];
      MetaqJson::Value::Members mbs = bas.getMemberNames();
      for (size_t i = 0; i < mbs.size(); i++) {
        string key = mbs.at(i);
        LOG_DEBUG("brokerid:%s,brokerAddr:%s", key.c_str(),
                  bas[key].asString().c_str());
        d.brokerAddrs[atoi(key.c_str())] = bas[key].asString();
      }

      trd->getBrokerDatas().push_back(d);
    }

    sort(trd->getBrokerDatas().begin(), trd->getBrokerDatas().end());

    return trd;
  }

  string selectBrokerAddr() {
    vector<BrokerData>::iterator it = m_brokerDatas.begin();
    for (; it != m_brokerDatas.end(); ++it) {
      map<int, string>::iterator it1 = (*it).brokerAddrs.find(MASTER_ID);
      if (it1 != (*it).brokerAddrs.end()) {
        return it1->second;
      }
    }
    return "";
  }

  vector<QueueData>& getQueueDatas() { return m_queueDatas; }

  vector<BrokerData>& getBrokerDatas() { return m_brokerDatas; }

  const string& getOrderTopicConf() const { return m_orderTopicConf; }

  void setOrderTopicConf(const string& orderTopicConf) {
    m_orderTopicConf = orderTopicConf;
  }

  bool operator==(const TopicRouteData& other) const {
    if (m_brokerDatas != other.m_brokerDatas) {
      return false;
    }

    if (m_orderTopicConf != other.m_orderTopicConf) {
      return false;
    }

    if (m_queueDatas != other.m_queueDatas) {
      return false;
    }
    return true;
  }

 public:
 private:
  string m_orderTopicConf;
  vector<QueueData> m_queueDatas;
  vector<BrokerData> m_brokerDatas;
};

}  //<!end namespace;

#endif
