#include "dfzq_grpc_consumer_trace.h"
#include <string.h>
#include "grpc/support/tls.h"
#include "dfzq_grpc_common.h"
#include "dfzq_grpc_utils.h"
#include "dfzq_grpc_common_utils.h"
#include "dfzq_grpc_consumer_trace_sample.h"
#include "grpc/support/thd.h"

//------------------start deal  consumer threadlocal traceinfo------------------
//获取流式推送时，生成及发送服务跟踪信息时间间隔，单位毫秒，默认值5000毫秒即5秒钟。
static uint64_t g_dfzq_grpc_push_trace_interval = 5000;

GPR_TLS_DECL(dfzq_grpc_consumer_traceinfo);

static void dfzq_grpc_enter_ctx(dfzq_grpc_common_traceinfo_t *traceinfo) {
	gpr_tls_set(&dfzq_grpc_consumer_traceinfo, (intptr_t)traceinfo);
}

//获取当前存放的threadlocal信息
dfzq_grpc_common_traceinfo_t *dfzq_grpc_consumer_getcurrenttrace() {
	dfzq_grpc_common_traceinfo_t *c = (dfzq_grpc_common_traceinfo_t *)gpr_tls_get(&dfzq_grpc_consumer_traceinfo);
	return c;
}

//组织C端的监控内容
dfzq_grpc_common_traceinfo_t *dfzq_grpc_consumer_newfirsttrace(const char *fullmethod)
{
	//获取threadid,把threadid存入threadlocal对象
	size_t size = 37 * sizeof(char);
	char *traceid = (char *)malloc(size);
	memset(traceid, 0, size);
	dfzq_grpc_uuid(traceid);
	dfzq_grpc_common_traceinfo_t *traceinfo = (dfzq_grpc_common_traceinfo_t*)gpr_zalloc(sizeof(dfzq_grpc_common_traceinfo_t));
	traceinfo->traceid = traceid;
	traceinfo->parentchainid = "";                        //无需释放
	traceinfo->chainid = "0";                             //无需释放
	traceinfo->callcount = 0;
	traceinfo->initial = true;
	dfzq_grpc_getserveice_by_fullmethod(fullmethod, &traceinfo->servicename);
	traceinfo->methodname = dfzq_grpc_getmethodname_by_fullmethod(fullmethod);
	traceinfo->success = true;
	traceinfo->consumerside = true;
	traceinfo->consumerhost = get_local_ip();                     //无需释放
	traceinfo->consumerport = 0;                                //无需释放
	traceinfo->protocol = DFZQ_GRPC_TRACE_PROTOCOL;             //无需释放
	traceinfo->appname = dfzq_get_provider_AppName();             //无需释放
	traceinfo->servicegroup = NULL;	                           //无需释放;
	traceinfo->serviceversion = dfzq_grpc_version();              //无需释放;
	traceinfo->starttime = dfzq_get_timestamp_in_mills();
	//当前设置为不采样时，直接发送kafka
	if (dfzq_grpc_consumer_trace_issample() == DFZQ_GRPC_KAFKA_FLAG_UNSAMPLE) {
		//需发送kafka
		traceinfo->writekafka = 1;
	}
	else { //根据采样算法计算出是否需要发kafka 
		traceinfo->writekafka = dfzq_grpc_consumer_trace_getsampleflag();
	}
	traceid = NULL;
	return traceinfo;
}

