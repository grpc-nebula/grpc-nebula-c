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
 *    Modified: Jianbin Yang
 */

#include "orientsec_provider_intf.h"
#include <grpc/support/alloc.h>
#include <grpc/support/log.h>
#include <grpc/support/sync.h>
#include <src/core/lib/gpr/spinlock.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <map>
#include <thread>
#include <time.h>
#include <assert.h>
#include "orientsec_grpc_properties_constants.h"
#include "orientsec_grpc_properties_tools.h"
#include "orientsec_grpc_registy_intf.h"
#include "orientsec_grpc_utils.h"
#include "registry_contants.h"
#include "registry_utils.h"
#include "zk_registry_service.h"

// zookeeper register,detect every 3s, times 3 days
const int TIMES_MAX_TRY = 86400;
const int DAY_IN_MILLS = 86400000;
 // 声明
void update_provider_is_master(bool, const char*, const char*);

struct provider_lst {
  provider_lst():provider(NULL),
    next(NULL),
    current_reqs(0), 
    last_log_time(0){
    //conns = new map<std::string, int>();
  }
  ~provider_lst(){}
  provider_t* provider;
  struct provider_lst* next;
  std::map<std::string, int> *conns;
  // int current_conns;  //当前并发连接数
  int current_reqs;        //当前并发请求数
  uint64_t last_log_time;  //最后一次打印日志时间
  gpr_mu conns_mu;
  gpr_mu reqs_mu;
} ;

static provider_lst provider_list_head;
static void init_providr_list_head(){
  //provider_list_head = new provider_lst();
  //std::map<std::string, int> mapStudent;
  provider_list_head.conns = new std::map<std::string,int>();
}

static provider_lst* p_provider_list_head = &provider_list_head;
static bool provider_lst_inited = false;

void cache_provider_node(provider_t* provider) {
  init_providr_list_head();
  provider_lst* pl = (provider_lst*)gpr_zalloc(sizeof(provider_lst));
  if (pl) {
    pl->provider = provider;
    pl->conns = new std::map<std::string,int>();
    pl->current_reqs = 0;
    pl->last_log_time = 0;
    gpr_mu_init(&pl->conns_mu);
    gpr_mu_init(&pl->reqs_mu);
    pl->next = p_provider_list_head->next;
    p_provider_list_head->next = pl;
  }
}

// support for grouping and grading information with service name
char* obtain_provider_group_with_service(const char* service_name) {
  if ((service_name == NULL) || strlen(service_name) == 0) return NULL;
  char* out = NULL;
  char* search_key = (char*)gpr_zalloc(128 * sizeof(char));
  char* serv_group = (char*)gpr_zalloc(128);
  char* prov_group = (char*)gpr_zalloc(128);
  strcpy(search_key, ORIENTSEC_GRPC_PROPERTIES_P_GROUP);
  strcat(search_key, "[");
  strcat(search_key, service_name);
  strcat(search_key, "]");
  orientsec_grpc_properties_get_value(search_key, NULL, serv_group);
  orientsec_grpc_properties_get_value(ORIENTSEC_GRPC_PROPERTIES_P_GROUP, NULL,
                                      prov_group);
  // 服务名配置的group为空
  if (strlen(serv_group) == 0) {
    gpr_free(serv_group);
    gpr_free(search_key);
    return prov_group;
  } else {
    gpr_free(prov_group);
    gpr_free(search_key);
    return serv_group;
  }
}
provider_t* init_and_cache_provider(int port, const char* intf,
                                    const char* methods) {
  provider_t* provider = new_provider();
  RegCode conf_code= init_provider(provider);
  if (APPL_NOT_CONF == conf_code) {
    gpr_log(
        GPR_ERROR,
        "Register service[%s] failed: [common.application] not configured.\n",
        intf);
    //assert(OK == conf_code);
    abort();
  }
  if (PROJ_NOT_CONF == conf_code) {
    gpr_log(
        GPR_ERROR,
        "Register service[%s] failed: [common.project] not configured.\n",intf);
    abort();
  }
  if (OWNR_NOT_CONF == conf_code) {
    gpr_log(GPR_ERROR,
            "Register service[%s] failed: [common.owner] not configured.\n",intf);
    abort();
  }  
  if (VSON_NOT_CONF == conf_code) {
    gpr_log(GPR_ERROR,
            "Register service[%s] failed: [provider.version] not configured.\n",
            intf);
    abort();
  } 

  char* ptr = get_provider_registry_ip();
  if (ptr) {
    size_t len = strlen(ptr);
    snprintf(provider->host,
             len < sizeof(provider->host) ? len + 1 : sizeof(provider->host),
             "%s", ptr);
  }
  // obtain grouping and grading information from config file
  char* pro_group_info = obtain_provider_group_with_service(intf);
  if (pro_group_info) {
    size_t lenth = strlen(pro_group_info);
    snprintf(
        provider->group,
        lenth < sizeof(provider->group) ? lenth + 1 : sizeof(provider->group),
        "%s", pro_group_info);
  }
  gpr_free(pro_group_info);

  provider->pid = grpc_getpid();
  // calculate port
  int registry_port = get_provider_registry_port();
  if (0 != registry_port)
    provider->port = registry_port;
  else
    provider->port = port;
  provider->timestamp = orientsec_get_timestamp_in_mills();
  provider->sInterface = gprc_strdup(intf);
  provider->methods = gprc_strdup(methods);
  cache_provider_node(provider);

  return provider;
}

