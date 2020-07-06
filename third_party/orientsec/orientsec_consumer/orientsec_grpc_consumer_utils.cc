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
 *    2017/07/19
 *    version 0.0.9
 *    consumer接口函数工具类函数定义
 */

#include <string.h>
#include "orientsec_grpc_common.h"
#include "orientsec_grpc_common_utils.h"
#include "orientsec_grpc_utils.h"
#include "orientsec_grpc_conf.h"
#include "registry_contants.h"
#include "orientsec_grpc_consumer_utils.h"
#include "orientsec_grpc_properties_tools.h"
#include "url.h"
#include "orientsec_grpc_consumer_control_deprecated.h"
#include <grpc/support/port_platform.h>
#include <grpc/support/alloc.h>
#include <grpc/support/log.h>

//读取配置文件中的参数组织成url串。
//生成的url串格式为 aaa=xx&bbb=xxx&ccc=xxxx
int orientsec_grpc_consumer_conf_param(char * urlparam, grpc_conf_template_t confitem[], int itemcount) {
  if (orientsec_grpc_properties_init() < 0) {
	  return -1;
  }
  char pconfitem[ORIENTSEC_GRPC_PROPERTY_VALUE_MAX_LEN];
  int count = 0;
  for (int i = 0; i < itemcount; i++) {
    memset(pconfitem, 0, ORIENTSEC_GRPC_PROPERTY_VALUE_MAX_LEN);
    if (confitem[i].keyreg == "" || confitem[i].keyreg == NULL) {
      continue;
    }
    strcat(urlparam, "&");
    strcat(urlparam, confitem[i].keyreg);
    strcat(urlparam, "=");
    //读取配置文件
    if (confitem[i].key == "" || confitem[i].key == NULL) {
       strcat(urlparam, confitem[i].defaultvalue);
    }
    else {
      orientsec_grpc_properties_get_value(confitem[i].key, NULL, pconfitem);
      if (pconfitem == NULL || strcmp(pconfitem, "") == 0) {  //取不到值，设置为默认值
        // add required fields check
        if (ORIENTSEC_GRPC_CONF_COMMON_APPLICATION == confitem[i].key) {
          gpr_log(GPR_ERROR,"[common.application] not configured.\n");
        }
        if (ORIENTSEC_GRPC_CONF_COMMON_PROJECT == confitem[i].key) {
          gpr_log(GPR_ERROR,"[common.project] not configured.\n");
        }
        if (ORIENTSEC_GRPC_CONF_COMMON_OWNER == confitem[i].key) {
          gpr_log(GPR_ERROR,"[common.owner] not configured.\n");
        }
        // joint default value if item not configured
	strcat(urlparam, confitem[i].defaultvalue);
      }
      else {
	strcat(urlparam, pconfitem);
      }
    }
  }
  return 0;
}

//拼接url参数
void orientsec_grpc_consumer_url_append(char *url, const char * prefix, const  char *key, const char *val) {
  if (prefix != NULL) {
    strcat(url, prefix);
  }
  strcat(url, key);
  strcat(url, "=");
  strcat(url, val);
}

