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
 *    通用操作函数 
 */
 
#ifndef ORIENTSEC_GRPC_UTILS_H
#define ORIENTSEC_GRPC_UTILS_H
#include "orientsec_types.h"

#if (defined WIN64) || (defined WIN32)
//#include <windows.h>
#else
#include <sys/types.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif
int orientsec_stricmp(const char* a, const char* b);


//获取provider端配置注册ip addr
char* get_provider_registry_ip();
//获取provider端配置注册port
int get_provider_registry_port();

//获取本地IP地址,返回字符指针无需释放
char *get_local_ip();

//获取进程ID
int grpc_getpid();

//返回当前时间（单位毫秒ms）
uint64_t orientsec_get_timestamp_in_mills();

//返回应用程序名，返回空间无需释放
char* orientsec_get_provider_appname();

//根据方法全名提取服务名,例如/com.test.hello/sayHello,返回com.test.hello,返回内容无需释放
char* get_service_name(char *method_full_name, char *buf, size_t buf_len);

//去除字符串开头以及结尾的空字符
void trim(char *strIn, char *strOut);

//版本信息格式化处理
char* grpc_verion_format(const char* str);


//获取执行程序的执行目录
#if (defined WIN64) || (defined WIN32)
#define PATH_SEPRATOR '\\'
#else
#define _getcwd getcwd
#define PATH_SEPRATOR '/'
#endif


// non C99 standard functions
//#if __STDC_VERSION__ >= 199901L
char *gprc_strdup(const char *str);
//#endif

#define CHECK_BUF(free_size) if((free_size) <= 0) {orientsec_log("buffer space is not enough");return -1;}
#define STRING(str) #str
#define FREE_PTR(ptr) if(ptr){free(ptr);ptr = NULL;}
#define REINIT(buf) {memset(buf,0,sizeof(buf));}


void orientsec_log(char *msg);

int countchar(char *str, char a);

//判断pattern字符串是否以value开头
bool is_start_with(const char *pattern, const char* value);

//判断pattern是否以value结尾
bool is_ends_with(const char *pattern, const char* value);

/*
拷贝字符串，不包括pEnd位置字符
必须确保buf空间足够大
*/
int copystr0(char* buf, char *pSrc, char *pEnd);

char *copystr(char *pSrc, char *pEnd);


//将负载均衡枚举类型转成字符串类型,返回空间无需释放
char* lb_strategy_to_str(loadbalance_strategy_t strategy);

//将负载均衡字符串类型转成枚举类型
loadbalance_strategy_t lb_strategy_from_str(char *strategy);

//将故障切换枚举类型转成字符串类型，范湖空间无需释放
char* cluster_strategy_to_str(cluster_strategy_t strategy);

//将故障切换字符串类型转成枚举类型
cluster_strategy_t cluster_strategy_from_str(char *strategy);

//新分配provider结构体对象，并初始化字段
provider_t* new_provider();


//根据配置文件初始化provider特定字段
enum RegCode init_provider(provider_t* provider);

//释放provider空间
void free_provider(provider_t **p);

//释放provider空间
void free_provider_v2(provider_t *p);

/*
* 使用线程Sleep指定毫秒数。
*/
void orientsec_grpc_sleep_mills(int64_t millsecond);

/*
* set consumer/provider flag
*/
void orientsec_grpc_side_set(int side);

/*
* 写入log信息
*/
void orientsec_grpc_debug_log(const char *loginfo);

//获取threadid
int orientsec_grpc_threadid_get();

#ifdef __cplusplus
}
#endif

#endif // !ORIENTSEC_GRPC_UTILS_H
