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
 *    zookeeper服务注册操作函数实现
 */

#include "orientsec_grpc_utils.h"
#include "registry_utils.h"
#include "zk_registry_service.h"
#include"registry_contants.h"
#include "orientsec_grpc_properties_tools.h"
#include <string.h>
#include <grpc/support/log.h>
#include <grpc/support/alloc.h>
#include <grpc/support/log.h>
#include <grpc/support/sync.h>
#include <src/core/lib/gpr/spinlock.h>
#include <src/core/lib/gpr/string.h>
#include <string.h>
#include <zookeeper.h>
#include <zookeeper_log.h>
#include "orientsec_grpc_properties_tools.h"
#include "orientsec_grpc_utils.h"
#include "registry_contants.h"
#include "registry_utils.h"
#include "zookeeper.jute.h"

//----begin---add for encryption by jianbin
#include "sha1.h"    //zookeeper 加密算法
#include "base64.h"  //zookeeper 加密算法
#include "des.h"     //读取配置文件中密文字符并解析出明文
//----end----


//每个url对应的订阅函数列表（监听器列表，当url下的子节点变化时，函数列表会被调用）
typedef struct _zk_notify_node zk_notify_node;
struct _zk_notify_node {
  gpr_mu mu;
  gpr_spinlock checker_notify_mu;
  registry_notify_f notify_func;
  zk_notify_node* next;
  int live;  //是否有效，0： 有效，非0：已设置为删除，等待被删除
};

#define zk_notify_node_len (sizeof(struct _zk_notify_node))

// 系统中包含的订阅url链表，每订阅一个node，生成一个zk_listener_list结构，
typedef struct _zk_listener_node zk_listener_node;
struct _zk_listener_node {
  //对本节点对应的订阅函数列表操作进行同步
  gpr_mu mu;
  gpr_spinlock checker_listener_mu;
  char url_full_string[ORIENTSEC_GRPC_URL_MAX_LEN];
  zk_notify_node* notify_list_head;
  zk_listener_node* next;
  int live;  //是否有效，0： 有效，非0：已设置为删除，等待被删除
};

#define zk_listener_node_len (sizeof(struct _zk_listener_node))

//保存注册url的链表
typedef struct _zk_registy_url_node zk_registy_url_node;
struct _zk_registy_url_node {
  gpr_mu mu;
  gpr_spinlock checker_registry_mu;
  char* urlStr;
  zk_registy_url_node* next;
  int live;  //是否有效，0： 有效，非0：已设置为删除，等待被删除
};

#define zk_registy_url_node_len (sizeof(struct _zk_registy_url_node))

typedef enum _zk_connect_state {
  ZK_INIT,              //初始化连接
  ZK_CONNECTING,        //正在连接
  ZK_CONNECTED,         //初始连接建立
  ZK_LOSTING,           //正在丢失连接
  ZK_RECONNECTING,      //正在重连
  ZK_RECONNECTED,       //闪断重连已成功
  ZK_MANU_RECONNECTED,  //闪断重连已成功
  ZK_MANU_CLOSED,       //手动关闭连接
  ZK_DISCONNECTED       //断开连接
} zk_connect_state;

// typedef struct {
//  char username[128];
//  char passwd[128];
//} zk_acl;

//每建立一个zk连接，生成一个如下结构对象，
typedef struct _zk_connection_t zk_connection_t;
struct _zk_connection_t {
  zhandle_t* zh;                                 // zk连接句柄
  clientid_t* clientid;
  zk_listener_node* listener_list_head;          //该连接上的订阅链表
  char zk_address[ORIENTSEC_GRPC_URL_MAX_LEN];   // zk连接地址
  // zk_acl* acl_info;
  zk_connection_t* next;
  zk_connect_state connecte_state;               // zk连接状态
  zk_registy_url_node* url_list_head;            //注册的url链表
  int reconnectCount;                            //重连次数
};
#define zk_connection_t_len (sizeof(struct _zk_connection_t))

static zk_connection_t zk_connection_list_head = {NULL, 0,       NULL, "",
                                                  NULL, ZK_INIT, NULL, 0};
static zk_connection_t* p_zk_connection_list_head = &zk_connection_list_head;

// zookeeper acl name and passwd
static char zk_acl_name[256] = {0};
static char zk_acl_pwd[256] = {0};
static bool g_acl_flag = false;

/* Protects listener_queue */
static gpr_mu g_conn_mu;
/* Allow only one access listener queue at once */
static gpr_spinlock g_checker_conn_mu = GPR_SPINLOCK_STATIC_INITIALIZER;

static bool g_initialized = false;

bool isZkConnected(zk_connection_t* conn) {
  if (conn && ((ZK_CONNECTED == conn->connecte_state) ||
               (ZK_RECONNECTED == conn->connecte_state) ||
               (ZK_MANU_RECONNECTED == conn->connecte_state))) {
    return true;
  }
  return false;
}

//分配注册url链表节点空间并初始化
zk_registy_url_node* new_zk_registy_url_node(url_t* url, bool bHead) {
  zk_registy_url_node* new_node =
      (zk_registy_url_node*)gpr_zalloc(zk_registy_url_node_len);
  if (new_node) {
    if (bHead) {
      gpr_mu_init(&new_node->mu);
      new_node->checker_registry_mu = GPR_SPINLOCK_INITIALIZER;
    }
    new_node->live = 0;
    new_node->urlStr = NULL;
    if (url != NULL) {
      new_node->urlStr = url_to_string(url);
    }
    new_node->next = NULL;
  } else {
    gpr_log(GPR_ERROR, "alloc zk_registy_url_node failed");
  }
  return new_node;
}

//在连接对象中查找注册的url对象
zk_registy_url_node* lookup_registry_url_node(zk_connection_t* conn,
                                              url_t* url) {
  zk_registy_url_node* p = NULL;
  char* urlStr = NULL;
  if (!conn || !url) {
    return NULL;
  }
  if (gpr_spinlock_trylock(&conn->url_list_head->checker_registry_mu)) {
    gpr_mu_lock(&conn->url_list_head->mu);
    p = conn->url_list_head->next;
    urlStr = url_to_string(url);
    while (p) {
      if (0 == strcmp(urlStr, p->urlStr)) {
        break;
      }
      p = p->next;
    }
    FREE_PTR(urlStr);
    gpr_mu_unlock(&conn->url_list_head->mu);
    gpr_spinlock_unlock(&conn->url_list_head->checker_registry_mu);
  }

  return p;
}