//预处理，只标记规则IP为0.0.0.0，与本地ip相同的规则，如果规则中指定了application，则application值也需要
//和本地配置application相同,
void overrides_preprocess(url_t* urls, int url_num) {
  int i = 0;
  char* local_ip = get_provider_registry_ip();
  char* local_app = orientsec_get_provider_appname();
  char* app = NULL;
  for (i = 0; i < url_num; i++) {
    if (urls[i].protocol != NULL &&
        0 != strcmp(urls[i].protocol, ORIENTSEC_GRPC_CONFIGURATOR_PROTOCOL)) {
      continue;
    }
    app = url_get_parameter_v2(urls + i,
                               ORIENTSEC_GRPC_REGISTRY_KEY_APPLICATION, NULL);
    if (app && 0 != strcmp(app, local_app)) {
      continue;
    }
    if ((urls[i].host != NULL &&
         0 != strcmp(urls[i].host, ORIENTSEC_GRPC_ANY_HOST)) &&
        (0 != strcmp(urls[i].host, local_ip))) {
      continue;
    }
    ORIENTSEC_GRPC_SET_BIT(urls[i].flag, ORIENTSEC_GRPC_URL_OVERRIDE_MATCH_POS);
  }
}


//检查服务是否过期
bool check_provider_deprecated(const char* intf) {
  provider_lst* provider_node = p_provider_list_head->next;
  bool ret = false;
  uint64_t curr_log_time = 0;
  char buf[ORIENTSEC_GRPC_BUF_LEN] = {0};
  for (; provider_node != NULL; provider_node = provider_node->next) {
    if (!provider_node->provider || !provider_node->provider->sInterface) {
      continue;
    }
    if (0 == strcmp(intf, provider_node->provider->sInterface)) {
      break;
    }
  }
  if (provider_node && provider_node->provider) {
    if (provider_node->provider->deprecated) {
      curr_log_time = orientsec_get_timestamp_in_mills();
      if (provider_node->last_log_time == 0 ||  //第一次被调用，打印日志
          (curr_log_time - provider_node->last_log_time) > DAY_IN_MILLS) {
        provider_node->last_log_time = curr_log_time;
        gpr_log(GPR_ERROR, "当前服务[%s]的信息发生过变更，或者已经有新版本上线",
                intf);
      }
      ret = true;
    }
  } else {
    gpr_log(GPR_ERROR, "Illegal service name [%s]", intf);
    ret = false;
  }
  return ret;
}

