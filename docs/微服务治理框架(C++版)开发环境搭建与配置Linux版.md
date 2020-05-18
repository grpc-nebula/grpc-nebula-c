# 微服务治理框架(C++版)Linux开发环境搭建与配置
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
# 4 发布包制作
   grpc-c 源代码存放/home/appadmin/下， zookeeper源代码存放在/home/appadmin/， redhat目录存放在/usr下
## 4.1 目录创建

    [root@zabbixserver usr]# mkdir -p redhat/config
	[root@zabbixserver usr]# mkdir -p redhat/demo
	[root@zabbixserver usr]# mkdir -p redhat/include
	[root@zabbixserver usr]# mkdir -p redhat/libs
	[root@zabbixserver usr]# mkdir -p redhat/include/include
	[root@zabbixserver usr]# mkdir -p redhat/include/orientsec_grpc
	[root@zabbixserver usr]# mkdir -p redhat/include/third_party
	[root@zabbixserver usr]# mkdir -p redhat/include/orientsec/orientsec_common
	[root@zabbixserver usr]# mkdir -p redhat/include/orientsec/orientsec_consumer            

## 4.2 代码编译
### 4.2.1 开发代码编译
	[root@zabbixserver orientsec]# pwd
	/home/appadmin/grpc-c/third_party/orientsec
	[root@zabbixserver orientsec]# ll
	total 20
	drwxr-xr-x 4 root root 4096 May 23 15:11 orientsec_common
	drwxr-xr-x 4 root root 4096 May 23 15:22 orientsec_consumer
	drwxr-xr-x 4 root root 4096 May 23 15:22 orientsec_provider
	drwxr-xr-x 4 root root 4096 May 23 15:12 orientsec_registry

	#make common module
	cd orientsec_common
	ls
	aclocal
	autoconf
	autoheader
	automake --add-missing
	./configure
	make
	cd ..

	#make registry module
	cd orientsec_registry
	ls
	aclocal
	autoconf
	autoheader
	automake --add-missing
	./configure
	make
	cd ..
 
	#make consumer module
	cd orientsec_consumer
	ls
	aclocal
	autoconf
	autoheader
	automake --add-missing
	./configure
	make
	cd .. 
 
	#make provider module
	cd orientsec_provider
	ls
	aclocal
	autoconf
	autoheader
	automake --add-missing
	./configure
	make
	cd .. 

### 4.2.2 zookeeper库编译
	#make zookeeper lib
	# cd zookeeper dir
	cd XXX
	chmod 755 configure
	./configure
	make

### 4.2.3 grpc-c库编译
	#make grpc-c lib
	ls
	make 

## 4.3 接口（头文件）拷贝
### 4.3.1 grpc头文件拷贝
   grpc头文件位于./include/grpc,./include/grpcpp目录下：

	#copy grpc include files to include dir
	cp -rf  /home/appadmin/grpc-c/include/grpc  /usr/redhat/include/include
	cp -rf  /home/appadmin/grpc-c/include/grpcpp  /usr/redhat/include/include

### 4.3.2 开发版头文件拷贝
   将开发版头文件拷贝到orientsec 和 orientsec\_grpc目录下, 源文件位于/home/appadmin/grpc-c/third\_party/orientsec.

	[root@zabbixserver include]# pwd
	/usr/redhat/include/
	[root@zabbixserver include]# ll
	total 4
	drwxr-xr-x 4 root root   30 May 23 16:33 include
	drwxr-xr-x 4 root root   54 Apr 28 17:42 orientsec
	drwxr-xr-x 2 root root 4096 Apr 28 17:02 orientsec_grpc
	drwxr-xr-x 3 root root   21 Apr 28 16:34 third_party
	[root@zabbixserver include]# 
	[root@zabbixserver include]# 
	[root@zabbixserver include]# cd orientsec
	[root@zabbixserver orientsec]# tree
	.
	├── orientsec_common
	│   └── orientsec_grpc_string_op.h
	└── orientsec_consumer
 	   └── orientsec_grpc_consumer_control_requests.h

	2 directories, 2 files
	[root@zabbixserver orientsec]# cd ../orientsec_grpc/
	[root@zabbixserver orientsec_grpc]# pwd
	/usr/redhat/include/orientsec_grpc
	[root@zabbixserver orientsec_grpc]# tree
	.
	├── orientsec_grpc_common_init.h
	├── orientsec_grpc_common_trace_key.h
	├── orientsec_grpc_common_utils.h
	├── orientsec_grpc_consumer_control_requests.h
	├── orientsec_grpc_properties_constants.h
	├── orientsec_grpc_properties_tools.h
	├── orientsec_grpc_string_op.h
	├── orientsec_grpc_utils.h
	└── orientsec_types.h

	0 directories, 9 files
