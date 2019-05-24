#include "dfzq_grpc_consumer_trace_sample.h"
#include <string.h>
#include "stdio.h"
#include <stdlib.h>
#include <math.h>
#include "src/core/ext/census/resource.h"
#include "dfzq_grpc_utils.h"
#include "dfzq_grpc_common_utils.h"
#include "dfzq_grpc_properties_tools.h"

static int issample = -1;            //Ĭ��Ϊ0��ʾ����Ҫ���� -1 δ��ʼ�� 1 ��Ҫ���� 0 ����Ҫ����
static uint64_t cyclebegintime = 0; //�������ڿ�ʼʱ��
static uint64_t cyclesamplecnt = 0; //��ǰʱ�������Ѳ����������������ڱ仯�����¿�ʼ������

//Ĭ��ÿ1000����ɼ�10��
static dfzq_grpc_consumer_sample_t sampleinfo = {
	.samplecount = 10,     //ÿ�����ڲ�������
	.sampleinterval = 1000   //ÿ���������ڵ�ʱ������λ������ 
};

/*
*
* ����Ƶ�ʷ�������
*
*/
void dfzq_grpc_consumer_trace_sample_fractions(char * samplefrequency, char * indexpos) {
	//��ȡ���������������н�����ֹ����������ݽṹ��
	//char *samplefrequency = "10/1"; //����������Ƶ��Ĭ��ֵ10��ÿ�룬������Կ��Ǵ������ȶ�ȡ 
	//char *indexpos = strchr(samplefrequency, '/');
	//û���ҵ�"/","/"����λ�û��߽�βλ��
	if (indexpos == NULL || (indexpos - samplefrequency) == 0 || strlen(indexpos) == 1) {
		issample = DFZQ_GRPC_KAFKA_FLAG_UNSAMPLE;
		return;
	}
	//  "/"ǰ���ַ�������
	int len = indexpos - samplefrequency;
	char *pbegin = (char*)calloc(sizeof(char), len + 1);
	int i = 0;
	for (i = 0; i < len; i++) {
		pbegin[i] = *(samplefrequency + i);
	}
	pbegin[len] = '\0';
	indexpos++;
	if (!dfzq_grpc_common_utils_isdigit(pbegin) || !dfzq_grpc_common_utils_isdigit(indexpos)) {
		issample = DFZQ_GRPC_KAFKA_FLAG_UNSAMPLE;
		return;
	}
	//���ò�������
	sampleinfo.samplecount = atol(pbegin);
	sampleinfo.sampleinterval = atol(indexpos++);
	issample = DFZQ_GRPC_KAFKA_FLAG_SAMPLE;
	indexpos = NULL;
	FREE_PTR(samplefrequency);
}

/*
*
* ����Ƶ��С������
*
*/
void dfzq_grpc_consumer_trace_sample_decimals(const char *samplefrequency, char * indexpos) {
	//��ȡ���������������н�����ֹ����������ݽṹ��
	//char *samplefrequency = "10/1"; //����������Ƶ��Ĭ��ֵ10��ÿ�룬������Կ��Ǵ������ȶ�ȡ 
	//char *indexpos = strchr(samplefrequency, '/');
	//û���ҵ�"/","/"����λ�û��߽�βλ��
	if (indexpos == NULL || (indexpos - samplefrequency) == 0 || strlen(indexpos) == 1) {
		issample = DFZQ_GRPC_KAFKA_FLAG_UNSAMPLE;
		return;
	}

	int decnum = strlen(samplefrequency) - (indexpos - samplefrequency) - 1;
	//С�������û�����֣���Ϊ��Ч����
	if (decnum <= 0) {
		issample = DFZQ_GRPC_KAFKA_FLAG_UNSAMPLE;
		return;
	}

	int charlen = strlen(samplefrequency);
	//atof�ò��ˣ�ֱ�����ַ��޳�
	char *datacount = (char*)calloc(sizeof(char), charlen);
	int j = 0;
	int i = 0;
	for (i = 0; i < charlen; i++) {
		if (*(samplefrequency + i) == '.') {
			continue;
		}
		datacount[j] = *(samplefrequency + i);
		j++;
	}
	datacount[j] = '\0';
	sampleinfo.samplecount = atol(datacount);
	sampleinfo.sampleinterval = 1000 * (int)(pow(10, decnum));

	issample = DFZQ_GRPC_KAFKA_FLAG_SAMPLE;
	indexpos = NULL;
	FREE_PTR(datacount);
	//FREE_PTR(samplefrequency);
}

