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
 *    2017/07/11
 *    version 0.0.9
 *    consumer项目提供的接口函数实现
 *    Modified: Jianbin Yang
 */

#include "orientsec_consumer_intf.h"

#include <stdio.h>
#include <string.h>
#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <thread>
#include <vector>
#include <chrono>
#include <limits.h>

#include <grpc/support/alloc.h>
#include <grpc/support/log.h>
#include <grpc/support/sync.h>
#include <src/core/lib/gpr/spinlock.h>
#include "src/core/lib/gpr/host_port.h"
#include "orientsec_grpc_properties_constants.h"
#include "orientsec_grpc_properties_tools.h"
#include "orientsec_grpc_registy_intf.h"
#include "orientsec_grpc_utils.h"
#include "orientsec_types.h"
#include "registry_contants.h"
#include "registry_utils.h"
#include "url.h"
#include "zk_registry_service.h"

#include "condition_router.h"
#include "consistent_hash_lb.h"
#include "failover_utils.h"
#include "orientsec_grpc_common_init.h"
#include "orientsec_grpc_consumer_control_deprecated.h"
#include "orientsec_grpc_consumer_control_group.h"
#include "orientsec_grpc_consumer_control_requests.h"
#include "orientsec_grpc_consumer_control_version.h"
#include "orientsec_grpc_consumer_utils.h"
#include "orientsec_grpc_string_op.h"
#include "orientsec_loadbalance.h"
#include "orientsec_router.h"
#include "pickfirst_lb.h"
#include "requests_controller_utils.h"
#include "round_robin_lb.h"
#include "weight_round_robin_lb.h"

// addbylm
#include "src/core/ext/filters/client_channel/lb_policy_registry.h"

// 全局provider list缓存
static std::map<std::string, provider_t*> g_cache_providers;

// 过滤后的缓存provider，用于建立subchannel，用于负载均衡
static std::map<std::string, provider_t*> g_valid_providers;

//保存某服务名下的router列表
static std::map<std::string, std::vector<router*> > g_valid_routers;
//保存请求某服务名的客户端,用于router中的过滤
static std::map<std::string, std::vector<url_t*> > g_consumers;

//保存客户端对应的负载均衡算法,key值由server_name组成
static std::map<std::string, std::string> g_consumer_lbstragry;
// 基于方法级的负载均衡模式
static std::map<std::string, std::string> g_method_lbstragry;
// 基于方法级的负载均衡算法
static std::map<std::string, std::string> g_method_lbalgorithem;

// 判断是否存在master provider online
static bool g_exist_master = true;
/* 由于主备属性发生变化是否需要resolve,规则：
 * (1) 当服务端列表中全部都是主服务器的时候，服务端列表不发生变化
 * (2) 当服务端列表中全部都是备服务器的时候，服务端列表不发生变化
 * (3) 当服务端列表中既有主服务器也有备服务器的时候，将备服务器从
 *     服务列表中移除出去，只保留主服务器
 */
static bool g_active_standby = false;

static bool g_group_grading = false;

static bool g_subscribe_provider_once = false;

/* used in failover,resolve when connection mode*/
static bool g_need_resolve = false;

/* switch provider when elapse time reach to setting in connection mode*/
static bool g_switch_provider_when_connection = false;
// record the time point of last called
static std::chrono::system_clock::time_point last_call_point =
    std::chrono::system_clock::now();

/* Protects provider_queue */
static gpr_mu g_providers_mu;
/* Allow only one access provider queue at once */
static gpr_spinlock g_checker_providers_mu = GPR_SPINLOCK_STATIC_INITIALIZER;

/* Protects consumer_queue */
static gpr_mu g_consumer_mu;
/* Allow only one access consumer queue at once */
static gpr_spinlock g_checker_consumer_mu = GPR_SPINLOCK_STATIC_INITIALIZER;

static bool g_initialized = false;

static bool g_isrequest_lb_mode = false;  //默认是连接负载均衡
// 是否基于方法的负载均衡
static bool g_method_lb = false;
// 负载均衡模式为connection时，设置连接自动切换的时间(mins)
static int g_conn_switch_time = 0;

const int GROUP_INFO_MAX = 256;
static char group_service[GROUP_INFO_MAX] = {0};

// global failure called retry times
static int failure_retry_times = 0;
// detect every 10s, times whole day
const int TIMES_PER_DAY = 8640;
// detect every 3s, times 3 days
const int TIMES_MAX_TRY = 86400;

static orientsec_grpc_loadbalance* pfLB = new pickfirst_lb();
static orientsec_grpc_loadbalance* rrLB = new round_robin_lb();
static orientsec_grpc_loadbalance* wrrLB = new weight_round_robin_lb();
static conistent_hash_lb* chLB = new conistent_hash_lb();

static failover_utils g_failover_utils;  // 容错切换
static int orientsec_grpc_provider_callback_ok =
    0;  //初始回调时置0 回调结束后置1
static int orientsec_grpc_router_callback_ok = 0;  //初始回调时置0 回调结束后置1

static requests_controller_utils g_request_controller_utils;

#define GRPC_PROVIDERS_LIST_LOCK_START          \
  {                                             \
    gpr_spinlock_lock(&g_checker_providers_mu); \
    gpr_mu_lock(&g_providers_mu);               \
  }

#define GRPC_PROVIDERS_LIST_LOCK_END              \
  {                                               \
    gpr_mu_unlock(&g_providers_mu);               \
    gpr_spinlock_unlock(&g_checker_providers_mu); \
  }

#define GRPC_CONSUMERS_LIST_LOCK_START         \
  {                                            \
    gpr_spinlock_lock(&g_checker_consumer_mu); \
    gpr_mu_lock(&g_consumer_mu);               \
  }

#define GRPC_CONSUMERS_LIST_LOCK_END             \
  {                                              \
    gpr_mu_unlock(&g_consumer_mu);               \
    gpr_spinlock_unlock(&g_checker_consumer_mu); \
  }

void init_providers_list() {
  if (!g_initialized) {
    char buf[ORIENTSEC_GRPC_PROPERTY_KEY_MAX_LEN] = {0};
    int value = 0;
    gpr_mu_init(&g_providers_mu);
    gpr_mu_init(&g_consumer_mu);
    pfLB->set_providers(&g_cache_providers);
    rrLB->set_providers(&g_cache_providers);
    wrrLB->set_providers(&g_cache_providers);
    chLB->set_providers(&g_cache_providers);
    orientsec_grpc_properties_init();
    if (0 == orientsec_grpc_properties_get_value(
                 ORIENTSEC_GRPC_PROPERTIES_C_CONSUMER_SWITH_THRES, NULL, buf)) {
      value = atoi(buf);
      if (value > 0) {
        g_failover_utils.set_switch_threshold(value);
      }
      memset(buf, 0, ORIENTSEC_GRPC_PROPERTY_KEY_MAX_LEN);
    }
    if (0 == orientsec_grpc_properties_get_value(
            ORIENTSEC_GRPC_PROPERTIES_C_CONSUMER_SERVICE_RECOVERY, NULL, buf)) {
      value = atoi(buf);
      if (value > 0) {
        g_failover_utils.set_recovery_time(value);
      }
    }
    memset(buf, 0, ORIENTSEC_GRPC_PROPERTY_KEY_MAX_LEN);
    if (0 == orientsec_grpc_properties_get_value(
                 ORIENTSEC_GRPC_PROPERTIES_C_LB_MODE, NULL, buf)) {
      if (0 == strcmp(buf, ORIENTSEC_GRPC_LB_MODE_REQUEST)) {
        g_isrequest_lb_mode = true;
      } else {
        g_isrequest_lb_mode = false;
      }
    }
    // read connection switch time from config file
    memset(buf, 0, ORIENTSEC_GRPC_PROPERTY_KEY_MAX_LEN);
    if (0 == orientsec_grpc_properties_get_value(
                 ORIENTSEC_GRPC_PROPERTIES_C_CONSUMER_CONNECTION_SWITCH_TIME,
                 NULL, buf)) {
      value = atoi(buf);
      if (value < INT_MAX/60000 && value > 0) {
        g_conn_switch_time = value;
      } else {
        g_conn_switch_time = 10;
      }
    }
    // add by liumin
    memset(buf, 0, ORIENTSEC_GRPC_PROPERTY_KEY_MAX_LEN);
    if (0 == orientsec_grpc_properties_get_value(
                 ORIENTSEC_GRPC_PROPERTIES_C_BACKOFF_MAXSECOND, NULL, buf)) {
      value = atoi(buf);
      if (value > 0) {
        g_failover_utils.set_max_backoff_time(value);
      }
    }
    // end by liumin

    g_initialized = true;
  }
}

bool is_request_loadbalance() { return g_isrequest_lb_mode; }

//  zookeeper:///serviceXXX ==> serviceXXX
char* orientsec_grpc_get_sn_from_target(char* target) {
  if (!target) {
    return NULL;
  }
  std::string tgt = target;
  std::string::size_type startpos = tgt.find("///");
  if (startpos == std::string::npos) {
    return NULL;
  }
  startpos += 3;
  std::string::size_type endpos = tgt.find("?", startpos);
  if (endpos == std::string::npos) {
    endpos = tgt.length();
  }
  std::string sn = tgt.substr(startpos, endpos - startpos);
  return gprc_strdup(sn.c_str());
}