//定义consumer端配置文件模板,数据样例
//consumer://192.168.13.1/com.orientsec.grpc.examples.helloworld.Greeter
//?application=grpc-test-app
//&category=consumers
//&check=true
//&default.cluster=failover
//&default.connections=0
//&default.loadbalance=round_robin
//&default.requests=10000
//&default.retries=2
//&default.timeout=1000
//&dynamic=true
//&grpc=1.0.0
//&side=consumer
//&interface=com.orientsec.grpc.examples.helloworld.Greeter 另外附加
//&pid=8104  另外附加
//&timestamp=1497246256401 另外附加
//定义不定长数组用while循环变量
static grpc_conf_template_t consumer_confitem_consumers[] = {
  //application
  { (char*)ORIENTSEC_GRPC_CONF_COMMON_APPLICATION,(char*)ORIENTSEC_GRPC_REGISTRY_KEY_APPLICATION,1,(char*)ORIENTSEC_GRPC_COMMON_BLANK },
  //category
  { (char*)ORIENTSEC_GRPC_CONF_CONSUMER_CATEGORY,(char*)ORIENTSEC_GRPC_CATEGORY_KEY,1,(char*)ORIENTSEC_GRPC_CONSUMERS_CATEGORY },
  //check
  { (char*)ORIENTSEC_GRPC_CONF_CONSUMER_CHECK,(char*)ORIENTSEC_GRPC_REGISTRY_KEY_CHECK,1,(char*)ORIENTSEC_GRPC_CONF_CONSUMER_CHECK_DEFAULT },
  //&default.cluster
  { (char*)ORIENTSEC_GRPC_CONF_CONSUMER_DEFAULT_CLUSTER,(char*)ORIENTSEC_GRPC_REGISTRY_KEY_DEFAULT_CLUSTER,1,(char*)ORIENTSEC_GRPC_CONF_CONSUMER_DEFAULT_CLUSTER_DEFAULT },
  //&default.connections
  { (char*)ORIENTSEC_GRPC_CONF_CONSUMER_DEFAULT_CONNECTIONS,(char*)ORIENTSEC_GRPC_REGISTRY_KEY_DEFAULT_CONNECTIONS,1,(char*)ORIENTSEC_GRPC_CONF_CONSUMER_DEFAULT_CONNECTIONS_DEFAULT },
  //&default.loadbalance=round_robin
  { (char*)ORIENTSEC_GRPC_CONF_CONSUMER_DEFAULT_LOADBALANCE,(char*)ORIENTSEC_GRPC_REGISTRY_KEY_DEFAULT_LOADBALANCE,1,(char*)ORIENTSEC_GRPC_CONF_CONSUMER_DEFAULT_LOADBALANCE_DEFAULT },
  //&default.requests   默认需要检查，设置为 10000？？？？ huyntodo 
  { (char*)ORIENTSEC_GRPC_CONF_CONSUMER_DEFAULT_REQUESTS,(char*)ORIENTSEC_GRPC_REGISTRY_KEY_DEFAULT_REQUESTS,1,(char*)ORIENTSEC_GRPC_CONF_CONSUMER_DEFAULT_REQUESTS_DEFAULT },
  //&default.retries=2
  { (char*)ORIENTSEC_GRPC_CONF_CONSUMER_DEFAULT_RETRIES,(char*)ORIENTSEC_GRPC_REGISTRY_KEY_DEFAULT_RETIES,0,(char*)ORIENTSEC_GRPC_CONF_CONSUMER_DEFAULT_RETRIES_DEFAULT },
  //&default.timeout=1000
  { (char*)ORIENTSEC_GRPC_CONF_CONSUMER_DEFAULT_TIMEOUT,(char*)ORIENTSEC_GRPC_REGISTRY_KEY_DEFAULT_TIMEOUT,0,(char*)ORIENTSEC_GRPC_CONF_CONSUMER_DEFAULT_TIMEOUT_DEFAULT },
  //&grpc=1.0.0
  { (char*)ORIENTSEC_GRPC_CONF_COMMON_GRPC,(char*)ORIENTSEC_GRPC_REGISTRY_KEY_VERSION,1,(char*)ORIENTSEC_GRPC_COMMON_BLANK },
  //&side=consumer
  { (char*)ORIENTSEC_GRPC_CONF_CONSUMER_SIDE,(char*)ORIENTSEC_GRPC_REGISTRY_KEY_SIDE,1,(char*)ORIENTSEC_GRPC_CONF_CONSUMER_SIDE_DEFAULT },
  //&dynamic=true  （默认应该为 true）
  { (char*)ORIENTSEC_GRPC_CONF_CONSUMER_DYNAMIC,(char*)ORIENTSEC_GRPC_REGISTRY_KEY_DYNAMIC,0,(char*)ORIENTSEC_GRPC_CONF_CONSUMER_DYNAMIC_DEFAULT },
  //move to upper since of grouping based on service
  //&group= (default value= NULL) 
  //{ (char*)ORIENTSEC_GRPC_CONF_CONSUMER_GROUP,(char*)ORIENTSEC_GRPC_REGISTRY_KEY_INVOKE_GROUP,1,(char*)ORIENTSEC_GRPC_CONF_CONSUMER_GROUP_DEFAULT},
  //&project=grpc-test-app （默认应该为 grpc-test-app ）
  { (char*)ORIENTSEC_GRPC_CONF_COMMON_PROJECT, (char*)ORIENTSEC_GRPC_REGISTRY_KEY_PROJECT, 1, (char*)ORIENTSEC_GRPC_CONF_COMMON_PROJECT_DEFAULT},
  //&owner=A0000 （默认应该为 A0000 ）
  {(char*)ORIENTSEC_GRPC_CONF_COMMON_OWNER, (char*)ORIENTSEC_GRPC_REGISTRY_KEY_COMM_OWNER, 1,(char*)ORIENTSEC_GRPC_CONF_COMMON_OWNER_DEFAULT},
  //&ops=B0000 （默认应该为 B0000 ）
  { (char*)ORIENTSEC_GRPC_CONF_COMMON_OPS, (char*)ORIENTSEC_GRPC_REGISTRY_KEY_COMM_OPS, 1, (char*)ORIENTSEC_GRPC_CONF_COMMON_OPS_DEFAULT}};
 

