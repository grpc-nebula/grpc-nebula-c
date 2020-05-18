# 微服务治理框架(C++版)windows开发手册
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

微服务治理平台地址： [https://github.com/grpc-nebula/grpc-nebula-c](https://github.com/grpc-nebula/grpc-nebula-c)

## 功能列表  
### 公共
- 基于Zookeeper的注册中心
- 支持Zookeeper开启ACL
- 主备切换
- 服务分组、多机房支持
- 区分内部外部服务
- 注册中心容灾和降级  


### Provider端(服务提供者)

1. 读取本地的配置文件信息
2. 服务启动时，自动向zk注册Provider信息
3. 服务关闭时，自动注销zk中的Provider信息
4. 服务流量控制：请求数控制，连接数控制。  
   并发请求数使用default.requests参数进行配置，连接数使用default.connections参数进行配置。
5. 监听zk中的配置信息节点configurators，能够根据default.requests/default.connections参数的配置信息动态改变服务流控的行为。
6. 当Provider端被调用时，根据配置文件中的deprecated参数（或者zk中的deprecated参数配置信息），判定是否打印告警日志、提示服务已经有新版本上线。
7. provider端分组，将机房或者区域的相同服务的provider进行分组划分。
8. 服务端主备属性，将provider划分为主服务provider和备provider。
9. 服务端注册时，可以任意指定服务注册时使用的IP和端口


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
8. 服务容错 
  
（1）基于累计错误次数： 一段时间内(10分钟)，如果客户端调用某个服务端累计出错5次，客户端自动切换到提供相同服务的新服务端。

（2）熔断机制：在一个统计周期内，客户端调用某个服务端总请求次数达到设定的阈值，计算在该统计周期内错误百分比，如果超过设定的错误率阈值，打开熔断器，即将该服务端从客户端的备选服务端列表中删除。

（3）服务调用出错后自动重试：支持通过配置consumer.default.retries、consumer.default.retries[服务名]、consumer.default.retries[服务+方法名]开启这个功能。  
9. 根据配置文件中provider.weight和zk中configurators/weight参数更新各provider的权重信息，负载均衡时会使用到该信息。  
10. 当Consumer端创建channel后，根据配置文件中consumers.loadbalance和zk中configurators/loadbalance更新负载均衡算法类型。以zk中的信息为准。  
11. 当Consumer端创建channel后，根据配置文件中的deprecated参数（或者zk中的deprecated参数配置信息），判定是否打印告警日志、提示服务已经有新版本上线。   
12. 客户端的流量控制  
   增加对客户端请求数控制的功能，可以控制客户端对某个服务的请求次数。  
   对客户端增加请求数控制的功能，通过在注册中心上动态增加参数配置。（不能通过在本地文件配置，因为一个客户端可能需要调用多个服务端）  
   为了和服务端区分，将参数的名称定为consumer.default.requests。程序中新增对注册中心上的consumer.default.requests参数进行监听，缺省情况下，不限制客户端对某个服务的请求次数。    
13. 主备切换
多个服务端提供服务的时候，能够区分主服务器和备服务器。当主服务器可用时客户端只能调用主服务器，不能调用备服务器；当所有主服务器不可用时，客户端自动切换到备服务器进行服务调用；当主服务器恢复时，客户端自动切换到主服务器进行服务调用。  
14. 支持优先级的服务分组  
 - 场景1：服务分组。当服务集群非常大时，客户端不必与每个服务节点建立连接，通过对服务分组，一个客户端只与一个服务组连接。
 - 场景2：业务隔离。例如服务端部署在三台服务器上，分别提供给三种业务的客户端。该场景下，可以将三台服务器配置不同的分组，然后不同业务的客户端配置各自的服务端分组。这样即使其中一种业务的客户端调用频繁导致服务端响应时间边长，也不会影响其它两种业务的客户端。
 - 场景3：多机房支持。例如某证券公司在上海有两个机房A和B，在深圳有一个机房C，三个机房都对外提供服务；每个机房都划分为两个分组，即三个机房共有6个分组，分别为A1、A2、B1、B2、C1、C2。在上海地区的客户端要调用证券公司的服务时，可以优先调用A1、A2两个分组的服务端；如果A1、A2分组的服务端不可用，调用B1、B2两个分组的服务端；如果B1、B2分组的服务端也不可用，调用C1、C2两个分组的服务端。


## 使用示例
下面以一个完整的客户端和服务端代码作为示例，具体介绍如何使用C++版本微服务治理框架。
### 1. 部署开发包和依赖库
- 1.1 下载压缩包并解压  
使用解压工具解压msgf_win7.rar

- 1.2 部署目录如下：  
进入release下，找到windows平台发布包，msgf_win7目录下共分五个目录，config、demo、include、lib和proto，其中config为程序启动时所用的配置文件，demo里为应用程序，include为运行所必须的头文件，lib为所必须的库文件，proto1.17.2为proto工具包。

		msgf_win7/
		├── config  
		├── demo  
		├── include  
        ├── lib 
		└── proto  

- 1.3 开发目录  
  服务端和客户端目录： 
 
		msgf_win7/demo/example  
		├── demo-async-client  
		├── demo-sync-client
		├── demo-async-server  
		└── demo-sync-server 
- 1.4 服务端和客户端主要文件


		msgf_win7/demo/example/demo-sync-client  
        ├── demo-async-client.vcxproj 	         // 项目属性文件
        ├── demo-async-client.vcxproj.filters 	 // 项目过滤信息文件
        ├── config                               // 配置文件目录
        ├── helloworld.grpc.pb.h                 // 声明生成的服务类的头文件
		├── helloworld.grpc.pb.cc                // 包含服务类的实现
		├── demo-sync-client.cc                  // 消费端定义文件
    	├── helloworld.pb.cc                     // 包含消息类的实现
		└── helloworld.pb.h                      // 声明生成的消息类的头文件

		msgf_win7/demo/example/demo-sync-server
        ├── demo-async-server.vcxproj 	        // 项目属性文件
        ├── demo-async-server.vcxproj.filters 	// 项目过滤信息文件
        ├── config                              // 配置文件目录
		├── helloworld.grpc.pb.h                // 声明生成的服务类的头文件
		├── helloworld.pb.h                     // 声明生成的消息类的头文件
		├── demo-sync-server.cc                 // 服务端定义文件
    	├── helloworld.grpc.pb.cc               // 包含服务类的实现
		└── helloworld.pb.cc                    // 包含消息类的实现
- 1.5 proto 工具目录  
protobuf是google团队开发的用于高效存储和读取结构化数据的工具。用户可以使用protoc工具将用户定义的proto文件生成pb文件（包含.h和.cc文件）。  
protoc 的命令格式为 protoc.exe  [OPTION]  PROTO_FILES （最后是待编译的 proto文件）具体使用详见下一章节。

		msgf_win7/proto1.17.2  
        ├── gen.bat 	                        // 生成pb文件的批处理文件
        ├── grpc_cpp_plugin.exe                 // cpp 插件
        ├── helloworld.proto                    // 服务和协议定义文件 
		└── protoc.exe                          // Protoc 工具

### 2.生成Protocol Buffers文件

Protocol Buffers文件用来定义服务名称、方法、入参、出参。项目中可以通过protoc
插件，根据Protocol Buffers文件生成c++代码。

- 2.1 示例： helloworld.proto  

		syntax = "proto3";
		option java_multiple_files = true;
		option java_package = "io.grpc.examples.helloworld";
		option java_outer_classname = "HelloWorldProto";
		option objc_class_prefix = "HLW";
		package helloworld;
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

- 2.2 PB生成  
将PB生成命令写在gen.bat批处理文件中，运行.bat 文件自动生成相对应的PB文件，--cpp_out 为输出C++代码的目录，这里指定的是当前目录。随后我们指定了proto文件的位置helloworld.proto 。  
windows批处理文件内容如下：

		protoc.exe-I=.--grpc_out=.--plugin=protoc-gen-grpc=.\grpc_cpp_plugin.exe helloworld.proto  
		protoc.exe -I=. --cpp_out=. helloworld.proto

 生成四个文件简要说明：

 - helloworld.grpc.pb.cc            //包含服务类的实现
 - helloworld.grpc.pb.h             //声明生成的服务类的头文件
 - helloworld.pb.cc                 //包含消息类的实现
 - helloworld.pb.h                  //声明生成的消息类的头文件

### 3. 配置文件
  
配置文件中设置了当前应用的名称、服务的版本号,注册中心zookeeper主机IP和ACL的相关信息，配置文件名称是固定的，修改名称后会导致微服务治理框架读取不到配置文件。
配置文件在加载过程中有一定的目录搜索顺序，默认放在应用程序启动目录下config目录中。

复杂情况下配置如下：
应用启动时，按以下顺序依次找配置文件；如果没找到，则顺延到下一条： 
 
- (1) c/c++ 用户可以通过环境变量 **DFZQ_GRPC_CONFIG=/xxx/xxx** 配置grpc配置文件所在的目录的绝对路径。 
- (2) 从启动目录下的**config**中查找grpc配置文件 
- (3) 从启动目录下查找grpc配置文件  

 dfzq-grpc-config.properties：

	
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
	provider.grpc=1.2.0
    # 可选,类型boolean,缺省值true,说明:表示当前服务器实例是否为主服务器
    # 参数值配置为true表示主服务器，false表示备服务器
    # 使用场合：实现主备自动切换
    # provider.master=true

    # 可选,类型string,缺省值为空，说明:表示当前服务器属于某个分组
    # 使用场合：服务分组功能
    #provider.group=A


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
	# 可选,类型string,缺省值为空，说明:表示当前客户端可以调用的服务端分组
    # 使用场合：服务分组，业务隔离，通过分组优先实现多机房支持
    # 配置示例：A      含义:当前客户端只能调用分组为A的服务端
    # 配置示例：A,B    含义:当前客户端只能调用分组为A或者分组为B的服务端
    # 配置示例：A1,A2;B1,B2;C1,C2
    # 含义: 当前客户端优先访问分组为A1、A2的服务端。如果分组为A1、A2服务端不存在，访问分组为B1、B2的服务端；
    #      如果分组为B1、B2的服务端也不存在，访问分组为C1、C2的服务端；如果分组为C1、C2的服务端也不存在，客户端报错。
    # consumer.group=

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
	
	# ----begin----区分内外部服务、私有注册中心配置----------------------

    # 类型string，说明: 私有注册中心服务器列表，既支持单机，也支持集群
    # 可选,zookeeper.host.server和zookeeper.private.host.server至少配置一个参数
    # zookeeper.private.host.server=

    # 可选，类型string，说明: 服务注册根路径，默认值/Application/grpc
    # zookeeper.private.root=

    # 可选，类型string，说明：私有注册中心digest模式ACL的用户名
    # zookeeper.private.acl.username=

    # 可选，类型string，说明：私有注册中心digest模式ACL的密码
    # zookeeper.private.acl.password=

    # 可选，类型string，说明：表示公共服务名称列表，多个服务名称之间以英文逗号分隔，如果不配置，表示所有服务都是公共服务
    # 参数值示例 com.orientsec.hello.Greeter3,com.orientsec.hello.Greeter4
    # 属性值过长可以在行末增加反斜杠\，然后在下一行继续配置其它属性值，注意反斜杠\后面不能有空格
    # public.service.list=

    # 可选，类型string，说明：表示私有服务名称列表，多个服务名称之间以英文逗号分隔，如果不配置，将公共服务名称列表之外的服务都视为私有服务
    # 参数值示例 com.orientsec.hello.Greeter1,com.orientsec.hello.Greeter2
    # 属性值过长可以在行末增加反斜杠\，然后在下一行继续配置其它属性值，注意反斜杠\后面不能有空格
    # private.service.list=

    #----end------区分内外部服务、私有注册中心配置----------------------


    # ----begin----配置中心容灾降级----------------------

    # 可选,类型string,说明：该参数用来手动指定提供服务的服务器地址列表。
    # 使用场合: 在zookeeper注册中心不可用时，通过该参数指定服务器的地址；如果有多个服务，需要配置多个参数。
    # 特别注意: 一旦配置该参数，客户端运行过程中，即使注册中心恢复可用，框架也不会访问注册中心。
    #           如果需要从配置中心查找服务端信息，需要注释掉该参数，并重启客户端应用。
    # xxx表示客户端调用的服务名称
    # service.server.list[xxx]=10.45.0.100:50051
    # service.server.list[xxx]=10.45.0.100:50051,10.45.0.101:50051,10.45.0.102:50051

    # service.server.list[com.orientsec.hello.Greeter1]=10.45.0.100:50051,10.45.0.101:50051
    # service.server.list[com.orientsec.hello.Greeter2]=10.45.0.100:50052,10.45.0.101:50052

    # ----end------配置中心容灾降级----------------------

    # ------------ end of zookeeper config ------------


    
### 4. 客户端调用的创新
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


### 5. 服务提供者(服务端)

服务提供者，启动一个Server对象，等待客户端的连接。

- 5.1 服务代码实现  
服务实现类，是对Protocol Buffers文件定义的方法sayHello进行具体的业务实现。


例子：helloword.greeter

	#include <iostream>
	#include <memory>
	#include <string>

	#include <grpcpp/grpcpp.h>

	#ifdef BAZEL_BUILD
	#include "examples/protos/helloworld.grpc.pb.h"
	#else
	#include "helloworld.grpc.pb.h"
	#endif

	using grpc::Server;
	using grpc::ServerBuilder;
	using grpc::ServerContext;
	using grpc::Status;
	using helloworld::HelloRequest;
	using helloworld::HelloReply;
	using helloworld::Greeter;

	static int invoke_count = 0;
	// Logic and data behind the server's behavior.
	class GreeterServiceImpl final : public Greeter::Service {
		Status SayHello(ServerContext* context, const HelloRequest* request,
			HelloReply* reply) override {
			std::string prefix("Hello ");
			reply->set_message(prefix + request->name());

			std::cout << "\n Service is Called ... " << request->name() << "\n" << std::endl;
			invoke_count++;
			std::cout << "invoke_count = " << invoke_count << std::endl;
			return Status::OK;
		}
	};

	void RunServer() {
		std::string server_address("0.0.0.0:50066");
		GreeterServiceImpl service;

		ServerBuilder builder;
		// Listen on the given address without any authentication mechanism.
		builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
		// Register "service" as the instance through which we'll communicate with
		// clients. In this case it corresponds to an *synchronous* service.
		builder.RegisterService(&service);
		// Finally assemble the server.
		std::unique_ptr<Server> server(builder.BuildAndStart());
		std::cout << "Server listening on " << server_address << std::endl;

		// Wait for the server to shutdown. Note that some other thread must be
		// responsible for shutting down the server for this call to ever return.
		server->Wait();
	}

	int main(int argc, char** argv) {
		RunServer();

		return 0;
	}
- 5.2 配置与编译服务端文件
 - 5.2.1 项目配置  
   1）打开项目属性

![配置图](https://raw.githubusercontent.com/grpc-nebula/grpc-nebula/master/images/config5.2.1.1.png)

2）配置属性--常规 选项卡  
如图所示：  
![配置图](https://raw.githubusercontent.com/grpc-nebula/grpc-nebula/master/images/config5.2.1.2.png)
3）配置属性—调试 选项卡  
如图所示：  
![配置图](https://raw.githubusercontent.com/grpc-nebula/grpc-nebula/master/images/config5.2.1.3.png)

4）配置属性—VC++目录 选项卡  
如图所示：默认配置  
![配置图](https://raw.githubusercontent.com/grpc-nebula/grpc-nebula/master/images/config5.2.1.4.png)

5）配置属性-->C/C++ -->常规 选项卡  
如图所示：  
附加库目录：  
../../../include/include  
../../../include/third_party/protobuf/src  
../../../include/orientsec_grpc  
../../../include/include/grpc  
![配置图](https://raw.githubusercontent.com/grpc-nebula/grpc-nebula/master/images/config5.2.1.5.png)

6）配置属性-->C/C++ -->优化 选项卡  
如图所示：  
Debug版本禁用优化，Release版本选择最大优化。  
![配置图](https://raw.githubusercontent.com/grpc-nebula/grpc-nebula/master/images/config5.2.1.6.png)

7）配置属性-->C/C++ -->预处理器 选项卡  
如图所示：  
预处理器定义：  
_WIN32_WINNT=0x600  
_SCL_SECURE_NO_WARNINGS  
_CRT_SECURE_NO_WARNINGS  
_DEBUG  
![配置图](https://raw.githubusercontent.com/grpc-nebula/grpc-nebula/master/images/config5.2.1.7.png)

8）配置属性-->C/C++ -->代码生成 选项卡  
如图所示：默认配置  
![配置图](https://raw.githubusercontent.com/grpc-nebula/grpc-nebula/master/images/config5.2.1.8.png)

9）配置属性-->C/C++ -->语言 选项卡  
如图所示：默认配置  
![配置图](https://raw.githubusercontent.com/grpc-nebula/grpc-nebula/master/images/config5.2.1.9.png)

10）配置属性-->C/C++ -->预编译头 选项卡  
如图所示：  
预编译头：不使用预编译头  

![配置图](https://raw.githubusercontent.com/grpc-nebula/grpc-nebula/master/images/config5.2.1.10.png)

11）配置属性-->C/C++ -->输出文件 选项卡  
如图所示：默认配置  

![配置图](https://raw.githubusercontent.com/grpc-nebula/grpc-nebula/master/images/config5.2.1.11.png)

12）配置属性-->C/C++ -->浏览信息 选项卡  
如图所示：默认配置  
![配置图](https://raw.githubusercontent.com/grpc-nebula/grpc-nebula/master/images/config5.2.1.12.png)

13）配置属性-->C/C++ -->高级 选项卡  
如图所示：默认配置  
![配置图](https://raw.githubusercontent.com/grpc-nebula/grpc-nebula/master/images/config5.2.1.13.png)

14）配置属性-->C/C++ -->所有选项 选项卡  
默认配置  
15）配置属性-->C/C++ -->命令行 选项卡  
默认配置  
16）配置属性-->链接器 -->常规 选项卡  
如图所示：   
附加库目录：  
../../../lib/x64/debug  
%(AdditionalDependencies)  
![配置图](https://raw.githubusercontent.com/grpc-nebula/grpc-nebula/master/images/config5.2.1.16.png)

17）配置属性-->链接器 -->输入 选项卡  
如图所示：   
附加依赖项：  
address_sorting.lib  
gpr.lib  
grpc.lib  
grpc++.lib  
orientsec_consumer.lib  
orientsec_common.lib  
orientsec_registry.lib  
cares.lib  
Ws2_32.lib  
zlibstaticd.lib  
ssl.lib  
crypto.lib  
libprotobufd.lib  
libprotobuf-lited.lib  

![配置图](https://raw.githubusercontent.com/grpc-nebula/grpc-nebula/master/images/config5.2.1.17.png)

18）配置属性-->链接器 -->清单文件 选项卡  
如图所示： 默认配置  
![配置图](https://raw.githubusercontent.com/grpc-nebula/grpc-nebula/master/images/config5.2.1.18.png)

19）配置属性-->链接器 -->调试 选项卡  
如图所示：   
![配置图](https://raw.githubusercontent.com/grpc-nebula/grpc-nebula/master/images/config5.2.1.19.png)  

20）配置属性-->链接器 -->系统 选项卡  
如图所示：    
子系统： 控制台 console  
![配置图](https://raw.githubusercontent.com/grpc-nebula/grpc-nebula/master/images/config5.2.1.20.png)  

21）配置属性-->链接器 -->优化 选项卡  
如图所示：  
![配置图](https://raw.githubusercontent.com/grpc-nebula/grpc-nebula/master/images/config5.2.1.21.png)  

22）配置属性-->链接器 -->嵌入IDL 选项卡  
默认配置  
23）配置属性-->链接器 -->windows元数据 选项卡  
默认配置  
24）配置属性-->链接器 -->高级 选项卡  
默认配置  

   - 5.2.2 编译服务端
    - 1）选中demo-sync-server工程，点击右键，点击生成按钮。  
输出信息如图所示：  
![配置图](https://raw.githubusercontent.com/grpc-nebula/grpc-nebula/master/images/config5.2.2.1.png)  
......  
![配置图](https://raw.githubusercontent.com/grpc-nebula/grpc-nebula/master/images/config5.2.2.1-1.png)  

2）在x64/Debug目录下生成server运行程序（.exe）。  
如图所示：  
![配置图](https://raw.githubusercontent.com/grpc-nebula/grpc-nebula/master/images/config5.2.2.2.png)  


### 6. 服务消费者（客户端）
- 6.1客户端代码实现

  服务消费者，通过调用**GreeterClientImpl client(grpc::CreateChannel(,));**方法建立Channel，其中target的组成组成规则为“ **zookeeper:///** + **服务名称** ”。框架会到注册中心zookeeper上去寻找对应服务名称的服务提供者。

  例子：demo-sync-client.cpp  
  代码比原生例子增加了多线程的支持。简单实例请参考原生示例。  
	
		#include <iostream>
		#include <memory>
		#include <string>

		#include <grpcpp/grpcpp.h>
		#include <grpc/support/thd_id.h>
		#include <grpc/support/alloc.h>

		#ifdef BAZEL_BUILD
		#include "examples/protos/helloworld.grpc.pb.h"
		#else
		#include "helloworld.grpc.pb.h"
		#endif

		#if defined(_MSC_VER)
		#define thread_local __declspec(thread)
		#elif defined(__GNUC__)
		#define thread_local __thread
		#else
		#error "Unknown compiler - please file a bug report"
		#endif
		static thread_local struct thd_info *g_thd_info;

		struct thd_info {
			void(*body)(void *arg); /* body of a thread */
			void *arg;               /* argument to a thread */
			HANDLE join_event;       /* if joinable, the join event */
			int joinable;            /* true if not detached */
		};
		static void destroy_thread(struct thd_info *t) {
			if (t->joinable) CloseHandle(t->join_event);
			gpr_free(t);
		}


		enum { GPR_THD_JOINABLE = 1 };

		static DWORD WINAPI thread_body(void *v) {
			g_thd_info = (struct thd_info *)v;
			g_thd_info->body(g_thd_info->arg);
			if (g_thd_info->joinable) {
				BOOL ret = SetEvent(g_thd_info->join_event);
				GPR_ASSERT(ret);
			}
			else {
				destroy_thread(g_thd_info);
			}
			return 0;
		}

		typedef struct {
			int flags; /* Opaque field. Get and set with accessors below. */
		} gpr_thd_options;

		using grpc::Channel;
		using grpc::ClientContext;
		using grpc::Status;
		using helloworld::HelloRequest;
		using helloworld::HelloReply;
		using helloworld::Greeter;

		class GreeterClient {
		public:
			GreeterClient(std::shared_ptr<Channel> channel)
				: stub_(Greeter::NewStub(channel)) {}

			// Assembles the client's payload, sends it and presents the response back
			// from the server.
			std::string SayHello(const std::string& user) {
				// Data we are sending to the server.
				HelloRequest request;
				request.set_name(user);

				// Container for the data we expect from the server.
				HelloReply reply;

				// Context for the client. It could be used to convey extra information to
				// the server and/or tweak certain RPC behaviors.
				ClientContext context;

				// The actual RPC.
				Status status = stub_->SayHello(&context, request, &reply);

				// Act upon its status.
				if (status.ok()) {
					return reply.message();
				}
				else {
					std::cout << status.error_code() << ": " << status.error_message()
						<< std::endl;
					return "RPC failed";
				}
			}
	
		private:
			std::unique_ptr<Greeter::Stub> stub_;
		};

		static int gpr_thd_options_is_joinable(const gpr_thd_options* options) {
			if (!options) return 0;
			return (options->flags & GPR_THD_JOINABLE) == GPR_THD_JOINABLE;
		}

		static int gpr_thd_new(gpr_thd_id *t, void(*thd_body)(void *arg), void *arg,
			const gpr_thd_options *options) {
			HANDLE handle;
			struct thd_info *info = static_cast<thd_info *>(gpr_malloc(sizeof(*info)));
			info->body = thd_body;
			info->arg = arg;
			*t = 0;
			if (gpr_thd_options_is_joinable(options)) {
				info->joinable = 1;
				info->join_event = CreateEvent(NULL, FALSE, FALSE, NULL);
				if (info->join_event == NULL) {
					gpr_free(info);
					return 0;
				}
			}
			else {
				info->joinable = 0;
			}
			handle = CreateThread(NULL, 64 * 1024, thread_body, info, 0, NULL);
			if (handle == NULL) {
				destroy_thread(info);
			}
			else {
				*t = (gpr_thd_id)info;
				CloseHandle(handle);
			}
			return handle != NULL;
		}

		void multiple(void * arg)
		{
		#if 0
			GreeterClient greeter(grpc::CreateChannel(
				"localhost:50051", grpc::InsecureChannelCredentials()));
		#else
			//GreeterClient greeter(grpc::CreateChannel("zookeeper:///dfzq.helloworld.Greeter", grpc::InsecureChannelCredentials()));
			GreeterClient greeter(grpc::CreateChannel(
				"zookeeper:///helloworld.Greeter", grpc::InsecureChannelCredentials()));
		#endif
	
		// for consistent hash testing	
		#if 0
			int count = 300;
			char *name = new char[20];
			memset(name, 0, sizeof(char) * 20);
			while (count--) {
				std::string user("world");
				char str[10];
				if (!(count % 10)) {
					sprintf(str, "%0d", count);
					user.append(str);
					strcpy(name, user.c_str());
				}
				std::string reply = greeter.SayHello(name);
				std::cout << "Greeter received: " << reply << std::endl;
			}
		#else
			int count = 100;
			while (count--) {
				std::string user("world");
				char str[10];
				sprintf(str, "%0d", count);
				user.append(str);

				std::string reply = greeter.SayHello(user);
				std::cout << "Greeter received: " << reply << std::endl;
				//Sleep(1000);
			}
		#endif
	
		}
	
		int main(int argc, char** argv) {
			// Instantiate the client. It requires a channel, out of which the actual RPCs
			// are created. This channel models a connection to an endpoint (in this case,
			// localhost at port 50051). We indicate that the channel isn't authenticated
			// (use of InsecureChannelCredentials()).
			//gpr_set_log_verbosity(GPR_LOG_SEVERITY_INFO);
			//gpr_set_log_verbosity(GPR_LOG_SEVERITY_ERROR);
			//gpr_set_log_verbosity(GPR_LOG_SEVERITY_DEBUG);

			gpr_thd_id thd_id;
			//开启n线程并发调用
			for (int i = 0; i < 1; i++) {
				gpr_thd_new(&thd_id, multiple, NULL, NULL);
			}

			getchar();

			return 0;
		}


- 6.2 配置客户端项目    
   同服务端项目配置。

- 6.3 编译客户端文件
  - 1）选中demo-sync-client工程，点击右键，点击生成按钮。  
如图所示：  
 ![配置图](https://raw.githubusercontent.com/grpc-nebula/grpc-nebula/master/images/config6.3.1.png)  
  - 2）编译信息输出：  
   ![配置图](https://raw.githubusercontent.com/grpc-nebula/grpc-nebula/master/images/config6.3.2-1.png) 
......  
   ![配置图](https://raw.githubusercontent.com/grpc-nebula/grpc-nebula/master/images/config6.3.2-2.png) 
  - 3）在x64/Debug目录下生成client运行程序（.exe）。  
如图所示：  
 ![配置图](https://raw.githubusercontent.com/grpc-nebula/grpc-nebula/master/images/config6.3.3.png)  



### 7. 运行程序   
- 7.1准备工作  
  1) 将config目录及其下的dfzq-grpc-config.properties配置文件拷贝到.\demo\x64\Debug中    
  2) 启动Zookeeper    
  3) 将Zookeeper IP地址和端口号修改到配置文件中   
zookeeper.host.server=XX：YY    
  4) 将zookeeper 动态链接库zookeeper.dll 拷贝到 客户端（exe）和服务端（exe）同级目录中    
- 7.2运行应用程序：  
   1) 服务端启动    
  双击demo-sync-server.exe  
 ![运行图](https://raw.githubusercontent.com/grpc-nebula/grpc-nebula/master/images/config7.2.1.png)  
   2) 客户端启动  
双击demo-async-client.exe  
 ![运行图](https://raw.githubusercontent.com/grpc-nebula/grpc-nebula/master/images/config7.2.2.png)  

### 8. 测试  
服务调用成功

服务端提示：

 ![结果图](https://raw.githubusercontent.com/grpc-nebula/grpc-nebula/master/images/config8.1.png)  

客户端提示:

  ![结果图](https://raw.githubusercontent.com/grpc-nebula/grpc-nebula/master/images/config8.2.png)  


## 常见问题
### 1． 注册不成功  
- A. 检查zookeeper IP和端口是否配置
- B. 检查zookeeper digest 用户名和密码是否正确
- C. 检查 服务端机器和zookeeper主机之间的网络连通

### 2.  调用失败
- A. 检查 服务端和客户端 之间网络连通。
- B. 检查服务端是否在线。
- C. 检查调用的服务名服务是否在线。

### 3.  更改过proto文件之后，注意修改代码中的命名空间。

### 4.  如果遇到编译通过而不能正确调用
- A．查看对应client文件里创建channel里服务名是否正确，服务名需跟proto一致
  channel = grpc::CreateChannel("zookeeper:///helloworld.Greeter", grpc::InsecureChannelCredentials());
- B．查看server与client对应的proto文件是否一致，如果proto文件不一致是无完成调用的


