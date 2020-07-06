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
 *    2017/06/13
 *    version 0.0.9
 *    配置文件字段常量
 */

#ifndef ORIENTSEC_GRPC_PROPERTIES_CONSTANTS_H
#define ORIENTSEC_GRPC_PROPERTIES_CONSTANTS_H

#ifdef __cplusplus
extern "C" {
#endif

#define ORIENTSEC_GRPC_PROPERTIES_COMMON_APP "common.application"
#define ORIENTSEC_GRPC_PROPERTIES_COMMON_GRPC "common.grpc"
// add by liumin
#define ORIENTSEC_GRPC_PROPERTIES_COMMON_PROJECT "common.project"
#define ORIENTSEC_GRPC_PROPERTIES_COMMON_PROJECT_DEFAULT "grpc-test-app"
// add for common.owner property by jianbin
#define ORIENTSEC_GRPC_PROPERTIES_COMMON_OWNER "common.owner"
#define ORIENTSEC_GRPC_PROPERTIES_COMMON_OWNER_DEFAULT "A0000"

// add for common.ops property for register
#define ORIENTSEC_GRPC_PROPERTIES_COMMON_OPS "common.ops"
#define ORIENTSEC_GRPC_PROPERTIES_COMMON_OPS_DEFAULT "B0000"

#define ORIENTSEC_GRPC_PROPERTIES_COMMON_LOCALHOST_IP "common.localhost.ip"
 // add for nominal ip and port for provider
#define ORIENTSEC_GRPC_PROPERTIES_COMMON_SERVICE_IP "common.service.ip"
#define ORIENTSEC_GRPC_PROPERTIES_COMMON_SERVICE_PORT "common.service.port"

#define ORIENTSEC_GRPC_PROPERTIES_P_VERSION "provider.version"
#define ORIENTSEC_GRPC_PROPERTIES_P_SIDE "provider.side"
#define ORIENTSEC_GRPC_PROPERTIES_P_MODULE "provider.module"
#define ORIENTSEC_GRPC_PROPERTIES_P_DEFAULT_TIMEOUT "provider.default.timeout"
#define ORIENTSEC_GRPC_PROPERTIES_P_DEFAULT_RETIES "provider.default.reties"
#define ORIENTSEC_GRPC_PROPERTIES_P_DEFAULT_CONN "provider.default.connections"
#define ORIENTSEC_GRPC_PROPERTIES_P_DEFAULT_REQ "provider.default.requests"
#define ORIENTSEC_GRPC_PROPERTIES_P_DEFAULT_LB "provider.default.loadbalance"
#define ORIENTSEC_GRPC_PROPERTIES_P_DEFAULT_ASYNC "provider.default.async"
#define ORIENTSEC_GRPC_PROPERTIES_P_TOKEN "provider.token"
#define ORIENTSEC_GRPC_PROPERTIES_P_DEPRECATED "provider.deprecated"
#define ORIENTSEC_GRPC_PROPERTIES_P_DYNAMIC "provider.dynamic"
#define ORIENTSEC_GRPC_PROPERTIES_P_ACCESSLOG "provider.accesslog"
#define ORIENTSEC_GRPC_PROPERTIES_P_OWNER "provider.owner"
#define ORIENTSEC_GRPC_PROPERTIES_P_WEIGHT "provider.weight"
#define ORIENTSEC_GRPC_PROPERTIES_P_DEFAULT_CLUSTER "provider.default.cluster"
#define ORIENTSEC_GRPC_PROPERTIES_P_APP_VERSION "provider.application.version"
#define ORIENTSEC_GRPC_PROPERTIES_P_ORG "provider.organization"
#define ORIENTSEC_GRPC_PROPERTIES_P_ENV "provider.environment"
#define ORIENTSEC_GRPC_PROPERTIES_P_MOD_VER "provider.module.version"
#define ORIENTSEC_GRPC_PROPERTIES_P_ANYHOST "provider.anyhost"
#define ORIENTSEC_GRPC_PROPERTIES_P_DUBBO "provider.dubbo"
#define ORIENTSEC_GRPC_PROPERTIES_P_ACCESS_PROTECTED "provider.access.protected"
//for active/standby server
#define ORIENTSEC_GRPC_PROPERTIES_P_MASTER "provider.master"  
// for service group and grading
#define ORIENTSEC_GRPC_PROPERTIES_P_GROUP "provider.group"
#define ORIENTSEC_GRPC_PROPERTIES_P_GROUP_DEFAULT ""

#define ORIENTSEC_GRPC_PROPERTIES_C_CHECK "consumer.check"
#define ORIENTSEC_GRPC_PROPERTIES_C_DEFAULT_TIMEOUT "consumer.default.timeout"
#define ORIENTSEC_GRPC_PROPERTIES_C_SIDE "consumer.side"
#define ORIENTSEC_GRPC_PROPERTIES_C_APP_VERSION "consumer.application.version"
#define ORIENTSEC_GRPC_PROPERTIES_C_DEFUALT_REF_FILTER \
  "consumer.default.reference.filter"
#define ORIENTSEC_GRPC_PROPERTIES_C_LOGGER "consumer.logger"
#define ORIENTSEC_GRPC_PROPERTIES_C_OWNER "consumer.owner"
#define ORIENTSEC_GRPC_PROPERTIES_C_ORG "consumer.organization"
#define ORIENTSEC_GRPC_PROPERTIES_C_DEFAULT_RETRIES "consumer.default.retries"
#define ORIENTSEC_GRPC_PROPERTIES_C_LB_MODE "consumer.loadbalance.mode"
#define ORIENTSEC_GRPC_PROPERTIES_C_DEFAULT_LB "consumer.default.loadbalance"
#define ORIENTSEC_GRPC_PROPERTIES_C_DFAULT_CLUSTER "consumer.default.cluster"
// add by liumin 增加掉线指数级重连参数
#define ORIENTSEC_GRPC_PROPERTIES_C_BACKOFF_MAXSECOND \
  "consumer.backoff.maxsecond"
// add by yang
#define ORIENTSEC_GRPC_PROPERTIES_C_CONSISTENT_HASH_ARG \
  "consumer.consistent.hash.arguments"
// # 可选, 类型integer, 缺省值5, 
// # 说明：连续多少次请求出错，自动切换到提供相同服务的新服务器
#define ORIENTSEC_GRPC_PROPERTIES_C_CONSUMER_SWITH_THRES \
  "consumer.switchover.threshold"
// # 可选，类型int，说明：服务端节点调用失败被移除请求列表后，经过多长时间将该服务端节点重新添加回服务端候选列表
// # 单位毫秒，默认值600000，即600秒，即10分钟
#define ORIENTSEC_GRPC_PROPERTIES_C_CONSUMER_SERVICE_RECOVERY \
  "consumer.service.recoveryMilliseconds"

//# 可选，默认int，缺省值10，单位分钟，说明：负载均衡模式为connection时，设置连接自动切换的时间
#define ORIENTSEC_GRPC_PROPERTIES_C_CONSUMER_CONNECTION_SWITCH_TIME \
"consumer.loadbalance.connection.switchTime"


#ifdef __cplusplus
}
#endif

#endif  // !ORIENTSEC_GRPC_PROPERTIES_CONSTANTS_H