//根据注册url地址查找url连接对象，如不存在则新分配内存并加入到链接队列中
zk_registy_url_node* get_registry_url_node(zk_connection_t* conn, url_t* url) {
  zk_registy_url_node* p = NULL;
  zk_registy_url_node* new_node = lookup_registry_url_node(conn, url);
  if (!new_node) {
    new_node = new_zk_registy_url_node(url, false);
    if (!new_node) {
      gpr_log(GPR_ERROR, "alloc zk_registy_url_node failed");
      return new_node;
    }
    if (gpr_spinlock_trylock(&conn->url_list_head->checker_registry_mu)) {
      gpr_mu_lock(&conn->url_list_head->mu);

      p = conn->url_list_head;
      new_node->next = p->next;
      conn->url_list_head->next = new_node;

      gpr_mu_unlock(&conn->url_list_head->mu);
      gpr_spinlock_unlock(&conn->url_list_head->checker_registry_mu);
    }
  }
  return new_node;
}

//删除某连接上的url链表节点
void remove_registry_url_node(zk_connection_t* conn, url_t* url) {
  zk_registy_url_node *p = NULL, *p1 = NULL;
  char* urlStr = url_to_string(url);
  if (!conn || !url) {
    return;
  }
  p = conn->url_list_head;
  p1 = p->next;

  while (p1) {
    if (0 == strcmp(p1->urlStr, urlStr)) {
      if (gpr_spinlock_trylock(&conn->url_list_head->checker_registry_mu)) {
        gpr_mu_lock(&conn->url_list_head->mu);

        p->next = p1->next;
        FREE_PTR(p1->urlStr);
        FREE_PTR(p1);

        gpr_mu_unlock(&conn->url_list_head->mu);
        gpr_spinlock_unlock(&conn->url_list_head->checker_registry_mu);
      }
      FREE_PTR(urlStr);
      return;
    }
    p = p1;
    p1 = p1->next;
  }
}

//分配订阅函数链表节点空间并初始化
zk_notify_node* new_zk_notify_node(registry_notify_f notify_func, bool bHead) {
  zk_notify_node* new_node = (zk_notify_node*)gpr_zalloc(zk_notify_node_len);
  if (new_node) {
    if (bHead) {
      gpr_mu_init(&new_node->mu);
      new_node->checker_notify_mu = GPR_SPINLOCK_INITIALIZER;
    }
    new_node->live = 0;
    new_node->next = NULL;
    new_node->notify_func = notify_func;
  } else {
    gpr_log(GPR_ERROR, "alloc zk_notify_node failed");
  }
  return new_node;
}

//移除指定url上某个订阅函数
void remove_zk_notify_node(zk_listener_node* p_listener_node,
                           registry_notify_f notify_func) {
  zk_notify_node *p0, *p1;
  p0 = p_listener_node->notify_list_head;
  p1 = p0->next;
  if (!p_listener_node || !notify_func) {
    return;
  }
  if (gpr_spinlock_trylock(
          &p_listener_node->notify_list_head->checker_notify_mu)) {
    gpr_mu_lock(&p_listener_node->notify_list_head->mu);
    while (p1) {
      if (p1->notify_func == notify_func) {
        p0->next = p1->next;
        FREE_PTR(p1);
        break;
      }
      p0 = p1;
      p1 = p0->next;
    }
    gpr_mu_unlock(&p_listener_node->notify_list_head->mu);
    gpr_spinlock_unlock(&p_listener_node->notify_list_head->checker_notify_mu);
  }
}

//释放某个url上所有的订阅函数链表节点
void release_zk_listener_node_notify(zk_listener_node* p_listener_node) {
  zk_notify_node *p0, *p1;
  p0 = p_listener_node->notify_list_head;
  p1 = p0->next;
  if (!p_listener_node) {
    return;
  }
  if (gpr_spinlock_trylock(
          &p_listener_node->notify_list_head->checker_notify_mu)) {
    gpr_mu_lock(&p_listener_node->notify_list_head->mu);
    while (p1) {
      p0->next = p1->next;
      FREE_PTR(p1);
      p1 = p0->next;
    }
    gpr_mu_unlock(&p_listener_node->notify_list_head->mu);
    gpr_spinlock_unlock(&p_listener_node->notify_list_head->checker_notify_mu);
  }
}

//调用指定url的所有订阅函数
void schedule_notify_func(zk_listener_node* p_listener_node, url_t* urls,
                          int url_num) {
  zk_notify_node* p0 = NULL;
  if (!p_listener_node) {
    return;
  }
  p0 = p_listener_node->notify_list_head->next;
  if (gpr_spinlock_trylock(
          &p_listener_node->notify_list_head->checker_notify_mu)) {
    gpr_mu_lock(&p_listener_node->notify_list_head->mu);
    while (p0) {
      if (0 == p0->live) {
        (p0->notify_func)(urls, url_num);
      }
      p0 = p0->next;
    }
    gpr_mu_unlock(&p_listener_node->notify_list_head->mu);
    gpr_spinlock_unlock(&p_listener_node->notify_list_head->checker_notify_mu);
  }
}

//订阅列表操作函数，创建列表节点，分配空间。
zk_listener_node* new_zk_listener_node(bool bHead) {
  zk_listener_node* new_node =
      (zk_listener_node*)gpr_zalloc(zk_listener_node_len);
  if (new_node) {
    if (bHead) {
      gpr_mu_init(&new_node->mu);
      new_node->checker_listener_mu = GPR_SPINLOCK_INITIALIZER;
    }
    new_node->live = 0;
    new_node->next = NULL;
    new_node->notify_list_head = new_zk_notify_node(NULL, true);
    GPR_ASSERT(NULL != new_node->notify_list_head);
    new_node->url_full_string[0] = '\0';
  } else {
    gpr_log(GPR_ERROR, "alloc zk_listener_node failed");
  }
  return new_node;
}

//根据url串查找对应的监听器列表节点
zk_listener_node* lookup_listener_node(zk_connection_t* conn, char* url) {
  zk_listener_node* p = NULL;
  if (!conn || !url) {
    return NULL;
  }
  p = conn->listener_list_head->next;

  while (p) {
    if (0 == orientsec_stricmp(url, p->url_full_string)) {
      return p;
    }
    p = p->next;
  }
  return NULL;
}

//往url订阅列表中插入新节点，新节点位于head节点之后。
zk_listener_node* add_zk_listener_list_node(zk_connection_t* conn, char* url) {
  zk_listener_node* new_node = NULL;
  int url_len = 0;
  if (!conn || !url) {
    gpr_log(GPR_INFO, "[zk_listener_list_add_node] param url is null");
    return new_node;
  }
  if (gpr_spinlock_trylock(&conn->listener_list_head->checker_listener_mu)) {
    gpr_mu_lock(&conn->listener_list_head->mu);
    new_node = new_zk_listener_node(false);
    if (new_node) {
      url_len = strlen(url);
      url_len = (url_len >= ORIENTSEC_GRPC_URL_MAX_LEN) ? (ORIENTSEC_GRPC_URL_MAX_LEN - 1)
                                                   : url_len;
      snprintf(new_node->url_full_string, url_len + 1, "%s", url);
      new_node->next = conn->listener_list_head->next;
      conn->listener_list_head->next = new_node;
    }
    gpr_mu_unlock(&conn->listener_list_head->mu);
    gpr_spinlock_unlock(&conn->listener_list_head->checker_listener_mu);
  }
  return new_node;
}

