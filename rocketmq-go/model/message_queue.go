/*
Licensed to the Apache Software Foundation (ASF) under one or more
contributor license agreements.  See the NOTICE file distributed with
this work for additional information regarding copyright ownership.
The ASF licenses this file to You under the Apache License, Version 2.0
(the "License"); you may not use this file except in compliance with
the License.  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

package model

type MessageQueue struct {
	Topic      string `json:"topic"`
	BrokerName string `json:"brokerName"`
	QueueId    int32  `json:"queueId"`
}

func (m *MessageQueue) clone() *MessageQueue {
	no := new(MessageQueue)
	no.Topic = m.Topic
	no.QueueId = m.QueueId
	no.BrokerName = m.BrokerName
	return no
}

type MessageQueues []*MessageQueue

func (m MessageQueues) Less(i, j int) bool {
	imq := m[i]
	jmq := m[j]

	if imq.Topic < jmq.Topic {
		return true
	} else if imq.Topic < jmq.Topic {
		return false
	}

	if imq.BrokerName < jmq.BrokerName {
		return true
	} else if imq.BrokerName < jmq.BrokerName {
		return false
	}

	if imq.QueueId < jmq.QueueId {
		return true
	} else {
		return false
	}
}

func (m MessageQueues) Swap(i, j int) {
	m[i], m[j] = m[j], m[i]
}

func (m MessageQueues) Len() int {
	return len(m)
}

func (m MessageQueue) Equals(messageQueue *MessageQueue) bool {
	if m.QueueId != messageQueue.QueueId {
		return false
	}
	if m.Topic != messageQueue.Topic {
		return false
	}
	if m.BrokerName != messageQueue.BrokerName {
		return false
	}
	return true
}
