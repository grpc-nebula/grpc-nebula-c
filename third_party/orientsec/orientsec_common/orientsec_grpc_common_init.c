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
 *本文件主要用于系统系统时预初始化操作
 *
 */
#include "orientsec_grpc_common_init.h"
#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>

#include "orientsec_grpc_properties_tools.h"
#include "orientsec_grpc_conf.h"
#include "orientsec_grpc_common_utils.h"
static long orientsec_grpc_consumer_maxrequest = 0;

//是否生成服务跟踪信息 1 生成 0 不生成
static int g_orientsec_grpc_common_gentrace_enabled = 1;

//是否把生成的信息写kafka 1 写 0 不写（此时gentrace.enabled生效）
static int g_orientsec_grpc_common_writekafka_enabled = 1;

//consumer端依赖服务版本
static char g_orientsec_grpc_common_consumer_service_version[1024]={0};

//服务的最大缓存数 默认值10
static int g_orientsec_grpc_cache_provider_count;

//服务根目录 默认值/Application/grpc
char* g_orientsec_grpc_common_root_directory;

//获取根目录参数
char* orientsec_grpc_common_get_root_dir() {
  return g_orientsec_grpc_common_root_directory;
}
void common_root_directory_init() {
  size_t size = sizeof(char) * 1024;
  char* readbuff = (char*)malloc(size);
  memset(readbuff, 0, size);
  orientsec_grpc_properties_get_value(ORIENTSEC_GRPC_CONF_COMMON_ROOT_DIRECTORY,
                                      NULL, readbuff);
  if (readbuff == NULL || strlen(readbuff) == 0) {
    g_orientsec_grpc_common_root_directory = ORIENTSEC_GRPC_CONF_COMMON_ROOT_DIRECTORY_DEFAULT;
  } else {
    g_orientsec_grpc_common_root_directory = readbuff;
  }
}

//获取是否生成服务跟踪信息参数
int orientsec_grpc_common_get_gentrace_enabled() {
	return g_orientsec_grpc_common_gentrace_enabled;
}

//获取是否生成服务跟踪信息参数
int orientsec_grpc_common_get_writekafka_enabled() {
	return g_orientsec_grpc_common_writekafka_enabled;
}

//获取是否生成服务跟踪信息参数
int orientsec_grpc_common_get_writekafka_enabled_set(int enable) {
	g_orientsec_grpc_common_writekafka_enabled = enable;
  return 0;
}

//获取consumer对服务版本要求
char * orientsec_grpc_consumer_service_version_get() {
	return g_orientsec_grpc_common_consumer_service_version;
}

//获取consumer允许缓存的provider数量
int orientsec_grpc_cache_provider_count_get() {
	return g_orientsec_grpc_cache_provider_count;
}

// 客户端服务版本读取配置文件初始化
void consumer_service_version_init() {
 /* size_t size = sizeof(char) * 1024;
  char *readbuff = (char*)malloc(size);*/

  //memset(readbuff, 0, size);
  orientsec_grpc_properties_get_value(
      ORIENTSEC_GRPC_CONF_CONSUMER_SERVICE_VERSION, NULL,
      g_orientsec_grpc_common_consumer_service_version);

  // init here
  orientsec_grpc_service_version_init();
  //if (readbuff == NULL || strlen(readbuff) == 0) {
  //  g_orientsec_grpc_common_consumer_service_version = NULL;
  //}
  //else {
  //  g_orientsec_grpc_common_consumer_service_version = readbuff;
  //}
  ////add by yang, free the memory for all branch
  //free(readbuff);
}

/*
* 初始化通用入口
*/
void orientsec_grpc_common_param_init() {
	//读取配置文件
	orientsec_grpc_properties_init();

	size_t size = sizeof(char) * 1024;
	char *readbuff = (char*)malloc(size);

	//是否生成服务跟踪信息
	memset(readbuff, 0, size);
	orientsec_grpc_properties_get_value(ORIENTSEC_GRPC_TRACE_GENTRACE_KEY, NULL, readbuff);
	if (readbuff == NULL || strcmp(readbuff, "") == 0 || strcmp(readbuff, "true") == 0) {
		g_orientsec_grpc_common_gentrace_enabled = ORIENTSEC_GRPC_COMMON_YES;
	}
	else {
		g_orientsec_grpc_common_gentrace_enabled = ORIENTSEC_GRPC_COMMON_NO;
	}

	//是否把服务跟踪信息发送kafka
	memset(readbuff, 0, size);
	orientsec_grpc_properties_get_value(ORIENTSEC_GRPC_TRACE_WRITEKAFKA_KEY, NULL, readbuff);
	if (readbuff == NULL || strcmp(readbuff, "") == 0 || strcmp(readbuff, "true") == 0) {
		g_orientsec_grpc_common_writekafka_enabled = ORIENTSEC_GRPC_COMMON_YES;
	}
	else {
		g_orientsec_grpc_common_writekafka_enabled = ORIENTSEC_GRPC_COMMON_NO;
	}

	memset(readbuff, 0, size);
	orientsec_grpc_properties_get_value(ORIENTSEC_GRPC_CACHE_PROVIDER_COUNT, NULL, readbuff);
	if (readbuff == NULL || strcmp(readbuff, "") == 0 || orientsec_grpc_common_utils_isdigit(readbuff) != 1) {
		g_orientsec_grpc_cache_provider_count = 5;
	}
	else {
		g_orientsec_grpc_cache_provider_count = atoi(readbuff);
	}

	free(readbuff);
	consumer_service_version_init();
        common_root_directory_init();
}




