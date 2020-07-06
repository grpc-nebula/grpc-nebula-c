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
 *    工具类
 *    Modified: Jianbin Yang
 *    Mod: new property added in url
 */

#include "registry_utils.h"
#include "orientsec_grpc_properties_tools.h"
#include "orientsec_grpc_utils.h"
#include "registry_contants.h"
#include <string.h>
#include <grpc/support/log.h>
#include <grpc/support/alloc.h>
#include <src/core/lib/gpr/string.h>

// shared variable of public and private zk register directory
extern char* g_orientsec_grpc_common_root_directory;
extern char* g_orientsec_grpc_private_common_root_directory;

int get_service_name_from_path(const char* path, char* service_name, int buf_len) {
	if (!path || !service_name)
	{
		return -2;
	}
	int len = 0;
	char buf[ORIENTSEC_GRPC_BUF_LEN] = { 0 };
        sprintf(buf, "%s%s", g_orientsec_grpc_common_root_directory,
                ORIENTSEC_GRPC_REGISTRY_SEPARATOR);
	if (strncmp(path, buf, strlen(buf)) != 0)
	{
		return -1;
	}
        char* p = path + strlen(g_orientsec_grpc_common_root_directory) + 1;
	if (!p)
	{
		return -1;
	}
	char *pEnd = strchr(p, '/');
	if (!pEnd)
	{
		pEnd = path + strlen(path);

	}
	len = pEnd - p;
	if (len >= buf_len)
	{
		return -1;
	}
	strncpy(service_name, p, len);
	return 0;
}

/*
* func:根据provider数据，accessprotected属性生成路由规则
* date:20170821
* auth:addbyhuyn
*/
url_t * url_for_router_from_param(char *host_ip, char *service_name) {
	url_t* url = (url_t*)gpr_zalloc(sizeof(url_t));
	int param_num = PROVIDER_PROPERTIES_MAX_NUM;
	int param_index = 0;
	char buf[100];
	url->protocol = gprc_strdup(ORIENTSEC_GRPC_ROUTE_PROTOCOL);
	url->host = gprc_strdup(host_ip);
	url->path = gprc_strdup(service_name);
	url->params_num = param_num;
	url->parameters = (url_param*)gpr_zalloc(sizeof(url_param) * param_num);
	//interface name

	url->parameters[param_index].key = gprc_strdup(ORIENTSEC_GRPC_REGISTRY_KEY_INTERFACE);
	url->parameters[param_index].value = gprc_strdup(service_name);
	param_index++;

	//category
	url->parameters[param_index].key = gprc_strdup(ORIENTSEC_GRPC_CATEGORY_KEY);
	url->parameters[param_index].value = gprc_strdup(ORIENTSEC_GRPC_ROUTERS_CATEGORY);
	param_index++;
	//dynamic
	url->parameters[param_index].key = gprc_strdup(ORIENTSEC_GRPC_REGISTRY_KEY_DYNAMIC);
	url->parameters[param_index].value = gprc_strdup("true");
	param_index++;
	//force
	url->parameters[param_index].key = gprc_strdup(ORIENTSEC_GRPC_FORCE_KEY);
	url->parameters[param_index].value = gprc_strdup("true");
	param_index++;

	url->parameters[param_index].key = gprc_strdup(ORIENTSEC_GRPC_REGISTRY_KEY_NAME);
	url->parameters[param_index].value = gprc_strdup("access-protected-rule");
	param_index++;

	url->parameters[param_index].key = gprc_strdup(ORIENTSEC_GRPC_REGISTRY_KEY_PRIORITY);
	url->parameters[param_index].value = gprc_strdup("1000");
	param_index++;

	url->parameters[param_index].key = gprc_strdup(ORIENTSEC_GRPC_REGISTRY_KEY_ROUTER);
	url->parameters[param_index].value = gprc_strdup("condition");
	param_index++;

	url->parameters[param_index].key = gprc_strdup(ORIENTSEC_GRPC_REGISTRY_KEY_RULE);
	strcpy(buf, gprc_strdup(" => host !="));
	strcat(buf, host_ip);
	url->parameters[param_index].value = gprc_strdup(buf);
	param_index++;

	url->parameters[param_index].key = gprc_strdup(ORIENTSEC_GRPC_REGISTRY_KEY_RUNTIME);
	url->parameters[param_index].value = gprc_strdup("false");
	param_index++;

	return url;
}


