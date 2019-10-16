noinst_LIBRARIES=liborientsec_consumer.a
#INCLUDES = -I../../../ -I../../../include -I../orientsec_common -I../orientsec_registry
AM_CPPFLAGS = -I../../../ -I../../../include -I../orientsec_common -I../orientsec_registry
CFLAGS += -fPIC
CPPFLAGS += -fPIC -std=c++11
liborientsec_consumer_a_SOURCES=condition_router.cc \
orientsec_consumer_intf.cc \
orientsec_grpc_consumer_utils.cc \
failover_utils.cc \
pickfirst_lb.cc \
round_robin_lb.cc \
weight_round_robin_lb.cc \
md5.cc \
consistent_hash.cc \
consistent_hash_lb.cc \
orientsec_grpc_consumer_control_deprecated.cc \
orientsec_grpc_consumer_control_requests.cc \
orientsec_grpc_consumer_control_version.cc \
orientsec_grpc_consumer_control_group.cc \
requests_controller_utils.cc
liborientsec_consumer_a_LIBADD=../orientsec_registry/liborientsec_registry.a ../orientsec_common/liborientsec_common.a
AUTOMAKE_OPTIONS=foreign
