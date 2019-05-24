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
 *    负载均衡处理类接口定义
 */

#ifndef ORIENTSEC_LOADBALANCE_H
#define ORIENTSEC_LOADBALANCE_H
#include "orientsec_types.h"
#include<map>
#include<vector>
#include<string>

//addbylm
#include "src/core/ext/filters/client_channel/lb_policy_registry.h"

#ifdef __cplusplus
extern "C" {
#endif

class orientsec_grpc_loadbalance {
public:
	//根据服务名sn选择一个provider，step:步长（干扰因子），默认为0，即策略本身决定provider位置，非零值时接口可以影响
	//策略选择provider的算法（主要影响provider的位置）,保留接口，功能未实现
	virtual provider_t* choose_provider(const char* sn, int step = 0) = 0;

	////virtual void setProviders(std::map<std::string, std::vector<provider_t*> > *_providers) = 0;
    //addbylm
    virtual int choose_subchannel(const char* sn,provider_t *provider, const int*nums) = 0;

	virtual void set_providers(std::map<std::string, provider_t*> *_providers) = 0;

	virtual	void reset_cursor(const char* sn) = 0;
};

#ifdef __cplusplus
}
#endif

#endif

