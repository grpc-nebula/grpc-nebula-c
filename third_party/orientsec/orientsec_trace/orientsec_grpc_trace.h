/*
*    Author :huyn 
*    2017/07/13
*    version 0.0.9
*    服务跟踪信息
*/
#ifndef DFZQ_GRPC_TRACE_H
#define DFZQ_GRPC_TRACE_H
#include <stdbool.h>
#include <stdint.h>
#include "dfzq_grpc_utils.h"
#ifdef __cplusplus
extern "C" {
#endif

    #define DFZQ_GRPC_TRACE_SEPARATOR "."

    //需要跟踪标记
    #define DFZQ_GRPC_TRACE_GENTRACE_Y 1

    #define DFZQ_GRPC_TRACE_GENTRACE_N 0  

    #define DFZQ_GRPC_TRACEID_LEN 37


	//服务跟踪信息
	struct _dfzq_grpc_common_traceinfo
	{

		/**
		* 服务链中每次完整链路中唯一id.
		*/
		char *traceid;

		//上级节点chainid
		char *parentchainid;

		//点击节点id
		char * chainid;


		/**
		* 监控消息/服务链
		* 服务名，仅支持-_.少数符号.
		*/
		char *servicename;

		/**
		* 方法名， 仅支持-_.$少数符号.
		*/
		char *methodname;

		/**
		* 服务调用开始时间.
		*/
		uint64_t starttime;

		/**
		* 服务调用结束时间.
		*/
		uint64_t endtime;


		/**
		* 服务消费方ip.
		*/
		char *consumerhost;


		/**
		* 服务提供方ip.
		*/
		char *providerhost;


		/**
		* 服务协议，dubbo、rest等.
		*/
		char *protocol;

		/**
		* 应用名称.
		* consumerside为是则为消费方应用名，否则为提供方应用名
		*/
		char *appname;

		/**
		* 服务分组， 仅支持-_.少数符号.
		*/
		char *servicegroup;

		/**
		* 服务版本.
		*/
		char *serviceversion;

		/**
		*推送次数，非流式推送时，取值为0
		*/
		long pushtime;


    /**
    * 服务提供方端口.
    */
    int providerport;

    /**
    * 服务消费方端口.
    */
    int consumerport;



    int callcount;

    /**
    * 当前服务链是否写入kafka
    * 1 需要写kafka，0 不需要写kafka
    */
    int writekafka;

    /**
    * 当前消息是否是服务链的起始节点.
    */
    bool initial;

    /**
    * 服务调用是否成功.
    */
    bool success;

    /**
    * 服务调用是否成功.
    */
    bool consumerside;
	};

	//服务跟踪类型定义
	typedef struct _dfzq_grpc_common_traceinfo dfzq_grpc_common_traceinfo_t;

	/*
	*  有用存放服务跟踪的threadlocal对象，
	*  把consumertrace和providertrace防止一个对象可以减少多次存取开销
	*/
	struct _dfzq_grpc_trace_threadlocal {
		dfzq_grpc_common_traceinfo_t *consumertrace;
		dfzq_grpc_common_traceinfo_t *providertrace;
	};
	typedef struct _dfzq_grpc_trace_threadlocal dfzq_grpc_trace_threadlocal_t;


	//释放dfzq_grpc_common_traceinfo_t结构体
	void dfzq_grpc_trace_free(dfzq_grpc_common_traceinfo_t **traceinfo);

	//是否进行服务跟踪 1 需要进行服务跟踪 0 不进行服务跟踪
	int dfzq_grpc_trace_info_istrace();

	//把服务跟踪信息写入待发送队列
	int dfzq_grpc_trace_write_queue(dfzq_grpc_common_traceinfo_t *traceinfo);

	//包对象写入threadlocal
	void dfzq_grpc_trace_threadlocal_enter(dfzq_grpc_trace_threadlocal_t *traceinfo);

	dfzq_grpc_trace_threadlocal_t *dfzq_grpc_trace_threadlocal_getcurrenttrace();

	//清除threadlocal对象
	void dfzq_grpc_trace_threadlocal_clear();

	//初始化threadlocal对象
	void dfzq_grpc_trace_threadlocal_init();

	char * dfzq_grpc_trace_protocol_get();

#ifdef __cplusplus
}
#endif
#endif // !DFZQ_GRPC_TRACE_H
