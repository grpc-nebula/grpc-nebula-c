/*
 * Copyright 2019 Orient Securities Co., Ltd.
 * Copyright 2019 BoCloud Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 *    Author : huyn
 *    2017/06/06
 *    version 0.0.9
 *    CONSUMER端相关常量常量定义
 */

#pragma once
#ifndef ORIENTSEC_GRPC_CONSUMER_CONTANTS_H
#define ORIENTSEC_GRPC_CONSUMER_CONTANTS_H

#ifdef __cplusplus
extern "C" {
#endif

#define PROVIDERS_LISTENER_KEY   "providers"
#define ROUTERS_LISTENER_KEY  "routers"
#define CONFIGURATORS_LISTENER_KEY "configurators"

#define HSTC_GRPC_CONSUMER_ZK_SCHEME  "zookeeper:///"

#define HSTC_GRPC_CONSUMER_SCHEME  "consumer://"

#define HSTC_GRPC_CONSUMER_URL_SCHEME  "grpc://"

#define HSTC_GRPC_CONSUMER_URL_LEN 1000

#define HSTC_GRPC_CONSUMER_URL_CHAR_EQ "="

#define HSTC_GRPC_CONSUMER_URL_CHAR_MH ":"

#define HSTC_GRPC_CONSUMER_URL_CHAR_XG "/"

#define HSTC_GRPC_CONSUMER_CONFITEM_NUM 30

#define HSTC_GRPC_CONSUMER_URL_SEP  "://"

//providrs 历史记录保存时长，单位毫秒。
#define HSTC_GRPC_CONSUMER_INTERVAL_MILISECOND 5000

#ifdef __cplusplus
}
#endif
#endif // !ORIENTSEC_GRPC_CONSUMER_CONTANTS_H
