#!/bin/bash

# Build rocketmq
docker build -t huanwei/rocketmq:4.3.0 --build-arg version=4.3.0 ./rocketmq

# Stop and remove existed containers if any
docker rm -f $(docker ps -a|awk '/huanwei\/rocketmq/ {print $1}')

# Run namesrv and broker
docker run -d -p 9876:9876 -v `pwd`/data/namesrv/logs:/root/logs -v `pwd`/data/namesrv/store:/root/store --name rmqnamesrv  huanwei/rocketmq:4.3.0 sh mqnamesrv
docker run -d -p 10911:10911 -p 10909:10909 -v `pwd`/data/broker/logs:/root/logs -v `pwd`/data/broker/store:/root/store --name rmqbroker --link rmqnamesrv:namesrv -e "NAMESRV_ADDR=namesrv:9876" huanwei/rocketmq:4.3.0 sh mqbroker