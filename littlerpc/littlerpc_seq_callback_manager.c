#include "littlerpc_seq_callback_manager.h"

inline bool _popSeqCallbackByIndex(LittleRPCSeqCallbackManager_t *handle, int index, LittleRPCSeqCallback_t *sc);

void LittleRPCSeqCallbackManager_Init(LittleRPCSeqCallbackManager_t *handle)
{
    handle->seqCounter = 0;
    handle->seqCallbacksAvailableLen = 0;
}

void LittleRPCSeqCallbackManager_Destroy(LittleRPCSeqCallbackManager_t *handle)
{
}

bool LittleRPCSeqCallbackManager_PopSeqCallbackBySeq(LittleRPCSeqCallbackManager_t *handle,
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
    return _popSeqCallbackByIndex(handle, scIndex, sc);
}

bool LittleRPCSeqCallbackManager_PushSeqCallback(
    LittleRPCSeqCallbackManager_t *handle, LittleRPCSequence seq,
    const ProtobufCServiceDescriptor *serviceDescriptor, LittleRPCInvokeCallback callback,
    void *user_data)
{
    if (handle->seqCallbacksAvailableLen >= LITTLE_RPC_PENDDING_RPC_NUM)
    {
        return false;
    }
#if LITTLE_RPC_ENABLE_TIMEOUT
    LITTLE_RPC_TIMEOUT_GET_TICK(&handle->seqCallbacks[handle->seqCallbacksAvailableLen].pushTick);
#endif
    handle->seqCallbacks[handle->seqCallbacksAvailableLen].seq = seq;
    handle->seqCallbacks[handle->seqCallbacksAvailableLen].serviceDescriptor = serviceDescriptor;
    handle->seqCallbacks[handle->seqCallbacksAvailableLen].callback = callback;
    handle->seqCallbacks[handle->seqCallbacksAvailableLen].user_data = user_data;
    handle->seqCallbacksAvailableLen++;
    return true;
}

LittleRPCSequence LittleRPCSeqCallbackManager_GenerateSeq(LittleRPCSeqCallbackManager_t *handle)
{
    
#if LITTLE_RPC_THREAD_SAFE
    void * lock = LITTLE_RPC_LOCK_ACQUIRE();
#endif
    LittleRPCSequence ret = handle->seqCounter;
    handle->seqCounter += 1;
#if LITTLE_RPC_THREAD_SAFE
    LITTLE_RPC_LOCK_RELEASE(lock);
#endif
    return ret;
}


#if LITTLE_RPC_ENABLE_TIMEOUT
bool LittleRPCSeqCallbackManager_PopTimeoutSeqCallback(LittleRPCSeqCallbackManager_t *handle,
                                                       LittleRPCSeqCallback_t *sc)
{
    LITTLE_RPC_TICK_TYPE nowTick = 0;
    LITTLE_RPC_TIMEOUT_GET_TICK(&nowTick);

    int scIndex = -1;
    for (int i = 0; i < handle->seqCallbacksAvailableLen; i++)
    {
        if (nowTick - handle->seqCallbacks[i].pushTick  > LITTLE_RPC_TIMEOUT_WAIT_RESPONSE)
        {
            scIndex = i;
            break;
        }
    }
    return _popSeqCallbackByIndex(handle, scIndex, sc);
}
#endif  // if LITTLE_RPC_ENABLE_TIMEOUT


bool _popSeqCallbackByIndex(LittleRPCSeqCallbackManager_t *handle, int index, LittleRPCSeqCallback_t *sc)
{
    if (index < 0)
    {
        return false;
    }

    LITTLE_RPC_MEMCPY(sc, handle->seqCallbacks + index, sizeof(LittleRPCSeqCallback_t));

    if (index != handle->seqCallbacksAvailableLen - 1)
    {
        LITTLE_RPC_MEMCPY(handle->seqCallbacks + index, handle->seqCallbacks + index + 1,
                          sizeof(LittleRPCSeqCallback_t) * (handle->seqCallbacksAvailableLen - index - 1));
    }
    handle->seqCallbacksAvailableLen--;
    return true;
}
