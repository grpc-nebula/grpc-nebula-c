#include "dfzq_grpc_consumer_trace_sample.h"
#include <string.h>
#include "stdio.h"
#include <stdlib.h>
#include <math.h>
#include "src/core/ext/census/resource.h"
#include "dfzq_grpc_utils.h"
#include "dfzq_grpc_common_utils.h"
#include "dfzq_grpc_properties_tools.h"

static int issample = -1;            //默认为0表示不需要采样 -1 未初始化 1 需要采样 0 不需要采样
static uint64_t cyclebegintime = 0; //计数周期开始时间
static uint64_t cyclesamplecnt = 0; //当前时钟周期已采样条数（采样周期变化后重新开始计数）

//默认每1000毫秒采集10条
static dfzq_grpc_consumer_sample_t sampleinfo = {
	.samplecount = 10,     //每个周期采样条数
	.sampleinterval = 1000   //每个采样周期的时长，单位毫秒秒 
};

/*
*
* 采样频率分数解析
*
*/
void dfzq_grpc_consumer_trace_sample_fractions(char * samplefrequency, char * indexpos) {
	//读取配置项，对配置项进行解析拆分构建采样数据结构。
	//char *samplefrequency = "10/1"; //服务链采样频率默认值10条每秒，后面可以考虑从配置先读取 
	//char *indexpos = strchr(samplefrequency, '/');
	//没有找到"/","/"在首位置或者结尾位置
	if (indexpos == NULL || (indexpos - samplefrequency) == 0 || strlen(indexpos) == 1) {
		issample = DFZQ_GRPC_KAFKA_FLAG_UNSAMPLE;
		return;
	}
	//  "/"前面字符串长度
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
	//设置采样参数
	sampleinfo.samplecount = atol(pbegin);
	sampleinfo.sampleinterval = atol(indexpos++);
	issample = DFZQ_GRPC_KAFKA_FLAG_SAMPLE;
	indexpos = NULL;
	FREE_PTR(samplefrequency);
}

/*
*
* 采样频率小数解析
*
*/
void dfzq_grpc_consumer_trace_sample_decimals(const char *samplefrequency, char * indexpos) {
	//读取配置项，对配置项进行解析拆分构建采样数据结构。
	//char *samplefrequency = "10/1"; //服务链采样频率默认值10条每秒，后面可以考虑从配置先读取 
	//char *indexpos = strchr(samplefrequency, '/');
	//没有找到"/","/"在首位置或者结尾位置
	if (indexpos == NULL || (indexpos - samplefrequency) == 0 || strlen(indexpos) == 1) {
		issample = DFZQ_GRPC_KAFKA_FLAG_UNSAMPLE;
		return;
	}

	int decnum = strlen(samplefrequency) - (indexpos - samplefrequency) - 1;
	//小数点后面没有数字，此为无效配置
	if (decnum <= 0) {
		issample = DFZQ_GRPC_KAFKA_FLAG_UNSAMPLE;
		return;
	}

	int charlen = strlen(samplefrequency);
	//atof用不了，直接做字符剔除
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
* 定义跟踪采样数据结构
* 供初始化时调用
* 	if (issample >= 0) 表示已经初始完毕直接返回
*    否则，读取配置文件初始化采样标记及采样频率相关参数
*/
void dfzq_grpc_consumer_trace_initsampleinfo() {
	//参数已经初始化
	if (issample >= 0) {
		return;
	}
	//初始化采样锁
	gpr_mu_init(&sampleinfo.mu);
	size_t buffsize = sizeof(char) * 100;
	char *samplefrequency = (char*)malloc(buffsize);
	memset(samplefrequency, 0, buffsize);
	dfzq_grpc_properties_get_value(DFZQ_GRPC_KAFKA_SAMPLING_FREQUENCY, NULL, samplefrequency);
	if (samplefrequency == NULL || strlen(samplefrequency) == 0) { //不采样
		issample = DFZQ_GRPC_KAFKA_FLAG_UNSAMPLE;
		FREE_PTR(samplefrequency);
		return;
	}
	//采样频率配置为分数
	char *indexpos = strchr(samplefrequency, '/');
	if ((indexpos - samplefrequency) > 0) {
		dfzq_grpc_consumer_trace_sample_fractions(samplefrequency, indexpos);
		return;
	}
	//采样频率为小数
	indexpos = strchr(samplefrequency, '.');
	if ((indexpos - samplefrequency) > 0) {
		dfzq_grpc_consumer_trace_sample_decimals(samplefrequency, indexpos);
		return;
	}

	//判断是否为整数
	if (dfzq_grpc_common_utils_isdigit(samplefrequency)) {
		//设置采样参数
		sampleinfo.samplecount = atol(samplefrequency);
		//按毫秒计量
		sampleinfo.sampleinterval = 1000L;
	}
	else { //不采样
		issample = DFZQ_GRPC_KAFKA_FLAG_UNSAMPLE;
	}
}

/*
* 返回参数标示是否需要采样,默认返回0
* 1、需要采样
* 0、不需要采样
*/
int dfzq_grpc_consumer_trace_issample() {
	//初始化采样标记，表示首次的时候进行初始化
	if (issample < 0) {
		dfzq_grpc_consumer_trace_initsampleinfo();
	}
	return issample;
}

/*
* 判读当前是否需要采样
*   1、获取当前时间
*   2、获取采样规则
*   3、判断当前是否采样
*/
int dfzq_grpc_consumer_trace_getsampleflag() {
	//读取配置项，对配置项进行解析拆分构建采样数据结构。
	dfzq_grpc_consumer_sample_t *samplerule = &sampleinfo;
	uint64_t currentime = dfzq_get_timestamp_in_mills();

	//采样周期变化
	gpr_mu_lock(&sampleinfo.mu);

	if ((currentime - cyclebegintime) > samplerule->sampleinterval) {
		cyclebegintime = currentime;
		cyclesamplecnt = 1;
		gpr_mu_unlock(&sampleinfo.mu);
		return 1;
	}
	else {
		//同一周期采样条数已满
		if (cyclesamplecnt > samplerule->samplecount) {
			gpr_mu_unlock(&sampleinfo.mu);
			return 0;
		}
		else { //同一周期采样条数未满
			cyclesamplecnt++;
			gpr_mu_unlock(&sampleinfo.mu);
			return 1;
		}
	}
	gpr_mu_unlock(&sampleinfo.mu);
	return 1;
}
