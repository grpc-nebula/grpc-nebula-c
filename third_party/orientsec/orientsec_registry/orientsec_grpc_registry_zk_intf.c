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

#include <grpc/support/alloc.h>
#include <grpc/support/log.h>
#include "orientsec_grpc_properties_tools.h"
#include "orientsec_grpc_registy_intf.h"
#include "registry_contants.h"
#include "registry_factory.h"
#include "registry_service.h"

// initialize once
static bool binit = false;
// static char zk_address[ORIENTSEC_GRPC_PROPERTY_VALUE_MAX_LEN] = {0};
// static registry_service_t* g_zk_registry_service = NULL;
// add public and private registry center
char zk_public_address[ORIENTSEC_GRPC_PROPERTY_VALUE_MAX_LEN] = {0};
char zk_private_address[ORIENTSEC_GRPC_PROPERTY_VALUE_MAX_LEN] = {0};
static registry_service_t* g_zk_pub_registry_service = NULL;
static registry_service_t* g_zk_pri_registry_service = NULL;
// static registry_service_t** g_zk_registry_service = NULL;
//
static zookeeper_reg zk_prov_reg = PUBLIC_REG;
static zookeeper_reg zk_cons_reg = PUBLIC_REG;
// static zookeeper_reg zk_reg = PUBLIC_REG;

// 字符串切割函数
// 返回一个 char *arr[], size为返回数组的长度
char** spilt_char(const char* str, const char sep, int* size) {
  if ((str == NULL) || (strlen(str) == 0)) {
    size = 0;
    return NULL;
  }
  int count = 0, i;
  for (i = 0; i < strlen(str); i++) {
    if (str[i] == sep) {
      count++;
    }
  }
  if (0 == count) {
    if (strlen(str) > 0) {
      char** ret = calloc(++count, sizeof(char*));
      ret[0] = calloc(strlen(str)+1, sizeof(char));
      memcpy(ret[0], str, strlen(str));
      *size = 1;

      return ret;

    } else {
      size = 0;
      return NULL;
    }

  }
  char** ret = calloc(++count, sizeof(char*));

  int lastindex = -1;
  int j = 0;

  for (i = 0; i < strlen(str); i++) {
    if (str[i] == sep) {
      // 分配子串长度+1的内存空间
      ret[j] = calloc(i - lastindex, sizeof(char));
      memcpy(ret[j], str + lastindex + 1, i - lastindex - 1);
      j++;
      lastindex = i;
    }
  }
  // 处理最后一个子串
  if (lastindex <= strlen(str) - 1) {
    ret[j] = calloc(strlen(str) - lastindex, sizeof(char));
    memcpy(ret[j], str + lastindex + 1, strlen(str) - 1 - lastindex);
    j++;
  }

  *size = j;

  return ret;
}

char* get_pub_reg_center() {
  char* pub = (char*)gpr_zalloc(ORIENTSEC_GRPC_PROPERTY_VALUE_MAX_LEN);
  memcpy(pub, zk_public_address, strlen(zk_public_address) + 1);
  // return zk_public_address;
  return pub;
}
char* get_pri_reg_center() {
  char* pri = (char*)gpr_zalloc(ORIENTSEC_GRPC_PROPERTY_VALUE_MAX_LEN);
  memcpy(pri, zk_private_address, strlen(zk_private_address) + 1);
  return pri;
}

zookeeper_reg get_cons_reg_scheme() { return zk_cons_reg; }
void set_cons_reg_scheme(zookeeper_reg sch) { zk_cons_reg = sch; }
zookeeper_reg get_prov_reg_scheme() { return zk_prov_reg; }
void set_prov_reg_scheme(zookeeper_reg sch) { zk_prov_reg = sch; }

