#include "littlerpc_seq_callback_manager.h"


LittleRPCSeqCallbackManager::LittleRPCSeqCallbackManager(void)
{

    this->seqCounter = 0;
    this->seqCallbacks =
        (SeqCallback *)LITTLE_RPC_ALLOC(sizeof(SeqCallback) * LITTLE_RPC_PENDDING_RPC_NUM);
    this->seqCallbacksAvailableLen = 0;
}

LittleRPCSeqCallbackManager::~LittleRPCSeqCallbackManager(void)
{

    LITTLE_RPC_FREE(this->seqCallbacks);
}

bool LittleRPCSeqCallbackManager::popSeqCallbackBySeq(LittleRPCSequence seq,
                                                      LittleRPCSeqCallbackManager::SeqCallback *sc)
{
    int scIndex = -1;
    for (int i = 0; i < this->seqCallbacksAvailableLen; i++)
    {
        if (this->seqCallbacks[i].seq == seq)
        {
            scIndex = i;
            break;
        }
    }
    if (scIndex < 0)
        return false;

    LITTLE_RPC_MEMCPY(sc, this->seqCallbacks + scIndex, sizeof(SeqCallback));

    if (scIndex != this->seqCallbacksAvailableLen - 1)
    {
        LITTLE_RPC_MEMCPY(this->seqCallbacks + scIndex, this->seqCallbacks + scIndex + 1, sizeof(SeqCallback));
    }
    this->seqCallbacksAvailableLen--;
    return true;
}
bool LittleRPCSeqCallbackManager::pushSeqCallback(LittleRPCSequence seq,
                                                  const ProtobufCService *service,
                                                  ProtobufCClosure closure, void *closure_data)
{
    if (this->seqCallbacksAvailableLen >= LITTLE_RPC_PENDDING_RPC_NUM)
    {
        return false;
    }
    this->seqCallbacks[this->seqCallbacksAvailableLen].seq = seq;
    this->seqCallbacks[this->seqCallbacksAvailableLen].service = service;
    this->seqCallbacks[this->seqCallbacksAvailableLen].closure = closure;
    this->seqCallbacks[this->seqCallbacksAvailableLen].closure_data = closure_data;
    this->seqCallbacksAvailableLen++;
    return true;
}
LittleRPCSequence LittleRPCSeqCallbackManager::generateSeq(void)
{
    LittleRPCSequence ret = this->seqCounter;
    this->seqCounter += 1;
    return ret;
}
