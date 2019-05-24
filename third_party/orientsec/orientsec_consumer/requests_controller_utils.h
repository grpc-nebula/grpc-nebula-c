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

#ifndef ORIENTSEC_REQUESTS_CONTROLLER_UTILS_H
#define ORIENTSEC_REQUESTS_CONTROLLER_UTILS_H

#include <stdint.h>
#include<map>
#include<string>

#ifdef __cplusplus
extern "C" {
#endif

	class requests_controller_utils
	{

	public:
		requests_controller_utils() {};
		~requests_controller_utils() {};
		void SetMaxRequestMap(std::string service_name, long maxRequest);
		//检查某服务的单位时间内的请求数是否小于最大请求数，满足条件返回true，否则返回false
		bool CheckRequest(std::string service_name);
		void DecreaseRequest(std::string service_name);

	private:
		bool CheckRequestInner(std::map<std::string, long>::iterator max_request_iter, std::string service_name, uint64_t current_time);
	private:
		std::map<std::string, long> max_request_vec_;
		std::map<std::string, uint64_t> last_invoke_time_vec_;
		std::map<std::string, long> request_num_counters_vec_;
	};

#ifdef __cplusplus
}
#endif

#endif