//根据url串查找对应的监听器列表节点,如果列表中
//无url对应的监听器，则新建节点并返回。
zk_listener_node* get_zk_listener_node(zk_connection_t* conn, char* url) {
  zk_listener_node* new_node = NULL;

  new_node = lookup_listener_node(conn, url);
  if (!new_node) {
    new_node = add_zk_listener_list_node(conn, url);
  }

  return new_node;
}

//在订阅函数链表中查找指定的订阅函数
zk_notify_node* lookup_notify_node(zk_listener_node* p_listener_node,
                                   registry_notify_f notify_func) {
  zk_notify_node* p_notify_node = p_listener_node->notify_list_head->next;
  if (!p_listener_node) {
    return NULL;
  }
  while (p_notify_node) {
    if (p_notify_node->notify_func == notify_func) {
      return p_notify_node;
    }
    p_notify_node = p_notify_node->next;
  }
  return NULL;
}

//将订阅函数加到指定url 链表中
zk_notify_node* get_zk_listener_notify_node(zk_connection_t* conn, char* url,
                                            registry_notify_f notify_func) {
  zk_listener_node* p_listener_node = get_zk_listener_node(conn, url);
  zk_notify_node* p_notify_node = NULL;
  if (p_listener_node) {
    p_notify_node = lookup_notify_node(p_listener_node, notify_func);
    if (!p_notify_node) {
      p_notify_node = new_zk_notify_node(notify_func, false);
      if (p_notify_node) {
        if (gpr_spinlock_trylock(
                &p_listener_node->notify_list_head->checker_notify_mu)) {
          gpr_mu_lock(&p_listener_node->notify_list_head->mu);

          p_notify_node->next = p_listener_node->notify_list_head->next;
          p_listener_node->notify_list_head->next = p_notify_node;

          gpr_mu_unlock(&p_listener_node->notify_list_head->mu);
          gpr_spinlock_unlock(
              &p_listener_node->notify_list_head->checker_notify_mu);
        }
      }
    }
  }
  return p_notify_node;
}

//移除某个url订阅链表
void remove_zk_listener_node(zk_connection_t* conn, char* url) {
  zk_listener_node *p0 = NULL, *p1 = NULL;
  p0 = conn->listener_list_head;
  p1 = p0->next;
  while (p1) {
    if (0 == orientsec_stricmp(url, p1->url_full_string)) {
      break;
    }
    p0 = p1;
    p1 = p1->next;
  }
  if (p1) {
    release_zk_listener_node_notify(p1);
    p0->next = p1->next;
    FREE_PTR(p1);
  }
  return;
}

//列表初始化
void init_zk_conn_list() {
  if (!g_initialized) {
    gpr_mu_init(&g_conn_mu);
    zoo_set_debug_level(
        ZOO_LOG_LEVEL_ERROR);  //设置日志级别,避免出现一些其他信息
    g_initialized = true;
  }
}

// 分配连接对象内存
zk_connection_t* zk_connection_t_new(char* address) {
  zk_connection_t* new_node = (zk_connection_t*)gpr_zalloc(zk_connection_t_len);
  int address_len = 0;
  if (new_node) {
    new_node->zh = NULL;
    new_node->clientid = 0;
    new_node->next = NULL;
    new_node->zk_address[0] = '\0';
    // new_node->acl_info = 0;
    new_node->connecte_state = ZK_INIT;
    new_node->url_list_head = new_zk_registy_url_node(NULL, true);
    new_node->listener_list_head = new_zk_listener_node(true);
    new_node->reconnectCount = 0;
    if (address) {
      address_len = strlen(address);
      snprintf(new_node->zk_address,
               address_len > ORIENTSEC_GRPC_URL_MAX_LEN ? ORIENTSEC_GRPC_URL_MAX_LEN
                                                   : address_len + 1,
               "%s", address);
    }
  } else {
    gpr_log(GPR_ERROR, "alloc zk_listener_node failed");
  }
  return new_node;
}

//根据地址查找zk连接对象，如不存在则分配新内存
zk_connection_t* get_zk_connection_t(char* zk_address) {
  zk_connection_t* conn = p_zk_connection_list_head->next;
  if (!zk_address) {
    return NULL;
  }
  while (conn) {
    if (0 == orientsec_stricmp(zk_address, conn->zk_address)) {
      return conn;
    }
    conn = conn->next;
  }
  conn = zk_connection_t_new(zk_address);
  if (conn && gpr_spinlock_trylock(&g_checker_conn_mu)) {
    gpr_mu_lock(&g_conn_mu);

    conn->next = p_zk_connection_list_head->next;
    p_zk_connection_list_head->next = conn;

    gpr_mu_unlock(&g_conn_mu);
    gpr_spinlock_unlock(&g_checker_conn_mu);
  }

  return conn;
}

//根据连接句柄查找对应的连接结构体
zk_connection_t* find_zk_connection_by_handle(zhandle_t* handle) {
  zk_connection_t* conn = NULL;
  if (gpr_spinlock_trylock(&g_checker_conn_mu)) {
    gpr_mu_lock(&g_conn_mu);
    conn = p_zk_connection_list_head->next;
    while (conn) {
      if (conn->zh == handle) {
        break;
      }
      conn = conn->next;
    }
    gpr_mu_unlock(&g_conn_mu);
    gpr_spinlock_unlock(&g_checker_conn_mu);
  }
  return conn;
}

