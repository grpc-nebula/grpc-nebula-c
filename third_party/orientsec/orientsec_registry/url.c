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
 *    url链接操作函数实现
 */

#include "registry_contants.h"
#include "url.h"
#include "registry_utils.h"
#include "orientsec_grpc_utils.h"
#include <grpc/support/log.h>
#include <stdio.h>
#include <ctype.h>
#include<string.h>

static char *strff(char *ptr, int n) {
	int i;
	int y = 0;
	for (i = 0; i < n; ++i) {
		y = *ptr++;
	}

	return gprc_strdup(ptr);
}

static char *strrwd(char *ptr, int n) {
	int y = 0;
	int i;
	for (i = 0; i < n; ++i) {
		y = *ptr--;
	}

	return gprc_strdup(ptr);
}


void url_init(url_t *url) {
	if (url)
	{
		url->href = NULL;
		url->protocol = NULL;
		url->username = NULL;
		url->password = NULL;
		url->auth = NULL;
		url->host = NULL;
		url->path = NULL;
		url->port = 0;
		url->params_num = 0;
		url->parameters = NULL;
		url->flag = 0;
	}
}

url_t *url_parse(char *url_data) {
	url_t *url = (url_t*)malloc(sizeof(url_t));
	if (!url)
	{
		gpr_log(GPR_ERROR, "[url_parse] malloc memory failed");
		return NULL;
	}
	url_init(url);
	if (0 == url_parse_v2(url_data, url))
	{
		return url;
	}
	url_free(url);
	FREE_PTR(url);
	return NULL;
}

//解析串填充到url_t *pUrl中
int url_parse_v2(char *url_data, url_t *pUrl) {
	char *tmp_url = NULL, *tmp_url_cache = NULL;
	char *paramStart = NULL;
	char *pHold = NULL;
	char *pToken = NULL;
	char cParamDelimiter = '&';
	char cKVDelimiter = '=';
	char *p = NULL;
	char buf[2048];
	int i = 0;
	if (!pUrl || !url_data)
	{
		gpr_log(GPR_ERROR, "[url_parse]invalid arguments url_data or pUrl ");
		return -1;
	}
	pUrl->href = gprc_strdup(url_data);
	tmp_url_cache = tmp_url = gprc_strdup(url_data);
	paramStart = strchr(tmp_url, '?');
	if (paramStart > 0)
	{
		*paramStart = '\0';
		paramStart = paramStart + 1;
		int params_num = countchar(paramStart, '&') + 1;
		pUrl->params_num = params_num;
		pUrl->parameters = malloc(sizeof(url_param) * params_num);
		pToken = strchr(paramStart, cParamDelimiter);
		while (pToken)
		{
			memset(buf, 0, 2048);
			snprintf(buf, pToken - paramStart + 1, "%s", paramStart);
			p = strchr(buf, '=');
			if (p)
			{
				pUrl->parameters[i].key = copystr(buf, p);
				pUrl->parameters[i].value = copystr(p + 1, NULL);
			}
			else {
				pUrl->parameters[i].key = copystr(paramStart, pToken);
				pUrl->parameters[i].value = copystr(paramStart, pToken);
			}
			paramStart = pToken + 1;
			pToken = strchr(paramStart, cParamDelimiter);
			i++;
		}
		memset(buf, 0, 2048);
		snprintf(buf, strlen(paramStart) + 1, "%s", paramStart);
		p = strchr(buf, '=');
		if (p)
		{
			pUrl->parameters[i].key = copystr(buf, p);
			pUrl->parameters[i].value = copystr(p + 1, NULL);
		}
		else {
			pUrl->parameters[i].key = copystr(paramStart, NULL);
			pUrl->parameters[i].value = copystr(paramStart, NULL);
		}

	}
	p = strstr(tmp_url, "://");
	if (p >= tmp_url) {
		if (p == tmp_url)
		{
			gpr_log(GPR_ERROR, "Invalid url str");
			return 1;
		}
		pUrl->protocol = copystr(tmp_url, p);
		tmp_url = p + 3;
	}
	else {
		p = strstr(tmp_url, ":/");
		if (!p)
		{
			gpr_log(GPR_ERROR, "Invalid url str");
			return 2;
		}
		pUrl->protocol = copystr(tmp_url, p);
		tmp_url = p + 2;
	}
	p = strchr(tmp_url, '/');
	if (p)
	{
		pUrl->path = copystr(p + 1, NULL);
		*p = '\0';
	}
	p = strchr(tmp_url, '@');
	if (p)
	{
		i = copystr0(buf, tmp_url, p);

		tmp_url = p + 1;

		p = strchr(buf, ':');
		if (p) {
			pUrl->username = copystr(buf, p);
			pUrl->password = copystr(p + 1, NULL);
		}
	}
	p = strchr(tmp_url, ':');
	if (p && strlen(p) > 1)
	{
		copystr0(buf, p + 1, NULL);
		pUrl->port = atol(buf);
		*p = '\0';
	}
	if (strlen(tmp_url) > 0)
	{
		pUrl->host = copystr(tmp_url, NULL);
	}
	if (tmp_url_cache)
	{
		free(tmp_url_cache);
	}
	return 0;
}

