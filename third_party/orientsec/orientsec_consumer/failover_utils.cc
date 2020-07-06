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

#include <thread>
#include <chrono>
#include "failover_utils.h"
#include "orientsec_consumer_intf.h"
#include "orientsec_grpc_utils.h"
#include "url.h"

#define CONSUMERID_PROVIDERID_SEPARATOR "@"
#define TEN_MINUTES_IN_MILLIS 600000

failover_utils::failover_utils() {
  // initialization
  m_switch_threshold = 5;
  m_recovery_time = TEN_MINUTES_IN_MILLIS;
}

failover_utils::~failover_utils() {}

void failover_utils::set_switch_threshold(int _switch_threshold) {
  m_switch_threshold = _switch_threshold;
}

void failover_utils::set_recovery_time(int _recoverytime) {
  m_recovery_time = _recoverytime;
}
void failover_utils::set_max_backoff_time(int _max_backoff_time) {
  max_backoff_time = _max_backoff_time;
}
int failover_utils::get_max_backoff_time() { return max_backoff_time; }

// new thread for clearing failover flag with copied parameter
void failover_utils::recover_provider_list(const std::string& consumerid,
                                           const std::string& providerid) {
  url_t* consumer_url = url_parse((char*)consumerid.c_str());
  gpr_log(GPR_ERROR,
          "recover_provider_list thread[%u] sleep %d ms,wait for recovering "
          "provider list...",
          std::this_thread::get_id(), m_recovery_time);
  // thread sleep for a while
  std::this_thread::sleep_for(std::chrono::milliseconds(m_recovery_time));

  clr_provider_failover_flag(consumer_url->path, (char*)providerid.c_str());
  gpr_log(GPR_ERROR,
          "recover_provider_list thread[%u] has recovered provider list and "
          "exit...",
          std::this_thread::get_id());
  url_free(consumer_url);
  FREE_PTR(consumer_url);
}

// client需要用长连接
void failover_utils::record_provider_failure(char* consumerid, char* providerid,
                                             const char* methods) {
  if (!consumerid || strlen(providerid) == 0 || providerid == NULL) {
    return;
  }
  // format provider ID(ip:port)
  std::string provider = providerid;
  char* pi = new char[32];
  size_t pos = provider.find_first_of(':');
  if (pos == std::string::npos) {
    gpr_log(GPR_ERROR,
            "invalid providerId,[%s],should split by : ", providerid);
    return;
  }
  std::string provider_host = provider.substr(pos + 1);
  strcpy(pi, provider_host.c_str());

  std::string key =
      std::string(consumerid) + CONSUMERID_PROVIDERID_SEPARATOR + pi;
  uint64_t current_timestamp = orientsec_get_timestamp_in_mills();
  uint64_t last_timestamp = 0;  //上次更新时间
  std::map<std::string, uint64_t>::iterator last_failing_timeiter =
      m_last_failing_timestamp.find(key.c_str());
  //查询上次更新时间
  if (last_failing_timeiter != m_last_failing_timestamp.end()) {
    last_timestamp = last_failing_timeiter->second;
  } else {
    last_timestamp = current_timestamp;
    m_last_failing_timestamp[key] =
        current_timestamp;  // 初始化为0时，首次满足容错次数立马更新
  }
  //如果已经更新,记录更新时间
  if (update_fail_times(consumerid, pi, last_timestamp, current_timestamp,
                        methods) == 1) {
    m_last_failing_timestamp[key] = current_timestamp;
  }

  delete(pi);
}

// reset provider failure if invoking successfully once
void failover_utils::reset_provider_failure(char* consumerid, char* providerid,
                                            const char* methods) {
  if (!consumerid || strlen(providerid) == 0 || providerid == NULL) {
    return;
  }
  // format provider id(ip:port)
  std::string provider = providerid;
  char* pi = new char[32];
  size_t pos = provider.find_first_of(':');
  if (pos == std::string::npos) {
    gpr_log(GPR_ERROR,
            "invalid providerId,[%s],should split by : ", providerid);
    return;
  }
  std::string provider_host = provider.substr(pos + 1);
  strcpy(pi, provider_host.c_str());

  std::string key =
      std::string(consumerid) + CONSUMERID_PROVIDERID_SEPARATOR + providerid;
  auto request_failure_iter = m_request_failures.find(key);
  if (request_failure_iter != m_request_failures.end()) {
    if (0 != request_failure_iter->second) {
      gpr_atm_rel_store(&request_failure_iter->second,
                        0);  // reset calling failed times
      gpr_log(GPR_ERROR, "request_failure_reset = %d",
              request_failure_iter->second);
    }
  }
  // release memory
  free(pi);
}