// 根据url中的服务名和服务public/private 属性Provider选择zookeeper注册中心
void zk_prov_reg_init(char* service) {
  // 读取配置文件中配置服务公开属性
  char* pub_list = NULL;
  char* pri_list = NULL;
  int pub_size = 0;
  int pri_size = 0;
  bool pub_flag = false;
  bool pri_flag = false;
  int index;
  pub_list = gpr_zalloc(ORIENTSEC_GRPC_PROPERTY_VALUE_MAX_LEN);
  pri_list = gpr_zalloc(ORIENTSEC_GRPC_PROPERTY_VALUE_MAX_LEN);
  if (orientsec_grpc_properties_get_value(ORIENTSEC_GRPC_ZK_PUBLIC_SERVICES,
                                          NULL, pub_list))
    gpr_log(GPR_INFO, "public.service.list not configured");
  if (orientsec_grpc_properties_get_value(ORIENTSEC_GRPC_ZK_PRIVATE_SERVICES,
                                          NULL, pri_list))
    gpr_log(GPR_INFO, "private.service.list not configured");
  char** pub_arr = spilt_char(pub_list, ',', &pub_size);
  char** pri_arr = spilt_char(pri_list, ',', &pri_size);

  // 遍历公共服务列表
  if (pub_size) {
    for (index = 0; index < pub_size; index++) {
      if (0 == strcmp(service, pub_arr[index])) {
        // service name existed in public service list
        pub_flag = true;
        break;
      }
    }
  }
  // 遍历私有服务列表

  if (pri_size) {
    for (index = 0; index < pri_size; index++) {
      if (0 == strcmp(service, pri_arr[index])) {
        // service name existed in public service list
        pri_flag = true;
        break;
      }
    }
  }
  // 如果public list和private list都不配置，默认公有注册
  if ((!pub_size) && (!pri_size)) {
    if (strlen(zk_public_address) > 0) {
      pub_flag = true;
      pri_flag = false;
    } else if (strlen(zk_private_address) > 0) {
      pub_flag = false;
      pri_flag = true;
    } else {
      gpr_log(GPR_ERROR, "public and private zk center are not configured");
    }
  }
  // 只配置了公共服务列表
  if (pub_size && (!pri_size)) {
    if (!pub_flag) pri_flag = true;

  }
  // 只配置了私有服务列表
  if ((!pub_size) && pri_size) {
    if (!pri_flag) pub_flag = true;
  }
  //如果public list和private list都配置,但是当前服务不在列表中,视为公共服务
  if (pub_size && pri_size) {
    if ((!pub_flag) && (!pri_flag)) pub_flag = true;
  }


  if (pub_flag && pri_flag) {
    //zk_prov_reg = HYBRID_REG;
    gpr_log(GPR_ERROR, "one service can not exist in public and private service list");
    abort();
  } else if (pri_flag) {
    zk_prov_reg = PRIVATE_REG;
  } else
    zk_prov_reg = PUBLIC_REG;
  free(pub_list);
  free(pri_list);
  free(pub_arr);
  free(pri_arr);
  return;
}

// 根据配置的公有和私有注册中心属性，Consumer选择zookeeper注册中心
static void zk_cons_reg_init() {
  // 根据配置的公有和私有注册中心选项进行判断
  bool pub_flag = false;
  bool pri_flag = false;

  if (strlen(zk_public_address) > 0) pub_flag = true;
  if (strlen(zk_private_address) > 0) pri_flag = true;

  if (pub_flag && pri_flag) {
    zk_cons_reg = HYBRID_REG;
  } else if (pri_flag) {
    zk_cons_reg = PRIVATE_REG;
  } else
    zk_cons_reg = PUBLIC_REG;

  return;
}

