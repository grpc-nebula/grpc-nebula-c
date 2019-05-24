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
 *    基于权重的round robin 负载均衡处理类方法定义
 */
#include "weight_round_robin_lb.h"
#include "url.h"
#include "registry_utils.h"
#include "orientsec_grpc_consumer_utils.h"
//addbyhuyn
#include "orientsec_grpc_consumer_control_version.h"
#include "orientsec_grpc_common_init.h"
//addbylm
#include "src/core/ext/filters/client_channel/lb_policy_registry.h"

weight_round_robin_lb::weight_round_robin_lb()
{
}

weight_round_robin_lb::~weight_round_robin_lb()
{
	cursors.clear();
}

void weight_round_robin_lb::set_providers(std::map<std::string, provider_t*> *_providers) {
	this->providers = _providers;
}
void weight_round_robin_lb::reset_cursor(const char* sn) {
	std::map<std::string, int>::iterator iter = cursors.find(sn);
	if (iter == cursors.end())
	{
		cursors.insert(std::pair<std::string, int>(sn, -1));
		return;
	}
	iter->second = -1;
}
provider_t* weight_round_robin_lb::choose_provider(const char* sn, int step) {
	if (!sn)
	{
		return NULL;
	}
	int index = -1, index_org = -1;
	int size = 0;
	std::map<std::string, provider_t*>::iterator provider_iter = providers->find(sn);
	if (provider_iter == providers->end())
	{
		return NULL;
	}
	size = orientsec_grpc_cache_provider_count_get();
	std::map<std::string, int>::iterator curIter = cursors.find(sn);
	if (curIter == cursors.end())
	{
		cursors.insert(std::pair<std::string, int>(sn, -1));
		curIter = cursors.find(sn);
	}

	index = curIter->second;

	index = (index + 1) % size;
	index_org = index;
	bool do_while = true;
	bool is_invalid = true;
	int index_valid = -1;
	//找出从index开始第一个不在黑名单中的provider,并且版本匹配的服务
	while (do_while)
	{
		is_invalid = is_provider_invalid(&provider_iter->second[index]);
		if (is_invalid == false) { //查找到未禁用的服务
								   //根据服务名和版本号校验是否匹配
			if (orientsec_grpc_consumer_control_version_match(provider_iter->second[index].sInterface, provider_iter->second[index].version) == true) {
				index_valid = index;
				break;   //找到未禁用，并且版本匹配的服务
			}
			else if (index_valid < 0) { //记录首个未禁用的服务,但是版本不匹配的服务
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
	curIter->second = index_valid;
	return clone_provider(&provider_iter->second[index_valid]);
}

int weight_round_robin_lb::choose_subchannel(const char* sn, provider_t *provider, const int*nums) {
  if (!sn) {
    return 0;
  }
  if (provider == NULL || *nums == 0) {
    return 0;
  }
  int i;
  int index = -1;
  int total = 0;

  for (i = 0; i < *nums; i++)
  {
    provider[i].curr_weight += provider[i].weight;
    total += provider[i].weight;

    if (index == -1 || provider[index].curr_weight < provider[i].curr_weight)
    {
     index = i;
    }
  }
  provider[index].curr_weight -= total;
  return index;
}
