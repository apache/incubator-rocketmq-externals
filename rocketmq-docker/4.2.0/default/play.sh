#!/bin/bash

docker build -t apache/rocketmq-namesrv:4.2.0 --build-arg version=4.2.0 ./namesrv

docker build -t apache/rocketmq-broker:4.2.0 --build-arg version=4.2.0 ./broker

docker run -d -p 9876:9876 --name rmqnamesrv  apache/rocketmq-namesrv:4.2.0

docker run -d -p 10911:10911 -p 10909:10909 --name rmqbroker --link rmqnamesrv:namesrv -e "NAMESRV_ADDR=namesrv:9876" apache/rocketmq-broker:4.2.0