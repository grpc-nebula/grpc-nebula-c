# 微服务治理框架(C++版)开发环境搭建与配置
# 1 系统环境  
## 1）操作系统  
`Centos7.4+ `  
OR   
`RedHat7.4+`  
## 2）编译软件  
	gcc4.8.5（支持C++11标准）   
	cmake2.8.10  
## 3）第三方软件  
	Zookeeper 3.4.13+  
# 2 开发配置文件
- 1）配置文件存放位置  
	redhat/config/  
	└── dfzq-grpc-config.properties
- 2）修改配置文件配置项  
 根据系统参数和配置项注释修改配置项（详见开发手册）
  
# 3 工具安装
## 3.1 安装cmake（已安装则忽略）

     yum install -y gcc gcc-c++ make automake                              
     yum install -y wget                                                   
     wget http://www.cmake.org/files/v2.8/cmake-2.8.10.2.tar.gz            
     tar -zxvf cmake-2.8.10.2.tar.gz                                        
     cd cmake-2.8.10.2                                                     
     ./bootstrap                                                           
     gmake                                                                 
     gmake install                                                         
   
## 3.2 安装ssl依赖包（已安装则忽略）
     yum list installed | grep ssl 查看ssl依赖包如果为空则运行下面三个指令
     yum install openssl.x86_64                                            
     yum install openssl-devel.x86_64                                      
     yum install openssl-libs.x86_64   
## 3.3 安装Zookeeper
安装Zookeeper，建议使用3.4.13或以上版本。  

	1.下载zookeeper
	2.将文件解压到/xx目录下
	3.在解压目录下创建data和log目录
	4.将解压后zookeeper-3.4.13文件夹下的zoo_sample.cfg文件拷贝一份命名为zoo.cfg
	5.修改zoo.cfg文件，添加data和log目录路径
	6.在.bash_profile文件中增加zookeeper配置
	7.使配置生效 source .bash_profile
	8.启动ZK
                                     
# 4 目录结构
进入release包下，找到redhat平台发布包，redhat目录下共分四个目录，config、demo、include、libs，其中config为程序启动时所用的配置文件，demo里为应用程序，include为运行所必须的头文件，libs为所必须的库文件。

	redhat/
	├── config
	├── demo
	├── include
	└── libs
	redhat/config/
	└── dfzq-grpc-config.properties
	redhat/demo/
	├── demo-pmtest-client
	├── demo-pmtest-client-async
	├── demo-pmtest-server
	└── demo-pmtest-server-async
	redhat/include/
	├── orientsec_grpc
	├── include
	└── third_party
	redhat/libs/
	├── lib
	├── opt
	├── protobuf
	└── zookeeper