// provider注册时的订阅函数，根据订阅回调函数更新provider列表相应的属性
void provider_configurators_callback(url_t* urls, int url_num) {
  //根据urls 属性值更新provider链表中的对应属性字段，主要包括
  // default.connections,default_requests,deprecated
  //更新时有如下约束： 确定ip的设置项比通配符0.0.0.0的优先级高
  int i = 0;
  char* local_ip = get_provider_registry_ip();
  char* param = NULL;
  char* intf = NULL;
  char hostip[32] = {0};
  bool access_protected = false;
  bool is_active = true;
  bool need_update = false;
  //预处理
  overrides_preprocess(urls, url_num);

  for (i = 0; i < url_num; i++) {
    if (urls[i].protocol != NULL &&
        0 == strcmp(urls[i].protocol, ORIENTSEC_GRPC_EMPTY_PROTOCOL)) {
      continue;
    }
    if (!ORIENTSEC_GRPC_CHECK_BIT(urls[i].flag,
                                  ORIENTSEC_GRPC_URL_OVERRIDE_MATCH_POS)) {
      continue;
    }
    // 针对ip 0.0.0.0 对所有provider起作用
    if (urls[i].host != NULL &&
        0 == strcmp(urls[i].host, ORIENTSEC_GRPC_ANY_HOST)) {
      param = url_get_parameter_v2(
          urls + i, ORIENTSEC_GRPC_REGISTRY_KEY_DEFAULT_CONNECTIONS, NULL);
      if (param) {
        update_provider_connection(NULL, atoi(param));
      }
      param = url_get_parameter_v2(
          urls + i, ORIENTSEC_GRPC_REGISTRY_KEY_DEFAULT_REQUESTS, NULL);
      if (param) {
        update_provider_request(NULL, atoi(param));
      }
      param = url_get_parameter_v2(
          urls + i, ORIENTSEC_GRPC_REGISTRY_KEY_DEPRECATED, NULL);
      if (param) {
        update_provider_deprecated(NULL,
                                   (0 == strcmp(param, "true")) ? true : false);
        check_provider_deprecated((urls + i)->path);
      }
      param = url_get_parameter_v2(
          urls + i, ORIENTSEC_GRPC_REGISTRY_KEY_ACCESS_PROTECTED, NULL);
      if (param) {
        need_update = true;
        access_protected = (0 == strcmp(param, "true")) ? true : false;
        update_provider_access_protected(NULL, access_protected);
      }
      param = url_get_parameter_v2(urls + i, ORIENTSEC_GRPC_REGISTRY_KEY_MASTER,
                                   NULL);
      if (param) {
        is_active =
            (0 != strcmp(param, "false")) ? true : false;  // default:true
        // update active/standby property
        update_provider_is_master(is_active, NULL, NULL);
      }
    }
  }

  for (i = 0; i < url_num; i++) {
    if (urls[i].protocol != NULL &&
        0 == strcmp(urls[i].protocol, ORIENTSEC_GRPC_EMPTY_PROTOCOL)) {
      continue;
    }
    if (!ORIENTSEC_GRPC_CHECK_BIT(urls[i].flag,
                                  ORIENTSEC_GRPC_URL_OVERRIDE_MATCH_POS)) {
      continue;
    }
    // 针对某个服务，指定ip的provider起作用
    intf = url_get_parameter_v2(urls + i, ORIENTSEC_GRPC_REGISTRY_KEY_INTERFACE,
                                NULL);
    if (intf && (urls[i].host != NULL && 0 == strcmp(urls[i].host, local_ip))) {
      param = url_get_parameter_v2(
          urls + i, ORIENTSEC_GRPC_REGISTRY_KEY_DEFAULT_CONNECTIONS, NULL);
      if (param) {
        update_provider_connection(intf, atoi(param));
      }
      param = url_get_parameter_v2(
          urls + i, ORIENTSEC_GRPC_REGISTRY_KEY_DEFAULT_REQUESTS, NULL);
      if (param) {
        update_provider_request(intf, atoi(param));
      }
      param = url_get_parameter_v2(
          urls + i, ORIENTSEC_GRPC_REGISTRY_KEY_DEPRECATED, NULL);
      if (param) {
        update_provider_deprecated(intf,
                                   (0 == strcmp(param, "true")) ? true : false);
      }
      param = url_get_parameter_v2(
          urls + i, ORIENTSEC_GRPC_REGISTRY_KEY_ACCESS_PROTECTED, NULL);
      if (param) {
        need_update = true;
        access_protected = (0 == strcmp(param, "true")) ? true : false;
        update_provider_access_protected(intf, access_protected);
      }
      param = url_get_parameter_v2(urls + i, ORIENTSEC_GRPC_REGISTRY_KEY_MASTER,
                                   NULL);
      if (param) {
        is_active = (0 == strcmp(param, "true")) ? true : false;
        strcpy(hostip, urls[i].host);
        update_provider_is_master(is_active, intf, urls[i].host);
      }
    }
  }
  //注册路由规则
  // 1、生成url串  2、调用注册方法或取消注册方法
  //------------addbyhuyn------------
  if (need_update == true && intf != NULL) {
    url_t* router_url = url_for_router_from_param(local_ip, intf);
    if (access_protected == true) {
      // 更新了url中的变量，需要修改url字符串并重新注册
      registry(router_url);
    } else {
      unregistry(router_url);
    }
    url_full_free(&router_url);
  }
  //----------------------------------
  return;
}

