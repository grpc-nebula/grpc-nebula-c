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
 *    2017/12/19
 *    version 0.1
 *    consistent hash 负载均衡处理类方法实现
 */

#include "url.h"
#include "registry_utils.h"
#include "orientsec_grpc_consumer_utils.h"

#include "orientsec_grpc_common_init.h"
#include "orientsec_grpc_consumer_control_version.h"
#include "consistent_hash_lb.h"

#include <cstdlib> 
#include <iostream>

const int REPLICA_NUMBER = 160;

template <class T>

int getArrayLen(T& array)
{
	return (sizeof(array) / sizeof(array[0]));
}


conistent_hash_lb::conistent_hash_lb()
{
	func = new cmd5_hash_fun();
	// make consistent hash object
	conhash = new ccon_hash(func);

	//init flag of rebuid tree
	set_rebuild_flag(true);
}

conistent_hash_lb::~conistent_hash_lb()
{
	delete func;
	delete conhash;
}

void conistent_hash_lb::set_providers(std::map<std::string, provider_t*>* _providers)
{
	this->providers = _providers;
}

//obsoleting
int conistent_hash_lb::choose_subchannel(const char * sn, provider_t * provider, const int * nums)
{
	return 0;
}

//choose provider based on value of hash "arg"
int conistent_hash_lb::choose_subchannel(const char * sn, provider_t * provider, const int * nums, const std::string &arg)
{
	if (!sn) {
		return 0;
	}
	if (provider == NULL || *nums == 0) {
		return 0;
	}
	// 得到服务列表
	int index = 0, index_valid = 0;
	int size = *nums;
	std::string factor;
	//bool rebuild_tree;
	//used for returning of provider node
	cnode_s * node;

	if (arg.empty())
		factor = sn;
	else
		factor = arg;

	//判断服务列表是否发生变化，如果发生变化，需要重新构造红黑树
	if (get_rebuild_flag() || !compare_provider_list(provider, size))  //两次不一样返回false
	{
		// rebuild hash tree
		gen_hash_rbtree(provider, size);
		// rebuild provider_list for comparing
		gen_provider_list(provider, nums);
		set_rebuild_flag(false);
	}

	//根据参数值选择指定的服务提供者
	node = conhash->lookup_node_s(factor.c_str());
	if (node != NULL)
	{
		if (prov_list.size() == 0)
			return index_valid;

		std::map<int, std::string>::iterator provider_iter = prov_list.begin();
		while (provider_iter != prov_list.end())  //???
		{
			if (strcmp(node->get_iden(), provider_iter->second.c_str()) == 0)
			{
				index_valid = index;
				break;

			}
			provider_iter++;
			index++;
		}
	}
	else
	{
		std::cout << "not find corresponding provider" << std::endl;
	}
	return index_valid;
}

void conistent_hash_lb::reset_cursor(const char * sn)
{
	std::map<std::string, int>::iterator iter = cursors.find(sn);
	if (iter == cursors.end())
	{
		cursors.insert(std::pair<std::string, int>(sn, -1));
		return;
	}
	iter->second = -1;
}


void conistent_hash_lb::gen_hash_rbtree(provider_t *prov, int num)
{
	// 根据provider list 生成 hash 红黑树
	 //every provider    &provider_lst_iter->second[index]  
	int i;
	char key[32] = { 0 };
	for (i = 0; i < num; ++i)
	{
		sprintf(key, "%s:%d", prov[i].host, prov[i].port);
		// index stored in data
		cnode_s * node = new cnode_s(key, REPLICA_NUMBER, (void*)i);
		// 虚拟节点的hash值到真实节点的映射
		conhash->add_node_s(node);
		memset(key, 0, 32);
	}
}


provider_t * conistent_hash_lb::choose_provider(const char * sn, const std::string arg, int step)
{
	// 得到服务列表
	if (!sn)
	{
		return NULL;
	}
	int index = -1, index_valid = -1;
	int size = 0;
	std::string factor;
	bool rebuild_tree = false;
	//用于返回找到的 provider 节点
	cnode_s * node;

	std::map<std::string, provider_t*>::iterator provider_iter = providers->find(sn);
	if (provider_iter == providers->end())
	{
		return NULL;
	}
	
        // fix divided by zero error
        size = orientsec_grpc_cache_provider_count_get();
        if (!size) return NULL;
        
        // object process
	if (arg.empty())
		factor = rand() % size;

	//判断服务列表是否发生变化，如果发生变化，需要重新构造红黑树
	provider_t *prov_list = provider_iter->second;
	if (rebuild_tree)
		gen_hash_rbtree(prov_list, 0);

	//根据参数值选择指定的服务提供者
	//用什么标识 provider?
	node = conhash->lookup_node_s(arg.c_str());
	if (node != NULL)
	{

		if (strcmp(node->get_iden(), "192.168.2.12:50033") == 0)
			index_valid = 1;
		if (strcmp(node->get_iden(), "192.168.2.12:50044") == 0)
			index_valid = 2;

	}
	else
	{
		std::cout << "not find corresponding provider" << std::endl;
	}

	//根据参数值选择指定的服务提供者

	return clone_provider(&provider_iter->second[index_valid]);

}

provider_t * conistent_hash_lb::choose_provider(const char * sn, int step)
{
	return NULL;
}

void conistent_hash_lb::set_rebuild_flag(bool flag)
{
	this->rebuild_tree = flag;
}

bool conistent_hash_lb::get_rebuild_flag()
{
	return rebuild_tree;
}

bool conistent_hash_lb::gen_provider_list(provider_t * provider, const int * nums)
{
	int i = 0;
	char prov_info[32] = { 0 };
	//根据传入provider列表,生成prov_list
	prov_list.clear();

	for (i = 0; i < *nums; ++i)
	{
		sprintf(prov_info, "%s:%d", provider[i].host, provider[i].port);
		prov_list.insert(std::pair<int, std::string>(i, prov_info));
	}
	if (prov_list.size() == *nums)
		return true;
	else
		return false;
}

bool conistent_hash_lb::compare_provider_list(provider_t * provider, int nums)
{
	//比对结果不一样返回false
	if ((int)(prov_list.size() != nums))
		return false;
	int comp_right_num = 0;
	int i = 0;
	char prov_info[32] = { 0 };
	//根据传入provider列表,生成prov_list
	std::map<int, std::string>::iterator provider_iter;
	for (i = 0; i < nums; ++i)
	{
		sprintf(prov_info, "%s:%d", provider[i].host, provider[i].port);
		provider_iter = prov_list.begin();
		while (provider_iter != prov_list.end())
		{
			if (provider_iter->second == prov_info)
			{
				comp_right_num++;
				break;
			}
			else {
				provider_iter++;
			}
		}
	}
	if (comp_right_num != nums)
	{
		return false;
	}
	else{
		return true;
	}

}
