# <center>微服务治理框架(C++版)详细设计</center>

---

## 概述
gRPC 是一款高性能、开源的 RPC 框架，产自 Google，基于 ProtoBuf 序列化协议进行开发，支持多种语言（C++、Golang、Python、Java等） 
gRPC 对 HTTP/2 协议的支持使其在 Android、IOS 等客户端后端服务的开发领域具有良好的前景。 
gRPC 提供了一种简单的方法来定义服务，同时客户端可以充分利用 HTTP2 stream 的特性，从而有助于节省带宽、降低 TCP 的连接次数、节省CPU的使用等。

（1）服务端：服务端需要实现.proto中定义的方法，并启动一个gRPC服务器用于处理客户端请求。gRPC反序列化到达的请求，执行服务方法，序列化服务端响应并发送给客户端。 

（2）客户端：客户端本地有一个实现了服务端一样方法的对象，gRPC中称为桩或者存根，其他语言中更习惯称为客户端。客户调用本地存根的方法，将参数按照合适的协议封装并将请求发送给服务端，并接收服务端的响应。

![通信模型图](https://raw.githubusercontent.com/grpc-nebula/grpc-nebula/master/images/grpc_transport.png)
 
通信模型示意图  
# 服务端
***

### 1. 服务端读取配置
- 1.1 原理分析  
 + 原生分析  
	无此功能
 + 定制分析   
    读取本地的配置文件信息 dfzq-grpc-config.properties应用程序启动时，按以下顺序依次找配置文件；如果没找到，则顺延到下一条：
	
		* 从系统环境变量ORIENTSEC_GRPC_CONFIG读取
		
		* 用户从启动目录下的config目录下查找grpc配置文件
		
		* 用户从当前目录下查找grpc配置文件
    
	框架第一次读取配置文件时，会将配置文件中的所有内容读取到内存。之后获取配置文件中的属性时，直接从内存中读取。

- 1.2 场景描述
 	1.	实现对provider端的参数控制
 	2.	实现对consumer端的参数控制
 	3.	实现zookeeper ip地址、acl相关信息配置

- 1.3接口设计
  - 1.3.1 关键文件  

			third_party/orientsec/orientsec_common/orientsec_grpc_properties_tools.h
			third_party/orientsec/orientsec_common/orientsec_grpc_properties_tools.c

  - 1.3.2关键接口

			01：  
			名称：orientsec_grpc_properties_init  
			参数：无  
			功能：实现配置文件的正确读取  
			返回值：0 正确读取  
			-1，-2 读取失败  
			02：
			名称：orientsec_grpc_properties_get_value  
			参数：  
			key：关键字  
			prefix：前缀  
			value：值  
			功能：读取各个参数的具体值  
			返回值：  
			0 通过指针返回值  
			-1 读取失败  

  - 1.4流程图  
  无  
- 1.5代码修改思路  
	- 增加配置文件  
	- 灵活的可配置参数  
	- 一次读取，内存存取  
- 1.6耦合度  
  与原生代码无耦合

### 2. Provider注册设计说明
- 2.1原理分析
  - 2.1.1 原生分析  
  无
  - 2.1.2 定制分析  
  增加zookeeper中间件，服务端启动后自动注册服务到zookeeper中，服务消费方可以通过zk拿到服务提供方的信息。

- 2.2场 景描述  
  服务provider和服务consumer通过和zookeeper进行通信实现服务注册、服务发现、服务调度、配置同步。
- 2.3接口设计
  - 2.3.1 关键文件 
  
			文件1：实现接口文件 
			third_party\orientsec\orientsec_provider\orientsec_provider_intf.c  
			文件2：调用接口文件
			src\cpp\server\server_cc.cc  
			src\cpp\server\server_builder.cc

  - 2.3.2 关键接口

			01：  
			名称：provider_registry  
			参数：  
			port：服务端口  
			sIntf：服务名  
			sMethods：方法名  
			功能：Zookeeper Resolve插件注册入口  
			接口全称：  
			void provider_registry(int port, const char *sIntf, const char *sMethods)   
			返回值  
			无  

- 2.4示意图

![注册示意图](https://raw.githubusercontent.com/grpc-nebula/grpc-nebula/master/images/zk.png)

- 2.5代码修改思路  
与原生代码耦合处：
需要在server启动的时候获取服务端口，服务名和方法名，并调用注册接口，此处会影响原生代码。而且同步和异步调用服务启动不同，需要分别获取和添加。


## 3 服务关闭时，自动注销zk中的Provider信息 
- 3.1原理分析：  
注销服务提供者，就是将服务提供者的信息或者节点从zookeeper中删除。我们将注销代码嵌入到服务器对象析构的方法中，在调用服务器对象shutdown方法时，将服务提供者信息删除。
由于我们在注册时，向zookeeper中写的是临时节点，如果程序未正常关闭，利用zookeeper自身的定时检测长连接的机制，一旦发现提供服务的程序与zookeeper之间的连接意外断开，就自动将服务提供者删除。
- 3.2实现思路  
在新增的 orientsec -provider 模块中实现注销服务信息。  
- 3.3 相关代码：

		third_party\orientsec\orientsec_provider\orientsec_provider_intf.h  
		void provider_unregistry();  
		在Server对象的析构中调用:  
		src\cpp\server\server_cc.cc  
## 4 服务流量控制
- 4.1原理分析  
服务端可以采取两种手段进行服务流量控制，一种是并发请求数控制，另一种是并发连接数控制。  
  - 并发请求数，指的是服务端同一时刻最多可以处理的请求数量。  
  - 并发连接数，指的是与服务端建立的 HTTP/2 连接数量，一个连接对应一个客户端。  
服务端在启动时订阅configurators目录,在订阅目录发生改变时，在回调函数`provider_configurators_callback（）`中会调用`update_provider_connection`方法，可以通过动态配置方法进行并发请求数的修改。
服务端启动时会调用`init_provider()`方法 ，该方法定义了服务端建立连接的初始化方法，并设置了初始化的请求数和连接数。
当客户端访问服务端时，可以通过`check_provider_connection()`来实现连接数控制。

- 4.2实现思路  
  - 4.2.1 并发请求数流量控制：  
（1）为服务端增加一个计数器，用来记录并发请求数。  
（2）每接收到一个请求时，检查当前的请求数是否达到最大请求数。如果已经达到最大请求数直接拒绝客户端的请求（客户端会接收到一个异常信息），如果未达到计数器增加1；  
（3）每处理完一个请求时，计数器减少1。  
 - 4.2.2. 连接数控制流量控制：  
服务端有一个集合类记录着服务端与客户端建立的连接信息。当接收到客户端建立连接的请求时，检查当前的连接数是否达到最大的连接数。如果已经达到最大连接数，拒绝客户端建立连接的请求（客户端会接收到一个异常信息）；如果未达到，建立连接，并将连接信息记录到集合类中。
- 4.3场景描述  
并发请求数使用default.requests参数进行配置，连接数使用default.connections参数进行配置。  
并发请求数参数和连接数参数可以同时配置，这是从两个不同的维度进行流量控制。前者控制的是服务者的并发处理能力，后者控制的是与之连接的客户端的个数。
默认情况下，并发请求数参数值为2000，连接数为20。即同一时刻最多可以有20个客户端与服务端建立连接，同时服务端处理客户端的请求最大并发数为2000。
如果因为客户端超过了默认连接数20的限制，从而导致客户端出现异常，可以通过服务治理平台配置该服务的最大连接数。
如果因为客户端调用频繁、并发程度高，或者服务端处理业务逻辑时间较长，可以通过服务治理平台配置该服务的最大并发请求数。
- 4.4相关代码  
涉及到的模块与代码：

		grpc-core 模块：
		http2传输层
		修改 on_accept（）增加check()

		orientsec-provider 模块：
		新增 bool check_provider_connection(const char *intf)

## 5. 服务端配置信息监听
 - 5.1实现思路  
在服务注册的最后一步，注册一个监听当前服务的监听器，用来监听注册中心上该服务的以下几种配置信息。
  - 5.1.1服务并发请求数配置(default.requests)  
当监听到注册中心上并发请求数参数配置发现改变后，将新的参数值更新到内存中；当监听到注册中心上并发请求参数配置信息被删除后，查询服务端初始的并发请求数配置，将内存中的并发请求数恢复到初始配置。
  - 5.1.2服务连接数配置(default.connections)  
当监听到注册中心上连接数参数配置发现改变后，将新的参数值更新到内存中；当监听到注册中心上连接数参数配置信息被删除后，查询服务端初始的连接数配置，将内存中的连接数恢复到初始配置。
  - 5.1.3访问保护状态配置(access.protected)  
当监听到注册中心上访问保护状态配置发现改变后，将新的参数值更新到内存中。当监听到注册中心上访问保护状态配置信息被删除后，查询服务端初始的访问保护状态，将内存中的访问保护状态恢复到初始配置。
然后根据当前的访问保护状态的参数值进行以下操作：  
如果参数值为true，向注册中心写入一条“禁止所有客户端访问当前服务端的路由规则”；
如果参数值为false，将注册中心上“禁止所有客户端访问当前服务端的路由规则”删除。
  - 5.1.4 服务是否有新版本配置(deprecated)  
当监听到注册中心上deprecated参数配置发现改变后，将新的参数值更新到内存中；当监听到注册中心上deprecated参数配置信息被删除后，查询服务端初始的deprecated配置，将内存中的deprecated参数值恢复到初始配置。
- 5.2 相关代码
涉及到的模块与代码：

		orientsec-provider 模块：
		新增 void init_provider(provider_t* provider)
		新增 void provider_configurators_callback(url_t *urls, int url_num)
		新增 check_provider_request(const char *intf)
		新增 update_provider_connection(const char *intf, int conns)
		新增 update_provider_access_protected(const char* intf, bool access_protected)
## 6 服务过时打印告警日志
- 6.1 原理分析  
服务端初始化时或者在zookeeper中动态设置deprecated参数时，修改provider的属性。

- 6.2 实现思路  
当服务端被调用时，如果当前服务的deprecated参数值为true，打印告警日志，1天只打印一次告警日志。
1天只打印一次日志可以通过增加一个变量存储“上一次记录deprecated日志的时间戳”来实现。
告警日志内容示例：当前服务[com.orientsec.grpc.examples.helloworld.Greeter]已经过时，请检查是否新服务上线替代了该服务。
- 6.3 相关代码  
涉及到的模块与代码：

		orientsec-grpc-provider 模块：
		新增 void update_provider_deprecated(const char *intf, bool deprecated)

## 7 服务注册支持配置自定义IP与端口

- 场景描述

使用场景1：针对利用Nginx做grpc反向代理的场景，服务提供者可以通过配置文件将Nginx的地址注册到Zookeeper

使用场景2：针对服务器跨网段调用时会被映射为另一个IP的场景，应允许服务将自身地址配置为一个映射的IP

- 实现思路

在配置文件，增加自定义IP与端口的配置信息

	# 可选，类型string，说明：服务注册时指定的IP，优先级高于common.localhost.ip参数
	# common.service.ip=
	
	# 可选，类型int，说明：服务注册时指定的端口
	# common.service.port=


在服务端向注册中心进行注册时，会将服务真实的IP与端口添加到``real.ip``和``real.port``参数中，如果配置了自定义的IP与端口，则使用该配置的IP与端口对服务进行注册；如果未配置，则使用真实的ip与端口进行注册；无论是否有配置，真实的IP与端口都将添加到``real.ip``与``real.port``参数中。

- 相关代码

涉及到的模块与代码：

	orientsec-provider 模块：
	修改 provider_registry()
	添加 orientsec_add_property_into_url_inner()

# 客户端
---
## 1 读取本地的配置文件信息
- 1.1原理分析    
框架第一次读取配置文件时，会将配置文件中的所有内容读取到内存。之后获取配置文件中的属性时，直接从内存中读取。
- 1.2实现思路  
  配置文件读取顺序 
 
    	1、从系统环境变量ORIENTSEC_GRPC_CONFIG读取  
    	2、从当前目录下config文件夹读取  
		3、从当前目录下读取  

- 1.3相关代码：

		orientsec_common\orientsec_grpc_properties_tools.h  
		int orientsec_grpc_properties_init();  
		int orientsec_grpc_properties_get_value(const char *key, char *prefix, char *value);

## 2. 客户端启动时，注册客户端信息
- 2.1原理分析  
客户端启动时， 创建 Channel 对象。在创建 Channel 对象时，增加调用注册客户端信息的接口。
- 2.2实现思路  
新增 orientsec-consumer 模块，在该模块中增加注册客户端信息的接口和实现。
- 2.3相关代码  
涉及到的模块与代码：

		grpc-core 模块：
		修改 grpc_channel_create_with_builder()
		新增 channel->reginfo = orientsec_grpc_consumer_register(sn)        

		orientsec-consumer 模块： 
		新增 char* orientsec_grpc_consumer_register(const char* fullmethod)
## 3. 客户端关闭时，注销客户端信息
- 3.1原理分析  
客户端关闭Channel时，会将ZookeeperNameResolver关闭，在关闭ZookeeperNameResolver时会注销客户端信息。
- 3.2实现思路  
在 orientsec-consumer 模块中，增加注册客户端信息的接口方法。
- 3.3相关代码  
涉及到的模块与代码：

		orientsec-consumer 模块：
		orientsec_consumer\orientsec_consumer_intf.h
		int orientsec_grpc_consumer_unregister(char * reginfo);

		orientsec-grpc 模块：
		src\core\lib\surface\channel.cc
		void grpc_channel_destroy()
		orientsec_grpc_consumer_unregister(channel->reginfo)
## 4. 基于zookeeper的NameResolver

- 4.1 原理分析
  - 4.1.1 原生分析  
     - 1），原生grpc-c 一共采用三种Resolve方法，分别是DNS，fake，sockaddr，其中dns分为Native DNS以及Area DNS。Sockaddr为传统的套接字模式，fack与sockaddr模式类似，区别是fack支持查询参数。目前V1.17.2默认采用DNS方式，但是支持参数化配置。  
     - 2），所有的方式都需要通过三方插件模式加载。  
   调用方式：  
     `1） GreeterClient greeter(grpc::CreateChannel("localhost:55555",*);  `
     `2） GreeterClient greeter(grpc::CreateChannel("IP:PORT",*);  `
  - 4.1.2 定制分析  
定制版准备延续原生模式，以插件形式注册，由于我们的服务治理是基于zookeeper，所以，当我们是原生访问模式，我们继续延续原生的方式，当使用zookeeper:///{serviceName}模式时，我们采用定制化Zookeeper重写Resolver。
调用方式：  
`1） GreeterClient greeter(grpc::CreateChannel("zookeeper:///{serviceName}",*);`

- 4.2 场景描述  
Zookeeper NameResolve定制化开发，实现基于Zookeeper的NameResolve功能。
- 4.3接口设计  
  - 4.3.1关键文件  
  
			文件1：Zookeeper Resolver建立文件  
			grpc\src\core\ext\filters\client_channel\resolver\zookeeper\zookeeper_resolver.cpp
			文件2：NameResolver功能文件
			grpc\src\core\lib\iomgr\zk_resolve_address.c
			文件3：插件注册入口文件
			grpc\src\core\plugin_registry\grpc_unsecure_plugin_registry.cc
			grpc\src\core\plugin_registry\grpc_plugin_registry.cc

  - 4.3.2关键接口  

			01，
			名称：consumer_query_providers
			参数：
			service_name：serviceName传入
			nums：provider数量传出
			hasharg：一致性哈希算法传参
			功能：Zookeeper Resolve插件注册入口
			接口全称：
			provider_t* consumer_query_providers(const char *service_name, int *nums, char*hasharg) 

			02：
			名称：zk_blocking_resolve_address
			参数：
			name：服务名
			default_port：默认端口（如果对应provider找不到端口，则使用默认端口）
			addresses：接收类
			功能：通过解析provider从ServiceName中获取host
			接口全称：
			static grpc_error* zk_blocking_resolve_address(
			const char* name,
			const char* default_port,
			grpc_resolved_addresses** addresses)

			03，
			名称：zk_resolve_address
			参数：
			name：服务名
			default_port：默认端口（如果对应provider找不到端口，则使用默认端口）
			interested_parties：略
			on_done：回调
			addresses：接收类
			功能：Zookeeper address resolve接口入口
			接口全称：
			void zk_resolve_address(const char* name, const char* default_port,
                                    grpc_pollset_set* interested_parties,
                                    grpc_closure* on_done,
                                    grpc_resolved_addresses** addresses)
			04，
			名称：grpc_resolver_zk_init
			参数：无
			功能：Zookeeper Resolve插件注册入口
			接口全称：
			void grpc_resolver_zk_init() 

- 4.4代码修改思路  
A，将Zookeeper Resolve插件注册到plugin模块初始化中  
B，通过DNS架构构建自己的ZookeeperResolve  
C，将ServiceName解析加入到ZookeeperResolve中，将原来的DNS解析去除替换。  
D，测试是否可以完成NameResolver全过程并完成调用。  

## 5. 监听服务端信息
- 5.1 原理分析  
在客户端启动，注册客户端信息时，同时注册一个监听器，用来监听服务端列表的变化。同时，将服务端列表存储到内存中。
当监听到服务端上线时，更新缓存。当监听到有服务端下线时，更新缓存。
- 5.2实现思路  
在orientsec-consumer模块中，修改客户端注册的方法，在注册客户端信息的同时，注册一个服务端列表的监听器。然后在监听器中实现服务端列表更新后的业务逻辑。
- 5.3相关代码  
涉及到的模块与代码：

		orientsec-consumer 模块：
		消费者 providers目录订阅函数
		新增 void consumer_providers_callback(url_t* urls, int url_num)

## 6. 实现客户端对路由规则的解析方法，用来过滤服务端列表
- 6.1原理分析

 ![workflow](https://raw.githubusercontent.com/grpc-nebula/grpc-nebula/master/images/workflow.png "consumer workflow")
  
- 6.2实现思路  
在grpc-core模块中，首先注册一个监听路由规则的目录订阅函数 consumer_routers_callback，然后新增一个路由规则解析器ConditionRouter，然后在获取服务端列表的模块 ZookeeperNameResolver调用路由规则解析器，实现服务端列表的过滤。
- 6.3相关代码  
涉及到的模块与代码：

		grpc-core 模块：
		新增 zk_blocking_resolve_address()
		orientsec_consumer模块：
		新增class condition_router 
		新增void consumer_routers_callback(url_t* urls, int url_num)

## 7. 支持路由规则可以设置为IP段、项目
- 7.1原理分析  
路由规则由两个条件组成，分别用于对客户端和服务端进行匹配。比如有这样一条规则：
`host = 192.168.1.* => host = 192.168.2.*`
该条规则表示 IP 为 192.168.1.* 的客户端只可调用 IP 为 192.168.2.* 服务器上的服务，不可调用其他服务端上的服务。路由规则的格式如下：
[客户端匹配条件] => [服务端匹配条件]
如果客户端匹配条件为空，表示不对客户端进行限制。如果服务端匹配条件为空，表示对某些客户端禁用服务。
客户端匹配条件不仅可以限定到IP(或IP段)，也可以限定到项目，例如这样一条规则：
project = grpc-test-apps => host = 192.168.2.*
表示项目名称为 grpc-test-apps 的客户端只可调用 IP 为 192.168.2.* 服务器上的服务。
需要特别注意的是：IP段条件（例如192.168.2.*）中只能有一个星号(*)。星号既可以在开头，也可以在末尾，还可以在中间。
- 7.2示例  
假设存在Ca，Cb，Cc 三个客户端，Sa，Sb，Sc三个服务端：
 
		1.rule= =>                    结果：Ca，Cb，Cc都不能访问服务 
		2.rule=host=Ca=>              结果：Ca不能访问服务 Cb、Cc能访问服务 
		3.rule=host!=Ca=>             结果：Cb，Cc不能访问服务 Ca能访问服务 
		4.rule= =>host=Sa             结果：Ca，Cb，Cc只能访问服务Sa 
		5.rule= =>host!=Sa            结果：Ca，Cb，Cc只能访问服务Sb,Sc 
		6.rule= host=Ca => host=Sb    结果：Ca能访问Sb;Cb,Cc能访问Sa,Sb,Sc 
		7.rule=host!=Ca=>host=Sb      结果：Cb，Cc能访问Sb; Ca能访问Sa,Sb,Sc 
		8.rule=host=Cb，Cc=>host=Sb   结果：Cb,Cc能访问Sb; Ca能访问Sa,Sb,Sc 
		9.rule=host=Cb，Cc=>host!=Sb  结果：Cb,Cc能访问Sa,Sc;Ca能访问Sa,Sb，Sc 

多条路由规则工作原理： 
 
	流程如上流程图所示， 如果多条规则，grpc-c会一条一条的匹配， 对于每一条，客户端先看 =>前面的匹配条件，是不是限制本身的，不是的话，跳过；
	是的话，继续看=>后面的 服务端IP过滤条件，对provider 列表中的provider进行遍历过滤，如果是限制访问的，置黑名单标志位。
	多条规则是 “与” 的工作模式，有一条限制了访问，就不能访问了。
 
- 7.3 实现思路
使用match_when()进行客户端条件匹配，如果适用规则，客户端返回true，不适用返回false。
使用match_then()进行服务端过滤，如果适用条件，服务端被过滤掉，不适用条件，服务端不被过滤掉，然后置黑名单flag。
- 7.4相关代码
涉及到的模块与代码：

		orientsec-consumer 模块： 
		condition_router.cc
		// 按照匹配条件进行匹配，客户端适用返回true，不适用返回false
		bool condition_router::match_when(url_t* url) {
			if (when_condition.size() == 0) {
				return true;// 如果匹配条件为空，表示对所有消费方应用
		}
			return match_condition(when_condition, url, NULL);
		}

		bool condition_router::match_then(url_t* url, url_t* param) {
			return match_condition(then_condition, url, param);
		}

		bool condition_router::match_condition(std::map<std::string, match_pair> condition, url_t* url, url_t* param) {
			std::map<std::string, std::string> sample = url_to_map(url);
			std::map<std::string, std::string>::iterator samp_iter;
			std::map<std::string, match_pair>::iterator cond_iter;
			for (samp_iter = sample.begin(); samp_iter != sample.end(); samp_iter++) {
			cond_iter = condition.find(samp_iter->first);
			if (cond_iter != condition.end())
			{
				match_pair &pair = cond_iter->second;
				if (!pair.is_match(samp_iter->second, param)) {
					return false;
				}
			}
		}
		return true;
	}

## 8. 监听路由规则，获取可访问的服务列表
- 8.1 原理分析  
在客户端启动注册客户端信息时，同时注册一个监听器，用来监听路由规则。路由规则在内存中缓存一份。
当监听到路由规则发生变化时，更新内存中缓存的路由规则。路由规则发生变化后，还需要重新计算当前客户端的服务端列表，然后将最新的服务端列表缓存更新到内存中。
- 8.2 实现思路  
首先，在模块注册客户端信息时，增加注册路由规则的监听器。然后实现监听到路由规则发生变化后，对内存中路由规则、客户端可能访问的服务端列表进行更新。
每一个服务客户端，对应着一个服务名称，每一个客户端需要监听的路由规则是隶属于该服务名称的路由规则。路由规则在内存中以列表的方式存储：
`static std::map<std::string, std::vector<router*> > g_valid_routers;`  
遍历执行路由规则，按照指定服务逐条执行路由规则  
`revoker_providers_list_process()`
- 8.3 相关代码  
 涉及到的模块与代码：  

		orientsec-consumer 模块：
		新增 condition_router类
		新增 revoker_providers_list_process()
		新增 consumer_routers_callback()

## 9. 监听服务端权重信息
- 9.1原理分析  
服务端权重表示的是服务端提供服务的能力，也可以简单地理解为服务端所在服务器的配置。服务端的权重越高，表示服务器提供服务的能力越强。
如果配置的负载均衡算法为“加权轮询算法”，那么各服务端被调用次数的比例等于服务端权重的比例。
- 9.2实现思路  
提供同一个服务的服务端可能有多个，因此一个客户端内存中存储的服务端权重信息在一个集合数据类型中。
当监听到注册中心上服务端权重参数配置值发生改变，客户端将内存中的服务权重更新为修改后的参数值；当监听到注册中心上服务端权重配置信息被删除后，查询服务端初始的权重，将内存中的服务权重恢复到初始配置。
- 9.3相关代码  
涉及到的模块与代码：

		orientsec-common模块：
		新增 init_provider(provider_t* provider)
		orientsec-consumer 模块：
		新增 consumer_configurators_callback()

## 10. 监听服务端是否过时
- 10.1原理分析
缺省情况下，服务端过时标志参数值为false。如果客户端就检测到服务端的过时标志被设置为true，那么客户端调用该服务端时会记录一条警告类型的日志信息。
- 10.2实现思路
当监听到注册中心上服务端过时标志参数配置值发生改变，客户端将内存中的服务端过时标志更新为修改后的参数值；当监听到注册中心上服务端过时标志配置信息被删除后，查询服务端初始的过时标志，将内存中的服务端过时标志恢复到初始配置。
- 10.3相关代码  
涉及到的模块与代码:

		orientsec-consumer 模块：
		新增 consumer_check_provider_deprecated()

## 11. 客户端监听配置信息的更新
- 11.1 原理分析  
客户端需要监听的配置信息有以下几种：  
（1）客户端指定的服务端版本号 service.version
客户端指定了服务端版本号之后，程序会优先选择具有指定版本的服务端；如果注册中心没有该版本的服务端，则不限制版本重新选择服务提供者。(使用场景：灰度发布、A/B测试)  
（2）客户端对服务端的每秒钟的请求次数参数 consumer.default.requests
用来限制客户端对服务端的请求频率。
- 11.2 实现思路  
当监听到服务端版本号参数值发生变化后，将新的服务端版本号更新到内存中，同时根据服务端版本号重选服务端；当监听到服务端版本号配置信息被删除后，将服务端版本号恢复为默认值（默认值为：空字符串），同时根据服务端版本号重选服务端。
当监听到客户端对服务端的每秒钟的请求次数参数值发生变化后，将新的每秒钟的请求次数参数更新到内存中；当监听到每秒钟的请求次数参数配置信息被删除后，将客户端对服务端的每秒钟的请求次数参数恢复为默认值（默认值为：0,即不限制）。
- 11.3 相关代码  
涉及到的模块与代码：

		orientsec-consumer 模块：
		新增 orientsec_grpc_consumer_control_requests.cc
		新增 orientsec_grpc_consumer_control_version.cc

## 12. 限制客户端对某个服务每秒钟的请求次数（Requests Per Second）
- 12.1原理分析  
增加对客户端请求数控制的功能，限制客户端对某个服务每秒钟的请求次数（Requests Per Second）。
对客户端增加请求数控制的功能，通过在注册中心上动态增加参数配置。（不支持通过在本地文件配置，因为一个系统可能需要调用多个服务端）
为了和服务端区分，将参数的名称定为consumer.default.requests。程序中新增对注册中心上的consumer.default.requests参数进行监听，缺省情况下，不限制客户端对某个服务的请求次数。
- 12.2实现思路  
客户端每次发起调用时总会调用 ClientCalls#startCall 方法，因此可以在这个方法中增加客户端每秒请求数的控制。
为了能够获取搭配客户端本次调用的方法属于哪个服务端，需要在抽象类 ClientCall 增加一个 getFullMethod 的方法，根据这个完成方法名，可以提取出服务端的服务名称。既然在抽象类中增加了方法，需要将继承该抽象类的子类也增加上 getFullMethod 的实现。
客户端每秒钟的请求次数的控制策略如下：
a. 首先判断当前客户端是否配置了每秒钟请求次数参数值，如果没有配置，直接退出流量控制代码段
b. 如果客户端配置了参数，取出每秒钟请求次数参数值，如果参数值小于或者等于0，直接退出流量控制代码段
c. 取出调用当前服务的客户端在一秒钟内的调用次数，判断调用次数是否达到每秒钟请求次数参数值，如果达到则拦截本次调用，给客户端返回一个异常信息；如果未达到每秒钟请求次数参数值，将客户端在一秒钟内的调用次数的计算增加1。这里的调用次数计数器使用的是带有并发控制的 AtomicLong 数据结构。

- 12.3相关代码  
涉及到的模块与代码：

		Orientsec-consumer：
		添加orientsec_grpc_consumer_control_requests.cc
		grpc-core 模块：
		修改 client_unary_call.h 和 async_unary_call.h
		调用 orientsec_grpc_consumer_control_requests

## 13. 两种负载均衡模式
- 13.1 原理分析  
支持两种模式：一种是“请求负载均衡”，另一种是“连接负载均衡”。
“请求负载均衡”指的是每次调用服务端都调用负载均衡算法选择一个服务端。“连接负载均衡”指的是，创建通道(Channel)后第一次调用选择服务端之后，一直复用与之前已选定的服务端建立的连接。
默认情况下为“连接负载均衡”。
- 13.2 实现思路  
客户端每次调用服务端时，grpc-c中都会调用 start_pick_locked 方法选择subchannel，及选择服务端发送数据，首先判断当前客户端的负载均衡模式，原生的流程是根据DNS等域名解析器解析出提供服务的endpoint（IP+port），然后根据这些endpoint创建一一对应的subchannel，根据原生的算法进行挑选合适的服务提供端进行调用，并传输数据。如果负载均衡模式为“请求负载均衡”，先调用负载均衡算法重新选择服务端，然后再继续原来的流程。
## 14. 四种负载均衡算法  
- 14.1原理分析  
框架支持以下四种负载均衡算法：  
(1) 随机算法 pick_first  
实现原理：数据集合下标随机数  
(2) 轮询算法 round_robin  
实现原理：数据集合下标加1，取余运算  
(3) 加权轮询算法 weight_round_robin  
实现原理：采用ngix的平滑加权轮询算法。  
(4) 一致性Hash算法 consistent_hash  
实现原理：采用MD5算法来将对应的key哈希到一个具有2^32次方个桶的空间中，即0~(2^32)-1的数字空间中。同时，引入虚拟机器节点，解决数据分配不均衡的问题。  
- 14.2实现思路  
  - 14.2.1 pick_first 随机数算法  
顾名思义，就是在选取后端服务器的时候，采用随机的一个方法。随机算法是最常用的算法，绝大多数情况都使用他。首先，从概率上讲，它能保证我们的请求基本是分散的，从而达到我们想要的均衡效果；其次，他又是无状态的，不需要维持上一次的选择状态，也不需要均衡因子等等。总体上，方便实惠又好用。  
算法细节：

			int pickfirst_lb::choose_subchannel(const char* sn, provider_t *provider, const int*nums) {
		  	if (!sn) {
		   	 return 0;
		  	}
		  	if (provider == NULL || *nums == 0) {
		  	return 0;
			}
			int size = *nums;
			srand(time(NULL));//设置随机数种子。
			int index = rand() % size;
			return index;
			}

配置文件：

	consumer.loadbalance.mode=request
	consumer.default.loadbalance=pick_first
 
- 14.2.2 round_robin 轮询算法  
轮询调度算法的原理是：每一次把来自用户的请求轮流分配给内部中的服务器。如：从1开始，一直到N(其中，N是内部服务器的总个数)，然后重新开始循环。  
算法细节：

		int round_robin_lb::choose_subchannel(const char* sn, provider_t *provider, const int*nums) {//sn="service_name"
  		if (!sn) {
    	return 0;
  		}
  		if (provider == NULL || *nums == 0) {
    	return 0;
		}
		int index = -1;
		  int size = *nums;
		  index = subchannlecursors;  //初始游标
		  index = (index + 1) % size;  //索引+1
		  subchannlecursors++;
		  if (subchannlecursors > *nums - 1)   
 	   	subchannlecursors = 0;
	  	return  index;
		}

配置文件：
	consumer.loadbalance.mode=request
	consumer.default.loadbalance=round_robin 
 
- 14.2.3 weight_round_robin  带权重的轮询算法  
该算法中，每个机器接受的连接数量是按权重比例分配的。这是对普通轮询算法的改进，比如你可以设定：第三台机器的处理能力是第一台机器的两倍，那么负载均衡器会把两倍的连接数量分配给第3台机器。  
算法细节：

		int weight_round_robin_lb::choose_subchannel(const char* sn, provider_t *provider, const int*nums) {
			if (!sn) {
				return 0;
			}
			if (provider == NULL || *nums == 0) {
				return 0;
			}
			int i;
			int index = -1;
			int total = 0;

			for (i = 0; i < *nums; i++)
			{
				provider[i].curr_weight += provider[i].weight;
				total += provider[i].weight;
				if (index == -1 || provider[index].curr_weight < provider[i].curr_weight)
				{
					index = i;
				}
			}
			provider[index].curr_weight -= total;
			return index;
		}

配置文件：

	consumer.loadbalance.mode=request
	consumer.default.loadbalance=weight_round_robin
	//可选,类型int,缺省值100,说明:服务provider权重，是服务provider的容量，在负载均衡基于权重的选择算法中用到
	provider.weight= 400 

Note： 根据经验或者服务器性能对所有服务器进行权重估算，处理能力越强，权重（黑体加粗数值）越大。算法会根据配置的权重比进行任务分配，权重越大，被调用的次数越多。 
- 14.2.4 consistent_hash 一致性hash算法
一致性hash, 相同参数的请求总是发到同一个提供者，当某一台提供者挂掉时，本来发往该提供者的要求，基于虚拟节点，平摊到其它服务提供者，不会引发剧烈变动。
 算法细节:

		//choose provider based on value of hash "arg"
		int conistent_hash_lb::choose_subchannel(const char * sn, provider_t * provider, const int * nums, const std::string &arg)
		{
		if (!sn) {
			return 0;
		}
		if (provider == NULL || *nums == 0) {
			return 0;
		}
		// 得到服务列表
		int index = 0, index_valid = 0;
		int size = *nums;
		std::string factor;
		//bool rebuild_tree;
		//used for returning of provider node
		cnode_s * node;

		if (arg.empty())
			factor = sn;
		else
			factor = arg;

		//判断服务列表是否发生变化，如果发生变化，需要重新构造红黑树
		if (get_rebuild_flag() || !compare_provider_list(provider, size))  //两次不一样返回false
		{
			// rebuild hash tree
			gen_hash_rbtree(provider, size);
			// rebuild provider_list for comparing
			gen_provider_list(provider, nums);
			set_rebuild_flag(false);
		}

		//根据参数值选择指定的服务提供者
		node = conhash->lookup_node_s(factor.c_str());
		if (node != NULL)
		{
			if (prov_list.size() == 0)
				return index_valid;

			std::map<int, std::string>::iterator provider_iter = prov_list.begin();
			while (provider_iter != prov_list.end())  //
			{
				if (strcmp(node->get_iden(), provider_iter->second.c_str()) == 0)
				{
					index_valid = index;
					break;
	
				}
				provider_iter++;
				index++;
			}
		}
		else
		{
			std::cout << "not find corresponding provider" << std::endl;
		}
		return index_valid;
		}

		void conistent_hash_lb::reset_cursor(const char * sn)
		{
			std::map<std::string, int>::iterator iter = cursors.find(sn);
			if (iter == cursors.end())
			{
				cursors.insert(std::pair<std::string, int>(sn, -1));
				return;
			}
			iter->second = -1;
		}

配置文件：

	consumer.loadbalance.mode=request
	consumer.default.loadbalance=consistent_hash

	consumer.consistent.hash.arguments=name,no


- 14.3相关代码  
涉及到的模块与代码：

		orientsec-consumer module：
		consistent_hash.cc
		consistent_hash_lb.cc
		orientsec_consumer_intf.cc
		md5.cc
		pickfirst_lb.cc
		round_robin_lb.cc
		weight_round_robin_lb.cc
		grpc++/grpc core:
		src\core\ext\filters\client_channel\client_channel.cc
		include\grpcpp\impl\codegen\client_unary_call.h
		include\grpcpp\impl\codegen\call_op_set.h
		include\grpcpp\impl\codegen\proto_utils.h
		include\grpc\grpc.h
		src\core\lib\surface\call.cc
		src\core\ext\filters\client_channel\lb_policy\round_robin\round_robin.cc
		src\core\lib\iomgr\zk_resolve_address.c


## 15. grpc断线重连指数退避算法支持参数配置功能
- 15.1原理分析  
当grpc连接到服务端发生失败时，通常希望不要立即重试(以避免泛滥的网络流量或大量的服务请求)，而是做某种形式的指数退避算法。  
相关参数：  
(1)INITIAL_BACKOFF (第一次失败重试前等待的时间)  
(2)MAX_BACKOFF (失败重试等待时间上限)  
(3)MULTIPLIER (下一次失败重试等待时间乘以的倍数)  
(4)JITTER (随机抖动因子)  
其中MAX_BACKOFF的值为120，单位秒，参数值目前是直接“硬编码”在框架中的，为了优化系统性能，支持不同的义务系统配置不同的参数值，将该参数的取值修改为可配置的。  
- 15.2实现思路  
在配置文件“dfzq-grpc-config.properties”增加如下参数，修改程序增加对这些配置参数的读取。在框架调用指数退避算法时，参数值优先使用配置文件中的数值：  

`consumer.backoff.maxsecond =120`

- 15.3相关代码
涉及到的模块与代码：

		orientsec-consumer 模块：

		新增 failover_utils.cc文件
		新增类 failover_utils{}

## 16. 服务容错
- 16.1 原理分析

配置服务调用出错后自动重试次数后，可以启用服务容错功能，当调用某个服务端出错后，框架自动尝试切换到提供相同服务的服务端再次发起请求。

调用某个服务端，如果连续5次请求出错，自动切换到提供相同服务的新服务端。（5这个数值支持配置）

调用某个服务端，如果连续5次请求出错，如果此时没有其他服务端，增加一个惩罚连接时间(例如60s)。

- 16.2实现思路

定义一个客户端调用服务端出现错误的数据集合：

    /**
    * 各个【客户端对应服务提供者】服务调用失败次数
    * key值为：consumerId@IP:port
    * value值为: 失败次数 
    * 其中consumerId指的是客户端在zk上注册的URL的字符串形式，@是分隔符，IP:port指的是服务提供者的IP和端口
    */
    ConcurrentHashMap<String, AtomicInteger> requestFailures = new ConcurrentHashMap<>();

当客户端调用服务端抛出异常时，将错误次数累加到以上的数据集合中。当客户端调用同一个服务端失败达到5次时，进行以下处理：

如果服务端个数大于1，将出错的服务端从客户端内存中的服务端候选列表中移除，然后重新选择一个服务端；

如果服务端个数为1，先记录一下当前的时间，然后出错的服务端从客户端内存中的服务端候选列表中移除。

如果服务端个数为0，但是注册中心上服务端个数大于0，并且当前时间与从内存中删除服务端的时间差大于惩罚时间时，将注册中心上服务端列表更新到客户端内存中，然后调用负载均衡算法重新选择服务端。

- 16.3相关代码

涉及到的模块与代码：
	
	orientsec-consumer 模块：
	新增 failover_utils类
    新增 record_provider_failure()方法
	修改 BlockingUnaryCallImpl()方法嵌入调用接口

### 17. 支持失败服务恢复到服务端列表时间自定义配置

- 17.1原理分析

调用某个服务端，如果连续出错5次（5次内有一次调用成功，会重置失败次数，以达到连续的效果；5这个数值支持配置），会把该服务从服务端列表中摘除该服务端节点，通过FATAL ERROR信息的日志记录服务调用失败的相关情况；被移除的服务在10分钟后（时间支持配置），自动恢复到服务端列表中。

- 17.2实现思路

在配置文件，增加服务恢复时间的配置

	# 可选,类型integer,缺省值5,说明：连续多少次请求出错，自动切换到提供相同服务的新服务器
	# consumer.switchover.threshold=5
	
	# 可选，类型int，说明：服务端节点调用失败被移除请求列表后，经过多长时间将该服务端节点重新添加回服务端候选列表
	# 单位毫秒，默认值600000，即600秒，即10分钟
	# consumer.service.recoveryMilliseconds=600000


服务调用失败时，比较当前失败服务的调用次数，如果服务端失败达到5次时，进行以下处理：

（1）将该服务从服务端列表中移除，并通过FATAL ERROR信息的日志进行输出；

（2）通过一个延迟执行的线程，在10分钟后，将该服务恢复到服务端列表中；

（3）重置该服务的失败次数，并重选服务提供者。

- 17.3相关代码

涉及到的模块与代码：

	orientsec-grpc-core 模块：
	修改 com.orientsec.grpc.consumer.ErrorNumberUtil#recordInvokeInfo
	修改 com.orientsec.grpc.consumer.ErrorNumberUtil#removeCurrentProvider
	新增 com.orientsec.grpc.consumer.ErrorNumberUtil#resetFailTimes


### 18. 服务调用出错后支持自动重试

- 18.1原理分析

当服务调用出错时，可通过配置的重试次数进行重试，调用重试次数的配置支持到服务级别以及服务方法级别；重试次数配置优先级如下：方法级别 > 服务级别 > 默认重试配置

- 18.2实现思路

在配置文件，增加服务调用重试次数的相关配置，具体如下：

	
	# 可选,类型int,缺省值0,0表示不进行重试,说明:服务调用出错后自动重试次数
	# consumer.default.retries=0
	
	# 可选,类型int,说明:指定服务名称的服务调用出错后,自动重试次数,[]中配置指定的服务名称
	# consumer.default.retries[helloworld.Greeter]=0
	
	# 可选,类型int,说明:指定服务的方法调用出错后,自动重试次数,[]中配置指定服务名称及方法名
	# 最小可到指定到方法名
	# consumer.default.retries[helloworld.Greeter.sayHello]=0


当某一服务在调用出错时，框架会进行调用重试，重试的次数根据配置来确定。在进行重试时，会根据当前出错服务的方法、服务名、默认配置来选择重试次数；获取重试次数的优先级：方法级别 > 服务级别 > 默认重试配置，确认重试次数后，会进行服务调用重试。

例：当前服务名：helloworld.Greeter，方法名为sayHello。当sayHello方法调用出错时，优先从配置文件获取consumer.default.retries[helloworld.Greeter.sayHello]属性值作为重试次数进行调用重试；如果未配置，则获取consumer.default.retries[helloworld.Greeter]属性值，若该属性也未配置，则取consumer.default.retries的配置作为重试次数。

- 18.3相关代码

涉及到的模块与代码：

	orientsec-consumer 模块：
	修改 BlockingUnaryCall() 判断调用结果，失败的话重新调用

   
# 系统组件
---
## 1. HTTP/2
- 1.1 概念  
•	消息(Message)：由一个或多个帧组合而成，例如请求和响应;  
•	流(Stream)：存在于连接中的一个虚拟通道，流可以承载双向消息，每个流都有一个唯一的整数ID；  
•	帧(Frame)：HTTP/2通信的最小单位，每个帧包含帧首部，至少也会标识出当前帧所属的流;  
•	连接(Connection)：与 HTTP/1 相同，都是指对应的 TCP 连接;  
- 1.2 特征  
•	多路复用、乱序收发：可以乱序收发数据报文，不用使用单步：发1->收1 或者流水线：发1->发2->收2->收1 的流程，提高效率；  
•	Header压缩：不用花大量篇幅重复发送常用header，采用发送增量的方法，由客户端和服务器端共同维护一个字典；  
•	stream优先级：可以在一个连接上，为不同stream设置不同优先级；  
•	服务器推送：提前发送需要的资源；  
## 2. Grpc-c 工作流程
- 2.1 发送流程   
•	解析地址：client消息发送给gRpc，然后resolver解析域名，并获取到目标服务器地址列表；  
•	负载均衡：客户端基于负载均衡算法，从连接服务器列表中找出一个目标服务器；  
•	连接：如果到目标服务器已有连接，则使用已有连接，访问目标服务器；如果没有可用连接，则创建HTTP/2连接；  
•	编码：对请求消息使用 Protobuf做序列化，通过 HTTP/2 Stream 发送给 gRPC 服务端；  
- 2.2 接收流程   
•	编码：接收到服务端响应之后，使用Protobuf 做反序列化；  
•	回调：回调 GrpcFuture 的 set(Response) 方法，唤醒阻塞的客户端调用线程，获取 RPC 响应。  
- 2.3 流程示意图  
 ![HTTP/2 workflow](https://raw.githubusercontent.com/grpc-nebula/grpc-nebula/master/images/index.png "HTTP/2 workflow")

## 3. 支持Zookeeper开启ACL
- 3.1原理分析  
 zookeeper是一个高效的分布式协调服务，他暴露了一些共用服务，比如命名、配置、管理、同步控制、群组服务等我们可以使用zk来实现比如打成共识，集群管理、leader选举等。  
 什么是ACL，Zookeeper作为一个分布式协调框架，其内部存储的都是一些关乎分布式系统运行时状态的元数据，尤其是设计到了一些分布式锁，Master选举和协调应用场景，有效的保障Zookeeper中的数据安全，ZookeeperZK提供了三种模式，权限模式，授权对象，权限权限模式：Scheme 开发人员最多使用的如下四种权限模式
  - IP: ip模式通过ip地址，来进行权限控制
  - Digest： digest是最常用的权限控制模式，zk会对形成的权限标识先后进行两次编码处理，分别是SHA-1加密        算法、BASE64编码。
  - World：World是一值最开放的权限控制模式、这种模式可以看做为特殊的Digest，他仅仅是一个标识而已
  - Super：超级用户模式，在超级用户模式下可以ZK进行操作  
 权限： 权限就是指那些通过权限检测后可以允许执行的操作，在ZK中，对数据的操作权限分为以下五大类:
 CREATE、Delete、READ、WRITE、ADMIN
permission: zookeeper目前支持下面一些权限：
  - CREATE(c): 创建权限，可以在在当前node下创建child node
  - DELETE(d): 删除权限，可以删除当前的node
  - READ(r): 读权限，可以获取当前node的数据，可以list当前node所有的child nodes
  - WRITE(w): 写权限，可以向当前node写数据
  - ADMIN(a): 管理权限，可以设置当前node的permission

综合考虑，digest权限控制方案比较适合grpc框架，因此采用这种方案进行访问控制。
 - 3.2实现思路
首先，在配置文件中增加zookeeper访问控制用户和密码的配置项。

		zookeeper.acl.username=admin

		zookeeper.acl.password=9b579c35ca6cc74230f1eed29064d10a
如果配置了zookeeper访问控制用户名和密码，那么在创建Zookeeper Client时，增加ACL验证数据。即客户端和服务端访问zookeeper时，需要进行ACL验证。验证失败的情况，无法正常访问服务。

- 3.3 ACL 使用场景  
开发、测试环境分离，开发者无权操作测试库节点
生产环境控制指定 ip 的服务可以访问相关节点，防止混乱

- 3.4 相关代码  
涉及到的模块与代码：

		orientsec_registry 模块：
		新增 base64.c,des.c,sha1.c
		修改 zk_registry_service.c中zk_create_node() 方法，增加zoo_add_auth 验证，使用ZOO_CREATOR_ALL_ACL 设置ACL，后调用zoo_create()创建node.
		修改zk_registry_service.c中start_zk_connect()方法，获得配置文件中的zk的认证的用户名和密码，并判断是否开启ACL验证。
		添加一些static函数进行内部处理：
		get_acl_info()
		combine_name_pwd()
		encrypt()
		get_acl_param()

## 4. 主备切换

- 4.1   使用场景

多个服务端提供服务的时候，能够区分主服务器和备服务器。当主服务器可用时客户端只能调用主服务器，不能调用备服务器；当所有主服务器不可用时，客户端自动切换到备服务器进行服务调用；当主服务器恢复时，客户端自动切换到主服务器进行服务调用。

- 4.2  实现思路

给服务端添加一个master属性，用来标识服务端是主服务器还是备服务器，master等于true表示主服务器，master等于false表示备服务器，master缺省时为true。

当客户端启动时，首先根据服务名获取所有的服务端列表，然后根据每个服务端的master属性进行筛选操作：

(1) 当服务端列表中全部都是主服务器的时候，服务端列表不发生变化

(2) 当服务端列表中全部都是备服务器的时候，服务端列表不发生变化

(3) 当服务端列表中既有主服务器也有备服务器的时候，将备服务器从服务列表中移除出去，只保留主服务器

同时，客户端监听注册中心中服务端主备属性的变化，一旦监听到变化，重新获取服务端列表，并进行以上筛选操作。

- 4.3 相关代码

涉及到的模块和代码:

    orientsec-provider 模块：
	增加provider的master属性

	orientsec-consumer 模块：
	修改 consumer_query_providers 函数
	增加 master 和 online 属性的判断和解析

## 5. 支持优先级的服务分组

- 5.1 使用场景

 - 场景1：服务分组。当服务集群非常大时，客户端不必与每个服务节点建立连接，通过对服务分组，一个客户端只与一个服务组连接。

 - 场景2：业务隔离。例如服务端部署在三台服务器上，分别提供给三种业务的客户端。该场景下，可以将三台服务器配置不同的分组，然后不同业务的客户端配置各自的服务端分组。这样即使其中一种业务的客户端调用频繁导致服务端响应时间边长，也不会影响其它两种业务的客户端。

 - 场景3：多机房支持。例如某证券公司在上海有两个机房A和B，在深圳有一个机房C，三个机房都对外提供服务；每个机房都划分为两个分组，即三个机房共有6个分组，分别为A1、A2、B1、B2、C1、C2。在上海地区的客户端要调用证券公司的服务时，可以优先调用A1、A2两个分组的服务端；如果A1、A2分组的服务端不可用，调用B1、B2两个分组的服务端；如果B1、B2分组的服务端也不可用，调用C1、C2两个分组的服务端。

- 5.2 实现思路

服务端添加一个group属性，用来标识服务端的服务分组，group缺省值为空，表示没有服务分组。客户端也添加一个group属性，用来标识当前客户端可以调用的服务端分组。

当客户端启动时，首先根据服务名获取所有的服务端列表，然后根据客户端的group属性和每个服务端的group属性，对服务端列表进行筛选操作：

(1) 当客户端group属性为空的时候，服务列表不发生变化

(2) 当客户端group属性不为空的时候，首先获取高优先级分组的服务端，如果获取不到，再获取优先级低的服务端。只要某个优先级分组的服务端获取到，就将获取到的服务端作为客户端的服务端列表。如果所有的优先级的分组服务端都没有获取到，客户端报错，提示找不到服务端。

同时，客户端监听注册中心中服务端和客户端分组的变化，一旦监听到变化，重新获取服务端列表，并进行以上筛选操作。
客户端与服务端允许对指定服务名的分组进行单独配置，配置项如下所示：

	# 客户端consumer 在中括号[]中配置指定服务的服务名
	consumer.invoke.group[helloworld.Greeter]=A1
	
	# 服务端provider 在中括号[]中配置指定服务的服务名
	provider.group[helloworld.Greeter]=B1


指定服务名的配置方式优先级高于未指定服务名的配置方式。

例：服务名为A的服务进行注册时，如果同时配置了group与group[A]两个属性，优先取group[A]的属性值作为服务的分组信息，同时如果有服务名为B的服务进行注册时，因为没有配置group[B]这个属性，所以会取group的属性值作为服务的分组信息。

- 5.3 相关代码

涉及到的模块和代码:

	orientsec-consumer 模块：  
	新增 orientsec_grpc_consumer_control_group.cc
	修改 consumer_query_providers 函数
	增加group属性的比对和解析


## 6. 实现系统内部grpc服务与系统外部grpc服务的区分
- 6.1使用场景

支持同一项目不同类型的grpc服务具有不同的可见性。

项目中可能会包括两类grpc服务，对于内部项目组件间grpc调用服务，此类服务并不对外暴露，因此应该避免外部项目可见；对于项目对外提供的grpc服务则需要允许外部系统可见。


- 6.2实现思路

公共注册中心参数配置包括：注册中心集群地址、注册根路径、digest模式ACL的用户名、digest模式ACL的密码。参数名称如下：

	zookeeper.host.server (zookeeper.host.server和zookeeper.private.host.server至少配置一个参数)
	common.root           (可选参数，默认值/Application/grpc)
	zookeeper.acl.username(可选参数)
	zookeeper.acl.password(可选参数)

私有注册中心参数配置包括：注册中心集群地址、注册根路径、digest模式ACL的用户名、digest模式ACL的密码。参数名称如下：

	zookeeper.private.host.server (zookeeper.host.server和zookeeper.private.host.server至少配置一个参数)
	zookeeper.private.root        (可选参数，默认值/Application/grpc)
	zookeeper.private.acl.username(可选参数)
	zookeeper.private.acl.password(可选参数)

如果服务端所有的服务都是公共服务(外部服务)，只需要配置“公共注册中心参数”。

如果服务端所有的服务都是私有服务(内部服务)，只需要配置“私有注册中心参数”。
 
如果服务端同时存在公共服务、私有服务，“公共注册中心参数”和“私有注册中心参数”都需要配置。


如果客户端只调用公共服务(外部服务)，只需要配置“公共注册中心参数”。

如果客户端只调用私有服务(内部服务)，只需要配置“私有注册中心参数”。

如果客户端同时调用公共服务、私有服务，“公共注册中心参数”和“私有注册中心参数”都需要配置。


如果系统存在私有服务(内部服务)，需要配置哪些服务属于私有服务、哪些服务属于公共服务。

参数`public.service.list`表示公共服务名称列表，多个服务名称之间以英文逗号分隔。该参数可选，如果不配置，表示所有服务都是公共服务。

参数`private.service.list`表示私有服务名称列表，多个服务名称之间以英文逗号分隔。该参数可选，如果不配置，将公共服务名称列表之外的服务都视为私有服务。

公共服务向公共注册中心上注册，私有服务向私有注册中心上注册。

给服务端增加一个服务类型(`service.type`)（公共/私有）的属性，服务类型根据服务所在的注册中心来判断，在公共注册中心上的服务为共有服务，在私有注册中心上的服务为私有服务。

- 注册中心管理方案

公共注册中心集群、私有注册中心集群分开管理。

公共注册中心集群服务注册路径统一为`/Application/grpc`，并且不设置访问控制权限。

私有注册中心集群服务注册路径为`/Application/grpc/private/xxx`，xxx表示应用（或开发团队）。区分内外部服务的应用首先需要向zookeeper管理员申请私有注册中心的服务注册路径。

对于私有注册中心集群，不同应用（或开发团队）申请不同的注册路径，zookeeper管理员给不同注册路径设置不同访问控制权限（digest模式）。

对于私有注册中心集群，为了方便服务治理平台管理注册中心，zookeeper管理员将服务治理平台服务器的IP地址列表配置到每个私有注册路径节点的ACL中，实现服务治理平台可以免密访问私有注册中心集群。


- 6.3相关代码

涉及到的模块和代码:

	orientsec-registry 模块：    
    修改 orientsec_grpc_registry_zk_intf_init（）
	修改 registry(url_t* url)
	新增 zk_prov_reg_init（）
    新增 zk_cons_reg_init() 


## 7. 支持注册中心断线自动重连最长时间配置

- 7.1原理分析

控制zookeeper的断线重连时间

- 7.2实现思路

配置文件中增加zookeeper断线重连最长时间配置项。

	# 可选,类型int,缺省值30,单位天,即缺省值30天,说明:ZK断线重连最长时间
	# zookeeper.retry.time=30


修改创建Zookeeper Client的代码，根据配置的重连最长时间计算重连的次数，创建重试指定次数的重试策略（RetryNTimes），在创建Zookeeper Client选用该重试策略并启用。

- 7.3相关代码

涉及到的模块与代码：

	orientsec-registry 模块：
	修改 zk_conn_watcher_g（）

## 8. 注册中心容灾、降级
- 8.1功能描述  
  容灾：注册中心不可用时服务端和客户端可以正常启动，注册中心恢复后注册信息需要自动注册到注册中心   
  降级：客户端可以通过配置文件指定服务端地址，此时即使注册中心不可用，客户端也可以访问服务端；这种情况下，注册中心即使恢复，也不会再去访问注册中心获取最新的服务列表

- 8.2实现思路        
  （1）服务端启动时，将自动向zk注册Provider信息的任务代码提取到一个新的线程   
  （2）客户端启动时，将自动向zk注册Consumer信息的任务代码提取到一个新的线程    
  （3）获取配置文件中（service.server.list）提供服务的服务器地址列表：如果不为null，将服务提供者存入allProviders、serviceProviderMap中，客户端进行服务调用（忽略注册中心）；否则客户端注册zk，获取zk的服务提供者列表进行服务调用    
  （4）**特别注意**：一旦配置service.server.list参数，客户端运行过程中，即使注册中心恢复可用，框架也不会访问注册中心。如果需要从配置中心查找服务端信息，需要注释掉该参数，并重启客户端应用。 
  
- 8.3配置方法 

在配置文件“dfzq-grpc-config.properties”增加如下配置：

	# 可选,类型string,说明：该参数用来手动指定提供服务的服务器地址列表。
	# 使用场合: 在zookeeper注册中心不可用时，通过该参数指定服务器的地址；如果有多个服务，需要配置多个参数。
	# 特别注意: 一旦配置该参数，客户端运行过程中，即使注册中心恢复可用，框架也不会访问注册中心。
	#           如果需要从配置中心查找服务端信息，需要注释掉该参数，并重启客户端应用。
	# xxx表示客户端调用的服务名称
	# service.server.list[xxx]=10.45.0.100:50051
	service.server.list[xxx]=10.45.0.100:50051,10.45.0.101:50051,10.45.0.102:50051

- 8.4相关代码

涉及到的模块和代码:


	
	新增 obtain_appointed_provider_list（）   
    新增 init_provider_from_appointed_list（）
    新增 init_provider_by_host_port（）
    修改 orientsec_grpc_consumer_register（）

## 9. 程序健壮性
当服务端与zookeeper断开连接、服务注册信息丢失后，如果客户端与服务端连接正常，那么客户端与服务端依然可以正常通信。
