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
#include "orientsec_grpc_consumer_control_group.h"
#include "orientsec_grpc_properties_tools.h"
#include "orientsec_grpc_conf.h"
#include "orientsec_grpc_common_init.h"
//#include "orientsec_grpc_string_op.h"
#include <cstring>
#include <vector>


// for initialztion once
static bool grpc_service_group_isinit = false;

#define ORIENTSEC_GRPC_GROUP_SEPARATOR_STR ";"

#define ORIENTSEC_GRPC_GRADE_SEPARATOR_STR ","

//�û������ķ���Լ�������Ϊ��ʱ��˵������Ҫ���������
//static std::vector < std::string > g_consumer_service_group;

void orientsec_grpc_consumer_control_group_update(char* service_name,
                                                  char* group_info) {}
//
//bool orientsec_grpc_consumer_control_group_match(provider_t* pro,
//                                                 char* group_info) {
//  if (servername == NULL) {
//    return false;
//  }
//  // ��ƥ�����ֱ�ӷ���
//  if (g_consumer_service_version.size() == 0) {
//    return true;
//  }
//  
//}

// �浽һ��vector���棬����vector��ά���жϴ���������������ʼ��
// ��ȡ�����ַ����浽 vector��

//std::vector<std::string> orientsec_grpc_global_service_group_get() {
//  return g_consumer_service_group;
//}

//bool is_group_grading() {
//  char* group_info = orientsec_grpc_consumer_service_group_get(); 
//  if (strlen(group_info) == 0)
//     return false; 
//  return true;
//}

//void orientsec_grpc_service_group_init() {
//  //��ִֻ֤��һ�γ�ʼ��
//  if (grpc_service_group_isinit) {
//    return;
//  }
//
//  //�����ַ�������map����
//  char* group_conf = orientsec_grpc_consumer_service_group_get();
//  if (group_conf == NULL || strlen(group_conf) == 0) {
//    return;
//  }
//
//  orientsec_grpc_split_to_vec(std::string(group_conf), g_consumer_service_group,
//                              ORIENTSEC_GRPC_GROUP_SEPARATOR_STR);
//  std::vector<std::string> service_group;
//  if (g_consumer_service_group.size() == 0) {
//    return;
//  }
//  
//  grpc_service_group_isinit = true;
//  // add debug log
//
//  for (int i = 0; i < g_consumer_service_group.size(); i++) {
//    std::cout << "init g_consumer_service_group[" << i
//              << "]:" << g_consumer_service_group[i]
//              << std::endl;
//
//  }
//
//}