//从连接队列中删除某个连接
void release_zk_conneciton(zk_connection_t* conn) {
  zk_connection_t *p0 = NULL, *p1 = NULL;
  zk_listener_node *listner_node = NULL, *listner_node_0 = NULL;
  zk_notify_node *notify_node = NULL, *notify_node_0 = NULL;
  zk_registy_url_node *registry_url_node = NULL, *registry_url_node_0 = NULL;
  if (!conn) return;
  if (gpr_spinlock_trylock(&g_checker_conn_mu)) {
    gpr_mu_lock(&g_conn_mu);
    p0 = p_zk_connection_list_head;
    p1 = p0->next;
    while (p1) {
      if (p1 == conn) {
        p0->next = p1->next;
        break;
      }
      p0 = p1;
      p1 = p1->next;
    }
    if (p1) {
      //释放listener链表节点空间
      if (gpr_spinlock_trylock(&p1->listener_list_head->checker_listener_mu)) {
        gpr_mu_lock(&p1->listener_list_head->mu);

        listner_node = p1->listener_list_head;
        listner_node_0 = listner_node->next;
        while (listner_node_0) {
          listner_node->next = listner_node_0->next;
          //释放notify链表节点空间
          if (gpr_spinlock_trylock(
                  &listner_node_0->notify_list_head->checker_notify_mu)) {
            gpr_mu_lock(&listner_node_0->notify_list_head->mu);

            notify_node = listner_node_0->notify_list_head;
            notify_node_0 = notify_node->next;
            while (notify_node_0) {
              notify_node->next = notify_node_0->next;
              FREE_PTR(notify_node_0);
              notify_node_0 = notify_node->next;
            }

            gpr_mu_unlock(&listner_node_0->notify_list_head->mu);
            gpr_spinlock_unlock(
                &listner_node_0->notify_list_head->checker_notify_mu);
          }

          FREE_PTR(listner_node_0->notify_list_head);

          FREE_PTR(listner_node_0);
          listner_node_0 = listner_node->next;
        }
        gpr_mu_unlock(&p1->listener_list_head->mu);
        gpr_spinlock_unlock(&p1->listener_list_head->checker_listener_mu);
      }
      FREE_PTR(p1->listener_list_head);

      //释放注册的url结构体对象
      if (gpr_spinlock_trylock(&p1->url_list_head->checker_registry_mu)) {
        gpr_mu_lock(&p1->url_list_head->mu);

        registry_url_node = p1->url_list_head;
        registry_url_node_0 = registry_url_node->next;
        while (registry_url_node_0) {
          registry_url_node->next = registry_url_node_0->next;
          FREE_PTR(registry_url_node_0->urlStr);
          FREE_PTR(registry_url_node_0);
          registry_url_node_0 = registry_url_node->next;
        }

        gpr_mu_unlock(&p1->url_list_head->mu);
        gpr_spinlock_unlock(&p1->url_list_head->checker_registry_mu);
      }
      FREE_PTR(p1->url_list_head);

      FREE_PTR(p1);
    }
    gpr_mu_unlock(&g_conn_mu);
    gpr_spinlock_unlock(&g_checker_conn_mu);
  }
}

extern char* g_orientsec_grpc_common_root_directory;
char* zk_get_service_path(url_t* url) {
  char* intf = url_get_service_interface(url);
  int intf_len = strlen(intf);
  int root_len = strlen(ORIENTSEC_GRPC_REGISTRY_ROOT);
  int sep_len = strlen(ORIENTSEC_GRPC_REGISTRY_SEPARATOR);
  char* buf = NULL;
  if (!url || !intf) {
    gpr_log(GPR_ERROR, "[zk_get_service_path] url is null or url.intf is null");
    return NULL;
  }
  buf = (char*)malloc(sizeof(char) * (root_len + sep_len + intf_len + 1));
  if (buf) {
    snprintf(buf, (root_len + sep_len + intf_len + 1), "%s%s%s",
             ORIENTSEC_GRPC_REGISTRY_ROOT, ORIENTSEC_GRPC_REGISTRY_SEPARATOR, intf);
  }
  FREE_PTR(intf);
  return buf;
}

char* zk_get_category_path(url_t* url) {
  char* svc = zk_get_service_path(url);
  int sep_len = strlen(ORIENTSEC_GRPC_REGISTRY_SEPARATOR);
  int svc_len = strlen(svc);
  char* buf = NULL;
  char* category = url_get_parameter(url, ORIENTSEC_GRPC_CATEGORY_KEY, NULL);
  int category_len = strlen(category);
  if (!url || !svc || !category) {
    FREE_PTR(svc);
    FREE_PTR(category);
    gpr_log(GPR_ERROR,
            "[zk_get_service_path] url is null or service_path is null");
    return NULL;
  }
  buf = (char*)malloc(sizeof(char) * (svc_len + sep_len + category_len + 1));
  if (buf) {
    snprintf(buf, (svc_len + sep_len + category_len + 1), "%s%s%s", svc,
             ORIENTSEC_GRPC_REGISTRY_SEPARATOR, category);
  }
  FREE_PTR(svc);
  FREE_PTR(category);
  return buf;
}

// get url and url encode
char* zk_get_url_path(url_t* url) {
  char* categoryPath = zk_get_category_path(url);
  char* url_string = url_to_string(url);
  char* url_encode_string = url_encode(url_string);
  char* buf = NULL;
  int buf_len = 0;
  int category_len = 0;
  size_t url_encode_string_len = 0;
  int sep_len = strlen(ORIENTSEC_GRPC_REGISTRY_SEPARATOR);
  if (!categoryPath || !url_string || !url_encode_string) {
    FREE_PTR(categoryPath);
    FREE_PTR(url_string);
    FREE_PTR(url_encode_string);
    gpr_log(
        GPR_INFO,
        "[zk_get_url_path] category path is null or convert to string failed");
    return NULL;
  }
  category_len = strlen(categoryPath);
  url_encode_string_len = strlen(url_encode_string);
  buf_len = category_len + sep_len + url_encode_string_len;
  buf = (char*)gpr_zalloc(buf_len + 1);
  if (!buf) {
    FREE_PTR(categoryPath);
    FREE_PTR(url_string);
    FREE_PTR(url_encode_string);
    gpr_log(GPR_ERROR, "[zk_get_url_path] alloc buffer failed");
    return NULL;
  }
  snprintf(buf, buf_len + 1, "%s%s%s", categoryPath,
           ORIENTSEC_GRPC_REGISTRY_SEPARATOR, url_encode_string);

  FREE_PTR(categoryPath);
  FREE_PTR(url_string);
  FREE_PTR(url_encode_string);
  return buf;
}

//DES algorithm decrypt
static char* get_des_plain(char* sourceData){

  uint8_t mi[32] = {0};
  int destSize;
  char* pKey = "OrientZQ";
  char key[8];
  StrToHex(mi, sourceData, 16);
  memcpy(key, pKey, 8);
  int sourceSize = strlen(sourceData)/2;
  char* dest = DES_Decrypt(mi, sourceSize, key, &destSize);
  return dest;
}

