/*
*    Author :huyn
*    2017/08/03
*    version 0.0.9
*    ��װ�²�ķ�����C++����
*/
#include "dfzq_grpc_trace_for_cc.h"

#include "stdio.h"
#include "dfzq_grpc_utils.h"
#include "dfzq_grpc_conf.h"
#include "dfzq_grpc_properties_tools.h"
#include "dfzq_grpc_common_utils.h"

static int dfzq_grpc_trace_sender_threadcount = -1;
static dfzq_grpc_trace_link_head_t dfzq_grpc_tracelinks[DFZQ_GRPC_TRACE_READ_THREADCOUNT];

//ͨ�����鷽ʽ��д���������Ϣ
dfzq_grpc_messageinfo_t *dfzq_grpc_trace_array[DFZQ_GRPC_TRACE_READ_THREADCOUNT];
static long dfzq_grpc_trace_index_write[DFZQ_GRPC_TRACE_READ_THREADCOUNT];
static long dfzq_grpc_trace_index_read[DFZQ_GRPC_TRACE_READ_THREADCOUNT];
static long dfzq_grpc_trace_array_len = 10000;
static char * dfzq_grpc_trace_message[DFZQ_GRPC_TRACE_READ_THREADCOUNT];
gpr_mu dfzq_grpc_array_index_mu[DFZQ_GRPC_TRACE_READ_THREADCOUNT];
static size_t dfzq_grpc_trace_message_size_mul = sizeof(char) *DFZQ_GRPC_TRACE_MESSAGE_CHARS_MULTIPLE;
static size_t dfzq_grpc_trace_message_size_sin = sizeof(char) *DFZQ_GRPC_TRACE_MESSAGE_CHARS_SINGLE;

//����dfzq_grpc_trace_sender_threadcount����������鳤��

//��ʼ������
void dfzq_grpc_trace_array_init() {
	int i = 0;
	int j = 0;
	for (i = 0; i < dfzq_grpc_trace_sender_threadcount; i++) {
		gpr_mu_init(&dfzq_grpc_array_index_mu[i]);
		*(dfzq_grpc_trace_array + i) = (dfzq_grpc_messageinfo_t*)malloc(dfzq_grpc_trace_array_len * sizeof(dfzq_grpc_messageinfo_t));
		for (j = 0; j < dfzq_grpc_trace_array_len; j++) {
			dfzq_grpc_trace_array[i][j].message = NULL;
			dfzq_grpc_trace_array[i][j].record_state = 0;
		}
		*(dfzq_grpc_trace_index_write + i) = -1;
		*(dfzq_grpc_trace_index_read + i) = -1;
		*(dfzq_grpc_trace_message + i) = (char*)malloc(dfzq_grpc_trace_message_size_mul);
	}
}

//��ȡָ�������д�±�
long dfzq_grpc_trace_new_write_index(int array_index) {
	long index = 0;
	//���������Ƿ�ִ���ٶȸ���  
	gpr_mu_lock(&dfzq_grpc_array_index_mu[array_index]);
	dfzq_grpc_trace_index_write[array_index] = (dfzq_grpc_trace_index_write[array_index] + 1) % dfzq_grpc_trace_array_len;
	index = dfzq_grpc_trace_index_write[array_index];
	gpr_mu_unlock(&dfzq_grpc_array_index_mu[array_index]);
	return index;
}

//д���� 
void dfzq_grpc_trace_array_write(int array_index, char *message) {
	long write_index = dfzq_grpc_trace_new_write_index(array_index);

	//addbylm
	if (dfzq_grpc_trace_array[array_index][write_index].message != NULL)
		free(dfzq_grpc_trace_array[array_index][write_index].message);
	dfzq_grpc_trace_array[array_index][write_index].message = message;
	dfzq_grpc_trace_array[array_index][write_index].record_state = 1;
	//addbylm
}

//������
long dfzq_grpc_message_read_count = 0;
char * dfzq_grpc_trace_array_read(int array_index) {
	long read_index = (*(dfzq_grpc_trace_index_read + array_index) + 1) % dfzq_grpc_trace_array_len;
	int rec_state = dfzq_grpc_trace_array[array_index][read_index].record_state;
	memset(*(dfzq_grpc_trace_message + array_index), 0, dfzq_grpc_trace_message_size_mul);
	strcpy(*(dfzq_grpc_trace_message + array_index), "[");
	while (rec_state == 1) {
		//ÿ������һ��
		if (strlen(dfzq_grpc_trace_message[array_index]) > 10) {
			break;
		}
		dfzq_grpc_trace_index_read[array_index]++;
		strcat(dfzq_grpc_trace_message[array_index], dfzq_grpc_trace_array[array_index][read_index].message);
		dfzq_grpc_trace_array[array_index][read_index].record_state = 0;
		read_index = (dfzq_grpc_trace_index_read[array_index] + 1) % dfzq_grpc_trace_array_len;
		rec_state = dfzq_grpc_trace_array[array_index][read_index].record_state;
	}
	if (strcmp(dfzq_grpc_trace_message[array_index], "[") == 0) {
		return  NULL;
	}
	else {
		strcat(dfzq_grpc_trace_message[array_index], "]");
	}
	return dfzq_grpc_trace_message[array_index];
}

