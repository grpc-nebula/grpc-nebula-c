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
 *    2017/07/19
 *    version 0.0.9
 *    黑白名单处理类声明
 */
#ifndef FAILOVER_UTILS_H
#define FAILOVER_UTILS_H
#include "grpc/impl/codegen/atm.h"
#include<map>
#include<vector>
#include<cstring>

#ifdef __cplusplus
extern "C" {
#endif

class failover_utils
{
private:
	//连续多少次请求出错，自动切换到提供相同服务的新服务器
	int m_switch_threshold;

	//服务端节点调用失败被移除请求列表后，经过多长时间将该服务端节点重新添加回服务端候选列表
	//单位为毫秒,缺省值为600000ms，10mins
	int m_recovery_time;

        //zookeeper算法里backoff算法里max backoff参数取值
        int max_backoff_time;

	//调用失败的客户端对应服务提供者列表,key值为：consumerId 
	std::map<std::string, std::vector<std::string> > m_failing_providers;

	//各个客户端最后一个服务提供者的删除时间,key值为：consumerId 
	std::map<std::string, uint64_t> m_remove_provider_timestamp;

	//各个客户端对应服务提供者最后一次服务调用失败时间,key值为：consumerId@IP:port
	std::map<std::string, uint64_t> m_last_failing_timestamp;

	//各个客户端对应服务提供者服务调用失败次数,key值为：consumerId@IP:port
	std::map<std::string, gpr_atm> m_request_failures;
public:
	failover_utils();
	~failover_utils();
        void recover_provider_list(const std::string& consumerid,
                                   const std::string& providerid);
	void set_switch_threshold(int _switch_threshold);
        void set_recovery_time(int _recoverytime);
        void set_max_backoff_time(int _max_backoff_time);
        int get_max_backoff_time();
        void record_provider_failure(char* consumerid, char* providerid,const char* methods);
        void reset_provider_failure(char* consumerid, char* providerid,
                                    const char* methods);
	void update_failing_providers(char* consumerid, char* providerid);
        int update_fail_times(char* consumerid, char* providerid,
                              int64_t lasttime_stamp, int64_t currenttime_stamp,
                              const char* methods);

};

#ifdef __cplusplus
}
#endif

#endif // !ORIENTSEC_FAILOVERUTILS_H