//复制url_t src中的内容到url_t dest中
int url_clone(url_t *src, url_t *dest) {
	size_t i = 0;
	if (!src || !dest)
	{
		gpr_log(GPR_INFO, "url_clone:src or dest is null");
		return -1;
	}
	if (src->href)
	{
		dest->href = gprc_strdup(src->href);
	}
	if (src->protocol)
	{
		dest->protocol = gprc_strdup(src->protocol);
	}
	if (src->auth)
	{
		dest->auth = gprc_strdup(src->auth);
	}
	if (src->username)
	{
		dest->username = gprc_strdup(src->username);
	}
	if (src->password)
	{
		dest->password = gprc_strdup(src->password);
	}
	if (src->host)
	{
		dest->host = gprc_strdup(src->host);
	}
	dest->port = src->port;
	if (src->path)
	{
		dest->path = gprc_strdup(src->path);
	}
	dest->params_num = src->params_num;
	if (src->params_num > 0)
	{
		dest->parameters = (url_param *)malloc((src->params_num) * sizeof(struct _url_param));
	}
	for (i = 0; i < src->params_num; i++)
	{
		dest->parameters[i].key = gprc_strdup(src->parameters[i].key);
		dest->parameters[i].value = gprc_strdup(src->parameters[i].value);
	}
	return 0;
}

char *url_get_parameter(url_t *url, const char *key, char *prefix) {
	char key_buf[URL_PARAM_KEY_MAX_LENGTH];
	char search_key_full[URL_PARAM_KEY_MAX_LENGTH];
	size_t i;
	if (!url || !key)
	{
		gpr_log(GPR_INFO, "url or key is null");
		return NULL;
	}
	sprintf(search_key_full, "%s%s", prefix, key);
	for (i = 0; i < url->params_num; i++)
	{
		if (!url->parameters[i].key)
		{
			continue;
		}
		if (strcmp(url->parameters[i].key, key) == 0)
		{
			return gprc_strdup(url->parameters[i].value);
		}
		if (prefix)
		{
			if (strcmp(url->parameters[i].key, search_key_full) == 0)
			{
				return gprc_strdup(url->parameters[i].value);
			}
			memset(key_buf, 0, URL_PARAM_KEY_MAX_LENGTH);
			sprintf(key_buf, "%s%s", prefix, url->parameters[i].key);
			if (strcmp(key_buf, key) == 0)
			{
				return gprc_strdup(url->parameters[i].value);
			}

		}
	}
	if (0 == strcmp(ORIENTSEC_GRPC_CATEGORY_KEY, key))
	{
		return gprc_strdup(ORIENTSEC_GRPC_DEFAULT_CATEGORY);
	}
	if (0 == strcmp(ORIENTSEC_GRPC_REGISTRY_KEY_INTERFACE, key))
	{
		return gprc_strdup(url->path);
	}
	return NULL;
}

