AUTOMAKE_OPTIONS=foreign
noinst_LIBRARIES=liborientsec_common.a
#INCLUDES = -I../../../ -I../../../include
AM_CPPFLAGS= -I../../../ -I../../../include
AM_CFLAGS=-I../../../ -I../../../include
CFLAGS += -fPIC
liborientsec_common_a_lib=../../../libs/opt
liborientsec_common_a_source=../orientsec_common
liborientsec_common_a_SOURCES=orientsec_grpc_properties_tools.c \
orientsec_grpc_utils.c \
orientsec_grpc_common_utils.c \
orientsec_grpc_common_init.c \
orientsec_grpc_string_op.cc \
uuid4gen.c
liborientsec_common_a_LIBADD=${liborientsec_common_a_lib}/libgpr.a

