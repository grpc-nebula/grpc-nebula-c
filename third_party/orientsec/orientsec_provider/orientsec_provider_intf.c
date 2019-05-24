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
 *    2017/06/16
 *    version 0.9
 *    provider操作接口实现文件
 */

#include "orientsec_provider_intf.h"
#include "orientsec_grpc_utils.h"
#include "orientsec_grpc_registy_intf.h"
#include "orientsec_grpc_properties_tools.h"
#include "registry_utils.h"
#include "registry_contants.h"
#include <grpc/support/log.h>
#include <grpc/support/alloc.h>
#include <grpc/support/sync.h>
#include <src/core/lib/gpr/spinlock.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct _provider_lst {
  provider_t *provider;
  struct _provider_lst *next;
  int current_conns;  //当前并发连接数
  int current_reqs;   //当前并发请求数
  uint64_t last_log_time; //最后一次打印日志时间
  gpr_mu conns_mu;
  gpr_mu reqs_mu;
}provider_lst;

static provider_lst provider_list_head = { .provider = NULL,
					  .next = NULL,
					  .current_conns = 0,
					  .current_reqs = 0,
					  .last_log_time = 0
};
static provider_lst *p_provider_list_head = &provider_list_head;
static bool provider_lst_inited = false;

void cache_provider_node(provider_t *provider) {
  provider_lst *pl = (provider_lst*)gpr_zalloc(sizeof(provider_lst));
  if (pl)
  {
    pl->provider = provider;
    pl->current_conns = 0;
    pl->current_reqs = 0;
    pl->last_log_time = 0;
    gpr_mu_init(&pl->conns_mu);
    gpr_mu_init(&pl->reqs_mu);
    pl->next = p_provider_list_head->next;
    p_provider_list_head->next = pl;
  }
}

provider_t* init_and_cache_provider(int port, const char *sIntf, const char *sMethods) {
  provider_t *provider = new_provider();
  init_provider(provider);
  char *ptr = get_local_ip();
  if (ptr)
  {
    size_t len = strlen(ptr);
    snprintf(provider->host, len < sizeof(provider->host) ? len + 1 : sizeof(provider->host), "%s",ptr);
  }
  provider->pid = grpc_getpid();
  provider->port = port;
  provider->timestamp = orientsec_get_timestamp_in_mills();
  provider->sInterface = gprc_strdup(sIntf);
  provider->methods = gprc_strdup(sMethods);
  cache_provider_node(provider);

  return provider;
}


//预处理，只标记规则IP为0.0.0.0，与本地ip相同的规则，如果规则中指定了application，则application值也需要
//和本地配置application相同,
void overrides_preprocess(url_t *urls, int url_num) {
  int i = 0;
  char *local_ip = get_local_ip();
  char *local_app = orientsec_get_provider_AppName();
  char *app = NULL;
  for (i = 0; i < url_num; i++)
  {
    if (urls[i].protocol != NULL && 0 != strcmp(urls[i].protocol, ORIENTSEC_GRPC_CONFIGURATOR_PROTOCOL))
    {
      continue;
    }
    app = url_get_parameter_v2(urls + i, ORIENTSEC_GRPC_REGISTRY_KEY_APPLICATION, NULL);
    if (app && 0 != strcmp(app, local_app))
    {
      continue;
    }
    if ((urls[i].host != NULL && 0 != strcmp(urls[i].host, ORIENTSEC_GRPC_ANY_HOST)) && (0 != strcmp(urls[i].host, local_ip)))
    {
      continue;
    }
    ORIENTSEC_GRPC_SET_BIT(urls[i].flag, ORIENTSEC_GRPC_URL_OVERRIDE_MATCH_POS);
  }
}

#define DAY_IN_MILLS 86400000
//检查服务是否过期
bool check_provider_deprecated(const char *intf) {
  provider_lst *provider_node = p_provider_list_head->next;
  bool ret = false;
  uint64_t curr_log_time = 0;
  char buf[ORIENTSEC_GRPC_BUF_LEN] = { 0 };
  for (; provider_node != NULL; provider_node = provider_node->next)
  {
    if (!provider_node->provider || !provider_node->provider->sInterface)
    {
      continue;
    }
    if (0 == strcmp(intf, provider_node->provider->sInterface))
    {
      break;
    }
  }
  if (provider_node && provider_node->provider)
  {
    if (provider_node->provider->deprecated)
    {
      curr_log_time = orientsec_get_timestamp_in_mills();
      if (provider_node->last_log_time == 0 ||    //第一次被调用，打印日志
	      (curr_log_time - provider_node->last_log_time) > DAY_IN_MILLS)
      {
	provider_node->last_log_time = curr_log_time;
	gpr_log(GPR_ERROR, "当前服务[%s]的信息发生过变更，或者已经有新版本上线", intf);
      }
      ret = true;
    }
  }
  else {
    gpr_log(GPR_ERROR, "非法服务名[%s]", intf);
    ret = false;
  }
  return ret;
}

