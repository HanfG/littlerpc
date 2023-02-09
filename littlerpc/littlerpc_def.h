#ifndef __LITTLE_RPC_DEF_H__
#define __LITTLE_RPC_DEF_H__

#include <stdint.h>

#ifdef LITTLE_RPC_USER_CONF
#include "littlerpc_conf.h"
#endif

typedef uint8_t LittleRPCServiceID;
typedef uint8_t LittleRPCMessageID;
typedef uint8_t LittleRPCSequence;
typedef int LittleRPCMethodIndex;

typedef enum LittleRPCInvokeResult LittleRPCInvokeResult_t;
enum LittleRPCInvokeResult
{
    INVOKE_RESULT_FINISH = 0,
    INVOKE_RESULT_TIMEOUT = 1,
};
typedef void (*LittleRPCInvokeCallback)(LittleRPCInvokeResult_t invokeResult, const ProtobufCMessage *, void *user_data);


#ifdef LITTLE_RPC_CONF_ALLOC
#define LITTLE_RPC_ALLOC(size) LITTLE_RPC_CONF_ALLOC(size)
#else
#include <stdlib.h>
#define LITTLE_RPC_ALLOC(size) malloc(size)
#endif  // ifdef LITTLE_RPC_CONF_ALLOC

#ifdef LITTLE_RPC_CONF_FREE
#define LITTLE_RPC_FREE(pointer) LITTLE_RPC_CONF_FREE(pointer)
#else
#include <stdlib.h>
#define LITTLE_RPC_FREE(pointer) free(pointer)
#endif  // ifdef LITTLE_RPC_CONF_FREE

#ifdef LITTLE_RPC_CONF_MEMCPY
#define LITTLE_RPC_MEMCPY(dest, src, size) LITTLE_RPC_CONF_MEMCPY(dest, src, size)
#else
#include <string.h>
#define LITTLE_RPC_MEMCPY(dest, src, size) memcpy(dest, src, size)
#endif  // ifdef LITTLE_RPC_CONF_MEMCPY

#ifdef LITTLE_RPC_CONF_PENDDING_RPC_NUM
#define LITTLE_RPC_PENDDING_RPC_NUM (LITTLE_RPC_CONF_PENDDING_RPC_NUM)
#else
#define LITTLE_RPC_PENDDING_RPC_NUM (10)
#endif  // ifdef LITTLE_RPC_CONF_PENDDING_RPC_NUM

#ifdef LITTLE_RPC_CONF_MEM_STATIC
#define LITTLE_RPC_MEM_STATIC LITTLE_RPC_CONF_MEM_STATIC
#else
#define LITTLE_RPC_MEM_STATIC 1
#endif  // ifdef LITTLE_RPC_CONF_MEM_STATIC

#ifdef LITTLE_RPC_CONF_CACHE_SIZE
#define LITTLE_RPC_CACHE_SIZE (LITTLE_RPC_CONF_CACHE_SIZE)
#else
#define LITTLE_RPC_CACHE_SIZE (16 * 1024)
#endif  // ifdef LITTLE_RPC_CONF_CACHE_SIZE

#ifdef LITTLE_RPC_CONF_NULLPTR
#define LITTLE_RPC_NULLPTR (LITTLE_RPC_CONF_NULLPTR)
#else
#define LITTLE_RPC_NULLPTR (NULL)
#endif  // ifdef LITTLE_RPC_CONF_NULLPTR

#ifdef LITTLE_RPC_CONF_PACK_VERIFY
#define LITTLE_RPC_PACK_VERIFY LITTLE_RPC_CONF_PACK_VERIFY
#else
#define LITTLE_RPC_PACK_VERIFY 1
#endif  // ifdef LITTLE_RPC_CONF_PACK_VERIFY

#ifdef LITTLE_RPC_CONF_ENABLE_TIMEOUT
#define LITTLE_RPC_ENABLE_TIMEOUT LITTLE_RPC_CONF_ENABLE_TIMEOUT
#else
#define LITTLE_RPC_ENABLE_TIMEOUT 0
#endif  // ifdef LITTLE_RPC_CONF_ENABLE_TIMEOUT


#if LITTLE_RPC_ENABLE_TIMEOUT != 0

#ifdef LITTLE_RPC_CONF_TIMEOUT_RECV_PACKAGE
#define LITTLE_RPC_TIMEOUT_RECV_PACKAGE LITTLE_RPC_CONF_TIMEOUT_RECV_PACKAGE
#else
#define LITTLE_RPC_TIMEOUT_RECV_PACKAGE 1000
#endif  // ifdef LITTLE_RPC_CONF_TIMEOUT_RECV_PACKAGE

#ifdef LITTLE_RPC_CONF_TIMEOUT_WAIT_RESPONSE
#define LITTLE_RPC_TIMEOUT_WAIT_RESPONSE LITTLE_RPC_CONF_TIMEOUT_WAIT_RESPONSE
#else
#define LITTLE_RPC_TIMEOUT_WAIT_RESPONSE 5000
#endif  // ifdef LITTLE_RPC_CONF_TIMEOUT_WAIT_RESPONSE

#ifndef LITTLE_RPC_CONF_TICK_TYPE
#error "add LITTLE_RPC_CONF_TICK_TYPE to config when enable `TIMEOUT` function"
#else
#define LITTLE_RPC_TICK_TYPE LITTLE_RPC_CONF_TICK_TYPE
#endif  // ifndef LITTLE_RPC_CONF_TICK_TYPE

#ifndef LITTLE_RPC_CONF_TIMEOUT_GET_TICK
#error "add LITTLE_RPC_CONF_TIMEOUT_GET_TICK to config when enable `TIMEOUT` function"
#else
#define LITTLE_RPC_TIMEOUT_GET_TICK(pTick) LITTLE_RPC_CONF_TIMEOUT_GET_TICK(pTick)
#endif  // ifndef LITTLE_RPC_CONF_TIMEOUT_GET_TICK

#endif  // if LITTLE_RPC_ENABLE_TIMEOUT != 0


#endif
