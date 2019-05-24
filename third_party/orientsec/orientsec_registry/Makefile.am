AUTOMAKE_OPTIONS=foreign
noinst_LIBRARIES=liborientsec_registry.a
liborientsec_registry_a_SOURCES=orientsec_grpc_registry_zk_intf.c registry_factory.c registry_utils.c url.c base64.c des.c sha1.c zk_registry_factory.c zk_registry_service.c
CFLAGS += -fPIC
AM_CPPFLAGS = -I../../../ -I../orientsec_common/ -I../../../include  -I../../../../zookeeper/include

#INCLUDES= -I../../../ -I../orientsec_common/ -I../../../include  -I/usr/local/include/zookeeper
liborientsec_registry_a_LIBADD=../orientsec_common/liborientsec_common.a ../../../../zookeeper/.libs/libzookeeper_mt.a
