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
 *    注册中心接口函数定义
 */

#ifndef ORIENTSEC_GRPC_REGISTRY_INTF_H
#define ORIENTSEC_GRPC_REGISTRY_INTF_H

#include "url.h"
#include "registry_service.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum { PUBLIC_REG = 0, PRIVATE_REG, HYBRID_REG} zookeeper_reg;

// exposed for consumer select registry center
// all used in internal module
bool orientsec_grpc_registry_zk_intf_init();

void zk_prov_reg_init(char*);
char* get_pub_reg_center();
char* get_pri_reg_center();
void write_pub_addr(char* addr, size_t size);
void write_pri_addr(char* addr, size_t size);

// for registry action check
zookeeper_reg get_cons_reg_scheme();
void set_cons_reg_scheme(zookeeper_reg sch);
zookeeper_reg get_prov_reg_scheme();
void set_prov_reg_scheme(zookeeper_reg sch);


/**
* 注册数据，比如：提供者地址，消费者地址，路由规则，覆盖规则，等数据。
* 注册需处理契约：<br>
* 1. 当URL设置了dynamic=false参数，则需持久存储，否则，当注册者出现断电等情况异常退出时，需自动删除。<br>
* 2. 当URL设置了category=routers时，表示分类存储，缺省类别为providers，可按分类部分通知数据。<br>
* 3. 当注册中心重启，网络抖动，不能丢失数据，包括断线自动删除数据。<br>
* 4. 允许URI相同但参数不同的URL并存，不能覆盖。<br>
*
* @param url 注册信息，不允许为空，如：
*            grpc://192.168.1.211/com.orientsec.grpc.BarService?version=1.0.0&application=test
*/
void registry(url_t *url);

/**
* 取消注册.
* 取消注册需处理契约：<br>
* 1. 如果是dynamic=false的持久存储数据，找不到注册数据，则抛IllegalStateException，否则忽略。<br>
* 2. 按全URL匹配取消注册。<br>
*
* @param url 注册信息，不允许为空，如：
*            grpc://192.168.1.211/com.orientsec.grpc.BarService?version=1.0.0&application=test
*/
void unregistry(url_t *url);

/**
* 订阅符合条件的已注册数据，当有注册数据变更时自动推送.
* 订阅需处理契约：<br>
* 1. 当URL设置了category=routers，只通知指定分类的数据<br>
* 2. 允许以interface,group,version,classifier作为条件查询，如：
*    interface=com.alibaba.foo.BarService&version=1.0.0<br>
* 3. 当注册中心重启，网络抖动，需自动恢复订阅请求。<br>
* 4. 允许URI相同但参数不同的URL并存，不能覆盖。<br>
* 5. 必须阻塞订阅过程，等第一次通知完后再返回。<br>
*
* @param url      订阅条件，不允许为空，如：
*     consumer://192.168.1.211/com.orientsec.grpc.BarService?category=providers&version=1.0.0&application=test
* @param listener 变更事件监听器，不允许为空
*/
void subscribe(url_t *url, registry_notify_f notify_f);
/**
* 取消订阅.
* 取消订阅需处理契约：<br>
* 1. 如果没有订阅，直接忽略。<br>
* 2. 按全URL匹配取消订阅。<br>
*
* @param url      订阅条件，不允许为空，如：
*      consumer://192.168.1.211/com.orientsec.grpc.BarService?version=1.0.0&application=test
* @param listener 变更事件监听器，不允许为空
*/
void unsubscribe(url_t *url, registry_notify_f notify_f);
/**
* 查询符合条件的已注册数据，与订阅的推模式相对应，这里为拉模式，只返回一次结果.<br>
*
* @param url 查询条件，不允许为空，如：
*      consumer://192.168.1.211/com.orientsec.grpc.BarService?version=1.0.0&application=test
* @return 已注册信息列表，可能为空，含义同
*
*/
url_t* lookup(url_t *url,int *nums,int *pri);

/**
* 提取指定目录下的数据
* @param path  zk节点下的路径
* @return 数据
*/
char* getData(const char* path);


/**
* 关闭注册中心，释放资源
* 无参数，无返回值
**/
void shutdown_registry();


void consumer_providers_callback(url_t *urls, int url_num);

#ifdef __cplusplus
}
#endif

#endif
