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

// 可选, 类型string, 说明:服务分组信息，一般用来区分一个服务接口的多种实现（provider）
#define ORIENTSEC_GRPC_CONF_PROVIDER_GROUP "provider.group"

// 可选, 类型int, 缺省值1000, 说明:远程服务调用超时时间（毫秒）
#define ORIENTSEC_GRPC_CONF_PROVIDER_DEFAULT_TIMEOUT "provider.default.timeout"

// 可选, 类型int, 缺省值2, 说明:远程服务调用重试次数，不包括第一次调用，不需要重试则设置为0
#define ORIENTSEC_GRPC_CONF_PROVIDER_DEFAULT_RETIES "provider.default.reties"

//// 可选, 类型int, 缺省值0, 说明:对每个provider最大连接次数，默认最大连接数
//#define ORIENTSEC_GRPC_CONF_PROVIDER_DEFAULT_CONNECTIONS  "provider.default.connections"

#define ORIENTSEC_GRPC_CONF_PROVIDER_DEFAULT_CONNECTIONS_DEFAULT "0"

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

// 可选, 类型int, 缺省值100, 说明:服务provider权重，是服务provider的容量，在负载均衡基于权重的选择算法中用到
#define ORIENTSEC_GRPC_CONF_PROVIDER_WEIGHT  "provider.weight"

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


#define ORIENTSEC_GRPC_CONF_PROVIDER_DEFAULT_REQUESTS "provider.default.requests"

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

// 可选, 类型string, 说明:应用版本号
#define ORIENTSEC_GRPC_CONF_CONSUMER_APPLICATION_VERSION "consumer.application.version"

// 可选, 类型String, 说明:服务消费方远程调用过程拦截器名称，多个名称用逗号分隔
#define ORIENTSEC_GRPC_CONF_CONSUMER_DEFAULT_REFERENCE_FILTER "consumer.default.reference.filter"

// 可选, 类型String, 缺省值slf4j, 说明:日志输出方式，可选：slf4j, jcl, log4j, jdk
#define ORIENTSEC_GRPC_CONF_CONSUMER_LOGGER "consumer.logger"

// 可选, 类型String, 说明:调用服务负责人，用于服务治理，请填写负责人公司邮箱前缀
#define ORIENTSEC_GRPC_CONF_CONSUMER_OWNER " consumer.owner"

// 可选, 类型string, 说明:组织名（BU或部门）
#define ORIENTSEC_GRPC_CONF_CONSUMER_ORGANIZATION "consumer.organization"

// 可选, 类型int, 缺省值2, 说明:远程服务调用重试次数
#define ORIENTSEC_GRPC_CONF_CONSUMER_DEFAULT_RETRIES "consumer.default.retries"
#define ORIENTSEC_GRPC_CONF_CONSUMER_DEFAULT_RETRIES_DEFAULT "2"

// 可选, 类型string, 缺省值round_robin, 说明:调度策略，可选范围：pick_first、round_robin、weight_round_robin
#define ORIENTSEC_GRPC_CONF_CONSUMER_DEFAULT_LOADBALANCE "consumer.default.loadbalance"
#define ORIENTSEC_GRPC_CONF_CONSUMER_DEFAULT_LOADBALANCE_DEFAULT "round_robin"

//可选, 类型string, 负载均衡策略选择是consistent_hash(一致性Hash)，配置进行hash运算的参数名称的列表
//多个参数之间使用英文逗号分隔，例如 id, name
//如果负载均衡策略选择是consistent_hash，但是该参数未配置参数值、或者参数值列表不正确，则随机取一个值来做散列运算
#define ORIENTSEC_GRPC_CONF_CONSUMER_CONSISTENT_HASH_ARGUMENTS consumer.consistent.hash.arguments


// 可选, 类型int, 缺省值0, 说明:每个服务对外最大连接数(暂时未用到)
#define ORIENTSEC_GRPC_CONF_CONSUMER_DEFAULT_CONNECTIONS "consumer.default.connections"
//&default.connections
#define ORIENTSEC_GRPC_CONF_CONSUMER_DEFAULT_CONNECTIONS_DEFAULT "0"

// 可选, 类型int, 缺省值0, 说明:每个服务对外最大请求数（非配置文件内配置项）
#define ORIENTSEC_GRPC_CONF_CONSUMER_DEFAULT_REQUESTS "consumer.default.requests"
#define ORIENTSEC_GRPC_CONF_CONSUMER_DEFAULT_REQUESTS_DEFAULT "0"

//add by liumin
#define ORIENTSEC_GRPC_CONF_DEFAULT_PROJECT "project"

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


//可选, 类型integer, 缺省值5, 说明：连续多少次请求出错，自动切换到提供相同服务的新服务器
#define ORIENTSEC_GRPC_CONF_CONSUMER_SWITCHOVER_THRESHOLD "consumer.switchover.threshold"

//可选, 类型为long, 单位为秒, 缺省值为60, 说明：服务提供者不可用时的惩罚时间，即多次请求出错的服务提供者一段时间内不再去请求
//属性值大于或等于0，等于0表示没有惩罚时间，如果客户端只剩下一个服务提供者，即使服务提供者不可用，也不做剔除操作。
#define ORIENTSEC_GRPC_CONF_CONSUMER_UNAVAILABLE_PROVIDER_PUNISH_TIME "consumer.unavailable.provider.punish.time"
// ------------ end of consumer config ------------

