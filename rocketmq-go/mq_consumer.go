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
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */
package rocketmq

import (
	"github.com/apache/incubator-rocketmq-externals/rocketmq-go/model"
	"github.com/apache/incubator-rocketmq-externals/rocketmq-go/service"
)

type MQConsumer interface {
	SendMessageBack(msgExt *message.MessageExt, delayLevel int, brokerName string) error
	FetchSubscribeMessageQueues(topic string) error
}

type MqConsumerConfig struct {
}
type DefaultMQPushConsumer struct {
	offsetStore           service.OffsetStore //for consumer's offset
	mqClient              service.RocketMqClient
	rebalance             *service.Rebalance //Rebalance's impl depend on offsetStore
	consumeMessageService service.ConsumeMessageService
	consumerConfig        *MqConsumerConfig

	consumerGroup string
	//consumeFromWhere      string
	consumeType  string
	messageModel string
	unitMode     bool

	subscription    map[string]string   //topic|subExpression
	subscriptionTag map[string][]string // we use it filter again
	// 分配策略
	pause bool //when reset offset we need pause
}

func NewDefaultMQPushConsumer(consumerGroup string, mqConsumerConfig *MqConsumerConfig) (defaultMQPushConsumer *DefaultMQPushConsumer) {
	defaultMQPushConsumer = &DefaultMQPushConsumer{}
	defaultMQPushConsumer.consumerConfig = mqConsumerConfig
	return
}

func (self *DefaultMQPushConsumer) RegisterMessageListener(messageListener model.MessageListener) {
	self.consumeMessageService = service.NewConsumeMessageConcurrentlyServiceImpl(messageListener)
}
func (self *DefaultMQPushConsumer) Subscribe(topic string, subExpression string) {
	//self.subscription[topic] = subExpression
	//if len(subExpression) == 0 || subExpression == "*" {
	//	return
	//}
	//tags := strings.Split(subExpression, "||")
	//tagsList := []string{}
	//for _, tag := range tags {
	//	t := strings.TrimSpace(tag)
	//	if len(t) == 0 {
	//		continue
	//	}
	//	tagsList = append(tagsList, t)
	//}
	//if len(tagsList) > 0 {
	//	self.subscriptionTag[topic] = tagsList
	//}
}
