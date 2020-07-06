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
 *    2017/06/12
 *    version 0.0.9
 *    定义配置文件内有的键
 */

#ifndef ORIENTSEC_GRPC_CONF_H
#define ORIENTSEC_GRPC_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

	// 配置信息分为五类：
	// 1. common config(公共配置)
	// 2. provider config(服务提供者需要填写)
	// 3. consumer config(服务消费者需要填写)
	// 4. kafka config(提供者、消费者都需要配置)
	// 5. zookeeper config(提供者、消费者都需要配置)

	// ------------ begin of common config ------------

	// 必填, 类型string, 说明:当前应用名称
#define ORIENTSEC_GRPC_CONF_COMMON_APPLICATION "common.application"

// 必填, 类型string, 说明:grpc版本号
#define ORIENTSEC_GRPC_CONF_COMMON_GRPC "common.grpc"

//add by liumin
#define ORIENTSEC_GRPC_CONF_COMMON_PROJECT "common.project"
#define ORIENTSEC_GRPC_CONF_COMMON_PROJECT_DEFAULT "grpc-test-app"

//add for common.owner property by jianbin
#define ORIENTSEC_GRPC_CONF_COMMON_OWNER "common.owner"
#define ORIENTSEC_GRPC_CONF_COMMON_OWNER_DEFAULT "A0000"

//可选, 类型string, 说明 : 服务注册根路径，默认值 / Application / grpc
#define ORIENTSEC_GRPC_CONF_COMMON_ROOT_DIRECTORY "common.root"
#define ORIENTSEC_GRPC_CONF_COMMON_ROOT_DIRECTORY_DEFAULT "/Application/grpc"
#define ORIENTSEC_GRPC_ZK_PRIVATE_ROOT "zookeeper.private.root"
// ------------ end of common config ------------



// ------------ begin of provider config ------------

// 必填, 类型string, 说明:服务的版本信息，一般表示服务接口的版本号
#define ORIENTSEC_GRPC_CONF_PROVIDER_VERSION "provider.version"

// 必填, 类型String, 固定值provider, 说明:provider表示服务提供端，consumer表示服务消费端
#define ORIENTSEC_GRPC_CONF_PROVIDER_SIDE "provider.side"

#define ORIENTSEC_GRPC_CONF_PROVIDER_SIDE_DEFAULT "provider"

// ----------------------------------------

// 可选, 类型string, 说明:当前模块名称
#define ORIENTSEC_GRPC_CONF_PROVIDER_MODULE "provider.module"

// 可选, 类型int, 缺省值1000, 说明:远程服务调用超时时间（毫秒）
#define ORIENTSEC_GRPC_CONF_PROVIDER_DEFAULT_TIMEOUT "provider.default.timeout"

// 可选, 类型int, 缺省值2, 说明:远程服务调用重试次数，不包括第一次调用，不需要重试则设置为0
#define ORIENTSEC_GRPC_CONF_PROVIDER_DEFAULT_RETIES "provider.default.reties"

// 可选, 类型string, 缺省值round_robin, 说明:负载均衡策略，可选值：pick_first、round_robin，weight_round_robin, 可扩展实现其他策略
#define ORIENTSEC_GRPC_CONF_PROVIDER_DEFAULT_LOADBALANCE  "provider.default.loadbalance"

// 可选, 类型Boolean, 缺省值false, 说明:是否缺省异步执行，如果true则可直接忽略返回值，不阻塞线程
#define ORIENTSEC_GRPC_CONF_PROVIDER_DEFAULT_ASYNC "provider.default.async"

// 可选, 类型sting / Boolean, 缺省值false, 说明:令牌验证，为空表示不开启，为true则随机生成动态令牌，否则使用镜头令牌
#define ORIENTSEC_GRPC_CONF_PROVIDER_TOKEN "provider.token"

// 可选, 类型boolean, 缺省值false, 说明:服务是否过时，如果为true则应用该服务时日志error告警
#define ORIENTSEC_GRPC_CONF_PROVIDER_DEPRECATED "provider.dynamic"