//provider注册时的订阅函数，根据订阅回调函数更新provider列表相应的属性
void provider_configurators_callback(url_t *urls, int url_num) {
  //根据urls 属性值更新provider链表中的对应属性字段，主要包括
  //default.connections,default_requests,deprecated
  //更新时有如下约束： 确定ip的设置项比通配符0.0.0.0的优先级高
  int i = 0;
  char *local_ip = get_local_ip();
  char *param = NULL;
  char *intf = NULL;
  bool access_protected = false;
  bool need_update = false;
  //预处理
  overrides_preprocess(urls, url_num);
  for (i = 0; i < url_num; i++)
  {
    if (urls[i].protocol != NULL && 0 == strcmp(urls[i].protocol, ORIENTSEC_GRPC_EMPTY_PROTOCOL))
    {
      continue;
    }
    if (!ORIENTSEC_GRPC_CHECK_BIT(urls[i].flag, ORIENTSEC_GRPC_URL_OVERRIDE_MATCH_POS))
    {
      continue;
    }
    // 针对ip 0.0.0.0 对所有provider起作用
    if (urls[i].host != NULL && 0 == strcmp(urls[i].host, ORIENTSEC_GRPC_ANY_HOST))
    {
      param = url_get_parameter_v2(urls + i, ORIENTSEC_GRPC_REGISTRY_KEY_DEFAULT_CONNECTIONS, NULL);
      if (param)
      {
        update_provider_connection(NULL, atoi(param));
      }
      param = url_get_parameter_v2(urls + i, ORIENTSEC_GRPC_REGISTRY_KEY_DEFAULT_REQUESTS, NULL);
      if (param)
      {
        update_provider_request(NULL, atoi(param));
      }
      param = url_get_parameter_v2(urls + i, ORIENTSEC_GRPC_REGISTRY_KEY_DEPRECATED, NULL);
      if (param)
      {
	update_provider_deprecated(NULL, (0 == strcmp(param, "true")) ? true : false);
	check_provider_deprecated((urls + i)->path);
      }
      param = url_get_parameter_v2(urls + i, ORIENTSEC_GRPC_REGISTRY_KEY_ACCESS_PROTECTED, NULL);
      if (param)
      {
        need_update = true;
        access_protected = (0 == strcmp(param, "true")) ? true : false;
        update_provider_access_protected(NULL, access_protected);
      }
    }
  }

  for (i = 0; i < url_num; i++)
  {
    if (urls[i].protocol != NULL && 0 == strcmp(urls[i].protocol, ORIENTSEC_GRPC_EMPTY_PROTOCOL))
    {
      continue;
    }
    if (!ORIENTSEC_GRPC_CHECK_BIT(urls[i].flag, ORIENTSEC_GRPC_URL_OVERRIDE_MATCH_POS))
    {
      continue;
    }
    // 针对某个服务，指定ip的provider起作用
    intf = url_get_parameter_v2(urls + i, ORIENTSEC_GRPC_REGISTRY_KEY_INTERFACE, NULL);
    if (intf && (urls[i].host != NULL && 0 == strcmp(urls[i].host, get_local_ip())))
    {
      param = url_get_parameter_v2(urls + i, ORIENTSEC_GRPC_REGISTRY_KEY_DEFAULT_CONNECTIONS, NULL);
      if (param)
      {
        update_provider_connection(intf, atoi(param));
      }
      param = url_get_parameter_v2(urls + i, ORIENTSEC_GRPC_REGISTRY_KEY_DEFAULT_REQUESTS, NULL);
      if (param)
      {
	update_provider_request(intf, atoi(param));
      }
      param = url_get_parameter_v2(urls + i, ORIENTSEC_GRPC_REGISTRY_KEY_DEPRECATED, NULL);
      if (param)
      {
	update_provider_deprecated(intf, (0 == strcmp(param, "true")) ? true : false);
      }
      param = url_get_parameter_v2(urls + i, ORIENTSEC_GRPC_REGISTRY_KEY_ACCESS_PROTECTED, NULL);
      if (param)
      {
	need_update = true;
	access_protected = (0 == strcmp(param, "true")) ? true : false;
	update_provider_access_protected(intf, access_protected);
      }
    }
  }
  //注册路由规则
  //1、生成url串  2、调用注册方法或取消注册方法
  //------------addbyhuyn------------
  if (need_update == true && intf != NULL) {
    url_t *router_url = url_for_router_from_param(local_ip, intf);
    if (access_protected == true) {
      // 更新了url中的变量，需要修改url字符串并重新注册
      registry(router_url);
    }
    else {
      unregistry(router_url);
    }
    url_full_free(&router_url);
  }
  //----------------------------------
  return;
}

