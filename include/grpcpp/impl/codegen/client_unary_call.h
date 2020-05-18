/*
 *
 * Copyright 2015 gRPC authors.
 * Modifications 2019 Orient Securities Co., Ltd.
 * Modifications 2019 BoCloud Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef GRPCPP_IMPL_CODEGEN_CLIENT_UNARY_CALL_H
#define GRPCPP_IMPL_CODEGEN_CLIENT_UNARY_CALL_H

#include <grpcpp/impl/codegen/call.h>
#include <grpcpp/impl/codegen/channel_interface.h>
#include <grpcpp/impl/codegen/config.h>
#include <grpcpp/impl/codegen/core_codegen_interface.h>
#include <grpcpp/impl/codegen/status.h>
//----begin----- add for hash algo
#include <grpc/grpc.h>
#include "../../orientsec/orientsec_common/orientsec_grpc_string_op.h"   // move string operation to common module by jianbin
#include  "../../orientsec/orientsec_consumer/orientsec_grpc_consumer_control_requests.h"
//----end----

namespace grpc {

class Channel;
class ClientContext;
class CompletionQueue;

namespace internal {

class RpcMethod;
/// Wrapper that performs a blocking unary call
template <class InputMessage, class OutputMessage>
Status BlockingUnaryCall(ChannelInterface* channel, const RpcMethod& method,
                         ClientContext* context, const InputMessage& request,
                         OutputMessage* result) {

  Status call_status_ =  BlockingUnaryCallImpl<InputMessage, OutputMessage>(
             channel, method, context, request, result).status();
  if (!call_status_.ok() && (orientsec_get_failure_retry_times() > 0)) {
    // retry call
    int retries = orientsec_get_failure_retry_times();
    do {
      context->reset_call();
      call_status_ = BlockingUnaryCallImpl<InputMessage, OutputMessage>(
                         channel, method, context, request, result)
                         .status();
      if( call_status_.ok() ) {
        break;
      }       
    }while (--retries);
    if (retries == 0) {
      std::cout << "Number of retries reached max times! return failure..."
                << std::endl;
    }
  }

  return call_status_;
}

template <class InputMessage, class OutputMessage>
class BlockingUnaryCallImpl {
 public:
  BlockingUnaryCallImpl(ChannelInterface* channel, const RpcMethod& method,
                        ClientContext* context, const InputMessage& request,
                        OutputMessage* result) {
    CompletionQueue cq(grpc_completion_queue_attributes{
        GRPC_CQ_CURRENT_VERSION, GRPC_CQ_PLUCK, GRPC_CQ_DEFAULT_POLLING,
        nullptr});  // Pluckable completion queue
    Call call(channel->CreateCall(method, context, &cq));
    CallOpSet<CallOpSendInitialMetadata, CallOpSendMessage,
              CallOpRecvInitialMetadata, CallOpRecvMessage<OutputMessage>,
              CallOpClientSendClose, CallOpClientRecvStatus>
        ops;
    status_ = ops.SendMessage(request);
    if (!status_.ok()) {
      return;
    }
    grpc_call* callobj = call.call();
    //----begin----
    //获得hash_arg,用于hash 算法
    char buf[ORIENTSEC_GRPC_PROPERTY_KEY_MAX_LEN] = {0};

    orientsec_grpc_properties_get_value(ORIENTSEC_GRPC_PROPERTIES_C_CONSISTENT_HASH_ARG,
                                   NULL, buf);
    // add by yang
    // style s = "name:\"heiden111111\"\n"
    std::string str = ops.GetMessageName(request);
    //多个值要用map值存储，需要查找
    std::map<std::string, std::string> map_;
    std::vector<std::string> buf_vec;
    orientsec_grpc_split_to_vec(buf, buf_vec, ",");
    orientsec_grpc_split_to_map(str, map_, "\n");

    std::string hash_arg;
    orientsec_grpc_joint_hash_input(map_, buf_vec, hash_arg);

    // Get method name
    std::string m_fullname = method.name();
    std::vector<std::string> ret;
    orientsec_grpc_split_to_vec(m_fullname,ret ,"/");
    std::string m_name = ret.back();
    orientsec_grpc_setcall_methodname(callobj, m_name.c_str());

    //传递给call 对象
    orientsec_grpc_transfer_setcall_hashinfo(callobj, hash_arg.c_str());
    
    //判断是否大于最大允许请求数
    //----debug use----
    //const char* name = method.name();
    if (orientsec_grpc_consumer_control_requests(method.name()) == -1) {
      Status state(StatusCode::EXCEEDING_REQUESTS,
                   "Exceeded maximum requests");
      return;
    }

    // obtain register info and provider host info
    char* reg_info = orientsec_grpc_call_get_reginfo(callobj);
    char* prov_host = orientsec_grpc_call_serverhost(callobj);

    //----end----
    ops.SendInitialMetadata(&context->send_initial_metadata_,
                            context->initial_metadata_flags());
    ops.RecvInitialMetadata(context);
    ops.RecvMessage(result);
    ops.AllowNoMessage();
    ops.ClientSendClose();
    ops.ClientRecvStatus(context, &status_);
    call.PerformOps(&ops);
    if (cq.Pluck(&ops)) {
      if (ops.got_message && status_.ok()) {
        // reset when customized call and last call was not okay
        if (reg_info != NULL && (!last_call_status_.ok())) { 
          reset_provider_failure(reg_info, prov_host,m_name.c_str());
        }
      }
      if (!ops.got_message && status_.ok()) {
        status_ = Status(StatusCode::UNIMPLEMENTED,
                         "No message returned for unary request");
      }
      if (!ops.got_message || !status_.ok()) {
        grpc_channel* chan_info = orientsec_grpc_call_get_channel(callobj);
        /*if (callobj != nullptr && callobj->is_client &&*/
        if (!orientsec_grpc_channel_is_native(chan_info)) {
          gpr_log(GPR_DEBUG, "terminate_with_error trigger failover... ");
          record_provider_failure(grpc_get_channel_client_reginfo(chan_info),
                                  grpc_get_channel_provider_addr(chan_info),
                                  method.name());
        }
      }

    } else {
      GPR_CODEGEN_ASSERT(!status_.ok());
      
      last_call_status_ = Status(StatusCode::CANCELLED,"Last call not okay");
    }
    //  testing code
   /* static int number = 0;
    number++;
    if (number < 30) {
      std::cout << "number = " << number << std::endl;
      status_ = Status(StatusCode::CANCELLED, "Testing xxx");
    }*/
  }
  Status status() { return status_; }
  //----begin----
  
  void orientsec_grpc_transfer_setcall_hashinfo(grpc_call* call, const char* s) {
    orientsec_grpc_setcall_hashinfo(call, s);
  }
  char* orientsec_grpc_transfer_getcall_hashinfo(grpc_call* call) {
    return orientsec_grpc_getcall_hashinfo(call);
  }
  //----end----

 private:
  Status status_;
  Status last_call_status_;
  int retry_times = 0;
  bool init_first = true;
};

}  // namespace internal
}  // namespace grpc

#endif  // GRPCPP_IMPL_CODEGEN_CLIENT_UNARY_CALL_H