void revoker_providers_list_process(const char* sn) {
  if (!sn) {
    return;
  }
  std::map<std::string, std::vector<url_t*> >::iterator consumerIter =
      g_consumers.find(sn);
  if (consumerIter == g_consumers.end() || consumerIter->second.size() == 0) {
    return;
  }
  std::map<std::string, std::vector<router*> >::iterator routeMapIter =
      g_valid_routers.find(sn);
  std::map<std::string, provider_t*>::iterator provider_lst_iter =
      g_cache_providers.find(sn);
  if (provider_lst_iter == g_cache_providers.end()) {
    return;
  }
  if (routeMapIter != g_valid_routers.end()) {
    //遍历执行路由规则，按指定服务逐条执行路由规则
    for (std::vector<router*>::iterator routeIter =
             routeMapIter->second.begin();
         routeIter != routeMapIter->second.end(); routeIter++) {
      router* route = *routeIter;

      // consumer参数作用是什么  ？
      route->route(provider_lst_iter->second, consumerIter->second[0]);
    }
  } else {  //清除黑名单，为对provider进行恢复
    for (size_t i = 0; i < orientsec_grpc_cache_provider_count_get(); i++) {
      provider_lst_iter->second[i].flag_in_blklist =
          ORIENTSEC_GRPC_PROVIDER_FLAG_IN_BLKLIST_NOT;
    }
  }
}

bool provider_weight_comp(provider_t* p1, provider_t* p2) {
  if (!p1 || !p2) {
    return false;
  }
  if (p1->weight > p2->weight) {
    return true;
  }
  return false;
}

//更新缓存，首先清除原空间，再保存
// reset  是否重置容错标记 1 重置 0 不重置
void updateProvidersCache(const char* service_name, url_t* urls, int url_num,
                          int reset) {
  GRPC_PROVIDERS_LIST_LOCK_START
  // char provider_add[HOST_MAX_LEN];
  bool need_free_cache = false;
  std::set<std::string> new_providers;
  std::set<std::string> old_providers;
  int i = 0;
  // change function to local variable
  int cache_providers_num = orientsec_grpc_cache_provider_count_get();
  int index_free = -1;
  int index_invalid = -1;
  uint64_t time_invalid = 0;
  uint64_t time_now = orientsec_get_timestamp_in_mills();
  std::map<std::string, provider_t*>::iterator provider_lst =
      g_cache_providers.find(service_name);
  if (urls == NULL) {
    if (provider_lst != g_cache_providers.end()) {
      //设置provider标记为无效myurl
      for (i = 0; i < cache_providers_num; i++) {
        provider_lst->second[i].flag_invalid = 1;
        provider_lst->second[i].flag_invalid_timestamp =
            orientsec_get_timestamp_in_mills();
      }
    }
  } else {
    if (provider_lst != g_cache_providers.end()) {
      provider_t provider;

      int index_provider = -1;  //同ip和端口的provider坐标位置
      int index_write = -1;     //可更新的坐标位置
      bool need_update = true;

      //如果服务列表在url列表不存在，代表服务已下线
      for (i = 0; i < cache_providers_num; i++) {
        need_update = true;
        for (int j = 0; j < url_num; j++) {
          //如果指定服务在url列表仍然存在，这不需要下线该服务
          if (provider_lst->second[i].flag_invalid == 0 &&
              (provider_lst->second[i].host != NULL && urls[j].host != NULL &&
               strcmp(provider_lst->second[i].host, urls[j].host) == 0) &&
              (provider_lst->second[i].port != NULL && urls[j].port != NULL &&
               provider_lst->second[i].port == urls[j].port)) {
            need_update = false;
          }
        }
        if (need_update) {
          provider_lst->second[i].flag_invalid = 1;
          provider_lst->second[i].flag_invalid_timestamp = time_now;
        }
      }

      for (int j = 0; j < url_num; j++) {
        init_provider_from_url(&provider, &urls[j]);

        index_free = -1;
        index_write = -1;
        need_update = true;
        for (i = 0; i < cache_providers_num; i++) {
          //查找首个未占用的数组元素
          if (provider_lst->second[i].flag_invalid_timestamp =
                  0 && index_free == -1) {
            index_free = i;
          }

          //最早的无效数据坐标
          if (provider_lst->second[i].flag_invalid == 1) {
            if (index_invalid == -1) {
              time_invalid = provider_lst->second[i].flag_invalid_timestamp;
              index_invalid = i;
            } else if (time_invalid >
                       provider_lst->second[i].flag_invalid_timestamp) {
              time_invalid = provider_lst->second[i].flag_invalid_timestamp;
              index_invalid = i;
            }
          }

          //原来数据位置
          if (provider_lst->second[i].flag_invalid == 0 &&
              strcmp(provider.host, provider_lst->second[i].host) == 0 &&
              provider.port == provider_lst->second[i].port) {
            //重置容错标记
            if (reset) {
              provider_lst->second[i].flag_call_failover = 0;
            }

            //比较时间戳确定是否需要更新
            if (provider.timestamp > provider_lst->second[i].timestamp) {
              index_provider = i;
            } else {
              need_update = false;
              break;
            }
          }
        }

        if (need_update == false) {
          free_provider_v2_ex(&provider);
          continue;
        }

        if (index_free >= 0) {
          index_write = index_free;
        } else if (index_invalid >= 0) {
          index_write = index_invalid;
        } else {  //数组已经放满
          printf("service buffer is full....");
          free_provider_v2_ex(&provider);
          continue;
        }

        //把新服务写入所查找到的位置。
        provider.flag_invalid_timestamp = time_now;
        provider_lst->second[index_write] = provider;

        //标记老数据为无效,并记录无效时间
        if (index_provider >= 0) {
          provider_lst->second[index_provider].flag_invalid = 1;
          provider_lst->second[index_provider].flag_invalid_timestamp =
              time_now;
        }
      }
    } else {
      provider_t* providers =
          (provider_t*)gpr_zalloc(sizeof(provider_t) * cache_providers_num);

      for (i = 0; i < cache_providers_num && i < url_num; i++) {
        init_provider_from_url(&providers[i], &urls[i]);
        providers[i].flag_invalid_timestamp = time_now;
      }
      if (cache_providers_num > url_num) {
        i = url_num;
      } else {
        i = cache_providers_num;
      }
      for (i; i < cache_providers_num; i++) {
        providers[i].flag_invalid = 1;  //初始列表
        providers[i].flag_invalid_timestamp = 0;
      }
      g_cache_providers.insert(std::pair<std::string, provider_t*>(
          std::string(service_name), providers));
    }
  }
  rrLB->reset_cursor(service_name);
  wrrLB->reset_cursor(service_name);
  GRPC_PROVIDERS_LIST_LOCK_END
}

//消费者 providers目录订阅函数
void consumer_providers_callback(url_t* urls, int url_num) {
  char* service_name = url_get_parameter_v2(
      &urls[0], ORIENTSEC_GRPC_REGISTRY_KEY_INTERFACE, NULL);

  if (urls[0].protocol != NULL &&
      0 == strcmp(urls[0].protocol, ORIENTSEC_GRPC_EMPTY_PROTOCOL)) {
    //参数为0
    updateProvidersCache(service_name, NULL, 0, 1);  // 置所有服无效 url_num
                                                     //重置服务状态。
  } else {
    updateProvidersCache(service_name, urls, url_num, 1);
    //重新计算黑白名单标记 ----debug 调试临时关闭
    revoker_providers_list_process(urls[0].path);

    if (g_subscribe_provider_once) {
      // zookeeper provider list changed
      g_active_standby = true;
    } else {
      g_subscribe_provider_once = true;
    }
  }
}

bool route_comp(router* a, router* b) {
  if (((condition_router*)a)->get_priority() >
      ((condition_router*)b)->get_priority()) {
    return true;
  }
  return false;
}

//消费者routers目录订阅函数
void consumer_routers_callback(url_t* urls, int url_num) {
  if (!urls) {
    return;
  }
  bool isEmptyProtocol = false;

  if (urls[0].protocol == NULL) {
    return;
  }
  if (0 == strcmp(urls[0].protocol, ORIENTSEC_GRPC_EMPTY_PROTOCOL)) {
    isEmptyProtocol = true;
  }
  if (urls[0].path == NULL) return;
  std::map<std::string, std::vector<router*> >::iterator routeMapIter =
      g_valid_routers.find(urls[0].path);
  //此处是否可以不清空路由规则  comment by huyn
  //先清空原有规则(如果此时provider同时发生变化可能会造成g_valid_provider列表计算错误)
  if (routeMapIter != g_valid_routers.end()) {
    std::vector<router*>& routes = routeMapIter->second;
    std::vector<router*>::iterator routerVecIter;
    for (routerVecIter = routes.begin(); routerVecIter != routes.end();) {
      delete *routerVecIter;
      routerVecIter = routes.erase(routerVecIter);
    }
  } else {
    std::vector<router*> routes;
    g_valid_routers.insert(
        std::pair<std::string, std::vector<router*> >(urls[0].path, routes));
  }
  routeMapIter = g_valid_routers.find(urls[0].path);
  if (!isEmptyProtocol) {
    for (size_t i = 0; i < url_num; i++) {
      router* route = new condition_router(urls + i);
      routeMapIter->second.push_back(route);
    }
    sort(routeMapIter->second.begin(), routeMapIter->second.end(), route_comp);
  }

  // comment by huyn,此处直接重置g_valid_provider不太合理
  //应该先用临时变量记录改服务对应的provider，并把临时变量的结果和g_valid_provider比对。
  //当黑白名单有变化时，需要重置provider列表中的标志位
  GRPC_PROVIDERS_LIST_LOCK_START
  std::map<std::string, provider_t*>::iterator provider_lst_iter =
      g_cache_providers.find(urls[0].path);
  if (provider_lst_iter != g_cache_providers.end()) {
    for (size_t i = 0; i < orientsec_grpc_cache_provider_count_get(); i++) {
      ORIENTSEC_GRPC_SET_BIT(provider_lst_iter->second[i].flag_in_blklist,
                             ORIENTSEC_GRPC_PROVIDER_FLAG_IN_BLKLIST_NOT);
    }
  }
  GRPC_PROVIDERS_LIST_LOCK_END
  //黑白名单有更新，更新provider列表,设置provider黑名单标记
  revoker_providers_list_process(urls[0].path);
}