//根据chainid规则生成chainid
void dfzq_grpc_consumer_trace_newchainid(dfzq_grpc_common_traceinfo_t *threadtrace, char *chainid) {
	//chain = "0.-1";
	if (threadtrace->callcount < 0 || threadtrace->parentchainid == NULL
		|| strlen(threadtrace->parentchainid) == 0) {
		return;
	}
	//传给C
	size_t plen = strlen(threadtrace->parentchainid);
	if (threadtrace->consumerside == false || plen < 3) {
		memcpy(chainid, threadtrace->parentchainid, strlen(threadtrace->parentchainid) * sizeof(char));
		return;
	}
	//待改写代码 
	size_t len = plen + 10;
	char *chain = (char*)malloc(len * sizeof(char));
	memset(chain, 0, len * sizeof(char));
	// 当前系统即为服务提供者，也为服务消费者；在调用其他服务提供者的时候，chainId有特殊的计算办法
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
* 根据P传递或C传递给C信息重新组织trace信息
* fullmethod:方法全名
* servertrace：P端trace信息
*/
dfzq_grpc_common_traceinfo_t *dfzq_grpc_consumer_newmiddletrace(const char *fullmethod,
	dfzq_grpc_common_traceinfo_t *threadtrace) {

	//校验threadlocal内部，consumerside标记校验threadlocal的来源
	//来自P端和C端时，有不同的处置流程。

	dfzq_grpc_common_traceinfo_t *traceinfo = (dfzq_grpc_common_traceinfo_t*)gpr_zalloc(sizeof(dfzq_grpc_common_traceinfo_t));

	if (threadtrace->consumerside == 0) {
		// 一个系统即作为服务端，又作为客户端的情况
		// Consumer A --> Provider B(&Consumer B) --> Provider C
		traceinfo->consumerside = 1;   //修改为消费端标记
		traceinfo->callcount = 0;  // 从0开始计数
	}
	//分配空间
	traceinfo->traceid = threadtrace->traceid;
	traceinfo->parentchainid = threadtrace->parentchainid;
	//chainid 生成
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
	traceinfo->appname = dfzq_get_provider_AppName();             //无需释放
	traceinfo->servicegroup = NULL;                               //无需释放
	traceinfo->serviceversion = dfzq_grpc_version();  //无需释放
	traceinfo->starttime = dfzq_get_timestamp_in_mills();
	traceinfo->writekafka = threadtrace->writekafka;

	return traceinfo;
}

/*
* 在C端调用方法时，生成trace信息.
*/
dfzq_grpc_common_traceinfo_t *dfzq_grpc_consumer_gentrace(const char *fullmethod) {
	//调用threadlocal实现接口，校验线程变量是否存在
	//如果线程变量不存在，说明当前是首节点,新建C端trace对象
	//如果现场变量已存在，说明当前非首节点,说明当前节点及既是P端
	//获取当前P的trace信息、生成C端trace。

	dfzq_grpc_trace_threadlocal_t *threadlocal = dfzq_grpc_trace_threadlocal_getcurrenttrace();
	dfzq_grpc_common_traceinfo_t *clienttrace = NULL;
	
	//首先校验C和P threadlocal是否存在
	//获取C和P的threadlocal如果都不存在
	if (threadlocal == NULL) {
		clienttrace = dfzq_grpc_consumer_newfirsttrace(fullmethod);
		//首节不需要持续计数，可以考虑不需要存放threadlocal 待验证	
	}
	else {
		//既是P端又是C端的情况。
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
		//组织代存入的threadlocal对象，写入threadlocal
		threadlocal->consumertrace = clienttrace;
		dfzq_grpc_trace_threadlocal_enter(threadlocal);
	}
	return clienttrace;
}

/*
* func：生成流式推送consumer端推动信息
* auth：huyn
* date：20170828
*/
dfzq_grpc_common_traceinfo_t *dfzq_grpc_consumer_newpushtrace(dfzq_grpc_common_traceinfo_t *trace) {
	dfzq_grpc_common_traceinfo_t *traceinfo = (dfzq_grpc_common_traceinfo_t*)gpr_zalloc(sizeof(dfzq_grpc_common_traceinfo_t));
	size_t size = 37 * sizeof(char);
	char *traceid = (char *)malloc(size);
	memset(traceid, 0, size);
	dfzq_grpc_uuid(traceid);
	traceinfo->traceid = traceid;
	traceinfo->parentchainid = "";                              //无需释放
	traceinfo->chainid = "0";                                   //无需释放
	traceinfo->callcount = 0;
	traceinfo->initial = true;
	traceinfo->servicename = trace->servicename;
	traceinfo->methodname = trace->methodname;
	traceinfo->success = true;
	traceinfo->consumerside = true;
	traceinfo->consumerhost = trace->consumerhost;                //无需释放
	traceinfo->consumerport = 0;                                //无需释放
	traceinfo->protocol = DFZQ_GRPC_TRACE_PROTOCOL;             //无需释放
	traceinfo->appname = dfzq_get_provider_AppName();             //无需释放
	traceinfo->servicegroup = NULL;	                           //无需释放;
	traceinfo->serviceversion = dfzq_grpc_version();  //无需释放;
	traceinfo->starttime = dfzq_get_timestamp_in_mills();
	traceinfo->writekafka = 1;
	traceinfo->pushtime = 0;
	traceid = NULL;
	return traceinfo;
}

//获取流式推送时，生成及发送服务跟踪信息时间间隔
uint64_t dfzq_grpc_push_trace_interval_get() {
	return g_dfzq_grpc_push_trace_interval;
}