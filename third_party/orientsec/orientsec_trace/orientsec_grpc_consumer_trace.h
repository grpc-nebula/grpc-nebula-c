/*
*    Author :huyn
*    2017/07/13
*    version 0.0.9
*    服务跟踪信息
*/

#ifndef DFZQ_GRPC_CONSUMER_TRACE_H
#define DFZQ_GRPC_CONSUMER_TRACE_H

#include "dfzq_grpc_trace.h"
#ifdef __cplusplus
extern "C" {
#endif
	//P端再次调用下级节点时生成trace对象
	dfzq_grpc_common_traceinfo_t * dfzq_grpc_consumer_gentrace(const char *fullmethod);

	//consumer调用结束处理
	void dfzq_grpc_consumer_callfinish(dfzq_grpc_common_traceinfo_t *traceinfo, bool callresult, char *serverhost);

	//获取流式推送时，生成及发送服务跟踪信息时间间隔
	uint64_t dfzq_grpc_push_trace_interval_get();

	dfzq_grpc_common_traceinfo_t *dfzq_grpc_consumer_newpushtrace(dfzq_grpc_common_traceinfo_t *trace);

#ifdef __cplusplus
}
#endif
#endif // !DFZQ_GRPC_CONSUMER_TRACE_H