bool url_comp(url_t* a, url_t* b) {
  if (!a || !b) {
    return true;
  }
  if (strcmp(a->host, b->host) < 0) {
    return true;
  } else if (strcmp(a->host, b->host) == 0) {
    if (a->port < b->port) {
      return true;
    }
  }

  return false;
}

//消费者 configurator订阅函数
void consumer_configurators_callback(url_t* urls, int url_num) {
  if (urls[0].protocol == NULL || 0 == url_num ||
      0 == strcmp(urls[0].protocol, ORIENTSEC_GRPC_EMPTY_PROTOCOL)) {
    return;
  }
  char* service_name = url_get_parameter_v2(
      &urls[0], ORIENTSEC_GRPC_REGISTRY_KEY_INTERFACE, NULL);

  char* method_name =
      url_get_parameter_v2(&urls[0], ORIENTSEC_GRPC_REGISTRY_KEY_METHODS, NULL);
  std::vector<url_t*> urlVec;
  for (size_t i = 0; i < url_num; i++) {
    urlVec.push_back(urls + i);
  }
  sort(urlVec.begin(), urlVec.end(), url_comp);
  char* ip = NULL;
  int url_port = 0;
  char* param = NULL;
  int weight = 0;
  // add method load balance
  char* lb_strategy = NULL;
  char* lb_algorithem = NULL;

  GRPC_PROVIDERS_LIST_LOCK_START

  std::map<std::string, provider_t*>::iterator provider_lst_iter =
      g_cache_providers.find(service_name);
  // updated weight of provider and deprecated property
  for (size_t i = 0; i < urlVec.size(); i++) {
    ip = urlVec[i]->host;
    url_port = urlVec[i]->port;
    if (provider_lst_iter != g_cache_providers.end()) {
      for (size_t j = 0; j < orientsec_grpc_cache_provider_count_get(); j++) {
        if (((0 == strcmp(ip, ORIENTSEC_GRPC_ANYHOST_VALUE)) ||
             (0 == strcmp(ip, provider_lst_iter->second[j].host))) &&
            (provider_lst_iter->second[j].port == url_port)) {
          param = url_get_parameter_v2(
              urlVec[i], ORIENTSEC_GRPC_REGISTRY_KEY_WEIGHT, NULL);
          if (param) {
            weight = atoi(param);
            if (weight > 0) {
              provider_lst_iter->second[j].weight = weight;
            }
          }
          param = url_get_parameter_v2(
              urlVec[i], ORIENTSEC_GRPC_REGISTRY_KEY_DEPRECATED, NULL);
          if (param) {
            if (0 == strcmp(param, "true")) {
              provider_lst_iter->second[j].deprecated = true;
            } else {
              provider_lst_iter->second[j].deprecated = false;
            }
            consumer_check_provider_deprecated(
                service_name, provider_lst_iter->second[j].deprecated);
          }
          param = url_get_parameter_v2(
              urlVec[i], ORIENTSEC_GRPC_REGISTRY_KEY_MASTER, NULL);
          if (param) {
            if (0 == strcmp(param, "true")) {
              provider_lst_iter->second[j].is_master = true;
              //当有主服务上线时，触发resolve
              g_active_standby = true;
              if (!g_exist_master) g_exist_master = true;
              gpr_log(GPR_DEBUG, "1provider:%s.is_master=%d",
                      provider_lst_iter->second[j].host,
                      provider_lst_iter->second[j].is_master);
            } else {
              provider_lst_iter->second[j].is_master = false;
              // 当有备服务上线时，
              // 1. 有主服务时，不resolve 2. 无主服务时，重新resolve
              g_active_standby = true;
              gpr_log(GPR_DEBUG, "2provider:%s.is_master=%d",
                      provider_lst_iter->second[j].host,
                      provider_lst_iter->second[j].is_master);
            }
          }
          // 更新服务端的服务分组
          param = url_get_parameter_v2(urlVec[i],
                                       ORIENTSEC_GRPC_REGISTRY_KEY_GROUP, NULL);
          if (param) {
            if (0 != strcmp(provider_lst_iter->second[j].group, param)) {
              // group 属性发生改变时，触发resolve
              g_group_grading = true;
              strcpy(provider_lst_iter->second[j].group, param);
            }
          }
        }
      }
    }
  }
  GRPC_PROVIDERS_LIST_LOCK_END

  //更新客户端负载均衡策略配置以及流量控制参数
  char* lb = NULL;
  char* nums = NULL;
  char* meth_lb = NULL;
  char* con_group = NULL;
  std::map<std::string, std::string>::iterator lbIter;
  for (size_t i = 0; i < urlVec.size(); i++) {
    if (urlVec[i]->port > 0) continue;
    if (0 == strcmp(urlVec[i]->host, ORIENTSEC_GRPC_ANYHOST_VALUE) ||
        0 == strcmp(urlVec[i]->host, get_local_ip())) {
      // 动态更新客户端流量控制参数
      nums = url_get_parameter_v2(
          urlVec[i], ORIENTSEC_GRPC_CONSUMER_DEFAULT_REQUEST, NULL);
      if (nums) {
        orientsec_grpc_consumer_update_maxrequest(urlVec[i]->path, atol(nums));
      }

      // 动态更新客户端负载均衡模式 connection/request
      lb = url_get_parameter_v2(urlVec[i], ORIENTSEC_GRPC_REGISTRY_KEY_LB_MODE,
                                NULL);
      lb_algorithem = url_get_parameter_v2(
          urlVec[i], ORIENTSEC_GRPC_REGISTRY_KEY_DEFAULT_LOADBALANCE, NULL);
      // method handle
      meth_lb = url_get_parameter_v2(urlVec[i],
                                     ORIENTSEC_GRPC_REGISTRY_KEY_METHOD, NULL);
      // 基于服务级的负载均衡模式更新
      if (!meth_lb) {
        if (lb) {
          if (strcmp(lb, "request") == 0) {
            if (!g_isrequest_lb_mode) {
              g_isrequest_lb_mode = true;
              g_method_lb = true;  // 仅用作做一次resolve的flag，和方法级无关
            }
          } else if (strcmp(lb, "connection") == 0) {
            if (g_isrequest_lb_mode) {
              g_isrequest_lb_mode = false;
              g_method_lb = true;  // 仅用作做一次resolve的flag，和方法级无关
            }
          }
        }
        if (lb_algorithem) {
          lbIter = g_consumer_lbstragry.find(urlVec[i]->path);

          if (lbIter == g_consumer_lbstragry.end()) {
            g_consumer_lbstragry.insert(std::pair<std::string, std::string>(
                urlVec[i]->path, lb_algorithem));
          } else {
            lbIter->second = std::string(lb_algorithem);
          }
        }
        
      }
      // 监听到客户端的负载均衡模式配置发生变化
      // 基于方法级的lb 更新
      if (meth_lb && lb) {
        // set method level flag
        g_method_lb = true;
        // get lb mode and algorithem from url
        lb_strategy = url_get_parameter_v2(
            urlVec[i], ORIENTSEC_GRPC_REGISTRY_KEY_LB_MODE, NULL);

        // 遍历基于方法的策略map
        if (lb_strategy) {
          // 更新或者添加新的基于方法的策略
          auto method_mode_iter = g_method_lbstragry.find(meth_lb);
          if (method_mode_iter != g_method_lbstragry.end()) {
            method_mode_iter->second = lb_strategy;
          } else {
            g_method_lbstragry.insert(make_pair(meth_lb, lb_strategy));
          }
        }
        if (lb_algorithem) {
          // 更新或者添加新的基于方法的均衡算法
          auto method_algo_iter = g_method_lbalgorithem.find(meth_lb);
          if (method_algo_iter != g_method_lbalgorithem.end()) {
            method_algo_iter->second = lb_algorithem;

          } else {
            g_method_lbalgorithem.insert(make_pair(meth_lb, lb_algorithem));
          }
        }
      }
      // 更新客户端服务分组
      con_group = url_get_parameter_v2(
          urlVec[i], ORIENTSEC_GRPC_REGISTRY_KEY_INVOKE_GROUP, NULL);

      if (con_group) {
        if (0 != strcmp(group_service, con_group)) {
          // group 属性发生改变时，修改consumer的属性，并重新resolve
          g_group_grading = true;
          strcpy(group_service, con_group);
        }
      }
      // 通过zk中configurators配置动态更新客户端调用配置的版本号
      char* version = url_get_parameter_v2(
          urlVec[i], ORIENTSEC_GRPC_CONSUMER_SERVICE_VERSION, NULL);
      char* intf = url_get_parameter_v2(
          urlVec[i], ORIENTSEC_GRPC_REGISTRY_KEY_INTERFACE, NULL);
      if (version && intf) {
        // 更新全局的 g_consumer_service_version
        orientsec_grpc_consumer_control_version_update(intf, version);
      }
      lb = NULL;
      version = NULL;
      intf = NULL;
      meth_lb = NULL;
    }
  }
}