/*
* ��ȡ�����߳���
*/
int dfzq_grpc_trace_getsendthreadcount() {
	return dfzq_grpc_trace_sender_threadcount;
}


//��ʼ����������
void dfzq_grpc_inittracesender() {
	size_t buffsize = sizeof(char) * 24;
	char *threadcount = (char*)malloc(buffsize);
	memset(threadcount, 0, buffsize);
	dfzq_grpc_properties_get_value(DFZQ_GRPC_CONF_KAFKA_SENDER_NUMBER, NULL, threadcount);

	//���Ϊ��������һ���߳�����˼��Ϣ

	// raise one thread for reflect info if not configurate 
	if (threadcount == NULL || strlen(threadcount) == 0) {
		dfzq_grpc_trace_sender_threadcount = 1;
	}
	else if (dfzq_grpc_common_utils_isdigit(threadcount) == true) {
		dfzq_grpc_trace_sender_threadcount = atoi(threadcount);
	}
	else {
		dfzq_grpc_trace_sender_threadcount = 1;
	}

	// add by yang, free the memory
	free(threadcount);
	//���õ��߳�����������DFZQ_GRPC_TRACE_READ_THREADCOUNT
	if (dfzq_grpc_trace_sender_threadcount > DFZQ_GRPC_TRACE_READ_THREADCOUNT) {
		dfzq_grpc_trace_sender_threadcount = DFZQ_GRPC_TRACE_READ_THREADCOUNT;
	}
	dfzq_grpc_trace_array_init();
}

/*
* ����ָ��ͷ���ָ��
*/
dfzq_grpc_trace_link_head_t *dfzq_grpc_gettracehead_pt(int index) {
	return &dfzq_grpc_tracelinks[index];
}

/*
* �ͷ�dfzq_grpc_trace_link_node_t�ڵ���Ϣ
*/
void dfzq_grpc_link_node_free(dfzq_grpc_trace_link_node_t **tracenode) {
	dfzq_grpc_trace_link_node_t *ptnode = *tracenode;
	ptnode->traceinfo = NULL; //traceinfo�����ڱ𴦽����ͷ�
	ptnode->next = NULL;
	free(ptnode);
}

void dfzq_grpc_trace_item_cat(char *outjson, char *pref, char *key, char *value) {
	if (pref != NULL)
		strcat(outjson, pref);
	strcat(outjson, "\"");
	strcat(outjson, key);
	strcat(outjson, "\":\"");
	if (value == NULL) {
	}
	else {
		strcat(outjson, value);
	}
	strcat(outjson, "\"");
}

void dfzq_grpc_trace_to_jsonstr(dfzq_grpc_common_traceinfo_t *traceinfo, char *outjson) {
	strcpy(outjson, "{");
	char buf[32];
	dfzq_grpc_trace_item_cat(outjson, NULL, "traceId", traceinfo->traceid);
	dfzq_grpc_trace_item_cat(outjson, ",", "chainId", traceinfo->chainid);
	dfzq_grpc_trace_item_cat(outjson, ",", "initial", traceinfo->initial ? "true" : "false");
	dfzq_grpc_trace_item_cat(outjson, ",", "serviceName", traceinfo->servicename);
	dfzq_grpc_trace_item_cat(outjson, ",", "methodName", traceinfo->methodname);
	REINIT(buf);
	sprintf(buf, "%ju", traceinfo->starttime);
	dfzq_grpc_trace_item_cat(outjson, ",", "startTime", buf);
	REINIT(buf);
	sprintf(buf, "%ju", traceinfo->endtime);
	dfzq_grpc_trace_item_cat(outjson, ",", "endTime", buf);
	dfzq_grpc_trace_item_cat(outjson, ",", "success", traceinfo->success ? "true" : "false");
	dfzq_grpc_trace_item_cat(outjson, ",", "consumerSide", traceinfo->consumerside ? "true" : "false");
	dfzq_grpc_trace_item_cat(outjson, ",", "consumerHost", traceinfo->consumerhost);
	REINIT(buf);
	sprintf(buf, "%d", traceinfo->consumerport);
	dfzq_grpc_trace_item_cat(outjson, ",", "consumerPort", buf);
	dfzq_grpc_trace_item_cat(outjson, ",", "providerHost", traceinfo->providerhost);
	REINIT(buf);
	sprintf(buf, "%d", traceinfo->providerport);
	dfzq_grpc_trace_item_cat(outjson, ",", "providerPort", buf);
	dfzq_grpc_trace_item_cat(outjson, ",", "protocol", traceinfo->protocol);
	dfzq_grpc_trace_item_cat(outjson, ",", "appName", traceinfo->appname);
	dfzq_grpc_trace_item_cat(outjson, ",", "serviceGroup", traceinfo->servicegroup);
	dfzq_grpc_trace_item_cat(outjson, ",", "serviceVersion", traceinfo->serviceversion);
	if (traceinfo->pushtime > 0) {
		REINIT(buf);
		sprintf(buf, "%ld", traceinfo->pushtime);
		dfzq_grpc_trace_item_cat(outjson, ",", "pushTimes", buf);
	}
	strcat(outjson, "}");
}

