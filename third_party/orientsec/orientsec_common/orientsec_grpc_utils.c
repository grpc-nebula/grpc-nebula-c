/*
 * Copyright 2019 Orient Securities Co., Ltd.
 * Copyright 2019 BoCloud Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 *    Author : heiden deng(dengjianquan@beyondcent.com)
 *    2017/05/15
 *    version 0.9
 *    通用工具方法实现
 */

#include "orientsec_grpc_utils.h"
#include "orientsec_grpc_properties_constants.h"
#include "orientsec_grpc_properties_tools.h"
#include "orientsec_types.h"

#include <grpc/support/alloc.h>
#include <grpc/support/log.h>
#include <grpc/support/port_platform.h>
#include <grpc/support/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "src/core/lib/gpr/string.h"

#if (defined WIN64) || (defined WIN32)
#include <Windows.h>
#include <direct.h>
#include <process.h>
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#include <net/if.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#endif  //

static int orientsec_grpc_side_flag = 2;  // 2 consumer 1 provider
static char* get_localhost_ip();

int grpc_getpid() {
#if (defined WIN64) || (defined WIN32)
  return (int)GetCurrentProcessId();
#else
  return (int)getpid();
#endif  // DEBUG
}

uint64_t orientsec_get_timestamp_in_mills() {
  uint64_t ret = 0;
  gpr_timespec now = gpr_now(GPR_CLOCK_REALTIME);
  ret = now.tv_sec * 1000 + now.tv_nsec / 1000000;
  return ret;
}

static char g_local_ip[128] = {0};

// 判断IP地址是否有效 
static bool is_ip_valid(const char* pszIPAddr) {
  if (!pszIPAddr) return false;  //若pszIPAddr为空
  char IP1[100], cIP[4];
  int len = strlen(pszIPAddr);
  int i = 0, j = len - 1;
  int k, m = 0, n = 0, num = 0;
  //去除首尾空格(取出从i-1到j+1之间的字符):
  while (pszIPAddr[i++] == ' ');
  while (pszIPAddr[j--] == ' ');

  for (k = i - 1; k <= j + 1; k++) {
    IP1[m++] = *(pszIPAddr + k);
  }
  IP1[m] = '\0';

  char* p = IP1;

  while (*p != '\0') {
    if (*p == ' ' || *p < '0' || *p > '9') return false;
    cIP[n++] = *p;  //保存每个子段的第一个字符，用于之后判断该子段是否为0开头

    int sum = 0;  // sum为每一子段的数值，应在0到255之间
    while (*p != '.' && *p != '\0') {
      if (*p == ' ' || *p < '0' || *p > '9') return false;
      sum = sum * 10 + *p - 48;  //每一子段字符串转化为整数
      p++;
    }
    if (*p == '.') {
      if ((*(p - 1) >= '0' && *(p - 1) <= '9') &&
          (*(p + 1) >= '0' &&
           *(p + 1) <= '9'))  //判断"."前后是否有数字，若无，则为无效IP，如“1.1.127.”
        num++;  //记录“.”出现的次数，不能大于3
      else
        return false;
    };
    if ((sum > 255) || (sum > 0 && cIP[0] == '0') || num > 3)
      return false;  //若子段的值>255或为0开头的非0子段或“.”的数目>3，则为无效IP

    if (*p != '\0') p++;
    n = 0;
  }
  if (num != 3) return false;
  return true;
}

  // 获得主机IP 接口函数
char* get_local_ip() {
  char config_host_ip[64] = {0};

  if (*g_local_ip != '\0') {
    return g_local_ip;
  }

  //读取配置文件中的ip信息
  if (0 == orientsec_grpc_properties_get_value(ORIENTSEC_GRPC_PROPERTIES_COMMON_LOCALHOST_IP, NULL, config_host_ip)) {
    // 校验配置IP是否合法
    if (is_ip_valid(config_host_ip)) {
      int len = strlen(config_host_ip);
      snprintf(g_local_ip, len + 1, "%s", config_host_ip);
      return g_local_ip;
    }
  }
  get_localhost_ip();
  return g_local_ip;
}

