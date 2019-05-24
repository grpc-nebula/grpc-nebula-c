#include "dfzq_grpc_provider_trace.h"
#include "grpc/support/tls.h"
#include "dfzq_grpc_common.h"
#include "dfzq_grpc_common_utils.h"

//consumer ������÷��ش���
void dfzq_grpc_provider_callfinish(dfzq_grpc_common_traceinfo_t *traceinfo, bool issuccess) {
	//�ѷ��������Ϣд�����
	traceinfo->success = issuccess;
	dfzq_grpc_trace_write_queue(traceinfo);

	//���threadlocal����
	//���׽ڵ���server�����,ͬʱ����¼�C�ڵ��threadlocal����
	dfzq_grpc_trace_threadlocal_clear();
	dfzq_grpc_trace_free(&traceinfo);
}


/*
*func:����provider��ͨ��trace��������ʽ����trace��Ϣ��
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
	traceinfo->parentchainid = "";                              //�����ͷ�
	traceinfo->chainid = "0";                                   //�����ͷ�
	traceinfo->callcount = 0;
	traceinfo->initial = true;
	traceinfo->servicename = trace->servicename;
	traceinfo->methodname = trace->methodname;
	traceinfo->success = true;
	traceinfo->consumerside = false;
	traceinfo->consumerhost = trace->consumerhost;                //�����ͷ�
	traceinfo->consumerport = 0;                                  //�����ͷ�
	traceinfo->providerhost = trace->providerhost;                //�����ͷ�
	traceinfo->providerport = trace->providerport;
	traceinfo->protocol = DFZQ_GRPC_TRACE_PROTOCOL;               //�����ͷ�
	traceinfo->appname = dfzq_get_provider_AppName();             //�����ͷ�
	traceinfo->servicegroup = NULL;	                              //�����ͷ�;
	traceinfo->serviceversion = dfzq_grpc_version();  //�����ͷ�;
	traceinfo->starttime = 0;
	traceinfo->writekafka = 1;
	traceinfo->pushtime = 0;
	new_traceid = NULL;
	return traceinfo;
}