// adding property into url
static void orientsec_add_property_into_url_inner(url_t* url, const char* key,
                                                  const char* value) {
  char* key_ptr = NULL;
  for (int i = 0; i < url->params_num; i++) {
    key_ptr = url->parameters[i].key;
    if (key_ptr) {
      continue;
    } else {
      url->parameters[i].key = gprc_strdup(key);
      url->parameters[i].value = gprc_strdup(value);
      break;
    }
  }
  return;
}
// add service public/private property into url
static void orientsec_add_property_into_url(url_t* url, zookeeper_reg reg) {
  const char *value = {0};
  if (PUBLIC_REG == reg) {
    value = "public";
  } else if (PRIVATE_REG == reg) {
    value = "private";
  } else if (HYBRID_REG)
    value = "public,private";

  // optimized code
  orientsec_add_property_into_url_inner(
      url, ORIENTSEC_GRPC_REGISTRY_KEY_SERVICE_TYPE, value);

  return;
}

static void grpc_provider_zk_registry_bg(url_t* url_clone, url_t* route_url,int port,
                                    const char* verison,bool access) {
  int last_time = 0;
  int now = 0;
  int times = 0;
  while (times < TIMES_MAX_TRY) {
    now = clock() / CLOCKS_PER_SEC;
    // try every 3 seconds
    if (now - last_time > 3) {
      orientsec_grpc_registry_zk_intf_init();
      // init zk public/private parameter when registry
      zk_prov_reg_init(url_clone->path);
      zookeeper_reg local_reg = PUBLIC_REG;
      local_reg = get_prov_reg_scheme();
      // add service public/private property into url
      orientsec_add_property_into_url(url_clone, local_reg);

      // adding real ip and port into url
      char* real_ip = get_local_ip();
      char c_real_port[8] = {0};
      sprintf(c_real_port, "%d", port);
      orientsec_add_property_into_url_inner(
          url_clone, ORIENTSEC_GRPC_REGISTRY_KEY_REAL_IP, real_ip);
      orientsec_add_property_into_url_inner(
          url_clone, ORIENTSEC_GRPC_REGISTRY_KEY_REAL_PORT, c_real_port);

      // obtain frame version info
      char* version = grpc_verion_format(verison);
      orientsec_add_property_into_url_inner(
          url_clone, ORIENTSEC_GRPC_REGISTRY_KEY_FRAME_VERSION, version);
      // release allocated memory
      free(version);
      // provider register
      registry(url_clone);

      if (!zk_get_create_node_flag()) { // not registry success
        gpr_log(GPR_ERROR,
                "Register in next 3 second......\n");
        // increase times
        last_time = now;
        times++;
        continue;
      }

      if (access) {
        registry(route_url);
      }
      url_update_parameter(url_clone, ORIENTSEC_GRPC_CATEGORY_KEY,
                           ORIENTSEC_GRPC_CONFIGURATORS_CATEGORY);
      subscribe(url_clone, provider_configurators_callback);

      // registre successfully, free memory, return and end of thread
      url_full_free(&url_clone);
      gpr_free((void*)verison);
      if (access && (route_url != NULL)) url_full_free(&route_url);
      gpr_log(GPR_ERROR, "Register successfully......thread exit....\n");
      return;
    } 

  }
  // 线程安全返回
  if (TIMES_MAX_TRY == times)
    gpr_log(GPR_ERROR,
            "Provider could not registered in corressponding zookeeper.\n");

  url_full_free(&url_clone);
  gpr_free((void*)verison);
  if (access && (route_url != NULL)) url_full_free(&route_url);
  gpr_log(GPR_ERROR, "Register failed.time was over......Thread exit....\n");
  return;
}