// ------------ begin of kafka.producer config ------------
// 转发服务跟踪信息给kafka的服务提供者的IP地址和端口
// 服务跟踪信息先发送给KafkaSenderServer(一个基于原生grpc的服务)，然后由KafkaSenderServer转发给kafka
// 多个服务提供者之间使用英文逗号分隔，IP地址和端口使用英文冒号分隔
// kafka.sender.servers = 192.168.0.1:50080, 192.168.0.2 : 50080, 192.168.0.3 : 50080
#define ORIENTSEC_GRPC_CONF_KAFKA_SENDER_SERVERS "kafka.sender.servers"
// 必填, 不需要修改, 目前规定grpc使用CRM - SDGP - DubboRPC - Trace - 1.1.0这个topic
#define ORIENTSEC_GRPC_CONF_KAFKA_PRODUCER_APP_TOPIC "kafka.producer.app.topic"

// 属性的key值必填, 属性的value可选, 说明:这里配置的是kakfa服务期的地址
// 程序优先使用从zookeeper目录 / crm_sdgp / kafka_servers读取到的kafka地址
// 如果获取不到，则使用这里配置的地址
#define ORIENTSEC_GRPC_CONF_KAFKA_PRODUCER_BOOTSTRAP_SERVERS "kafka.producer.bootstrap.servers"

// 必填，Producer的数据确认阻塞设置;
//     0表示不管任何响应，只管发，发完了立即执行下个任务，这种方式最快
//     1表示只确保leader成功响应，接收到数据。
//     all表示确保leader及其所有follwer成功接收保存消息。
#define ORIENTSEC_GRPC_CONF_KAFKA_PRODUCER_ACKS "kafka.producer.acks"

// 必填, key的序列化类
#define ORIENTSEC_GRPC_CONF_KAFKA_PRODUCER_KEY_SERIALIZER "kafka.producer.key.serializer"

// 必填, value的序列化类
#define ORIENTSEC_GRPC_CONF_KAFKA_PRODUCER_VALUE_SERIALIZER "kafka.producer.value.serializer"

// --------------------------

// 可选, 每个RecordBatch可以缓存的最大字节数
#define ORIENTSEC_GRPC_CONF_KAFKA_PRODUCER_BATCH_SIZE "kafka.producer.batch.size"

// 可选, 所有RecordBatch的总共最大字节数
#define ORIENTSEC_GRPC_CONF_kafka_PRODUCER_BUFFER_MEMORY "kafka.producer.buffer.memory"

// 可选, 每个RecordBatch的最长延迟发送时间
#define ORIENTSEC_GRPC_CONF_KAFKA_PRODUCER_LINGER_MS "kafka.producer.linger.ms"

// 可选, 每个RecordBatch的最长阻塞时间
#define ORIENTSEC_GRPC_CONF_KAFKA_PRODUCER_MAX_BLOCK_MS "kafka.producer.max.block.ms"

// 可选, 消息发送失败重试的次数
#define ORIENTSEC_GRPC_CONF_KAFKA_PRODUCER_RETRIES "kafka.producer.retries"

// 可选, 失败补偿时间，每次失败重试的时间间隔，不可设置太短，避免第一条消息的响应还没返回，第二条消息又发出去了，造成逻辑错误
#define ORIENTSEC_GRPC_CONF_KAFKA_PRODUCER_RETRY_BACKOFF_MS "kafka.producer.retry.backoff.ms"

// 可选, 同一时间，producer到broker的一个连接上允许的最大未确认请求数，默认为5
#define ORIENTSEC_GRPC_CONF_KAFKA_PRODUCER_MAX_IN_FLIGHT_REQUESTS_PER_CONNECTION "kafka.producer.max.in.flight.requests.per.connection"

// 可选，采样频率，指的是每秒向kafka写服务链的数据条数（这里的一条理解为traceId相同的多个json串）；
// 默认值为空，表示所有服务链都发送到kafka；
// 如果采样率的数值不为空，并且数值大于0，根据指定的采样率采集数据
// 配置值可以为正整数、小数(例如0.5)、分数(例如3 / 1024)
#define ORIENTSEC_GRPC_CONF_KAFKA_SAMPLING_FREQUENCY "kafka.sampling.frequency"


//可选, 将服务跟踪信息发送给KafkaSenderServer的线程个数
//配置值为正整数，默认值为1
//# kafka.sender.number = 1
#define ORIENTSEC_GRPC_CONF_KAFKA_SENDER_NUMBER "kafka.sender.number"
// ------------ end of kafka.producer config ------------




// ------------ begin of zookeeper config ------------

// zookeeper主机列表
//zookeeper.host.server = 192.168.207.4:2181, 192.168.207.6 : 2181
#define ORIENTSEC_GRPC_CONF_ZOOKEEPER_HOST_SERVER "zookeeper.host.server"

//重试次数
#define ORIENTSEC_GRPC_CONF_ZOOKEEPER_RETRYNUM "zookeeper.retryNum"

//连接超时时间
#define ORIENTSEC_GRPC_CONF_ZOOKEEPER_CONNECTIONTIMEOUT "zookeeper.connectiontimeout"

// ------------ end of zookeeper config ------------


//--------------------begin switch----------------------
//标记是否需要生成服务跟踪信息，默认值为 true即需要生成服务跟踪信息
//生成服务跟踪信息，表示会服务跟踪信息写入链表，至于是否发送kafka由另外的开关进行控制
#define ORIENTSEC_GRPC_TRACE_GENTRACE_KEY "gentrace.enabled"

//可选, 类型boolean, 说明:是否启用向kafka写日志的功能
//# true表示启用，缺省值为true，开发阶段可以配置为false，上线阶段注释掉该配置项
#define ORIENTSEC_GRPC_TRACE_WRITEKAFKA_KEY "writekafka.enabled"

//---------------------end switch-----------------------

//consumer端缓存provider的最大数量默认值10
#define ORIENTSEC_GRPC_CACHE_PROVIDER_COUNT "consumer.cache.provider.count"

#ifdef __cplusplus
}
#endif
#endif // !ORIENTSEC_GRPC_CONF_H
