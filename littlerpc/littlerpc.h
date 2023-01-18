#ifndef __LITTLE_RPC_SVR_H__
#define __LITTLE_RPC_SVR_H__

#include <stdint.h>
#include <stdlib.h>

#include "protobuf-c/protobuf-c.h"

#include "littlerpc_def.h"
#include "littlerpc_pbc_service_manager.h"
#include "littlerpc_seq_callback_manager.h"

class LittleRPC
{

public:
    typedef void (*LittleRPCSendBufferCallback)(uint8_t *buff, size_t len);

    typedef enum
    {
        MSG_TYPE_INPUT = 1,
        MSG_TYPE_OUTPUT = 2,
    } LittleRPCMsgType_t;

    typedef struct
    {
        LittleRPCSequence seq;
        LittleRPCServiceID serviceID;
        LittleRPCMsgType_t msgType;
        LittleRPCMethodIndex methodIndex;
        size_t contentBufferLen;
    } LittleRPCHeader_t;

    typedef struct
    {
        LittleRPC *server;
        LittleRPCHeader_t *reqHeader;
    } ClosureContext_t;

    typedef enum
    {
        INVOKE_SUCC = 0,
        INVOKE_MEHTOD_NO_EXISTS = 1,
        INVOKE_TOO_MANY_PEDDING_RPC = 2,
    } RPCInvokeRet_t;

public:
    LittleRPC(LittleRPCSendBufferCallback sendBufferCallback = nullptr);
    ~LittleRPC();

    size_t onRecv(uint8_t *buff, size_t len);
    void sendBuffer(uint8_t *buff, size_t len);

    void setSendBufferCallback(LittleRPCSendBufferCallback sendBufferCallback);

    /* RPC Server Side*/
    void registService(LittleRPCServiceID serviceID, ProtobufCService *service);
    /* RPC Client Side*/
    RPCInvokeRet_t RpcInvoke(LittleRPCServiceID serviceID, const ProtobufCService *service,
                             const char *methodName, const ProtobufCMessage *input,
                             ProtobufCClosure closure, void *closure_data);

private:
    void processBuffer(void);

    // RPC Server Side
    void callService(LittleRPCHeader_t *header, uint8_t *bodyBuff, size_t buffLen);

    // RPC Client Side
    void callClosure(LittleRPCHeader_t *header, uint8_t *bodyBuff, size_t buffLen);

private:
    ProtobufCAllocator pbcAllocator;

    uint8_t *recvBuffer;
    size_t recvBufferAvailableSize;

    LittleRPCSendBufferCallback sendBufferCallback;

    // RPC Server Side
    LittleRPCProtobufCServicerManager serviceManager;

    /* RPC Client Side */
    LittleRPCSeqCallbackManager seqCallbackManager;
};

#endif
