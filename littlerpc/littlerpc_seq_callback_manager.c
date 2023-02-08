#include "littlerpc_seq_callback_manager.h"


void LittleRPCSeqCallbackManager_Init(LittleRPCSeqCallbackManager_t *handle)
{

    handle->seqCounter = 0;
    handle->seqCallbacks = (LittleRPCSeqCallback_t *)LITTLE_RPC_ALLOC(
        sizeof(LittleRPCSeqCallback_t) * LITTLE_RPC_PENDDING_RPC_NUM);
    handle->seqCallbacksAvailableLen = 0;
}

void LittleRPCSeqCallbackManager_Destroy(LittleRPCSeqCallbackManager_t *handle)
{
    LITTLE_RPC_FREE(handle->seqCallbacks);
}

bool LittleRPCSeqCallbackManager_popSeqCallbackBySeq(LittleRPCSeqCallbackManager_t *handle,
                                                     LittleRPCSequence seq,
                                                     LittleRPCSeqCallback_t *sc)
{
    int scIndex = -1;
    for (int i = 0; i < handle->seqCallbacksAvailableLen; i++)
    {
        if (handle->seqCallbacks[i].seq == seq)
        {
            scIndex = i;
            break;
        }
    }
    if (scIndex < 0)
    {
        return false;
    }

    LITTLE_RPC_MEMCPY(sc, handle->seqCallbacks + scIndex, sizeof(LittleRPCSeqCallback_t));

    if (scIndex != handle->seqCallbacksAvailableLen - 1)
    {
        LITTLE_RPC_MEMCPY(handle->seqCallbacks + scIndex, handle->seqCallbacks + scIndex + 1,
                          sizeof(LittleRPCSeqCallback_t));
    }
    handle->seqCallbacksAvailableLen--;
    return true;
}

bool LittleRPCSeqCallbackManager_pushSeqCallback(
    LittleRPCSeqCallbackManager_t *handle, LittleRPCSequence seq,
    const ProtobufCServiceDescriptor *serviceDescriptor, ProtobufCClosure closure,
    void *closure_data)
{
    if (handle->seqCallbacksAvailableLen >= LITTLE_RPC_PENDDING_RPC_NUM)
    {
        return false;
    }
    handle->seqCallbacks[handle->seqCallbacksAvailableLen].seq = seq;
    handle->seqCallbacks[handle->seqCallbacksAvailableLen].serviceDescriptor = serviceDescriptor;
    handle->seqCallbacks[handle->seqCallbacksAvailableLen].closure = closure;
    handle->seqCallbacks[handle->seqCallbacksAvailableLen].closure_data = closure_data;
    handle->seqCallbacksAvailableLen++;
    return true;
}

LittleRPCSequence LittleRPCSeqCallbackManager_generateSeq(LittleRPCSeqCallbackManager_t *handle)
{
    LittleRPCSequence ret = handle->seqCounter;
    handle->seqCounter += 1;
    return ret;
}