// 组织consumer配置参数
int orientsec_grpc_concate_consumer_str(char * urlparam) {
  return orientsec_grpc_consumer_conf_param(urlparam, consumer_confitem_consumers, sizeof(consumer_confitem_consumers) / sizeof(consumer_confitem_consumers[0]));
}

// 获得基于服务名或者方法名的重试次数
int obtain_retries_based_service_or_method(const char* service_name,
                                             const char* method_name){
  if (strlen(service_name) == 0 && strlen(method_name) == 0) return NULL;
  char* search_service_key = (char*)gpr_zalloc(256 * sizeof(char));
  char* search_method_key = (char*)gpr_zalloc(256 * sizeof(char));
  char serv_retries[32] = {0};
  char serv_specific_retries[32] = {0};
  char meth_specific_retries[32] = {0};
  strcpy(search_service_key, ORIENTSEC_GRPC_CONF_CONSUMER_DEFAULT_RETRIES);
  strcat(search_service_key, "[");
  strcat(search_service_key, service_name);
  if (strlen(method_name) > 0 ){
    strcpy(search_method_key, search_service_key);
    strcat(search_method_key, ".");
    strcat(search_method_key, method_name);
    strcat(search_method_key, "]");
  }
  strcat(search_service_key, "]");
  orientsec_grpc_properties_get_value(search_method_key, NULL,
                                      meth_specific_retries);
  orientsec_grpc_properties_get_value(search_service_key, NULL,
                                      serv_specific_retries);
  orientsec_grpc_properties_get_value(
      ORIENTSEC_GRPC_CONF_CONSUMER_DEFAULT_RETRIES, NULL, serv_retries);
  int serv_retry_times = atoi(serv_retries);
  int serv_spec_retry_times = atoi(serv_specific_retries);
  int meth_spec_retry_times = atoi(meth_specific_retries);
  // 服务名配置的group为空
  if (strlen(meth_specific_retries) > 0) {
    gpr_free(search_service_key);
    gpr_free(search_method_key);
    return meth_spec_retry_times;
  } else if (strlen(serv_specific_retries) > 0) {
    gpr_free(search_service_key);
    gpr_free(search_method_key);
    return serv_spec_retry_times;
  } else {
    return serv_retry_times;
  }
}


char* obtain_appointed_provider_list(const char* service_name) {
  if ((service_name == NULL) || strlen(service_name) == 0) return NULL;
  char* search_key = (char*)gpr_zalloc(128 * sizeof(char));
  char* spec_prov_list = (char*)gpr_zalloc(1024);
  strcpy(search_key, ORIENTSEC_GRPC_CONF_SERVICE_SERVER_LIST);
  strcat(search_key, "[");
  strcat(search_key, service_name);
  strcat(search_key, "]");
  orientsec_grpc_properties_get_value(search_key, NULL, spec_prov_list);
  free(search_key);
  return spec_prov_list;
}

char* obtain_invoke_group_based_service(const char* service_name) {
  if ((service_name == NULL) || strlen(service_name) == 0) return NULL;
  char* search_key = (char*)gpr_zalloc(128 * sizeof(char));
  char* serv_group = (char*)gpr_zalloc(128);
  char* invoke_group = (char*)gpr_zalloc(128);
  strcpy(search_key, ORIENTSEC_GRPC_CONF_CONSUMER_INVOKE_GROUP);
  strcat(search_key, "[");
  strcat(search_key, service_name);
  strcat(search_key, "]");
  orientsec_grpc_properties_get_value(search_key, NULL, serv_group);
  orientsec_grpc_properties_get_value(ORIENTSEC_GRPC_CONF_CONSUMER_INVOKE_GROUP,
                                      NULL, invoke_group);
  // 服务名配置的group为空   
  if (strlen(serv_group) == 0) { 
    gpr_free(serv_group);
    gpr_free(search_key);
    return invoke_group;
  } else {
    gpr_free(invoke_group);
    gpr_free(search_key);
    return serv_group;
  } 
}

