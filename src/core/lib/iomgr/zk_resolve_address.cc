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
 *    Author : liumin
 *    2019/1/7
 *    version 1.17.2
 *    实现根据服务名查询获得provider列表
 */
#include <grpc/support/port_platform.h>

#include "src/core/lib/iomgr/port.h"
#if (defined WIN64) || (defined WIN32)
#include <WS2tcpip.h>
#include <WinSock2.h>
#include <grpc/support/log_windows.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "src/core/lib/iomgr/unix_sockets_posix.h"
#endif
#include "src/core/lib/iomgr/sockaddr_utils.h"
#include "src/core/lib/iomgr/sockaddr.h"
#include "src/core/lib/iomgr/resolve_address.h"
#include <inttypes.h>
#include <string.h>
#include <sys/types.h>

#include <grpc/support/alloc.h>
#include <grpc/support/log.h>
#include <grpc/support/log_windows.h>
#include <grpc/support/string_util.h>
#include <grpc/support/time.h>

#include "src/core/lib/gpr/host_port.h"
#include "src/core/lib/gpr/string.h"
#include "src/core/lib/gprpp/thd.h"
#include "src/core/lib/iomgr/block_annotate.h"
#include "src/core/lib/iomgr/executor.h"
#include "src/core/lib/iomgr/iomgr_internal.h"

#include "src/core/lib/iomgr/zk_resolve_address.h"
#include "orientsec_grpc_utils.h"
#include "orientsec_consumer_intf.h"

typedef struct {
  char* name;
  char* default_port;
  grpc_closure request_closure;
  grpc_closure* on_done;
  grpc_resolved_addresses** addresses;
  //----begin----add for hash consistent
  char* hasharg;
  //----end---

} request;

static grpc_error* zk_blocking_resolve_address(
    const char* name, const char* default_port,
   //grpc_resolved_addresses** addresses) {
   grpc_resolved_addresses** addresses, char* hasharg) {
  //struct addrinfo hints;
  //struct addrinfo *result = NULL, *resp;
  //char* host;
  //char* port;
  //int s;
  size_t i;
  grpc_error* error = GRPC_ERROR_NONE;

  //------begin--add by liumin
  provider_t* providers;

  int provider_num = 0;

  //providers = consumer_query_providers(name, &provider_num, NULL);
  providers = consumer_query_providers(name, &provider_num, hasharg);
  if (provider_num == 0)
  {
    char* msg;
    gpr_asprintf(&msg, "no providers in resolution '%s'", name);
    error = GRPC_ERROR_CREATE_FROM_COPIED_STRING(msg);
    goto done;
  }
  /* Success path: set addrs non-NULL, fill it in */
  (*addresses) =(grpc_resolved_addresses*)gpr_malloc(sizeof(grpc_resolved_addresses));
  (*addresses)->naddrs = (size_t)provider_num;

  (*addresses)->addrs = (grpc_resolved_address*)gpr_malloc(
      sizeof(grpc_resolved_address) * (*addresses)->naddrs);
  i = 0;
  for (i = 0; i < (*addresses)->naddrs; i++) {
    struct sockaddr_in my_addr;
    int myport =
        (0 == providers[i].port) ? atoi(default_port) : providers[i].port;
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(myport);
    inet_pton(AF_INET, providers[i].host, (void*)&my_addr.sin_addr.s_addr);
    memset(&my_addr.sin_zero, 0, 8);

    memcpy(&(*addresses)->addrs[i].addr, &my_addr, sizeof(struct sockaddr_in));
    (*addresses)->addrs[i].len = sizeof(struct sockaddr_in);
  }
  for (i = 0; i < (*addresses)->naddrs; i++) {
    char* buf;
    grpc_sockaddr_to_string(&buf, &(*addresses)->addrs[i], 0);
    gpr_free(buf);
  }
  // comment_debug_begin
  for (i = 0; i < (size_t)provider_num; i++) {
    free_provider_v2(providers + i);
  }
  free(providers);
  //------end

done:
 /* if (providers) {
    free(providers);
  }*/
  return error;
}

/* Callback to be passed to grpc_executor to asynch-ify
 * grpc_blocking_resolve_address */
static void do_request_thread(void* rp, grpc_error* error) {
  request* r = (request*)rp;
  if (error == GRPC_ERROR_NONE) {
    error = zk_blocking_resolve_address(r->name, r->default_port, r->addresses, r->hasharg);
  } else {
    GRPC_ERROR_REF(error);
  }
  GRPC_CLOSURE_SCHED(r->on_done, error);
  gpr_free(r->name);
  gpr_free(r->default_port);
  gpr_free(r);
}

void zk_resolve_address(const char* name, const char* default_port,
                                    grpc_pollset_set* interested_parties,
                                    grpc_closure* on_done,
                        //grpc_resolved_addresses** addresses) {
                        grpc_resolved_addresses** addresses,char* hasharg) {
  request* r = (request*)gpr_malloc(sizeof(request));
  GRPC_CLOSURE_INIT(
      &r->request_closure, do_request_thread, r,
      grpc_executor_scheduler(GRPC_RESOLVER_EXECUTOR, GRPC_EXECUTOR_SHORT));
  r->name = gpr_strdup(name);
  r->default_port = gpr_strdup(default_port);
  r->on_done = on_done;
  r->addresses = addresses;
  //----begin----
  r->hasharg = hasharg;
  //----end----
  GRPC_CLOSURE_SCHED(&r->request_closure, GRPC_ERROR_NONE);
}

grpc_zk_address_resolver_vtable grpc_zk_resolver_vtable = {
    zk_resolve_address, zk_blocking_resolve_address};

