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
 *    黑白名单处理类接口声明
 */

#pragma once

#ifndef ORIENTSEC_ROUTER_H
#define ORIENTSEC_ROUTER_H
#include "url.h"
#include "orientsec_grpc_properties_tools.h"
#include "orientsec_grpc_utils.h"

#include<vector>

#ifdef __cplusplus
extern "C" {
#endif

class router
{
public:
	//virtual int route(std::vector<provider_t*> &providers, url_t *url_param) = 0;
	virtual int route(provider_t* providers, url_t *url_param) = 0;
};

#ifdef __cplusplus
}
#endif

#endif // !ORIENTSEC_ROUTER_H