// 获得本机IP
static char* get_localhost_ip() {
  bool bInitIP = false;
  int len = 0;
#if (defined WIN64) || (defined WIN32)
  int nAdapter = 0;
  WSADATA wsaData;
  char name[255];  //定义用于存放获得的主机名的变量
  char* ip;        //定义IP地址变量
  PHOSTENT hostinfo;

  if (*g_local_ip != '\0') {
    return g_local_ip;
  }
  //调用MAKEWORD（）获得Winsock版本的正确值，用于加载Winsock库

  if (WSAStartup(MAKEWORD(2, 0), &wsaData) == 0) {
    //现在是加载Winsock库，如果WSAStartup（）函数返回值为0，说明加载成功，程序可以继续
    if (gethostname(name, sizeof(name)) == 0) {
      //如果成功地将本地主机名存放入由name参数指定的缓冲区中
      if ((hostinfo = gethostbyname(name)) != NULL) {
        //这是获取主机名，如果获得主机名成功的话，将返回一个指针，指向hostinfo，hostinfo
        //为PHOSTENT型的变量，下面即将用到这个结构体
        while (*(hostinfo->h_addr_list + nAdapter)) {
          ip = inet_ntoa(*(struct in_addr*)hostinfo->h_addr_list[nAdapter]);
          //调用inet_ntoa（）函数，将hostinfo结构变量中的h_addr_list转化为标准的点分表示的IP
          //地址（如192.168.0.1）
          if (0 != strncmp(ip, "169.254", 7)) {
            len = strlen(ip);
            snprintf(g_local_ip, len + 1, "%s", ip);
            bInitIP = true;
            break;
          }
          nAdapter++;
        }
      }
    }
    WSACleanup();  //卸载Winsock库，并释放所有资源
  }

#else
  int i = 0;
  int sockfd;
  struct ifconf ifconf;
  char buf[512];
  struct ifreq* ifreq;
  char* ip;

  if (g_local_ip[0] != '\0') {
    return g_local_ip;
  }

  //初始化ifconf
  ifconf.ifc_len = 512;
  ifconf.ifc_buf = buf;

  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    return "127.0.0.1";
  }
  ioctl(sockfd, SIOCGIFCONF, &ifconf);  //获取所有接口信息
  close(sockfd);
  //接下来一个一个的获取IP地址
  ifreq = (struct ifreq*)buf;

  for (i = (ifconf.ifc_len / sizeof(struct ifreq)); i > 0; i--) {
    ip = inet_ntoa(((struct sockaddr_in*)&(ifreq->ifr_addr))->sin_addr);
    //排除127.0.0.1，继续下一个
    if (strcmp(ip, "127.0.0.1") == 0) {
      ifreq++;
      continue;
    }
    bInitIP = true;
    len = strlen(ip);
    snprintf(g_local_ip, len + 1, "%s", ip);
    break;
  }

#endif
  if (!bInitIP) {
    snprintf(g_local_ip, strlen(ORIENTSEC_GRPC_LOCAL_HOST) + 1, "%s",
             ORIENTSEC_GRPC_LOCAL_HOST);
  }
  return g_local_ip;
}

static char g_provider_applicationName[ORIENTSEC_GRPC_PROPERTY_VALUE_MAX_LEN] = {0};
char* orientsec_get_provider_AppName() {
  char* ptr = NULL;
  if (*g_provider_applicationName != '\0') {
    return g_provider_applicationName;
  }
  if (0 == orientsec_grpc_properties_get_value(ORIENTSEC_GRPC_PROPERTIES_COMMON_APP, NULL,
                                          g_provider_applicationName)) {
    return g_provider_applicationName;
  }
  return NULL;
}

