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
 *    黑白名单处理类定义
 */

#include "condition_router.h"
#include <grpc/support/alloc.h>
#include <grpc/support/log.h>
#include <cstring>
#include <map>
#include <sstream>
#include "orientsec_grpc_common_init.h"
std::string& grpc_trim(std::string &s)
{
	if (s.empty())
	{
		return s;
	}

	s.erase(0, s.find_first_not_of(" "));
	s.erase(s.find_last_not_of(" ") + 1);
	return s;
}

void int2str(const int &int_temp, string &string_temp)
{
	stringstream stream;
	stream << int_temp;
	string_temp = stream.str();   //此处也可以用 stream>>string_temp  
}

int grpc_split_to_vec(const std::string& str, std::vector<string>& ret_, std::string sep = ",")
{
	if (str.empty())
	{
		return 0;
	}

	std::string tmp;
	std::string::size_type pos_begin = str.find_first_not_of(sep);
	std::string::size_type comma_pos = 0;

	while (pos_begin != string::npos)
	{
		comma_pos = str.find(sep, pos_begin);
		if (comma_pos != string::npos)
		{
			tmp = str.substr(pos_begin, comma_pos - pos_begin);
			pos_begin = comma_pos + sep.length();
		}
		else
		{
			tmp = str.substr(pos_begin);
			pos_begin = comma_pos;
		}

		if (!tmp.empty())
		{
			ret_.push_back(tmp);
			tmp.clear();
		}
	}
	return 0;
}

int grpc_split_to_set(const std::string& str, std::set<string>& ret_, std::string sep = ",")
{
	if (str.empty())
	{
		return 0;
	}

	std::string tmp;
	std::string::size_type pos_begin = str.find_first_not_of(sep);
	std::string::size_type comma_pos = 0;

	while (pos_begin != string::npos)
	{
		comma_pos = str.find(sep, pos_begin);
		if (comma_pos != string::npos)
		{
			tmp = str.substr(pos_begin, comma_pos - pos_begin);
			pos_begin = comma_pos + sep.length();
		}
		else
		{
			tmp = str.substr(pos_begin);
			pos_begin = comma_pos;
		}

		if (!tmp.empty())
		{
			ret_.insert(tmp);
			tmp.clear();
		}
	}
	return 0;
}


std::map<std::string, std::string> url_to_map(url_t *url) {
	std::map<std::string, std::string> result;
	if (url->protocol)
	{
		result.insert(std::pair<std::string, std::string>("protocol",url->protocol));
	}
	if (url->username)
	{
		result.insert(std::pair<std::string, std::string>("username", url->username));
	}
	if (url->password)
	{
		result.insert(std::pair<std::string, std::string>("password", url->password));
	}
	if (url->host)
	{
		result.insert(std::pair<std::string, std::string>("host", url->host));
	}

	if (url->port > 0)
	{
		std::string port_str;
		int2str(url->port, port_str);
		result.insert(std::pair<std::string, std::string>("port", port_str));
	}
	if (url->path)
	{
		result.insert(std::pair<std::string, std::string>("path", url->path));
	}
	for (size_t i = 0; i < url->params_num; i++)
	{
          // fix value = null error
          if (url->parameters[i].value)
	    result.insert(std::pair<std::string, std::string>(url->parameters[i].key, url->parameters[i].value));
	}
	return result;
}