// 读配置文件中的acl认证用户名密码
static void get_acl_info() {
  bool user_flag = false;
  bool pwd_flag = false;
  char zk_acl_pwd_enc[256] = {0};
  const char* pwd = NULL;

  if (0 == orientsec_grpc_properties_get_value(ORIENTSEC_GRPC_ZK_ACL_USERNAME, NULL,
                                          zk_acl_name)) {
    if (zk_acl_name == NULL || strlen(zk_acl_name) == 0) {
      user_flag = false;
    } else {
      user_flag = true;
    }
  }
  if (0 == orientsec_grpc_properties_get_value(ORIENTSEC_GRPC_ZK_ACL_PASSWORD, NULL,
                                          zk_acl_pwd_enc)) {
    if (zk_acl_pwd_enc == NULL || strlen(zk_acl_pwd_enc) == 0) {
      pwd_flag = false;
    } else {
      pwd_flag = true;
      pwd = get_des_plain(zk_acl_pwd_enc);
      strncpy(zk_acl_pwd, pwd, strlen(pwd) + 1);

    }
  }

  if (user_flag && pwd_flag) {
    g_acl_flag = true;    //设置全局acl开关，用于判断acl是否开启
  }
}

//拼接 name 和 pwd
char* combine_name_pwd(char* name, char* pwd, size_t* len) {
  char szUserPwd[ORIENTSEC_GRPC_PROPERTY_VALUE_MAX_LEN] = {0};
  snprintf(szUserPwd, strlen(name) + strlen(pwd) + 2, "%s:%s", name, pwd);
  *len = strlen(szUserPwd);

  return szUserPwd;
}

//加密算法
char* encrypt(char* plain, size_t len) {
  // sha1 algorithm
  char result[21] = {0};
  // calculate hash
  SHA1(result, plain, len);
  // base64 encrypt
  int size = sizeof(result) - 1;
  return base64_encode(result, size);
}
// ACL参数设置
static char* get_acl_param() {
  //加密user:pwd，使用base64(sha1(username:password))
  size_t length = 0;
  char* szEncUserPwd = NULL;  // 28 byte
  char szDigestIds[64] = {0};

  szEncUserPwd =
      encrypt(combine_name_pwd(zk_acl_name, zk_acl_pwd, &length), length);

  snprintf(szDigestIds, sizeof(szDigestIds), "%s:%s", zk_acl_name,
           szEncUserPwd);
  return (char*)szDigestIds;
}

//在zookeeper 上创建节点
void zk_create_node(zk_connection_t* conn, char* path, bool dynamic) {
  zhandle_t* zkhandle = conn->zh;
  char buf[ORIENTSEC_GRPC_PATH_MAX_LEN] = {0};
  char* p = NULL;
  int ret = 0;
  if (!path || 0 == strlen(path)) {
    return;
  }
  p = strrchr(path, '/');
  if (p) {
    snprintf(buf, p - path + 1, "%s", path);
    zk_create_node(conn, buf, false);
  }

  if (g_acl_flag) {
  
    // 1.create ACL \ZOO_CREATOR_ALL_ACL
    // 2.zoo_add_auth 应用程序使用zoo_add_auth方法来向服务器认证自己

    size_t length = 0;
    char* plain = combine_name_pwd(zk_acl_name, zk_acl_pwd, &length);
    ret = zoo_add_auth(zkhandle, "digest", plain, length, 0, 0);
    if (ZOK != ret)
      printf("Auth failed,zoo_add_auth = %d\n", ret);

    // enc格式digest ID为 userName:base64(sha1(userName:password))
    char enc[64] = {0};
    strcpy(enc, get_acl_param()); 

    //设置ACL_vector
    struct ACL _CREATOR_ALL_ACL_ACL[] = {{0x1f, {"digest", enc}}};
    struct ACL_vector ZOO_CREATOR_ALL_ACL = {1, _CREATOR_ALL_ACL_ACL};

    if (dynamic) {
      ret = zoo_create(zkhandle, path, NULL, -1, &ZOO_CREATOR_ALL_ACL, ZOO_EPHEMERAL, NULL, 0);
   
    } else {
      ret = zoo_create(zkhandle, path, NULL, -1, &ZOO_CREATOR_ALL_ACL, 0, NULL, 0);
    }
    //reset enc
    memset(enc, 0, 64);
    memset(_CREATOR_ALL_ACL_ACL, 0, sizeof(_CREATOR_ALL_ACL_ACL));
    //memset(ZOO_CREATOR_ALL_ACL, 0, sizeof(ZOO_CREATOR_ALL_ACL));

      if (ZOK == ret) {
      gpr_log(GPR_INFO, "create zk acl node success[%s]", path);
    } else if (ZNODEEXISTS == ret) {
      gpr_log(GPR_INFO, "zk acl node %s exists", path);
    } else {
      gpr_log(GPR_ERROR, "create zk acl node faild[%s],reason=[%s],error code=%d",
              path, zerror(ret), ret);
    }
  }else {// acl 没有开启
    if (dynamic) {
      ret = zoo_create(zkhandle, path, NULL, -1, &ZOO_OPEN_ACL_UNSAFE,
                       ZOO_EPHEMERAL, NULL, 0);
    } else {
      ret = zoo_create(zkhandle, path, NULL, -1, &ZOO_OPEN_ACL_UNSAFE, 0, NULL, 0);
    }
    if (ZOK == ret) {
      gpr_log(GPR_INFO, "create zk node success[%s]", path);
    } else if (ZNODEEXISTS == ret) {
      gpr_log(GPR_INFO, "zk node %s exists", path);
    } else {
      gpr_log(GPR_ERROR, "create zk node faild[%s],reason=[%s],error code=%d",
              path, zerror(ret), ret);
    }
  }
}

//在zk上删除指定节点
int zk_delete_node(zk_connection_t* conn, char* path) {
  zhandle_t* zkhandle = conn->zh;
  int ret = 0;
  ret = zoo_delete(zkhandle, path, -1);
  if (ZOK == ret) {
    gpr_log(GPR_INFO, "delete zk node success[%s]", path);
  } else {
    gpr_log(GPR_ERROR, "delete zk node faild[%s],reason=[%s],error code=%d",
            path, zerror(ret), ret);
  }
  return ret;
}

void start_zk_connect(zk_connection_t* conn, int keepSession);
void registy_recover(zk_connection_t* conn);

