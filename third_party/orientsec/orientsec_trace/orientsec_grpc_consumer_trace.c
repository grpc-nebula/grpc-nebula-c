#include "dfzq_grpc_consumer_trace.h"
#include <string.h>
#include "grpc/support/tls.h"
#include "dfzq_grpc_common.h"
#include "dfzq_grpc_utils.h"
#include "dfzq_grpc_common_utils.h"
#include "dfzq_grpc_consumer_trace_sample.h"
#include "grpc/support/thd.h"

//------------------start deal  consumer threadlocal traceinfo------------------
//��ȡ��ʽ����ʱ�����ɼ����ͷ��������Ϣʱ��������λ���룬Ĭ��ֵ5000���뼴5���ӡ�
static uint64_t g_dfzq_grpc_push_trace_interval = 5000;

GPR_TLS_DECL(dfzq_grpc_consumer_traceinfo);

static void dfzq_grpc_enter_ctx(dfzq_grpc_common_traceinfo_t *traceinfo) {
	gpr_tls_set(&dfzq_grpc_consumer_traceinfo, (intptr_t)traceinfo);
}

//��ȡ��ǰ��ŵ�threadlocal��Ϣ
dfzq_grpc_common_traceinfo_t *dfzq_grpc_consumer_getcurrenttrace() {
	dfzq_grpc_common_traceinfo_t *c = (dfzq_grpc_common_traceinfo_t *)gpr_tls_get(&dfzq_grpc_consumer_traceinfo);
	return c;
}

//��֯C�˵ļ������
dfzq_grpc_common_traceinfo_t *dfzq_grpc_consumer_newfirsttrace(const char *fullmethod)
{
	//��ȡthreadid,��threadid����threadlocal����
	size_t size = 37 * sizeof(char);
	char *traceid = (char *)malloc(size);
	memset(traceid, 0, size);
	dfzq_grpc_uuid(traceid);
	dfzq_grpc_common_traceinfo_t *traceinfo = (dfzq_grpc_common_traceinfo_t*)gpr_zalloc(sizeof(dfzq_grpc_common_traceinfo_t));
	traceinfo->traceid = traceid;
	traceinfo->parentchainid = "";                        //�����ͷ�
	traceinfo->chainid = "0";                             //�����ͷ�
	traceinfo->callcount = 0;
	traceinfo->initial = true;
	dfzq_grpc_getserveice_by_fullmethod(fullmethod, &traceinfo->servicename);
	traceinfo->methodname = dfzq_grpc_getmethodname_by_fullmethod(fullmethod);
	traceinfo->success = true;
	traceinfo->consumerside = true;
	traceinfo->consumerhost = get_local_ip();                     //�����ͷ�
	traceinfo->consumerport = 0;                                //�����ͷ�
	traceinfo->protocol = DFZQ_GRPC_TRACE_PROTOCOL;             //�����ͷ�
	traceinfo->appname = dfzq_get_provider_AppName();             //�����ͷ�
	traceinfo->servicegroup = NULL;	                           //�����ͷ�;
	traceinfo->serviceversion = dfzq_grpc_version();              //�����ͷ�;
	traceinfo->starttime = dfzq_get_timestamp_in_mills();
	//��ǰ����Ϊ������ʱ��ֱ�ӷ���kafka
	if (dfzq_grpc_consumer_trace_issample() == DFZQ_GRPC_KAFKA_FLAG_UNSAMPLE) {
		//�跢��kafka
		traceinfo->writekafka = 1;
	}
	else { //���ݲ����㷨������Ƿ���Ҫ��kafka 
		traceinfo->writekafka = dfzq_grpc_consumer_trace_getsampleflag();
	}
	traceid = NULL;
	return traceinfo;
}

//����chainid��������chainid
void dfzq_grpc_consumer_trace_newchainid(dfzq_grpc_common_traceinfo_t *threadtrace, char *chainid) {
	//chain = "0.-1";
	if (threadtrace->callcount < 0 || threadtrace->parentchainid == NULL
		|| strlen(threadtrace->parentchainid) == 0) {
		return;
	}
	//����C
	size_t plen = strlen(threadtrace->parentchainid);
	if (threadtrace->consumerside == false || plen < 3) {
		memcpy(chainid, threadtrace->parentchainid, strlen(threadtrace->parentchainid) * sizeof(char));
		return;
	}
	//����д���� 
	size_t len = plen + 10;
	char *chain = (char*)malloc(len * sizeof(char));
	memset(chain, 0, len * sizeof(char));
	// ��ǰϵͳ��Ϊ�����ṩ�ߣ�ҲΪ���������ߣ��ڵ������������ṩ�ߵ�ʱ��chainId������ļ���취
	strcpy(chain, threadtrace->parentchainid);
	int count = threadtrace->callcount;
	char *p = strrchr(chain, '.');
	if ((p - chain) > 1) {
		memcpy(chainid, chain, (p - chain) * sizeof(char));
		strcat(chainid, "-1");
	}
	p = NULL;
	FREE_PTR(chain);
}