char* get_service_name(char* method_full_name, char* buf, size_t buf_len) {
  char* p_start = NULL;
  char* p_end = NULL;
  int len = 0;
  p_start = strchr(method_full_name, '/');
  p_end = strrchr(method_full_name, '/');
  if (!p_start || !p_end || p_start == p_end) {
    return NULL;
  }
  len = p_end - p_start - 1;
  if (len >= buf_len) {
    return NULL;
  }
  memset(buf, 0, buf_len);
  snprintf(buf, len + 1, "%s", p_start + 1);
  return buf;
}

void trim(char* strIn, char* strOut) {
  char *start, *end, *temp;  //定义去除空格后字符串的头尾指针和遍历指针
  char* pOut = strOut;
  temp = strIn;

  while (*temp == ' ' || *temp == '\t') {
    ++temp;
  }

  start = temp;  //求得头指针

  temp = strIn + strlen(strIn) - 1;  //得到原字符串最后一个字符的指针(不是'\0')

  while (*temp == ' ' || *temp == '\n' || *temp == '\r' || *temp == '\t') {
    --temp;
  }

  end = temp;  //求得尾指针

  for (temp = start; temp <= end;) {
    *pOut++ = *temp++;
  }

  *pOut = '\0';
}

#if (defined WIN64) || (defined WIN32)
#else

int get_executable_path(char* processdir, int len, char* processname) {
  char* filename;

  if (readlink("/proc/self/exe", processdir, len) <= 0) {
    return -1;
  }

  filename = strrchr(processdir, PATH_SEPRATOR);  //函数查找字符在指定字符串中最后一次出现的位置如果成功，则返回指定字符最后一次出现位置的地址，如果失败，则返回
                                                  //false
  if (filename == NULL) return -1;
  ++filename;
  strcpy(processname, filename);
  *filename = '\0';
  return (int)(filename - processdir);
}
#endif

//#if __STDC_VERSION__ >= 199901L
char* gprc_strdup(const char* str) {
  if (!str) {
    return NULL;
  }
  int n = strlen(str) + 1;
  char* dup = malloc(n);
  if (dup) strcpy(dup, str);
  return dup;
}
//#endif

void orientsec_log(char* msg) { printf("%s\n", msg); }

int countchar(char* str, char a) {
  int n = 0;
  int i = 0;
  while (*(str + i) != '\0') {
    if (*(str + i) == a) n++;
    i++;
  }
  return n;
}

bool is_start_with(const char* pattern, const char* value) {
  if (!pattern || !value) {
    return false;
  }
  char* p = (char*)pattern;
  char* q = (char*)value;
  while (p && q) {
    if (strcmp("", q) == 0) {
      return true;
    }
    if (*p != *q) {
      return false;
    }
    p++;
    q++;
  }
  if (!q) {
    return true;
  }
  return false;
}

bool is_ends_with(const char* pattern, const char* value) {
  if (!pattern || !value) {
    return false;
  }
  char* p = (char*)pattern;
  p = p + strlen(pattern) - 1;

  char* q = (char*)value;
  q = q + strlen(value) - 1;
  while (p != pattern && q != value) {
    if (*p != *q) {
      return false;
    }
    p--;
    q--;
  }
  if (q == value) {
    if (*p == *q) {
      return true;
    }
  }
  return false;
}

/*
拷贝字符串，不包括pEnd位置字符
必须确保buf空间足够大
*/
int copystr0(char* buf, char* pSrc, char* pEnd) {
  size_t len = pEnd - pSrc;
  char* ret = NULL;
  char* p = pSrc;
  char* q = buf;
  if (!pSrc) {
    return -1;
  }
  if (!pEnd) {
    len = strlen(pSrc);
  }
  if (len > 0) {
    while (p != pEnd && *p) {
      *q++ = *p++;
    }
    *q = '\0';
  }
  return 0;
}

char* copystr(char* pSrc, char* pEnd) {
  size_t len = pEnd - pSrc;
  char* ret = NULL;
  char* p = pSrc;
  char* q = NULL;
  if (!pSrc) {
    return ret;
  }
  if (!pEnd) {
    len = strlen(pSrc);
  }
  if (len > 0) {
    ret = (char*)malloc(sizeof(char) * len + 1);
    q = ret;
    while (q && p != pEnd && *p) {
      *q++ = *p++;
    }
    *q = '\0';
  }
  return ret;
}

