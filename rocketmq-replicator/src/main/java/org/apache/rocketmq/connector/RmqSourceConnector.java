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
package org.apache.rocketmq.connector;

import io.openmessaging.KeyValue;
import io.openmessaging.connector.api.Task;
import io.openmessaging.connector.api.source.SourceConnector;
import org.apache.rocketmq.client.exception.MQClientException;
import org.apache.rocketmq.common.MixAll;
import org.apache.rocketmq.common.message.MessageQueue;
import org.apache.rocketmq.common.protocol.route.QueueData;
import org.apache.rocketmq.common.protocol.route.TopicRouteData;
import org.apache.rocketmq.connector.common.ConstDefine;
import org.apache.rocketmq.connector.common.Utils;
import org.apache.rocketmq.connector.config.ConfigDefine;
import org.apache.rocketmq.connector.config.DataType;
import org.apache.rocketmq.connector.config.TaskDivideConfig;
import org.apache.rocketmq.connector.strategy.DivideStrategyEnum;
import org.apache.rocketmq.connector.strategy.DivideTaskByQueue;
import org.apache.rocketmq.connector.strategy.DivideTaskByTopic;
import org.apache.rocketmq.connector.strategy.TaskDivideStrategy;
import org.apache.rocketmq.remoting.RPCHook;
import org.apache.rocketmq.tools.admin.DefaultMQAdminExt;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.*;

public class RmqSourceConnector extends SourceConnector {

    private static final Logger log = LoggerFactory.getLogger(RmqSourceConnector.class);

    private boolean syncDLQ = false;

    private boolean syncRETRY = false;

    private KeyValue config;

    private Map<String, List<MessageQueue>> topicRouteMap;

    private TaskDivideStrategy taskDivideStrategy;

    private Set<String> whiteList;

    private volatile boolean started = false;

    private volatile boolean configValid = false;

    private DefaultMQAdminExt defaultMQAdminExt;

    private int taskParallelism = 1;

    public RmqSourceConnector() {

        topicRouteMap = new HashMap<String, List<MessageQueue>>();
        whiteList = new HashSet<String>();
    }

    public String verifyAndSetConfig(KeyValue config) {

        // check the need key.
        for(String requestKey : ConfigDefine.REQUEST_CONFIG){
            if(!config.containsKey(requestKey)){
                return "Request config key: " + requestKey;
            }
        }

        // check the whitelist, whitelist is required.
        String whileListStr = this.config.getString(ConfigDefine.CONN_WHITE_LIST);
        String[] wl = whileListStr.trim().split(",");
        if (wl.length <= 0) return "White list must be not empty.";
        else {
            for (String t: wl) {
                this.whiteList.add(t.trim());
            }
        }

        if (this.config.containsKey(ConfigDefine.CONN_TASK_DIVIDE_STRATEGY) &&
                this.config.getInt(ConfigDefine.CONN_TASK_DIVIDE_STRATEGY) == DivideStrategyEnum.BY_QUEUE.ordinal()) {
            this.taskDivideStrategy = new DivideTaskByQueue();
        } else {
            this.taskDivideStrategy = new DivideTaskByTopic();
        }

        if (config.containsKey(ConfigDefine.CONN_TASK_PARALLELISM)) {
            this.taskParallelism = this.config.getInt(ConfigDefine.CONN_TASK_PARALLELISM);
        }

        this.config = config;
        this.configValid = true;
        return "";
    }

    public void start() {
      
        if (configValid) {
            RPCHook rpcHook = null;
            this.defaultMQAdminExt = new DefaultMQAdminExt(rpcHook);
            this.defaultMQAdminExt.setNamesrvAddr(this.config.getString(ConfigDefine.CONN_SOURCE_RMQ));
            this.defaultMQAdminExt.setAdminExtGroup(Utils.createGroupName(ConstDefine.REPLICATOR_ADMIN_PREFIX));
            this.defaultMQAdminExt.setInstanceName(Utils.createInstanceName(this.config.getString(ConfigDefine.CONN_SOURCE_RMQ)));
            try {
                defaultMQAdminExt.start();
            } catch (MQClientException e) {
                log.error("Replicator start failed for `defaultMQAdminExt` exception.", e);
            }
            started = true;
        }
    }

    public void stop() {
        if (started) {
            if (defaultMQAdminExt != null) {
                defaultMQAdminExt.shutdown();
            }
            started = false;
        }
    }

    public void pause() {

    }

    public void resume() {

    }

    public Class<? extends Task> taskClass() {
      
        return RmqSourceTask.class;
    }

    public List<KeyValue> taskConfigs() {
      
        if (started && configValid) {
            try {
                for (String topic : this.whiteList) {
                    if ((syncRETRY && topic.startsWith(MixAll.RETRY_GROUP_TOPIC_PREFIX)) ||
                            (syncDLQ && topic.startsWith(MixAll.DLQ_GROUP_TOPIC_PREFIX)) ||
                            !topic.equals(ConfigDefine.CONN_STORE_TOPIC)) {

                        TopicRouteData topicRouteData = defaultMQAdminExt.examineTopicRouteInfo(topic);
                        if (!topicRouteMap.containsKey(topic)) {
                            topicRouteMap.put(topic, new ArrayList<MessageQueue>());
                        }
                        for (QueueData qd : topicRouteData.getQueueDatas()) {
                            for (int i = 0; i < qd.getReadQueueNums(); i++) {
                                MessageQueue mq = new MessageQueue(topic, qd.getBrokerName(), i);
                                topicRouteMap.get(topic).add(mq);
                            }
                        }
                    }
                }
            } catch (Exception e) {
                log.error("Fetch topic list error.", e);
            }

            TaskDivideConfig tdc = new TaskDivideConfig(
                    this.config.getString(ConfigDefine.CONN_SOURCE_RMQ),
                    this.config.getString(ConfigDefine.CONN_STORE_TOPIC),
                    this.config.getString(ConfigDefine.CONN_SOURCE_RECORD_CONVERTER),
                    DataType.COMMON_MESSAGE.ordinal(),
                    this.taskParallelism
            );
            return this.taskDivideStrategy.divide(this.topicRouteMap, tdc);
        } else {
            return new ArrayList<KeyValue>();
        }
    }
}

