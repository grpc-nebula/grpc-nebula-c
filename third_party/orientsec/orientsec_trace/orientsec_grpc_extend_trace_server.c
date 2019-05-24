/*
*
* Copyright 2015, Google Inc.
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are
* met:
*
*     * Redistributions of source code must retain the above copyright
* notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above
* copyright notice, this list of conditions and the following disclaimer
* in the documentation and/or other materials provided with the
* distribution.
*     * Neither the name of Google Inc. nor the names of its
* contributors may be used to endorse or promote products derived from
* this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
* A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
* OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/
#include <stdio.h> 
#include "dfzq_grpc_utils.h"
#include "dfzq_grpc_trace.h"
#include "dfzq_grpc_common_trace_key.h"
#include "dfzq_grpc_utils.h"
#include "dfzq_grpc_common_utils.h"
#include "dfzq_grpc_trace_for_cc.h"
#include "dfzq_grpc_extend_trace.h"
#include "dfzq_grpc_extend_trace_server.h"

/*
*func:推送服务跟踪信息到缓存对象
*auth:huyn
*date:20170829
*/
void dfzq_grpc_provider_push_send(dfzq_grpc_common_traceinfo_t *traceinfo_push,
	long push_times_period, uint64_t current_time, bool is_initial) {
	traceinfo_push->consumerside = false;

	traceinfo_push->initial = is_initial;
	traceinfo_push->starttime = current_time;
	traceinfo_push->endtime = current_time;
	traceinfo_push->pushtime = push_times_period;
	dfzq_grpc_trace_write(traceinfo_push);
}

//provider 服务调用返回处理
void dfzq_grpc_provider_finish(dfzq_grpc_common_traceinfo_t *traceinfo, bool issuccess) {
	//把服务跟踪信息写入队列
	traceinfo->success = issuccess;

	//把服务跟踪信息写入链表
	traceinfo->endtime = dfzq_get_timestamp_in_mills();
	dfzq_grpc_trace_write(traceinfo);

	//清除threadlocal变量
	//非首节点在server端清除,同时清除下级C节点的threadlocal对象		
	dfzq_grpc_trace_threadlocal_clear();
	traceinfo = NULL;
}