condition_router::condition_router(url_t* url_param) {
	char *p = NULL;
	url = (url_t*)gpr_zalloc(sizeof(url_t));
	url_init(url);
	url_clone(url_param, url);
	p = url_get_parameter_v2(url, ORIENTSEC_GRPC_REGISTRY_KEY_PRIORITY, NULL);
  if (p == NULL)return;
	priority = atoi(p);
	p = url_get_parameter_v2(url, ORIENTSEC_GRPC_REGISTRY_KEY_FORCE, NULL);
  if (p == NULL)return;
	force = false;
	if (0 == strcmp(p,"true"))
	{
		force = true;
	}
	p = url_get_paramter_decode(url, (char*)ORIENTSEC_GRPC_REGISTRY_KEY_RULE, NULL);
	if (!p)
	{
		gpr_log(GPR_ERROR, "Illegal route rule!");
		return;
	}
	std::string rule = p;
	FREE_PTR(p);
	std::string consumer_prefix = "consumer.";
	std::string provider_prefix = "provider.";
	string::size_type   pos(0);
	//删除consumer.以及provider.前缀
	pos = rule.find(consumer_prefix);
	if (pos != std::string::npos)
	{
		rule = rule.replace(pos, consumer_prefix.length(), "");
	}
	pos = rule.find(provider_prefix);
	if (pos != std::string::npos)
	{
		rule = rule.replace(pos, provider_prefix.length(), "");
	}
	string::size_type i = rule.find("=>");
	std::string when_rule = (i == std::string::npos) ? "" : rule.substr(0, i);
	std::string then_rule = (i == std::string::npos) ? rule: rule.substr(i + 2);
	grpc_trim(when_rule);
	grpc_trim(then_rule);
	if (!when_rule.empty() && 0 != strcmp(when_rule.c_str(),"true"))
	{
		when_condition = parse_rule(when_rule);
	}
	if (!then_rule.empty() && 0 != strcmp(then_rule.c_str(), "false"))
	{
		then_condition = parse_rule(then_rule);
	}
}

std::map<std::string, condition_router::match_pair> condition_router::parse_rule(std::string rule) {
	std::map<string, condition_router::match_pair> condition;
	if (rule.empty())
	{
		return condition;
	}
	std::map<string, condition_router::match_pair>::iterator cond_iter;
	std::vector<std::string> buf_vec;
	string::size_type pos_begin;
	std::string rule_key, rule_value;
	std::string no_equal_str = "!=";
	std::string equal_str = "=";
	grpc_split_to_vec(rule, buf_vec, "&");//处理多个key=value的情形
	for (size_t i = 0; i < buf_vec.size(); i++)
	{
		std::string &item = buf_vec[i];
		pos_begin = item.find(no_equal_str);//先处理不等于逻辑，即白名单
		
		if (pos_begin != std::string::npos)
		{
			rule_key = item.substr(0, pos_begin);
			rule_value = item.substr(pos_begin + no_equal_str.length());
			grpc_trim(rule_key);
			grpc_trim(rule_value);
			cond_iter = condition.find(rule_key);
			if (cond_iter != condition.end())
			{
				match_pair &pair = cond_iter->second;
				std::set<std::string> &values = pair.mismatches;
				grpc_split_to_set(rule_value, values, ",");
			}
			else {
				match_pair pair;
				std::set<std::string> &values = pair.mismatches;
				grpc_split_to_set(rule_value, values, ",");
				condition.insert(std::pair<std::string, match_pair>(rule_key, pair));
			}
		}
		else {
			pos_begin = item.find_first_of(equal_str);
			if (pos_begin != std::string::npos)
			{
				rule_key = item.substr(0, pos_begin);
				rule_value = item.substr(pos_begin + equal_str.length());
				grpc_trim(rule_key);
				grpc_trim(rule_value);
				cond_iter = condition.find(rule_key);
				if (cond_iter != condition.end())
				{
					match_pair &pair = cond_iter->second;
					std::set<std::string> &values = pair.matches;
					grpc_split_to_set(rule_value, values, ",");
				}
				else {
					match_pair pair;
					std::set<std::string> &values = pair.matches;
					grpc_split_to_set(rule_value, values, ",");
					condition.insert(std::pair<std::string, match_pair>(rule_key, pair));
				}
			}
			else {
				gpr_log(GPR_ERROR, "Unsupport rule. rule:%s", item.c_str());
				continue;
			}
		}
	}
	return condition;
}