//将负载均衡枚举类型转成字符串类型,返回空间无需释放
char* lb_strategy_to_str(loadbalance_strategy_t strategy) {
  switch (strategy) {
    case GRPCLB:
      return ORIENTSEC_GRPC_LB_TYPE_GRPCLB;
      break;
    case PICK_FIRST:
      return ORIENTSEC_GRPC_LB_TYPE_PF;
      break;
    case ROUND_ROBIN:
      return ORIENTSEC_GRPC_LB_TYPE_RR;
      break;
    case WEIGHT_ROUND_ROBIN:
      return ORIENTSEC_GRPC_LB_TYPE_WRR;
      break;
    case CONSISTENT_HASH:
      return ORIENTSEC_GRPC_LB_TYPE_CH;
      break;
    default:
      return ORIENTSEC_GRPC_LB_TYPE_RR;
      break;
  }
}

//将负载均衡字符串类型转成枚举类型
loadbalance_strategy_t lb_strategy_from_str(char* strategy) {
  loadbalance_strategy_t ret = ROUND_ROBIN;
  if (strategy == NULL) {
    return ret;
  }
  if (0 == orientsec_stricmp(ORIENTSEC_GRPC_LB_TYPE_GRPCLB, strategy)) {
    ret = GRPCLB;
  } else if (0 == orientsec_stricmp(ORIENTSEC_GRPC_LB_TYPE_PF, strategy)) {
    ret = PICK_FIRST;
  } else if (0 == orientsec_stricmp(ORIENTSEC_GRPC_LB_TYPE_RR, strategy)) {
    ret = ROUND_ROBIN;
  } else if (0 == orientsec_stricmp(ORIENTSEC_GRPC_LB_TYPE_WRR, strategy)) {
    ret = WEIGHT_ROUND_ROBIN;
  } else if (0 == orientsec_stricmp(ORIENTSEC_GRPC_LB_TYPE_CH, strategy)) {
    ret = CONSISTENT_HASH;
  } else {
    ret = ROUND_ROBIN;
  }
  return ret;
}

//将故障切换枚举类型转成字符串类型，范湖空间无需释放
char* cluster_strategy_to_str(cluster_strategy_t strategy) {
  switch (strategy) {
    case FAILOVER:
      return ORIENTSEC_GRPC_CLUSTER_TYPE_FO;
      break;
    case FAILFAST:
      return ORIENTSEC_GRPC_CLUSTER_TYPE_FF;
      break;
    case FAILBACK:
      return ORIENTSEC_GRPC_CLUSTER_TYPE_FB;
      break;
    case FORKING:
      return ORIENTSEC_GRPC_CLUSTER_TYPE_FK;
      break;
    default:
      return ORIENTSEC_GRPC_CLUSTER_TYPE_FO;
      break;
  }
}

//将故障切换字符串类型转成枚举类型
cluster_strategy_t cluster_strategy_from_str(char* strategy) {
  cluster_strategy_t ret = FAILOVER;
  if (strategy == NULL) {
    return ret;
  }
  if (0 == orientsec_stricmp(ORIENTSEC_GRPC_CLUSTER_TYPE_FO, strategy)) {
    ret = FAILOVER;
  } else if (0 == orientsec_stricmp(ORIENTSEC_GRPC_CLUSTER_TYPE_FF, strategy)) {
    ret = FAILFAST;
  } else if (0 == orientsec_stricmp(ORIENTSEC_GRPC_CLUSTER_TYPE_FB, strategy)) {
    ret = FAILBACK;
  } else if (0 == orientsec_stricmp(ORIENTSEC_GRPC_CLUSTER_TYPE_FK, strategy)) {
    ret = FORKING;
  } else {
    ret = FAILOVER;
  }
  return ret;
}

