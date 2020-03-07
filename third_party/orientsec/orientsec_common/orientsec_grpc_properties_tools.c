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
 *    配置文件操作实现方法
 *    Modified: Jianbin Yang
 *    Add long item reading from configuration file
 */

#include "orientsec_grpc_properties_tools.h"
#include "orientsec_grpc_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#if (defined WIN64) || (defined WIN32)
#include<direct.h>
#else
#include<unistd.h>
#endif

#define ORIENTSEC_GRPC_PROPERTIES_LINUX_DEFAULT_PATH  "/etc/"
#define ORIENTSEC_GRPC_PROPERTIES_DEFAULT_DIR "config"

typedef struct _properties_param_t {
	char key[ORIENTSEC_GRPC_PROPERTY_KEY_MAX_LEN];
	char value[ORIENTSEC_GRPC_PROPERTY_VALUE_MAX_LEN];
}properties_param_t;

static bool properties_inited = false;
static properties_param_t properties[ORIENTSEC_GRPC_MAX_PERPERTIES_NUM];
static int properties_num = 0;

// 从字符串中提取key、value到缓存区中
int get_property_value(char *keyAndValue, char * key, char * value) {

	char *p = keyAndValue;
	char *pValue = NULL;
	char *key_start = keyAndValue;
	char *value_start = NULL;
	int line_len = strlen(keyAndValue);
	if (!keyAndValue || !key || !value || line_len < 3 || keyAndValue[0] == '#')
	{
		return -2;
	}
	p = strstr(keyAndValue, "=");
	if (p == NULL) {
		return -1;
	}
	value_start = p + 1;
	*p = '\0';
	trim(key_start, key);

	trim(value_start, value);

	return 0;
}

