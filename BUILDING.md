gRPC Nebula C++ - 源代码编译
===========================

# 准备工作

## Linux平台

```sh
 $ [sudo] apt-get install build-essential autoconf libtool pkg-config
```

如果您计划从源代码构建，安装以下:

```sh
 $ [sudo] apt-get install libgflags-dev libgtest-dev  
 $ [sudo] apt-get install clang libc++-dev
```

## Windows平台

安装 Microsoft Visual C++ compiler build  
- Install Visual Studio 2017 


## Protoc

gRPC nebula 和原生gRPC 一样使用protobuf  [protocol buffers](https://github.com/google/protobuf),
使用 `protoc` 编译和生成 stub server 和 client 代码.

如果您使用从源代码编译的方式使用gRPC-Nebula-C，Makefile 会自动编译存放在 third_party 目录下的protoc，前提是使用了递归参数克隆了全部代码。

## Zookeeper
下载zookeeper。

### Linux平台
克隆zookeeper 和 grpc-nebula-c 同级目录，编译zookeeper 库。参考zookeeper编译文档

### windows平台
下载zookeeper到 任意目录，编译生成与开发应用（debug/release, x86/x64)相对应的版本库。


# 克隆grpc-nebula-c 代码库 (包含 submodules)

使用带递归参数的克隆

### Linux平台

```sh
 $ git clone -b $(curl -L https://grpc.io/release) https://github.com/grpc/grpc
 $ cd grpc
 $ git submodule update --init
 ```

### Windows平台

批处理模式：

```
> @rem You can also do just "git clone --recursive -b THE_BRANCH_YOU_WANT https://github.com/grpc/grpc"
> powershell git clone --recursive -b ((New-Object System.Net.WebClient).DownloadString(\"https://grpc.io/release\").Trim()) https://github.com/grpc/grpc
> cd grpc
> @rem To update submodules at later time, run "git submodule update --init"
```
git 工具模式：
同linux

# 编译源代码

参考《微服务治理框架(C++版)开发环境搭建与配置》
存放于 /docs 目录