void dfzq_grpc_cat_trace_jsonstr(char *message, char *outjson) {
	if (strcmp(outjson, "[") != 0) {
		strcat(outjson, ",");
	}
	strcat(outjson, message);
}

//��ȡ����ڵ㣬��ȡ�ڵ��ɾ���ڵ㣬�������ȡ�ڵ����һ���ڵ���ɾ�����޸Ķ�ȡ���Ϊ�Ѷ�ȡ
int readcount = 0;
char *dfzq_grpc_trace_link_read(int index) {
	return dfzq_grpc_trace_array_read(index);
}

char * dfzq_grpc_trace_link_read_bk(int index) {
	dfzq_grpc_trace_link_head_t *pt = dfzq_grpc_gettracehead_pt(index);
	//���������ڵ�
	if (pt->isfirst != 1) {
		return NULL;
	}
	//���pt->ptread == NULL˵����δ��ȡȡ�������׽ڵ㿪ʼ��ȡ��
	if (pt->ptread == NULL) {
		pt->ptread = pt->ptfirst;
		pt->ptfirst = NULL;
	}

	if (pt->ptread == NULL) {
		printf(" \n----Error:trace link is null.");
		return NULL;
	}
	int maxcharnumbers = 100000;
	size_t size = maxcharnumbers * sizeof(char);
	char *result = (char*)malloc(size);
	memset(result, 0, size);
	strcpy(result, "[");

	dfzq_grpc_trace_link_node_t *ptnode = pt->ptread;
	dfzq_grpc_trace_link_node_t *ptnext = NULL;
	while (ptnode != NULL) {
		//�����ڴ�ռ�����500�ַ��ǲ��ڶ�ȡ��
		if ((maxcharnumbers - strlen(result)) < 500) {
			break;
		}
		ptnext = ptnode->next;
		//ƴ���ַ���
		//����������¼��ڵ㣬��ɾ�����ڵ�֮���޸ı��
		if (ptnext != NULL && ptnext->readflag != 0 && ptnext->readflag != 1) {
			printf("\n==============================readDataError!");
		}
		if (ptnode->readflag != 0 && ptnode->readflag != 1) {
			printf("\n==============================readDataError!");
		}
		if (ptnext == NULL) {
			if (ptnode->readflag == 1) {
				continue;
			}
			dfzq_grpc_cat_trace_jsonstr(ptnode->traceinfo, result);
			readcount++;
			ptnode->readflag = 1;
			pt->ptread = ptnode;
		}
		else {
			if (ptnode->readflag == 1) {

			}
			else {
				dfzq_grpc_cat_trace_jsonstr(ptnode->traceinfo, result);
				readcount++;
			}
			pt->ptread = ptnext;
			//ɾ��ptnode�ڵ�
			ptnode->next = NULL;
			dfzq_grpc_link_node_free(&ptnode);
		}
		ptnode = ptnext;
	}
	if (strcmp(result, "[") == 0) {
		result = NULL;
	}
	else {
		strcat(result, "]");
	}
	return result;
}

//�ѷ��������Ϣд������д��ʱ��Ҫ����
void dfzq_grpc_trace_write(dfzq_grpc_common_traceinfo_t *traceinfo) {
	//��ȡ���������±�
	int index = traceinfo->endtime % dfzq_grpc_trace_sender_threadcount;

	size_t size = DFZQ_GRPC_TRACE_MESSAGE_CHARS_SINGLE * sizeof(char);
	char *message = (char*)malloc(size);
	memset(message, 0, size);
	dfzq_grpc_trace_to_jsonstr(traceinfo, message);

	dfzq_grpc_trace_array_write(index, message);
}

void dfzq_grpc_trace_write_link(int index, char *message) {
	dfzq_grpc_trace_link_head_t *pt = dfzq_grpc_gettracehead_pt(index);
	dfzq_grpc_trace_link_node_t *ptnode = (dfzq_grpc_trace_link_node_t*)malloc(sizeof(dfzq_grpc_trace_link_node_t));
	ptnode->readflag = 0;
	ptnode->traceinfo = message;
	ptnode->next = NULL;

	//lock 
	if (pt->isfirst == 0) {
		gpr_mu_init(&pt->mu);
	}
	gpr_mu_lock(&pt->mu);
	if (pt->isfirst == 0) {
		pt->ptfirst = ptnode;
		pt->ptend = ptnode;
		pt->isfirst = 1;
	}
	else {
		pt->ptend->next = ptnode;
		pt->ptend = ptnode;
	}
	gpr_mu_unlock(&pt->mu);
}

/*
*func����ȡkafka�����������ַ
*date��20170811
*code��huyn
*/
void dfzq_grpc_trace_read_kafkaproxy_servers(char *confitem) {
	dfzq_grpc_properties_get_value(DFZQ_GRPC_CONF_KAFKA_SENDER_SERVERS, NULL, confitem);
}


//���ض�ȡ��¼����
long dfzq_grpc_trace_read_count() {
	return dfzq_grpc_message_read_count;
}





