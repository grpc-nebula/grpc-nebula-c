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
 *    2017/08/03
 *    version 0.0.9
 *    pick first负载均衡处理类方法定义
 */
#include <iostream>
#include "pickfirst_lb.h"
#include "registry_utils.h"
#include "orientsec_grpc_consumer_utils.h"
//addbyhuyn
#include "orientsec_grpc_consumer_control_version.h"
#include <grpc/support/time.h>
#include "orientsec_grpc_common_init.h"

//addbylm
#include "src/core/ext/filters/client_channel/lb_policy_registry.h"

pickfirst_lb::pickfirst_lb()
{
}

pickfirst_lb::~pickfirst_lb()
{
}

//随机选取一个可用服务
provider_t* pickfirst_lb::choose_provider(const char* sn, int step) {
  if (!providers || !sn)
  {
	  return NULL;
  }
  std::map<std::string, provider_t*>::iterator provider_lst_iter = providers->find(sn);
  if (provider_lst_iter == providers->end())
  {
	  return NULL;
  }
  int size = orientsec_grpc_cache_provider_count_get();
  srand(time(NULL));//设置随机数种子。

  int index = rand() % size;
  int index_org = index;

  bool do_while = true;
  bool is_invalid = true;
  int index_valid = -1;
  while (do_while)
  {
    is_invalid = is_provider_invalid(&provider_lst_iter->second[index]);
    if (is_invalid == false) { //查找到未禁用的服务
      //根据服务名和版本号校验是否匹配
      if (orientsec_grpc_consumer_control_version_match(provider_lst_iter->second[index].sInterface, provider_lst_iter->second[index].version) == true) {
	index_valid = index;
	break;              //找到未禁用，并且版本匹配的服务
      }
      else if (index_valid < 0) { //记录首个未禁用的服务
	index_valid = index;
      }
    }
    index = (index + 1) % size;
    if (index == index_org)
    {
      if (index_valid >= 0) {
	break;
      }
      else {
	return NULL;//没有合适的provider
      }
    }
  }
  return clone_provider(&provider_lst_iter->second[index_valid]);
}


void pickfirst_lb::set_providers(std::map<std::string, provider_t*> *_providers) {
  this->providers = _providers;
}

void pickfirst_lb::reset_cursor(const char* sn) {
  return;
}

int pickfirst_lb::choose_subchannel(const char* sn, provider_t *provider, const int*nums) {
  if (!sn) {
    return 0;
  }
  if (provider == NULL || *nums == 0) {
    return 0;
  }
  int size = *nums;
  srand(time(NULL));  //设置随机数种子
  int index = rand() % size;
  return index;
}