// return the index(or subchannel) from the load balance aglorithm
// modify this interface ( adding hash_info) for consistent hash algorithm
int get_index_from_lb_aglorithm(const char* service_name, provider_t* provider,
                                const int* nums, char* hash_info,
                                const char* stragry) {
  if (service_name == NULL || provider == NULL || nums == 0) return 0;
  if (stragry == NULL) return 0;

  // 1,判断负载均衡模式，确认负载均衡算法
  // std::map<std::string, std::string>::iterator lbIter =
  //    g_consumer_lbstragry.find(service_name);
  // std::string strLbStragry = ORIENTSEC_GRPC_DEFAULT_LB_TYPE;
  // if (lbIter != g_consumer_lbstragry.end()) {
  //  strLbStragry = lbIter->second;
  //}
  // pick_first
  if (0 == strcmp(stragry, ORIENTSEC_GRPC_LB_TYPE_PF)) {
    // 2，进入相应的负载均衡算法中进行计算
    return pfLB->choose_subchannel(service_name, provider, nums);
  }
  // weight_round_robin
  else if (0 == strcmp(stragry, ORIENTSEC_GRPC_LB_TYPE_WRR)) {
    return wrrLB->choose_subchannel(service_name, provider, nums);
  }
  // round_robin
  else if (0 == strcmp(stragry, ORIENTSEC_GRPC_LB_TYPE_RR)) {
    return rrLB->choose_subchannel(service_name, provider, nums);
  } else {
    // std::string arg("ip:port");
    int select_idx =
        chLB->choose_subchannel(service_name, provider, nums, hash_info);
    return select_idx;
  }
  return 0;
}

// obsoleting code commented by yang
provider_t* get_providers_intf(const char* service_name) {
  if (!service_name) {
    return NULL;
  }
  std::map<std::string, std::string>::iterator lbIter =
      g_consumer_lbstragry.find(service_name);
  std::string strLbStragry = ORIENTSEC_GRPC_DEFAULT_LB_TYPE;
  if (lbIter != g_consumer_lbstragry.end()) {
    strLbStragry = lbIter->second;
  }
  if (0 == strcmp(strLbStragry.c_str(), ORIENTSEC_GRPC_LB_TYPE_PF)) {
    return pfLB->choose_provider(service_name);
  } else if (0 == strcmp(strLbStragry.c_str(), ORIENTSEC_GRPC_LB_TYPE_WRR)) {
    return wrrLB->choose_provider(service_name);
  } else {
    return rrLB->choose_provider(service_name);
  }
}

// used in failover
void get_all_providers_by_name(const char* service_name) {
  char query_str[ORIENTSEC_GRPC_PATH_MAX_LEN] = {0};
  size_t i = 0;
  if (!service_name) {
    return;
  }

  int provider_nums = 0;
  int pri = 0;

  strcat(query_str, ORIENTSEC_GRPC_REGISTRY_KEY_GRPC);
  strcat(query_str, "://0.0.0.0/");
  strcat(query_str, service_name);

  strcat(query_str, "?");
  strcat(query_str, ORIENTSEC_GRPC_CATEGORY_KEY);
  strcat(query_str, "=");
  strcat(query_str, ORIENTSEC_GRPC_PROVIDERS_CATEGORY);

  strcat(query_str, "&");
  strcat(query_str, ORIENTSEC_GRPC_REGISTRY_KEY_INTERFACE);
  strcat(query_str, "=");
  strcat(query_str, service_name);

  url_t* query_url = url_parse(query_str);

  url_t* provider_urls = lookup(query_url, &provider_nums, &pri);
  if (provider_urls) {
    updateProvidersCache(service_name, provider_urls, provider_nums, 1);
    for (size_t i = 0; i < provider_nums; i++) {
      url_free(provider_urls + i);
    }
    if (!provider_urls) free(provider_urls);
  }
  url_full_free(&query_url);
  revoker_providers_list_process(service_name);
}

provider_t* consumer_query_providers_one(const char* service_name, int* nums) {
  char query_str[ORIENTSEC_GRPC_PATH_MAX_LEN] = {0};
  size_t i = 0;
  if (!service_name) {
    return NULL;
  }

  int provider_nums = 0;
  provider_t* providers = NULL;
  *nums = 0;
#ifdef DEBUG_DIRECT_QUERY
  strcat(query_str, ORIENTSEC_GRPC_REGISTRY_KEY_GRPC);
  strcat(query_str, "://0.0.0.0/");
  strcat(query_str, service_name);

  strcat(query_str, "?");
  strcat(query_str, ORIENTSEC_GRPC_CATEGORY_KEY);
  strcat(query_str, "=");
  strcat(query_str, ORIENTSEC_GRPC_PROVIDERS_CATEGORY);

  strcat(query_str, "&");
  strcat(query_str, ORIENTSEC_GRPC_REGISTRY_KEY_INTERFACE);
  strcat(query_str, "=");
  strcat(query_str, service_name);

  url_t* query_url = url_parse(query_str);

  url_t* provider_urls = lookup(query_url, &provider_nums);

  *nums = provider_nums;
  if (nums > 0) {
    providers = (provider_t*)gpr_zalloc(sizeof(provider_t) * provider_nums);
    for (i = 0; i < provider_nums; i++) {
      init_provider_from_url(providers + i, provider_urls + i);
    }
  }
  url_full_free(&query_url);
  for (i = 0; i < provider_nums; i++) {
    url_free(provider_urls + i);
  }
  free(provider_urls);
#else

  GRPC_PROVIDERS_LIST_LOCK_START
  providers = get_providers_intf(service_name);
  if (providers) {
    *nums = 1;
  }
  GRPC_PROVIDERS_LIST_LOCK_END

#endif
  return providers;
}

// 在cc_start_transport_stream_op_batch中查看是否需要resolve
bool orientsec_need_resolved() { return g_need_resolve; }

void orientsec_need_resolved_reset() { g_need_resolve = false; }

bool orientsec_group_grade_changed() { return g_group_grading; }

void orientsec_group_grade_reset() { g_group_grading = false; }

bool orientsec_active_standby_changed() { return g_active_standby; }

void orientsec_active_standby_reset() { g_active_standby = false; }

bool orientsec_method_lb_changed() { return g_method_lb; }

void orientset_method_lb_reset() { g_method_lb = false; }

// 判断provider是否提供此方法的调用
static bool method_check(provider_t* provider, const char* method_name,
                         const int length) {
  std::string methods = provider->methods;
  // std::string methods("SayHello1,SayHello,SayBay,SayHello2");
  std::vector<std::string> resultVec;
  orientsec_grpc_split_to_vec(methods, resultVec, ",");
  int nRet = std::count(resultVec.begin(), resultVec.end(), method_name);
  if (nRet > 0) return true;
  return false;
}

// 客户端需要分组调用时，根据规则调用优先级高的分组中的provider
// 算法：对每个分组进行匹配，如果有provider，则返回对应的provider
//       如果没有则循环到下一分组。
static provider_t* orientsec_grpc_consumer_control_group_filter(
    provider_t* providers, int* num) {
  // 获取consumer group information
  std::vector<std::string> group_info;
  int grade_count = 0;      // 统计各个分组里面的provider数量
  provider_t* prov = NULL;  // 用于返回堆上的provider list

  std::vector<std::string> tmp;
  std::vector<std::string> temp;

  // 判断分组信息
  if (group_service == NULL || strlen(group_service) == 0) {
    return prov;
  }
  // 字符串分解成字符vector
  orientsec_grpc_split_to_vec(std::string(group_service), group_info, ";");
  int grade = group_info.size();

  for (int index = 0; index < grade; index++) {
    std::string str = group_info[index];
    orientsec_grpc_split_to_vec(str, tmp, ",");
    grade_count = 0;
    for (int in = 0; in < tmp.size(); in++) {
      // 取高优先级的provider
      for (int pro = 0; pro < *num; pro++) {
        if (0 == strcmp(providers[pro].group, tmp[in].c_str())) grade_count++;
      }
    }
    // tmp清零
    tmp.clear();
    // 对每个分组的count进行处理
    if (grade_count > 0) break;
  }
  // 如果找不到所有分组的provider，返回调用失败
  if (0 == grade_count) {
    *num = grade_count;
    return prov;
  }

  // 取出对应分组的provider
  prov = (provider_t*)gpr_zalloc(sizeof(provider_t) * grade_count);
  int j = 0;
  for (int index = 0; index < grade; index++) {
    std::string str = group_info[index];
    orientsec_grpc_split_to_vec(str, temp, ",");
    grade_count = 0;
    for (int in = 0; in < temp.size(); in++) {
      // 取高优先级的provider
      for (int pro = 0; pro < *num; pro++) {
        if (0 == strcmp(providers[pro].group, temp[in].c_str())) {
          grade_count++;
          strcpy(prov[j].host, providers[pro].host);
          prov[j].port = providers[pro].port;
          prov[j].weight = providers[pro].weight;
          prov[j].curr_weight = providers[pro].curr_weight;
          j++;
        }
      }
    }
    temp.clear();
    // 对每个分组的count进行处理
    if (grade_count > 0) break;
  }
  gpr_free(providers);
  *num = grade_count;
  return prov;
}

// init and set global failure retry times
static void grpc_retry_times_init(const char* service_name, const char* method_name) {
  // read from config file and calculate retry times
  failure_retry_times =
      obtain_retries_based_service_or_method(service_name, method_name);
  return;
}

// inner interface: get retry times for every call
int orientsec_get_failure_retry_times(){ return failure_retry_times; }

// init and set global grouping info
static void grpc_group_grading_init(const char* sname) {
  char* group_info = obtain_invoke_group_based_service(sname);
  if (strlen(group_service) <= 0){
      strcpy(group_service, group_info);
  }
    
  gpr_free(group_info);
  return;
}

// 判断是否配置服务分组信息
static bool is_group_grading() {
  if (strlen(group_service) == 0) {
    return false;
  }
  return true;
}