char *url_get_raw_parameter(url_t *url, const char *key) {
	char buf[50] = { 0 };
	if (0 == strcmp(key, "protocol"))
	{
		return gprc_strdup(url->protocol);
	}
	else if (0 == strcmp(key, "username"))
	{
		return gprc_strdup(url->username);
	}
	else if (0 == strcmp(key, "password"))
	{
		return gprc_strdup(url->password);
	}
	else if (0 == strcmp(key, "host"))
	{
		return gprc_strdup(url->host);
	}
	else if (0 == strcmp(key, "port"))
	{
		sprintf(buf, "%d", url->port);
		return gprc_strdup(buf);
	}
	else if (0 == strcmp(key, "path"))
	{
		return gprc_strdup(url->path);
	}
	else {
		return url_get_parameter(url, key, NULL);
	}
}

char *url_get_parameter_v2(url_t *url, const char *key, char *prefix) {
	char key_buf[URL_PARAM_KEY_MAX_LENGTH];
	char search_key_full[URL_PARAM_KEY_MAX_LENGTH];
	char *ret = NULL;
	size_t i;
	if (!url || !key)
	{
		gpr_log(GPR_INFO, "url or key is null");
		return NULL;
	}
	sprintf(search_key_full, "%s%s", prefix, key);
	for (i = 0; i < url->params_num; i++)
	{
		if (!url->parameters[i].key)
		{
			continue;
		}
		if (url->parameters[i].key != NULL && 0 == strcmp(url->parameters[i].key, key))
		{
			return url->parameters[i].value;
		}
		if (prefix)
		{
			if (url->parameters[i].key != NULL && 0 == strcmp(url->parameters[i].key, search_key_full))
			{
				return url->parameters[i].value;
			}
			memset(key_buf, 0, URL_PARAM_KEY_MAX_LENGTH);
			sprintf(key_buf, "%s%s", prefix, url->parameters[i].key);
			if (strcmp(key_buf, key) == 0)
			{
				return url->parameters[i].value;
			}

		}
	}
	if (0 == strcmp(ORIENTSEC_GRPC_CATEGORY_KEY, key))
	{
		return ORIENTSEC_GRPC_DEFAULT_CATEGORY;
	}
	if (0 == strcmp(ORIENTSEC_GRPC_REGISTRY_KEY_INTERFACE, key))
	{
		return url->path;
	}
	return NULL;
}

int url_update_parameter(url_t *url, char *key, char *value) {
	size_t i = 0;
	char *key_ptr = NULL;
	bool bFind = false;
	int ret = 0;
	if (!url || !key || !value)
	{
		return -1;
	}
	for (i = 0; i < url->params_num; i++)
	{
		key_ptr = url->parameters[i].key;
		if (!key_ptr)
		{
			continue;
		}
		if (0 == strcmp(key, key_ptr))
		{
			bFind = true;
			break;
		}
	}
	if (!bFind)
	{
		url->params_num++;
		url->parameters = realloc(url->parameters, sizeof(url_param) * (url->params_num + 1));
		url->parameters[i].key = gprc_strdup(key);
		url->parameters[i].value = gprc_strdup(value);
		ret = 1;
	}
	else {
		if ((url->parameters[i].value) && (0 == strcmp(value, url->parameters[i].value)))
		{
			ret = 0;
		}
		else {
			FREE_PTR(url->parameters[i].value);
			url->parameters[i].value = gprc_strdup(value);
		}
	}
	return ret;
}

char *url_get_paramter_decode(url_t *url, char *key, char *prefix) {
	char *buf = url_get_parameter(url, key, prefix);
	if (buf == NULL)return;
	size_t len = strlen(buf);
	if (buf && len > 0)
	{
		char *buf_decode = url_decode(buf);
		if (buf_decode)
		{
			FREE_PTR(buf);
			return buf_decode;
		}
	}
	return NULL;
}

char *url_get_service_interface(url_t *url) {
	return url_get_parameter(url, ORIENTSEC_GRPC_REGISTRY_KEY_INTERFACE, NULL);
}

