/*
*    Author :huyn
*    2017/08/03
*    version 0.0.9
*    封装下层的方法供C++调用
*/

#ifndef DFZQ_GRPC_TRACE_FOR_CC_H
#define DFZQ_GRPC_TRACE_FOR_CC_H

#include "dfzq_grpc_trace.h"
#include <grpc/support/log.h>
#include <grpc/support/alloc.h>
#include <grpc/support/sync.h>
#include <src/core/lib/gpr/spinlock.h>

#ifdef __cplusplus
extern "C" {
#endif

//服务跟踪信息读取最大发送线程数
#define DFZQ_GRPC_TRACE_READ_THREADCOUNT 20

//单条服务更新信息最大字符数
#define DFZQ_GRPC_TRACE_MESSAGE_CHARS_SINGLE 1000
	
//拼接的多条服务跟踪信息最大字符数
#define DFZQ_GRPC_TRACE_MESSAGE_CHARS_MULTIPLE 20000

	//服务跟踪信息节点标记
	struct _dfzq_grpc_trace_link_node {
		struct _dfzq_grpc_trace_link_node *next;
		//存服务跟踪信息
		char *traceinfo;
		//dfzq_grpc_common_traceinfo_t *traceinfo;
		//标记本节点是否已被读取。 0 未读取 1 已结读取
		int readflag;
	};

	typedef struct _dfzq_grpc_trace_link_node dfzq_grpc_trace_link_node_t;

	/*
	* 服务跟踪链表头文件
	*
	*/
	struct _dfzq_grpc_trace_link_head {
		//是否首次创建节点
		int isfirst;
		dfzq_grpc_trace_link_node_t *ptfirst;
		//表已结读取到那个节点
		dfzq_grpc_trace_link_node_t *ptread;
		dfzq_grpc_trace_link_node_t *ptend;
		gpr_mu mu;   //写入锁		
	};

	typedef struct _dfzq_grpc_trace_link_head dfzq_grpc_trace_link_head_t;

	typedef struct _dfzq_grpc_messageinfo_t {
		char * message;
		int record_state;  // 0 可写入 1 新增  
	} dfzq_grpc_messageinfo_t;

	//读取链表节点，读取节点后删除节点，如果被读取节点最后一个节点则不删除，修改读取标记为已读取
	char *dfzq_grpc_trace_link_read(int index);

	//把服务跟踪信息写入链表，写入时需要加锁
	void dfzq_grpc_trace_write(dfzq_grpc_common_traceinfo_t *traceinfo);

	//trace初始化
	void dfzq_grpc_inittracesender();

	/*
	* 获取发送线程数
	*/
	int dfzq_grpc_trace_getsendthreadcount();

	void dfzq_grpc_trace_read_kafkaproxy_servers(char *confitem);

	//返回读取记录条数
	long dfzq_grpc_trace_read_count();

#ifdef __cplusplus
}
#endif
#endif // !DFZQ_GRPC_TRACE_FOR_CC_H
