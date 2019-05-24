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
 *    服务注册接口函数实现
 */

#include "orientsec_grpc_registy_intf.h"
#include "orientsec_grpc_properties_tools.h"
#include "registry_contants.h"
#include "registry_factory.h"
#include "registry_service.h"
#include <grpc/support/alloc.h>
#include <grpc/support/log.h>

static bool binit = false;
static char zk_address[ORIENTSEC_GRPC_PROPERTY_VALUE_MAX_LEN] = {0};
static registry_service_t *g_zk_registry_service = NULL;

void orientsec_grpc_registry_zk_intf_init() {
	char *pData = NULL;
	int data_len = 0;
	registry_factory_t *factory = NULL;
	if (!binit)
	{
		if (orientsec_grpc_properties_init() < 0) {
			gpr_log(GPR_ERROR, "load properties failed,exit");
			exit(1);
		}
		//data_len = strlen(ORIENTSEC_GRPC_REGISTRY_DEFAULT_PROTO_HEADER) + ORIENTSEC_GRPC_PROPERTY_VALUE_MAX_LEN;
                pData = gpr_zalloc(ORIENTSEC_GRPC_PROPERTY_VALUE_MAX_LEN);
		if (0 != orientsec_grpc_properties_get_value(ORIENTSEC_GRPC_ZK_HOSTS, NULL, pData))
		{
			gpr_log(GPR_ERROR, "miss property %s in properties %s,exit", ORIENTSEC_GRPC_ZK_HOSTS, ORIENTSEC_GRPC_PROPERTIES_FILENAME);
			exit(1);
		}
		snprintf(zk_address, strlen(ORIENTSEC_GRPC_REGISTRY_DEFAULT_PROTO_HEADER) + strlen(pData) + 1, "%s%s",
			ORIENTSEC_GRPC_REGISTRY_DEFAULT_PROTO_HEADER, pData);
		factory = lookup_registry_factory(zk_address);
		g_zk_registry_service = factory->get_registry_service(zk_address);
		if (g_zk_registry_service != NULL)
		{
			binit = true;
		}
	}
	//add by yang, fix the memory leak during zk registry 
	free(pData);
}


void registry(url_t *url) {
	orientsec_grpc_registry_zk_intf_init();
	if (!g_zk_registry_service)
	{
		gpr_log(GPR_ERROR, "call registry failed for intf init failed");
		return;
	}
	registry_service_args_t args;
	args.param = g_zk_registry_service;
	g_zk_registry_service->registe(&args, url);
}
void unregistry(url_t *url) {
	orientsec_grpc_registry_zk_intf_init();
	if (!g_zk_registry_service)
	{
		gpr_log(GPR_ERROR, "call registry failed for intf init failed");
		return;
	}
	registry_service_args_t args;
	args.param = g_zk_registry_service;
	g_zk_registry_service->unregiste(&args, url);
}

void subscribe(url_t *url, registry_notify_f notify_f) {
	orientsec_grpc_registry_zk_intf_init();
	if (!g_zk_registry_service)
	{
		gpr_log(GPR_ERROR, "call subscribe failed for intf init failed");
		return;
	}
	registry_service_args_t args;
	args.param = g_zk_registry_service;
	g_zk_registry_service->subscribe(&args, url, notify_f);
}

void unsubscribe(url_t *url, registry_notify_f notify_f) {
	orientsec_grpc_registry_zk_intf_init();
	if (!g_zk_registry_service)
	{
		gpr_log(GPR_ERROR, "call unsubscribe failed for intf init failed");
		return;
	}
	registry_service_args_t args;
	args.param = g_zk_registry_service;
	g_zk_registry_service->unsubscribe(&args, url, notify_f);
}

url_t* lookup(url_t *url, int *nums) {
	url_t *ret = NULL;
	orientsec_grpc_registry_zk_intf_init();
	if (!g_zk_registry_service)
	{
		gpr_log(GPR_ERROR, "call lookup failed for intf init failed");
		return NULL;
	}
	registry_service_args_t args;
	args.param = g_zk_registry_service;
	g_zk_registry_service->lookup(&args, url, &ret, nums);
	return ret;
}

char* getData(const char *path) {
	orientsec_grpc_registry_zk_intf_init();
	if (!g_zk_registry_service)
	{
		gpr_log(GPR_ERROR, "call getData(%s) failed for intf init failed", path);
		return NULL;
	}
	registry_service_args_t args;
	args.param = g_zk_registry_service;
	return g_zk_registry_service->getData(&args, path);
}

void shutdown_registry() {
	orientsec_grpc_registry_zk_intf_init();
	if (!g_zk_registry_service)
	{
		gpr_log(GPR_ERROR, "call shutdown_registry failed for intf init failed");
		return;
	}
	registry_service_args_t args;
	args.param = g_zk_registry_service;
	g_zk_registry_service->stop(&args);
	g_zk_registry_service->destroy(&args);
}

