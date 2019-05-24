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
 *    2017/05/15
 *    version 0.0.9
 *    注册中心通用函数定义
 */

#ifndef ORIENTSEC_UTILS_H
#define ORIENTSEC_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "url.h"
#include "orientsec_types.h"
#include<string.h>

//从注册路径中提取服务名,/orientsec/com.orientsec.hello.greeter/providers中提取出com.orientsec.hello.greeter
int get_service_name_from_path(const char* path, char* service_name, int buf_len);

//从provider_t结构体转化成url_t结构
url_t* url_from_provider(provider_t *provider);

//从url_t结构转成provider_t结构体
provider_t* provider_from_url(url_t *url);

//使用url_t结构数据初始化provider_t结构体
void init_provider_from_url(provider_t *provider ,url_t *url);

//从现有provider中克隆provider信息
provider_t* clone_provider(provider_t* src);

//释放provider空间,释放ext_data空间
void free_provider_ex(provider_t **p);

//释放provider空间,是否ext_data空间
void free_provider_v2_ex(provider_t *p);

/*
* func:根据provider数据，accessprotected属性生成路由规则
* date:20170821
* auth:addbyhuyn
*/
url_t * url_for_router_from_param(char *host_ip, char *service_name);


//int orientsec_stricmp(const char* a, const char* b);
#ifdef __cplusplus
}
#endif
#endif // !ORIENTSEC_UTILS_H
