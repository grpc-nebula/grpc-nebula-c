noinst_LIBRARIES=liborientsec_provider.a
#INCLUDES = -I../../../ -I../../../include -I../orientsec_common -I../orientsec_registry
AM_CPPFLAGS = -I../../../ -I../../../include -I../orientsec_common -I../orientsec_registry
CFLAGS += -fPIC
CPPFLAGS += -fPIC -std=c++11
liborientsec_provider_a_SOURCES=orientsec_provider_intf.cc
liborientsec_provider_a_LIBADD=../orientsec_registry/liborientsec_registry.a
AUTOMAKE_OPTIONS=foreign
