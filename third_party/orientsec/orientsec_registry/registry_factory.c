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
 *    注册中心工厂接口函数实现
 */

#include "registry_factory.h"
#include <grpc/support/log.h>

#define MAX_REGISTRY_FACTORY 10
#define DEFAULT_REGISTRY_PREFIX_MAX_LENGTH 32

static registry_factory_t *g_all_of_the_registry_factories[MAX_REGISTRY_FACTORY];
static int g_number_of_registry_factories = 0;

static char g_default_registry_factory_prefix[DEFAULT_REGISTRY_PREFIX_MAX_LENGTH] = "zookeeper://";

void orientsec_grpc_register_registry_factory_type(registry_factory_t *registry_factory) {
	int i;
	for (i = 0; i < g_number_of_registry_factories; i++) {
		//add by huyn 不重复注册zk factory
		if (strcmp(registry_factory->scheme,
			g_all_of_the_registry_factories[i]->scheme) == 0) {
			return;
		}
	}
	GPR_ASSERT(g_number_of_registry_factories != MAX_REGISTRY_FACTORY);
	g_all_of_the_registry_factories[g_number_of_registry_factories++] = registry_factory;
}

registry_factory_t* lookup_registry_factory(char *address) {
	char *p = NULL;
	int schema_len = 0;
	int i = 0;
	char scheme[DEFAULT_REGISTRY_PREFIX_MAX_LENGTH] = "zookeeper";
	if (!address)
	{
		return NULL;
	}
	p = strstr(address, "://");
	if (p)
	{
		schema_len = p - address;
		schema_len = (schema_len > DEFAULT_REGISTRY_PREFIX_MAX_LENGTH) ? DEFAULT_REGISTRY_PREFIX_MAX_LENGTH : schema_len;
		snprintf(scheme, schema_len + 1, "%s", address);
	}
	for (i = 0; i < g_number_of_registry_factories; i++) {
		if (0 == strcmp(scheme,
			g_all_of_the_registry_factories[i]->scheme)) {
			return g_all_of_the_registry_factories[i];
		}
	}
	return NULL;

}
