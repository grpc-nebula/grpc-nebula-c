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
 *    2017/06/13
 *    version 0.0.9
 *    定义全局通用方法
 */

#ifndef ORIENTSEC_GRPC_COMMON_UTILS_H
#define ORIENTSEC_GRPC_COMMON_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

	
//定义通用函数
#define ORIENTSEC_GRPC_COMMON_ZK_SCHEME  "zookeeper:///"


//路由规则结构体
	typedef struct _orientsec_grpc_router_t
	{
		char *url;          //路由字符串
		char *servicename;  //服务名
		char *host;	        //ip地址
	} orientsec_grpc_router_t;


void orientsec_grpc_int_to_char(int inval,char * str);

int orientsec_grpc_getservicename_by_target(char * target);

//解析fullmethod内部包含的方法名称
//servicename 返回取到的方法名称
void orientsec_grpc_getserveicename_by_fullmethod(const char* fullmethod, char* servicename);

//char * orientsec_grpc_getserveice_by_fullmethod(const char* fullmethod);

int orientsec_grpc_getserveice_by_fullmethod(const char* fullmethod, char ** ret_param);

char * orientsec_grpc_getmethodname_by_fullmethod(const char* fullmethod);

int orientsec_grpc_router_free(orientsec_grpc_router_t **router);

//校验给的字符串指针是否为数字
int orientsec_grpc_common_utils_isdigit(char *num);

char* orientsec_provider_service_version();

char* orientsec_provider_service_group();

//生成uuid
void orientsec_grpc_uuid(char *my_uuid);


//获取consumer端所需要的provider版本
//获取内容不需要释放
char* orientsec_consumer_service_version();

//获取grpc版本
char* orientsec_grpc_version();
#ifdef __cplusplus
}
#endif
#endif // !ORIENTSEC_GRPC_COMMON_UTILS_H
