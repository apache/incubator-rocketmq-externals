#include <stdlib.h>
#include <string.h>

#include <iomanip>
#include <iostream>
#include <map>
#include <vector>
#include <chrono>

#include "common.h"

using namespace metaq;

std::map<MQMessageQueue, uint64_t> g_offseTable;

void putMessageQueueOffset(MQMessageQueue mq, uint64_t offset)
{
    g_offseTable[mq] = offset;
}

uint64_t getMessageQueueOffset(MQMessageQueue mq)
{
    map<MQMessageQueue, uint64_t>::iterator it = g_offseTable.find(mq);
    if (it != g_offseTable.end())
    {
        return it->second;
    }
    return 0;
}


int main(int argc, char *argv[]) {
  RocketmqSendAndConsumerArgs info;
  if (!ParseArgs(argc, argv, &info)) {
    exit(-1);
  }
  PrintRocketmqSendAndConsumerArgs(info);
  DefaultMQPullConsumer consumer("please_rename_unique_group_name");
  consumer.setNamesrvAddr(info.namesrv);
  consumer.setNamesrvDomain(info.namesrv_domain);
  consumer.setGroupName(info.groupname);
  consumer.setInstanceName(info.groupname);
  consumer.registerMessageQueueListener(info.topic, NULL);
  consumer.start();
  std::vector<MQMessageQueue> mqs;

  try {
    consumer.fetchSubscribeMessageQueues(info.topic, mqs);
    auto iter = mqs.begin();
    for (; iter != mqs.end(); ++iter) {
      std::cout << "mq:" << (*iter).toString() << endl;
    }
  } catch (MQException &e) {
    std::cout << e << endl;
  }

  auto start = std::chrono::system_clock::now();
  auto iter = mqs.begin();
  for (; iter != mqs.end(); ++iter) {

    MQMessageQueue mq = (*iter);
    // if cluster model
    // putMessageQueueOffset(mq, g_consumer.fetchConsumeOffset(mq,true));
    // if broadcast model
    // putMessageQueueOffset(mq, your last consume offset);

    bool noNewMsg = false;
    do {
      try {

        PullResult result =
            consumer.pull(mq, "*", getMessageQueueOffset(mq), 32);
        g_msgCount += result.msgFoundList.size();
        std::cout << result.msgFoundList.size() << std::endl;
        // if pull request timeout or received NULL response, pullStatus will be
        // setted to BROKER_TIMEOUT,
        // And nextBeginOffset/minOffset/MaxOffset will be setted to 0
        if (result.pullStatus != BROKER_TIMEOUT) {
          putMessageQueueOffset(mq, result.nextBeginOffset);
          PrintPullResult(&result);
        } else {
          cout << "broker timeout occur" << endl;
        }
        switch (result.pullStatus) {
        case FOUND:
        case NO_MATCHED_MSG:
        case OFFSET_ILLEGAL:
        case BROKER_TIMEOUT:
          break;
        case NO_NEW_MSG:
          noNewMsg = true;
          break;
        default:
          break;
        }
      } catch (MQClientException &e) {
        std::cout << e << std::endl;
      }
    } while (!noNewMsg);
  }

  auto end = std::chrono::system_clock::now();
  auto duration =
      std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

  std::cout << "msg count: " << g_msgCount.load() << "\n";
  std::cout << "per msg time: "
      << duration.count() / (double)g_msgCount.load() << "ms \n"
      << "========================finished==============================\n";

  consumer.shutdown();
  return 0;
}
