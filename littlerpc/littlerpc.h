#ifndef __LITTLE_RPC_SVR_H__
#define __LITTLE_RPC_SVR_H__

#ifdef __cplusplus
extern "C"{
#endif


#include <stdint.h>
#include <stdlib.h>

#include "protobuf-c/protobuf-c.h"

#include "littlerpc_def.h"
#include "littlerpc_pbc_service_manager.h"
#include "littlerpc_seq_callback_manager.h"

typedef enum LittleRPCMsgType LittleRPCMsgType_t;
typedef enum LittleRPCInvokeRet LittleRPCInvokeRet_t;

typedef struct LittleRPC LittleRPC_t;
typedef struct LittleRPCHeader LittleRPCHeader_t;
typedef struct LittleRPCClosureContext LittleRPCClosureContext_t;

typedef void (*LittleRPCSendBufferCallback)(uint8_t *buff, size_t len, void *usedData);

enum LittleRPCMsgType
{
    LITTLERPC_MSG_TYPE_INPUT = 1,
    LITTLERPC_MSG_TYPE_OUTPUT = 2,
};

enum LittleRPCInvokeRet
{
    INVOKE_SUCC = 0,
    INVOKE_MEHTOD_NO_EXISTS = 1,
    INVOKE_TOO_MANY_PEDDING_RPC = 2,
};

struct LittleRPCHeader
{
    LittleRPCSequence seq;
    LittleRPCServiceID serviceID;
    LittleRPCMsgType_t msgType;
    LittleRPCMethodIndex methodIndex;
    size_t contentBufferLen;
};

struct LittleRPCClosureContext
{
    LittleRPC_t *server;
    LittleRPCHeader_t *reqHeader;
};

struct LittleRPC
{
    ProtobufCAllocator pbcAllocator;
#if LITTLE_RPC_CACHE_SIZE_INIT_STATIC == 1
    uint8_t recvBuffer[LITTLE_RPC_CACHE_SIZE];
#else
    uint8_t *recvBuffer;
#endif
    size_t recvBufferAvailableSize;

    void *sendBufferCallbackUserData;
    LittleRPCSendBufferCallback sendBufferCallback;

    // RPC Server Side
    LittleRPCProtobufCServicerManager_t serviceManager;

    /* RPC Client Side */
    LittleRPCSeqCallbackManager_t seqCallbackManager;
};


void LittleRPC_Init(LittleRPC_t *handle);
void LittleRPC_Destroy(LittleRPC_t *handle);

size_t LittleRPC_OnRecv(LittleRPC_t *handle, uint8_t *buff, size_t len);

void LittleRPC_SetSendBufferCallback(LittleRPC_t *handle,
                                     LittleRPCSendBufferCallback sendBufferCallback,
                                     void *sendBufferCallbackUserData);

/* RPC Server Side*/
void LittleRPC_RegistService(LittleRPC_t *handle, LittleRPCServiceID serviceID,
                             ProtobufCService *service);
/* RPC Client Side*/
LittleRPCInvokeRet_t LittleRPC_RpcInvoke(LittleRPC_t *handle, LittleRPCServiceID serviceID,
                                         const ProtobufCServiceDescriptor *service,
                                         const char *methodName, const ProtobufCMessage *input,
                                         ProtobufCClosure closure, void *closure_data);


#ifdef __cplusplus
}
#endif

#endif
