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
 *    2017/07/11
 *    version 0.0.9  
 *    consumer项目提供的接口函数
 */

#ifndef ORIENTSEC_CONSUMER_INTF_H
#define ORIENTSEC_CONSUMER_INTF_H

#include "orientsec_types.h"
//addbylm
#include "src/core/ext/filters/client_channel/lb_policy_registry.h"

#ifdef __cplusplus
extern "C" {
#endif

//手动根据服务名查询provider，更新缓存
void get_all_providers_by_name(const char*service_name);

//根据服务名查询provider，返回provider_t数组（只返回一个有效服务）//addbylm
provider_t* consumer_query_providers_write_point_policy(
    const char* service_name, grpc_core::LoadBalancingPolicy* lb_policy,
    int* nums, char* method_name);

//根据服务名查询provider，返回provider_t数组（只返回一个有效服务）
provider_t* consumer_query_providers(const char *service_name, int *nums,char*hasharg,char * method_name);

//从可用的provider列表里算出对应的provider，将host/port存入policy
int get_index_from_lb_aglorithm(const char *service_name, provider_t *provider, const int*nums, char * hash_info, const char * stragry);


//根据服务名查询provider，返回provider_t数组(provider不在黑名单列表内)
provider_t* consumer_query_providers_all(const char *service_name, int *nums);

//consumer注册，返回注册的url串。
char* orientsec_grpc_consumer_register(const char* fullmethod,
                                       const char* frameversion);

//consumer取消注册。
int orientsec_grpc_consumer_unregister(char * reginfo);

//从target中提取出服务名，返回空间需要释放,例如zookeeper:///com.orientsec.grpc.hello.greeter返回
//com.orientsec.grpc.hello.greeter
char* orientsec_grpc_get_sn_from_target(char* target);

//标记clientId(注册时填写的信息)调用providerId(provider_ip:provider_port)失败信息
void record_provider_failure(char* clientId, char* providerId,char* methods);

//获取backoff算法参数
int get_max_backoff_time() ;
 
//获取经过服务版本检测后的provider数量
int provider_num_after_service_check(char* service_name);

//获取provider的active数量
int provider_num_active_check(char* service_name);

// 设置provider是否提供服务属性
void provider_active_standby_setting(char* service_name, bool have_active);

//获取经过黑白名单处理之后用于负载均衡的provider数量
int get_consumer_lb_providers_acount(char *service_name);

//获取可以调用的provider数量，用于容错切换
int get_valid_providers_acount(char* service_name, char* method_name);

//获取zk上某服务的provider数量
int get_consumer_providers_acount(char* service_name);

//设置某个provider 调用失败标记
void set_provider_failover_flag(char* service_name, char *providerId);

//清除某个provider 调用失败标记
void clr_provider_failover_flag(char* service_name, char *providerId);

//设置服务在单位时间内被访问的次数
void set_consmuer_flow_control_threshold(char *service_name, long max_request);

//检查客户端对某个服务的单位时间内的请求计数，如果超过最大值则返回false,否则增加计数并返回true
bool check_consumer_flow_control(char *service_name);

//检查对某个服务的请求计数
void decrease_consumer_request_count(char *service_name);

 // 判断是否发生方法级的负载均衡改变
bool orientsec_method_lb_changed();
// reset method lb flag
void orientset_method_lb_reset();
// 判断是否主备切换，重新resolve
bool orientsec_active_standby_changed();
// reset active/standby flag
void orientsec_active_standby_reset();

// 判断是否group属性发生改变，重新resolve
bool orientsec_group_grade_changed();
// reset group/grade flag
void orientsec_group_grade_reset();

// 判断是否需要resolve
bool orientsec_need_resolved();
// reset need_resolve flag
void orientsec_need_resolved_reset();

// switch provider every 10min when connection mode
void check_elapse_time_reach_setting();
bool orientsec_switch_when_connection_resolved();
void orientsec_switch_when_connection_resolved_reset();

#ifdef __cplusplus
}
#endif

#endif // !ORIENTSEC_CONSUMER_INTF_H



