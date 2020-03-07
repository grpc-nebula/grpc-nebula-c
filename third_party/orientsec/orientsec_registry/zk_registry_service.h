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
 *    Author : liumin(liumin@beyondcent.com)
 *    2019/01/04
 *    version 0.0.9
 *    zookeeper注册中心工厂接口函数定义
 */

#ifndef ORIENTSEC_ZK_REGISTRY_SERVICE_H
#define ORIENTSEC_ZK_REGISTRY_SERVICE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "url.h"
#include "registry_service.h"

//接口实现
void zk_start(registry_service_args_t *param);
void zk_registe(registry_service_args_t *param, url_t *url);
void zk_unregiste(registry_service_args_t *param, url_t *url);
void zk_subscribe(registry_service_args_t *param, url_t *url, registry_notify_f notify);
void zk_unsubscribe(registry_service_args_t *param, url_t *url, registry_notify_f notify);
void zk_lookup(registry_service_args_t *param, url_t *src, url_t **result, int *len);
char* zk_getData(registry_service_args_t* param, char* path);
void zk_stop(registry_service_args_t* param);
void zk_destroy(registry_service_args_t* param);



//工具类函数
/**
* 返回url对应注册中心服务路径，例如,/orientsec/com.orientsec.example.hello
**/
//char *zk_get_service_path(url_t *url);

/**
*返回url对应注册中心的路径，到catogory为止，例如/orientsec/com.orientsec.example.hello/providers
**/
//char *zk_get_category_path(url_t *url);

/**
*返回url对应的注册中心全路径，包括子节点节点名
*
**/
//char* zk_get_url_path(zk_connection_t* conn, url_t* url);
extern int orientsec_stricmp(const char* a, const char* b);

/**
 * 设置节点是否创建成功标识
 *
 **/
void zk_set_create_node_flag(bool);

/**
 * 获取节点是否创建成功标识
 *
 **/
bool zk_get_create_node_flag();

#ifdef __cplusplus
}
#endif
#endif // !ORIENTSEC_ZK_REGISTRY_SERVICE_H
