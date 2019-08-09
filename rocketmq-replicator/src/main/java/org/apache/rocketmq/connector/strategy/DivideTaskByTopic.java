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
package org.apache.rocketmq.connector.strategy;

import com.alibaba.fastjson.JSONObject;
import io.openmessaging.KeyValue;
import io.openmessaging.internal.DefaultKeyValue;
import org.apache.rocketmq.common.message.MessageQueue;
import org.apache.rocketmq.connector.config.*;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class DivideTaskByTopic extends TaskDivideStrategy {

    public List<KeyValue> divide(Map<String, List<MessageQueue>> topicRouteMap, TaskDivideConfig tdc) {

        List<KeyValue> config = new ArrayList<KeyValue>();
        int parallelism = tdc.getTaskParallelism();
        int id = -1;
        Map<Integer, List<TaskTopicInfo>> taskTopicList = new HashMap<Integer, List<TaskTopicInfo>>();
        for (String t: topicRouteMap.keySet()) {
            int ind = ++id%parallelism;
            if (!taskTopicList.containsKey(ind)) {
                taskTopicList.put(ind, new ArrayList<TaskTopicInfo>());
            }
            taskTopicList.get(ind).add(new TaskTopicInfo(t, "", ""));
        }

        for (int i=0; i<parallelism; i++) {
            KeyValue keyValue = new DefaultKeyValue();
            keyValue.put(TaskConfigEnum.TASK_STORE_ROCKETMQ.getKey(), tdc.getStoreTopic());
            keyValue.put(TaskConfigEnum.TASK_SOURCE_ROCKETMQ.getKey(), tdc.getSourceNamesrvAddr());
            keyValue.put(TaskConfigEnum.TASK_DATA_TYPE.getKey(), DataType.COMMON_MESSAGE.ordinal());
            keyValue.put(TaskConfigEnum.TASK_TOPIC_INFO.getKey(), JSONObject.toJSONString(taskTopicList.get(i)));
            keyValue.put(TaskConfigEnum.TASK_SOURCE_RECORD_CONVERTER.getKey(), tdc.getSrcRecordConverter());
            config.add(keyValue);
        }

        return config;
    }
}