bool orientsec_grpc_registry_zk_intf_init() {
  char* pPub = NULL;
  char* pPri = NULL;
  int data_len = 0;
  int ret_pub = 0;
  int ret_pri = 0;
  size_t size = 0;

  registry_factory_t* factory = NULL;
  if (!binit) {
    if (orientsec_grpc_properties_init() < 0) {
      gpr_log(GPR_ERROR, "load properties failed,exit");
      exit(1);
    }
    // data_len = strlen(ORIENTSEC_GRPC_REGISTRY_DEFAULT_PROTO_HEADER) +
    // ORIENTSEC_GRPC_PROPERTY_VALUE_MAX_LEN;
    pPub = gpr_zalloc(ORIENTSEC_GRPC_PROPERTY_VALUE_MAX_LEN);
    pPri = gpr_zalloc(ORIENTSEC_GRPC_PROPERTY_VALUE_MAX_LEN);

    // get public and privater registry center data
    ret_pub = orientsec_grpc_properties_get_value(ORIENTSEC_GRPC_ZK_HOSTS, NULL,
                                                  pPub);
    ret_pri = orientsec_grpc_properties_get_value(
        ORIENTSEC_GRPC_ZK_PRIVATE_HOSTS, NULL, pPri);

    if (ret_pub && ret_pri) {
      gpr_log(GPR_ERROR, "miss required configuration %s and %s in properties %s,exit",
              ORIENTSEC_GRPC_ZK_HOSTS, ORIENTSEC_GRPC_ZK_PRIVATE_HOSTS,
              ORIENTSEC_GRPC_PROPERTIES_FILENAME);
      exit(1);
    }
    // 判断服务是公有还是私有，注册到公有注册中心or私有注册中心
    // 做一次zk相关的变量的初始化
    if (!ret_pub) {
      snprintf(zk_public_address,
               strlen(ORIENTSEC_GRPC_REGISTRY_DEFAULT_PROTO_HEADER) +
                   strlen(pPub) + 1,
               "%s%s", ORIENTSEC_GRPC_REGISTRY_DEFAULT_PROTO_HEADER, pPub);
      size = strlen(zk_public_address);
      write_pub_addr(zk_public_address, size);
      factory = lookup_registry_factory(zk_public_address);
      g_zk_pub_registry_service =
          factory->get_registry_service(zk_public_address);
      factory = NULL;
    }

    if (!ret_pri) {
      snprintf(zk_private_address,
               strlen(ORIENTSEC_GRPC_REGISTRY_DEFAULT_PROTO_HEADER) +
                   strlen(pPri) + 1,
               "%s%s", ORIENTSEC_GRPC_REGISTRY_DEFAULT_PROTO_HEADER, pPri);
      size = strlen(zk_private_address);
      write_pri_addr(zk_private_address, size);
      factory = lookup_registry_factory(zk_private_address);
      g_zk_pri_registry_service =
          factory->get_registry_service(zk_private_address);
      factory = NULL;
    }

    // 调用consumer的registry的初始化
    zk_cons_reg_init();
    // add by yang, fix the memory leak during zk registry
    free(pPub);
    free(pPri);
    if ((g_zk_pub_registry_service != NULL) ||
        (g_zk_pri_registry_service != NULL)) {
      binit = true;
      return true;
    }
    return false;
  }
  return true;

}

// 注册中心注册
void registry(url_t* url) {
  if ((g_zk_pub_registry_service == NULL) &&
      (g_zk_pri_registry_service == NULL)) {
    gpr_log(GPR_ERROR, "call registry failed since interface init failed");
    return;
  }
  // 根据服务名做公有或者私有注册中心的注册
  registry_service_args_t args;
  if (0 == strcmp(url->protocol, "consumer")) {
    // consumer registry
    if (zk_cons_reg == PUBLIC_REG) {
      args.param = g_zk_pub_registry_service;
      g_zk_pub_registry_service->registe(&args, url);
    } else if (zk_cons_reg == PRIVATE_REG) {
      args.param = g_zk_pri_registry_service;
      g_zk_pri_registry_service->registe(&args, url);
    } else {
      args.param = g_zk_pub_registry_service;
      g_zk_pub_registry_service->registe(&args, url);
      args.param = g_zk_pri_registry_service;
      g_zk_pri_registry_service->registe(&args, url);
    }
  } else {
    // provider registry
    if (zk_prov_reg == PUBLIC_REG) {
      args.param = g_zk_pub_registry_service;
      g_zk_pub_registry_service->registe(&args, url);
    } else if (zk_prov_reg == PRIVATE_REG) {
      args.param = g_zk_pri_registry_service;
      g_zk_pri_registry_service->registe(&args, url);
    } else {
      args.param = g_zk_pub_registry_service;
      g_zk_pub_registry_service->registe(&args, url);
      args.param = g_zk_pri_registry_service;
      g_zk_pri_registry_service->registe(&args, url);
    }
  }
  return;
}
void unregistry(url_t* url) {
  orientsec_grpc_registry_zk_intf_init();
  if ((g_zk_pub_registry_service == NULL) &&
      (g_zk_pri_registry_service == NULL)) {
    gpr_log(GPR_ERROR,
            "call registry center failed since interface init failed");
    return;
  }
  // 根据服务名做公有或者私有注册中心的注销
  registry_service_args_t args;
  if (0 == strcmp(url->protocol, "consumer")) {
    // consumer registry
    if (zk_cons_reg == PUBLIC_REG) {
      args.param = g_zk_pub_registry_service;
      g_zk_pub_registry_service->unregiste(&args, url);
    } else if (zk_cons_reg == PRIVATE_REG) {
      args.param = g_zk_pri_registry_service;
      g_zk_pri_registry_service->unregiste(&args, url);
    } else {
      args.param = g_zk_pub_registry_service;
      g_zk_pub_registry_service->unregiste(&args, url);
      args.param = g_zk_pri_registry_service;
      g_zk_pri_registry_service->unregiste(&args, url);
    }

  } else {
    if (zk_prov_reg == PUBLIC_REG) {
      args.param = g_zk_pub_registry_service;
      g_zk_pub_registry_service->unregiste(&args, url);
    } else if (zk_prov_reg == PRIVATE_REG) {
      args.param = g_zk_pri_registry_service;
      g_zk_pri_registry_service->unregiste(&args, url);
    } else {
      args.param = g_zk_pub_registry_service;
      g_zk_pub_registry_service->unregiste(&args, url);
      args.param = g_zk_pri_registry_service;
      g_zk_pri_registry_service->unregiste(&args, url);
    }
  }
  return;
}

