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
 * func：服务过时校验
 * date：20170821
 * auth：addbyhuyn
 */

#pragma once
#ifndef ORIENTSEC_GRPC_CONSUMER_CONTROL_DEPRECATED_H
#define ORIENTSEC_GRPC_CONSUMER_CONTROL_DEPRECATED_H

#ifdef __cplusplus
extern "C" {
#endif

	//服务过期提醒时间间隔（单位毫秒） 默认为86400000毫秒即一天
#define ORIENTSEC_GRPC_COMMON_DEPRECATED_TIP_INTERVAL 86400000


/*
* 校验服务deprecated属性。
*/
	void consumer_check_provider_deprecated(char * servicename, bool flag);

#ifdef __cplusplus
}
#endif
#endif // !ORIENTSEC_GRPC_CONSUMER_CONTROL_DEPRECATED_H



