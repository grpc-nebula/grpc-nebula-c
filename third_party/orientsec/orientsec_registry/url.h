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
 *    url链接定义
 */

#ifndef ORIENTSEC_URL_H
#define ORIENTSEC_URL_H

#ifdef __cplusplus
extern "C" {
#endif

/**
* Dependencies
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>


/**
* url.h version
*/

#define URL_VERSION 0.0.2


/**
* Max length of a url protocol scheme
*/

#define URL_PROTOCOL_MAX_LENGTH 16


/**
* Max length of a url host part
*/

#define URL_HOSTNAME_MAX_LENGTH 128


/**
* Max length of a url tld part
*/

#define URL_TLD_MAX_LENGTH 16


/**
* Max length of a url auth part
*/

#define URL_AUTH_MAX_LENGTH 32

#define URL_PARAM_KEY_MAX_LENGTH 512
#define URL_PARAM_VALUE_MAX_LENTH 1024

#define ORIENTSEC_GRPC_URL_MAX_LEN 2048

//#define ORIENTSEC_GRPC_SET_BIT(x,y) ((x)|=(0x01<<(y))) //置位
//#define ORIENTSEC_GRPC_CLR_BIT(x,y) ((x)&=(~(0x01<<(y)))) //清零
//#define ORIENTSEC_GRPC_CHECK_BIT(x,y) ((x)&(0x01<<(y))) //检测

#define ORIENTSEC_GRPC_SET_BIT(x,y) (x=y)  //置位
//#define ORIENTSEC_GRPC_CLR_BIT(x,y) ((x)&=(~(0x01<<(y)))) //清零
#define ORIENTSEC_GRPC_CHECK_BIT(x,y) (x==y) //检测

#define ORIENTSEC_GRPC_URL_MATCH_POS 0
#define ORIENTSEC_GRPC_URL_OVERRIDE_MATCH_POS (ORIENTSEC_GRPC_URL_MATCH_POS+1)
/**
* `url_t` struct that defines parts
* of a parsed URL such as host and protocol
*/
//定义url的key=value参数
typedef struct _url_param {
	char *key;
	char *value;
}url_param,*p_url_param;

//url结构体
typedef struct _url_t {
	char *href;             // URL
	char *protocol;         // 协议
	char *username;         // 用户名
	char *password;         // 密码
	char *auth;             // 认证
	char *host;             // 作用主机， 指定IP或者0.0.0.0
	int  port;              // 作用端口
	char *path;             // 作用路径
	int params_num;         // 变量数量
	url_param *parameters;  // 变量名称和数值
	int flag;               // 标志位
} url_t;


// prototype

/**
* Parses a url into parts and returns
* a `url *` pointer
*/
url_t *url_parse(char *url_data);

//解析串填充到url_t中
int url_parse_v2(char *url_data, url_t *pUrl);

//复制url_t src中的内容到url_t dest中
int url_clone(url_t *src, url_t *dest);

void url_init(url_t *url);

/**
* 返回指定key的参数值，使用完之后需要释放空间
* prefix指定前缀，例如default.version  <<==>> version
**/
char *url_get_parameter(url_t *url,const char *key, char *prefix);

char *url_get_raw_parameter(url_t *url, const char *key);

/**
* 返回指定key的参数值，返回值是url中value的地址，不需要释放
* prefix指定前缀，例如default.version  <<==>> version
**/
char *url_get_parameter_v2(url_t *url,const  char *key, char *prefix);

/**
* 更新url结构体parameters的key属性对应的值，如果key不存在，则新建属性并赋值为value
* 返回值： 0：更新成功，1，新建属性成功，-1：错误
**/
int url_update_parameter(url_t *url, char *key, char *value);

/*
返回指定参数key的解码后的参数值，使用完之后需要释放空间
*/
char *url_get_paramter_decode(url_t *url, char *key, char *prefix);

/*
获取服务接口，返回新分配的空间，用完后需要释放
*/
char *url_get_service_interface(url_t *url);

/*
获取服务key,key可以包括interface,group等信息,返回新分配的空间，用完后需要释放
*/
char *url_get_service_key(url_t *url);


/*
将url转成字符串形式，返回char*，使用完之后需要释放空间
*/
char *url_to_string(url_t *url);

/*
将url转成字符串保存到buf中，返回0表示成功,其他值表示失败
*/
int url_to_string_buf(url_t *url, char *buf,int buf_len);

/**
* 返回编码后的串，用完后需要释放空间
**/
char *url_encode(char *s);

int url_encode_buf(char *s, char *buf, int buf_len);

/**
* 返回解码后的串，用完后需要释放空间
**/
char *url_decode(char *str);

int url_decode_buf(char *str,char *buf, int buf_len);

void url_free(url_t *data);
void url_full_free(url_t **data);

void url_inspect(char *url);

/*
打印url内容
*/
void url_data_inspect(url_t *data);


//根据filter过滤url
int filterUrls(url_t *src, int src_num, url_t *filter);

//缓存的Provider最大数量
//#define ORIENTSEC_GRPC_PROVIDER_MAX_SIZE 5

#ifdef __cplusplus
}
#endif

#endif // !ORIENTSEC_URL_H

