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

#ifndef ORIENTSEC_GRPC_PROPERTIES_TOOLS_H
#define ORIENTSEC_GRPC_PROPERTIES_TOOLS_H

#ifdef __cplusplus
extern "C" {
#endif

#define ORIENTSEC_GRPC_PATH_MAX_LEN 2048
#define ORIENTSEC_GRPC_BUF_LEN 2048

//配置文件中key最大长度
#define ORIENTSEC_GRPC_PROPERTY_KEY_MAX_LEN 100

//配置文件中value最大长度
#define ORIENTSEC_GRPC_PROPERTY_VALUE_MAX_LEN 1024

//配置文件中最大属性（key=value）条数
#define ORIENTSEC_GRPC_MAX_PERPERTIES_NUM 100

#define ORIENTSEC_GRPC_CONF_ENV "ORIENTSEC_GRPC_CONFIG"

//默认的配置文件名
#define ORIENTSEC_GRPC_PROPERTIES_FILENAME "dfzq-grpc-config.properties"

//加载配置文件,配置文件名由宏ORIENTSEC_GRPC_PROPERTIES_FILENAME定义
//默认配置文件查找顺序，当前执行目录./， ./config/, ../config目录下, /etc/目录下,打开成功返回0,
// 示例 
// .bin/xxx.exe
// .conf/orientsec-grpc-c-config.conf
int orientsec_grpc_properties_init();

//从配置文件中读取指定key的属性值，支持前缀方式读写,返回值0代表成功
//例如 consumer.version <=> version,其中prefix="consumer."
int orientsec_grpc_properties_get_value(const char *key, char *prefix, char *value);


#ifdef __cplusplus
}
#endif
#endif // !ORIENTSEC_GRPC_PROPERTIES_TOOLS_H