provider_t* new_provider() {
  provider_t* provider = (provider_t*)malloc(sizeof(provider_t));
  memset(provider, 0, sizeof(provider_t));
  REINIT(provider->protocol);
  snprintf(provider->protocol, sizeof("grpc") + 1, "%s", "grpc");
  provider->username = NULL;
  provider->password = NULL;
  REINIT(provider->host);
  snprintf(provider->host, sizeof("127.0.0.1") + 1, "%s", "127.0.0.1");
  provider->port = 0;
  REINIT(provider->version);
  //provider->version = NULL;
  provider->group = NULL;
  provider->default_timeout = 1000;
  provider->default_reties = 2;
  provider->default_connections = 0;
  provider->default_requests = 0;
  provider->access_protected = false;
  provider->default_loadbalance = ROUND_ROBIN;
  provider->default_async = false;
  provider->token = NULL;
  provider->deprecated = false;
  provider->dynamic = true;
  provider->accesslog = false;
  provider->owner = NULL;
  provider->weight = 100;
  provider->default_cluster = FAILOVER;
  provider->application = NULL;
  provider->application_version = NULL;
  provider->organization = NULL;
  provider->environment = NULL;
  provider->module = NULL;
  provider->module_version = NULL;
  provider->anyhost = false;
  provider->sInterface = NULL;
  provider->methods = NULL;
  provider->pid = 0;
  snprintf(provider->side, sizeof("provider") + 1, "%s", "provider");
  provider->timestamp = 0;
  provider->grpc = NULL;
  provider->dubbo = NULL;
  provider->flag_call_failover = 0;
  provider->flag_in_blklist = 0;
  provider->ext_data = NULL;
  provider->project = NULL;
  provider->comm_owner = NULL;
  return provider;
}

