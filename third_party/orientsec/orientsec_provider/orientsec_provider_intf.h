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
 *    Author : heiden deng(dengjianquan@beyondcent.com)
 *    2017/06/15
 *    version 0.0.9
 *    provider通用操作函数
 */
#ifndef ORIENTSEC_PROVIDER_INTF_H
#define ORIENTSEC_PROVIDER_INTF_H

#include<stdbool.h>
#include "../orientsec_common/orientsec_types.h"
//#include "orientsec_types.h"

#ifdef __cplusplus
extern "C" {
#endif

//注册Provider，并订阅configurators目录
void provider_registry(int port, const char *sIntf, const char *sMethods);

//注销本应用所有的服务并取消订阅
void providers_unregistry();

#define ORIENTSEC_GRPC_PROVIDER_TOO_MANY_CONNS "Cancelled client connection because concurrent connect upper limit "
#define ORIENTSEC_GRPC_PROVIDER_TOO_MANY_REQUEST "Cancelled service called because concurrent request exceed MAX request"
#define ORIENTSEC_GRPC_PROVIDER_IN_ACCESS_PROTECTED "Cancelled service called because service being in access protected status"

//检查指定服务并发连接数满足条件，即 0 < 当前连接数 + 1 <=max,满足条件时，连接数+1，并返回true,否则返回false
bool check_provider_connection(const char *intf);

//减少Provider的当前并发连接计数
void decrease_provider_connection(const char *intf);

//检查指定服务并发请求数满足条件，即 0 < 当前并发请求数 + 1 <=max,满足条件时，连接数+1，并返回true,否则返回false
bool check_provider_request(const char *intf);

//减少provider的当前并发请求计数
void decrease_provider_request(const char *intf);

//检查服务是否过期修改为在call时调用
//bool check_provider_deprecated(const char *intf);

//提交provider到链表中
void add_provider_to_list(provider_t *provider);

//检查服务是否处于保护状态,处于保护状态返回true
//bool check_provider_access_protected(const char *intf);


//更新provider 最大并发连接数,intf为空时，更新所有的provider
void update_provider_connection(const char *intf, int conns);

//更新provider 最大并发请求数,intf为空时，更新所有的provider
void update_provider_request(const char *intf, int req);

//更新provider deprecated属性,intf为空时，更新所有的provider
void update_provider_deprecated(const char *intf, bool deprecated);

//更新provider access_protected属性,intf为空时，更新所有的provider
void update_provider_access_protected(const char *intf, bool access_protected);

#ifdef __cplusplus
}
#endif

#endif // !ORIENTSEC_PROVIDER_INTF_H


