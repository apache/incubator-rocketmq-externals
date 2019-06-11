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

package org.apache.rocketmq.connect.runtime.utils.datasync;

import io.openmessaging.MessagingAccessPoint;
import io.openmessaging.connector.api.data.Converter;
import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import org.apache.rocketmq.client.consumer.DefaultMQPushConsumer;
import org.apache.rocketmq.client.consumer.MessageSelector;
import org.apache.rocketmq.client.exception.MQClientException;
import org.apache.rocketmq.client.impl.consumer.DefaultMQPushConsumerImpl;
import org.apache.rocketmq.client.impl.producer.DefaultMQProducerImpl;
import org.apache.rocketmq.client.producer.DefaultMQProducer;
import org.apache.rocketmq.client.producer.SendCallback;
import org.apache.rocketmq.common.message.Message;
import org.apache.rocketmq.common.message.MessageExt;
import org.apache.rocketmq.connect.runtime.config.ConnectConfig;
import org.apache.rocketmq.remoting.exception.RemotingException;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.junit.MockitoJUnitRunner;

import static org.mockito.Mockito.any;
import static org.mockito.Mockito.anyString;
import static org.mockito.Mockito.doReturn;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;

@RunWith(MockitoJUnitRunner.class)
public class BrokerBasedLogTest {

    @Mock
    private DefaultMQProducer producer;

    @Mock
    private DefaultMQPushConsumer consumer;

    private String queueName;

    private String consumerGroup;

    private String producerGroup;

    private BrokerBasedLog brokerBasedLog;

    @Mock
    private MessagingAccessPoint messagingAccessPoint;

    @Mock
    private DataSynchronizerCallback dataSynchronizerCallback;

    @Mock
    private MessageExt messageExt;

//    @Mock
//    private Future<SendResult> future;

    @Mock
    private Converter converter;

    private ConnectConfig connectConfig;

    @Mock
    private DefaultMQProducerImpl defaultMQProducerImpl;

    @Mock
    private DefaultMQProducer defaultMQProducer;

    @Before
    public void init() throws IllegalAccessException, NoSuchFieldException {
        queueName = "testQueueName";
        consumerGroup = "testConsumerGroup1";
        producerGroup = "testProducerGroup1";
        connectConfig = new ConnectConfig();
        connectConfig.setRmqConsumerGroup(consumerGroup);
        connectConfig.setRmqProducerGroup(producerGroup);
        connectConfig.setNamesrvAddr("127.0.0.1:9876");
        connectConfig.setRmqMinConsumeThreadNums(1);
        connectConfig.setRmqMaxConsumeThreadNums(32);
        connectConfig.setRmqMessageConsumeTimeout(3 * 1000);
//        doReturn().when(defaultMQProducerImpl).

//        final Method defaultMQProducerImplMethod = DefaultMQProducer.class.getDeclaredMethod("defaultMQProducerImpl");
//        defaultMQProducerImplMethod.setAccessible(true);
//        defaultMQProducerImplMethod.invoke(producer, defaultMQProducerImpl);

//        final Method defaultMQProducerMethod = DefaultMQProducerImpl.class.getDeclaredMethod("defaultMQProducer");
//        defaultMQProducerMethod.setAccessible(true);
//        defaultMQProducerMethod.invoke(defaultMQProducerImpl, defaultMQProducer);
//
//        doReturn(producerGroup).when(defaultMQProducer).getProducerGroup();

//        doReturn().when(producer).start();
//        doReturn(producer).when(messagingAccessPoint).createProducer();
//        doReturn(consumer).when(messagingAccessPoint).createPushConsumer(any(KeyValue.class));
//        doReturn(bytesMessage).when(producer).createBytesMessage(anyString(), any(byte[].class));
//        when(producer).send(any(Message.class), any(SendResult.class));
//        verify(producer, times(1)).send(any(Message.class), any(SendCallback.class));

        doReturn(new byte[0]).when(converter).objectToByte(any(Object.class));
        brokerBasedLog = new BrokerBasedLog(connectConfig, queueName, consumerGroup, dataSynchronizerCallback, converter, converter);

        final Field producerField = BrokerBasedLog.class.getDeclaredField("producer");
        producerField.setAccessible(true);
        producerField.set(brokerBasedLog, producer);

        final Field consumerField = BrokerBasedLog.class.getDeclaredField("consumer");
        consumerField.setAccessible(true);
        consumerField.set(brokerBasedLog, consumer);

    }

    @Test
    public void testStart() throws MQClientException {
        brokerBasedLog.start();
        verify(producer, times(1)).start();
        verify(consumer, times(1)).subscribe(anyString(), anyString());
        verify(consumer, times(1)).start();
    }

    @Test
    public void testStop() {
        brokerBasedLog.stop();
        verify(producer, times(1)).shutdown();
        verify(consumer, times(1)).shutdown();
    }

    @Test
    public void testSend() throws RemotingException, MQClientException, InterruptedException, NoSuchMethodException, InvocationTargetException, IllegalAccessException {
        brokerBasedLog.send(new Object(), new Object());
        verify(producer, times(1)).send(any(Message.class), any(SendCallback.class));
    }

}