// zk连接状态监控，重连时需要恢复注册和订阅
void zk_conn_watcher_g(zhandle_t* zh, int type, int state, const char* path,
                       void* watcherCtx) {
  int timeout = 5000;  // 5000ms
  char* host = NULL;
  char* key = NULL;
  char* p = NULL;
  int key_len = 0;
  zk_connection_t* conn = (zk_connection_t*)zoo_get_context(zh);

  if (ZOO_SESSION_EVENT == type && ZOO_CONNECTED_STATE == state) {
    if (ZK_RECONNECTING == conn->connecte_state) {
      conn->connecte_state = ZK_RECONNECTED;  //闪断重连成功
    } else if (ZK_DISCONNECTED == conn->connecte_state) {
      conn->connecte_state = ZK_MANU_RECONNECTED;  //手动重连成功
    } else {
      conn->connecte_state = ZK_CONNECTED;  //初始已连接状态
      conn->clientid = zoo_client_id(zh);
    }

  } else if (ZOO_SESSION_EVENT == type && ZOO_CONNECTING_STATE == state) {
    conn->connecte_state = ZK_RECONNECTING;
    //重连中,可能重连成功，也可能失败，失败之后状态为ZOO_EXPIRED_SESSION_STATE,否则为ZOO_CONNECTED_STATE
  } else if (ZOO_SESSION_EVENT == type && ZOO_EXPIRED_SESSION_STATE == state) {
    //会话超时，断开连接,需要手动开启重连，重连成功后恢复注册与订阅
    //重连
    if (ZK_DISCONNECTED == conn->connecte_state) {
      start_zk_connect(conn, 0);
    } else {  //第一次试图恢复原session，恢复失败后新建session
      start_zk_connect(conn, 1);
    }
    conn->connecte_state = ZK_DISCONNECTED;

    conn->reconnectCount++;
  }

  if (ZK_MANU_RECONNECTED == conn->connecte_state) {
    //重连成功，恢复注册与订阅
    registy_recover(conn);
  }
}

void construct_empty_schema_url(const char* service_name, url_t* url) {
  if (!service_name || !url) {
    return;
  }
  url->protocol = gprc_strdup(ORIENTSEC_GRPC_EMPTY_PROTOCOL);
  url->host = gprc_strdup(ORIENTSEC_GRPC_ANYHOST_VALUE);
  url->path = gprc_strdup(service_name);
  url->params_num = 0;
  return;
}

//节点监控函数
void zk_node_watcher_g(zhandle_t* zh, int type, int state, const char* path,
                       void* watcherCtx) {
  zk_notify_node* p_notify_node = NULL;
  struct String_vector childs;
  url_t* urls = NULL;
  int urls_num = 0;
  size_t i = 0;
  int ret = 0;
  char buf[ORIENTSEC_GRPC_PATH_MAX_LEN] = {0};
  zk_connection_t* conn = (zk_connection_t*)watcherCtx;
  zk_listener_node* p_listener_node = lookup_listener_node(conn, path);
  if (!conn || !p_listener_node) {
    return;
  }
  if (ZOO_CHILD_EVENT != type) {
    return;
  }
  ret = zoo_wget_children(conn->zh, path, zk_node_watcher_g, (void*)conn,
                          &childs);
  if (ZOK == ret) {
    /*对childs分析，删除无效节点by lm*/
    url_t* myurl;
    int url_valid = childs.count;
    int url_curse = 0;
    urls_num = childs.count;
    myurl = (url_t*)gpr_zalloc(1 * sizeof(url_t));
    if (urls_num ==
        0)  //对于子节点为空的情形，返回一个empty://0.0.0.0/service_name格式url
    {
    } else {
      urls = (url_t*)gpr_zalloc(urls_num * sizeof(url_t));
      for (i = 0; i < urls_num; i++) {
        memset(buf, 0, ORIENTSEC_GRPC_PATH_MAX_LEN);
        url_decode_buf(childs.data[i], buf, ORIENTSEC_GRPC_PATH_MAX_LEN);
        url_parse_v2(buf, myurl);
        if (myurl->protocol == NULL || myurl->host == NULL) {
          url_valid--;
          memcpy(&urls[url_valid], myurl, sizeof(url_t));
          memset(myurl, 0, sizeof(url_t));
        } else {
          memcpy(&urls[url_curse], myurl, sizeof(url_t));
          memset(myurl, 0, sizeof(url_t));
          url_curse++;
        }
      }
      for (i = 0; i < childs.count - url_valid; i++) {
        url_free(&urls[url_curse + i]);
      }
    }
    urls_num = url_valid;
    if (url_valid == 0) {
      urls_num = 1;
      urls = (url_t*)gpr_zalloc(urls_num * sizeof(url_t));
      get_service_name_from_path(p_listener_node->url_full_string, buf,
                                 ORIENTSEC_GRPC_PATH_MAX_LEN);
      construct_empty_schema_url(buf, urls);
    }
    url_free(myurl);
    if (NULL != myurl) gpr_free(myurl);
    /*end by lm*/

    schedule_notify_func(p_listener_node, urls, urls_num);
    for (i = 0; i < urls_num; i++) {
      url_free(urls + i);
    }
    gpr_free(urls);
  }
  deallocate_String_vector(&childs);
}

void registy_recover(zk_connection_t* conn) {
  zk_listener_node *p_listener_node = NULL, *p_listener_node0 = NULL;
  zk_registy_url_node *p_registry_node = NULL, *p_registry_node0 = NULL;
  char* p = NULL;
  url_t* url = NULL;
  char buf[ORIENTSEC_GRPC_PATH_MAX_LEN] = {0};
  int ret = 0;
  struct String_vector childs;
  if (!conn || ZK_MANU_RECONNECTED != conn->connecte_state) {
    return;
  }
  p_registry_node0 = conn->url_list_head;
  p_registry_node = p_registry_node0->next;
  while (p_registry_node) {
    //恢复动态节点注册
    if (0 == p_registry_node->live) {
      p = strstr(p_registry_node->urlStr, "dynamic=true");
      if (p) {
        memset(buf, 0, ORIENTSEC_GRPC_PATH_MAX_LEN);
        url = url_parse(p_registry_node->urlStr);
        p = zk_get_url_path(url);
        zk_create_node(conn, p, true);
        url_free(url);
        FREE_PTR(p);
      }
      p_registry_node0 = p_registry_node;
      p_registry_node = p_registry_node->next;
    } else {  //删除状态为非0的url对应的zk节点并清空对应的内存
      memset(buf, 0, ORIENTSEC_GRPC_PATH_MAX_LEN);
      url = url_parse(p_registry_node->urlStr);
      p = zk_get_url_path(url);
      zk_delete_node(conn, p);
      url_free(url);
      FREE_PTR(p);

      //释放删除状态链接结构体内存
      p_registry_node0->next = p_registry_node->next;
      FREE_PTR(p_registry_node->urlStr);
      FREE_PTR(p_registry_node);
      p_registry_node = p_registry_node0->next;
    }
  }
  p_listener_node0 = conn->listener_list_head;
  p_listener_node = p_listener_node0->next;
  while (p_listener_node) {
    if (0 == p_listener_node->live) {
      ret = zoo_wget_children(conn->zh, p_listener_node->url_full_string,
                              zk_node_watcher_g, (void*)conn, &childs);
      if (ZOK == ret) {
        gpr_log(GPR_INFO, "recover connection [%s] subscribe[%s] success\n",
                conn->zk_address, p_listener_node->url_full_string);
      } else {
        gpr_log(GPR_ERROR,
                "recover connection [%s] subscribe[%s] faild,error "
                "code=%d,reason=%s\n",
                conn->zk_address, p_listener_node->url_full_string, ret,
                zerror(ret));
      }
    }
    p_listener_node = p_listener_node->next;
  }
}

