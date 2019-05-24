//c����pʱ���ȸ��ݵ�ǰ
//p��
#include "dfzq_grpc_trace.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "grpc/support/tls.h"
#include "grpc/support/thd.h"
#include "grpc/support/alloc.h"

#include "dfzq_common/dfzq_grpc_properties_tools.h"
#include "dfzq_grpc_conf.h"
#include "dfzq_grpc_utils.h"

//�Ƿ����ɷ�������� 
// -1 δ��ʼ�� 1 ��Ҫ���ɷ����� 0 ����Ҫ���ɷ�����
static int dfzq_grpc_trace_gentrace_flag = -1;
static int dfzq_grpc_trace_threadlocal_isinit = -1;

//------------------start deal  consumer threadlocal traceinfo------------------
GPR_TLS_DECL(dfzq_grpc_trace_threadlocal);

//��ʼ��threadlocal����
void dfzq_grpc_trace_threadlocal_init() {
	if (dfzq_grpc_trace_threadlocal_isinit != 1) {
		gpr_tls_init(&dfzq_grpc_trace_threadlocal);
		dfzq_grpc_trace_threadlocal_isinit = 1;
	}
}

void dfzq_grpc_trace_threadlocal_enter(dfzq_grpc_trace_threadlocal_t *traceinfo) {
	gpr_tls_set(&dfzq_grpc_trace_threadlocal, (intptr_t)traceinfo);
}

//���threadlocal����
void dfzq_grpc_trace_threadlocal_clear_bk(dfzq_grpc_trace_threadlocal_t *traceinfo) {
	intptr_t pt = gpr_tls_get(&dfzq_grpc_trace_threadlocal);
	intptr_t pn = (intptr_t)traceinfo;
	//У��threadlocal�Ƿ�һ��
	if (pt > 0) {
		dfzq_grpc_trace_threadlocal_t *c = (dfzq_grpc_trace_threadlocal_t *)gpr_tls_get(&dfzq_grpc_trace_threadlocal);
	}
	gpr_tls_set(&dfzq_grpc_trace_threadlocal, 0);
}

/*
* �Ƿ�trace�ṹ����Ϣ
* ע�⣺�в��������ǲ���Ҫ�ͷŵ�ָ��ֱ���ÿվͿ�����
*/
void dfzq_grpc_trace_free(dfzq_grpc_common_traceinfo_t **traceinfo) {
	dfzq_grpc_common_traceinfo_t *ptrace = *traceinfo;
	if (ptrace == NULL) {
		return;
	}
	ptrace->traceid = NULL;
	if (ptrace->initial != true) {
		FREE_PTR(ptrace->chainid);
		ptrace->chainid = NULL;
		//parentchainid��provider���ͷ�
		ptrace->parentchainid = NULL;
	}

	/**
	* �����Ϣ/������
	* ����������֧��-_.��������.
	*/
	FREE_PTR(ptrace->servicename);

	/**
	* �������� ��֧��-_.$��������.
	*/
	FREE_PTR(ptrace->methodname);

	/**
	* ������ÿ�ʼʱ��.
	*/
	//FREE_PTR(ptrace->starttime);

	/**
	* ������ý���ʱ��.
	*/
	//FREE_PTR(ptrace->endtime); 

	/**
	* �������ѷ�ip.
	*/
	//FREE_PTR(ptrace->consumerhost);

	/**
	* �������ѷ��˿�.
	*/
	//FREE_PTR(ptrace->consumerport);

	/**
	* �����ṩ��ip.
	*/
	FREE_PTR(ptrace->providerhost);

	/**
	* �����ṩ���˿�.
	*/
	//FREE_PTR(ptrace->providerport);

	/**
	* ����Э�飬dubbo��rest��.
	*/
	//FREE_PTR(ptrace->protocol);   //��������Ҫ�ͷŸ�ָ��

	/**
	* ������飬 ��֧��-_.��������.
	*/
	//FREE_PTR(ptrace->servicegroup); 
	ptrace->servicegroup = NULL;
	ptrace->serviceversion = NULL;  //����Ҫ�ͷŸ�ָ��
	ptrace->appname = NULL;         //����Ҫ�ͷŸ�ָ��
	FREE_PTR(ptrace);
	ptrace = NULL;
}

