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
 *    黑白名单处理类声明
 */
#pragma once
#ifndef ORIENTSEC_CONDITION_ROUTER_H
#define ORIENTSEC_CONDITION_ROUTER_H

#include "orientsec_router.h"
#include "orientsec_grpc_consumer_utils.h"
#include <grpc/support/log.h>
#include <set>
#include <string>
#include <map>

#ifdef __cplusplus
extern "C" {
#endif

using namespace std;
class condition_router :
	public router
{
	class match_pair {
	public:
		set<string> matches;
		set<string> mismatches;
	public:
		bool is_match(string value, url_t* param) {
			int mathces_num = matches.size();
			int mismatches_num = mismatches.size();

			if (mathces_num > 0 && mismatches_num > 0) {
				gpr_log(GPR_ERROR,"路由规则配置出错，条件不能同时出现=和!=，请检查！");
				return false;
			}
			if (mathces_num == 0 && mismatches_num == 0) {
				gpr_log(GPR_ERROR, "ConditionRouter.MatchPair.isMatch程序发现一个不应该出现的情况");
				return false;
			} 

			bool default_value;
			if (mathces_num > 0) {
				default_value = false;// matches，如果都匹配不上返回false
			}
			else {
				default_value = true;// mismatches，如果都匹配不上返回true
			}

			for (std::set<std::string>::iterator it = matches.begin();it != matches.end();it++) {
				if (is_match_glob_pattern(*it, value, param)) {
					return true;
				}
			}

			for (std::set<std::string>::iterator it = mismatches.begin();it != mismatches.end();it++) {
				if (is_match_glob_pattern(*it, value, param)) {
					return false;
				}
			}

			return default_value;
		}
	};
	url_t *url;
	int priority;
	bool force;
	std::map<std::string, match_pair> when_condition;
	std::map<std::string, match_pair> then_condition;
private:
	bool match_when(url_t* url);
	bool match_then(url_t* url, url_t* param);
	bool match_condition(std::map<std::string, match_pair> condition, url_t* url, url_t* param);
public:
	condition_router(url_t* url_param);
	static std::map<std::string, match_pair> parse_rule(std::string rule);
	int route(provider_t* providers, url_t *url_param);
	int get_priority() { return priority; }
	~condition_router();
};


#ifdef __cplusplus
}
#endif

#endif // !ORIENTSEC_CONDITIONROUTER_H


