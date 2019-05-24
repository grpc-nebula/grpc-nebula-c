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

#include "orientsec_grpc_consumer_control_version.h"
#include "orientsec_grpc_common_init.h"
#include <string>
#include <map>
#include <vector>
#include <iostream>
 
//用户存服务的版本约束，如果为空时，说明不需要做版本控制
static std::map<std::string, std::string> g_consumer_service_version;

static bool grpc_service_version_isinit = false; // 只做一次初始化

static bool grpc_version_change_when_conn = false;

//字符串拆分
std::vector<std::string> orientsec_grpc_common_split(const std::string &s, const std::string &seperator) {
  std::vector<std::string> result;
  typedef std::string::size_type string_size;
  string_size i = 0;

  while (i != s.size()) {
    //找到字符串中首个不等于分隔符的字母
    int flag = 0;
    while (i != s.size() && flag == 0) {
      flag = 1;
      for (string_size x = 0; x < seperator.size(); ++x)
	if (s[i] == seperator[x]) {
	  ++i;
	  flag = 0;
	  break;
	}
    }

    //找到又一个分隔符，将两个分隔符之间的字符串取出
    flag = 0;
    string_size j = i;
    while (j != s.size() && flag == 0) {
      for (string_size x = 0; x < seperator.size(); ++x)
	if (s[j] == seperator[x]) {
	  flag = 1;
	  break;
	}
      if (flag == 0)
        ++j;
    }
    if (i != j) {
      result.push_back(s.substr(i, j - i));
      i = j;
    }
  }
  return result;
}

//解析配置字符串，初始化map对象
//如果当前应用只调用一个服务，属性值直接配置版本号，例如1.0.0
//如果当前应用需要调用多个服务，属性值按照冒号逗号的方式分隔，
//例如com.orientsec.examples.Greeter:1.0.0,com.orientsec.examples.Hello:1.2.1
void orientsec_grpc_service_version_init() {
  //保证只执行一次初始化
  if (grpc_service_version_isinit) {
    return;
  }

  //解析字符串放入map对象
  char * service_conf = orientsec_grpc_consumer_service_version_get();
  if (service_conf == NULL || strlen(service_conf)==0) {
    return;
  }
  char * indexchar = strchr(service_conf, ORIENTSEC_GRPC_VERSION_SEPARATOR_CHAR);
  //不包含":",表示所有服务都是同一版本。
  if ((indexchar - service_conf) <= 0) { 
    g_consumer_service_version.insert(std::pair<std::string, std::string>(ORIENTSEC_GRPC_SERVICE_VERSION_COMMON_KEY, service_conf));
    return;
  }

  std::vector<std::string> services = orientsec_grpc_common_split(std::string(service_conf), ",");
  std::vector<std::string> service_version;
  if (services.size() == 0) {
    return;
  }    
  for (int i = 0;i < services.size();i++) {
    if (services[i].empty()) {
      continue;
    }
    service_version = orientsec_grpc_common_split(std::string(services[i]), ORIENTSEC_GRPC_VERSION_SEPARATOR_STR);
    if (service_version.size() != 2) {
      continue;
    }
    g_consumer_service_version.insert(std::pair<std::string, std::string>(service_version[0], service_version[1]));
  }
  grpc_service_version_isinit = true;

    for (std::map<std::string, std::string>::iterator ii =
           g_consumer_service_version.begin();
       ii != g_consumer_service_version.end(); ii++)
    std::cout << "init g_consumer_service_version[" << ii->first
              << "]:" << ii->second << std::endl;
}

//获取版本控制信息
std::map<std::string, std::string> orientsec_grpc_consumer_control_version_get() {
  return g_consumer_service_version;
}

// 更新版本控制信息
void orientsec_grpc_consumer_control_version_update(char *service_name,char *service_version) {
  orientsec_grpc_service_version_init();
  std::map<std::string, std::string >::iterator version_iter = g_consumer_service_version.find(service_name);
  if (version_iter != g_consumer_service_version.end()) {
    g_consumer_service_version.erase(version_iter);
    g_consumer_service_version.insert(std::pair<std::string, std::string>(service_name, service_version));
    //version_iter->second = service_version;
  }
  else {
    g_consumer_service_version.insert(std::pair<std::string, std::string>(service_name, service_version));
  }
  if (!is_request_loadbalance())
    grpc_version_change_when_conn = true;

  for (std::map<std::string, std::string>::iterator ii =
           g_consumer_service_version.begin();
       ii != g_consumer_service_version.end(); ii++)
    std::cout << "g_consumer_service_version[" << ii->first << "]:" << ii->second
              << std::endl;
}

//校验服务版本是否变动
bool orientsec_grpc_version_changed_conn() { return grpc_version_change_when_conn; }
//重置服务版本是否变动标记
void orientsec_reset_grpc_version_changed_conn() {
  grpc_version_change_when_conn = false;
}

//校验服务版本是否匹配
bool orientsec_grpc_consumer_control_version_match(const char *servername,char* version) {
  if (servername == NULL) {
    return false;
  }
  // 无匹配规则，直接返回
  if (g_consumer_service_version.size() == 0) {
    return true;
  }

  std::map<std::string, std::string >::iterator version_iter = g_consumer_service_version.find(servername);

  // 不做版本控制
  if (version_iter == g_consumer_service_version.end() || version_iter->second.empty()) {
    // 查询是否只配置版本号，未指定服务
    version_iter = g_consumer_service_version.find(ORIENTSEC_GRPC_SERVICE_VERSION_COMMON_KEY);
    if (version_iter == g_consumer_service_version.end() || version_iter->second.empty()) {
      return true;
    }
  }
  // consumer指定版本，provider未指定版本
  if (version == NULL || strlen(version) == 0) {
    return false;
  }
  // 匹配版本一致
  if (strcmp(version_iter->second.data(), version) == 0) {
    return true;
  }
  else {
    return false;
  }
}
