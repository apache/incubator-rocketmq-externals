/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.apache.spark.sql.rocketmq

import java.{util => ju}

import org.apache.spark.internal.Logging
import org.apache.spark.sql.{AnalysisException, SparkSession}
import org.apache.spark.sql.catalyst.expressions._
import org.apache.spark.sql.execution.{QueryExecution, SQLExecution}
import org.apache.spark.sql.types.{BinaryType, StringType}
import org.apache.spark.util.Utils

/**
  * The [[RocketMQWriter]] class is used to write data from a batch query
  * or structured streaming query, given by a [[QueryExecution]], to RocketMQ.
  * The data is assumed to have body column and an optional topic and tag
  * column. If the topic column is missing, then the topic must come from
  * the 'topic' configuration option.
  */
private[rocketmq010] object RocketMQWriter extends Logging {
  val TOPIC_ATTRIBUTE_NAME: String = "topic"
  val TAGS_ATTRIBUTE_NAME: String = "tags"
  val BODY_ATTRIBUTE_NAME: String = "body"

  override def toString: String = "RocketMQWriter"

  def validateQuery(
      schema: Seq[Attribute],
      rocketmqParameters: ju.Map[String, String],
      topic: Option[String] = None): Unit = {
    schema.find(_.name == TOPIC_ATTRIBUTE_NAME).getOrElse(
      if (topic.isEmpty) {
        throw new AnalysisException(s"topic option required when no " +
            s"'$TOPIC_ATTRIBUTE_NAME' attribute is present. Use the " +
            s"${RocketMQSourceProvider.TOPIC_OPTION_KEY} option for setting a topic.")
      } else {
        Literal(topic.get, StringType)
      }
    ).dataType match {
      case StringType => // good
      case _ =>
        throw new AnalysisException(s"Topic type must be a String")
    }
    schema.find(_.name == TAGS_ATTRIBUTE_NAME).getOrElse(
      Literal(null, StringType)
    ).dataType match {
      case StringType => // good
      case _ =>
        throw new AnalysisException(s"$TAGS_ATTRIBUTE_NAME attribute type must be String")
    }
    schema.find(_.name == BODY_ATTRIBUTE_NAME).getOrElse(
      throw new AnalysisException(s"Required attribute '$BODY_ATTRIBUTE_NAME' not found")
    ).dataType match {
      case StringType | BinaryType => // good
      case _ =>
        throw new AnalysisException(s"$BODY_ATTRIBUTE_NAME attribute type " +
            s"must be a String or BinaryType")
    }
  }

  def write(
      sparkSession: SparkSession,
      queryExecution: QueryExecution,
      rocketmqParameters: ju.Map[String, String],
      topic: Option[String] = None): Unit = {
    val schema = queryExecution.analyzed.output
    validateQuery(schema, rocketmqParameters, topic)
    queryExecution.toRdd.foreachPartition { iter =>
      val writeTask = new RocketMQWriteTask(rocketmqParameters, schema, topic)
      Utils.tryWithSafeFinally(block = writeTask.execute(iter))(
        finallyBlock = writeTask.close())
    }
  }
}
