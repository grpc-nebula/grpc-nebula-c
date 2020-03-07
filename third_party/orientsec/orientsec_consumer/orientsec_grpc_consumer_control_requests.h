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
 *  请求数控制
 *    addbyhuyn
 */

#pragma once
#ifndef ORIENTSEC_GRPC_CONSUMER_CONTROL_REQUESTS_H
#define ORIENTSEC_GRPC_CONSUMER_CONTROL_REQUESTS_H

#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

  //请求数控制结构体
  typedef struct _orientsec_grpc_consumer_request {
                long currentrequests;
                int64_t timestamp;
  } orientsec_grpc_consumer_request_t;

  //最大请求数控制锁初始化
  void orientsec_grpc_consumer_request_mu_init();

  void orientsec_grpc_consumer_update_maxrequest(char * servicename, long requestnum);

  //校验制定服务是否需要进行请求数控制
  int orientsec_grpc_consumer_control_requests(const char * fullmethod);

  //reset clientId(注册时填写的信息)调用providerId(provider_ip:provider_port)失败信息
  void reset_provider_failure(char* clientId, char* providerId,const char* methods);

  // obtain retry times to consumer call
  int orientsec_get_failure_retry_times();

#ifdef __cplusplus
}
#endif
#endif // !ORIENTSEC_GRPC_CONSUMER_CONTROL_REQUESTS_H



