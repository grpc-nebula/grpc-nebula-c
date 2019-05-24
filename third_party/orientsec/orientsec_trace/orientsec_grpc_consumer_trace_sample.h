/*
*    Author :huyn
*    2017/07/17
*    version 0.0.9
*    ���������Ϣ
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
	//kafka����Ƶ�ʼ�ֵ����������ļ��������˲�����������ʾ��Ҫ������
	//���Ϊ���ñ�ʾ����Ҫ�������������ɵĸ������ݶ����͵�kafka
    #define DFZQ_GRPC_KAFKA_SAMPLING_FREQUENCY "kafka.sampling.frequency"

	//Ϊ��ֵʱ����ʾ��Ҫ����
	#define DFZQ_GRPC_KAFKA_FLAG_SAMPLE 1

	//Ϊ��ֵʱ����ʾ����Ҫ����
	#define DFZQ_GRPC_KAFKA_FLAG_UNSAMPLE 0

	//�����ṹ��
	struct _dfzq_grpc_consumer_sample_t {
		uint64_t samplecount;     //ÿ�����ڲ�������
		uint64_t sampleinterval;  //ÿ���������ڵ�ʱ������λ��
		gpr_mu mu;
	};

	//���������������
	typedef struct _dfzq_grpc_consumer_sample_t dfzq_grpc_consumer_sample_t;

	/*
	* ���ز�����ʾ�Ƿ���Ҫ����,Ĭ�Ϸ���0
	* 1����Ҫ����
	* 0������Ҫ����
	*/
	int dfzq_grpc_consumer_trace_issample();

	/*
	* ������ٲ������ݽṹ
	* ����ʼ��ʱ����
	*/
	void dfzq_grpc_consumer_trace_initsampleinfo();

	/*
	* �жϵ�ǰ��¼�Ƿ���Ҫ����
	*/
	int dfzq_grpc_consumer_trace_getsampleflag();

#ifdef __cplusplus
}
#endif
#endif // !DFZQ_GRPC_CONSUMER_TRACE_SAMPLE_H