void subscribe(url_t* url, registry_notify_f notify_f) {
  orientsec_grpc_registry_zk_intf_init();
  if ((g_zk_pub_registry_service == NULL) &&
      (g_zk_pri_registry_service == NULL)) {
    gpr_log(GPR_ERROR, "call subscribe failed since interface init failed");
    return;
  }
  // 根据服务名做公有或者私有注册中心的订阅
  registry_service_args_t args;
  if (0 == strcmp(url->protocol, "consumer")) {
    // consumer registry
    if (zk_cons_reg == PUBLIC_REG) {
      args.param = g_zk_pub_registry_service;
      g_zk_pub_registry_service->subscribe(&args, url, notify_f);
    } else if (zk_cons_reg == PRIVATE_REG) {
      args.param = g_zk_pri_registry_service;
      g_zk_pri_registry_service->subscribe(&args, url, notify_f);
    } else {
      args.param = g_zk_pub_registry_service;
      g_zk_pub_registry_service->subscribe(&args, url, notify_f);
      args.param = g_zk_pri_registry_service;
      g_zk_pri_registry_service->subscribe(&args, url, notify_f);
    }
  } else {
    // for provider
    if (zk_prov_reg == PUBLIC_REG) {
      args.param = g_zk_pub_registry_service;
      g_zk_pub_registry_service->subscribe(&args, url, notify_f);
    } else if (zk_prov_reg == PRIVATE_REG) {
      args.param = g_zk_pri_registry_service;
      g_zk_pri_registry_service->subscribe(&args, url, notify_f);
    } else {
      args.param = g_zk_pub_registry_service;
      g_zk_pub_registry_service->subscribe(&args, url, notify_f);
      args.param = g_zk_pri_registry_service;
      g_zk_pri_registry_service->subscribe(&args, url, notify_f);
    }
  }
  return;
}

void unsubscribe(url_t* url, registry_notify_f notify_f) {
  orientsec_grpc_registry_zk_intf_init();
  if ((g_zk_pub_registry_service == NULL) &&
      (g_zk_pri_registry_service == NULL)) {
    gpr_log(GPR_ERROR, "call unsubscribe failed since interface init failed");
    return;
  }

  // 根据服务名做公有或者私有注册中心的取消订阅
  registry_service_args_t args;
  if (0 == strcmp(url->protocol, "consumer")) {
    // consumer registry
    if (zk_cons_reg == PUBLIC_REG) {
      args.param = g_zk_pub_registry_service;
      g_zk_pub_registry_service->unsubscribe(&args, url, notify_f);
    } else if (zk_cons_reg == PRIVATE_REG) {
      args.param = g_zk_pri_registry_service;
      g_zk_pri_registry_service->unsubscribe(&args, url, notify_f);
    } else {
      args.param = g_zk_pub_registry_service;
      g_zk_pub_registry_service->unsubscribe(&args, url, notify_f);
      args.param = g_zk_pri_registry_service;
      g_zk_pri_registry_service->unsubscribe(&args, url, notify_f);
    }
  } else {
    if (zk_prov_reg == PUBLIC_REG) {
      args.param = g_zk_pub_registry_service;
      g_zk_pub_registry_service->unsubscribe(&args, url, notify_f);
    } else if (zk_prov_reg == PRIVATE_REG) {
      args.param = g_zk_pri_registry_service;
      g_zk_pri_registry_service->unsubscribe(&args, url, notify_f);
    } else {
      args.param = g_zk_pub_registry_service;
      g_zk_pub_registry_service->unsubscribe(&args, url, notify_f);
      args.param = g_zk_pri_registry_service;
      g_zk_pri_registry_service->unsubscribe(&args, url, notify_f);
    }
  }
  return;
}

