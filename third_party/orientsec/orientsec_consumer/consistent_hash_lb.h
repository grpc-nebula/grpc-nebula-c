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
 *    Author : jianbin yang(yangjianbin@beyondcent.com)
 *    2017/12/18
 *    version 0.9
 *    consistent hash 负载均衡处理类声明
 */

#ifndef CONSISTENT_HASH_LB_H
#define CONSISTENT_HASH_LB_H

#include "orientsec_loadbalance.h"
#include "src/core/ext/filters/client_channel/lb_policy_registry.h"
#include<vector>
#include<string>
#include<map>

#include "consistent_hash.h"

#ifdef __cplusplus
extern "C" {
#endif

	class conistent_hash_lb :
		public orientsec_grpc_loadbalance
	{
	public:
		conistent_hash_lb();
		~conistent_hash_lb();
		
		void set_providers(std::map<std::string, provider_t*> *_providers);
		int choose_subchannel(const char * sn, provider_t * provider, const int * nums);

		int choose_subchannel(const char * sn, provider_t * provider, const int * nums, const std::string &arg);
		

		void reset_cursor(const char* sn);

		void gen_hash_rbtree(provider_t * prov,int num);
		
		provider_t * choose_provider(const char * sn, const std::string arg, int step);

		provider_t* choose_provider(const char* sn, int step = 0);

		void set_rebuild_flag(bool flag);

		bool get_rebuild_flag();

		bool gen_provider_list(provider_t * provider, const int * nums);

		bool compare_provider_list(provider_t * provider, int nums);

	private:
		std::map<std::string, provider_t*> *providers;
		//某服务的选择的provider位置
		std::map<std::string, int> cursors;
		std::string argument;

		cmd5_hash_fun * func;
		ccon_hash * conhash;

		bool rebuild_tree;
		std::map<int, std::string> prov_list;
	};


#ifdef __cplusplus
}
#endif

#endif // !ORIENTSEC_GRPC_CONSISTENTHASH_H