/*
* �Ƿ�trace�ṹ����Ϣ
* ע�⣺�в��������ǲ���Ҫ�ͷŵ�ָ��ֱ���ÿվͿ�����
*/
void dfzq_grpc_trace_provider_free(dfzq_grpc_common_traceinfo_t **traceinfo) {
	dfzq_grpc_common_traceinfo_t *ptrace = *traceinfo;
	if (ptrace == NULL) {
		return;
	}
	FREE_PTR(ptrace->traceid);
	FREE_PTR(ptrace->chainid);
	FREE_PTR(ptrace->parentchainid);  //�ϼ��ڵ�chainid

	/**
	* �����Ϣ/������
	* ����������֧��-_.��������.
	*/
	FREE_PTR(ptrace->servicename);

	/**
	* �������� ��֧��-_.$��������.
	*/
	FREE_PTR(ptrace->methodname);

	/**
	* ������ÿ�ʼʱ��.
	*/
	//FREE_PTR(ptrace->starttime);

	/**
	* ������ý���ʱ��.
	*/
	//FREE_PTR(ptrace->endtime); 

	/**
	* �������ѷ�ip.
	*/
	FREE_PTR(ptrace->consumerhost);

	/**
	* �������ѷ��˿�.
	*/
	//FREE_PTR(ptrace->consumerport);

	/**
	* �����ṩ��ip.
	*/
	//FREE_PTR(ptrace->providerhost);

	/**
	* �����ṩ���˿�.
	*/
	//FREE_PTR(ptrace->providerport);

	/**
	* ����Э�飬dubbo��rest��.
	*/
	//FREE_PTR(ptrace->protocol);   //��������Ҫ�ͷŸ�ָ��

	/**
	* ������飬 ��֧��-_.��������.
	*/
	//FREE_PTR(ptrace->servicegroup); 
	ptrace->servicegroup = NULL;
	ptrace->serviceversion = NULL;  //����Ҫ�ͷŸ�ָ��
	ptrace->appname = NULL;         //����Ҫ�ͷŸ�ָ��
	FREE_PTR(ptrace);
	ptrace = NULL;
}

//���threadlocal����
void dfzq_grpc_trace_threadlocal_clear() {
	intptr_t pt = gpr_tls_get(&dfzq_grpc_trace_threadlocal);
	if (pt > 0) {
		dfzq_grpc_trace_threadlocal_t* thread_local = (dfzq_grpc_trace_threadlocal_t *)pt;
		dfzq_grpc_trace_provider_free(&thread_local->providertrace);
		dfzq_grpc_trace_free(&thread_local->consumertrace);
		thread_local->providertrace = NULL;
		dfzq_grpc_trace_threadlocal_t** thread_pt = &thread_local;
		free(*thread_pt);
		thread_pt = NULL;
		thread_local = NULL;
		gpr_tls_set(&dfzq_grpc_trace_threadlocal, 0);
	}
}

//��ȡ��ǰ��ŵ�threadlocal��Ϣ
dfzq_grpc_trace_threadlocal_t *dfzq_grpc_trace_threadlocal_getcurrenttrace() {
	intptr_t tthreadlocal = gpr_tls_get(&dfzq_grpc_trace_threadlocal);
	if (tthreadlocal <= 0) {
		return NULL;
	}
	return (dfzq_grpc_trace_threadlocal_t *)tthreadlocal;
}



//�Ƿ���з������ 
//1 ��Ҫ���з������ 
//0 �����з������
int dfzq_grpc_trace_info_istrace() {
	//����Ϊ��ʼ��
	if (dfzq_grpc_trace_gentrace_flag == -1) {
		//��ȡ�����ļ�
		size_t buffsize = sizeof(char) * 100;
		char *paramconf = (char*)gpr_malloc(buffsize);
		memset(paramconf, 0, buffsize);
		dfzq_grpc_properties_get_value(DFZQ_GRPC_TRACE_GENTRACE_KEY, NULL, paramconf);
		if (paramconf == NULL || strcmp(paramconf, "") == 0 || strcmp(paramconf, "true") == 0) {
			dfzq_grpc_trace_gentrace_flag = DFZQ_GRPC_TRACE_GENTRACE_Y;
		}
		else {
			dfzq_grpc_trace_gentrace_flag = DFZQ_GRPC_TRACE_GENTRACE_N;
		}
	}
	return dfzq_grpc_trace_gentrace_flag;
}

//��trace��Ϣд�����
//��ʱδʹ��
int dfzq_grpc_trace_write_queue(dfzq_grpc_common_traceinfo_t *traceinfo) {
	return 0;
}

char * dfzq_grpc_trace_protocol_get() {
	return "grpc";
}