// reserved,not use now
char *url_get_service_key(url_t *url) {
	char *intf = url_get_parameter(url, ORIENTSEC_GRPC_REGISTRY_KEY_INTERFACE, NULL);
	char *version = NULL, *group = NULL;
	int intf_len = 0, version_len = 0, group_len = 0;
	int skip_len = 0;
	char *buf = NULL, *p = NULL;
	if (!intf)
	{
		gpr_log(GPR_INFO, "url dont contains interface key");
		return NULL;
	}
	intf_len = strlen(intf);
	version = url_get_parameter(url, ORIENTSEC_GRPC_REGISTRY_KEY_VERSION, NULL);
	version_len = strlen(version);
	group = url_get_parameter(url, ORIENTSEC_GRPC_REGISTRY_KEY_GROUP, NULL);
	group_len = strlen(group);

	p = buf = (char*)malloc(sizeof(char) * (intf_len + 1 + version_len + 1 + group_len + 1));
	if (!buf) {
		gpr_log(GPR_ERROR, "url_get_service_key malloc memeory failed");
		return NULL;
	}
	memset(buf, 0, intf_len + 1 + version_len + 1 + group_len + 1);
	if (group && group_len > 0)
	{
		strcat(p, group);
		strcat(p, "/");
		p = p + group_len + 1;
	}
	strcat(p, intf);
	p = p + intf_len;
	if (version && version_len > 0)
	{
		strcat(p, ":");
		p = p + 1;
		strcat(p, version);
	}
	FREE_PTR(intf);
	FREE_PTR(version);
	FREE_PTR(group);
	return buf;
}

static unsigned char hexchars[] = "0123456789ABCDEF";

static int url_htoi(char *s)
{
	int value;
	int c;

	c = ((unsigned char *)s)[0];
	if (isupper(c))
		c = tolower(c);
	value = (c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10) * 16;

	c = ((unsigned char *)s)[1];
	if (isupper(c))
		c = tolower(c);
	value += c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10;

	return (value);
}

/*
使用parameters填充缓存区
buf:缓存区
buf_len:缓存区可用长度
url
concat: true,添加前置&,false
*/
int build_paramters(char* buf, size_t buf_len, url_t *url, bool concat) {
	bool first = true;
	size_t len = 0;
	char *p = buf;
	char *key = NULL;
	char *value = NULL;
	size_t i;
	if (url->parameters && url->params_num > 0)
	{
		for (i = 0; i < url->params_num; i++)
		{
			key = url->parameters[i].key;
			if (!key)
			{
				continue;
			}
			value = url->parameters[i].value;
			len = strlen(key);
			if (key && len > 0)
			{
				CHECK_BUF(buf_len - len - 2);
				if (first)
				{
					if (concat) {
						strcat(p, "?");
						p = p + 1;
						buf_len = buf_len - 1;
					}
					first = false;
				}
				else {
					strcat(p, "&");
					p = p + 1;
					buf_len = buf_len - 1;
				}
				strcat(p, key);
				p = p + len;
				buf_len = buf_len - len;
				strcat(p, "=");
				p = p + 1;
				buf_len = buf_len - 1;
				if (value)
				{
					len = strlen(value);
					CHECK_BUF(buf_len - len);
					strcat(p, value);
					p = p + len;
					buf_len = buf_len - len;
				}

			}
		}

	}
	return 0;
}