void init_provider(provider_t* provider) {
  char buf[ORIENTSEC_GRPC_PROPERTY_VALUE_MAX_LEN] = {0};
  if (!provider) {
    return;
  }
  orientsec_grpc_properties_init();
  if (0 == orientsec_grpc_properties_get_value(ORIENTSEC_GRPC_PROPERTIES_COMMON_APP, NULL,
                                          buf)) {
    provider->application = gprc_strdup(buf);
  }
  REINIT(buf);
  //begin by liumin
  if (0 == orientsec_grpc_properties_get_value(ORIENTSEC_GRPC_PROPERTIES_COMMON_PROJECT, NULL, buf)) {
      provider->project = gprc_strdup(buf) ;
  } else {
    provider->project = ORIENTSEC_GRPC_PROPERTIES_COMMON_PROJECT_DEFAULT;
  }
  REINIT(buf);
  if (0 == orientsec_grpc_properties_get_value(ORIENTSEC_GRPC_PROPERTIES_COMMON_OWNER, NULL, buf)) {
    provider->comm_owner = gprc_strdup(buf);
  } else {
    provider->comm_owner = ORIENTSEC_GRPC_PROPERTIES_COMMON_OWNER_DEFAULT;
  }
  REINIT(buf);
  //end by liumin

  orientsec_grpc_properties_get_value(ORIENTSEC_GRPC_PROPERTIES_P_VERSION, NULL,
                                      provider->version);
  /*if (0 == orientsec_grpc_properties_get_value(ORIENTSEC_GRPC_PROPERTIES_P_VERSION, NULL,
                                          buf)) {
    provider->version = gprc_strdup(buf);
  }*/
  REINIT(buf);
  if (0 == orientsec_grpc_properties_get_value(ORIENTSEC_GRPC_PROPERTIES_P_MODULE, NULL,
                                          buf)) {
    provider->module = gprc_strdup(buf);
  }
  REINIT(buf);
  if (0 ==
      orientsec_grpc_properties_get_value(ORIENTSEC_GRPC_PROPERTIES_P_GROUP, NULL, buf)) {
    provider->group = gprc_strdup(buf);
  }
  REINIT(buf);
  if (0 == orientsec_grpc_properties_get_value(
               ORIENTSEC_GRPC_PROPERTIES_P_DEFAULT_TIMEOUT, NULL, buf)) {
    provider->default_timeout = atoi(buf);
  }
  REINIT(buf);
  if (0 == orientsec_grpc_properties_get_value(ORIENTSEC_GRPC_PROPERTIES_P_DEFAULT_RETIES,
                                          NULL, buf)) {
    provider->default_reties = atoi(buf);
  }
  REINIT(buf);
  if (0 == orientsec_grpc_properties_get_value(ORIENTSEC_GRPC_PROPERTIES_P_DEFAULT_CONN,
                                          NULL, buf)) {
    provider->default_connections = atoi(buf);
  }
  REINIT(buf);
  if (0 == orientsec_grpc_properties_get_value(ORIENTSEC_GRPC_PROPERTIES_P_DEFAULT_REQ,
                                          NULL, buf)) {
    provider->default_requests = atoi(buf);
  }
  REINIT(buf);
  if (0 == orientsec_grpc_properties_get_value(ORIENTSEC_GRPC_PROPERTIES_P_DEFAULT_LB,
                                          NULL, buf)) {
    provider->default_loadbalance = lb_strategy_from_str(buf);
  }
  REINIT(buf);
  if (0 == orientsec_grpc_properties_get_value(ORIENTSEC_GRPC_PROPERTIES_P_DEFAULT_ASYNC,
                                          NULL, buf)) {
    provider->default_async = (orientsec_stricmp(buf, "true") == 0) ? true : false;
  }
  REINIT(buf);
  if (0 ==
      orientsec_grpc_properties_get_value(ORIENTSEC_GRPC_PROPERTIES_P_TOKEN, NULL, buf)) {
    provider->token = gprc_strdup(buf);
  }

  REINIT(buf);
  if (0 == orientsec_grpc_properties_get_value(ORIENTSEC_GRPC_PROPERTIES_P_DEPRECATED,
                                          NULL, buf)) {
    provider->deprecated = (orientsec_stricmp(buf, "true") == 0) ? true : false;
  }

  REINIT(buf);
  if (0 == orientsec_grpc_properties_get_value(ORIENTSEC_GRPC_PROPERTIES_P_DYNAMIC, NULL,
                                          buf)) {
    provider->dynamic = (orientsec_stricmp(buf, "true") == 0) ? true : false;
  }

  REINIT(buf);
  if (0 == orientsec_grpc_properties_get_value(ORIENTSEC_GRPC_PROPERTIES_P_ACCESSLOG,
                                          NULL, buf)) {
    provider->accesslog = (orientsec_stricmp(buf, "true") == 0) ? true : false;
  }

  REINIT(buf);
  if (0 == orientsec_grpc_properties_get_value(ORIENTSEC_GRPC_PROPERTIES_P_OWNER, NULL, buf)) {
    provider->owner = gprc_strdup(buf);
  }

  REINIT(buf);
  if (0 == orientsec_grpc_properties_get_value(ORIENTSEC_GRPC_PROPERTIES_P_WEIGHT, NULL,
                                          buf)) {
    provider->weight = atoi(buf);
  }

  REINIT(buf);
  if (0 == orientsec_grpc_properties_get_value(
               ORIENTSEC_GRPC_PROPERTIES_P_DEFAULT_CLUSTER, NULL, buf)) {
    provider->default_cluster = cluster_strategy_from_str(buf);
  }

  REINIT(buf);
  if (0 == orientsec_grpc_properties_get_value(ORIENTSEC_GRPC_PROPERTIES_P_APP_VERSION,
                                          NULL, buf)) {
    provider->application_version = gprc_strdup(buf);
  }

  REINIT(buf);
  if (0 ==
      orientsec_grpc_properties_get_value(ORIENTSEC_GRPC_PROPERTIES_P_ORG, NULL, buf)) {
    provider->organization = gprc_strdup(buf);
  }

  REINIT(buf);
  if (0 ==
      orientsec_grpc_properties_get_value(ORIENTSEC_GRPC_PROPERTIES_P_ENV, NULL, buf)) {
    provider->environment = gprc_strdup(buf);
  }

  REINIT(buf);
  if (0 == orientsec_grpc_properties_get_value(ORIENTSEC_GRPC_PROPERTIES_P_MOD_VER, NULL,
                                          buf)) {
    provider->module_version = gprc_strdup(buf);
  }

  REINIT(buf);
  if (0 == orientsec_grpc_properties_get_value(ORIENTSEC_GRPC_PROPERTIES_P_ANYHOST, NULL,
                                          buf)) {
    provider->anyhost = (orientsec_stricmp(buf, "true") == 0) ? true : false;
  }

  REINIT(buf);
  if (0 ==
      orientsec_grpc_properties_get_value(ORIENTSEC_GRPC_PROPERTIES_P_DUBBO, NULL, buf)) {
    provider->dubbo = gprc_strdup(buf);
  }

  REINIT(buf);
  if (0 == orientsec_grpc_properties_get_value(
               ORIENTSEC_GRPC_PROPERTIES_P_ACCESS_PROTECTED, NULL, buf)) {
    provider->access_protected =
        (orientsec_stricmp(buf, "true") == 0) ? true : false;
  }
}