//目前不需要记录错误关系
void failover_utils::update_failing_providers(char* consumerid,
                                              char* providerid) {}
// 记录更新时间
int failover_utils::update_fail_times(char* consumerid, char* providerid,
                                      int64_t lasttime_stamp,
                                      int64_t current_timestamp,
                                      const char* methods) {
  int result = 0;
  if (!consumerid || !providerid || strlen(providerid) == 0) {
    return result;
  }
  std::string key =
      std::string(consumerid) + CONSUMERID_PROVIDERID_SEPARATOR + providerid;
  std::map<std::string, gpr_atm>::iterator request_failure_iter =
      m_request_failures.find(key);
  if (request_failure_iter == m_request_failures.end()) {
    m_request_failures.insert(std::pair<std::string, int>(key, 0));
    request_failure_iter = m_request_failures.find(key);
  }
  // 为了提高性能，不对调用成功的情况进行记录，使用以下策略近似判断连续多次调用失败：
  // 将当前时间和最后一次出错时间的记录做比较，如果时间间隔大于600秒，将之前的错误次数清0
  if (current_timestamp - lasttime_stamp > m_recovery_time) {
    gpr_atm_rel_store(&request_failure_iter->second, 0);
  }

  //出错次数+1
  gpr_atm_full_fetch_add(&request_failure_iter->second, 1);
  url_t* consumer_url = url_parse(consumerid);
  if (!consumer_url) {
    return result;
  }

  //可用provider
  int consumer_lb_provider_count =
      get_valid_providers_acount(consumer_url->path, methods);
  //出错次数大于允许次数
  if (gpr_atm_acq_load(&request_failure_iter->second) > m_switch_threshold) {
    //只存在一个可用provider，并且调用还失败了
    if (consumer_lb_provider_count == 1) {
      //需要进行惩罚
      if (m_recovery_time > 0) {
        set_provider_failover_flag(consumer_url->path, providerid);
        std::thread reco(&failover_utils::recover_provider_list,this,
                         std::string(consumerid),
                         std::string(providerid)); 
        reco.detach();
        if (request_failure_iter->second == m_switch_threshold) {
          gpr_log(GPR_ERROR,
                  "FATAL ERROR : 服务器节点[%s]连续调用出错[%d]"
                  "次,从客户端备选服务器列表中删除.",
                  providerid, request_failure_iter->second);
        }
      }
    } else if (consumer_lb_provider_count >
               1) {  //存在多个provider的情况，可以选择其他provider
      set_provider_failover_flag(consumer_url->path, providerid);
      std::thread recov(&failover_utils::recover_provider_list, this,
                       std::string(consumerid),
                       std::string(providerid));
      recov.detach();
      if (request_failure_iter->second == m_switch_threshold) {
        gpr_log(
            GPR_ERROR,
            "FATAL ERROR : "
            "服务器节点{%s}连续调用出错{%d}次，从客户端备选服务器列表中删除.",
            providerid, request_failure_iter->second);
      }
        
    }
    gpr_atm_rel_store(&request_failure_iter->second, 0);
  }
  gpr_log(GPR_ERROR, "Failed times = %Id,valid provider num= %d\n",
         request_failure_iter->second, consumer_lb_provider_count);

  //获取可用provider数量
  consumer_lb_provider_count =
      get_valid_providers_acount(consumer_url->path, methods);

  //无可用provider时更新provider列表
  if (consumer_lb_provider_count == 0) {
    //出错次数清理
    gpr_atm_rel_store(&request_failure_iter->second, 0);

    // 重新查询一遍服务提供者，将注册中心上的服务列表写入当前消费者的服务列表
    get_all_providers_by_name(consumer_url->path);
    result = 1;
  }
  url_free(consumer_url);
  FREE_PTR(consumer_url);
  return result;
}
