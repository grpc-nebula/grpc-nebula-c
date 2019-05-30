# 微服务治理框架(C++版)开发手册
---
## grpc简介
grpc是一个多语言、高性能、开源的通用远程过程调用(RPC)框架。

- 来自于Google的开源项目，2016年8月19日发布了成熟的1.0.0版本
- 基于HTTP/2技术
- grpc支持Java, C，C++, Python等多种常用的编程语言，客户端和服务端可以采用不同的编程语言来实现。
- 数据序列化使用[Protocol Buffers](https://github.com/google/protobuf)
- grpc社区非常活跃，版本迭代也比较快速，2019年4月11日发布了1.20.0版本。

官网地址： [http://www.grpc.io/](http://www.grpc.io/)

源码地址(原生)： [https://github.com/grpc](https://github.com/grpc)

## 功能列表  
### 公共
- 基于Zookeeper的注册中心
- 支持Zookeeper开启ACL

### Provider端(服务提供者)

1. 读取本地的配置文件信息
2. 服务启动时，自动向zk注册Provider信息
3. 服务关闭时，自动注销zk中的Provider信息
4. 服务流量控制：请求数控制，连接数控制。  
   并发请求数使用default.requests参数进行配置，连接数使用default.connections参数进行配置。
5. 监听zk中的配置信息节点configurators，能够根据default.requests/default.connections参数的配置信息动态改变服务流控的行为。
6. 当Provider端被调用时，根据配置文件中的deprecated参数（或者zk中的deprecated参数配置信息），判定是否打印告警日志、提示服务已经有新版本上线。


### Consumer端(服务消费者)

1. 读取本地的配置文件信息
2. Consumer建立服务通道(Channel)时，自动向zk注册Consumer信息
3. Consumer关闭服务通道(Channel)时，自动注销zk中的Consumer信息
4. 从zk里获取相应服务的Provider列表，并能监听到zk中的Provider列表的更新
5. 服务调度——黑白名单  
   从候选Provider列表中剔除处于黑名单中Provider，或者剔除对当前客户端不开放访问权限的Provider。  
   每个服务端可以配置access.protected参数，控制默认情况下是否开放给客户端。即某些服务端默认禁止访问，另外一些服务端默认开放访问。     
6. 服务调度——负载均衡   
   支持四种负载均衡算法：round_robin(轮询), pick_first(任选一个), weight_round_robin(有权重的轮询),consistent_hash（一致性哈希）。 
7. 负载均衡的模式  
   支持两种模式：一种是“请求负载均衡”，另一种是“连接负载均衡”。  
   “请求负载均衡”指的是每次调用服务端都调用负载均衡算法选择一台服务器。“连接负载均衡”指的是，创建通道(Channel)后第一次调用选择服务器之后，一直复用与之前已选定的服务器建立的连接。  
    默认情况下设置为“连接负载均衡”。通过参数loadbalance.mode来进行控制。
8. 容错策略   
(1)调用某个服务提供者，如果连续N次请求出错，自动切换到提供相同服务的新服务器。（N这个数值支持配置）    
(2)调用某个服务提供者，如果连续N次请求出错，如果此时没有其他服务提供者，增加一个惩罚连接时间60s，而不是永远不连了（N与60s均可配）。
9. 根据配置文件中provider.weight和zk中configurators/weight参数更新各provider的权重信息，负载均衡时会使用到该信息。
10. 当Consumer端创建channel后，根据配置文件中consumers.loadbalance和zk中configurators/loadbalance更新负载均衡算法类型。以zk中的信息为准。
11. 当Consumer端创建channel后，根据配置文件中的deprecated参数（或者zk中的deprecated参数配置信息），判定是否打印告警日志、提示服务已经有新版本上线。 
12. 客户端的流量控制  
   增加对客户端请求数控制的功能，可以控制客户端对某个服务的请求次数。  
   对客户端增加请求数控制的功能，通过在注册中心上动态增加参数配置。（不能通过在本地文件配置，因为一个客户端可能需要调用多个服务端）  
   为了和服务端区分，将参数的名称定为consumer.default.requests。程序中新增对注册中心上的consumer.default.requests参数进行监听，缺省情况下，不限制客户端对某个服务的请求次数。

## 使用示例
下面以一个完整的客户端和服务端代码作为示例，具体介绍如何使用C++版本微服务治理框架。
### 1. 部署开发包和依赖库
- 1）部署目录如下：  
进入release包下，找到redhat平台发布包，redhat目录下共分四个目录，config、demo、include、libs，其中config为程序启动时所用的配置文件，demo里为应用程序，include为运行所必须的头文件，libs为所必须的库文件。

		redhat/
		├── config  
		├── demo  
		├── include  
		└── libs  

- 2）开发目录  
  服务端和客户端目录： 
 
		redhat/demo/  
		├── demo-async-client  
		├── demo-sync-client
		├── demo-async-server  
		└── demo-sync-server 
- 3）服务端和客户端主要文件


		redhat/demo/demo-sync-client  
		├── helloworld.grpc.pb.h  
		├── helloworld.pb.h  
		├── demo-sync-client.cc  
    	├── helloworld.grpc.pb.cc  
		└── helloworld.pb.h  

		redhat/demo/demo-sync-server  
		├── helloworld.grpc.pb.h  
		├── helloworld.pb.h  
		├── demo-sync-server.cc  
    	├── helloworld.grpc.pb.cc  
		└── helloworld.pb.h  

### 2. 新增dfzq-grpc-config.properties配置文件
在config 目录下增加配置文件

应用启动时，按以下顺序依次找配置文件；如果没找到，则顺延到下一条：  
(1) c/c++ 用户可以通过环境变量 **DFZQ_GRPC_CONFIG=/xxx/xxx** 配置grpc配置文件所在的目录的绝对路径。  
(2) 从启动目录下的**config**中查找grpc配置文件  
(3) 从启动目录下查找grpc配置文件  


配置文件中设置了当前应用的名称、服务的版本号,注册中心zookeeper主机IP和ACL的相关信息。

例子：./config/dfzq-grpc-config.properties
	
	# ------------ begin of common config ------------
	
	# 必填,类型string,说明:当前应用名称
	common.application=grpc-test-application
	
	# 必填,类型string,说明:当前项目名
	common.project=grpc-test-project
	
	# 必填,类型string,说明:项目负责人,员工工号,多个工号之间使用英文逗号
	common.owner=1023,1234

	# 可选,类型string,说明:服务注册根路径,默认值/Application/grpc
	common.root=/Application/grpc
	
	# 可选,类型string,说明:服务注册使用的IP地址
	# 如果不配置该参数值，当前服务器的IP地址为"非127.0.0.1的第一个网卡的IP地址"
	# 使用场合:一台服务器安装有多个网卡,如果需要指定不是第一个网卡的IP地址为服务注册的IP地址
	#common.localhost.ip=xxx.xxx.xxx.xxx
	
	# ------------ end of common config ------------
	
	# ------------ begin of provider config ------------
	
	# 必填,类型string,说明:服务的版本信息，一般表示服务接口的版本号
	provider.version=1.0.0

	# 必填,类型String,固定值provider,说明:provider表示服务提供端，consumer表示服务消费端
	provider.side=provider
	# 可选,类型boolean,缺省值false,说明:服务是否过时，如果为true则应用该服务时日志error告警
	provider.deprecated=false
	# 可选,类型int,缺省值100,说明:服务provider权重，是服务provider的容量，在负载均衡基于权重的选择算法中用到
	provider.weight=100

	# ----------------------------------------
	# 可选,类型int,缺省值20,说明:服务提供端可处理的最大连接数，即同一时刻最多有多少个消费端与当前服务端建立连接
	# 如果不限制连接数，将这个值配置为0
	# 对连接数的控制，无法控制到指定的服务，只能控制到指定的IP:port
	provider.default.connections=20

	# 可选,类型int,缺省值2000,说明:服务提供端可处理的最大并发请求数
	# 如果不限制并发请求数，将这个值配置为0
	# 备注：同一个连接发送多次请求
	provider.default.requests=2000

	# 可选, 类型boolean, 缺省值false, 说明:服务是否处于访问保护状态
	# 属性的可选值为false 、true ，分别表示不受保护、受保护，缺省值为false （不受保护）
	provider.access.protected=false

	# 可选,类型string,缺省值1.0.0,说明:gRPC 协议版本号
	provider.grpc=1.1.0

	# ------------ end of provider config ------------
	
	
	# ------------ begin of consumer config ------------
	
	# 必填,类型String,固定值consumer,说明:provider表示服务提供端，consumer表示服务消费端
	consumer.side=consumer

	# --------------------------
	# 可选,类型string,说明:服务提供者的版本号
	# 指定了服务提供者的版本号之后，程序会优先选择具有指定版本的服务；如果注册中心没有该版本的服务，则不限制版本重新选择服务提供者。
	# 使用场景为：注册中心上同一个服务多版本共存，并且服务的高版本与低版本不兼容，而当前应用由于特殊原因只能调用低版本的服务
	# 如果当前应用只调用一个服务，属性值直接配置版本号，例如1.0.0
	# 如果当前应用需要调用多个服务，属性值按照冒号逗号的方式分隔，例如com.dfzq.examples.Greeter:1.0.0,com.dfzq.examples.Hello:1.2.1
	# 如果当前应用需要调用多个服务，建议在服务治理平台维护该属性，只有一个版本的服务可以不维护
	consumer.service.version=com.dfzq.grpc.helloworld.Hello:1.0.1,com.dfzq.grpc.helloworld.Bye:1.0.2,com.dfzq.grpc.helloworld.Echo:1.0.3

	# 可选,类型string,缺省值 connection ,说明：负载均衡模式
	# 可选值为 connection 和 request,分别表示“连接负载均衡”、“请求负载均衡”
	# “连接负载均衡”适用于大部分业务场景，服务端和客户端消耗的资源较小。
	# “请求负载均衡”适用于服务端业务逻辑复杂、并有多台服务器提供相同服务的场景。
	consumer.loadbalance.mode=request
	#consumer.loadbalance.mode=connection
	# 可选,类型string,缺省值round_robin,说明:调度策略，可选范围： pick_first 、 round_robin 、 weight_round_robin , consistent_hash
	consumer.default.loadbalance=pick_first
	# 可选,类型string,负载均衡策略选择是consistent_hash(一致性Hash)，配置进行hash运算的参数名称的列表
	# 多个参数之间使用英文逗号分隔，例如 id,name
	# 如果负载均衡策略选择是consistent_hash，但是该参数未配置参数值、或者参数值列表不正确，则随机取一个值来做散列运算

	#consumer.consistent.hash.arguments=name
	# 可选,类型integer,缺省值5,说明：连续多少次请求出错，自动切换到提供相同服务的新服务器
	consumer.switchover.threshold=5

	# 可选,类型为long,单位为秒,缺省值为60,说明：服务提供者不可用时的惩罚时间，即多次请求出错的服务提供者一段时间内不再去请求
	# 属性值大于或等于0，等于0表示没有惩罚时间，如果客户端只剩下一个服务提供者，即使服务提供者不可用，也不做剔除操作。
	consumer.unavailable.provider.punish.time=60

	# 可选,类型String,默认值consumers,说明:所属范畴
	consumer.category=consumers

	# 可选,类型String,固定值consumer,说明:provider表示服务提供端，consumer表示服务消费端
	consumer.side=consumer

	# 可选,类型int,缺省值0,0表示不进行重试,说明:服务调用出错后自动重试次数
	consumer.default.retries=2

	# 指数退避协议https://github.com/grpc/grpc/blob/master/doc/connection-backoff.md
	# 可选,类型long,缺省值120,单位秒,说明:grpc断线重连指数退避协议"失败重试等待时间上限"参数
	consumer.backoff.max=120
	
	# ------------ end of consumer config ------------
	
	# ------------ begin of zookeeper config ------------
	
	# zookeeper主机列表
	zookeeper.host.server=127.0.0.1:2181

	# 可选,类型int,缺省值86400000,单位毫秒,即缺省值为1天,说明:zk断线重连最长时间
	zookeeper.retry.time=86400000

	#连接超时时间
	zookeeper.connectiontimeout=5000
	# 可选,类型string,访问控制用户名
	zookeeper.acl.username=admin

	# 可选,类型string,访问控制密码
	# 这里的密码配置的是密文，使用DES算法进行加密
	
	zookeeper.acl.password=9883c580ae8226f0dd8200620e4bc899
	
	# ------------ end of zookeeper config ------------

    
### 3. 客户端调用的创新
- 使用原生grpc-c的写法

		GreeterClientImpl  client(grpc::CreateChannel("IP:port", 
		grpc::InsecureChannelCredentials()));

		GreeterClientImpl  client(grpc::CreateChannel(dfzq_server_address, 
		grpc::InsecureChannelCredentials()));
	
- 使用微服务治理框架(C++版)的写法

		GreeterClientImpl client(grpc::CreateChannel("zookeeper:///com.dfzq.grpc.helloworld.Greeter", grpc::InsecureChannelCredentials()));

		GreeterClientImpl  client(grpc::CreateChannel(dfzq_server_address, 
		grpc::InsecureChannelCredentials()));


- 区别在于使用字符串“ **zookeeper:///** + **服务名称** ”的方式指定服务。这样可以根据服务名称去从多台提供相同服务的服务器中动态选择一台服务器，即达到服务器负载均衡的目的。  

 
### 4. Protocol Buffers文件

Protocol Buffers文件用来定义服务名称、方法、入参、出参。项目中可以通过protoc
插件，根据Protocol Buffers文件生成c++代码。


- 示例： helloworld.proto

		syntax = "proto3";
		option java_multiple_files = true;
		option java_package = "com.dfzq.grpc.helloworld";
		option java_outer_classname = "HelloWorldProto";
		option objc_class_prefix = "HLW";
		package com.dfzq.grpc.helloworld;
		// The greeting service definition.
		service Greeter {
		// Sends a greeting
		rpc SayHello (HelloRequest) returns (HelloReply) {}
		}
		// The request message containing the user's name.
		message HelloRequest {
		string name = 1;
		}
		// The response message containing the greetings
		message HelloReply {
		string message = 1;
		}
- PB生成
 - 将PB生成写在CMakeLists.txt中自动生成，运行命令如下：  
 
			protoc.exe -I=. --grpc_out=. --plugin=protoc-gen-grpc=.\grpc_cpp_plugin.exe helloworld.proto
			protoc.exe -I=. --cpp_out=. helloworld.proto
 
 - CMakeLists.txt片段：

			#设置proto工具目录
			get_filename_component(proto_tool "${PROJECT_SOURCE_DIR}/../../libs/protobuf" ABSOLUTE)
			#生成PB
			execute_process(COMMAND ${proto_tool}/protoc -I ${PROJECT_SOURCE_DIR} --cpp_out=${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/helloworld.proto)
			execute_process(COMMAND ${proto_tool}/protoc -I ${PROJECT_SOURCE_DIR} --grpc_out=${PROJECT_SOURCE_DIR} --plugin=protoc-gen-grpc=${proto_tool}/grpc_cpp_plugin ${PROJECT_SOURCE_DIR}/helloworld.proto)                                  

 - 生成四个文件简要说明：  

			helloworld.grpc.pb.cc         //包含服务类的实现
			helloworld.grpc.pb.h          //声明你生成的服务类的头文件
			helloworld.pb.cc              //包含消息类的实现
			helloworld.pb.h               //声明生成的消息类的头文件

### 5. 服务实现

服务实现类，是对Protocol Buffers文件定义的方法sayHello进行具体的业务实现。

例子： GreeterClientImpl.cpp

	GreeterClientImpl::GreeterClientImpl(std::shared_ptr<grpc::Channel> channel) :stub_(Greeter::NewStub(channel))
	{
	}
	GreeterClientImpl::~GreeterClientImpl()
	{
	 	std::cout << " GreeterClientImpl released ...... " <<  std::endl;
	}
	std::string GreeterClientImpl::SayHello(const std::string &user)
	{
		HelloRequest request;
		HelloReply reply;

		request.set_name(user);
		grpc::ClientContext context;
		grpc::Status status = stub_->SayHello(&context, request, &reply);
		if (status.ok())
		{
			return reply.message();
		}
		else
		{
			std::cout << status.error_code() << ":" << status.error_message() << std::endl;
			return "rpc failed";
		}
	}


### 6. 服务提供者(服务端)

服务提供者，启动一个Server对象，等待客户端的连接。

服务提供者可以提供一个服务，也可以提供多个服务（addService方法）。

例子：GreeterServer

	using grpc::Server;
	using grpc::ServerBuilder;
	void RunServer()
	{
		std::string server_addr("0.0.0.0:55556");
		GreeterImpl greeterService;
		EchoImpl echoService;
		ServerBuilder builder;
		builder.AddListeningPort(server_addr, grpc::InsecureServerCredentials());
		builder.RegisterService(&greeterService);
		std::unique_ptr<Server> server(builder.BuildAndStart());
		std::cout << "Server1 listen on " << server_addr << std::endl;
		server->Wait();
	}
	int main(int argc, char *argv[])
	{
		RunServer();
		return 0;
	}
### 7. 服务消费者（客户端）

服务消费者，通过调用**GreeterClientImpl client(grpc::CreateChannel(,));**方法建立Channel，其中target的组成组成规则为“ **zookeeper:///** + **服务名称** ”。框架会到注册中心zookeeper上去寻找对应服务名称的服务提供者。

例子：client.GreeterClient

	int dfzq_grpc_test_multhread_call_exe_count = 100000;
	void dfzq_grpc_test_multhread_call_sayhello_01(void* arg) {
		helloClientImpl client(grpc::CreateChannel("zookeeper:///com.dfzq.grpc.helloworld.Hello", grpc::InsecureChannelCredentials()));
		gpr_timespec tsp = gpr_now(GPR_CLOCK_REALTIME);
		int64_t timespend = tsp.tv_sec;
		for (int i = 0; i < dfzq_grpc_test_multhread_call_exe_count; i++) {
			std::string reply = client.SayHello(user);
			dfzq_grpc_sleep_mills(100);
		}
		gpr_thd_id currentid = gpr_thd_currentid();
		timespend = gpr_now(GPR_CLOCK_REALTIME).tv_sec - timespend;
	}
	int main()
	{
		gpr_thd_id thd_id;
		//开启n线程并发调用
		for (int i = 0;i < 1;i++) {
			gpr_thd_new(&thd_id, dfzq_grpc_test_multhread_call_sayhello_01, NULL, NULL);
		}
		getchar();
		return 0;
	}


### 8. 编译部署
#### 代码编译
1)	windows:  
使用VisualStudio2017进行导入编译，一键式编译  
2)	Linux：   
以客户端代码为例   
代码目录：`/home/appadmin/redhat/demo/demo-sync-client`  

编写CMakeList.txt：  

	# CMake 最低版本号要求
	cmake_minimum_required (VERSION 2.8)

	# 项目信息
	project (demo-sync-client CXX)

	# 版本信息
	set (Demo_VERSION_MAJOR 1)
	set (Demo_VERSION_MINOR 0)

	set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})
	#设置proto工具目录
	get_filename_component(proto_tool "${PROJECT_SOURCE_DIR}/../../libs/protobuf" ABSOLUTE)
	#生成PB
	execute_process(COMMAND ${proto_tool}/protoc -I ${PROJECT_SOURCE_DIR} --cpp_out=${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/helloworld.proto)
	execute_process(COMMAND ${proto_tool}/protoc -I ${PROJECT_SOURCE_DIR} --grpc_out=${PROJECT_SOURCE_DIR}
	--plugin=protoc-gen-grpc=${proto_tool}/grpc_cpp_plugin ${PROJECT_SOURCE_DIR}/helloworld.proto)

	# 编译参数
	set (CMAKE_BUILE_TYPE RELEASE) 
	add_definitions(-DRELEASE)

	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC -pthread")

	# 查找当前目录下的所有源文件
	# 并将名称保存到 DIR_LIB_SRCS 变量
	aux_source_directory(. DIR_LIB_SRCS)
	#aux_source_directory(model DIR_LIB_SRCS)


	# 链接库目录
	link_directories ("${PROJECT_SOURCE_DIR}/../../libs/opt"
                  "${PROJECT_SOURCE_DIR}/../../libs/lib"
                  "${PROJECT_SOURCE_DIR}/../../libs/protobuf"
                  "${PROJECT_SOURCE_DIR}/../../libs/zookeeper")

	# 添加包含目录
	include_directories("${PROJECT_SOURCE_DIR}"
	            "${PROJECT_SOURCE_DIR}/../../include/orientsec_grpc"
                "${PROJECT_SOURCE_DIR}/../../include/include"
                "${PROJECT_SOURCE_DIR}/../../include/third_party/protobuf/src/"
	            "${PROJECT_SOURCE_DIR}/../../include/third_party/protobuf"
	            "${PROJECT_SOURCE_DIR}/model")

	# 生成链接库
	add_executable (demo-sync-client ${DIR_LIB_SRCS})

	# 添加链接库
	target_link_libraries(demo-sync-client libgpr.so)
	target_link_libraries(demo-sync-client libgrpc.so)
	target_link_libraries(demo-sync-client libgrpc++.so)
	target_link_libraries(demo-sync-client libprotobuf.a)
	target_link_libraries(demo-sync-client libssl.so)
	target_link_libraries(demo-sync-client liborientsec_common.a)
	target_link_libraries(demo-sync-client liborientsec_provider.a)
	target_link_libraries(demo-sync-client liborientsec_consumer.a)
	target_link_libraries(demo-sync-client liborientsec_registry.a)
	target_link_libraries(demo-sync-client libzookeeper_mt.so)
 
编译方法：  
1）在cmake目录中运行cmake .. 生成Makefile  
2）运行make,在 /home/appadmin/redhat/demo/demo-sync-client 目录下生成demo-sync-client.

	[root@zabbixserver demo-sync-client]# cd cmake
	[root@zabbixserver cmake]# cmake ..
	[root@zabbixserver cmake]# make
	[root@zabbixserver cmake]# cd ..
	[root@zabbixserver demo-sync-client]#./demo-sync-client

服务端方法同上。

### 9.运行程序：

  - 启动Zookeeper
  - 将Zookeeper IP地址和端口号修改到配置文件中  
zookeeper.host.server=XX:YY(XX为IP地址，YY为端口号，中间加“:”)
  - 启动服务端:    
`./demo-sync-server`
  - 启动客户端：  
`./demo-sync-client`

### 10. 测试
服务调用成功

服务端提示：
  
	[root@zabbixserver demo-sync-server]# ./demo-sync-server 

	Config path : /home/appadmin/redhat/demo/demo-sync-server/config/dfzq-grpc-config.properties
	Server listening on 0.0.0.0:50066

 	Service is Called ... world9

	invoke_count = 1

 	Service is Called ... world8

	invoke_count = 2 
	….


客户端提示：  

	[root@zabbixserver demo-sync-client]# ./demo-sync-client 

	Config path : /home/appadmin/redhat/demo/demo-sync-client/config/dfzq-grpc-config.properties
	Greeter received: Hello world9
	Greeter received: Hello world8
	Greeter received: Hello world7
	.
	.
	.
	Greeter received: Hello world0





