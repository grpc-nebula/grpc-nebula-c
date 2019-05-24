//c调用p时首先根据当前
//p跟
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

//是否生成服务链标记 
// -1 未初始化 1 需要生成服务链 0 不需要生成服务链
static int dfzq_grpc_trace_gentrace_flag = -1;
static int dfzq_grpc_trace_threadlocal_isinit = -1;

//------------------start deal  consumer threadlocal traceinfo------------------
GPR_TLS_DECL(dfzq_grpc_trace_threadlocal);

//初始化threadlocal对象
void dfzq_grpc_trace_threadlocal_init() {
	if (dfzq_grpc_trace_threadlocal_isinit != 1) {
		gpr_tls_init(&dfzq_grpc_trace_threadlocal);
		dfzq_grpc_trace_threadlocal_isinit = 1;
	}
}

void dfzq_grpc_trace_threadlocal_enter(dfzq_grpc_trace_threadlocal_t *traceinfo) {
	gpr_tls_set(&dfzq_grpc_trace_threadlocal, (intptr_t)traceinfo);
}

//清除threadlocal对象
void dfzq_grpc_trace_threadlocal_clear_bk(dfzq_grpc_trace_threadlocal_t *traceinfo) {
	intptr_t pt = gpr_tls_get(&dfzq_grpc_trace_threadlocal);
	intptr_t pn = (intptr_t)traceinfo;
	//校验threadlocal是否一致
	if (pt > 0) {
		dfzq_grpc_trace_threadlocal_t *c = (dfzq_grpc_trace_threadlocal_t *)gpr_tls_get(&dfzq_grpc_trace_threadlocal);
	}
	gpr_tls_set(&dfzq_grpc_trace_threadlocal, 0);
}

/*
* 是否trace结构体信息
* 注意：有部分属性是不需要释放的指针直接置空就可以了
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
		//parentchainid在provider中释放
		ptrace->parentchainid = NULL;
	}

	/**
	* 监控消息/服务链
	* 服务名，仅支持-_.少数符号.
	*/
	FREE_PTR(ptrace->servicename);

	/**
	* 方法名， 仅支持-_.$少数符号.
	*/
	FREE_PTR(ptrace->methodname);

	/**
	* 服务调用开始时间.
	*/
	//FREE_PTR(ptrace->starttime);

	/**
	* 服务调用结束时间.
	*/
	//FREE_PTR(ptrace->endtime); 

	/**
	* 服务消费方ip.
	*/
	//FREE_PTR(ptrace->consumerhost);

	/**
	* 服务消费方端口.
	*/
	//FREE_PTR(ptrace->consumerport);

	/**
	* 服务提供方ip.
	*/
	FREE_PTR(ptrace->providerhost);

	/**
	* 服务提供方端口.
	*/
	//FREE_PTR(ptrace->providerport);

	/**
	* 服务协议，dubbo、rest等.
	*/
	//FREE_PTR(ptrace->protocol);   //常量不需要释放该指针

	/**
	* 服务分组， 仅支持-_.少数符号.
	*/
	//FREE_PTR(ptrace->servicegroup); 
	ptrace->servicegroup = NULL;
	ptrace->serviceversion = NULL;  //不需要释放该指针
	ptrace->appname = NULL;         //不需要释放该指针
	FREE_PTR(ptrace);
	ptrace = NULL;
}

/*
* 是否trace结构体信息
* 注意：有部分属性是不需要释放的指针直接置空就可以了
*/
void dfzq_grpc_trace_provider_free(dfzq_grpc_common_traceinfo_t **traceinfo) {
	dfzq_grpc_common_traceinfo_t *ptrace = *traceinfo;
	if (ptrace == NULL) {
		return;
	}
	FREE_PTR(ptrace->traceid);
	FREE_PTR(ptrace->chainid);
	FREE_PTR(ptrace->parentchainid);  //上级节点chainid

	/**
	* 监控消息/服务链
	* 服务名，仅支持-_.少数符号.
	*/
	FREE_PTR(ptrace->servicename);

	/**
	* 方法名， 仅支持-_.$少数符号.
	*/
	FREE_PTR(ptrace->methodname);

	/**
	* 服务调用开始时间.
	*/
	//FREE_PTR(ptrace->starttime);

	/**
	* 服务调用结束时间.
	*/
	//FREE_PTR(ptrace->endtime); 

	/**
	* 服务消费方ip.
	*/
	FREE_PTR(ptrace->consumerhost);

	/**
	* 服务消费方端口.
	*/
	//FREE_PTR(ptrace->consumerport);

	/**
	* 服务提供方ip.
	*/
	//FREE_PTR(ptrace->providerhost);

	/**
	* 服务提供方端口.
	*/
	//FREE_PTR(ptrace->providerport);

	/**
	* 服务协议，dubbo、rest等.
	*/
	//FREE_PTR(ptrace->protocol);   //常量不需要释放该指针

	/**
	* 服务分组， 仅支持-_.少数符号.
	*/
	//FREE_PTR(ptrace->servicegroup); 
	ptrace->servicegroup = NULL;
	ptrace->serviceversion = NULL;  //不需要释放该指针
	ptrace->appname = NULL;         //不需要释放该指针
	FREE_PTR(ptrace);
	ptrace = NULL;
}

//清除threadlocal对象
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

//获取当前存放的threadlocal信息
dfzq_grpc_trace_threadlocal_t *dfzq_grpc_trace_threadlocal_getcurrenttrace() {
	intptr_t tthreadlocal = gpr_tls_get(&dfzq_grpc_trace_threadlocal);
	if (tthreadlocal <= 0) {
		return NULL;
	}
	return (dfzq_grpc_trace_threadlocal_t *)tthreadlocal;
}



//是否进行服务跟踪 
//1 需要进行服务跟踪 
//0 不进行服务跟踪
int dfzq_grpc_trace_info_istrace() {
	//参数为初始化
	if (dfzq_grpc_trace_gentrace_flag == -1) {
		//读取配置文件
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

//把trace信息写入队列
//暂时未使用
int dfzq_grpc_trace_write_queue(dfzq_grpc_common_traceinfo_t *traceinfo) {
	return 0;
}

char * dfzq_grpc_trace_protocol_get() {
	return "grpc";
}