//连接zk,keepSession:   0：新建session，1：使用原session,
void start_zk_connect(zk_connection_t* conn, int keepSession) {
  char* host = NULL;
  char* key = NULL;
  char* p = NULL;
  int key_len = 0;
  char buf[40] = {0};
  int timeout = 5000;  // 5000ms
  zhandle_t* handle = NULL;
  if (!conn) {
    return;
  }
  key_len = strlen(conn->zk_address);
  key = (char*)gpr_zalloc(key_len + 1);
  snprintf(key, key_len + 1, "%s", conn->zk_address);
  p = strstr(key, "://");
  if (p) {
    p = p + strlen("://");
    host = (char*)gpr_zalloc(key_len + 1);
    snprintf(host, strlen(p) + 1, "%s", p);
  } else {
    host = key;
  }

  //----begin--- 获取acl信息并设置acl开关
  get_acl_info();
  //----end----
  if (0 == orientsec_grpc_properties_get_value(ORIENTSEC_GRPC_ZK_TIMEOUT, NULL, buf)) {
    timeout = atoi(buf);
  }

  if (0 != keepSession) {
    conn->zh = zookeeper_init(host, zk_conn_watcher_g, timeout, conn->clientid,
                              (void*)conn, 0);
  } else {
    conn->zh =
        zookeeper_init(host, zk_conn_watcher_g, timeout, 0, (void*)conn, 0);
  }

  FREE_PTR(host);
  FREE_PTR(key);
}