// 可选, 类型boolean, 缺省值ture, 说明:服务是否动态注册，如果为false则注册后将显示为disable状态，
//     需要人工启用，且服务停止时需手动注销
#define ORIENTSEC_GRPC_CONF_PROVIDER_DYNAMIC "provider.dynamic"

#define ORIENTSEC_GRPC_CONF_PROVIDER_DYNAMIC_DEFAULT "true"

// 可选, 类型string / boolean, 缺省值false, 说明:设为true，将向logger中输出访问日志，也可填写访问
//     日志文件路径，直接把访问日志输出到指定文件
#define ORIENTSEC_GRPC_CONF_PROVIDER_ACCESSLOG "provider.accesslog"

// 可选, 类型string, 说明:服务负责人，填写负责人公司邮箱前缀
#define ORIENTSEC_GRPC_CONF_PROVIDER_OWNER "provider.owner"

// 可选, 类型string, 缺省值failover, 说明:集群方式，可选：failover / failfast / failback / forking
#define ORIENTSEC_GRPC_CONF_PROVIDER_DEFAULT_CLUSTER "provider.default.cluster"

#define ORIENTSEC_GRPC_CONF_PROVIDER_DEFAULT_CLUSTER_DEFAULT "failover"

// 可选, 类型string, 说明:应用版本号
#define ORIENTSEC_GRPC_CONF_PROVIDER_APPLICATION_VERSION "provider.application.version"

// 可选, 类型string, 说明:组织名（BU或部门）
#define ORIENTSEC_GRPC_CONF_PROVIDER_ORGANIZATION "provider.organization"

// 可选, 类型string, 说明:应用环境，如：develop / test / product
#define ORIENTSEC_GRPC_CONF_PROVIDER_ENVIRONMENT "provider.environment"

// 可选, 类型string, 说明:模块的版本号
#define ORIENTSEC_GRPC_CONF_PROVIDER_MODULE_VERSION "provider.module.version"

// 可选, 类型boolean, 缺省值false, 说明:表示是否为跨主机访问
#define ORIENTSEC_GRPC_CONF_PROVIDER_ANYHOST "provider.anyhost"

// 可选, 类型string, 说明:dubbo版本号, 缺省值为grpc的版本号
#define ORIENTSEC_GRPC_CONF_PROVIDER_DUBBO "provider.dubbo"

//本字段为预留字段（当前配置文件不包含本配置项）
#define ORIENTSEC_GRPC_CONF_PROVIDER_CATEGORY "provider.category"

#define ORIENTSEC_GRPC_CONF_PROVIDER_DEFAULT_REQUESTS_DEFAULT "0"

// 必填, 类型int, 固定值1000, 说明:远程服务调用超时时间(毫秒)
#define ORIENTSEC_GRPC_CONF_PROVIDER_DEFAULT_TIMEOUT "provider.default.timeout"

#define ORIENTSEC_GRPC_CONF_PROVIDER_DEFAULT_TIMEOUT_DEFAULT "1000"

// ------------ end of provider config ------------

// ------------ begin of consumer config ------------

// 必填, 类型boolean, 缺省值true, 说明:启动时检查提供者是否存在，true报错，false忽略
#define ORIENTSEC_GRPC_CONF_CONSUMER_CHECK "consumer.check"
//conf文件 consumer.check默认值
#define ORIENTSEC_GRPC_CONF_CONSUMER_CHECK_DEFAULT "true"


// 必填, 类型int, 固定值1000, 说明:远程服务调用超时时间(毫秒)
#define ORIENTSEC_GRPC_CONF_CONSUMER_DEFAULT_TIMEOUT "consumer.default.timeout"

#define ORIENTSEC_GRPC_CONF_CONSUMER_DEFAULT_TIMEOUT_DEFAULT "1000"

// 必填, 类型String, 默认值consumers, 说明:所属范畴
#define ORIENTSEC_GRPC_CONF_CONSUMER_CATEGORY "consumer.category"

