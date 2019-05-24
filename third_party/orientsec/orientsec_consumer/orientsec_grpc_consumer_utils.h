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
 *    version 0.9
 *    consumer相关工具函数
 */
#pragma once
#ifndef ORIENTSEC_GRPC_CONSUMER_UTILS_H
#define ORIENTSEC_GRPC_CONSUMER_UTILS_H

#include "orientsec_grpc_common.h"
#include "url.h"
#include "orientsec_grpc_common_utils.h"
#include "orientsec_grpc_conf.h"
#include "registry_contants.h"
#include "orientsec_types.h"
#include <string>


#ifdef __cplusplus
extern "C" {
#endif	

//拼接生成consumer注册所需的字符串。
char *orientsec_grpc_consumer_url(const char *service_name);


bool is_match_glob_pattern(std::string pattern, std::string value, url_t* param);

bool is_match_glob_pattern2(std::string pattern, std::string value);

//判断provider是否处于非法状态，例如处于黑名单或者此前出现过连接失败，非法状态返回true
bool is_provider_invalid(provider_t* provider);

#ifdef __cplusplus
}
#endif


#endif // !ORIENTSEC_GRPC_CONSUMER_UTILS_H