//算法优先考虑黑名单
//注意：本处直接操作g_valid_provider不太合理
int condition_router::route(provider_t* providers, url_t *url_param) {

	//匹配条件和过滤条件为空时。
	if ((when_condition.size() == 0) && (0 == then_condition.size())) {
           gpr_log(GPR_ERROR, "The current consumer in the service blacklist. consumer:%s", url_param->host);
           // 全部设置黑名单标记
          for (size_t i = 0;i <orientsec_grpc_cache_provider_count_get();i++) 
             ORIENTSEC_GRPC_SET_BIT( providers[i].flag_in_blklist,ORIENTSEC_GRPC_PROVIDER_FLAG_IN_BLKLIST);  //设置黑名单标记

	  return -1;
	}
		
	//校验定义的规则对改客户端是否有效
        // 无效直接返回，有效在下面设置黑白名单
	if (!match_when(url_param))
	{
		return 0;
	}
	int valid_provider_num = 0;;
	
	std::vector<int> provider_pos;
	for (size_t i = 0; i < orientsec_grpc_cache_provider_count_get(); i++)
	{
		if (ORIENTSEC_GRPC_CHECK_BIT(providers[i].flag_in_blklist, ORIENTSEC_GRPC_PROVIDER_FLAG_IN_BLKLIST))
		{
			continue;
		}

		if (providers[i].flag_invalid) { // 无效的provider不做处理
			continue;
		}

		//筛选条件为空或不在允许内
		if ((then_condition.size() == 0) || !match_then((url_t*)providers[i].ext_data, url_param)) {
			ORIENTSEC_GRPC_SET_BIT(providers[i].flag_in_blklist, ORIENTSEC_GRPC_PROVIDER_FLAG_IN_BLKLIST); //设置黑名单标记
			provider_pos.push_back(i);
		}
		else {		
			ORIENTSEC_GRPC_CHECK_BIT(providers[i].flag_in_blklist,ORIENTSEC_GRPC_PROVIDER_FLAG_IN_BLKLIST_NOT);
			valid_provider_num++;
		}
	}
	if (valid_provider_num > 0)
	{
		return 1;
	}
	else if (force)
	{
		gpr_log(GPR_ERROR, "The route result is empty and force execute. consumer:%s", url_param->host);
		return -1;
	}
	//如果valid_provider_num<=0 并且为非force模式,则不启用本条路由规则。
	// add by huyn   
	for (size_t i = 0; i < provider_pos.size(); i++)
	{
		ORIENTSEC_GRPC_SET_BIT(providers[provider_pos[i]].flag_in_blklist, ORIENTSEC_GRPC_PROVIDER_FLAG_IN_BLKLIST_NOT); //清除黑名单标记
	}
	return 0;
}
condition_router::~condition_router() {
	if (url)
	{
		url_full_free(&url);
	}
}

// 按照匹配条件进行匹配，客户端适用返回true，不适用返回false
bool condition_router::match_when(url_t* url) {
	if (when_condition.size() == 0) {
		return true;// 如果匹配条件为空，表示对所有消费方应用
	}
        bool ret;
	ret= match_condition(when_condition, url, NULL);
        return ret;
}

// url: provider param : consumer
bool condition_router::match_then(url_t* url, url_t* param) {
  bool ret_then;
	//return match_condition(then_condition, url, param);
  ret_then= match_condition(then_condition, url, param);
  return ret_then;
}

bool condition_router::match_condition(std::map<std::string, match_pair> condition, url_t* url, url_t* param) {
	std::map<std::string, std::string> sample = url_to_map(url);
	std::map<std::string, std::string>::iterator samp_iter;
	std::map<std::string, match_pair>::iterator cond_iter;
        std::vector<int> result;
        int multi = 0;
        bool match = false;
	for (samp_iter = sample.begin(); samp_iter != sample.end(); samp_iter++) {
		cond_iter = condition.find(samp_iter->first);
		if (cond_iter != condition.end())
		{
		  match_pair &pair = cond_iter->second;
                  match = pair.is_match(samp_iter->second, param);
                  //if (param == NULL) {
                  //  if (match) {
                  //    result.push_back(1);
                  //  } else {
                  //    return false;
                  //  }
                  //} else {
                  //  if (!match) {
                  //      return false;
                  //  }
                  //}
	           if (!match) {
                      return false;
	           }
		}
	}
        if (result.size() > 0) {
          for (auto it = result.begin(); it != result.end(); it++) multi += *it;
          if (multi != result.size()) return false;
        }
       
	return true;
}
