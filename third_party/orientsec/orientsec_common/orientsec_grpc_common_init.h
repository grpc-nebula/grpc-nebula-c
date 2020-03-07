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
 *    Author : huyn
 *    2017/08/14
 *    version 0.0.9
 *    系统启动时进行的预初始化操作
 */

#ifndef ORIENTSEC_GRPC_COMMON_INIT_H
#define ORIENTSEC_GRPC_COMMON_INIT_H

#ifdef __cplusplus
extern "C" {
#endif
	#define ORIENTSEC_GRPC_COMMON_YES 1

    #define ORIENTSEC_GRPC_COMMON_NO 0

	//预初始化配置文件，读取配置文件相关参数
	void orientsec_grpc_common_param_init();

	//获取是否生成服务跟踪信息参数
	int orientsec_grpc_common_get_writekafka_enabled();

	//获取consumer对服务版本要求
	char * orientsec_grpc_consumer_service_version_get();

	//获取consumer允许缓存的provider数量
	int orientsec_grpc_cache_provider_count_get();

        //获取consumer对服务分组的要求
        //char *orientsec_grpc_consumer_service_group_get();

	int orientsec_grpc_common_get_writekafka_enabled_set(int enable);

       // 获取注册根目录
        char* orientsec_grpc_common_get_root_dir();

#ifdef __cplusplus
}
#endif
#endif // !ORIENTSEC_GRPC_COMMON_INIT_H
