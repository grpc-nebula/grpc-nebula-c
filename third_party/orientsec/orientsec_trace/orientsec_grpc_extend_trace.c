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

#include "dfzq_grpc_extend_trace.h"
#include "string.h"
#include <stdio.h> 
#include <grpc/support/thd.h>
#include <grpc/support/port_platform.h>

#include "dfzq_grpc_trace.h"
#include "dfzq_grpc_common_trace_key.h"
#include "dfzq_grpc_trace_for_cc.h"
#include "dfzq_grpc_conf.h"
#include "dfzq_grpc_utils.h"
#include "dfzq_grpc_properties_tools.h"
#include "dfzq_grpc_common_utils.h"

/*
*调用发送接口队列写入接口，把跟踪信息写入
*
*/
void dfzq_grpc_write_trace_to_array(dfzq_grpc_common_traceinfo_t *traceinfo, long pushtimes, uint64_t currentime, bool is_initial)
{
	traceinfo->initial = is_initial;
	traceinfo->starttime = currentime;
	traceinfo->endtime = currentime;
	traceinfo->pushtime = pushtimes;
	dfzq_grpc_trace_write(traceinfo);
}

/*
* func：释放推送信息
* auth：huyn
*/
void dfzq_grpc_free_pushtrace(dfzq_grpc_common_traceinfo_t **traceinfo) {
	dfzq_grpc_common_traceinfo_t *trace = *traceinfo;
	FREE_PTR(trace->traceid);
	trace->traceid = NULL;
	trace->parentchainid = NULL;                              //无需释放
	trace->chainid = NULL;                                   //无需释放
	trace->servicename = NULL;
	trace->methodname = NULL;
	trace->consumerhost = NULL;
	trace->providerhost = NULL;
	trace->providerport = 0;
	trace->protocol = NULL;
	trace->appname = NULL;
	trace->servicegroup = NULL;
	trace->serviceversion = NULL;
	FREE_PTR(trace);
	trace = NULL;
}

/*
*流式推送结束
*/
void dfzq_grpc_push_finish(dfzq_grpc_common_traceinfo_t *traceinfo, long pushtimes, uint64_t currentime) {
	//写入待推送内容
	dfzq_grpc_write_trace_to_array(traceinfo, pushtimes, currentime, false);

	//释放推送内容
	dfzq_grpc_free_pushtrace(&traceinfo);
}

/*
* func:这是一个测试方法用于测试一些内部方法
* date：20170824
* auth：huyn
*/
void dfzq_grpc_extend_trace_test() {
	dfzq_grpc_trace_read_count();
}