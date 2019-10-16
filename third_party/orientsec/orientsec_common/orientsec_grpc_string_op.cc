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

#include "orientsec_grpc_string_op.h"

//删除空格
std::string& orientsec_grpc_trim(std::string& s) {
  if (s.empty()) {
    return s;
  }
  // remove blank
  s.erase(0, s.find_first_not_of(" "));
  s.erase(s.find_last_not_of(" ") + 1);

  // remove quotation
  s.erase(0, s.find_first_not_of("\""));
  s.erase(s.find_last_not_of("\"") + 1);
  return s;
}

bool orientsec_grpc_split_to_vec(const std::string& str,
                                 std::vector<std::string>& ret_,
                                 std::string sep) {
  if (str.empty()) {
    return true;
  }

  std::string tmp;
  std::string::size_type pos_begin = str.find_first_not_of(sep);
  std::string::size_type comma_pos = 0;

  while (pos_begin != std::string::npos) {
    comma_pos = str.find(sep, pos_begin);
    if (comma_pos != std::string::npos) {
      tmp = str.substr(pos_begin, comma_pos - pos_begin);
      pos_begin = comma_pos + sep.length();
    } else {
      tmp = str.substr(pos_begin);
      pos_begin = comma_pos;
    }

    if (!tmp.empty()) {
      ret_.push_back(tmp);
      tmp.clear();
    }
  }
  return true;
}

bool orientsec_grpc_split_to_map(const std::string& str,
                                 std::map<std::string, std::string>& ret_,
                                 std::string sep) {
  if (str.empty()) {
    return true;
  }

  std::string tmp;
  std::string::size_type pos_begin = str.find_first_not_of(sep);
  std::string::size_type comma_pos = 0;
  std::vector<std::string> vec_tmp;
  std::string key;
  std::string value;
  std::string::size_type idx;

  // 循环分割字符串
  while (pos_begin != std::string::npos) {
    comma_pos = str.find(sep, pos_begin);
    if (comma_pos != std::string::npos) {
      tmp = str.substr(pos_begin, comma_pos - pos_begin);
      pos_begin = comma_pos + sep.length();
    } else {
      tmp = str.substr(pos_begin);
      pos_begin = comma_pos;
    }

    if (!tmp.empty()) {
      idx = tmp.find(":");
      if (idx != std::string::npos) {
      
        if (orientsec_grpc_split_to_vec(tmp, vec_tmp, ":")) {
          key = orientsec_grpc_trim(vec_tmp[0]);
          value = orientsec_grpc_trim(vec_tmp[1]);
          ret_.insert(std::pair<std::string, std::string>(key, value));
          vec_tmp.clear();
        }
      }
      tmp.clear();
    }
  }
  return true;
}

bool orientsec_grpc_joint_hash_input(std::map<std::string, std::string> map_,
                                     std::vector<std::string>& vec_,
                                     std::string& hash_arg) {
  if (vec_.empty()) {
    // 哈希参数未配置参数值取第一个参数的参数值返回
    hash_arg = map_.begin()->second;
    return true;
  }
  //根据vector里面的字段 读取map中相应的数据拼接
  for (std::vector<std::string>::iterator vec_iter = vec_.begin();
       vec_iter != vec_.end(); vec_iter++) {
    std::map<std::string, std::string>::iterator map_iter =
        map_.find((*vec_iter).c_str());
    if (map_iter != map_.end())
      // joint into hash_arg
      hash_arg += map_iter->second;
  }
  // 参数值列表不正确，harh_arg为空，则取第一个参数的参数值
  if (hash_arg.empty() || (hash_arg.size() == 0)) 
    hash_arg = map_.begin()->second;
  return true;
}
