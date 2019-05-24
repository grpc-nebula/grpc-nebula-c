/*
*    Author :huyn
*    2017/07/17
*    version 0.0.9
*    服务跟踪信息
*/

#ifndef DFZQ_GRPC_CONSUMER_TRACE_SAMPLE_H
#define DFZQ_GRPC_CONSUMER_TRACE_SAMPLE_H
#include "grpc/support/log.h"
#include "grpc/support/alloc.h"
#include "grpc/support/sync.h"
#include "../src/core/lib/support/spinlock.h"

#ifdef __cplusplus
extern "C" {
#endif
	//kafka采样频率键值，如果配置文件中配置了不本参数，表示需要采样，
	//如果为配置表示不需要采样，所有生成的跟踪数据都发送到kafka
    #define DFZQ_GRPC_KAFKA_SAMPLING_FREQUENCY "kafka.sampling.frequency"

	//为该值时，表示需要采样
	#define DFZQ_GRPC_KAFKA_FLAG_SAMPLE 1

	//为该值时，表示不需要采样
	#define DFZQ_GRPC_KAFKA_FLAG_UNSAMPLE 0

	//采样结构体
	struct _dfzq_grpc_consumer_sample_t {
		uint64_t samplecount;     //每个周期采样条数
		uint64_t sampleinterval;  //每个采样周期的时长，单位秒
		gpr_mu mu;
	};

	//定义采样数据类型
	typedef struct _dfzq_grpc_consumer_sample_t dfzq_grpc_consumer_sample_t;

	/*
	* 返回参数标示是否需要采样,默认返回0
	* 1、需要采样
	* 0、不需要采样
	*/
	int dfzq_grpc_consumer_trace_issample();

	/*
	* 定义跟踪采样数据结构
	* 供初始化时调用
	*/
	void dfzq_grpc_consumer_trace_initsampleinfo();

	/*
	* 判断当前记录是否需要采样
	*/
	int dfzq_grpc_consumer_trace_getsampleflag();

#ifdef __cplusplus
}
#endif
#endif // !DFZQ_GRPC_CONSUMER_TRACE_SAMPLE_H


