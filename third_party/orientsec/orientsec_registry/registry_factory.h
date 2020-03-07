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
 *    2017/05/15
 *    version 0.0.9
 *    注册中心工厂接口函数定义
 */

#pragma once
#ifndef ORIENTSEC_REGISTRY_FACTORY_H
#define ORIENTSEC_REGISTRY_FACTORY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "registry_service.h"

/**
*  使用注意事项：
*  需要在grpc\src\core\plugin_registry\grpc_unsecure_plugin_registry.c
*  文件中grpc_register_built_in_plugins函数添加插件注册调用代码，如下
*	grpc_register_plugin(grpc_registry_zookeeper_native_init,
*                       grpc_registry_zookeeper_native_shutdown);
*
*
*  使用方法：
*
*  char *zk_address = "zookeeper://192.168.1.215:2181";
*  url_t *url = url_parse("grpc://192.168.71.40:7010/cn.com.git.orientsec.sproc2grpc.service.SprocService?application=sproc2dubbo-server&cluster=failfast&grpc=1.1.0-SNAPSHOT&generic=false&interface=cn.com.git.orientsec.sproc2grpc.service.SprocService&methods=execp,exec&pid=7474&application.version=1.1-REALEASE&side=provider&timeout=30000&timestamp=1485315053879&version=1.0.0");
*  registry_factory_t *factory = lookup_registry_factory(zk_address);
*  registry_service_t *registry = factory->get_registry_service(zk_address);
*  registry->registe(url);
* 
**/


//注册中心工厂结构体
typedef struct _registry_factory {
	//根据key获取注册中心接口，如果不存在则新建
	registry_service_t *(*get_registry_service)(char*);
	//释放指定注册中心接口
	void(*destroy_service)(registry_service_t*);
	//释放所有注册中心接口
	void(*destroyAll)(void);
	//注册中心schema
	char *scheme;
}registry_factory_t;

//向系统注册指定类型的注册中心工厂结构
void orientsec_grpc_register_registry_factory_type(registry_factory_t *registry);

/**
* 根据地址查找注册中心工厂,address格式为:
     zookeeper://192.168.1.215:2181,192.168.1.216:2181
	 zookeeper://192.168.1.215:2181
	 192.168.1.215:2181
  如果address中未指明协议schema，则默认使用zookeeper注册中心工厂
**/
registry_factory_t* lookup_registry_factory(char *address);

void grpc_registry_zookeeper_init(void);

void grpc_registry_zookeeper_shutdown(void);

// 注册中心工厂类接口函数实现
registry_service_t *zk_get_registry_service(char* address);

#ifdef __cplusplus
}
#endif

#endif // !ORIENTSEC_REGISTRY_H
