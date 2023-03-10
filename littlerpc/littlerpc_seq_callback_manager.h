#ifndef __LITTLE_RPC_SEQ_CALLBACK_MANAGER_H__
#define __LITTLE_RPC_SEQ_CALLBACK_MANAGER_H__
#ifdef __cplusplus
extern "C"{
#endif

#include <stdbool.h>

#include "protobuf-c/protobuf-c.h"

#include "littlerpc_def.h"

typedef struct LittleRPCSeqCallback
{
    LittleRPCSequence seq;
    const ProtobufCServiceDescriptor *serviceDescriptor;
#if LITTLE_RPC_ENABLE_TIMEOUT
    LITTLE_RPC_TICK_TYPE pushTick;
#endif  // if LITTLE_RPC_ENABLE_TIMEOUT
    LittleRPCInvokeCallback callback;
    void *user_data;
} LittleRPCSeqCallback_t;

typedef struct LittleRPCSeqCallbackManager
{
    LittleRPCSequence seqCounter;
    LittleRPCSeqCallback_t seqCallbacks[LITTLE_RPC_PENDDING_RPC_NUM];
    size_t seqCallbacksAvailableLen;
} LittleRPCSeqCallbackManager_t;

void LittleRPCSeqCallbackManager_Init(LittleRPCSeqCallbackManager_t *handle);

void LittleRPCSeqCallbackManager_Destroy(LittleRPCSeqCallbackManager_t *handle);

bool LittleRPCSeqCallbackManager_PopSeqCallbackBySeq(LittleRPCSeqCallbackManager_t *handle,
                                                     LittleRPCSequence seq,
                                                     LittleRPCSeqCallback_t *sc);
bool LittleRPCSeqCallbackManager_PushSeqCallback(
    LittleRPCSeqCallbackManager_t *handle, LittleRPCSequence seq,
    const ProtobufCServiceDescriptor *serviceDescriptor, LittleRPCInvokeCallback callback,
    void *user_data);

LittleRPCSequence LittleRPCSeqCallbackManager_GenerateSeq(LittleRPCSeqCallbackManager_t *handle);

#if LITTLE_RPC_ENABLE_TIMEOUT
bool LittleRPCSeqCallbackManager_PopTimeoutSeqCallback(LittleRPCSeqCallbackManager_t *handle,
                                                       LittleRPCSeqCallback_t *sc);
#endif  // if LITTLE_RPC_ENABLE_TIMEOUT

#ifdef __cplusplus
}
#endif

#endif
