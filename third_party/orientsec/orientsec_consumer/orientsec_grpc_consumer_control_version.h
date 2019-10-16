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
 * func:服务版本控制
 * auth:addbyhuyn
 * date:20170816
 */

#pragma once
#ifndef ORIENTSEC_GRPC_CONSUMER_CONTROL_VERSION_H
#define ORIENTSEC_GRPC_CONSUMER_CONTROL_VERSION_H

#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif

#define ORIENTSEC_GRPC_SERVICE_SEPARATOR_CHAR ','

#define ORIENTSEC_GRPC_VERSION_SEPARATOR_CHAR ':'

#define ORIENTSEC_GRPC_SERVICE_SEPARATOR_STR ","

#define ORIENTSEC_GRPC_VERSION_SEPARATOR_STR ":"

#define ORIENTSEC_GRPC_SERVICE_VERSION_COMMON_KEY "_common_service_version"
  //初始化版本对象
  void orientsec_grpc_service_version_init();
  //获取版本控制信息
  void orientsec_grpc_consumer_control_version_update(char *service_name, char *service_version);

  //校验服务版本是否匹配
  bool orientsec_grpc_consumer_control_version_match(const char *servername, char* version);

  //校验服务版本是否变动
  bool orientsec_grpc_version_changed_conn();
  //重置服务版本是否变动标记
  void orientsec_reset_grpc_version_changed_conn();
        	
  //判断是否是请求负载均衡，否则是连接负载均衡
  bool is_request_loadbalance();

#ifdef __cplusplus
}
#endif


#endif // !ORIENTSEC_GRPC_CONSUMER_CONTROL_VERSION_H