//注册Provider，并订阅configurators目录
void provider_registry(int port, const char* sIntf, const char* sMethods,
                       const char* grpc_verison) {
  provider_t* provider = init_and_cache_provider(port, sIntf, sMethods);
  url_t* provider_url = url_from_provider(provider);
  orientsec_grpc_registry_zk_intf_init();
  // init zk public/private parameter when registry
  zk_prov_reg_init(provider_url->path);
  zookeeper_reg local_reg = PUBLIC_REG;
  local_reg = get_prov_reg_scheme();
  // add service public/private property into url
  orientsec_add_property_into_url(provider_url, local_reg);

  // adding real ip and port into url
  char* real_ip = get_local_ip();
  char c_real_port[8]={0};
  sprintf(c_real_port, "%d", port);
  orientsec_add_property_into_url_inner(
      provider_url, ORIENTSEC_GRPC_REGISTRY_KEY_REAL_IP, real_ip);
  orientsec_add_property_into_url_inner(
      provider_url, ORIENTSEC_GRPC_REGISTRY_KEY_REAL_PORT, c_real_port);

  // obtain frame version info
  char* version = grpc_verion_format(grpc_verison);
  orientsec_add_property_into_url_inner(
      provider_url, ORIENTSEC_GRPC_REGISTRY_KEY_FRAME_VERSION, version);
  // release allocated memory
  free(version);
  // provider register
  registry(provider_url);
  // create deteach thread when registry failed
  if (!zk_get_create_node_flag()) { 
    // url clone
    url_t* pro_url = (url_t*)gpr_zalloc(sizeof(url_t));
    url_init(pro_url);
    url_clone(provider_url, pro_url);
    url_t* router_url = NULL;
    bool acc = false;
    if (provider->access_protected) {
      url_t* router_url =
          url_for_router_from_param(provider->host, provider->sInterface);
      acc = true;
    } 
    // transport char *
    char* ver = (char*)gpr_zalloc(16);
    strcpy(ver,grpc_verison);
    std::thread zk_registry_agent_thread(grpc_provider_zk_registry_bg,
                                         pro_url,router_url,
                                         port,ver,acc);
    gpr_log(GPR_ERROR, "zookeeper registry failed,new thread was created [%ld]\n",
            std::this_thread::get_id());
    //zk_registry_agent_thread.detach();
    zk_registry_agent_thread.detach();
  }

  //根据accese protected属性更新路由规则。
  //----begin----addbyhuyn
  if (provider->access_protected) {
    url_t* router_url =
        url_for_router_from_param(provider->host, provider->sInterface);
    registry(router_url);
    url_full_free(&router_url);
  }
  //-----end-----
  url_update_parameter(provider_url, ORIENTSEC_GRPC_CATEGORY_KEY,
                       ORIENTSEC_GRPC_CONFIGURATORS_CATEGORY);
  subscribe(provider_url, provider_configurators_callback);
  url_full_free(&provider_url);
}

//注销本应用所有的服务并取消订阅
void providers_unregistry() {
  url_t* provider_url = NULL;
  provider_lst* provider_node = p_provider_list_head->next;
  while (provider_node) {
    provider_url = url_from_provider(provider_node->provider);
    unregistry(provider_url);

    url_update_parameter(provider_url, ORIENTSEC_GRPC_CATEGORY_KEY,
                         ORIENTSEC_GRPC_CONFIGURATORS_CATEGORY);
    unsubscribe(provider_url, provider_configurators_callback);
    url_full_free(&provider_url);

    p_provider_list_head->next = provider_node->next;
    free_provider(&(provider_node->provider));
    FREE_PTR(provider_node);
    provider_node = p_provider_list_head->next;
  }
}
// obtain client ip address
static char* obtain_ip_addr(const char* client_ip){
  const char* host_start;
  char* host = (char*)gpr_zalloc(32);
  const char* colon = strchr(client_ip, ':');
  //if (colon != nullptr && strchr(colon + 1, ':') == nullptr)
  if (colon != nullptr) {
    /* Exactly 1 colon.  Split into host:port. */
    host_start = colon + 1;
    //host = strchr(host_start, ':'); 
    // pNext = (char *)strtok(src,separator);
    host = (char*)strtok((char*)host_start, ":");
    return host;
  }
  return NULL;
}