void zk_start(registry_service_args_t* param) {
  zk_connection_t* conn = NULL;
  char* host = NULL;
  char* key = NULL;
  char* p = NULL;
  int key_len = 0;
  int timeout = 5000;  // 5000ms
  char buf[40] = {0};
  if (!param) {
    return;
  }
  init_zk_conn_list();  // set log level
  conn = get_zk_connection_t(param->param->key);
  if (!conn) {
    gpr_log(GPR_ERROR,
            "registry_service->zk_start get_zk_connection_t return null[alloc "
            "failed]");
    return;
  }
  if (ZK_INIT != conn->connecte_state) {
    return;
  }
  start_zk_connect(conn, 0);
  //保存连接对象
  param->param->data = conn;
  FREE_PTR(host);
  // FREE_PTR(p);
}
void zk_registe(registry_service_args_t* param, url_t* url) {
  zk_connection_t* conn = NULL;
  ;
  char* url_full_path = NULL;
  char* dynamic_str = NULL;
  bool dynamic = true;
  zk_registy_url_node* p_registry_url_node = NULL;
  if (!param || !url) {
    return;
  }
  conn = (zk_connection_t*)(param->param->data);
  if (conn) {
    p_registry_url_node = get_registry_url_node(conn, url);
    if (NULL == p_registry_url_node) {
      gpr_log(GPR_ERROR, "分配registry_url_node失败");
      return;
    }
    p_registry_url_node->live = 0;
    url_full_path = zk_get_url_path(url);
    dynamic_str =
        url_get_parameter_v2(url, ORIENTSEC_GRPC_REGISTRY_KEY_DYNAMIC, NULL);
    if (dynamic_str && 0 == orientsec_stricmp(dynamic_str, "false")) {
      dynamic = false;
    }
    // debug code
    //url_full_path = "/bocloud/acll/providers/grpc";
    // url_full_path =
    // "/bocloud/acll/providers/grpc%3A%2F192.168.2.123%3A50088";
    // zoo_add_auth(conn->zh, SCHEME,
    // USER_PASSWORD_MING,sizeof(USER_PASSWORD_MING), 0, 0);

    zk_create_node(conn, url_full_path, dynamic);
  }
  FREE_PTR(url_full_path);
}
void zk_unregiste(registry_service_args_t* param, url_t* url) {
  zk_connection_t* conn = NULL;
  char* url_full_path = NULL;
  int ret = 0;
  zk_registy_url_node* p_registry_url_node = NULL;
  if (!param || !url) {
    return;
  }
  conn = (zk_connection_t*)(param->param->data);
  if (conn) {
    p_registry_url_node = lookup_registry_url_node(conn, url);
    if (p_registry_url_node) {
      p_registry_url_node->live = 1;
    }
    if (!isZkConnected(conn)) return;
    url_full_path = zk_get_url_path(url);
    ret = zk_delete_node(conn, url_full_path);
    if (ZOK == ret) {
      remove_registry_url_node(conn, url);
    }
    //----addbyhuyn 20171114
    gpr_free(url_full_path);
  }
}
void zk_subscribe(registry_service_args_t* param, url_t* url,
                  registry_notify_f notify) {
  zk_connection_t* conn = NULL;
  char* url_category_path = NULL;
  zk_listener_node* p_listener_node = NULL;
  zk_notify_node* p_notify_node = NULL;
  struct String_vector childs;
  childs.count = 0;
  childs.data = NULL;
  url_t* urls = NULL;
  int urls_num = 0;
  size_t i = 0;
  int ret = 0;
  char buf[ORIENTSEC_GRPC_PATH_MAX_LEN] = {0};
  if (!param || !url) {
    gpr_log(GPR_INFO, "zk_subscribe failed for param or url is null");
    return;
  }
  conn = (zk_connection_t*)(param->param->data);
  if (!conn) return;
  url_category_path = zk_get_category_path(url);
  zk_create_node(conn, url_category_path, false);
  p_listener_node = lookup_listener_node(conn, url_category_path);

  p_notify_node = get_zk_listener_notify_node(conn, url_category_path, notify);

  //读取子节点信息
  if (p_listener_node) {
    ret = zoo_get_children(conn->zh, url_category_path, 0, &childs);
  } else {
    ret = zoo_wget_children(conn->zh, url_category_path, zk_node_watcher_g,
                            (void*)conn, &childs);
  }

  if (ZOK == ret) {
    p_listener_node = get_zk_listener_node(conn, url_category_path);
    if (p_listener_node) {
      /*对childs分析，删除无效节点by lm*/
      url_t* myurl;
      int url_valid = childs.count;
      int url_curse = 0;
      urls_num = childs.count;
      myurl = (url_t*)gpr_zalloc(1 * sizeof(url_t));
      //对于子节点为空的情形，返回一个empty://0.0.0.0/service_name格式url
      if (urls_num == 0)  
      {
      } else {
        urls = (url_t*)gpr_zalloc(urls_num * sizeof(url_t));
        for (i = 0; i < urls_num; i++) {
          memset(buf, 0, ORIENTSEC_GRPC_PATH_MAX_LEN);
          url_decode_buf(childs.data[i], buf, ORIENTSEC_GRPC_PATH_MAX_LEN);
          url_parse_v2(buf, myurl);
          if (myurl->protocol == NULL || myurl->host == NULL) {
            url_valid--;
            memcpy(&urls[url_valid], myurl, sizeof(url_t));
            memset(myurl, 0, sizeof(url_t));
          } else {
            memcpy(&urls[url_curse], myurl, sizeof(url_t));
            memset(myurl, 0, sizeof(url_t));
            url_curse++;
          }
        }
        for (i = 0; i < childs.count - url_valid; i++) {
          url_free(&urls[url_curse + i]);
          printf("%s\n", urls[url_curse + i]);
        }
      }
      urls_num = url_valid;
      if (url_valid == 0) {
        urls_num = 1;
        urls = (url_t*)gpr_zalloc(urls_num * sizeof(url_t));
        get_service_name_from_path(p_listener_node->url_full_string, buf,
                                   ORIENTSEC_GRPC_PATH_MAX_LEN);
        construct_empty_schema_url(buf, urls);
      }
      url_free(myurl);
      if (NULL != myurl) gpr_free(myurl);
      /*end by lm*/
      //第一次订阅时调用回调函数
      notify(urls, urls_num);
      for (i = 0; i < urls_num; i++) {
        url_free(urls + i);
      }
      gpr_free(urls);
    }
  } else {
    gpr_log(GPR_ERROR, "add subscribe failed,reason=%s", zerror(ret));
  }
  deallocate_String_vector(&childs);
  FREE_PTR(url_category_path);
  return;
}
void zk_unsubscribe(registry_service_args_t* param, url_t* url,
                    registry_notify_f notify) {
  zk_connection_t* conn = NULL;
  char* url_category_path = NULL;
  zk_listener_node* p_listener_node = NULL;
  int ret = 0;
  if (!param || !url) {
    gpr_log(GPR_INFO, "zk_unsubscribe failed for param or url is null");
    return;
  }
  conn = (zk_connection_t*)(param->param->data);
  url_category_path = zk_get_category_path(url);
  p_listener_node = get_zk_listener_node(conn, url_category_path);
  if (p_listener_node) {
    remove_zk_notify_node(p_listener_node, notify);
  }
  //如果该链接上的所有订阅函数已取消，则移除节点
  if (p_listener_node && (!p_listener_node->notify_list_head->next)) {
    remove_zk_listener_node(conn, url_category_path);

    // add by huyn
    p_listener_node->notify_list_head = NULL;
    p_listener_node->next = NULL;
    p_listener_node = NULL;
  }
  FREE_PTR(url_category_path);
  return;
}
void zk_lookup(registry_service_args_t* param, url_t* src, url_t** result,
               int* len) {
  zk_connection_t* conn = NULL;
  char* url_category_path = NULL;
  zk_listener_node* p_listener_node = NULL;
  struct String_vector childs;
  url_t* urls = NULL;
  int urls_num = 0;
  int ret = 0;
  size_t i = 0;
  int index = 0;
  char buf[ORIENTSEC_GRPC_PATH_MAX_LEN] = {0};
  if (!param || !src) {
    gpr_log(GPR_INFO, "zk_lookup failed for param or url is null");
    return;
  }
  conn = (zk_connection_t*)(param->param->data);
  url_category_path = zk_get_category_path(src);

  ret = zoo_get_children(conn->zh, url_category_path, 0, &childs);
  if (ZOK == ret) {
    urls_num = childs.count;
    urls = (url_t*)gpr_zalloc(urls_num * sizeof(url_t));
    for (i = 0; i < urls_num; i++) {
      memset(buf, 0, ORIENTSEC_GRPC_PATH_MAX_LEN);
      url_decode_buf(childs.data[i], buf, ORIENTSEC_GRPC_PATH_MAX_LEN);
      url_parse_v2(buf, urls + i);
    }
    deallocate_String_vector(&childs);
    ret = filterUrls(urls, urls_num, src);
    *len = ret;
    if (ret > 0) {
      *len = ret;
      *result = gpr_zalloc(ret * sizeof(url_t));
      for (i = 0; i < urls_num; i++) {
        if (ORIENTSEC_GRPC_CHECK_BIT(urls[i].flag, ORIENTSEC_GRPC_URL_MATCH_POS)) {
          url_clone(urls + i, *result + index);
          index++;
        }
      }
    }
    for (i = 0; i < urls_num; i++) {
      url_free(urls + i);
    }
    gpr_free(urls);
  } else {
    gpr_log(GPR_ERROR, "get child data failed,reason=%s", zerror(ret));
  }

  FREE_PTR(url_category_path);
}
char* zk_getData(registry_service_args_t* param, char* path) {
  zk_connection_t* conn = NULL;
  char buf[ORIENTSEC_GRPC_BUF_LEN] = {0};
  char buf_len = ORIENTSEC_GRPC_BUF_LEN;
  int ret = 0;
  if (!param || !path) {
    gpr_log(GPR_INFO, "zk_getData failed for param or path is null");
    return NULL;
  }
  conn = (zk_connection_t*)(param->param->data);
  ret = zoo_get(conn->zh, path, 0, buf, &buf_len, NULL);
  if (ZOK == ret && buf_len != -1) {
    return gprc_strdup(buf);
  } else {
    gpr_log(GPR_INFO, "zk_getData(%s) failed ,error code=%d,reason=%s", path,
            ret, zerror(ret));
  }
  return NULL;
}
void zk_stop(registry_service_args_t* param) {
  zk_connection_t* conn = NULL;
  int ret = 0;
  if (!param) {
    gpr_log(GPR_ERROR, "zk_stop failed for param is null");
    return;
  }
  conn = (zk_connection_t*)(param->param->data);
  ret = zookeeper_close(conn->zh);
  if (ZOK == ret) {
    conn->connecte_state = ZK_MANU_CLOSED;
  } else {
    gpr_log(GPR_ERROR, "zk_stop failed ,error code=%d,reason=%s", ret,
            zerror(ret));
    return;
  }
}
void zk_destroy(registry_service_args_t* param) {
  zk_connection_t* conn = NULL;
  if (!param) {
    gpr_log(GPR_ERROR, "zk_destroy failed for param is null");
    return;
  }
  conn = (zk_connection_t*)(param->param->data);
  release_zk_conneciton(conn);
}
/*
static int orientsec_stricmp(const char* a, const char* b) {
  int ca, cb;
  do {
    ca = tolower(*a);
    cb = tolower(*b);
    ++a;
    ++b;
  } while (ca == cb && ca && cb);
  return ca - cb;
}*/
