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
#include "dfzq_grpc_trace.h"
#include "dfzq_grpc_common_trace_key.h"
#include <grpc/support/port_platform.h>
#include "src/core/lib/surface/channel.h"
#include "src/core/lib/surface/call.h"
#include "dfzq_grpc_trace_for_cc.h"
#include <dfzq_trace/dfzq_grpc_extend_trace.h>
#include "dfzq_grpc_extend_trace_client.h"
#include "grpc/support/tls.h"
#include "grpc/support/thd.h"
//#include "grpc/support/alloc.h"
#include "string.h"

//spilt  ipv4:ip:port
void dfzq_grpc_consumer_deal_hostinfo_bk(dfzq_grpc_common_traceinfo_t *traceinfo, char *serverhost) {
	//解析serverhost
	if (serverhost == NULL) {
		return;
	}
	char *p = strchr(serverhost, ':') + 1;
	char *pvalue = strchr(p, ':') + 1;
	if ((pvalue - p) > 1) {
		size_t size = sizeof(char) * 100;
		traceinfo->providerhost = (char*)gpr_malloc(size);
		memset(traceinfo->providerhost, 0, size);
		memcpy(traceinfo->providerhost, p, sizeof(char)*(pvalue - p - 1));
	}

	pvalue = strrchr(serverhost, ':') + 1;
	if ((pvalue - serverhost) > 1) {
		size_t size = sizeof(char) * 100;
		char* port = (char*)gpr_malloc(size);
		memset(port, 0, size);
		memcpy(port, pvalue, sizeof(char)*(serverhost + strlen(serverhost) - pvalue));
		traceinfo->providerport = atoi(port);
		free(port);
	}
	p = NULL;
	pvalue = NULL;
}

//spilt  ip:port
void dfzq_grpc_consumer_deal_hostinfo(dfzq_grpc_common_traceinfo_t *traceinfo, char *serverhost) {
	//解析serverhost
	if (serverhost == NULL) {
		return;
	}
	char *pvalue = strchr(serverhost, ':') + 1;
	if ((pvalue - serverhost) > 1) {
		size_t size = sizeof(char) * 32;
		traceinfo->providerhost = (char*)gpr_malloc(size);
		memset(traceinfo->providerhost, 0, size);
		memcpy(traceinfo->providerhost, serverhost, sizeof(char)*(pvalue - serverhost - 1));
	}

	pvalue = strrchr(serverhost, ':') + 1;
	if ((pvalue - serverhost) > 1) {
		//addbylm
		traceinfo->providerport = atoi(pvalue);
		//endbylm
	}
	pvalue = NULL;
}


void dfzq_grpc_consumer_finish(dfzq_grpc_common_traceinfo_t *traceinfo, bool issuccess, char *serverhost) {
	//解析serverhost
	dfzq_grpc_consumer_deal_hostinfo(traceinfo, serverhost);

	traceinfo->success = issuccess;

	//把服务跟踪信息写入链表
	traceinfo->endtime = dfzq_get_timestamp_in_mills();
	dfzq_grpc_trace_write(traceinfo);

	//清除thread变量
	//首节点在此处释放、中间节点在调用返回后释放。
	if (traceinfo->initial == true) {
		//首节点清除，非首节点在server端清除
		//dfzq_grpc_trace_threadlocal_clear();
		free(traceinfo->traceid); //首节点释放traceid
		traceinfo->traceid = NULL;
		dfzq_grpc_trace_free(&traceinfo);
	}
}