## 4.1 依赖库配置
### 4.1.1 原生库

	-rw-r--r-- 1 root root    46770 Mar 29 10:30 libaddress_sorting.a
	lrwxrwxrwx 1 root root       27 Apr  1 11:42 libaddress_sorting.so -> libaddress_sorting.so.7.0.0
	lrwxrwxrwx 1 root root       27 Apr  1 11:42 libaddress_sorting.so.7 -> libaddress_sorting.so.7.0.0
	-rwxr-xr-x 1 root root    35501 Apr  1 11:42 libaddress_sorting.so.7.0.0
	-rw-r--r-- 1 root root   803066 Mar 29 10:30 libares.a
	-rw-r--r-- 1 root root 21130192 Mar 29 10:31 libboringssl.a
	-rw-r--r-- 1 root root   469188 Mar 29 10:30 libgpr.a
	lrwxrwxrwx 1 root root       15 Apr  1 11:42 libgpr.so -> libgpr.so.7.0.0
	lrwxrwxrwx 1 root root       15 Apr  1 11:42 libgpr.so.7 -> libgpr.so.7.0.0
	-rwxr-xr-x 1 root root   217371 Apr  1 11:42 libgpr.so.7.0.0
	-rw-r--r-- 1 root root 47655272 Mar 29 15:29 libgrpc.a
	-rw-r--r-- 1 root root 23456332 Apr  1 11:41 libgrpc++.a
	-rw-r--r-- 1 root root  2444544 Apr  1 11:41 libgrpc++_core_stats.a
	-rw-r--r-- 1 root root 42081928 Mar 29 15:29 libgrpc_cronet.a
	-rw-r--r-- 1 root root 58634318 Apr  1 11:41 libgrpc++_cronet.a
	lrwxrwxrwx 1 root root       23 Apr  1 11:42 libgrpc_cronet.so -> libgrpc_cronet.so.7.0.0
	lrwxrwxrwx 1 root root       26 Apr  1 11:42 libgrpc++_cronet.so -> libgrpc++_cronet.so.1.17.1
	lrwxrwxrwx 1 root root       26 Apr  1 11:42 libgrpc++_cronet.so.1 -> libgrpc++_cronet.so.1.17.1
	-rwxr-xr-x 1 root root 13876954 Apr  1 11:42 libgrpc++_cronet.so.1.17.1
	lrwxrwxrwx 1 root root       23 Apr  1 11:42 libgrpc_cronet.so.7 -> libgrpc_cronet.so.7.0.0
	-rwxr-xr-x 1 root root 17107898 Apr  1 11:42 libgrpc_cronet.so.7.0.0
	-rw-r--r-- 1 root root  3435770 Apr  1 11:42 libgrpc++_error_details.a
	lrwxrwxrwx 1 root root       33 Apr  1 11:42 libgrpc++_error_details.so -> libgrpc++_error_details.so.1.17.1
	lrwxrwxrwx 1 root root       33 Apr  1 11:42 libgrpc++_error_details.so.1 -> libgrpc++_error_details.so.1.17.1
	-rwxr-xr-x 1 root root 17272703 Apr  1 11:42 libgrpc++_error_details.so.1.17.1
	-rw-r--r-- 1 root root 15608012 Apr  1 11:41 libgrpc_plugin_support.a
	-rw-r--r-- 1 root root 13794246 Apr  1 11:42 libgrpcpp_channelz.a
	lrwxrwxrwx 1 root root       28 Apr  1 11:42 libgrpcpp_channelz.so -> libgrpcpp_channelz.so.1.17.1
	lrwxrwxrwx 1 root root       28 Apr  1 11:42 libgrpcpp_channelz.so.1 -> libgrpcpp_channelz.so.1.17.1
	-rwxr-xr-x 1 root root 29520984 Apr  1 11:42 libgrpcpp_channelz.so.1.17.1
	-rw-r--r-- 1 root root  7735042 Apr  1 11:42 libgrpc++_reflection.a
	lrwxrwxrwx 1 root root       30 Apr  1 11:42 libgrpc++_reflection.so -> libgrpc++_reflection.so.1.17.1
	lrwxrwxrwx 1 root root       30 Apr  1 11:42 libgrpc++_reflection.so.1 -> libgrpc++_reflection.so.1.17.1
	-rwxr-xr-x 1 root root 19282168 Apr  1 11:42 libgrpc++_reflection.so.1.17.1
	lrwxrwxrwx 1 root root       16 Apr  1 11:42 libgrpc.so -> libgrpc.so.7.0.0
	lrwxrwxrwx 1 root root       19 Apr  1 11:42 libgrpc++.so -> libgrpc++.so.1.17.1
	lrwxrwxrwx 1 root root       19 Apr  1 11:42 libgrpc++.so.1 -> libgrpc++.so.1.17.1
	-rwxr-xr-x 1 root root  8102514 Apr  1 11:42 libgrpc++.so.1.17.1
	lrwxrwxrwx 1 root root       16 Apr  1 11:42 libgrpc.so.7 -> libgrpc.so.7.0.0
	-rwxr-xr-x 1 root root 19530190 Apr  1 11:42 libgrpc.so.7.0.0
	-rw-r--r-- 1 root root 21900144 Mar 29 15:29 libgrpc_unsecure.a
	-rw-r--r-- 1 root root 21821150 Apr  1 11:42 libgrpc++_unsecure.a
	lrwxrwxrwx 1 root root       25 Apr  1 11:42 libgrpc_unsecure.so -> libgrpc_unsecure.so.7.0.0
	lrwxrwxrwx 1 root root       28 Apr  1 11:42 libgrpc++_unsecure.so -> libgrpc++_unsecure.so.1.17.1
	lrwxrwxrwx 1 root root       28 Apr  1 11:42 libgrpc++_unsecure.so.1 -> libgrpc++_unsecure.so.1.17.1
	-rwxr-xr-x 1 root root  7501766 Apr  1 11:42 libgrpc++_unsecure.so.1.17.1
	lrwxrwxrwx 1 root root       25 Apr  1 11:42 libgrpc_unsecure.so.7 -> libgrpc_unsecure.so.7.0.0
	-rwxr-xr-x 1 root root  8852400 Apr  1 11:42 libgrpc_unsecure.so.7.0.0
	drwxr-xr-x 2 root root       97 Mar 29 15:29 pkgconfig
	drwxr-xr-x 2 root root       44 Apr  1 11:39 protobuf
 
### 4.1.2 开发生成库

	redhat/libs/lib
	├── liborientsec_common.a
	├── liborientsec_consumer.a
	├── liborientsec_provider.a
	└── liborientsec_registry.a
### 4.1.3 protobuf库
	redhat/libs/protobuf  
	├── grpc_cpp_plugin  
	├── libprotobuf.a  
	└── protoc  

### 4.1.4 zookeeper库
	redhat/libs/zookeeper
	├── libhashtable.a
	├── libzkmt.a
	├── libzkst.a
	├── libzookeeper_mt.a
	├── libzookeeper_mt.so
	├── libzookeeper_mt.so.2
	├── libzookeeper_mt.so.2.0.0
	├── libzookeeper_st.a
	├── libzookeeper_st.so
	├── libzookeeper_st.so.2
	└── libzookeeper_st.so.2.0.0

## 4.2 开发目录
  在demo目录下，进行client 和 server端的应用程序开发。


## 4.3 编译与运行

### 4.3.1 编译
   本篇主要是linux下的开发，采用编写CMakeLists.txt，经cmake生成Makefile，再make编译的步骤。  
  以客户端为例，linux下C++编译命令如下：

	[root@zabbixserver demo-sync-client]# mkdir cmake
	[root@zabbixserver demo-sync-client]# cd cmake
	[root@zabbixserver cmake]# cmake ..
	[root@zabbixserver cmake]# make
	[root@zabbixserver cmake]# cd ..
	[root@zabbixserver cmake]# ls -l demo*

### 4.3.2 运行
  linux下C++运行命令如下：

	[root@zabbixserver demo-sync-client]#./demo-sync-client