//检查指定服务并发连接数满足条件，即 0 < 当前连接数 + 1
//<=max,满足条件时，连接数+1，并返回true,否则返回false
bool check_provider_connection(const char* intf, const char* client) {
  provider_lst* provider_node = p_provider_list_head->next;
  bool ret = false;
  char* clientip= obtain_ip_addr(client);

  // 查找client对应的连接数
  auto iter = provider_node->conns->find(clientip);

  if (iter != provider_node->conns->end()) {
    // find corresponding client info
    if (!intf) {
      if (provider_node && provider_node->provider) {
        gpr_mu_lock(&provider_node->conns_mu);
        if (0 == provider_node->provider->default_connections) {
          // provider_node->current_conns++;
          iter->second = 0;
          ret = true;
        } else if (iter->second <
                   provider_node->provider->default_connections) {
          iter->second++;
          ret = true;
        } else {
          gpr_log(GPR_ERROR, "connection nums reach MAX provider connection:%d",
                  provider_node->provider->default_connections);
          ret = false;
        }
        gpr_mu_unlock(&provider_node->conns_mu);
      }
      return ret;
    }
    for (; provider_node != NULL; provider_node = provider_node->next) {
      if (!provider_node->provider || !provider_node->provider->sInterface) {
        continue;
      }
      if (0 == strcmp(intf, provider_node->provider->sInterface)) {
        break;
      }
    }
    if (provider_node && provider_node->provider) {
      //不需要做连接数控制
      if (0 == provider_node->provider->default_connections) {
        iter->second = 0;
        return true;
      }
      gpr_mu_lock(&provider_node->conns_mu);

      if (iter->second < provider_node->provider->default_connections) {
        iter->second++;
        ret = true;
      } else {
        gpr_log(GPR_ERROR,
                "connection nums reach MAX provider default connection:%d",
                provider_node->provider->default_connections);
        ret = false;
      }
      gpr_mu_unlock(&provider_node->conns_mu);
    }
   

  } else {
    // 如果没有找到，则插入键值对，并返回true
    provider_node->conns->insert(std::make_pair(clientip, 1));
    ret = true;
  } 
  // free clientip,not free here
  // gpr_free(clientip);
  return ret; 
}

  // 减少provider 并发连接数
  void decrease_provider_connection(const char* intf,const char * client) {
    provider_lst* provider_node = p_provider_list_head->next;
    bool ret = true;
    char* clientip = obtain_ip_addr(client);

    // 查找client对应的连接数
    auto iter = provider_node->conns->find(clientip);

    if (iter != provider_node->conns->end()) {
      if (!intf) {
        if (provider_node && provider_node->provider) {
          gpr_mu_lock(&provider_node->conns_mu);
          /*if (provider_node->current_conns > 0) {
            provider_node->current_conns--;
          }*/
          if (iter->second > 0) {
            iter->second--;
          }
          gpr_mu_unlock(&provider_node->conns_mu);
        }
        return;
      }
      for (; provider_node != NULL; provider_node = provider_node->next) {
        if (!provider_node->provider || !provider_node->provider->sInterface) {
          continue;
        }
        if (0 == strcmp(intf, provider_node->provider->sInterface)) {
          break;
        }
      }
      if (provider_node && provider_node->provider) {
        //不需要做连接数控制
        if (0 == provider_node->provider->default_connections) {
          return;
        }
        gpr_mu_lock(&provider_node->conns_mu);
        if (iter->second > 0) {
          iter->second--;
        }
        gpr_mu_unlock(&provider_node->conns_mu);
      }
    }
    return;
  }

  //检查指定服务并发请求数满足条件，即 0 < 当前并发请求数 + 1
  //<=max,满足条件时，连接数+1，并返回true,否则返回false
  bool check_provider_request(const char* intf) {
    provider_lst* provider_node = p_provider_list_head->next;
    bool ret = false;
    for (; provider_node != NULL; provider_node = provider_node->next) {
      if (!provider_node->provider || !provider_node->provider->sInterface) {
        continue;
      }
      if (0 == strcmp(intf, provider_node->provider->sInterface)) {
        break;
      }
    }
    if (provider_node && provider_node->provider) {
      //不做请求数控制时不需要加锁
      if (0 == provider_node->provider->default_requests) {
        provider_node->current_reqs = 0;  //不计算请求数
        return true;
      }
      gpr_mu_lock(&provider_node->reqs_mu);
      if (provider_node->current_reqs <
          provider_node->provider->default_requests) {
        provider_node->current_reqs++;
        ret = true;
      } else {
        gpr_log(GPR_ERROR, "request nums reach MAX provider request num:%d",
                provider_node->provider->default_requests);
        ret = false;
      }
      gpr_mu_unlock(&provider_node->reqs_mu);
    }
    return ret;
  }
  //减少provider 并发请求数
  void decrease_provider_request(const char* intf) {
    provider_lst* provider_node = p_provider_list_head->next;
    bool ret = true;
    for (; provider_node != NULL; provider_node = provider_node->next) {
      if (!provider_node->provider || !provider_node->provider->sInterface) {
        continue;
      }
      if (0 == strcmp(intf, provider_node->provider->sInterface)) {
        break;
      }
    }
    if (provider_node && provider_node->provider) {
      //首先判读是否进行请求数控制。
      if (0 == provider_node->provider->default_requests) {
        return;
      }
      gpr_mu_lock(&provider_node->reqs_mu);
      if (provider_node->current_reqs > 0) {
        provider_node->current_reqs--;
      }
      gpr_mu_unlock(&provider_node->reqs_mu);
    }
  }

  //提交provider到链表中
  void add_provider_to_list(provider_t * provider) {}

  //更新provider 最大并发连接数
  void update_provider_connection(const char* intf, int conns) {
    provider_lst* provider_node = p_provider_list_head->next;
    if (conns < 0) {
      return;
    }
    for (; provider_node != NULL; provider_node = provider_node->next) {
      if (!provider_node->provider || !provider_node->provider->sInterface) {
        continue;
      }
      if (intf == NULL) {
        gpr_mu_lock(&provider_node->conns_mu);
        provider_node->provider->default_connections = conns;
        gpr_mu_unlock(&provider_node->conns_mu);
      } else if (0 == strcmp(intf, provider_node->provider->sInterface)) {
        gpr_mu_lock(&provider_node->conns_mu);
        provider_node->provider->default_connections = conns;
        gpr_mu_unlock(&provider_node->conns_mu);
        break;
      }
    }
  }

  //更新provider 最大并发请求数
  void update_provider_request(const char* intf, int req) {
    provider_lst* provider_node = p_provider_list_head->next;
    if (req < 0) {
      return;
    }
    for (; provider_node != NULL; provider_node = provider_node->next) {
      if (!provider_node->provider || !provider_node->provider->sInterface) {
        continue;
      }
      if (intf == NULL) {
        gpr_mu_lock(&provider_node->reqs_mu);
        provider_node->provider->default_requests = req;
        gpr_mu_unlock(&provider_node->reqs_mu);
      } else if (0 == strcmp(intf, provider_node->provider->sInterface)) {
        gpr_mu_lock(&provider_node->reqs_mu);
        provider_node->provider->default_requests = req;
        gpr_mu_unlock(&provider_node->reqs_mu);
        break;
      }
    }
  }

  //更新provider deprecated属性
  void update_provider_deprecated(const char* intf, bool deprecated) {
    provider_lst* provider_node = p_provider_list_head->next;
    bool ret = true;
    for (; provider_node != NULL; provider_node = provider_node->next) {
      if (!provider_node->provider || !provider_node->provider->sInterface) {
        continue;
      }
      if (intf == NULL) {
        provider_node->provider->deprecated = deprecated;
      } else if (0 == strcmp(intf, provider_node->provider->sInterface)) {
        provider_node->provider->deprecated = deprecated;
        break;
      }
    }
  }

  // update provider access protected property
  void update_provider_access_protected(const char* intf,
                                        bool access_protected) {
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

  // update provider active/standby property
  void update_provider_is_master(bool is_master, const char* intf,
                                 const char* host) {
    provider_lst* provider_node = p_provider_list_head->next;
    char host_in[32] = {0};
    if (host != NULL) strcpy(host_in, host);

    bool ret = true;
    for (; provider_node != NULL; provider_node = provider_node->next) {
      if (!provider_node->provider || !provider_node->provider->sInterface) {
        continue;
      }
      // 不指定服务名，全部provider调整active/standby属性
      if (intf == NULL) {
        provider_node->provider->is_master = is_master;

      } else if (0 == strcmp(intf, provider_node->provider->sInterface)) {
        // 指定服务名，对指定IP provider处理
        if (0 == strcmp(host_in, provider_node->provider->host)) {
          provider_node->provider->is_master = is_master;
          break;
        }
      }
    }
  }
