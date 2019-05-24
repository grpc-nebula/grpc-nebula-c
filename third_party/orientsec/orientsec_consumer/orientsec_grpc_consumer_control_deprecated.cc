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

#include "orientsec_grpc_consumer_control_deprecated.h"
#include "orientsec_grpc_utils.h"
#include "grpc/support/log.h"
#include <map>
#include <string>

//存放所有设置已过期的服务
static std::map<std::string, uint64_t> g_orientsec_grpc_consumer_deprected;

/*
* 校验服务deprecated属性。
*/
void consumer_check_provider_deprecated(char * servicename, bool flag) {
	//服务未过时（更新）
	if (flag == false) {
		return;
	}
	//查找map，比较时间间隔
	uint64_t currenttime = orientsec_get_timestamp_in_mills();
	std::map<std::string, uint64_t>::iterator finditem = g_orientsec_grpc_consumer_deprected.find(servicename);
	if (finditem == g_orientsec_grpc_consumer_deprected.end()) {
		g_orientsec_grpc_consumer_deprected.insert(std::pair<std::string, uint64_t>(servicename, currenttime));
		gpr_log(GPR_ERROR, "当前服务[%s]的信息发生过变更，或者已经有新版本上线", servicename);
	}
	else if ((currenttime - finditem->second) > ORIENTSEC_GRPC_COMMON_DEPRECATED_TIP_INTERVAL) { //一天之内调用不再提示
		finditem->second = currenttime;
		gpr_log(GPR_ERROR, "当前服务[%s]的信息发生过变更，或者已经有新版本上线", servicename);
	}
}