url_t* lookup(url_t* url, int* nums, int* pri_num) {
  url_t* ret = NULL;
  orientsec_grpc_registry_zk_intf_init();
  if ((g_zk_pub_registry_service == NULL) &&
      (g_zk_pri_registry_service == NULL)) {
    gpr_log(GPR_ERROR, "call lookup failed for intf init failed");
    return NULL;
  }

  // 根据服务名做公有或者私有注册中心的查询
  registry_service_args_t args;
  if (0 == strcmp(url->protocol, "consumer")) {
    // consumer registry
    if (zk_cons_reg == PUBLIC_REG) {
      args.param = g_zk_pub_registry_service;
      g_zk_pub_registry_service->lookup(&args, url, &ret, nums);
    } else if (zk_cons_reg == PRIVATE_REG) {
      args.param = g_zk_pri_registry_service;
      g_zk_pri_registry_service->lookup(&args, url, &ret, nums);
    } else {
      args.param = g_zk_pub_registry_service;
      g_zk_pub_registry_service->lookup(&args, url, &ret, nums);
      args.param = g_zk_pri_registry_service;
      g_zk_pri_registry_service->lookup(&args, url, &ret, pri_num);
    }
  } else {
    if (zk_prov_reg == PUBLIC_REG) {
      args.param = g_zk_pub_registry_service;
      g_zk_pub_registry_service->lookup(&args, url, &ret, nums);
    } else if (zk_prov_reg == PRIVATE_REG) {
      args.param = g_zk_pri_registry_service;
      g_zk_pri_registry_service->lookup(&args, url, &ret, nums);
    } else {
      args.param = g_zk_pub_registry_service;
      g_zk_pub_registry_service->lookup(&args, url, &ret, nums);
      args.param = g_zk_pri_registry_service;
      g_zk_pri_registry_service->lookup(&args, url, &ret, nums);
    }
  }

  return ret;
}

char* getData(const char* path) {
  orientsec_grpc_registry_zk_intf_init();
  if ((g_zk_pub_registry_service == NULL) &&
      (g_zk_pri_registry_service == NULL)) {
    gpr_log(GPR_ERROR, "call getData(%s) failed since interface init failed",
            path);
    return NULL;
  }
  // 根据服务名做公有或者私有注册中心的数据获取
  registry_service_args_t args;
  char* ret = NULL;
  if (zk_cons_reg == PUBLIC_REG) {
    args.param = g_zk_pub_registry_service;
    ret = g_zk_pub_registry_service->getData(&args, path);
  } else if (zk_cons_reg == PRIVATE_REG) {
    args.param = g_zk_pri_registry_service;
    ret = g_zk_pri_registry_service->getData(&args, path);
  } else {
    args.param = g_zk_pub_registry_service;
    ret = g_zk_pub_registry_service->getData(&args, path);
  }
  return ret;
}

void shutdown_registry() {
  orientsec_grpc_registry_zk_intf_init();
  if ((g_zk_pub_registry_service == NULL) &&
      (g_zk_pri_registry_service == NULL)) {
    gpr_log(GPR_ERROR,
            "call shutdown_registry failed since interface init failed");
    return;
  }

  registry_service_args_t args;
  // provider公有或者私有注册中心的销毁
  if (zk_prov_reg == PUBLIC_REG) {
    args.param = g_zk_pub_registry_service;
    g_zk_pub_registry_service->stop(&args);
    g_zk_pub_registry_service->destroy(&args);
  } else if (zk_prov_reg == PRIVATE_REG) {
    args.param = g_zk_pri_registry_service;
    g_zk_pri_registry_service->stop(&args);
    g_zk_pri_registry_service->destroy(&args);
  } else {  // HYBRID
    // 公有注册中心的销毁
    args.param = g_zk_pub_registry_service;
    g_zk_pub_registry_service->stop(&args);
    g_zk_pub_registry_service->destroy(&args);
    // 私有注册中心的销毁
    args.param = g_zk_pri_registry_service;
    g_zk_pri_registry_service->stop(&args);
    g_zk_pri_registry_service->destroy(&args);
  }

  return;
}
