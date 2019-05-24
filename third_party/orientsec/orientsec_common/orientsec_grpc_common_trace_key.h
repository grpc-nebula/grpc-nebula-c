
/*
*    Author : huyn 
*    2017/07/12
*    version 0.0.9
*    服务链跟着头文件key定义
*/

#ifndef ORIENTSEC_GRPC_COMMON_TRACE_KEY_H
#define ORIENTSEC_GRPC_COMMON_TRACE_KEY_H

#ifdef __cplusplus
extern "C" {
#endif
	/**
	* define trace relat key .
	* save TraceInfo: traceId
	*/
#define EXT_TRACEINFO_KEY "ex_traceinfo"

	/**
	* define trace relat key .
	* save TraceInfo: traceId
	*/
	#define EXT_TRACEID_KEY "ex_traceid"


	/**
	* define trace relat key .
	* save TraceInfo: spanId
	*/
    #define EXT_CHAINID_KEY "ex_chainid"

	/**
	* define trace relat key .
	* save TraceInfo: spanId
	*/
     #define EXT_PARENT_CHAINID_KEY "ex_pchainid"

	 

	/**
	* define trace relat key .
	* save TraceInfo: ex_callCount
	*/
    #define EXT_CALLCOUNT_KEY  "ex_callcount"
	 
	/**
	* define trace relat key .
	* save TraceInfo: serviceName
	*/
	#define EXT_SERVICENAME_KEY  "ex_sername"
 
	/**
	* define trace relat key .
	* save TraceInfo: user_extend_methodName
	*/
    #define EXT_METHODNAME_KEY  "ex_metname"

	/**
	* define trace relat key .
	* save TraceInfo: consumer host
	*/
    #define EXT_CONSUMERHOST_KEY  "ex_conhost"

	/**
	* define trace relat key .
	* save TraceInfo: consumer port
	*/
	#define EXT_CONSUMERPORT_KEY  "ex_conport"

	/**
	* define trace relat key .
	* save TraceInfo: provider host
	*/
	#define EXT_PROVIDERHOST_KEY  "ex_prohost"

	/**
	* define trace relat key .
	* save TraceInfo: provider port
	*/
	#define EXT_PROVIDERPORT_KEY  "ex_proport"


#ifdef __cplusplus
}
#endif
#endif // !ORIENTSEC_GRPC_COMMON_TRACE_KEY_H
