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
 *    Author : Jianbin Yang(yangjianbin@beyondcent.com)
 *    2019/02/27
 *    version 0.9
 *    C++ string operation
 */


#ifndef ORIENTSEC_GRPC_STRING_OP_H
#define ORIENTSEC_GRPC_STRING_OP_H

#include <string>
#include <iostream>
#include <vector>
#include <map>
#include "orientsec_grpc_properties_tools.h"
#include "orientsec_grpc_properties_constants.h"


#ifdef __cplusplus
extern "C" {
#endif

 //----begin----pre declaration
 std::string& orientsec_grpc_trim(std::string& s);
 bool orientsec_grpc_split_to_vec(const std::string& str,
                           std::vector<std::string>& ret_, std::string sep);
 bool orientsec_grpc_split_to_map(const std::string& str,
                           std::map<std::string, std::string>& ret_,
                           std::string sep);
 bool orientsec_grpc_joint_hash_input(std::map<std::string, std::string> map_,
                               std::vector<std::string>& vec_,
                               std::string& hash_arg);

 bool orientsec_grpc_split_to_map_by_equal(
     const std::string& str, std::map<std::string, std::string>& ret_,
     std::string sep);
 //----end----



#ifdef __cplusplus
}
#endif 
  
#endif