void free_provider_v2(provider_t* provider) {
  if (!provider) {
    return;
  }
  //FREE_PTR(provider->version);
  FREE_PTR(provider->group);
  FREE_PTR(provider->token);
  FREE_PTR(provider->owner);
  FREE_PTR(provider->application);
  FREE_PTR(provider->application_version);
  FREE_PTR(provider->organization);
  FREE_PTR(provider->environment);
  FREE_PTR(provider->module);
  FREE_PTR(provider->module_version);
  FREE_PTR(provider->sInterface);
  FREE_PTR(provider->methods);
  FREE_PTR(provider->grpc);
  FREE_PTR(provider->dubbo);
  //FREE_PTR(provider->project);
  //FREE_PTR(provider->comm_owner);
}

void free_provider(provider_t** p) {
  provider_t* provider = *p;
  if (!provider) {
    return;
  }
  free_provider_v2(provider);
  FREE_PTR(provider);
}

/*
 * 使用线程Sleep指定毫秒数。
 */
void orientsec_grpc_sleep_mills(int64_t millsecond) {
  gpr_timespec span;
  span.tv_sec = millsecond / 1000;
  span.tv_nsec = (millsecond % 1000) * 1000000;
  span.clock_type = GPR_TIMESPAN;

  uint64_t ret = 0;
  gpr_timespec now = gpr_now(GPR_CLOCK_REALTIME);
  gpr_timespec fur = gpr_time_add(now, span);
  gpr_sleep_until(fur);
}

void orientsec_grpc_side_set(int side) { orientsec_grpc_side_flag = side; }

FILE* p_orientsec_grpc_log;
int orientsec_grpc_log_open_flag = 0;

void orientsec_grpc_debug_log(const char* loginfo) {
  if (orientsec_grpc_log_open_flag != 1) {
    orientsec_grpc_log_open_flag = 1;
    if (orientsec_grpc_side_flag == 1) {  // provider log
      p_orientsec_grpc_log = fopen("C:\\log_provider.txt", "w+");
    } else {
      p_orientsec_grpc_log = fopen("C:\\log_consumer.txt", "w+");
    }
  }
  fprintf(p_orientsec_grpc_log, "%s", loginfo);
  fflush(p_orientsec_grpc_log);
}
int orientsec_stricmp(const char* a, const char* b) {
  int ca, cb;
  do {
    ca = tolower(*a);
    cb = tolower(*b);
    ++a;
    ++b;
  } while (ca == cb && ca && cb);
  return ca - cb;
}
int orientsec_grpc_threadid_get() {
#ifdef WIN64
  return (int)GetCurrentThreadId();
#endif

#ifdef WIN32
  return (int)GetCurrentThreadId();
#else
  return 0;
#endif
}
