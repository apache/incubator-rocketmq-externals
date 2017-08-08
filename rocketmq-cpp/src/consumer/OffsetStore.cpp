/********************************************************************
author:  qiwei.wqw@alibaba-inc.com
*********************************************************************/
#include "OffsetStore.h"
#include "Logging.h"
#include "MQClientFactory.h"
#include "MessageQueue.h"

#include <fstream>
#include <sstream>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/map.hpp>

namespace metaq {

//<!***************************************************************************
OffsetStore::OffsetStore(const string& groupName, MQClientFactory* pfactory)
    : m_groupName(groupName), m_pClientFactory(pfactory) {}

OffsetStore::~OffsetStore() {
  m_pClientFactory = NULL;
  m_offsetTable.clear();
}

//<!***************************************************************************
LocalFileOffsetStore::LocalFileOffsetStore(const string& groupName,
                                           MQClientFactory* pfactory)
    : OffsetStore(groupName, pfactory) {
  MQConsumer* pConsumer = pfactory->selectConsumer(groupName);
  if (pConsumer) {
    LOG_INFO("new LocalFileOffsetStore");
    string directoryName =
        UtilAll::getLocalAddress() + "@" + pConsumer->getInstanceName();
    m_storePath = ".rocketmq_offsets/" + directoryName + "/" + groupName + "/";
    string homeDir(UtilAll::getHomeDirectory());
    m_storeFile = homeDir + "/" + m_storePath + "offsets.Json";

    string storePath(homeDir);
    storePath.append(m_storePath);
    if (access(storePath.c_str(), F_OK) != 0) {
      if (mkdir(storePath.c_str(), S_IRWXU | S_IRWXG | S_IRWXO) != 0) {
        LOG_ERROR("create data dir:%s error", storePath.c_str());
      }
    }
  }
}

LocalFileOffsetStore::~LocalFileOffsetStore() {}

void LocalFileOffsetStore::load() {
  boost::lock_guard<boost::mutex> lock(m_lock);

  std::ifstream ifs(m_storeFile.c_str(), std::ios::in);
  if (ifs.good()) {
    if (ifs.is_open()) {
      if (ifs.peek() != std::ifstream::traits_type::eof()) {
        map<string, int64> m_offsetTable_tmp;
        boost::archive::text_iarchive ia(ifs);
        ia >> m_offsetTable_tmp;
        ifs.close();

        for (map<string, int64>::iterator it = m_offsetTable_tmp.begin();
             it != m_offsetTable_tmp.end(); ++it) {
          // LOG_INFO("it->first:%s, it->second:%lld", it->first.c_str(),
          // it->second);
          MetaqJson::Reader reader;
          MetaqJson::Value object;
          reader.parse(it->first.c_str(), object);
          MQMessageQueue mq(object["topic"].asString(),
                            object["brokerName"].asString(),
                            object["queueId"].asInt());
          m_offsetTable[mq] = it->second;
        }
        m_offsetTable_tmp.clear();
        /*for(map<MQMessageQueue, int64>::iterator it2 = m_offsetTable.begin();
        it2!=m_offsetTable.end();++it2 ){
            LOG_INFO("it->first:%s, it->second:%lld",
        it2->first.toString().c_str(), it2->second);
        }*/
      } else {
        LOG_ERROR(
            "open offset store file failed, please check whether file: %s is "
            "cleared by operator, if so, delete this offsets.Json file and "
            "then restart consumer",
            m_storeFile.c_str());
        THROW_MQEXCEPTION(MQClientException,
                          "open offset store file failed, please check whether "
                          "offsets.Json is cleared by operator, if so, delete "
                          "this offsets.Json file and then restart consumer",
                          -1);
      }
    } else {
      LOG_ERROR(
          "open offset store file failed, please check whether file:%s is "
          "deleted by operator and then restart consumer",
          m_storeFile.c_str());
      THROW_MQEXCEPTION(MQClientException,
                        "open offset store file failed, please check "
                        "directory:%s is deleted by operator or offset.Json "
                        "file is cleared by operator, and then restart "
                        "consumer",
                        -1);
    }
  } else {
    LOG_WARN(
        "offsets.Json file not exist, maybe this is the first time "
        "consumation");
  }
}

void LocalFileOffsetStore::updateOffset(const MQMessageQueue& mq,
                                        int64 offset) {
  boost::lock_guard<boost::mutex> lock(m_lock);
  m_offsetTable[mq] = offset;
}

int64 LocalFileOffsetStore::readOffset(
    const MQMessageQueue& mq, ReadOffsetType type,
    const SessionCredentials& session_credentials) {
  boost::lock_guard<boost::mutex> lock(m_lock);

  switch (type) {
    case MEMORY_FIRST_THEN_STORE:
    case READ_FROM_MEMORY: {
      MQ2OFFSET::iterator it = m_offsetTable.find(mq);
      if (it != m_offsetTable.end()) {
        return it->second;
      } else if (READ_FROM_MEMORY == type) {
        return -1;
      }
    }
    case READ_FROM_STORE: {
      try {
        load();
      } catch (MQException& e) {
        LOG_ERROR("catch exception when load local file");
        return -1;
      }
      MQ2OFFSET::iterator it = m_offsetTable.find(mq);
      if (it != m_offsetTable.end()) {
        return it->second;
      }
    }
    default:
      break;
  }
  LOG_ERROR(
      "can not readOffset from offsetStore.json, maybe first time consumation");
  return -1;
}

void LocalFileOffsetStore::persist(
    const MQMessageQueue& mq, const SessionCredentials& session_credentials) {}

void LocalFileOffsetStore::persistAll(const std::vector<MQMessageQueue>& mqs) {
  boost::lock_guard<boost::mutex> lock(m_lock);

  map<string, int64> m_offsetTable_tmp;
  vector<MQMessageQueue>::const_iterator it = mqs.begin();
  for (; it != mqs.end(); ++it) {
    MessageQueue mq_tmp((*it).getTopic(), (*it).getBrokerName(),
                        (*it).getQueueId());
    string mqKey = mq_tmp.toJson().toStyledString();
    m_offsetTable_tmp[mqKey] = m_offsetTable[*it];
  }

  std::ofstream s;
  s.open(m_storeFile.c_str(), std::ios::out);
  if (s.is_open()) {
    boost::archive::text_oarchive oa(s);
    // Boost is nervous that archiving non-const class instances which might
    // cause a problem with object tracking if different tracked objects use the
    // same address.
    oa << const_cast<const map<string, int64>&>(m_offsetTable_tmp);
    s.close();
    m_offsetTable_tmp.clear();
  } else {
    LOG_ERROR("open offset store file failed");
    m_offsetTable_tmp.clear();
    THROW_MQEXCEPTION(MQClientException,
                      "persistAll:open offset store file failed", -1);
  }
}

void LocalFileOffsetStore::removeOffset(const MQMessageQueue& mq) {}

//<!***************************************************************************
RemoteBrokerOffsetStore::RemoteBrokerOffsetStore(const string& groupName,
                                                 MQClientFactory* pfactory)
    : OffsetStore(groupName, pfactory) {}

RemoteBrokerOffsetStore::~RemoteBrokerOffsetStore() {}

void RemoteBrokerOffsetStore::load() {}

void RemoteBrokerOffsetStore::updateOffset(const MQMessageQueue& mq,
                                           int64 offset) {
  boost::lock_guard<boost::mutex> lock(m_lock);
  m_offsetTable[mq] = offset;
}

int64 RemoteBrokerOffsetStore::readOffset(
    const MQMessageQueue& mq, ReadOffsetType type,
    const SessionCredentials& session_credentials) {
  switch (type) {
    case MEMORY_FIRST_THEN_STORE:
    case READ_FROM_MEMORY: {
      boost::lock_guard<boost::mutex> lock(m_lock);

      MQ2OFFSET::iterator it = m_offsetTable.find(mq);
      if (it != m_offsetTable.end()) {
        return it->second;
      } else if (READ_FROM_MEMORY == type) {
        return -1;
      }
    }
    case READ_FROM_STORE: {
      try {
        int64 brokerOffset =
            fetchConsumeOffsetFromBroker(mq, session_credentials);
        //<!update;
        updateOffset(mq, brokerOffset);
        return brokerOffset;
      } catch (MQBrokerException& e) {
        LOG_ERROR(e.what());
        return -1;
      } catch (MQException& e) {
        LOG_ERROR(e.what());
        return -2;
      }
    }
    default:
      break;
  }
  return -1;
}

void RemoteBrokerOffsetStore::persist(
    const MQMessageQueue& mq, const SessionCredentials& session_credentials) {
  MQ2OFFSET offsetTable;
  {
    boost::lock_guard<boost::mutex> lock(m_lock);
    offsetTable = m_offsetTable;
  }

  MQ2OFFSET::iterator it = offsetTable.find(mq);
  if (it != offsetTable.end()) {
    try {
      updateConsumeOffsetToBroker(mq, it->second, session_credentials);
    } catch (MQException& e) {
      LOG_ERROR("updateConsumeOffsetToBroker error");
    }
  }
}

void RemoteBrokerOffsetStore::persistAll(
    const std::vector<MQMessageQueue>& mq) {}

void RemoteBrokerOffsetStore::removeOffset(const MQMessageQueue& mq) {
  boost::lock_guard<boost::mutex> lock(m_lock);
  if (m_offsetTable.find(mq) != m_offsetTable.end()) m_offsetTable.erase(mq);
}

void RemoteBrokerOffsetStore::updateConsumeOffsetToBroker(
    const MQMessageQueue& mq, int64 offset,
    const SessionCredentials& session_credentials) {
  unique_ptr<FindBrokerResult> pFindBrokerResult(
      m_pClientFactory->findBrokerAddressInAdmin(mq.getBrokerName()));

  if (pFindBrokerResult == NULL) {
    m_pClientFactory->updateTopicRouteInfoFromNameServer(mq.getTopic(),
                                                         session_credentials);
    pFindBrokerResult.reset(
        m_pClientFactory->findBrokerAddressInAdmin(mq.getBrokerName()));
  }

  if (pFindBrokerResult != NULL) {
    UpdateConsumerOffsetRequestHeader* pRequestHeader =
        new UpdateConsumerOffsetRequestHeader();
    pRequestHeader->topic = mq.getTopic();
    pRequestHeader->consumerGroup = m_groupName;
    pRequestHeader->queueId = mq.getQueueId();
    pRequestHeader->commitOffset = offset;

    try {
      LOG_INFO(
          "oneway updateConsumeOffsetToBroker of mq:%s, its offset is:%lld",
          mq.toString().c_str(), offset);
      return m_pClientFactory->getMQClientAPIImpl()->updateConsumerOffsetOneway(
          pFindBrokerResult->brokerAddr, pRequestHeader, 1000 * 5,
          session_credentials);
    } catch (MQException& e) {
      LOG_ERROR(e.what());
    }
  }
  LOG_WARN("The broker not exist");
}

int64 RemoteBrokerOffsetStore::fetchConsumeOffsetFromBroker(
    const MQMessageQueue& mq, const SessionCredentials& session_credentials) {
  unique_ptr<FindBrokerResult> pFindBrokerResult(
      m_pClientFactory->findBrokerAddressInAdmin(mq.getBrokerName()));

  if (pFindBrokerResult == NULL) {
    m_pClientFactory->updateTopicRouteInfoFromNameServer(mq.getTopic(),
                                                         session_credentials);
    pFindBrokerResult.reset(
        m_pClientFactory->findBrokerAddressInAdmin(mq.getBrokerName()));
  }

  if (pFindBrokerResult != NULL) {
    QueryConsumerOffsetRequestHeader* pRequestHeader =
        new QueryConsumerOffsetRequestHeader();
    pRequestHeader->topic = mq.getTopic();
    pRequestHeader->consumerGroup = m_groupName;
    pRequestHeader->queueId = mq.getQueueId();

    return m_pClientFactory->getMQClientAPIImpl()->queryConsumerOffset(
        pFindBrokerResult->brokerAddr, pRequestHeader, 1000 * 5,
        session_credentials);
  } else {
    LOG_ERROR("The broker not exist when fetchConsumeOffsetFromBroker");
    THROW_MQEXCEPTION(MQClientException, "The broker not exist", -1);
  }
}
//<!***************************************************************************
}  //<!end namespace;
