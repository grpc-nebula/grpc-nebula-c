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
 * func: 服务分组与分级控制
 * auth: Jianbin YANG
 * date: 20190911
 */

#ifndef ORIENTSEC_GRPC_CONSUMER_CONTROL_GROUP_H
#define ORIENTSEC_GRPC_CONSUMER_CONTROL_GROUP_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

//更新服务分组信息
void orientsec_grpc_consumer_control_group_update(char* service_name,
                                                    char* group_info);
//校验服务分组是否匹配
//bool orientsec_grpc_consumer_control_group_match(const char* servername,
 //                                                char* group_info);

//std::vector<std::string> orientsec_grpc_global_service_group_get();

//bool is_group_grading();

#ifdef __cplusplus
}
#endif

#endif  // !ORIENTSEC_GRPC_CONSUMER_CONTROL_GROUP_H
