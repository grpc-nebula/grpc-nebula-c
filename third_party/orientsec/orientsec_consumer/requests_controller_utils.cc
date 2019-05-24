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

#include "requests_controller_utils.h"
#include "grpc/impl/codegen/atm.h"
#include "orientsec_grpc_utils.h"

void requests_controller_utils::SetMaxRequestMap(std::string service_name, long maxRequest) {
	if (service_name.empty())
	{
		return;
	}
	std::string sn_localip_key = service_name + std::string(get_local_ip());
	std::map<std::string, long>::iterator max_request_iter = max_request_vec_.find(sn_localip_key);
	if (max_request_iter != max_request_vec_.end())
	{
		if (maxRequest == 0)
		{
			max_request_vec_.erase(max_request_iter);
		}
		else {
			gpr_atm_rel_store((gpr_atm*)&max_request_iter->second, maxRequest);
		}
	}
	else {
		if (maxRequest > 0)
		{
			max_request_vec_.insert(std::pair<std::string, long>(sn_localip_key, maxRequest));
		}
	}

	max_request_iter = max_request_vec_.find(service_name);
	if (max_request_iter != max_request_vec_.end())
	{
		if (maxRequest == 0)
		{
			max_request_vec_.erase(max_request_iter);
		}
		else {
			gpr_atm_rel_store((gpr_atm*)&max_request_iter->second, maxRequest);
		}
	}
	else {
		if (maxRequest > 0)
		{
			max_request_vec_.insert(std::pair<std::string, long>(service_name, maxRequest));
		}
	}

}

bool requests_controller_utils::CheckRequestInner(std::map<std::string, long>::iterator max_request_iter,
	std::string service_name, uint64_t current_time) {
	if (max_request_iter != max_request_vec_.end())
	{
		long max_request = gpr_atm_acq_load((gpr_atm*)&max_request_iter->second);
		if (max_request <= 0)
		{
			return true;;
		}
		std::map<std::string, long>::iterator request_num_iter = request_num_counters_vec_.find(service_name);
		long current_request_num = 0;
		if (request_num_iter != request_num_counters_vec_.end())
		{
			current_request_num = request_num_iter->second;
			uint64_t last_time = 0;
			std::map<std::string, uint64_t>::iterator last_invoke_time_iter = last_invoke_time_vec_.find(service_name);
			if (last_invoke_time_iter != last_invoke_time_vec_.end())
			{
				last_time = last_invoke_time_iter->second;
			}
			if (current_time - last_time > 1000)//已经 超过1s，重置计数
			{
				if (last_invoke_time_iter != last_invoke_time_vec_.end())
				{
					last_invoke_time_vec_.erase(last_invoke_time_iter);
				}
				last_invoke_time_vec_.insert(std::pair<std::string, uint64_t>(service_name, current_time));
				gpr_atm_rel_store((gpr_atm*)&request_num_iter->second, 0);
			}
		}
		else {
			request_num_counters_vec_.insert(std::pair<std::string, uint64_t>(service_name, 0));
			request_num_iter = request_num_counters_vec_.find(service_name);
		}
		if (gpr_atm_acq_load((gpr_atm*)&request_num_iter->second) <
			gpr_atm_acq_load((gpr_atm*)&max_request_iter->second))
		{
			gpr_atm_full_fetch_add((gpr_atm*)&request_num_iter->second, 1);
			return true;
		}
		else {
			return false;
		}

	}
	return true;
}

bool requests_controller_utils::CheckRequest(std::string service_name) {
	if (service_name.empty())
	{
		return true;
	}
	std::string sn_localip_key = service_name + std::string(get_local_ip());
	uint64_t current_time = orientsec_get_timestamp_in_mills();
	////首先检查是否检查确定IP的流量控制
	std::map<std::string, long>::iterator max_request_iter = max_request_vec_.find(sn_localip_key);
	if (max_request_iter != max_request_vec_.end())
	{
		return CheckRequestInner(max_request_iter, service_name, current_time);
	}

	//检查有0.0.0.0通用IP指定的规则
	max_request_iter = max_request_vec_.find(service_name);
	if (max_request_iter != max_request_vec_.end())
	{
		return CheckRequestInner(max_request_iter, service_name, current_time);
	}
	return true;
}

void requests_controller_utils::DecreaseRequest(std::string service_name) {
	if (service_name.empty())
	{
		return;
	}
	std::map<std::string, long>::iterator request_num_iter = request_num_counters_vec_.find(service_name);
	if (request_num_iter != request_num_counters_vec_.end())
	{
		gpr_atm_full_fetch_add((gpr_atm*)&request_num_iter->second, -1);
		if (gpr_atm_acq_load((gpr_atm*)&request_num_iter->second) < 0)
		{
			gpr_atm_rel_store((gpr_atm*)&request_num_iter->second, 0);
		}
	}
}
