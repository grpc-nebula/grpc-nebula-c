/*
 *    Author : heiden deng(dengjianquan@beyondcent.com)
 *    2017/06/13
 *    version 0.0.9
 *    类型定义
 */

#ifndef ORIENTSEC_TYPES_H
#define ORIENTSEC_TYPES_H

#include <stdbool.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

#define ORIENTSEC_GRPC_LOCAL_HOST "127.0.0.1"
#define ORIENTSEC_GRPC_ANY_HOST "0.0.0.0"

#define ORIENTSEC_GRPC_LB_TYPE_GRPCLB "grpclb"
#define ORIENTSEC_GRPC_LB_TYPE_PF "pick_first"
#define ORIENTSEC_GRPC_LB_TYPE_RR "round_robin"
#define ORIENTSEC_GRPC_LB_TYPE_WRR "weight_round_robin"
#define ORIENTSEC_GRPC_LB_TYPE_CH "consistent_hash"
#define ORIENTSEC_GRPC_DEFAULT_LB_TYPE ORIENTSEC_GRPC_LB_TYPE_RR

#define ORIENTSEC_GRPC_LB_MODE_REQUEST "request"
#define ORIENTSEC_GRPC_LB_MODE_CONNECT "connection"

#define ORIENTSEC_GRPC_CLUSTER_TYPE_FO "failover"
#define ORIENTSEC_GRPC_CLUSTER_TYPE_FF "failfast"
#define ORIENTSEC_GRPC_CLUSTER_TYPE_FB "failback"
#define ORIENTSEC_GRPC_CLUSTER_TYPE_FK "forking"

// used in grpc framework definition
#define ORIENTSEC_GRPC_VERSION_PRIFIX "c-"

typedef enum _loadbalance_strategy_t {
  GRPCLB,
  PICK_FIRST,
  ROUND_ROBIN,
  WEIGHT_ROUND_ROBIN,
  CONSISTENT_HASH
} loadbalance_strategy_t;

typedef enum _cluster_strategy_t {
  FAILOVER,
  FAILFAST,
  FAILBACK,
  FORKING
} cluster_strategy_t;

typedef enum _service_type_t { PUBLIC, PRIVATE, HYBRID } service_type_t;

#define PROVIDER_PROPERTIES_MAX_NUM 50
#define PROTOCOL_NAME_MAX_LEN 16
#define HOST_MAX_LEN 32
#define VERSION_MAX_LEN 32
#define GROUP_MAX_LEN 32
#define SIDE_MAX_LEN 16

typedef struct _provider_t {
  char protocol[PROTOCOL_NAME_MAX_LEN];
  char* username;
  char* password;
  char host[HOST_MAX_LEN];
  // char *version;
  char version[VERSION_MAX_LEN];
  char group[GROUP_MAX_LEN];
  int port;
  int default_timeout;
  int default_reties;
  int default_connections;
  char side[SIDE_MAX_LEN];  // provider category
  char* token;
  char* owner;
  long pid;
  bool access_protected;
  bool default_async;
  bool deprecated;
  bool dynamic;
  bool accesslog;
  bool anyhost;
  bool flag_in_blklist;     // 0: not in blacklist 1: in blacklist
  bool flag_call_failover;  // 1: not failover     1: failover
  int weight;
  int curr_weight;
  loadbalance_strategy_t default_loadbalance;
  cluster_strategy_t default_cluster;
  char* application;
  char* application_version;
  char* organization;
  char* environment;
  char* module;
  char* module_version;
  char* sInterface;
  char* methods;
  int64_t timestamp;
  int64_t flag_invalid_timestamp;  // invalid timestamp
  int64_t time_subchannel_close;   // timestamp of subchannel last closed
  char* grpc;
  char* dubbo;
  int default_requests;
  int flag_invalid;           // 1: invalid 0:valid
  int flag_subchannel_close;  // 1: closed  0: not closed
  void* ext_data;
  char* project;  // add by liumin //default=grpc-test-app
  char* comm_owner;
  char* serv_type;
  bool is_master;  // add by yang, for active/standby server
  bool online;     // mark provider was online or offline
} provider_t;

#define ORIENTSEC_GRPC_PROVIDER_FLAG_SUB_CLOSED_NOT 0  //有效标记可正常访问
#define ORIENTSEC_GRPC_PROVIDER_FLAG_SUB_CLOSED 1      // subchannel已经关闭
#define ORIENTSEC_GRPC_PROVIDER_FLAG_OK 0  //有效标记可正常访问
#define ORIENTSEC_GRPC_PROVIDER_FLAG_IN_BLKLIST_NOT 0  //非黑名单
#define ORIENTSEC_GRPC_PROVIDER_FLAG_IN_BLKLIST 1      //在黑名单中
#define ORIENTSEC_GRPC_PROVIDER_FLAG_CALL_OK 0  //有效标记可正常访问
#define ORIENTSEC_GRPC_PROVIDER_FLAG_CALL_FAIlOVER \
  1  //调用出错，多次调用出错设置本标记
//#define ORIENTSEC_GRPC_PROVIDER_FLAG_NOT_ACCESSED_BY_OLD_CLIENT 3
////新服务上线，新服务不被旧客户端调用

enum RegCode {
  /// Not an error; returned on success.
  OK = 0,
  APPL_NOT_CONF = 1,
  PROJ_NOT_CONF = 2,
  OWNR_NOT_CONF = 3,
  VSON_NOT_CONF = 4,
  OTHER = -1
};

#ifdef __cplusplus
}
#endif

#endif  // !ORIENTSEC_TYPES_H
