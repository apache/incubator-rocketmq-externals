#include "ConsumerRunningInfo.h"
#include "UtilAll.h"

namespace metaq {
const string ConsumerRunningInfo::PROP_NAMESERVER_ADDR = "PROP_NAMESERVER_ADDR";
const string ConsumerRunningInfo::PROP_THREADPOOL_CORE_SIZE =
    "PROP_THREADPOOL_CORE_SIZE";
const string ConsumerRunningInfo::PROP_CONSUME_ORDERLY = "PROP_CONSUMEORDERLY";
const string ConsumerRunningInfo::PROP_CONSUME_TYPE = "PROP_CONSUME_TYPE";
const string ConsumerRunningInfo::PROP_CLIENT_VERSION = "PROP_CLIENT_VERSION";
const string ConsumerRunningInfo::PROP_CONSUMER_START_TIMESTAMP =
    "PROP_CONSUMER_START_TIMESTAMP";

const map<string, string> ConsumerRunningInfo::getProperties() const {
  return properties;
}

void ConsumerRunningInfo::setProperties(
    const map<string, string>& input_properties) {
  properties = input_properties;
}

void ConsumerRunningInfo::setProperty(const string& key, const string& value) {
  properties[key] = value;
}

const map<MessageQueue, ProcessQueueInfo> ConsumerRunningInfo::getMqTable()
    const {
  return mqTable;
}

void ConsumerRunningInfo::setMqTable(MessageQueue queue,
                                     ProcessQueueInfo queueInfo) {
  mqTable[queue] = queueInfo;
}

/*const map<string, ConsumeStatus> ConsumerRunningInfo::getStatusTable() const
{
return statusTable;
}


void ConsumerRunningInfo::setStatusTable(const map<string, ConsumeStatus>&
input_statusTable)
{
statusTable = input_statusTable;
}	*/

const vector<SubscriptionData> ConsumerRunningInfo::getSubscriptionSet() const {
  return subscriptionSet;
}

void ConsumerRunningInfo::setSubscriptionSet(
    const vector<SubscriptionData>& input_subscriptionSet) {
  subscriptionSet = input_subscriptionSet;
}

const string ConsumerRunningInfo::getJstack() const { return jstack; }

void ConsumerRunningInfo::setJstack(const string& input_jstack) {
  jstack = input_jstack;
}

string ConsumerRunningInfo::encode() {
  MetaqJson::Value outData;

  outData[PROP_NAMESERVER_ADDR] = properties[PROP_NAMESERVER_ADDR];
  outData[PROP_CONSUME_TYPE] = properties[PROP_CONSUME_TYPE];
  outData[PROP_CLIENT_VERSION] = properties[PROP_CLIENT_VERSION];
  outData[PROP_CONSUMER_START_TIMESTAMP] =
      properties[PROP_CONSUMER_START_TIMESTAMP];
  outData[PROP_CONSUME_ORDERLY] = properties[PROP_CONSUME_ORDERLY];
  outData[PROP_THREADPOOL_CORE_SIZE] = properties[PROP_THREADPOOL_CORE_SIZE];

  MetaqJson::Value root;
  root["jstack"] = jstack;
  root["properties"] = outData;

  {
    vector<SubscriptionData>::const_iterator it = subscriptionSet.begin();
    for (; it != subscriptionSet.end(); it++) {
      root["subscriptionSet"].append(it->toJson());
    }
  }

  MetaqJson::FastWriter fastwrite;
  string finals = fastwrite.write(root);

  MetaqJson::Value mq;
  string key = "\"mqTable\":";
  key.append("{");
  for (map<MessageQueue, ProcessQueueInfo>::iterator it = mqTable.begin();
       it != mqTable.end(); ++it) {
    key.append((it->first).toJson().toStyledString());
    key.erase(key.end() - 1);
    key.append(":");
    key.append((it->second).toJson().toStyledString());
    key.append(",");
  }
  key.erase(key.end() - 1);
  key.append("}");

  // insert mqTable to final string
  key.append(",");
  finals.insert(1, key);

  return finals;
}
}
