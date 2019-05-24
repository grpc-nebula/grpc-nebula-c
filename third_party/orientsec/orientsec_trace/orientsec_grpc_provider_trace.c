#include "dfzq_grpc_provider_trace.h"
#include "grpc/support/tls.h"
#include "dfzq_grpc_common.h"
#include "dfzq_grpc_common_utils.h"

//consumer 服务调用返回处理
void dfzq_grpc_provider_callfinish(dfzq_grpc_common_traceinfo_t *traceinfo, bool issuccess) {
	//把服务跟踪信息写入队列
	traceinfo->success = issuccess;
	dfzq_grpc_trace_write_queue(traceinfo);

	//清除threadlocal变量
	//非首节点在server端清除,同时清除下级C节点的threadlocal对象
	dfzq_grpc_trace_threadlocal_clear();
	dfzq_grpc_trace_free(&traceinfo);
}


/*
*func:根据provider端通用trace，生成流式推送trace信息。
*/
dfzq_grpc_common_traceinfo_t *dfzq_grpc_provider_newpushtrace(dfzq_grpc_common_traceinfo_t *trace) {
	if (trace == NULL) {
		return NULL;
	}
	dfzq_grpc_common_traceinfo_t *traceinfo = (dfzq_grpc_common_traceinfo_t*)malloc(sizeof(dfzq_grpc_common_traceinfo_t));
	size_t size = 37 * sizeof(char);
	char *new_traceid = (char *)malloc(size);
	memset(new_traceid, 0, size);
	dfzq_grpc_uuid(new_traceid);
	traceinfo->traceid = new_traceid;
	traceinfo->parentchainid = "";                              //无需释放
	traceinfo->chainid = "0";                                   //无需释放
	traceinfo->callcount = 0;
	traceinfo->initial = true;
	traceinfo->servicename = trace->servicename;
	traceinfo->methodname = trace->methodname;
	traceinfo->success = true;
	traceinfo->consumerside = false;
	traceinfo->consumerhost = trace->consumerhost;                //无需释放
	traceinfo->consumerport = 0;                                  //无需释放
	traceinfo->providerhost = trace->providerhost;                //无需释放
	traceinfo->providerport = trace->providerport;
	traceinfo->protocol = DFZQ_GRPC_TRACE_PROTOCOL;               //无需释放
	traceinfo->appname = dfzq_get_provider_AppName();             //无需释放
	traceinfo->servicegroup = NULL;	                              //无需释放;
	traceinfo->serviceversion = dfzq_grpc_version();  //无需释放;
	traceinfo->starttime = 0;
	traceinfo->writekafka = 1;
	traceinfo->pushtime = 0;
	new_traceid = NULL;
	return traceinfo;
}