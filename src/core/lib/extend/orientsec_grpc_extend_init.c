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

#include "orientsec_grpc_extend_init.h"
#include "orientsec_grpc_common_init.h"
#include "orientsec_grpc_consumer_control_version.h"
#include "orientsec_grpc_consumer_control_group.h"


/*
*  对象扩展业务运营，所需的参数进行初始化缓存操作。
*/

void orientsec_grpc_extend_param_init() {
  //初始化common层配置参数
  orientsec_grpc_common_param_init();
  orientsec_grpc_service_version_init();
  //orientsec_grpc_service_group_init();

}
