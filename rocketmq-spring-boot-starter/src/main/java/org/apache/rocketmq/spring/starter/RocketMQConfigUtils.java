package org.apache.rocketmq.spring.starter;

/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


public class RocketMQConfigUtils {
  /**
   * The bean name of the internally managed RocketMQ transaction annotation processor.
   */
  public static final String ROCKET_MQ_TRANSACTION_ANNOTATION_PROCESSOR_BEAN_NAME =
      "org.springframework.rocketmq.spring.starter.internalRocketMQTransAnnotationProcessor";

  public static final String ROCKET_MQ_TRANSACTION_DEFAULT_GLOBAL_NAME =
      "rocket_mq_transaction_defaut_global_name";
}
