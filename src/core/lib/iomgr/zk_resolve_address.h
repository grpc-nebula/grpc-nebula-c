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
 *    2017/07/11
 *    version 0.9
 *    实现根据服务名查询获得provider列表
 */

#ifndef ORIENTSEC_ZK_RESOLVE_ADDRESS
#define ORIENTSEC_ZK_RESOLVE_ADDRESS

#include <stddef.h>
#include "src/core/lib/iomgr/exec_ctx.h"
#include "src/core/lib/iomgr/pollset_set.h"
#include "src/core/lib/iomgr/resolve_address.h"

/* Asynchronously resolve addr. Use default_port if a port isn't designated
in addr, otherwise use the port in addr. */
/* TODO(ctiller): add a timeout here */
extern void (*grpc_zk_resolve_address)(grpc_core::ExecCtx* exec_ctx,
                                       const char* addr,
	const char *default_port,
	grpc_pollset_set *interested_parties,
	grpc_closure *on_done,
    grpc_resolved_addresses** addresses, char* hasharg,char* meth_name);


/* Resolve addr in a blocking fashion. Returns NULL on failure. On success,
result must be freed with grpc_resolved_addresses_destroy. */
extern grpc_error *(*grpc_zk_blocking_resolve_address)(
	const char *name, const char *default_port,
    grpc_resolved_addresses** addresses, char* hasharg,char* meth_name);


extern void zk_resolve_address(const char* name, const char* default_port,
                               grpc_pollset_set* interested_parties,
                               grpc_closure* on_done,
                               grpc_resolved_addresses** addresses,
                               char* hasharg,
                               char* meth_name);

#endif // !ORIENTSEC_ZK_RESOLVE_ADDRESS


