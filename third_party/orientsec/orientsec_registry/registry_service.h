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
 *    注册中心服务接口函数定义
 */

#ifndef ORIENTSEC_REGISTRY_SERVICE_H
#define ORIENTSEC_REGISTRY_SERVICE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "url.h"


/**
* 回调函数声明
**/
typedef void(*registry_notify_f)(url_t*, int);

typedef struct _registry_service registry_service_t;

//typedef enum { PUBLIC_CENTER = 0, PRIVATE_CENTER } zookeeper_center_property;

//接口参数类型
typedef struct _registry_service_args {
	registry_service_t *param;
}registry_service_args_t;

//注册中心接口
typedef  struct _registry_service {
	//初始化，启动连接
	void (*start)(registry_service_args_t*);
	//注册url
	void (*registe)(registry_service_args_t*, url_t*);
	//取消注册
	void (*unregiste)(registry_service_args_t*, url_t*);
	//订阅path
	void (*subscribe)(registry_service_args_t*, url_t*, registry_notify_f );
	//取消订阅path
	void (*unsubscribe)(registry_service_args_t*, url_t*, registry_notify_f );
	//查找匹配某个Url的节点
	void (*lookup)(registry_service_args_t*, url_t*, url_t**, int *);
	//读取某个路径下的数据
	char* (*getData)(registry_service_args_t*, char*);
	//断开连接
        void (*stop)(registry_service_args_t*);
	//释放接口中的数据
	void(*destroy)(registry_service_args_t*);
        // 定义注册中心的public/private 属性
        //zookeeper_center_property zk_center;
	//接口的唯一标识，例如为注册中心地址
	char *key;
	//接口的其他数据，
	void *data;
}registry_service_t,*p_registry_service_t;

#ifdef __cplusplus
}
#endif

#endif // !ORIENTSEC_REGISTRY_SERVICE_H