/*
* ������ٲ������ݽṹ
* ����ʼ��ʱ����
* 	if (issample >= 0) ��ʾ�Ѿ���ʼ���ֱ�ӷ���
*    ���򣬶�ȡ�����ļ���ʼ��������Ǽ�����Ƶ����ز���
*/
void dfzq_grpc_consumer_trace_initsampleinfo() {
	//�����Ѿ���ʼ��
	if (issample >= 0) {
		return;
	}
	//��ʼ��������
	gpr_mu_init(&sampleinfo.mu);
	size_t buffsize = sizeof(char) * 100;
	char *samplefrequency = (char*)malloc(buffsize);
	memset(samplefrequency, 0, buffsize);
	dfzq_grpc_properties_get_value(DFZQ_GRPC_KAFKA_SAMPLING_FREQUENCY, NULL, samplefrequency);
	if (samplefrequency == NULL || strlen(samplefrequency) == 0) { //������
		issample = DFZQ_GRPC_KAFKA_FLAG_UNSAMPLE;
		FREE_PTR(samplefrequency);
		return;
	}
	//����Ƶ������Ϊ����
	char *indexpos = strchr(samplefrequency, '/');
	if ((indexpos - samplefrequency) > 0) {
		dfzq_grpc_consumer_trace_sample_fractions(samplefrequency, indexpos);
		return;
	}
	//����Ƶ��ΪС��
	indexpos = strchr(samplefrequency, '.');
	if ((indexpos - samplefrequency) > 0) {
		dfzq_grpc_consumer_trace_sample_decimals(samplefrequency, indexpos);
		return;
	}

	//�ж��Ƿ�Ϊ����
	if (dfzq_grpc_common_utils_isdigit(samplefrequency)) {
		//���ò�������
		sampleinfo.samplecount = atol(samplefrequency);
		//���������
		sampleinfo.sampleinterval = 1000L;
	}
	else { //������
		issample = DFZQ_GRPC_KAFKA_FLAG_UNSAMPLE;
	}
}

/*
* ���ز�����ʾ�Ƿ���Ҫ����,Ĭ�Ϸ���0
* 1����Ҫ����
* 0������Ҫ����
*/
int dfzq_grpc_consumer_trace_issample() {
	//��ʼ��������ǣ���ʾ�״ε�ʱ����г�ʼ��
	if (issample < 0) {
		dfzq_grpc_consumer_trace_initsampleinfo();
	}
	return issample;
}

/*
* �ж���ǰ�Ƿ���Ҫ����
*   1����ȡ��ǰʱ��
*   2����ȡ��������
*   3���жϵ�ǰ�Ƿ����
*/
int dfzq_grpc_consumer_trace_getsampleflag() {
	//��ȡ���������������н�����ֹ����������ݽṹ��
	dfzq_grpc_consumer_sample_t *samplerule = &sampleinfo;
	uint64_t currentime = dfzq_get_timestamp_in_mills();

	//�������ڱ仯
	gpr_mu_lock(&sampleinfo.mu);

	if ((currentime - cyclebegintime) > samplerule->sampleinterval) {
		cyclebegintime = currentime;
		cyclesamplecnt = 1;
		gpr_mu_unlock(&sampleinfo.mu);
		return 1;
	}
	else {
		//ͬһ���ڲ�����������
		if (cyclesamplecnt > samplerule->samplecount) {
			gpr_mu_unlock(&sampleinfo.mu);
			return 0;
		}
		else { //ͬһ���ڲ�������δ��
			cyclesamplecnt++;
			gpr_mu_unlock(&sampleinfo.mu);
			return 1;
		}
	}
	gpr_mu_unlock(&sampleinfo.mu);
	return 1;
}
