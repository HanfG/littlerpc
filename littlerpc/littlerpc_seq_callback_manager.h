#ifndef __LITTLE_RPC_SEQ_CALLBACK_MANAGER_H__
#define __LITTLE_RPC_SEQ_CALLBACK_MANAGER_H__

#include "protobuf-c/protobuf-c.h"

#include "littlerpc_def.h"

class LittleRPCSeqCallbackManager
{
public:
    typedef struct
    {
        LittleRPCSequence seq;
        const ProtobufCService *service;
        ProtobufCClosure closure;
        void *closure_data;
    } SeqCallback;

public:
    LittleRPCSeqCallbackManager(void);
    ~LittleRPCSeqCallbackManager(void);

    bool popSeqCallbackBySeq(LittleRPCSequence seq, SeqCallback *sc);
    bool pushSeqCallback(LittleRPCSequence seq, const ProtobufCService *service,
                         ProtobufCClosure closure, void *closure_data);
    LittleRPCSequence generateSeq(void);

private:
    LittleRPCSequence seqCounter;
    SeqCallback *seqCallbacks;
    size_t seqCallbacksAvailableLen;
};

#endif