int orientsec_grpc_properties_init_inner(const char *filename) {
	FILE *pf = NULL;
	char line[ORIENTSEC_GRPC_BUF_LEN] = { 0 };
        char tmp[ORIENTSEC_GRPC_BUF_LEN] = {0};  //temporary character
        char str[ORIENTSEC_GRPC_BUF_LEN] = {0};  //存储拼接的长字符串
	int index = 0;
        bool need_strcat = false;
	if (properties_inited)
	{
		return 1;
	}
	pf = fopen(filename, "r");
	if (!pf)
	{
		printf("open %s failed\n", filename);
		return -1;
	}
	while (!feof(pf)) { 
		memset(line, 0, ORIENTSEC_GRPC_BUF_LEN);
                memset(tmp, 0, ORIENTSEC_GRPC_BUF_LEN);
		fgets(line, ORIENTSEC_GRPC_BUF_LEN, pf);
                // 续行符的处理
                if (strstr(line, "\\\n") != NULL){  //Found '\' in line
                  if (strlen(str) > 0) {   
                    trim(line, tmp);
                    strcat(str, tmp);
                  } else {
                    trim(line, tmp);
                    strcpy(str, tmp);
                  }
                  need_strcat = true;
                }
                // 需要拼接的，去下一行拼接
                if (need_strcat) {
                  need_strcat = false;
                  continue;   
                } else {
                  if (strlen(str) > 0) {
                    //strcat(str, ",");
                    strcat(str, line);
                    memset(line, 0, sizeof(line));
                    strcpy(line, str);
                  }
                }

		if (0 == get_property_value(line, properties[index].key, properties[index].value)) {
		  index++;
		}

                // 清理str
                memset(str, 0, ORIENTSEC_GRPC_BUF_LEN);
                
	}
	fclose(pf);
	properties_num = index;
	properties_inited = true;
	return 0;
}
//配置文件读取顺序
// 1、从系统环境变量ORIENTSEC_GRPC_CONFIG读取 
// 2、从当前目录下config文件夹读取
// 3、从当前目录下读取
int orientsec_grpc_properties_init() {
	if (properties_inited)
	{
		return 1;
	}

	char *p = NULL, *project_path = NULL;
	char conf_path[ORIENTSEC_GRPC_PATH_MAX_LEN] = { 0 };
	int ret = 0;
	int path_len = 0;
	int filename_len = strlen(ORIENTSEC_GRPC_PROPERTIES_FILENAME);
	int default_dir_len = strlen(ORIENTSEC_GRPC_PROPERTIES_DEFAULT_DIR);

#if (defined WIN64) || (defined WIN32)
	int separator_len = 2;
#else
	int separator_len = 1;
#endif	

	//从环境变量获取
	char *tmp_dir = getenv(ORIENTSEC_GRPC_CONF_ENV);
	if (tmp_dir != NULL) {
		path_len = strlen(tmp_dir);
		char * path = (char*)malloc(sizeof(char)*ORIENTSEC_GRPC_PATH_MAX_LEN);
		memset(path, 0, sizeof(char)*ORIENTSEC_GRPC_PATH_MAX_LEN);
		memcpy(path, tmp_dir, path_len * sizeof(char));
		p = path + path_len;
		snprintf(p, separator_len + filename_len + 1, "%c%s", PATH_SEPRATOR, ORIENTSEC_GRPC_PROPERTIES_FILENAME);
		printf("\nConfig path : %s\n", path);
		if (0 == orientsec_grpc_properties_init_inner(path))
		{
			p = NULL;
			free(path);
			return 0;
		}
	}


	p = _getcwd(conf_path, ORIENTSEC_GRPC_PATH_MAX_LEN);
	path_len = strlen(conf_path);
	if (path_len + strlen(ORIENTSEC_GRPC_PROPERTIES_FILENAME) >= ORIENTSEC_GRPC_PATH_MAX_LEN)
	{
		return -1;
	}
	project_path = strrchr(conf_path, PATH_SEPRATOR);
	p = conf_path + path_len;

	//处理./config/xxx.properties文件
	memset(p, 0, ORIENTSEC_GRPC_PATH_MAX_LEN - path_len);
	snprintf(p, 2 * separator_len + default_dir_len + filename_len + 1, "%c%s%c%s", PATH_SEPRATOR, ORIENTSEC_GRPC_PROPERTIES_DEFAULT_DIR,
		PATH_SEPRATOR, ORIENTSEC_GRPC_PROPERTIES_FILENAME);
	if (0 == orientsec_grpc_properties_init_inner(conf_path))
	{
		printf("\nConfig path : %s\n", conf_path);
		return 0;
	}

	//处理./xxx.properties文件 ,
	snprintf(p, separator_len + filename_len + 1, "%c%s", PATH_SEPRATOR, ORIENTSEC_GRPC_PROPERTIES_FILENAME);
	if (0 == orientsec_grpc_properties_init_inner(conf_path))
	{
		printf("\nConfig path : %s\n", conf_path);
		return 0;
	}
	return -2;
}

int orientsec_grpc_properties_get_value(const char *key, char *prefix, char *value) {
	char buf[ORIENTSEC_GRPC_BUF_LEN];
	int index = 0;
	char *pOut = value;
	memset(buf, 0, ORIENTSEC_GRPC_BUF_LEN);
	for (index = 0; index < properties_num; index++) {
		if (0 == strcmp(key, properties[index].key))
		{
			snprintf(pOut, strlen(properties[index].value) + 1, "%s", properties[index].value);
			return 0;
		}
		//处理有前缀的key情形
		if (prefix)
		{
			memset(buf, 0, ORIENTSEC_GRPC_BUF_LEN);
			snprintf(buf, strlen(prefix) + strlen(key) + 1, "%s%s", prefix, key);
			if (0 == strcmp(buf, properties[index].key))
			{
				snprintf(pOut, strlen(properties[index].value) + 1, "%s", properties[index].value);
				return 0;
			}
			memset(buf, 0, ORIENTSEC_GRPC_BUF_LEN);
			snprintf(buf, strlen(prefix) + strlen(properties[index].key) + 1, "%s%s", prefix, properties[index].key);
			if (0 == strcmp(buf, key))
			{
				snprintf(pOut, strlen(properties[index].value) + 1, "%s", properties[index].value);
				return 0;
			}
		}

	}
	return 1;
}