//两种实现方式：1.先转成字符串，再转成url_t；2.各字段分别转化
url_t* url_from_provider(provider_t *provider) {
	url_t* url = (url_t*)gpr_zalloc(sizeof(url_t));
	int param_num = PROVIDER_PROPERTIES_MAX_NUM;
	int param_index = 0;
	char buf[64];
	if (provider->ext_data)
	{
		url_clone((url_t*)provider->ext_data, url);
		return url;
	}
	url->protocol = gprc_strdup(provider->protocol);
	if (provider->username)
	{
		url->username = gprc_strdup(provider->username);
	}
	if (provider->password)
	{
		url->password = gprc_strdup(provider->password);
	}
	url->host = gprc_strdup(provider->host);
	url->port = provider->port;
	url->path = gprc_strdup(provider->sInterface);

	url->params_num = param_num;
	url->parameters = (url_param*)gpr_zalloc(sizeof(url_param) * param_num);

	if (provider->version)
	{
		url->parameters[param_index].key = gprc_strdup(ORIENTSEC_GRPC_REGISTRY_KEY_VERSION);
		url->parameters[param_index].value = gprc_strdup(provider->version);
		param_index++;
	}

	url->parameters[param_index].key = gprc_strdup(ORIENTSEC_GRPC_REGISTRY_KEY_DEFAULT_TIMEOUT);
	sprintf(buf, "%d", provider->default_timeout);
	url->parameters[param_index].value = gprc_strdup(buf);
	param_index++;

	REINIT(buf);
	url->parameters[param_index].key = gprc_strdup(ORIENTSEC_GRPC_REGISTRY_KEY_DEFAULT_RETIES);
	sprintf(buf, "%d", provider->default_reties);
	url->parameters[param_index].value = gprc_strdup(buf);
	param_index++;

	REINIT(buf);
	url->parameters[param_index].key = gprc_strdup(ORIENTSEC_GRPC_REGISTRY_KEY_DEFAULT_CONNECTIONS);
	sprintf(buf, "%d", provider->default_connections);
	url->parameters[param_index].value = gprc_strdup(buf);
	param_index++;

	REINIT(buf);
	url->parameters[param_index].key = gprc_strdup(ORIENTSEC_GRPC_REGISTRY_KEY_DEFAULT_REQUESTS);
	sprintf(buf, "%d", provider->default_requests);
	url->parameters[param_index].value = gprc_strdup(buf);
	param_index++;

	REINIT(buf);
	url->parameters[param_index].key = gprc_strdup(ORIENTSEC_GRPC_REGISTRY_KEY_ACCESS_PROTECTED);
	sprintf(buf, "%s", provider->access_protected ? "true" : "false");
	url->parameters[param_index].value = gprc_strdup(buf);
	param_index++;

        // provider active/standby property
        REINIT(buf);
        url->parameters[param_index].key = gprc_strdup(ORIENTSEC_GRPC_REGISTRY_KEY_MASTER);
        sprintf(buf, "%s", provider->is_master ? "true" : "false");
        url->parameters[param_index].value = gprc_strdup(buf);
        param_index++;

        // provider service grouping and grading
        REINIT(buf);
        url->parameters[param_index].key =
            gprc_strdup(ORIENTSEC_GRPC_REGISTRY_KEY_GROUP);
        sprintf(buf, "%s", provider->group);
        url->parameters[param_index].value = gprc_strdup(buf);
        param_index++;

       
        url->parameters[param_index].key = gprc_strdup(ORIENTSEC_GRPC_CATEGORY_KEY);
        url->parameters[param_index].value = gprc_strdup(ORIENTSEC_GRPC_PROVIDERS_CATEGORY);
        param_index++;
       

	url->parameters[param_index].key = gprc_strdup(ORIENTSEC_GRPC_REGISTRY_KEY_DEFAULT_LOADBALANCE);
	url->parameters[param_index].value = gprc_strdup(lb_strategy_to_str(provider->default_loadbalance));
	param_index++;

	REINIT(buf);
	url->parameters[param_index].key = gprc_strdup(ORIENTSEC_GRPC_REGISTRY_KEY_DEFAULT_ASYNC);
	sprintf(buf, "%s", provider->default_async ? "true" : "false");
	url->parameters[param_index].value = gprc_strdup(buf);
	param_index++;

	if (provider->token)
	{
		url->parameters[param_index].key = gprc_strdup(ORIENTSEC_GRPC_REGISTRY_KEY_TOKEN);
		url->parameters[param_index].value = gprc_strdup(provider->token);
		param_index++;
	}

	REINIT(buf);
	url->parameters[param_index].key = gprc_strdup(ORIENTSEC_GRPC_REGISTRY_KEY_DEPRECATED);
	sprintf(buf, "%s", provider->deprecated ? "true" : "false");
	url->parameters[param_index].value = gprc_strdup(buf);
	param_index++;

	REINIT(buf);
	url->parameters[param_index].key = gprc_strdup(ORIENTSEC_GRPC_REGISTRY_KEY_DYNAMIC);
	sprintf(buf, "%s", provider->dynamic ? "true" : "false");
	url->parameters[param_index].value = gprc_strdup(buf);
	param_index++;

	REINIT(buf);
	url->parameters[param_index].key = gprc_strdup(ORIENTSEC_GRPC_REGISTRY_KEY_ACCESSLOG);
	sprintf(buf, "%s", provider->accesslog ? "true" : "false");
	url->parameters[param_index].value = gprc_strdup(buf);
	param_index++;

	if (provider->owner)
	{
		url->parameters[param_index].key = gprc_strdup(ORIENTSEC_GRPC_REGISTRY_KEY_OWNER);
		url->parameters[param_index].value = gprc_strdup(provider->owner);
		param_index++;
	}

	REINIT(buf);
	url->parameters[param_index].key = gprc_strdup(ORIENTSEC_GRPC_REGISTRY_KEY_WEIGHT);
	sprintf(buf, "%d", provider->weight);
	url->parameters[param_index].value = gprc_strdup(buf);
	param_index++;

	url->parameters[param_index].key = gprc_strdup(ORIENTSEC_GRPC_REGISTRY_KEY_DEFAULT_CLUSTER);
	url->parameters[param_index].value = gprc_strdup(cluster_strategy_to_str(provider->default_cluster));
	param_index++;

	if (provider->application)
	{
		url->parameters[param_index].key = gprc_strdup(ORIENTSEC_GRPC_REGISTRY_KEY_APPLICATION);
		url->parameters[param_index].value = gprc_strdup(provider->application);
		param_index++;
	}
	if (provider->application_version)
	{
		url->parameters[param_index].key = gprc_strdup(ORIENTSEC_GRPC_REGISTRY_KEY_APPLICATION_VERSION);
		url->parameters[param_index].value = gprc_strdup(provider->application_version);
		param_index++;
	}
	if (provider->organization)
	{
		url->parameters[param_index].key = gprc_strdup(ORIENTSEC_GRPC_REGISTRY_KEY_ORGANIZATION);
		url->parameters[param_index].value = gprc_strdup(provider->organization);
		param_index++;
	}
	if (provider->environment)
	{
		url->parameters[param_index].key = gprc_strdup(ORIENTSEC_GRPC_REGISTRY_KEY_ENVIRONMENT);
		url->parameters[param_index].value = gprc_strdup(provider->environment);
		param_index++;
	}
	if (provider->module)
	{
		url->parameters[param_index].key = gprc_strdup(ORIENTSEC_GRPC_REGISTRY_KEY_MODULE);
		url->parameters[param_index].value = gprc_strdup(provider->module);
		param_index++;
	}
	if (provider->module_version)
	{
		url->parameters[param_index].key = gprc_strdup(ORIENTSEC_GRPC_REGISTRY_KEY_MODULE_VERSION);
		url->parameters[param_index].value = gprc_strdup(provider->module_version);
		param_index++;
	}

	REINIT(buf);
	url->parameters[param_index].key = gprc_strdup(ORIENTSEC_GRPC_REGISTRY_KEY_ANYHOST);
	sprintf(buf, "%s", provider->anyhost ? "true" : "false");
	url->parameters[param_index].value = gprc_strdup(buf);
	param_index++;

	if (provider->sInterface)
	{
		url->parameters[param_index].key = gprc_strdup(ORIENTSEC_GRPC_REGISTRY_KEY_INTERFACE);
		url->parameters[param_index].value = gprc_strdup(provider->sInterface);
		param_index++;
	}
	if (provider->methods)
	{
		url->parameters[param_index].key = gprc_strdup(ORIENTSEC_GRPC_REGISTRY_KEY_METHODS);
		url->parameters[param_index].value = gprc_strdup(provider->methods);
		param_index++;
	}
	REINIT(buf);
	url->parameters[param_index].key = gprc_strdup(ORIENTSEC_GRPC_REGISTRY_KEY_PID);
	sprintf(buf, "%d", provider->pid);
	url->parameters[param_index].value = gprc_strdup(buf);
	param_index++;
        //begin by liumin
        REINIT(buf);
        url->parameters[param_index].key = gprc_strdup(ORIENTSEC_GRPC_REGISTRY_KEY_PROJECT);
        sprintf(buf, "%s", provider->project);
        url->parameters[param_index].value = gprc_strdup(buf);
        param_index++;
        // for owner use
        REINIT(buf);
        url->parameters[param_index].key = gprc_strdup(ORIENTSEC_GRPC_REGISTRY_KEY_COMM_OWNER);
        sprintf(buf, "%s", provider->comm_owner);
        url->parameters[param_index].value = gprc_strdup(buf);
        param_index++;
        // for owner operation executive
        REINIT(buf);
        url->parameters[param_index].key =
            gprc_strdup(ORIENTSEC_GRPC_REGISTRY_KEY_COMM_OPS);
        sprintf(buf, "%s", provider->comm_ops);
        url->parameters[param_index].value = gprc_strdup(buf);
        param_index++;
        //end by liumin
	if (provider->side)
	{
		url->parameters[param_index].key = gprc_strdup(ORIENTSEC_GRPC_REGISTRY_KEY_SIDE);
		url->parameters[param_index].value = gprc_strdup(provider->side);
		param_index++;
	}

	REINIT(buf);
	url->parameters[param_index].key = gprc_strdup(ORIENTSEC_GRPC_REGISTRY_KEY_TIMESTAMP);
#if (defined WIN64)||(defined WIN32)
	sprintf(buf, "%I64d", provider->timestamp);
#else
	sprintf(buf, "%lld", provider->timestamp);
#endif
	url->parameters[param_index].value = gprc_strdup(buf);
	param_index++;

	if (provider->grpc)
	{
		url->parameters[param_index].key = gprc_strdup(ORIENTSEC_GRPC_REGISTRY_KEY_GRPC);
		url->parameters[param_index].value = gprc_strdup(provider->grpc);
		param_index++;
	}
	if (provider->dubbo)
	{
		url->parameters[param_index].key = gprc_strdup(ORIENTSEC_GRPC_REGISTRY_KEY_DUBBO);
		url->parameters[param_index].value = gprc_strdup(provider->dubbo);
		param_index++;
	}
	return url;
}


