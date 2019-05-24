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

#include "orientsec_grpc_consumer_control_requests.h"
#include <map>
#include <string>
#include <vector>
#include "grpc/support/alloc.h"
#include "grpc/support/sync.h"
#include "grpc/support/log.h"
#include "src/core/lib/gpr/spinlock.h"
#include "orientsec_grpc_utils.h"
#include "orientsec_grpc_common_utils.h"

//请求数更新锁
static bool orientsec_grpc_consumer_request_lock_isinit = false;
static gpr_mu g_consumer_requests_mu;

//记录指定服务的请求数
static std::map<std::string, orientsec_grpc_consumer_request_t*> g_consumer_requests;

//存储哪些服务需要进行最大请求数控制
static std::map<std::string,long> g_consumer_maxrequests;

//最大请求数控制锁初始化
void orientsec_grpc_consumer_request_mu_init() {
	if (!orientsec_grpc_consumer_request_lock_isinit) {
		orientsec_grpc_consumer_request_lock_isinit = true;
		//此处可以模拟测试。。
		gpr_mu_init(&g_consumer_requests_mu);
	}
}

//调用本方法更新最大请求数
void orientsec_grpc_consumer_update_maxrequest(char * servicename, long requestnum) {
	if (requestnum >= 0) {
		g_consumer_maxrequests.insert(std::pair<std::string, long >(servicename, requestnum));
	}
}

//校验制定服务是否需要进行请求数控制
int orientsec_grpc_consumer_control_requests(const char * fullmethod) {
	//根据fullname提取服务名
	char * servicename = NULL;
    orientsec_grpc_getserveice_by_fullmethod(fullmethod, &servicename);
	orientsec_grpc_consumer_request_mu_init();

	//根据fullname获取服务名
	std::map<std::string, long>::iterator maxrequest = g_consumer_maxrequests.find(servicename);
	long requestnum = 0;
	if (maxrequest == g_consumer_maxrequests.end()) {
		free(servicename);
		servicename = NULL;
		return 0;
	}
	else { 
		//回去允许最大请求数
		requestnum = maxrequest->second;
		if (requestnum<=0) {
			free(servicename);
			servicename = NULL;
			return 0;
		}
	}

	uint64_t currentime = orientsec_get_timestamp_in_mills();
	orientsec_grpc_consumer_request_t* requestinfo = (orientsec_grpc_consumer_request_t*)gpr_malloc(sizeof(orientsec_grpc_consumer_request_t));

	//此处开始加锁
	gpr_mu_lock(&g_consumer_requests_mu);
	std::map<std::string, orientsec_grpc_consumer_request_t*>::iterator request = g_consumer_requests.find(servicename);
	if (request != g_consumer_requests.end()) {
		long currentrequests = request->second->currentrequests;
		uint64_t timestamp = request->second->timestamp;		
		//重新开始计数（计数周期进入下一秒，重新开始计数）
		if ((currentime-timestamp) > 1000) {
			request->second->currentrequests = 0;
			request->second->timestamp = currentime;
        }
		else {  //在同一周期内做数量加一操作。
			++currentrequests;
			//requestnum > 0 表示需要进行请求数控制， currentrequests > requestnum 表示当前请求数大于允许请求数
			if (currentrequests > requestnum && requestnum > 0) {
				free(servicename);
				servicename = NULL;
                gpr_mu_unlock(&g_consumer_requests_mu);
				return -1;
			}
			request->second->currentrequests = currentrequests;
		}	   
	}
	else {
		requestinfo->currentrequests = 0;
		requestinfo->timestamp = currentime;
		g_consumer_requests.insert(std::pair<std::string, orientsec_grpc_consumer_request_t* >(servicename, requestinfo));
	} 
	free(servicename);
	servicename = NULL;
	gpr_mu_unlock(&g_consumer_requests_mu);
	return 0;
	//此处进行解锁
}
