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
 *    Author : huyn
 *    2017/05/15
 *    version 0.0.9
 *    配置文件操作实现方法
 */

#include "orientsec_grpc_common_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grpc/support/time.h"
#include "orientsec_grpc_utils.h"
#include "orientsec_grpc_conf.h"
#include "orientsec_grpc_properties_tools.h"
#include "uuid4gen.h"

#define ORIENTSEC_GRPC_SERVICE_VERSION_MAX 1024
/*
* 本功能windows平台和linux平台实现由差异
*/
void orientsec_grpc_uuid(char *my_uuid) {
	uuid4gen(my_uuid);
}



//把整数转为对应的字符串
void orientsec_grpc_int_to_char(int inval, char * str) {
	sprintf(str, "%d", inval);
}


//用于解析zookeeper:///前缀的服务名
//例如zookeeper:///com.orientsec.aaaa.bbb.cccImpl
int orientsec_grpc_getservicename_by_target(char * target) {
	if (target == NULL) {
		return -1;
	}
	int targetlen = strlen(target);
	int schemelen = strlen(ORIENTSEC_GRPC_COMMON_ZK_SCHEME);
	if (targetlen <= schemelen) {
		return -1;
	}
	char * servicename = strrchr(target, '/');
	if (servicename == NULL) {
		return -1;
	}
	return targetlen - strlen(servicename) + 1;
}


//解析fullmethod变量内包含的服务名
//fullmethod格式为：  /com.aaa.bbb.ccc/methodname
void orientsec_grpc_getserveicename_by_fullmethod(const char* fullmethod, char* servicename) {
	if (fullmethod == NULL) {
		return;
	}
	char *strmethod = (char*)malloc(200);
	memset(strmethod, 0, 200);
	strcpy(strmethod, fullmethod);
	char *temp = strchr(strmethod, '/');
	if ((strmethod - temp) == 0) {
		temp = strchr(temp + 1, '/');
		strmethod++;
	}
	int len = temp - strmethod;
	if (len > 0) {
		memcpy(servicename, strmethod, (temp - strmethod));
	}
	return;
}

//解析fullmethod变量内包含的服务名
//fullmethod格式为：  /com.aaa.bbb.ccc/methodname
int orientsec_grpc_getserveice_by_fullmethod(const char* fullmethod, char ** ret_param) {
	if (fullmethod == NULL) {
		return NULL;
	}
	size_t len = strlen(fullmethod);
	if (len <= 0) {
		return NULL;
	}
	char *temp = strchr(fullmethod, '/');
	if ((fullmethod - temp) == 0) {
		temp = strchr(temp + 1, '/');
		fullmethod++;
	}
	len = temp - fullmethod;
	if (len <= 0) {
		return NULL;
	}
	*ret_param = (char*)malloc(len * sizeof(char) + sizeof(char));
	memset(*ret_param, 0, len * sizeof(char) + sizeof(char));
	memcpy(*ret_param, fullmethod, len);
	temp = NULL;
	fullmethod--; //恢复到头位置
	return len;
}


//解析fullmethod变量内包含的服务名
//fullmethod格式为：  /com.aaa.bbb.ccc/methodname
char * orientsec_grpc_getmethodname_by_fullmethod(const char* fullmethod) {
	if (fullmethod == NULL) {
		return NULL;
	}
	size_t len = strlen(fullmethod);
	if (len <= 0) {
		return NULL;
	}
	char *temp = strrchr(fullmethod, '/');
	if ((temp - fullmethod) <= 0) {
		return NULL;
	}
	temp++;
	len = len - (temp - fullmethod);
	if (len <= 0) {
		return NULL;
	}
	size_t size = sizeof(char) * len;
	char *methodname = (char*)malloc(size + sizeof(char));
	memset(methodname, 0, size + sizeof(char));
	memcpy(methodname, temp, len);
	temp = NULL;
	return methodname;
}

int orientsec_grpc_router_free(orientsec_grpc_router_t **router) {
	orientsec_grpc_router_t *routertemp = *router;
	if (!routertemp)
	{
		return 0;
	}
	FREE_PTR(routertemp->url);
	FREE_PTR(routertemp->servicename);
	FREE_PTR(routertemp->host);
	FREE_PTR(routertemp);
	return 0;
}

//获取consumer端所需要的provider版本
//获取内容不需要释放
static char g_consumer_service_version[ORIENTSEC_GRPC_SERVICE_VERSION_MAX] = {0};
char* orientsec_consumer_service_version() {
	if (g_consumer_service_version[0] != '\0')
	{
		return g_consumer_service_version;
	}
	if (0 == orientsec_grpc_properties_get_value(ORIENTSEC_GRPC_CONF_CONSUMER_SERVICE_VERSION, NULL, g_consumer_service_version)) {
		return g_consumer_service_version;
	}
	return NULL;
}

//获取provider端提供服务版本版本
//获取内容不需要释放
static char g_provider_service_version[500] = { 0 };
char* orientsec_provider_service_version() {
	if (g_provider_service_version[0] != '\0')
	{
		return g_provider_service_version;
	}
	if (0 == orientsec_grpc_properties_get_value(ORIENTSEC_GRPC_CONF_PROVIDER_VERSION, NULL, g_provider_service_version)) {
		return g_provider_service_version;
	}
	return NULL;
}

//获取provider端提供服务版本版本
//获取内容不需要释放
static char g_provider_service_group[500] = { 0 };
char* orientsec_provider_service_group() {
	if (g_provider_service_version[0] != '\0')
	{
		return g_provider_service_group;
	}
	if (0 == orientsec_grpc_properties_get_value(ORIENTSEC_GRPC_CONF_PROVIDER_GROUP, NULL, g_provider_service_group)) {
		return g_provider_service_group;
	}
	return NULL;
}


//校验正整数
//如果是正整数返回1,否则返回0
int orientsec_grpc_common_utils_isdigit(char *num)
{
	int i;
	for (i = 0; num[i]; i++)
	{
		if (i == 0 && num[i] == '0') {
			return 0;
		}
		if (num[i] > '9' || num[i] < '0') //只要有非数字，就返回错误
		{
			return 0;
		}
	}
	return 1;
}

long orientsec_grpc_common_utils_strtolong(char * strdigit) {
	return atol(strdigit);
}

//获取consumer端所需要的provider版本
//获取内容不需要释放
static char g_orientsec_grpc_version[100] = { 0 };
char* orientsec_grpc_version() {
	if (*g_orientsec_grpc_version != '\0')
	{
		return g_orientsec_grpc_version;
	}
	if (0 == orientsec_grpc_properties_get_value(ORIENTSEC_GRPC_CONF_COMMON_GRPC, NULL, g_orientsec_grpc_version)) {
		return g_orientsec_grpc_version;
	}
	return NULL;
}