provider_t* provider_from_url(url_t *url) {
	provider_t *provider = new_provider();
	init_provider_from_url(provider, url);
	return provider;
}

void init_provider_from_url(provider_t *provider, url_t *url) {
	char buf[ORIENTSEC_GRPC_BUF_LEN] = { 0 };
	int len = 0;
	char *p = NULL;
	if (!provider || !url)
	{
		return;
	}
	if (url->protocol)
	{
		len = strlen(url->protocol);
		snprintf(provider->protocol, len < PROTOCOL_NAME_MAX_LEN ? (len + 1) : PROTOCOL_NAME_MAX_LEN, "%s", url->protocol);
	}
	if (url->username)
	{
		provider->username = gprc_strdup(url->username);
	}
	if (url->password)
	{
		provider->password = gprc_strdup(url->password);
	}
	if (url->host)
	{
		len = strlen(url->host);
		snprintf(provider->host, len < HOST_MAX_LEN ? (len + 1) : HOST_MAX_LEN, "%s", url->host);
	}
	provider->port = url->port;
	//provider->version = url_get_parameter(url, ORIENTSEC_GRPC_REGISTRY_KEY_VERSION, NULL);
	p = url_get_parameter(url, ORIENTSEC_GRPC_REGISTRY_KEY_GROUP, NULL);
        if (p) {
          snprintf(provider->group, strlen(p) + 1, "%s", p);
          FREE_PTR(p);
        }

        p = url_get_parameter(url, ORIENTSEC_GRPC_REGISTRY_KEY_VERSION, NULL);
        if (p) {
          snprintf(provider->version, strlen(p)+1, "%s", p);
          FREE_PTR(p);
        }
	p = url_get_parameter(url, ORIENTSEC_GRPC_REGISTRY_KEY_DEFAULT_TIMEOUT, NULL);
	if (p)
	{
		provider->default_timeout = atoi(p);
		FREE_PTR(p);
	}
	p = url_get_parameter(url, ORIENTSEC_GRPC_REGISTRY_KEY_DEFAULT_RETIES, NULL);
	if (p)
	{
		provider->default_reties = atoi(p);
		FREE_PTR(p);
	}
	p = url_get_parameter(url, ORIENTSEC_GRPC_REGISTRY_KEY_DEFAULT_CONNECTIONS, NULL);
	if (p)
	{
		provider->default_connections = atoi(p);
		FREE_PTR(p);
	}
	p = url_get_parameter(url, ORIENTSEC_GRPC_REGISTRY_KEY_DEFAULT_REQUESTS, NULL);
	if (p)
	{
		provider->default_requests = atoi(p);
		FREE_PTR(p);
	}
	p = url_get_parameter(url, ORIENTSEC_GRPC_REGISTRY_KEY_ACCESS_PROTECTED, NULL);
	if (p)
	{
          provider->access_protected =
              (0 == orientsec_stricmp("true", p)) ? true : false;
		FREE_PTR(p);
	}

	p = url_get_parameter(url, ORIENTSEC_GRPC_REGISTRY_KEY_DEFAULT_LOADBALANCE, NULL);
	if (p)
	{
          provider->default_loadbalance = lb_strategy_from_str(p);
	  FREE_PTR(p);
	}

	p = url_get_parameter(url, ORIENTSEC_GRPC_REGISTRY_KEY_DEFAULT_ASYNC, NULL);
	if (p)
	{
          provider->default_async =
              (0 == orientsec_stricmp("true", p)) ? true : false;
		FREE_PTR(p);
	}

	provider->token = url_get_parameter(url, ORIENTSEC_GRPC_REGISTRY_KEY_TOKEN, NULL);

	p = url_get_parameter(url, ORIENTSEC_GRPC_REGISTRY_KEY_DEPRECATED, NULL);
	if (p)
	{
          provider->deprecated = (0 == orientsec_stricmp("true", p)) ? true : false;
		FREE_PTR(p);
	}

	p = url_get_parameter(url, ORIENTSEC_GRPC_REGISTRY_KEY_DYNAMIC, NULL);
	if (p)
	{
          provider->dynamic = (0 == orientsec_stricmp("true", p)) ? true : false;
		FREE_PTR(p);
	}

	p = url_get_parameter(url, ORIENTSEC_GRPC_REGISTRY_KEY_ACCESSLOG, NULL);
	if (p)
	{
          provider->accesslog = (0 == orientsec_stricmp("true", p)) ? true : false;
		FREE_PTR(p);
	}

	provider->owner = url_get_parameter(url, ORIENTSEC_GRPC_REGISTRY_KEY_OWNER, NULL);

	p = url_get_parameter(url, ORIENTSEC_GRPC_REGISTRY_KEY_WEIGHT, NULL);
	if (p)
	{
		provider->weight = atoi(p);
		FREE_PTR(p);
	}

	p = url_get_parameter(url, ORIENTSEC_GRPC_REGISTRY_KEY_DEFAULT_CLUSTER, NULL);
	if (p)
	{
		provider->default_cluster = cluster_strategy_from_str(p);
		FREE_PTR(p);
	}

	provider->application = url_get_parameter(url, ORIENTSEC_GRPC_REGISTRY_KEY_APPLICATION, NULL);
	provider->application_version = url_get_parameter(url, ORIENTSEC_GRPC_REGISTRY_KEY_APPLICATION_VERSION, NULL);

	provider->organization = url_get_parameter(url, ORIENTSEC_GRPC_REGISTRY_KEY_ORGANIZATION, NULL);
	provider->environment = url_get_parameter(url, ORIENTSEC_GRPC_REGISTRY_KEY_ENVIRONMENT, NULL);
	provider->module = url_get_parameter(url, ORIENTSEC_GRPC_REGISTRY_KEY_MODULE, NULL);
	provider->module_version = url_get_parameter(url, ORIENTSEC_GRPC_REGISTRY_KEY_MODULE_VERSION, NULL);

	p = url_get_parameter(url, ORIENTSEC_GRPC_REGISTRY_KEY_ANYHOST, NULL);
	if (p)
	{
          provider->anyhost = (0 == orientsec_stricmp("true", p)) ? true : false;
		FREE_PTR(p);
	}

	provider->sInterface = url_get_parameter(url, ORIENTSEC_GRPC_REGISTRY_KEY_INTERFACE, NULL);
	provider->methods = url_get_parameter(url, ORIENTSEC_GRPC_REGISTRY_KEY_METHODS, NULL);

	p = url_get_parameter(url, ORIENTSEC_GRPC_REGISTRY_KEY_PID, NULL);
	if (p)
	{
		provider->pid = atol(p);
		FREE_PTR(p);
	}

	p = url_get_parameter(url, ORIENTSEC_GRPC_REGISTRY_KEY_SIDE, NULL);
	if (p)
	{
		len = strlen(p);
		snprintf(provider->side, len < 20 ? len + 1 : 20, "%s", p);
		FREE_PTR(p);
	}

	p = url_get_parameter(url, ORIENTSEC_GRPC_REGISTRY_KEY_TIMESTAMP, NULL);
	if (p)
	{
		provider->timestamp = atoll(p);
		FREE_PTR(p);
	}

	provider->grpc = url_get_parameter(url, ORIENTSEC_GRPC_REGISTRY_KEY_GRPC, NULL);
	provider->dubbo = url_get_parameter(url, ORIENTSEC_GRPC_REGISTRY_KEY_DUBBO, NULL);
	provider->flag_invalid = 0;       //有效标记
	provider->flag_invalid_timestamp = 0;
	provider->flag_call_failover = 0; //未发生调用出错。
	provider->flag_in_blklist = 0;    //不在黑名单
	provider->flag_subchannel_close = 0;
	provider->time_subchannel_close = 0;
        p = url_get_parameter(url, ORIENTSEC_GRPC_REGISTRY_KEY_MASTER, NULL);
        if (p) {
          provider->is_master =
              (0 == orientsec_stricmp("true", p)) ? true : false;
          FREE_PTR(p);
        }
        provider->online = 1;
	provider->ext_data = gpr_zalloc(sizeof(url_t));
	url_clone(url, (url_t*)provider->ext_data);
}