int url_to_string_buf(url_t *url, char *buf, int buf_len) {
	char *p = buf;
	size_t len = 0;
	size_t size = 0;
	char tmp_buf[2048];
	char *ptr = NULL;
	if (!url || !buf)
	{
		gpr_log(GPR_ERROR, "url is null or buf is invalid");
		return -1;
	}
	if (url->protocol)
	{
		len = strlen(url->protocol);
		if (len > 0) {
			CHECK_BUF(buf_len - len);
			strcat(p, url->protocol);
			p = p + len;
			buf_len -= len;

			len = strlen("://");
			CHECK_BUF(buf_len - len);
			strcat(p, "://");
			p = p + len;
			buf_len -= len;
		}

	}
	if (url->username)
	{
		len = strlen(url->username);
		if (len > 0)
		{
			CHECK_BUF(buf_len - len);
			strcat(p, url->username);
			p = p + len;
			buf_len -= len;

			if (url->password)
			{
				len = strlen(url->password);
				if (len > 0)
				{
					CHECK_BUF(buf_len - len - 1);
					strcat(p, ":");
					p = p + 1;
					strcat(p, url->password);
					p = p + len;
					buf_len = buf_len - len - 1;
				}
			}
			CHECK_BUF(buf_len - 1);
			strcat(p, "@");
			p = p + 1;
			buf_len -= 1;
		}
	}
	if (url->host)
	{
		len = strlen(url->host);
		if (len > 0) {
			CHECK_BUF(buf_len - len);
			strcat(p, url->host);
			p = p + len;
			buf_len -= len;
			if (url->port > 0)
			{
				memset(tmp_buf, 0, 50);
				sprintf(tmp_buf, ":%d", url->port);
				len = strlen(tmp_buf);
				CHECK_BUF(buf_len - len);
				strcat(p, tmp_buf);
				p = p + len;
				buf_len -= len;
			}
		}

	}
	ptr = url->path;
	if (!ptr)
	{
		ptr = url_get_parameter(url, ORIENTSEC_GRPC_REGISTRY_KEY_INTERFACE, NULL);
	}
	len = strlen(ptr);
	if (ptr && len > 0)
	{
		CHECK_BUF(buf_len - len - 1);
		strcat(p, "/");
		p = p + 1;
		strcat(p, ptr);
		p = p + len;
		buf_len = buf_len - len - 1;
	}
	return build_paramters(p, buf_len, url, true);
}

char *url_to_string(url_t *url) {
	size_t buf_size = sizeof(char) * ORIENTSEC_GRPC_URL_MAX_LEN;
	char *buf = (char*)malloc(buf_size);
	memset(buf, 0, buf_size);
	if (!url_to_string_buf(url, buf, buf_size))
	{
		return buf;
	}
	return NULL;
}

char *url_encode(char *s) {
	size_t len = strlen(s);
	size_t buf_len = 3 * len + 1;
	char *buf = (unsigned char *)malloc(buf_len);
	if (url_encode_buf(s, buf, buf_len))
	{
		return NULL;
	}
	return buf;
}

int url_encode_buf(char *s, char *buf, int buf_len) {
	register unsigned char c;
	unsigned char *to, *start;
	unsigned char const *from, *end;

	size_t len = strlen(s);
	from = (unsigned char *)s;
	end = (unsigned char *)s + len;
	start = to = buf;
	if (!buf || buf_len < (3 * len + 1))
	{
		gpr_log(GPR_ERROR, "buf is invalid or buf length is too small");
		return -1;
	}

	while (from < end)
	{
		c = *from++;

		if (c == ' ')
		{
			*to++ = '+';
		}
		else if ((c < '0' && c != '-' && c != '.') ||
			(c < 'A' && c > '9') ||
			(c > 'Z' && c < 'a' && c != '_') ||
			(c > 'z'))
		{
			to[0] = '%';
			to[1] = hexchars[c >> 4];
			to[2] = hexchars[c & 15];
			to += 3;
		}
		else
		{
			*to++ = c;
		}
	}
	*to = 0;
	return 0;
}

int url_decode_buf(char *str, char *buf, int buf_len) {
	char *data = str;
	size_t len = strlen(str);
	char *dest = buf;
	char *p = dest;
	if (!buf || buf_len < len)
	{
		gpr_log(GPR_ERROR, "buf is invalid or buf length is too small");
		return -1;
	}
	while (len--)
	{
		if (*data == '+')
		{
			*p = ' ';
		}
		else if (*data == '%' && len >= 2 && isxdigit((int) *(data + 1)) && isxdigit((int) *(data + 2)))
		{
			*p = (char)url_htoi(data + 1);
			data += 2;
			len -= 2;
		}
		else
		{
			*p = *data;
		}
		data++;
		p++;
	}
	*p = '\0';
	return 0;
}

