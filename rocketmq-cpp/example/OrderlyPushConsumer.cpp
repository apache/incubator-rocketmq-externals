#include <stdlib.h>
#include <string.h>

#include <atomic>
#include <condition_variable>
#include <iomanip>
#include <iostream>
#include <map>
#include <mutex>
#include <thread>
#include <vector>

#include "common.h"

using namespace metaq;

std::condition_variable g_finished;
std::mutex g_mtx;
std::atomic<int> g_consumedCount(0);
std::atomic<bool> g_quit(false);
TpsReportService g_tps;


class MyMsgListener : public MessageListenerOrderly {
public:
  MyMsgListener() {}
  virtual ~MyMsgListener() {}

  virtual ConsumeStatus consumeMessage(const vector<MQMessageExt> &msgs) {
    if (g_consumedCount.load() >= g_msgCount) {
      std::unique_lock<std::mutex> lK(g_mtx);
      g_quit.store(true);
      g_finished.notify_one();
    }
    for (size_t i = 0; i < msgs.size(); i++) {
      ++g_consumedCount;
      g_tps.Increment();
    }
    return CONSUME_SUCCESS;
  }
};


int main(int argc, char *argv[]) {
  RocketmqSendAndConsumerArgs info;
  if (!ParseArgs(argc, argv, &info)) {
    exit(-1);
  }
  PrintRocketmqSendAndConsumerArgs(info);
  DefaultMQPushConsumer consumer("please_rename_unique_group_name");
  DefaultMQProducer producer("please_rename_unique_group_name");

  producer.setNamesrvAddr(info.namesrv);
  producer.setGroupName("msg-persist-group_producer_sandbox");
  producer.start();

  consumer.setNamesrvAddr(info.namesrv);
  consumer.setNamesrvDomain(info.namesrv_domain);
  consumer.setGroupName(info.groupname);
  consumer.setConsumeFromWhere(CONSUME_FROM_LAST_OFFSET);
  consumer.subscribe(info.topic, "*");
  consumer.setConsumeThreadCount(info.thread_count);
  consumer.setConsumeMessageBatchMaxSize(31);
  if (info.syncpush)
    consumer.setAsyncPull(false);

  MyMsgListener msglistener;
  consumer.registerMessageListener(&msglistener);
  g_tps.start();

  try {
    consumer.start();
  } catch (MQClientException &e) {
    std::cout << e << std::endl;
  }

  int msgcount = g_msgCount.load();
  for (int i = 0; i < msgcount; ++i) {
    MQMessage msg(info.topic,    // topic
                  "*",      // tag
                  info.body);    // body

  try {
      producer.send(msg);
    } catch (MQException &e) {
      std::cout << e << endl; // if catch excepiton , need re-send this msg by
                         // service
    }
  }

  while(!g_quit.load()) {
    std::unique_lock<std::mutex> lk(g_mtx);
    g_finished.wait(lk);
  }

  producer.shutdown();
  consumer.shutdown();
  return 0;
}
