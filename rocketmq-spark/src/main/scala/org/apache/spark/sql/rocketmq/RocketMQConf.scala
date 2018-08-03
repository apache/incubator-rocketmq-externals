package org.apache.spark.sql.rocketmq

/**
 * Options for RocketMQ consumer or producer client.
 *
 * See also [[RocketMQSourceProvider]]
 */
object RocketMQConf {

  //*******************************
  //        Shared Options
  //*******************************

  val NAME_SERVER_ADDR = "nameserver.addr"

  //*******************************
  //  Source (Consumer) Options
  //*******************************

  val CONSUMER_GROUP = "consumer.group"

  val CONSUMER_TOPIC = "consumer.topic"

  // What point should be consuming from (options: "earliest", "latest", default: "latest")
  val CONSUMER_OFFSET = "consumer.offset"

  // Subscription expression (default: "*")
  val CONSUMER_SUB_EXPRESSION = "consumer.tag"

  // To pick up the consume speed, the consumer can pull a batch of messages at a time (default: 32)
  val PULL_MAX_BATCH_SIZE = "pull.max.batch.size"

  // Pull timeout for the consumer (default: 3000)
  val PULL_TIMEOUT_MS = "pull.timeout.ms"

  //*******************************
  //   Sink (Producer) Options
  //*******************************

  val PRODUCER_GROUP = "producer.group"

  // Default topic of produced messages if `topic` is not among the attributes
  val PRODUCER_TOPIC = "producer.topic"

  //*******************************
  //         Global Options
  //*******************************

  // Max number of cached pull consumer (default: 64)
  val PULL_CONSUMER_CACHE_MAX_CAPACITY = "spark.sql.rocketmq.pull.consumer.cache.maxCapacity"

  // Producer cache timeout (default: "10m")
  val PRODUCER_CACHE_TIMEOUT = "spark.rocketmq.producer.cache.timeout"
}