//拼接url字符串。参数样例如下：
char* orientsec_grpc_consumer_url(const char* service_name,const char* frame_version) {
	//此处可以判断是否需要注册不需要注册的话ruturn "" 即可。
	size_t buf_size = sizeof(char) * ORIENTSEC_GRPC_URL_MAX_LEN;
	char buf[128] = { 0 };
	//拼接consumer注册url串
	char *result = (char*)malloc(buf_size);
	memset(result, 0, buf_size);
	strcat(result, ORIENTSEC_GRPC_CONSUMER_PROTOCOL);  //consumer
	strcat(result, HSTC_GRPC_URL_PROTOCOL_SUFFIX);
	strcat(result, get_local_ip());             //ipaddress
	strcat(result, HSTC_GRPC_URL_CHAR_XG); // 斜杠 /	
	strcat(result, service_name); //服务名称

	//附加接口名称 infterface
	orientsec_grpc_consumer_url_append(result, "?", ORIENTSEC_GRPC_REGISTRY_KEY_INTERFACE, service_name);

	//附加进程ID pid 
	sprintf(buf, "%d", grpc_getpid());
	orientsec_grpc_consumer_url_append(result, "&", ORIENTSEC_GRPC_REGISTRY_KEY_PID, buf);

#if (defined WIN64) || (defined WIN32)
	sprintf(buf, "%I64d", orientsec_get_timestamp_in_mills());
#else
	sprintf(buf, "%lld", orientsec_get_timestamp_in_mills());
#endif	
	//附加时间戳 timestamp 
	orientsec_grpc_consumer_url_append(result, "&", ORIENTSEC_GRPC_REGISTRY_KEY_TIMESTAMP, buf);

	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%d", orientsec_grpc_thdid_get());
	orientsec_grpc_consumer_url_append(result, "&", ORIENTSEC_GRPC_REGISTRY_KEY_THREADID, buf);
       
        // append property of grpc framework version
        memset(buf, 0, sizeof(buf));
        char* grpc_version = grpc_verion_format(frame_version);
        strcpy(buf, grpc_version);
        free(grpc_version);
        orientsec_grpc_consumer_url_append(
            result, "&", ORIENTSEC_GRPC_REGISTRY_KEY_FRAME_VERSION, buf);

        // append property of grouping info
        memset(buf, 0, sizeof(buf));
        char* group_info = obtain_invoke_group_based_service(service_name);
        if (0 == strlen(group_info))
          strcpy(buf, (char*)ORIENTSEC_GRPC_CONF_CONSUMER_GROUP_DEFAULT);
        else
          strcpy(buf, group_info);
        gpr_free(group_info);
        orientsec_grpc_consumer_url_append(
            result, "&", ORIENTSEC_GRPC_REGISTRY_KEY_INVOKE_GROUP, buf);
        memset(buf, 0, sizeof(buf));

	//附加consumers其他附加参数
	orientsec_grpc_concate_consumer_str(result);

	return result;
}

bool is_match_glob_pattern(std::string pattern, std::string value, url_t* param) {
	if (param != NULL && is_start_with(pattern.c_str(), "$"))
	{
	      char *paramPtr = url_get_raw_parameter(param, pattern.substr(1).c_str());
	      pattern = paramPtr;
	      FREE_PTR(paramPtr);
	}
	return is_match_glob_pattern2(pattern, value);
}

bool is_match_glob_pattern2(std::string pattern, std::string value) {
	if (0 == strcmp("*", pattern.c_str()))
		return true;
	if ((pattern.length() == 0)
		&& (value.length() == 0))
		return true;
	if ((pattern.length() == 0)
		|| (value.length() == 0))
		return false;

	int i = pattern.find_last_of('*');
	// 没有找到星号
	if (i == -1) {
		return strcmp(pattern.c_str(), value.c_str()) == 0;
	}
	// 星号在末尾
	else if (i == pattern.length() - 1) {
		return  is_start_with(value.c_str(), pattern.substr(0, i).c_str());
	}
	// 星号的开头
	else if (i == 0) {
		return is_ends_with(value.c_str(), pattern.substr(i + 1).c_str());
	}
	// 星号的字符串的中间
	else {
		std::string prefix = pattern.substr(0, i);
		std::string suffix = pattern.substr(i + 1);
		return is_start_with(value.c_str(), prefix.c_str()) && is_ends_with(value.c_str(), suffix.c_str());
	}
}

//校验是否在黑名单内，是否多次调用出错。
bool is_provider_invalid(provider_t* provider) {
	if (ORIENTSEC_GRPC_CHECK_BIT(provider->flag_in_blklist, ORIENTSEC_GRPC_PROVIDER_FLAG_IN_BLKLIST) ||
		ORIENTSEC_GRPC_CHECK_BIT(provider->flag_call_failover, ORIENTSEC_GRPC_PROVIDER_FLAG_CALL_FAIlOVER) ||
		ORIENTSEC_GRPC_CHECK_BIT(provider->flag_subchannel_close, ORIENTSEC_GRPC_PROVIDER_FLAG_SUB_CLOSED) ||
		ORIENTSEC_GRPC_CHECK_BIT(provider->flag_invalid, 1))
	{
		return true;
	}
	//addbyhuyn 校验服务是否已过时（更新）
	consumer_check_provider_deprecated(provider->sInterface, provider->deprecated);
	return false;
}

