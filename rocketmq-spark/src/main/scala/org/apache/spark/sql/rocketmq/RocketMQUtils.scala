package org.apache.spark.sql.rocketmq

import java.{util => ju}

import org.apache.rocketmq.client.consumer.DefaultMQPullConsumer
import org.apache.rocketmq.client.producer.DefaultMQProducer

/**
 * Some helper methods of RocketMQ
 */
object RocketMQUtils {

  def makePullConsumer(groupId: String, optionParams: ju.Map[String, String]): DefaultMQPullConsumer = {
    val consumer = new DefaultMQPullConsumer(groupId)
    if (optionParams.containsKey(RocketMQConf.PULL_TIMEOUT_MS))
      consumer.setConsumerTimeoutMillisWhenSuspend(optionParams.get(RocketMQConf.PULL_TIMEOUT_MS).toLong)
    if (optionParams.containsKey(RocketMQConf.NAME_SERVER_ADDR))
      consumer.setNamesrvAddr(optionParams.get(RocketMQConf.NAME_SERVER_ADDR))

    consumer.start()
    consumer.setOffsetStore(consumer.getDefaultMQPullConsumerImpl.getOffsetStore)
    consumer
  }

  def makeProducer(groupId: String, optionParams: ju.Map[String, String]): DefaultMQProducer = {
    val producer = new DefaultMQProducer(groupId)
    if (optionParams.containsKey(RocketMQConf.NAME_SERVER_ADDR))
      producer.setNamesrvAddr(optionParams.get(RocketMQConf.NAME_SERVER_ADDR))

    producer.start()
    producer
  }

}