//注册Provider，并订阅configurators目录
void provider_registry(int port, const char *sIntf, const char *sMethods) {
  provider_t *provider = init_and_cache_provider(port, sIntf, sMethods);
  url_t *provider_url = url_from_provider(provider);
  registry(provider_url);

  //根据accese protected属性更新路由规则。
  //----begin----addbyhuyn
  if (provider->access_protected) {
    url_t *router_url = url_for_router_from_param(provider->host, provider->sInterface);
    registry(router_url);
    url_full_free(&router_url);
  }
  //-----end-----
  url_update_parameter(provider_url, ORIENTSEC_GRPC_CATEGORY_KEY, ORIENTSEC_GRPC_CONFIGURATORS_CATEGORY);
  subscribe(provider_url, provider_configurators_callback);
  url_full_free(&provider_url);
}

//注销本应用所有的服务并取消订阅
void providers_unregistry() {
  url_t *provider_url = NULL;
  provider_lst *provider_node = p_provider_list_head->next;
  while (provider_node)
  {
    provider_url = url_from_provider(provider_node->provider);
    unregistry(provider_url);

    url_update_parameter(provider_url, ORIENTSEC_GRPC_CATEGORY_KEY, ORIENTSEC_GRPC_CONFIGURATORS_CATEGORY);
    unsubscribe(provider_url, provider_configurators_callback);
    url_full_free(&provider_url);

    p_provider_list_head->next = provider_node->next;
    free_provider(&(provider_node->provider));
    FREE_PTR(provider_node);
    provider_node = p_provider_list_head->next;
  }
}


//检查指定服务并发连接数满足条件，即 0 < 当前连接数 + 1 <=max,满足条件时，连接数+1，并返回true,否则返回false
bool check_provider_connection(const char *intf) {
  provider_lst *provider_node = p_provider_list_head->next;
  bool ret = false;
  if (!intf)
  {
    if (provider_node && provider_node->provider) {
      gpr_mu_lock(&provider_node->conns_mu);
      //provider_node->provider->default_connections = 2;
      if (0 == provider_node->provider->default_connections)
      {
	provider_node->current_conns++;
        ret = true;
      }
      else if (provider_node->current_conns < provider_node->provider->default_connections)
      {
	provider_node->current_conns++;
	ret = true;
      }
      else {
	ret = false;
      }
      gpr_mu_unlock(&provider_node->conns_mu);
    }
    return ret;
  }
  for (; provider_node != NULL; provider_node = provider_node->next)
  {
    if (!provider_node->provider || !provider_node->provider->sInterface)
    {
      continue;
    }
    if (0 == strcmp(intf, provider_node->provider->sInterface))
    {
      break;
    }
  }
  if (provider_node && provider_node->provider)
  {
    //不需要做连接数控制
    if (0 == provider_node->provider->default_connections)
    {
      provider_node->current_conns = 0;
      return true;
    }
    gpr_mu_lock(&provider_node->conns_mu);

    if (provider_node->current_conns < provider_node->provider->default_connections)
    {
      provider_node->current_conns++;
      ret = true;
    }
    else {
      ret = false;
    }
    gpr_mu_unlock(&provider_node->conns_mu);
  }

  return ret;
}

// 减少provider 并发连接数
void decrease_provider_connection(const char *intf) {
  provider_lst *provider_node = p_provider_list_head->next;
  bool ret = true;
  if (!intf)
  {
    if (provider_node && provider_node->provider)
    {
      gpr_mu_lock(&provider_node->conns_mu);
      if (provider_node->current_conns > 0)
      {
	provider_node->current_conns--;
      }
      gpr_mu_unlock(&provider_node->conns_mu);
    }
    return;
  }
  for (; provider_node != NULL; provider_node = provider_node->next)
  {
    if (!provider_node->provider || !provider_node->provider->sInterface)
    {
      continue;
    }
    if (0 == strcmp(intf, provider_node->provider->sInterface))
    {
      break;
    }
  }
  if (provider_node && provider_node->provider)
  {
    //不需要做连接数控制
    if (0 == provider_node->provider->default_connections)
    {
      return ;
    }
    gpr_mu_lock(&provider_node->conns_mu);
    if (provider_node->current_conns > 0)
    {
      provider_node->current_conns--;
    }
    gpr_mu_unlock(&provider_node->conns_mu);
  }
}

