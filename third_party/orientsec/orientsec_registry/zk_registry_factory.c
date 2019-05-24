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
 *    zookeeper注册中心工厂接口函数实现
 */

#include "registry_service.h"
#include "registry_factory.h"
#include "orientsec_grpc_utils.h"
#include "registry_utils.h"
#include "zk_registry_service.h"
#include <grpc/support/log.h>

#define MAX_REGISTRY 50
#define DEFAULT_REGISTRY_PREFIX_MAX_LENGTH 32

static registry_service_t *g_all_of_the_zk_registries[MAX_REGISTRY];
static int g_number_of_zk_registries = 0;

//查找指定地址对应的zk接口
registry_service_t* zk_lookup_registry(char *address) {
	int i;
	for (i = 0; i < g_number_of_zk_registries; i++) {
		if (NULL == g_all_of_the_zk_registries[i])
		{
			continue;
		}
		if (0 == strcmp(address,
			g_all_of_the_zk_registries[i]->key)) {
			return g_all_of_the_zk_registries[i];
		}
	}
	return NULL;
}

//注册中心工厂类接口函数实现，根据地址查找注册中心接口，如不存在则新建并添加到缓存中
registry_service_t *zk_get_registry_service(char* address) {
	int len = 0;
	registry_service_args_t args;
	registry_service_t *registry = zk_lookup_registry(address);
	if (registry) {
		return registry;
	}
	registry = (registry_service_t *)malloc(sizeof(registry_service_t));
	memset(registry, 0, sizeof(registry_service_t));
	registry->start = zk_start;
	registry->registe = zk_registe;
	registry->unregiste = zk_unregiste;
	registry->subscribe = zk_subscribe;
	registry->unsubscribe = zk_unsubscribe;
	registry->lookup = zk_lookup;
	registry->getData = zk_getData;
	registry->stop = zk_stop;
	registry->destroy = zk_destroy;
	len = strlen(address);
	registry->key = (char*)malloc(len + 1);
	memset(registry->key, 0, len + 1);
	snprintf(registry->key, len + 1, "%s", address);
	args.param = registry;
	//创建时进行初始化，例如建立连接
	registry->start(&args);

	g_all_of_the_zk_registries[g_number_of_zk_registries++] = registry;
	return registry;
}

void zk_destroy_registry_service(registry_service_t *service) {
	int i = 0;
	registry_service_args_t args;
	if (service)
	{
		for (i = 0; i < g_number_of_zk_registries; i++) {
			if (service != g_all_of_the_zk_registries[i])
			{
				continue;
			}
			args.param = service;
			service->destroy(&args);
			FREE_PTR(g_all_of_the_zk_registries[i]);
			g_all_of_the_zk_registries[i] = NULL;
		}

	}
}

void zk_destroy_all_registry_service(void) {
	int i = 0;
	registry_service_args_t args;
	for (i = 0; i < g_number_of_zk_registries; i++) {
		if (NULL == g_all_of_the_zk_registries[i])
		{
			continue;
		}
		args.param = g_all_of_the_zk_registries[i];
		g_all_of_the_zk_registries[i]->destroy(&args);
		FREE_PTR(g_all_of_the_zk_registries[i]);
		g_all_of_the_zk_registries[i] = NULL;
	}
	g_number_of_zk_registries = 0;
}

static const registry_factory_t zk_registy_factory = {
	zk_get_registry_service,
	zk_destroy_registry_service,
	zk_destroy_all_registry_service,
	"zookeeper"
};

static registry_factory_t *zk_registry_factory_create() {
	return &zk_registy_factory;
}

//需要在grpc lib 初始化时被调用，向系统注册
void grpc_registry_zookeeper_init(void) {
	orientsec_grpc_register_registry_factory_type(zk_registry_factory_create());
}

void grpc_registry_zookeeper_shutdown(void) {}