char *url_decode(char *str) {
	size_t len = strlen(str);
	size_t buf_len = sizeof(char) * len + 1;
	char *buf = (char *)malloc(buf_len);
	if (url_decode_buf(str, buf, buf_len))
	{
		return NULL;
	}
	return buf;
}

void
url_inspect(char *url) {
	url_data_inspect(url_parse(url));
}


void
url_data_inspect(url_t *data) {
	size_t i;
	printf("#url =>\n");
	printf("    .href: \"%s\"\n", data->href);
	printf("    .protocol: \"%s\"\n", data->protocol);
	printf("    .username: \"%s\"\n", data->username);
	printf("    .password: \"%s\"\n", data->password);
	printf("    .auth: \"%s\"\n", data->auth);
	printf("    .host: \"%s\"\n", data->host);
	printf("    .port: \"%d\"\n", data->port);
	printf("    .path: \"%s\"\n", data->path);
	printf("    .flag: \"%d\"\n", data->flag);
	for (i = 0; i < data->params_num; i++)
	{
		printf("    .params:  \"%s\" = \"%s\" \n", data->parameters[i].key,
			data->parameters[i].value);
	}

}

void url_free(url_t *data) {
	size_t i;
	if (!data) return;
	if (data->href)
	{
		free(data->href);
	}
	if (data->protocol) free(data->protocol);

	if (data->username) free(data->username);
	if (data->password) free(data->password);
	if (data->auth) free(data->auth);

	if (data->host) free(data->host);

	if (data->path) free(data->path);
	for (i = 0; i < data->params_num; i++)
	{
		if (data->parameters[i].key)
			free(data->parameters[i].key);

		if (data->parameters[i].value)
			free(data->parameters[i].value);
	}
	free(data->parameters);
}

void url_full_free(url_t **data) {
	url_t *url = *data;
	url_free(url);
	free(url);
}

//p2比较器，p1被比较对象
bool isMatch(url_t *p1, url_t *p2) {
	char *intf1 = NULL, *intf2 = NULL;
	char *version1 = NULL, *version2 = NULL;
	char *group1 = NULL, *group2 = NULL;
	if (!p1 || !p2)
	{
		return false;
	}
	intf1 = url_get_parameter_v2(p1, ORIENTSEC_GRPC_REGISTRY_KEY_INTERFACE, NULL);
	intf2 = url_get_parameter_v2(p2, ORIENTSEC_GRPC_REGISTRY_KEY_INTERFACE, NULL);

	if ((!intf1 && intf2) ||
		(intf1 && intf2 && (0 != strcmp(intf1, intf2))))
	{
		return false;
	}

	version1 = url_get_parameter_v2(p1, ORIENTSEC_GRPC_REGISTRY_KEY_VERSION, NULL);
	version2 = url_get_parameter_v2(p2, ORIENTSEC_GRPC_REGISTRY_KEY_VERSION, NULL);

	if ((!version1 && version2) ||
		(version1 && version2 && (0 != strcmp(version1, version2))))
	{
		return false;
	}

	group1 = url_get_parameter_v2(p1, ORIENTSEC_GRPC_REGISTRY_KEY_GROUP, NULL);
	group2 = url_get_parameter_v2(p2, ORIENTSEC_GRPC_REGISTRY_KEY_GROUP, NULL);
	if ((!group1 && group2) ||
		(group1 && group2 && (0 != strcmp(group1, group2))))
	{
		return false;
	}
	return true;
}

int filterUrls(url_t *src, int src_num, url_t *filter) {
	int nums = 0;
	size_t i = 0;
	for (i = 0; i < src_num; i++)
	{
		if (isMatch(src + i, filter))
		{
			ORIENTSEC_GRPC_SET_BIT(src[i].flag, ORIENTSEC_GRPC_URL_MATCH_POS);
			nums++;
		}
	}
	return nums;
}
