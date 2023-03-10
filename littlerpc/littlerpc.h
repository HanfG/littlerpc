#ifndef __LITTLE_RPC_SVR_H__
#define __LITTLE_RPC_SVR_H__

#ifdef __cplusplus
extern "C" {
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

typedef void (*LittleRPCSendBufferCallback)(uint8_t* buff, size_t len, void* usedData);

enum LittleRPCMsgType {
    LITTLERPC_MSG_TYPE_INPUT = 0x01,
    LITTLERPC_MSG_TYPE_OUTPUT = 0x02,
    ___LITTLERPC_MSG_TYPE_MAX___ = 0xFF, // limit in one bytes
};

enum LittleRPCInvokeRet {
    INVOKE_RET_SUCC = 0,
    INVOKE_RET_MEHTOD_NO_EXISTS = 1,
    INVOKE_RET_TOO_MANY_PEDDING_RPC = 2,
    INVOKE_RET_SERVICE_ID_NOT_FOUND = 3,
};

struct LittleRPCHeader {
    uint8_t headerCRC8;
    LittleRPCSequence seq;
    LittleRPCServiceID serviceID;
    LittleRPCMsgType_t msgType;
    LittleRPCMethodIndex methodIndex;
    size_t contentBufferLen;
};

struct LittleRPCClosureContext {
    LittleRPC_t* server;
    LittleRPCHeader_t* reqHeader;
};

struct LittleRPC {
    ProtobufCAllocator pbcAllocator;
    uint8_t recvBuffer[LITTLE_RPC_CACHE_SIZE];
    size_t recvBufferAvailableSize;
#if LITTLE_RPC_ENABLE_TIMEOUT != 0
    LITTLE_RPC_TICK_TYPE lastOnRecvTick;
#endif // if LITTLE_RPC_ENABLE_TIMEOUT != 0

    void* sendBufferCallbackUserData;
    LittleRPCSendBufferCallback sendBufferCallback;

    // RPC Server Side
    LittleRPCProtobufCServicerManager_t serviceManager;

    /* RPC Client Side */
    LittleRPCSeqCallbackManager_t seqCallbackManager;
};

void LittleRPC_Init(LittleRPC_t* handle);
void LittleRPC_Destroy(LittleRPC_t* handle);

size_t LittleRPC_OnRecv(LittleRPC_t* handle, uint8_t* buff, size_t len);

void LittleRPC_SetSendBufferCallback(LittleRPC_t* handle,
    LittleRPCSendBufferCallback sendBufferCallback,
    void* sendBufferCallbackUserData);

/* RPC Server Side*/
int LittleRPC_RegistService(LittleRPC_t* handle,
    ProtobufCService* service);
/* RPC Client Side*/
LittleRPCInvokeRet_t LittleRPC_RpcInvoke(LittleRPC_t* handle,
    const ProtobufCServiceDescriptor* service,
    const char* methodName, const ProtobufCMessage* input,
    LittleRPCInvokeCallback callback, void* user_data);

#ifdef __cplusplus
}
#endif

#endif