provider_t* clone_provider(provider_t* src) {
	if (!src)
	{
		return NULL;
	}
	provider_t* ret = new_provider();
	strcpy(ret->protocol, src->protocol);
	ret->username = gprc_strdup(src->username);
	ret->password = gprc_strdup(src->password);
	strcpy(ret->host, src->host);
	ret->port = src->port;
        strcpy(ret->version, src->version);
	//ret->version = gprc_strdup(src->version);
	strcpy(ret->group,src->group);
	ret->default_timeout = src->default_timeout;
	ret->default_reties = src->default_reties;
	ret->default_connections = src->default_connections;
	ret->default_requests = src->default_requests;
	ret->access_protected = src->access_protected;
        ret->is_master = src->is_master;
	ret->default_loadbalance = src->default_loadbalance;
	ret->default_async = src->default_async;
	ret->token = gprc_strdup(src->token);
	ret->deprecated = src->deprecated;
	ret->dynamic = src->dynamic;
	ret->accesslog = src->accesslog;
	ret->owner = gprc_strdup(src->owner);
	ret->weight = src->weight;
	ret->default_cluster = src->default_cluster;
	ret->application = gprc_strdup(src->application);
	ret->application_version = gprc_strdup(src->application_version);
	ret->organization = gprc_strdup(src->organization);
	ret->environment = gprc_strdup(src->environment);
	ret->module = gprc_strdup(src->module);
	ret->module_version = gprc_strdup(src->module_version);
	ret->anyhost = src->anyhost;
	ret->sInterface = gprc_strdup(src->sInterface);
	ret->methods = gprc_strdup(src->methods);
	ret->pid = src->pid;
	strcpy(ret->side, src->side);
	ret->timestamp = src->timestamp;
	ret->grpc = gprc_strdup(src->grpc);
	ret->dubbo = gprc_strdup(src->dubbo);
	ret->flag_call_failover = src->flag_call_failover;
	ret->flag_in_blklist = src->flag_in_blklist;
	ret->flag_subchannel_close = src->flag_subchannel_close;
	if (src->ext_data)
	{
		ret->ext_data = gpr_zalloc(sizeof(url_t));
		url_clone((url_t*)src->ext_data, (url_t*)ret->ext_data);
	}

	return ret;
}


//释放provider空间,释放ext_data空间
void free_provider_ex(provider_t **p) {
	provider_t *provider = *p;
	if (!provider)
	{
		return;
	}
	free_provider_v2_ex(provider);
	FREE_PTR(provider);
}

//释放provider空间,是否ext_data空间
void free_provider_v2_ex(provider_t *p) {
	free_provider_v2(p);
	if (p && p->ext_data)
	{
		url_free(((url_t*)(p->ext_data)));
		free(p->ext_data);
	}
}
//int orientsec_stricmp(const char* a, const char* b) {
//  int ca, cb;
//  do {
//    ca = tolower(*a);
//    cb = tolower(*b);
//    ++a;
//    ++b;
//  } while (ca == cb && ca && cb);
//  return ca - cb;
//}
