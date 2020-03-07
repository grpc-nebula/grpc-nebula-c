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
 *    Maintainer : Jianbin Yang
 *    2017/05/15
 *    version 0.0.9
 *    常量定义, zookeeper registry key
 */

#pragma once
#ifndef ORIENTSEC_CONTANTS_H
#define ORIENTSEC_CONTANTS_H

#ifdef __cplusplus
extern "C" {
#endif

#define ORIENTSEC_GRPC_REGISTRY_ROOT  "/Application/grpc" //当前根目录变为common监控
#define ORIENTSEC_GRPC_REGISTRY_SEPARATOR   "/"

#define ORIENTSEC_GRPC_REGISTRY_KEY_VERSION   "version"
#define ORIENTSEC_GRPC_REGISTRY_KEY_DEFAULT_TIMEOUT   "default.timeout"
#define ORIENTSEC_GRPC_REGISTRY_KEY_DEFAULT_RETIES   "default.reties"
#define ORIENTSEC_GRPC_REGISTRY_KEY_DEFAULT_CONNECTIONS   "default.connections"
#define ORIENTSEC_GRPC_REGISTRY_KEY_DEFAULT_REQUESTS   "default.requests"
#define ORIENTSEC_GRPC_REGISTRY_KEY_ACCESS_PROTECTED   "access.protected"
#define ORIENTSEC_GRPC_REGISTRY_KEY_DEFAULT_LOADBALANCE   "default.loadbalance"
#define ORIENTSEC_GRPC_REGISTRY_KEY_DEFAULT_ASYNC   "default.async"
#define ORIENTSEC_GRPC_REGISTRY_KEY_TOKEN   "token"
#define ORIENTSEC_GRPC_REGISTRY_KEY_DEPRECATED   "deprecated"
#define ORIENTSEC_GRPC_REGISTRY_KEY_DYNAMIC   "dynamic"
#define ORIENTSEC_GRPC_REGISTRY_KEY_ACCESSLOG   "accesslog"
#define ORIENTSEC_GRPC_REGISTRY_KEY_OWNER   "owner"
#define ORIENTSEC_GRPC_REGISTRY_KEY_WEIGHT   "weight"
#define ORIENTSEC_GRPC_REGISTRY_KEY_DEFAULT_CLUSTER   "default.cluster"
#define ORIENTSEC_GRPC_REGISTRY_KEY_APPLICATION   "application"
#define ORIENTSEC_GRPC_REGISTRY_KEY_APPLICATION_VERSION   "application.version"
#define ORIENTSEC_GRPC_REGISTRY_KEY_ORGANIZATION   "organization"
#define ORIENTSEC_GRPC_REGISTRY_KEY_ENVIRONMENT   "environment"
#define ORIENTSEC_GRPC_REGISTRY_KEY_MODULE   "module"
#define ORIENTSEC_GRPC_REGISTRY_KEY_MODULE_VERSION   "module.version"
#define ORIENTSEC_GRPC_REGISTRY_KEY_ANYHOST   "anyhost"
#define ORIENTSEC_GRPC_REGISTRY_KEY_INTERFACE   "interface"
#define ORIENTSEC_GRPC_REGISTRY_KEY_METHODS   "methods"
#define ORIENTSEC_GRPC_REGISTRY_KEY_PID   "pid"
#define ORIENTSEC_GRPC_REGISTRY_KEY_SIDE   "side"
#define ORIENTSEC_GRPC_REGISTRY_KEY_TIMESTAMP   "timestamp"
#define ORIENTSEC_GRPC_REGISTRY_KEY_THREADID "tid"
#define ORIENTSEC_GRPC_REGISTRY_KEY_GRPC "grpc"
#define ORIENTSEC_GRPC_REGISTRY_KEY_DUBBO "dubbo"
#define ORIENTSEC_GRPC   "grpc"
// add for method level load balance
#define ORIENTSEC_GRPC_REGISTRY_KEY_METHOD  "method"
#define ORIENTSEC_GRPC_REGISTRY_KEY_LB_MODE "loadbalance.mode"
#define ORIENTSEC_GRPC_DEFAULT_METHOD_KEY  "i_am_wildcard"

// add by liumin
#define ORIENTSEC_GRPC_REGISTRY_KEY_PROJECT "project"
#define ORIENTSEC_GRPC_REGISTRY_KEY_COMM_OWNER "owner"

// add for provider.master key in zookeeper registry
#define ORIENTSEC_GRPC_REGISTRY_KEY_MASTER "master"

// add for provider service group key in zookeeper registry
#define ORIENTSEC_GRPC_REGISTRY_KEY_GROUP "group"
// add for consumer service group key in zookeeper registry
#define ORIENTSEC_GRPC_REGISTRY_KEY_INVOKE_GROUP "invoke.group"

#define ORIENTSEC_GRPC_REGISTRY_KEY_CHECK   "check"
#define ORIENTSEC_GRPC_REGISTRY_KEY_DEFAULT_REFERENCE_FILTER   "default.reference.filter"
#define ORIENTSEC_GRPC_REGISTRY_KEY_LOGGER   "logger"

#define ORIENTSEC_GRPC_REGISTRY_KEY_FORCE   "force"
#define ORIENTSEC_GRPC_REGISTRY_KEY_NAME   "name"
#define ORIENTSEC_GRPC_REGISTRY_KEY_ENABLED   "enabled"
#define ORIENTSEC_GRPC_REGISTRY_KEY_PRIORITY   "priority"
#define ORIENTSEC_GRPC_REGISTRY_KEY_ROUTER   "router"
#define ORIENTSEC_GRPC_REGISTRY_KEY_RULE   "rule"
#define ORIENTSEC_GRPC_REGISTRY_KEY_RUNTIME   "runtime"

#define ORIENTSEC_GRPC_CATEGORY_KEY   "category"
#define ORIENTSEC_GRPC_PROVIDERS_CATEGORY   "providers"
#define ORIENTSEC_GRPC_CONSUMERS_CATEGORY   "consumers"
#define ORIENTSEC_GRPC_ROUTERS_CATEGORY   "routers"
#define ORIENTSEC_GRPC_CONFIGURATORS_CATEGORY   "configurators"
#define ORIENTSEC_GRPC_DEFAULT_CATEGORY   "providers"

#define ORIENTSEC_GRPC_DEFAULT_GRPC_PROPERTIES   "grpc.properties"
#define ORIENTSEC_GRPC_DEFAULT_PROTOCOL   "grpc"
#define ORIENTSEC_GRPC_DEFAULT_WEIGHT   100

#define ORIENTSEC_GRPC_DEFAULT_KEY_PREFIX   "default."
#define ORIENTSEC_GRPC_DEFAULT_KEY   "default"
#define ORIENTSEC_GRPC_LOADBALANCE_KEY   "loadbalance"


#define ORIENTSEC_GRPC_ANYHOST_VALUE   "0.0.0.0"

#define ORIENTSEC_GRPC_LOCALHOST_KEY   "localhost"
#define ORIENTSEC_GRPC_LOCALHOST_VALUE   "127.0.0.1"

#define ORIENTSEC_GRPC_ANY_VALUE   "*"


#define ORIENTSEC_GRPC_PROVIDER_PROTOCOL   "provider"
#define ORIENTSEC_GRPC_CONSUMER_PROTOCOL   "consumer"
#define ORIENTSEC_GRPC_ROUTE_PROTOCOL   "route"
#define ORIENTSEC_GRPC_CONFIGURATOR_PROTOCOL   "override"
#define ORIENTSEC_GRPC_EMPTY_PROTOCOL   "empty"


//服务降级相关常量
#define ORIENTSEC_GRPC_MOCK_PROTOCOL   "mock"
#define ORIENTSEC_GRPC_RETURN_PREFIX   "return "
#define ORIENTSEC_GRPC_THROW_PREFIX   "throw"
#define ORIENTSEC_GRPC_FAIL_PREFIX   "fail:"
#define ORIENTSEC_GRPC_FORCE_PREFIX   "force:"
#define ORIENTSEC_GRPC_FORCE_KEY   "force"


#define DEFAULT_ZOOKEEPER_PORT   2181
#define ORIENTSEC_GRPC_REGISTRY_DEFAULT_PROTO_HEADER "zookeeper://"

#define HSTC_GRPC_URL_CHAR_EQ "="

#define HSTC_GRPC_URL_CHAR_MH ":"

#define HSTC_GRPC_URL_CHAR_XG "/"

#define HSTC_GRPC_URL_PROTOCOL_SUFFIX  "://"

#define ORIENTSEC_GRPC_ZK_HOSTS "zookeeper.host.server"
#define ORIENTSEC_GRPC_ZK_TIMEOUT "zookeeper.connectiontimeout"
#define ORIENTSEC_GRPC_ZK_RETRY_TIME "zookeeper.retry.time"

// 区分内外部服务
#define ORIENTSEC_GRPC_ZK_PRIVATE_HOSTS "zookeeper.private.host.server"
#define ORIENTSEC_GRPC_ZK_PRIVATE_ACL_USERNAME "zookeeper.private.acl.username"
#define ORIENTSEC_GRPC_ZK_PRIVATE_ACL_PASSWORD "zookeeper.private.acl.password"
#define ORIENTSEC_GRPC_ZK_PUBLIC_SERVICES "public.service.list"
#define ORIENTSEC_GRPC_ZK_PRIVATE_SERVICES "private.service.list"

//访问控制用户名和密码

#define ORIENTSEC_GRPC_ZK_ACL_USERNAME "zookeeper.acl.username"
#define ORIENTSEC_GRPC_ZK_ACL_PASSWORD "zookeeper.acl.password"

#define ORIENTSEC_GRPC_CONSUMER_DEFAULT_REQUEST "consumer.default.requests"

// 类型string, 说明:客户端调用服务版本号
#define ORIENTSEC_GRPC_CONSUMER_SERVICE_VERSION "service.version"

// 服务端服务公开性
#define ORIENTSEC_GRPC_REGISTRY_KEY_SERVICE_TYPE "service.type"

// grpc框架版本号
#define ORIENTSEC_GRPC_REGISTRY_KEY_FRAME_VERSION "grpc"

// provider real ip addr
#define ORIENTSEC_GRPC_REGISTRY_KEY_REAL_IP "real.ip"

// provider real port
#define ORIENTSEC_GRPC_REGISTRY_KEY_REAL_PORT "real.port"

#ifdef __cplusplus
}
#endif




#endif // !ORIENTSEC_CONTANTS_H