### 4.3.3 protobuf库头文件拷贝
   拷贝 ./third\_party/protobuf/src/google/protobuf 目录到 redhat/include/third_party/protobuf/src/google/下.

	[root@zabbixserver include]# cd third_party
	[root@zabbixserver third_party]# tree
	.
	└── protobuf
	    └── src
	        └── google
	            └── protobuf
	                ├── io
	                ├── stubs
	                ├── util
                    └── ....

## 4.4 动态库和静态库拷贝

### 4.4.1 开发库拷贝

	[root@zabbixserver orientsec]# pwd
	/home/appadmin/grpc-c/third_party/orientsec
	[root@zabbixserver orientsec]# cp -f orientsec_*/lib*.a  /usr/redhat/libs/lib/

### 4.4.2 原生库拷贝

	[root@zabbixserver opt]# pwd
	/home/appadmin/grpc-c/libs/opt
	[root@zabbixserver opt]# cp -d * /usr/redhat/libs/opt/ -rf

### 4.4.3 protobuf库拷贝

	[root@zabbixserver opt]# pwd
	/home/appadmin/grpc-c/bins/opt
	[root@zabbixserver opt]# cp -f grpc_cpp_plugin /usr/redhat/libs/protobuf/
	[root@zabbixserver opt]# cd protobuf/
	[root@zabbixserver protobuf]# pwd
	/home/appadmin/grpc-c/bins/opt/protobuf
	[root@zabbixserver protobuf]# cp -f protoc /usr/redhat/libs/protobuf/

	[root@zabbixserver protobuf]# cd /home/appadmin/grpc-c/libs/opt/protobuf	
	[root@zabbixserver protobuf]# pwd
	/home/appadmin/grpc-c/libs/opt/protobuf
	[root@zabbixserver protobuf]# ll
	total 186428
	-rw-r--r-- 1 root root  79614074 May 23 16:08 libprotobuf.a
	-rw-r--r-- 1 root root 111283790 May 23 16:08 libprotoc.a
	[root@zabbixserver protobuf]# cp libprotobuf.a  /usr/redhat/libs/protobuf/

### 4.4.4 zookeeper库拷贝
	[root@zabbixserver .libs]# pwd
	/home/appadmin/zookeeper/.libs
	[root@zabbixserver .libs]# cp -f libhashtable.a /usr/redhat/libs/zookeeper/
	[root@zabbixserver .libs]# cp -f libzkmt.a /usr/redhat/libs/zookeeper/
	[root@zabbixserver .libs]# cp -f libzkst.a /usr/redhat/libs/zookeeper/
	[root@zabbixserver .libs]# cp -f libzookeeper_mt.a /usr/redhat/libs/zookeeper/
	[root@zabbixserver .libs]# cp -d libzookeeper_mt.so /usr/redhat/libs/zookeeper/ -f
	[root@zabbixserver .libs]# cp -d libzookeeper_mt.so.2 /usr/redhat/libs/zookeeper/ -f
	[root@zabbixserver .libs]# cp -f libzookeeper_mt.so.2.0.0 /usr/redhat/libs/zookeeper/
	[root@zabbixserver .libs]# cp -f libzookeeper_st.a /usr/redhat/libs/zookeeper/
	[root@zabbixserver .libs]# cp -d libzookeeper_st.so /usr/redhat/libs/zookeeper/ -f
	[root@zabbixserver .libs]# cp -d libzookeeper_st.so.2 /usr/redhat/libs/zookeeper/ -f
	[root@zabbixserver .libs]# cp -f libzookeeper_st.so.2.0.0 /usr/redhat/libs/zookeeper/

## 4.5 打包发布包


	[root@zabbixserver usr]# cd /usr
	[root@zabbixserver usr]# tar -czvf redhat.tar.gz redhat/
	[root@zabbixserver usr]# ll
	drwxr-xr-x    6 root root        55 Apr 26 10:56 redhat
	-rw-r--r--    1 root root 195736631 May 28 14:34 redhat.tar.gz

# 5 目录结构
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
	├── orientsec
	├── orientsec_grpc
	├── include
	└── third_party
	redhat/libs/
	├── lib
	├── opt
	├── protobuf
	└── zookeeper


## 5.1 依赖库配置
### 5.1.1 原生库

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
 
### 5.1.2 开发生成库

	redhat/libs/lib
	├── liborientsec_common.a
	├── liborientsec_consumer.a
	├── liborientsec_provider.a
	└── liborientsec_registry.a
### 5.1.3 protobuf库
	redhat/libs/protobuf  
	├── grpc_cpp_plugin  
	├── libprotobuf.a  
	└── protoc  

### 5.1.4 zookeeper库
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

## 5.2 开发目录
  在demo目录下，进行client 和 server端的应用程序开发。