/*
*
* ����P���ݻ�C���ݸ�C��Ϣ������֯trace��Ϣ
* fullmethod:����ȫ��
* servertrace��P��trace��Ϣ
*/
dfzq_grpc_common_traceinfo_t *dfzq_grpc_consumer_newmiddletrace(const char *fullmethod,
	dfzq_grpc_common_traceinfo_t *threadtrace) {

	//У��threadlocal�ڲ���consumerside���У��threadlocal����Դ
	//����P�˺�C��ʱ���в�ͬ�Ĵ������̡�

	dfzq_grpc_common_traceinfo_t *traceinfo = (dfzq_grpc_common_traceinfo_t*)gpr_zalloc(sizeof(dfzq_grpc_common_traceinfo_t));

	if (threadtrace->consumerside == 0) {
		// һ��ϵͳ����Ϊ����ˣ�����Ϊ�ͻ��˵����
		// Consumer A --> Provider B(&Consumer B) --> Provider C
		traceinfo->consumerside = 1;   //�޸�Ϊ���Ѷ˱��
		traceinfo->callcount = 0;  // ��0��ʼ����
	}
	//����ռ�
	traceinfo->traceid = threadtrace->traceid;
	traceinfo->parentchainid = threadtrace->parentchainid;
	//chainid ����
	size_t size = 1000 * sizeof(char);
	traceinfo->chainid = (char*)malloc(size);
	memset(traceinfo->chainid, 0, size);
	dfzq_grpc_consumer_trace_newchainid(threadtrace, traceinfo->chainid);

	traceinfo->callcount = traceinfo->callcount + 1;
	traceinfo->initial = false;
	dfzq_grpc_getserveice_by_fullmethod(fullmethod, &traceinfo->servicename);
	traceinfo->methodname = dfzq_grpc_getmethodname_by_fullmethod(fullmethod);
	traceinfo->success = true;
	traceinfo->consumerside = true;
	traceinfo->consumerhost = get_local_ip();
	traceinfo->consumerport = 0;
	traceinfo->protocol = DFZQ_GRPC_TRACE_PROTOCOL;
	traceinfo->appname = dfzq_get_provider_AppName();             //�����ͷ�
	traceinfo->servicegroup = NULL;                               //�����ͷ�
	traceinfo->serviceversion = dfzq_grpc_version();  //�����ͷ�
	traceinfo->starttime = dfzq_get_timestamp_in_mills();
	traceinfo->writekafka = threadtrace->writekafka;

	return traceinfo;
}

/*
* ��C�˵��÷���ʱ������trace��Ϣ.
*/
dfzq_grpc_common_traceinfo_t *dfzq_grpc_consumer_gentrace(const char *fullmethod) {
	//����threadlocalʵ�ֽӿڣ�У���̱߳����Ƿ����
	//����̱߳��������ڣ�˵����ǰ���׽ڵ�,�½�C��trace����
	//����ֳ������Ѵ��ڣ�˵����ǰ���׽ڵ�,˵����ǰ�ڵ㼰����P��
	//��ȡ��ǰP��trace��Ϣ������C��trace��

	dfzq_grpc_trace_threadlocal_t *threadlocal = dfzq_grpc_trace_threadlocal_getcurrenttrace();
	dfzq_grpc_common_traceinfo_t *clienttrace = NULL;
	
	//����У��C��P threadlocal�Ƿ����
	//��ȡC��P��threadlocal�����������
	if (threadlocal == NULL) {
		clienttrace = dfzq_grpc_consumer_newfirsttrace(fullmethod);
		//�׽ڲ���Ҫ�������������Կ��ǲ���Ҫ���threadlocal ����֤	
	}
	else {
		//����P������C�˵������
		if (threadlocal->consumertrace != NULL) {
			//create new trace info.
			clienttrace = dfzq_grpc_consumer_newmiddletrace(fullmethod, threadlocal->consumertrace);
			//free old trace
			gpr_free(threadlocal->consumertrace);
			//
		}
		else {
			clienttrace = dfzq_grpc_consumer_newmiddletrace(fullmethod, threadlocal->providertrace);
		}
		//��֯�������threadlocal����д��threadlocal
		threadlocal->consumertrace = clienttrace;
		dfzq_grpc_trace_threadlocal_enter(threadlocal);
	}
	return clienttrace;
}

/*
* func��������ʽ����consumer���ƶ���Ϣ
* auth��huyn
* date��20170828
*/
dfzq_grpc_common_traceinfo_t *dfzq_grpc_consumer_newpushtrace(dfzq_grpc_common_traceinfo_t *trace) {
	dfzq_grpc_common_traceinfo_t *traceinfo = (dfzq_grpc_common_traceinfo_t*)gpr_zalloc(sizeof(dfzq_grpc_common_traceinfo_t));
	size_t size = 37 * sizeof(char);
	char *traceid = (char *)malloc(size);
	memset(traceid, 0, size);
	dfzq_grpc_uuid(traceid);
	traceinfo->traceid = traceid;
	traceinfo->parentchainid = "";                              //�����ͷ�
	traceinfo->chainid = "0";                                   //�����ͷ�
	traceinfo->callcount = 0;
	traceinfo->initial = true;
	traceinfo->servicename = trace->servicename;
	traceinfo->methodname = trace->methodname;
	traceinfo->success = true;
	traceinfo->consumerside = true;
	traceinfo->consumerhost = trace->consumerhost;                //�����ͷ�
	traceinfo->consumerport = 0;                                //�����ͷ�
	traceinfo->protocol = DFZQ_GRPC_TRACE_PROTOCOL;             //�����ͷ�
	traceinfo->appname = dfzq_get_provider_AppName();             //�����ͷ�
	traceinfo->servicegroup = NULL;	                           //�����ͷ�;
	traceinfo->serviceversion = dfzq_grpc_version();  //�����ͷ�;
	traceinfo->starttime = dfzq_get_timestamp_in_mills();
	traceinfo->writekafka = 1;
	traceinfo->pushtime = 0;
	traceid = NULL;
	return traceinfo;
}

//��ȡ��ʽ����ʱ�����ɼ����ͷ��������Ϣʱ����
uint64_t dfzq_grpc_push_trace_interval_get() {
	return g_dfzq_grpc_push_trace_interval;
}