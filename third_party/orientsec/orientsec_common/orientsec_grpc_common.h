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
 *    配置文件读取相关接口函数
 */

#ifndef ORIENTSEC_GRPC_COMMON_H
#define ORIENTSEC_GRPC_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

	//配置项结构体
	struct _grpc_conf_template
	{
		char * key;      //配置文件中的键
		char * keyreg;   //拼接url串所有键
		char isrequied;  // 1 必填  其他值非必填
		char * defaultvalue;
	};

	typedef struct _grpc_conf_template grpc_conf_template_t;

#define ORIENTSEC_GRPC_COMMON_BLANK  ""

#define ORIENTSEC_GRPC_TRACE_PROTOCOL "grpc"

#ifdef __cplusplus
}
#endif
#endif // !ORIENTSEC_GRPC_COMMON_H