// 必填, 类型String, 固定值consumer, 说明:provider表示服务提供端，consumer表示服务消费端
#define ORIENTSEC_GRPC_CONF_CONSUMER_SIDE "consumer.side"

#define ORIENTSEC_GRPC_CONF_CONSUMER_SIDE_DEFAULT "consumer"

// --------------------------
// 可选, 类型string, 说明:服务版本号,多个版本号用逗号分隔
#define ORIENTSEC_GRPC_CONF_CONSUMER_SERVICE_VERSION "consumer.service.version"

// 可选, 类型int, 缺省值2, 说明:远程服务调用重试次数
#define ORIENTSEC_GRPC_CONF_CONSUMER_DEFAULT_RETRIES "consumer.default.retries"
#define ORIENTSEC_GRPC_CONF_CONSUMER_DEFAULT_RETRIES_DEFAULT "2"

// 可选, 类型string, 缺省值round_robin, 说明:调度策略，可选范围：pick_first、round_robin、weight_round_robin
#define ORIENTSEC_GRPC_CONF_CONSUMER_DEFAULT_LOADBALANCE "consumer.default.loadbalance"
#define ORIENTSEC_GRPC_CONF_CONSUMER_DEFAULT_LOADBALANCE_DEFAULT "round_robin"

// 可选, 类型int, 缺省值0, 说明:每个服务对外最大连接数(暂时未用到)
#define ORIENTSEC_GRPC_CONF_CONSUMER_DEFAULT_CONNECTIONS "consumer.default.connections"
//&default.connections
#define ORIENTSEC_GRPC_CONF_CONSUMER_DEFAULT_CONNECTIONS_DEFAULT "0"

// 可选, 类型int, 缺省值0, 说明:每个服务对外最大请求数（非配置文件内配置项）
#define ORIENTSEC_GRPC_CONF_CONSUMER_DEFAULT_REQUESTS "consumer.default.requests"
#define ORIENTSEC_GRPC_CONF_CONSUMER_DEFAULT_REQUESTS_DEFAULT "0"

// 可选, 类型string, 缺省值failover, 说明:集群方式，可选：failover / failfast / failback / forking
#define ORIENTSEC_GRPC_CONF_CONSUMER_DEFAULT_CLUSTER "consumer.default.cluster"
//&default.cluster
#define ORIENTSEC_GRPC_CONF_CONSUMER_DEFAULT_CLUSTER_DEFAULT "failover"

// 可选, 类型boolean, 缺省值true, 说明：调用服务时动态选择服务提供者（根据负载均衡策略）
#define ORIENTSEC_GRPC_CONF_CONSUMER_KEEPALIVE "consumer.keepAlive"

//本键非conf文件配置项， 默认值为true
#define ORIENTSEC_GRPC_CONF_CONSUMER_DYNAMIC "consumer.dynamic"
//conf文件 consumer.dynamic默认值
#define	ORIENTSEC_GRPC_CONF_CONSUMER_DYNAMIC_DEFAULT "true"

//可选, 类型string, 缺省值为空，说明 : 表示当前客户端服务器属于某个分组
//使用场合：服务分组功能
#define ORIENTSEC_GRPC_CONF_CONSUMER_GROUP "consumer.group"
#define ORIENTSEC_GRPC_CONF_CONSUMER_INVOKE_GROUP "consumer.invoke.group"
// conf文件 consumer.invoke.group默认值
#define ORIENTSEC_GRPC_CONF_CONSUMER_GROUP_DEFAULT ""

// ------------ end of consumer config ------------

//consumer端缓存provider的最大数量默认值32
#define ORIENTSEC_GRPC_CACHE_PROVIDER_COUNT "consumer.cache.provider.count"

//可选, 类型string, 说明：该参数用来手动指定提供服务的服务器地址列表。
#define ORIENTSEC_GRPC_CONF_SERVICE_SERVER_LIST  "service.server.list"

#ifdef __cplusplus
}
#endif
#endif // !ORIENTSEC_GRPC_CONF_H
