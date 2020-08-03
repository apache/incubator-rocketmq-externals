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

package org.apache.rocketmq.connect.tools.command;

import org.apache.commons.cli.CommandLine;
import org.apache.commons.cli.Options;
import org.apache.rocketmq.connect.tools.commom.Config;
import org.apache.rocketmq.connect.tools.utils.RestSender;
import org.apache.rocketmq.tools.command.SubCommandException;

public class GetConfigInfoSubCommand implements SubCommand {

    private final Config config;

    public GetConfigInfoSubCommand(Config config) {
        this.config = config;
    }

    @Override
    public String commandName() {
        return "getConfigInfo";
    }

    @Override
    public String commandDesc() {
        return "Get all configuration information";
    }

    @Override
    public Options buildCommandlineOptions(Options options) {
        return options;
    }

    @Override
    public void execute(CommandLine commandLine, Options options) throws SubCommandException {
        try {
            String url = "http://" + config.getHttpAddr() + ":" + config.getHttpPort() + "/" + commandName();
            System.out.printf("Send request to %s %n", url);
            String result = new RestSender().sendHttpRequest(url, "");
            System.out.printf("%s%n", result);
        } catch (Exception e) {
            throw new SubCommandException(this.getClass().getSimpleName() + " command failed", e);
        }
    }
}