// query valid provider and write ip:port information into policy for
// transferring
// adding method input for lb based method
provider_t* consumer_query_providers_write_point_policy(
    const char* service_name, grpc_core::LoadBalancingPolicy* lb_policy,
    int* nums, char* method_name) {
  char query_str[ORIENTSEC_GRPC_PATH_MAX_LEN] = {0};
  // change function to local variable
  int cache_providers_num = orientsec_grpc_cache_provider_count_get();
  size_t i = 0;
  if (!service_name) {
    return NULL;
  }
  if (!method_name) {
    return NULL;
  }
  int provider_nums = 0;
  provider_t* providers = NULL;
  // provider_t* providers_trans = NULL;

  // 计算负载均衡算法
  std::string strLbStragry = ORIENTSEC_GRPC_DEFAULT_LB_TYPE;
  bool is_method_level = false;
  auto lbIter = g_consumer_lbstragry.find(service_name);
  if (lbIter != g_consumer_lbstragry.end()) {
    strLbStragry = lbIter->second;
  }
  auto meth_iter = g_method_lbalgorithem.find(method_name);
  if (meth_iter != g_method_lbalgorithem.end()) {
    strLbStragry = meth_iter->second;
    is_method_level = true;
  }

  // 是否开启服务分组
  bool is_group = is_group_grading();
  // providers 每次申请再释放，未和全局provider效率作比较，
  // 有待试验

  GRPC_PROVIDERS_LIST_LOCK_START
  //原始代码代码，返回所有providers，未应用负载均衡策略
  std::map<std::string, provider_t*>::iterator provider_lst_iter =
      g_cache_providers.find(service_name);
  if (provider_lst_iter != g_cache_providers.end()) {
    //首先统计合法的provider个数
    for (i = 0; i < cache_providers_num; i++) {
      provider_t* provider = &provider_lst_iter->second[i];
      if (!ORIENTSEC_GRPC_CHECK_BIT(provider->flag_in_blklist,
                                    ORIENTSEC_GRPC_PROVIDER_FLAG_IN_BLKLIST) &&
          provider->flag_invalid == 0 && provider->flag_call_failover == 0 &&
          provider->online) {
        if (!is_method_level) {
          if (orientsec_grpc_consumer_control_version_match(
                  service_name, provider->version) == true)
            provider_nums++;
        } else {
          if (method_check(provider, method_name, cache_providers_num)) {
            if (orientsec_grpc_consumer_control_version_match(
                    service_name, provider->version) == true)
              provider_nums++;
          }
        }
      }
    }
    if (provider_nums != 0) {
      providers = (provider_t*)gpr_zalloc(sizeof(provider_t) * provider_nums);
      int j = 0;
      for (int i = 0; i < cache_providers_num; i++) {
        provider_t* provider = &provider_lst_iter->second[i];
        //有效且未在黑名单内的数据。
        if (!ORIENTSEC_GRPC_CHECK_BIT(
                provider->flag_in_blklist,
                ORIENTSEC_GRPC_PROVIDER_FLAG_IN_BLKLIST) &&
            provider->flag_invalid == 0 && provider->flag_call_failover == 0 &&
            provider->online) {
          // 服务版本check
          if (!is_method_level) {
            if (orientsec_grpc_consumer_control_version_match(
                    service_name, provider->version) == true) {
              strcpy(providers[j].host, provider->host);
              strcpy(providers[j].group, provider->group);
              providers[j].port = provider->port;
              providers[j].weight = provider->weight;
              providers[j].curr_weight = provider->curr_weight;
              j++;
            }
          } else {
            if (method_check(provider, method_name, cache_providers_num)) {
              if (orientsec_grpc_consumer_control_version_match(
                      service_name, provider->version) == true) {
                strcpy(providers[j].host, provider->host);
                strcpy(providers[j].group, provider->group);
                providers[j].port = provider->port;
                providers[j].weight = provider->weight;
                providers[j].curr_weight = provider->curr_weight;
                j++;
              }
            }
          }
        }
      }
      *nums = provider_nums;
    }
  }

  // no appriate provide
  if (provider_nums == 0) {
    printf("-----------------provider_nums=%d  !!!\n", provider_nums);
    *nums = provider_nums;
    // memset(lb_policy->provider_addr,0,sizeof(lb_policy->provider_addr));
    // sprintf(lb_policy->provider_addr, "%s:%s","2.2.2.2","2222");
    GRPC_PROVIDERS_LIST_LOCK_END
    return providers;
  } else {
    // 进行服务分组的校验
    if (is_group) {
      providers = orientsec_grpc_consumer_control_group_filter(providers,
                                                               &provider_nums);
      *nums = provider_nums;
    }
  }
  // printf("REQ valid provider_num = %d\n", provider_nums);
  // add by yang
  char* hash_info = lb_policy->hash_lb;
  int index =
      get_index_from_lb_aglorithm(service_name, providers, nums, hash_info,
                                  strLbStragry.c_str());  // provider为一个列表
  sprintf(lb_policy->provider_addr, "ipv4:%s:%d", providers[index].host,
          providers[index].port);
  // addbylm
  for (int i = 0, j = 0; i < cache_providers_num; i++) {
    if (!strcmp(providers[j].host, provider_lst_iter->second[i].host) &&
        providers[j].port == provider_lst_iter->second[i].port) {
      provider_lst_iter->second[i].curr_weight = providers[j].curr_weight;
      j++;
      if (j > *nums - 1) break;
    }
  }
  GRPC_PROVIDERS_LIST_LOCK_END
  *nums = provider_nums;
  return providers;
}

// put the provider selected by hash into first location
provider_t* sort_hash_to_first(provider_t* providers, int provider_nums,
                               int index) {
  provider_t tmp_provider;
  memset(&tmp_provider, 0, sizeof(provider_t));
  memcpy(&tmp_provider, &providers[index], sizeof(provider_t));
  memcpy(&providers[index], &providers[0], sizeof(provider_t));
  memcpy(&providers[0], &tmp_provider, sizeof(provider_t));

  return providers;
}

/*
 * query all providers not in blacklist
 * only invoke during zookeeper resolving
 * input: service name, hash variable, method name
 * output: providers list,valid provider number
 */