//检查指定服务并发请求数满足条件，即 0 < 当前并发请求数 + 1 <=max,满足条件时，连接数+1，并返回true,否则返回false
bool check_provider_request(const char *intf) {
  provider_lst *provider_node = p_provider_list_head->next;
  bool ret = false;
  for (; provider_node != NULL; provider_node = provider_node->next)
  {
    if (!provider_node->provider || !provider_node->provider->sInterface)
    {
      continue;
    }
    if (0 == strcmp(intf, provider_node->provider->sInterface))
    {
      break;
    }
  }
  if (provider_node && provider_node->provider)
  {
    //不做请求数控制时不需要加锁
    if (0 == provider_node->provider->default_requests) {
      provider_node->current_reqs = 0;  //不计算请求数
      return true;
    }
    gpr_mu_lock(&provider_node->reqs_mu);
    if (provider_node->current_reqs < provider_node->provider->default_requests)
    {
      provider_node->current_reqs++;
      ret = true;
    }
    else {
      ret = false;
    }
    gpr_mu_unlock(&provider_node->reqs_mu);
  }
  return ret;
}
//减少provider 并发请求数
void decrease_provider_request(const char *intf) {
  provider_lst *provider_node = p_provider_list_head->next;
  bool ret = true;
  for (; provider_node != NULL; provider_node = provider_node->next)
  {
    if (!provider_node->provider || !provider_node->provider->sInterface)
    {
      continue;
    }
    if (0 == strcmp(intf, provider_node->provider->sInterface))
    {
      break;
    }
  }
  if (provider_node && provider_node->provider)
  {
    //首先判读是否进行请求数控制。
    if (0 == provider_node->provider->default_requests) {
      return true;
    }
    gpr_mu_lock(&provider_node->reqs_mu);
    if (provider_node->current_reqs > 0)
    {
      provider_node->current_reqs--;
    }
    gpr_mu_unlock(&provider_node->reqs_mu);
  }
}

//提交provider到链表中
void add_provider_to_list(provider_t *provider) {
}

//更新provider 最大并发连接数
void update_provider_connection(const char *intf, int conns) {
  provider_lst *provider_node = p_provider_list_head->next;
  if (conns < 0)
  {
    return;
  }
  for (; provider_node != NULL; provider_node = provider_node->next)
  {
    if (!provider_node->provider || !provider_node->provider->sInterface)
    {
      continue;
    }
    if (intf == NULL)
    {
      gpr_mu_lock(&provider_node->conns_mu);
      provider_node->provider->default_connections = conns;
      gpr_mu_unlock(&provider_node->conns_mu);
    }
    else if (0 == strcmp(intf, provider_node->provider->sInterface))
    {
      gpr_mu_lock(&provider_node->conns_mu);
      provider_node->provider->default_connections = conns;
      gpr_mu_unlock(&provider_node->conns_mu);
      break;
    }
  }
}

//更新provider 最大并发请求数
void update_provider_request(const char *intf, int req) {
  provider_lst *provider_node = p_provider_list_head->next;
  if (req < 0)
  {
    return;
  }
  for (; provider_node != NULL; provider_node = provider_node->next)
  {
    if (!provider_node->provider || !provider_node->provider->sInterface)
    {
      continue;
    }
    if (intf == NULL)
    {
      gpr_mu_lock(&provider_node->reqs_mu);
      provider_node->provider->default_requests = req;
      gpr_mu_unlock(&provider_node->reqs_mu);
    }
    else if (0 == strcmp(intf, provider_node->provider->sInterface))
    {
      gpr_mu_lock(&provider_node->reqs_mu);
      provider_node->provider->default_requests = req;
      gpr_mu_unlock(&provider_node->reqs_mu);
      break;
    }
  }
}

//更新provider deprecated属性
void update_provider_deprecated(const char *intf, bool deprecated) {
  provider_lst *provider_node = p_provider_list_head->next;
  bool ret = true;
  for (; provider_node != NULL; provider_node = provider_node->next)
  {
    if (!provider_node->provider || !provider_node->provider->sInterface)
    {
      continue;
    }
    if (intf == NULL)
    {
      provider_node->provider->deprecated = deprecated;
    }
    else if (0 == strcmp(intf, provider_node->provider->sInterface))
    {
      provider_node->provider->deprecated = deprecated;
      break;
    }
  }

}

// update provider access protected property
void update_provider_access_protected(const char* intf, bool access_protected) {
  provider_lst* provider_node = p_provider_list_head->next;
  bool ret = true;
  for (; provider_node != NULL; provider_node = provider_node->next) {
    if (!provider_node->provider || !provider_node->provider->sInterface) {
      continue;
    }
    if (intf == NULL) {
      provider_node->provider->access_protected = access_protected;
    } else if (0 == strcmp(intf, provider_node->provider->sInterface)) {
      provider_node->provider->access_protected = access_protected;
      break;
    }
  }
}
