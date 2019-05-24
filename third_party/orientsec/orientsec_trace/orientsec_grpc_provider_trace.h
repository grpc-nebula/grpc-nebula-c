/*
*    Author :huyn
*    2017/07/13
*    version 0.0.9
*    服务跟踪信息-P端实现
*/

#ifndef DFZQ_GRPC_PROVIDR_TRACE_H
#define DFZQ_GRPC_PROVIDR_TRACE_H

#include "dfzq_grpc_trace.h"
#include "dfzq_types.h"

#ifdef __cplusplus
extern "C" {
#endif

	void dfzq_grpc_provider_callfinish(dfzq_grpc_common_traceinfo_t *traceinfo, bool issuccess);

	/*
	*func:根据provider端通用trace，生成流式推送trace信息。
	*/
	dfzq_grpc_common_traceinfo_t *dfzq_grpc_provider_newpushtrace(dfzq_grpc_common_traceinfo_t *trace);

#ifdef __cplusplus
}
#endif
#endif // !DFZQ_GRPC_PROVIDR_TRACE_H