provider_t* consumer_query_providers(const char* service_name, int* nums,
                                     char* hasharg, char* method_name) {
  char query_str[ORIENTSEC_GRPC_PATH_MAX_LEN] = {0};
  size_t i = 0;
  size_t ind = 0;
  size_t pro_size = 0;
  int standby_count = 0;
  int j = 0;
  if (!service_name) {
    return NULL;
  }
  if (!method_name) return NULL;
  bool is_req = is_request_loadbalance();
  int cache_providers_num = orientsec_grpc_cache_provider_count_get();
  int provider_nums = 0;
  provider_t* providers = NULL;
  // provider_t* providers_trans = NULL;

  // 计算负载均衡模式
  std::string strLbMode = ORIENTSEC_GRPC_LB_MODE_CONNECT;
  auto mode_iter = g_method_lbstragry.find(method_name);
  if (mode_iter != g_method_lbstragry.end()) {
    strLbMode = mode_iter->second;
  }
  if (0 == strcmp(ORIENTSEC_GRPC_LB_MODE_REQUEST, strLbMode.c_str())) {
    is_req = true;
  }

  // 计算负载均衡算法
  std::string strLbStragry = ORIENTSEC_GRPC_DEFAULT_LB_TYPE;
  bool is_method_level = false;
  auto lbIter = g_consumer_lbstragry.find(service_name);
  if (lbIter != g_consumer_lbstragry.end()) {
    strLbStragry = lbIter->second;
  }
  auto meth_iter = g_method_lbalgorithem.find(method_name);
  if (meth_iter != g_method_lbalgorithem.end()) {
    strLbStragry = meth_iter->second;
    is_method_level = true;
  }

  // 获得分组信息并判断是否开启服务分组
  grpc_group_grading_init(service_name);
  bool is_group = is_group_grading();
  // get the retry times from config file and store it
  grpc_retry_times_init(service_name, method_name);

  GRPC_PROVIDERS_LIST_LOCK_START
  //*nums = 0;
  //原始代码实现，返回所有providers，未应用负载均衡策略
  std::map<std::string, provider_t*>::iterator provider_lst_iter =
      g_cache_providers.find(service_name);
  if (provider_lst_iter != g_cache_providers.end()) {
    // 判断是否存在master server
    for (ind = 0; ind < cache_providers_num; ind++) {
      provider_t* provider = &provider_lst_iter->second[ind];
      // 备服务或者 下线的主服务
      if ((!provider->is_master) ||
          (provider->is_master && provider->flag_invalid)) {
        standby_count++;
      }
    }
    if (standby_count == cache_providers_num) {
      g_exist_master = false;
      // g_active_standby = true;
    } else {
      g_exist_master = true;
    }

    // 标记provider 是否提供服务，根据active/standby 状态
    for (ind = 0; ind < cache_providers_num; ind++) {
      provider_t* provider = &provider_lst_iter->second[ind];
      // 如果存在active provider，标记standby provider不可用
      if (g_exist_master) {
        if (!provider->is_master) {
          provider->online = 0;
        } else {
          provider->online = 1;  // for zookeeper dynamic switch
        }

      } else {
        provider->online = 1;
      }
    }

    //首先统计合法的provider个数
    for (i = 0; i < cache_providers_num; i++) {
      provider_t* provider = &provider_lst_iter->second[i];
      if (!ORIENTSEC_GRPC_CHECK_BIT(provider->flag_in_blklist,
                                    ORIENTSEC_GRPC_PROVIDER_FLAG_IN_BLKLIST) &&
          provider->flag_invalid == 0 && provider->flag_call_failover == 0 &&
          provider->online) {
        if (!is_method_level) {
          // 请求负载均衡情况下，加上版本校验，切换版本时无 subchannel可连接，
          // 这里获得的provider ip:port 和 num
          // 将用于创建连接各个服务的subchannel service version check
          if (!is_req) {  // connection mode
            if (orientsec_grpc_consumer_control_version_match(
                    service_name, provider->version) == true)
              provider_nums++;
          } else {
            provider_nums++;
          }
        } else {  // 方法级别的判断
          if (method_check(provider, method_name, cache_providers_num)) {
            if (!is_req) {  // connection mode
              if (orientsec_grpc_consumer_control_version_match(
                      service_name, provider->version) == true) {
                provider_nums++;
              }
            } else {
              provider_nums++;
            }
          }
        }
      }
    }
    if (provider_nums != 0) {
      // memory alignment
      pro_size = sizeof(provider_t) + (64 - sizeof(provider_t) % 64);
      providers = (provider_t*)gpr_zalloc(pro_size * provider_nums);

      for (int k = 0; k < cache_providers_num; k++) {
        provider_t* provider = &provider_lst_iter->second[k];

        //有效且未在黑名单内的数据。
        if (!ORIENTSEC_GRPC_CHECK_BIT(
                provider->flag_in_blklist,
                ORIENTSEC_GRPC_PROVIDER_FLAG_IN_BLKLIST) &&
            provider->flag_invalid == 0 && provider->flag_call_failover == 0 &&
            provider->online) {
          if (!is_method_level) {
            if (!is_req) {  // connection mode
                            // service version check
              if (orientsec_grpc_consumer_control_version_match(
                      service_name, provider->version) == true) {
                strcpy(providers[j].host, provider->host);
                strcpy(providers[j].group, provider->group);
                providers[j].port = provider->port;
                providers[j].weight = provider->weight;
                providers[j].curr_weight = provider->curr_weight;
                j++;
              }
            } else {
              strcpy(providers[j].host, provider->host);
              strcpy(providers[j].group, provider->group);
              providers[j].port = provider->port;
              providers[j].weight = provider->weight;
              providers[j].curr_weight = provider->curr_weight;
              j++;
            }       // end of is_req
          } else {  //方法级别的判断
            if (method_check(provider, method_name, cache_providers_num)) {
              if (!is_req) {  // connection mode
                              // service version check
                if (orientsec_grpc_consumer_control_version_match(
                        service_name, provider->version) == true) {
                  strcpy(providers[j].host, provider->host);
                  strcpy(providers[j].group, provider->group);
                  providers[j].port = provider->port;
                  providers[j].weight = provider->weight;
                  providers[j].curr_weight = provider->curr_weight;
                  j++;
                }
              } else {
                strcpy(providers[j].host, provider->host);
                strcpy(providers[j].group, provider->group);
                providers[j].port = provider->port;
                providers[j].weight = provider->weight;
                providers[j].curr_weight = provider->curr_weight;
                j++;
              }  // end of is_req
            }
          }
        }
      }  // end of for
      *nums = provider_nums;
    }
  }

  if (provider_nums == 0) {
    printf("--------------query provider_nums=%d  !!!\n", provider_nums);
    *nums = provider_nums;
    GRPC_PROVIDERS_LIST_LOCK_END
    return providers;
  } else {
    // 进行服务分组的校验
    if (is_group) {
      providers = orientsec_grpc_consumer_control_group_filter(providers,
                                                               &provider_nums);
      // 重新赋值nums
      *nums = provider_nums;
    }
  }

  // 服务分组后再检查provider number
  if (provider_nums == 0) {
    printf("after grouping query provider_nums=%d  !!!\n", provider_nums);
    GRPC_PROVIDERS_LIST_LOCK_END
    return providers;
  }
  printf("Resolved valid provider_num = %d\n", provider_nums);

  if (is_req) {  // request 情况下
    // 如果0或1个provider, 不考虑算法
    if (provider_nums != 0 && provider_nums != 1) {
      //判断负载均衡算法
      std::map<std::string, std::string>::iterator lbIter =
          g_consumer_lbstragry.find(service_name);

      int prov_index = 0;
      if (lbIter != g_consumer_lbstragry.end()) {
        strLbStragry = lbIter->second;
      }
      //第一次如果是hash，就走一次算法，将hash算法选出来的provider放在第一个
      if (0 == strcmp(strLbStragry.c_str(), ORIENTSEC_GRPC_LB_TYPE_CH)) {
        //进入相应的负载均衡算法中进行计算
        // std::string hash_arg;
        prov_index =
            chLB->choose_subchannel(service_name, providers, nums, hasharg);
        // 假设provider index 已经选出
        if (prov_index != 0)
          providers = sort_hash_to_first(providers, provider_nums, prov_index);
      }
    }
    GRPC_PROVIDERS_LIST_LOCK_END
    *nums = provider_nums;
    return providers;
  } else {  // for connection mode
    // 过滤掉服务版本校验失败的provider

    int index = get_index_from_lb_aglorithm(
        service_name, providers, nums, hasharg,
        strLbStragry.c_str());  // provider为一个列表
    // addbylm
    for (i = 0, j = 0; i < cache_providers_num; i++) {
      if (!strcmp(providers[j].host, provider_lst_iter->second[i].host) &&
          providers[j].port == provider_lst_iter->second[i].port) {
        provider_lst_iter->second[i].curr_weight = providers[j].curr_weight;
        j++;
        if (j > *nums - 1) break;
      }
    }
    if (*nums == 1) {
      GRPC_PROVIDERS_LIST_LOCK_END
      return providers;
    } else {
      provider_t* myproviders = (provider_t*)gpr_zalloc(sizeof(provider_t));
      memcpy(myproviders, &providers[index], sizeof(provider_t));
      // myproviders->version = NULL;

      for (i = 0; i < *nums; i++) {
        free_provider_v2(providers + i);
      }
      free(providers);
      *nums = 1;
      GRPC_PROVIDERS_LIST_LOCK_END
      return myproviders;
    }
  }
}

provider_t* consumer_query_providers_all(const char* service_name, int* nums) {
  return NULL;
}

static void init_provider_by_host_port(provider_t* provider, const char* host, const char* port) {
  if (!provider) return;
  size_t len;
  if (host) {
    len = strlen(host);
    snprintf(provider->host, len < HOST_MAX_LEN ? (len + 1) : HOST_MAX_LEN,
             "%s",host);
  }
  provider->weight = 0;
  provider->port = atoi(port);
  provider->flag_invalid = 0;  //有效标记
  provider->flag_invalid_timestamp = 0;
  provider->flag_call_failover = 0;  //未发生调用出错。
  provider->flag_in_blklist = 0;     //不在黑名单
  provider->flag_subchannel_close = 0;
  provider->time_subchannel_close = 0;
  provider->is_master = true;
  provider->online = 1;
}

// initialize provider list by appointed list configured in config file
static void init_provider_from_appointed_list(const char* service_name,
                                       const char* appointed_list) {
  if (appointed_list == NULL && strlen(appointed_list) == 0) return;
  std::vector<std::string> appoint_list;
  char* host = NULL;
  char* port = NULL;
  orientsec_grpc_split_to_vec(appointed_list, appoint_list, ",");
  auto it = appoint_list.begin();
  size_t length = appoint_list.size();
  int ind;
  int cache_num = orientsec_grpc_cache_provider_count_get();
  provider_t* providers =
      (provider_t*)gpr_zalloc(sizeof(provider_t) * cache_num);
  for (ind = 0; ind < cache_num && ind < length; ind++) {
    gpr_split_host_port((*it).c_str(), &host, &port);
    init_provider_by_host_port(&providers[ind], host, port);
    g_cache_providers.insert(std::pair<std::string, provider_t*>(
        std::string(service_name), providers));
    gpr_free(host);
    gpr_free(port);
  }
  return;
}

static void common_consumer_register_operation(url_t* url) {
  url_update_parameter(url, (char*)ORIENTSEC_GRPC_CATEGORY_KEY,
                       (char*)ORIENTSEC_GRPC_CONSUMERS_CATEGORY);
  registry(url);
}

static void common_consumer_subscribe_operation(url_t* url) {
  //消费者订阅providers目录
  url_update_parameter(url, (char*)ORIENTSEC_GRPC_CATEGORY_KEY,
                       (char*)ORIENTSEC_GRPC_PROVIDERS_CATEGORY);
  subscribe(url, consumer_providers_callback);

  //消费者订阅routers目录
  url_update_parameter(url, (char*)ORIENTSEC_GRPC_CATEGORY_KEY,
                       (char*)ORIENTSEC_GRPC_ROUTERS_CATEGORY);
  subscribe(url, consumer_routers_callback);

  //消费者订阅configurators目录
  url_update_parameter(url, (char*)ORIENTSEC_GRPC_CATEGORY_KEY,
                       (char*)ORIENTSEC_GRPC_CONFIGURATORS_CATEGORY);
  subscribe(url, consumer_configurators_callback);
}
static void common_operation(url_t* url) {
  // common operation
  common_consumer_register_operation(url);
  common_consumer_subscribe_operation(url);
}

// new thread for consumer registry with zookeeper offline
static void grpc_consumer_zk_registry_bg(url_t* url_clone) {
  int last_time = 0;
  int now = 0;
  int times = 0;
  while (times < TIMES_MAX_TRY) {
    now = clock() / CLOCKS_PER_SEC;
    // try every 3 seconds
    if (now - last_time > 3) {
      // initial zookeeper registry factory
      orientsec_grpc_registry_zk_intf_init();

      common_operation(url_clone);

      if (!zk_get_create_node_flag()) {  // not register successfully
        gpr_log(GPR_ERROR, "Registry in next 3 seconds...\n");
        // increase times
        last_time = now;
        times++;
        continue;
      }
      // registre successfully, free memory, return and end of thread
      url_full_free(&url_clone);
      gpr_log(GPR_ERROR, "Register successfully......thread exit...\n");
      return;
    }
  }
  // thread returns safely
  if (TIMES_MAX_TRY == times)
    gpr_log(GPR_ERROR,
            "Consumer could not registered in corressponding zookeeper.\n");
  url_full_free(&url_clone);
  gpr_log(GPR_ERROR, "Register failed.time was over......Thread exit...\n");
  return;
}

//  new thread for consumer registry with zookeeper online
static void grpc_lookup_registry_center(url_t* url) {
  url_t* u = NULL;
  int num = 0;  //用来返回公有注册中心的提供服务的provider number
  int pn = 0;  //用来返回私有注册中心的提供服务的provider number
  url_update_parameter(url, (char*)ORIENTSEC_GRPC_CATEGORY_KEY,
                       (char*)ORIENTSEC_GRPC_PROVIDERS_CATEGORY);

  int last_time = 0;
  int now = 0;
  int times = 0;
  while (times < TIMES_PER_DAY) {
    now = clock() / CLOCKS_PER_SEC;
    if (now - last_time > 10) {
      u = lookup(url, &num, &pn);
      if (num > 0) {
        // 修改zk_reg的属性为公有注册
        set_cons_reg_scheme(PUBLIC_REG);
        common_operation(url);
        break;

      } else if (pn > 0) {
        // 修改zk_reg属性为私有属性
        set_cons_reg_scheme(PRIVATE_REG);
        common_operation(url);
        break;
      }
      last_time = now;
      times++;
    }
  }
  // 线程安全返回
  if (TIMES_PER_DAY == times)
    gpr_log(GPR_ERROR,
            "Consumer could not find corressponding registry center");
  url_full_free(&url);
  return;
}

// consumer注册。
// 1、解析fullmethod 获取conf文件配置信息、拼接consumer
// url串，调用接口写入向zk写入url串、
// 记录已经注册的url串，以便consumer停止时调用zk接口注销url串。
// 2、注册监听器
//  a.消费者订阅providers目录
//  b.消费者订阅routers目录
//  c.消费者订阅configurators目录
char* orientsec_grpc_consumer_register(const char* fullmethod,
                                       const char* frameversion) {
  init_providers_list();
  //未加锁
  char* consumerurl = orientsec_grpc_consumer_url(fullmethod, frameversion);
  url_t* url = url_parse(consumerurl);

  //缓存该客户端信息
  GRPC_CONSUMERS_LIST_LOCK_START
  std::map<std::string, std::vector<url_t*> >::iterator consumerIter =
      g_consumers.find(fullmethod);
  url_t* consumerUrl = (url_t*)gpr_zalloc(sizeof(url_t));
  url_init(consumerUrl);
  url_clone(url, consumerUrl);
  if (consumerIter == g_consumers.end()) {
    std::vector<url_t*> consumers;
    consumers.push_back(consumerUrl);
    g_consumers.insert(
        std::pair<std::string, std::vector<url_t*> >(fullmethod, consumers));
  } else {
    consumerIter->second.push_back(consumerUrl);
  }

  //更新loadbalance配置
  std::map<std::string, std::string>::iterator lbIter =
      g_consumer_lbstragry.find(fullmethod);
  if (lbIter == g_consumer_lbstragry.end()) {
    g_consumer_lbstragry.insert(std::pair<std::string, std::string>(
        fullmethod, ORIENTSEC_GRPC_LB_TYPE_RR));
    lbIter = g_consumer_lbstragry.find(fullmethod);
  }
  char* consumerLb = url_get_parameter_v2(
      url, ORIENTSEC_GRPC_REGISTRY_KEY_DEFAULT_LOADBALANCE, NULL);
  if (consumerLb) {
    lbIter->second = std::string(consumerLb);
  }

  // 初始化基于方法的loadbalance配置
  if (g_method_lbstragry.empty())
    g_method_lbstragry.insert(std::pair<std::string, std::string>(
        ORIENTSEC_GRPC_DEFAULT_METHOD_KEY, ORIENTSEC_GRPC_LB_MODE_CONNECT));
  if (g_method_lbalgorithem.empty()) {
    g_method_lbalgorithem.insert(std::pair<std::string, std::string>(
        ORIENTSEC_GRPC_DEFAULT_METHOD_KEY, ORIENTSEC_GRPC_LB_TYPE_RR));
    // 测试用
    // g_method_lbalgorithem.insert(std::pair<std::string,
    // std::string>("CreateBook", ORIENTSEC_GRPC_LB_TYPE_PF));
  }
  GRPC_CONSUMERS_LIST_LOCK_END

  // check whether exist appointed provider ip and port
  // if configured, return without registered and subscribed
  char* appo_prov_info = obtain_appointed_provider_list(fullmethod);
  if (strlen(appo_prov_info) > 0) {
    // update global provider list
    init_provider_from_appointed_list(fullmethod,appo_prov_info);
    // release memory and return directly
    gpr_free(appo_prov_info);
    url_full_free(&url);
    return consumerurl;
  } else  {
    gpr_free(appo_prov_info);
  }


  // differentiated inside and outside service
  zookeeper_reg zk_center = PUBLIC_REG;
  // 先进行zk连接并query provider，条件是公有和私有中心都配置的情况下
  orientsec_grpc_registry_zk_intf_init();
  char* pub = get_pub_reg_center();
  char* pri = get_pri_reg_center();
  if ((strlen(pri) > 0) || strlen(pub) == 0) zk_center = PRIVATE_REG;
  if ((strlen(pri) > 0) && strlen(pub) > 0) zk_center = HYBRID_REG;
  gpr_free(pub);
  gpr_free(pri);
  // registry when public/private registry mode
  if (zk_center == PUBLIC_REG || zk_center == PRIVATE_REG) {
    // common operation
    common_operation(url);

    // register okay or not，if not， create new thread do work background
    if (!zk_get_create_node_flag()) {
      // url clone
      url_t* cons_url = (url_t*)gpr_zalloc(sizeof(url_t));
      url_init(cons_url);
      url_clone(url, cons_url);
      std::thread zk_registry_agent_thread(grpc_consumer_zk_registry_bg,
                                           cons_url);
      gpr_log(GPR_ERROR,
              "zookeeper register failed, New thread[%ld] for consumer was "
              "created\n",
              std::this_thread::get_id());
      // zk_registry_agent_thread.detach();
      zk_registry_agent_thread.detach();
    }
    url_full_free(&url);

  } else {  // for hybrid register center
    // 1. first check the corresponding provider
    // 调用一下zk lookup
    url_t* u = NULL;
    int num = 0;  //用来返回公有注册中心的提供服务的provider number
    int pn = 0;  //用来返回私有注册中心的提供服务的provider number
    url_update_parameter(url, (char*)ORIENTSEC_GRPC_CATEGORY_KEY,
                         (char*)ORIENTSEC_GRPC_PROVIDERS_CATEGORY);
    u = lookup(url, &num, &pn);
    if (num > 0) {
      // 修改zk_reg的属性为公有注册
      set_cons_reg_scheme(PUBLIC_REG);
      common_operation(url);

    } else if (pn > 0) {
      //  修改zk_reg属性为私有属性
      set_cons_reg_scheme(PRIVATE_REG);
      common_operation(url);

    } else {
      // 公有或者私有注册中心均无相关provider可调用
      //  启动一个新线程，监控
      // allocting one url on heap to transfer， since detach
      url_t* reg_url = (url_t*)gpr_zalloc(sizeof(url_t));
      url_init(reg_url);
      url_clone(url, reg_url);
      std::thread th_agent(grpc_lookup_registry_center, reg_url);
      th_agent.detach();
    }
    url_full_free(&url);
  }
  return consumerurl;
}

// consumer取消注册\取消监听器。
// 1、注销zk注册信息。
// 2、取消订阅监听器信息。
//  a.消费者取消订阅providers目录
//  b.消费者取消订阅routers目录
//  c.消费者取消订阅configurators目录
int orientsec_grpc_consumer_unregister(char* reginfo) {
  //取消zk注册
  GRPC_CONSUMERS_LIST_LOCK_START

  url_t* url = url_parse(reginfo);
  unregistry(url);

  //释放provider监听
  url_update_parameter(url, (char*)ORIENTSEC_GRPC_CATEGORY_KEY,
                       (char*)ORIENTSEC_GRPC_PROVIDERS_CATEGORY);
  unsubscribe(url, consumer_providers_callback);

  //释放router监听
  url_update_parameter(url, (char*)ORIENTSEC_GRPC_CATEGORY_KEY,
                       (char*)ORIENTSEC_GRPC_ROUTERS_CATEGORY);
  unsubscribe(url, consumer_routers_callback);

  //释放configurators监听
  url_update_parameter(url, (char*)ORIENTSEC_GRPC_CATEGORY_KEY,
                       (char*)ORIENTSEC_GRPC_CONFIGURATORS_CATEGORY);
  unsubscribe(url, consumer_configurators_callback);

  char* intf =
      url_get_parameter_v2(url, ORIENTSEC_GRPC_REGISTRY_KEY_INTERFACE, NULL);

  std::map<std::string, std::vector<url_t*> >::iterator consumerIter =
      g_consumers.find(intf);
  if (consumerIter != g_consumers.end()) {
    char buf[ORIENTSEC_GRPC_URL_MAX_LEN] = {0};
    std::vector<url_t*>& consumers = consumerIter->second;
    for (std::vector<url_t*>::iterator consumerIter = consumers.begin();
         consumerIter != consumers.end(); consumerIter++) {
      memset(buf, 0, ORIENTSEC_GRPC_URL_MAX_LEN);
      url_to_string_buf(*consumerIter, buf, ORIENTSEC_GRPC_URL_MAX_LEN);
      if (strcmp(buf, reginfo) == 0) {
        url_free(*consumerIter);
        consumers.erase(consumerIter);
        break;
      }
    }
  }
  url_full_free(&url);
  GRPC_CONSUMERS_LIST_LOCK_END  // add by hyn
      return 0;
}

void record_provider_failure(char* clientId, char* providerId, const char* methods) {
  g_failover_utils.record_provider_failure(clientId, providerId,
                                           methods);  //使用实例对象调用静态方法
}

void reset_provider_failure(char* clientId, char* providerId, const char* methods) {
  g_failover_utils.reset_provider_failure(clientId, providerId,methods);  //使用实例对象调用静态方法
}

int get_max_backoff_time() {
  int value = 120;
  if (g_failover_utils.get_max_backoff_time() > 0)
    return g_failover_utils.get_max_backoff_time();
  else
    return value;
}

// provider active/standby check
int provider_num_active_check(char* service_name) {
  int count = 0;
  std::map<std::string, provider_t*>::iterator provider_lst_iter =
      g_cache_providers.find(service_name);
  if (provider_lst_iter != g_cache_providers.end()) {
    //首先统计合法的provider个数
    for (int i = 0; i < orientsec_grpc_cache_provider_count_get(); i++) {
      provider_t* provider = &provider_lst_iter->second[i];
      if (!ORIENTSEC_GRPC_CHECK_BIT(provider->flag_in_blklist,
                                    ORIENTSEC_GRPC_PROVIDER_FLAG_IN_BLKLIST) &&
          provider->flag_invalid == 0 && provider->flag_call_failover == 0) {
        // active/standby check
        if (provider->is_master) count++;
      }
    }
  }
  return count;
}

// provider online property reset
// call when no active provider
void provider_active_standby_setting(char* service_name, bool have_active) {
  std::map<std::string, provider_t*>::iterator provider_lst_iter =
      g_cache_providers.find(service_name);
  if (provider_lst_iter != g_cache_providers.end()) {
    // traverse global provider list
    for (int i = 0; i < orientsec_grpc_cache_provider_count_get(); i++) {
      provider_t* provider = &provider_lst_iter->second[i];
      if (provider->is_master) {
        provider->online = (have_active ? 1 : 0);
      } else {
        // 无主服务器时，备服务器上线
        provider->online = (have_active ? 0 : 1);
      }
    }
  }
  return;
}

// service version check
int provider_num_after_service_check(char* service_name) {
  int count = 0;
  std::map<std::string, provider_t*>::iterator provider_lst_iter =
      g_cache_providers.find(service_name);
  if (provider_lst_iter != g_cache_providers.end()) {
    //首先统计合法的provider个数
    for (int i = 0; i < orientsec_grpc_cache_provider_count_get(); i++) {
      provider_t* provider = &provider_lst_iter->second[i];
      if (!ORIENTSEC_GRPC_CHECK_BIT(provider->flag_in_blklist,
                                    ORIENTSEC_GRPC_PROVIDER_FLAG_IN_BLKLIST) &&
          provider->flag_invalid == 0 && provider->flag_call_failover == 0) {
        // service version check
        if (orientsec_grpc_consumer_control_version_match(
                service_name, provider->version) == true)
          count++;
      }
    }
  }
  return count;
}

//获取经过黑白名单处理之后用于负载均衡的provider数量
int get_consumer_lb_providers_acount(char* service_name) {
  int count = 0;
  std::map<std::string, provider_t*>::iterator providerMapIter =
      g_cache_providers.find(service_name);
  if (providerMapIter != g_cache_providers.end()) {
    for (size_t i = 0; i < orientsec_grpc_cache_provider_count_get(); i++) {
      if (!is_provider_invalid(&providerMapIter->second[i])) {
        count++;
      }
    }
  }
  return count;
}

//获取可以调用的provider数量，用于容错切换
int get_valid_providers_acount(char* service_name, const char* method_name) {
  int provider_nums = 0;
  int cache_providers_num = orientsec_grpc_cache_provider_count_get();
  bool is_method_level = false;
  // 计算负载均衡算法
  std::string strLbStragry = ORIENTSEC_GRPC_DEFAULT_LB_TYPE;
  auto meth_iter = g_method_lbalgorithem.find(method_name);
  if (meth_iter != g_method_lbalgorithem.end()) {
    strLbStragry = meth_iter->second;
    is_method_level = true;
  }

  std::map<std::string, provider_t*>::iterator providerMapIter =
      g_cache_providers.find(service_name);
  if (providerMapIter != g_cache_providers.end()) {
    //首先统计合法的provider个数
    for (int i = 0; i < cache_providers_num; i++) {
      provider_t* provider = &providerMapIter->second[i];
      if (!ORIENTSEC_GRPC_CHECK_BIT(provider->flag_in_blklist,
                                    ORIENTSEC_GRPC_PROVIDER_FLAG_IN_BLKLIST) &&
          provider->flag_invalid == 0 && provider->flag_call_failover == 0 &&
          provider->online) {
        if (!is_method_level) {
          if (orientsec_grpc_consumer_control_version_match(
                  service_name, provider->version) == true)
            provider_nums++;
        } else {  // 方法级别的判断
          if (method_check(provider, method_name, cache_providers_num)) {
            if (orientsec_grpc_consumer_control_version_match(
                    service_name, provider->version) == true) {
              provider_nums++;
            }
          }
        }
      }
    }
  }
  return provider_nums;
}

//获取zk上某服务的provider数量
int get_consumer_providers_acount(char* service_name) {
  int count = 0;
  std::map<std::string, provider_t*>::iterator providerMapIter =
      g_cache_providers.find(service_name);
  if (providerMapIter == g_cache_providers.end()) {
    return 0;
  }
  for (int i = 0; i < orientsec_grpc_cache_provider_count_get(); i++) {
    if (providerMapIter->second[i].flag_invalid == 0) {
      count++;
    }
  }
  return count;
}

//设置某个provider 调用失败标记,isSet == 1表示设置标记位，否则为清除标记位
void set_or_clr_provider_failover_flag_inner(char* service_name,
                                             char* providerId, int isSet) {
  if (!service_name || !providerId || strlen(providerId) == 0) return;

  bool is_req = is_request_loadbalance();
  std::string provider = providerId;
  size_t pos = provider.find_first_of(':');
  if (pos == std::string::npos) {
    gpr_log(GPR_ERROR,
            "invalid providerId,[%s],should split by : ", providerId);
    return;
  }
  std::string provider_host = provider.substr(0, pos);
  int provider_port = 0;
  provider_port = atoi(provider.substr(pos + 1).c_str());

  GRPC_PROVIDERS_LIST_LOCK_START
  std::map<std::string, provider_t*>::iterator providerMapIter =
      g_cache_providers.find(service_name);
  if (providerMapIter != g_cache_providers.end()) {
    for (size_t i = 0; i < orientsec_grpc_cache_provider_count_get(); i++) {
      if (0 == strcmp(providerMapIter->second[i].host, provider_host.c_str()) &&
          providerMapIter->second[i].port == provider_port &&
          providerMapIter->second[i].flag_invalid == 0) {
        if (isSet) {
          ORIENTSEC_GRPC_SET_BIT(providerMapIter->second[i].flag_call_failover,
                                 1);
        } else {
          ORIENTSEC_GRPC_SET_BIT(providerMapIter->second[i].flag_call_failover,
                                 0);
        }
        break;
      }
    }
  }
  if (!is_req)  //连接方式需要重新解析
    g_need_resolve = true;
  GRPC_PROVIDERS_LIST_LOCK_END
}

void set_provider_failover_flag(char* service_name, char* providerId) {
  set_or_clr_provider_failover_flag_inner(service_name, providerId, 1);
}

//清除某个provider 调用失败标记
void clr_provider_failover_flag(char* service_name, char* providerId) {
  set_or_clr_provider_failover_flag_inner(service_name, providerId, 0);
}

void set_consmuer_flow_control_threshold(char* service_name, long max_request) {
  if (!service_name || max_request <= 0) {
    return;
  }
  g_request_controller_utils.SetMaxRequestMap(service_name, max_request);
}

bool check_consumer_flow_control(char* service_name) {
  if (!service_name) {
    return true;
  }
  return g_request_controller_utils.CheckRequest(service_name);
}

void decrease_consumer_request_count(char* service_name) {
  if (!service_name) {
    return;
  }
  g_request_controller_utils.DecreaseRequest(service_name);
}

// check the time point when connection mode
// if reach the set limit time，to set resovling flag
void check_elapse_time_reach_setting() {
  std::chrono::system_clock::time_point start =
      std::chrono::system_clock::now();
  std::chrono::seconds elapse =
      std::chrono::time_point_cast<std::chrono::seconds>(start) -
      std::chrono::time_point_cast<std::chrono::seconds>(last_call_point);
  int limit = g_conn_switch_time * 60;
  if (limit > 0) {
    if (elapse.count() >= limit) {
      g_switch_provider_when_connection = true;
      last_call_point = start;
      std::cout << "connection time reached switchover time ["
                << g_conn_switch_time << "] minutes..." << std::endl;
    }
  }
  
}

// read the global switchover flag when connection mode
bool orientsec_switch_when_connection_resolved() {
  return g_switch_provider_when_connection;
}

// reset the global switchover flag
void orientsec_switch_when_connection_resolved_reset() {
  g_switch_provider_when_connection = false;
}
