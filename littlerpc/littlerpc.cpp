#include "littlerpc.h"

#include <string.h>

#include "littlerpc_def.h"

static int getServiveMethodIndex(const ProtobufCServiceDescriptor *service, const char *methodName);
static void *little_rpc_alloc(void *allocator_data, size_t size);
static void little_rpc_free(void *allocator_data, void *pointer);
static void serviceClosure(const ProtobufCMessage *msg, void *closure_data);

LittleRPC::LittleRPC(LittleRPCSendBufferCallback sendBufferCallback,
                     void *sendBufferCallbackUserData)
{
    this->pbcAllocator.alloc = little_rpc_alloc;
    this->pbcAllocator.free = little_rpc_free;
    this->pbcAllocator.allocator_data = this;

    this->recvBufferAvailableSize = 0;
    this->recvBuffer = (uint8_t *)LITTLE_RPC_ALLOC(LITTLE_RPC_CACHE_SIZE);

    this->setSendBufferCallback(sendBufferCallback, sendBufferCallbackUserData);
}

LittleRPC::~LittleRPC() { LITTLE_RPC_FREE(this->recvBuffer); }

void LittleRPC::setSendBufferCallback(LittleRPCSendBufferCallback sendBufferCallback,
                                      void *sendBufferCallbackUserData = LITTLE_RPC_NULLPTR)
{
    this->sendBufferCallback = sendBufferCallback;
    this->sendBufferCallbackUserData = sendBufferCallbackUserData;
}


void LittleRPC::registService(LittleRPCServiceID serviceID, ProtobufCService *service)
{
    this->serviceManager.registService(serviceID, service);
}

void LittleRPC::sendBuffer(uint8_t *buff, size_t len)
{
    if (this->sendBufferCallback != LITTLE_RPC_NULLPTR)
    {
        this->sendBufferCallback(buff, len, this->sendBufferCallbackUserData);
    }
}

void LittleRPC::processBuffer(void)
{
    if (this->recvBufferAvailableSize < sizeof(LittleRPCHeader_t))
        return;

    LittleRPCHeader_t *header = (LittleRPCHeader_t *)this->recvBuffer;
    size_t packageLen = sizeof(LittleRPCHeader_t) + header->contentBufferLen;

    if (this->recvBufferAvailableSize < packageLen)
        return;
    if (header->msgType == MSG_TYPE_INPUT)
    {
        this->callService(header, this->recvBuffer + sizeof(LittleRPCHeader_t),
                          header->contentBufferLen);
    }
    else if (header->msgType == MSG_TYPE_OUTPUT)
    {
        this->callClosure(header, this->recvBuffer + sizeof(LittleRPCHeader_t),
                          header->contentBufferLen);
    }
    LITTLE_RPC_MEMCPY(this->recvBuffer, this->recvBuffer + packageLen,
                      this->recvBufferAvailableSize - packageLen);
    this->recvBufferAvailableSize -= packageLen;
}

void LittleRPC::callService(LittleRPC::LittleRPCHeader_t *header, uint8_t *bodyBuff, size_t buffLen)
{
    ProtobufCService *service = this->serviceManager.findServiceByID(header->serviceID);

    if (service == LITTLE_RPC_NULLPTR)
        return;
    if (header->methodIndex >= service->descriptor->n_methods)
        return;

    const ProtobufCMessageDescriptor *reqMsgDesc =
        service->descriptor->methods[header->methodIndex].input;
    ProtobufCMessage *msg =
        protobuf_c_message_unpack(reqMsgDesc, &this->pbcAllocator, buffLen, bodyBuff);

    ClosureContext_t closureContext;
    closureContext.server = this;
    closureContext.reqHeader = header;

    service->invoke(service, header->methodIndex, msg, serviceClosure, &closureContext);
    protobuf_c_message_free_unpacked(msg, &this->pbcAllocator);
}

void LittleRPC::callClosure(LittleRPC::LittleRPCHeader_t *header, uint8_t *bodyBuff, size_t buffLen)
{
    LittleRPCSeqCallbackManager::SeqCallback sc;
    if (!this->seqCallbackManager.popSeqCallbackBySeq(header->seq, &sc))
    {
        return;
    }
    const ProtobufCMessageDescriptor *outputMsgDesc =
        sc.serviceDescriptor->methods[header->methodIndex].output;

    ProtobufCMessage *msg =
        protobuf_c_message_unpack(outputMsgDesc, &this->pbcAllocator, buffLen, bodyBuff);

    sc.closure(msg, sc.closure_data);
    protobuf_c_message_free_unpacked(msg, &this->pbcAllocator);
}

size_t LittleRPC::onRecv(uint8_t *buff, size_t len)
{
    size_t copyLen = len < (LITTLE_RPC_CACHE_SIZE - this->recvBufferAvailableSize)
                         ? len
                         : (LITTLE_RPC_CACHE_SIZE - this->recvBufferAvailableSize);
    size_t shiftLen = len - copyLen;

    LITTLE_RPC_MEMCPY(this->recvBuffer + this->recvBufferAvailableSize, buff, copyLen);
    this->recvBufferAvailableSize += copyLen;

    this->processBuffer();
    return shiftLen;
}

LittleRPC::RPCInvokeRet_t LittleRPC::RpcInvoke(LittleRPCServiceID serviceID,
                                               const ProtobufCServiceDescriptor *serviceDescriptor,
                                               const char *methodName,
                                               const ProtobufCMessage *input,
                                               ProtobufCClosure closure, void *closure_data)
{

    int methidIndex = getServiveMethodIndex(serviceDescriptor, methodName);
    if (methidIndex < 0)
    {
        return INVOKE_MEHTOD_NO_EXISTS;
    }

    size_t bodySize = protobuf_c_message_get_packed_size(input);
    uint8_t *bodyBuffer = (uint8_t *)LITTLE_RPC_ALLOC(bodySize);

    protobuf_c_message_pack(input, bodyBuffer);
    LittleRPCHeader_t header;
    header.seq = this->seqCallbackManager.generateSeq();
    header.serviceID = serviceID;
    header.msgType = MSG_TYPE_INPUT;
    header.methodIndex = methidIndex;
    header.contentBufferLen = bodySize;

    if (closure != LITTLE_RPC_NULLPTR && this->seqCallbackManager.pushSeqCallback(header.seq, serviceDescriptor, closure,
                                                                       closure_data) == false)
    {
        LITTLE_RPC_FREE(bodyBuffer);
        return INVOKE_TOO_MANY_PEDDING_RPC;
    }
    this->sendBuffer((uint8_t *)&header, sizeof(LittleRPCHeader_t));
    this->sendBuffer(bodyBuffer, bodySize);
    LITTLE_RPC_FREE(bodyBuffer);
    return INVOKE_SUCC;
}

void serviceClosure(const ProtobufCMessage *msg, void *closure_data)
{
    LittleRPC::ClosureContext_t *context = (LittleRPC::ClosureContext_t *)closure_data;

    size_t bodySize = protobuf_c_message_get_packed_size(msg);
    uint8_t *buff = (uint8_t *)LITTLE_RPC_ALLOC(bodySize);
    protobuf_c_message_pack(msg, buff);

    LittleRPC::LittleRPCHeader_t header;
    header.seq = context->reqHeader->seq;
    header.serviceID = context->reqHeader->serviceID;
    header.methodIndex = context->reqHeader->methodIndex;
    header.msgType = LittleRPC::MSG_TYPE_OUTPUT;
    header.contentBufferLen = bodySize;

    context->server->sendBuffer((uint8_t *)&header, sizeof(header) / sizeof(uint8_t));
    context->server->sendBuffer(buff, bodySize);
    LITTLE_RPC_FREE(buff);
}

int getServiveMethodIndex(const ProtobufCServiceDescriptor *serviceDescriptor, const char *methodName)
{
    for (int i = 0; i < serviceDescriptor->n_methods; i++)
    {
        if (0 == strcmp(methodName, serviceDescriptor->methods[i].name))
        {
            return i;
        }
    }
    return -1;
}

void *little_rpc_alloc(void *allocator_data, size_t size)
{
    // LittleRPC *rpcHandler = (LittleRPC *)allocator_data;
    return LITTLE_RPC_ALLOC(size);
}

void little_rpc_free(void *allocator_data, void *pointer)
{
    // LittleRPC *rpcHandler = (LittleRPC *)allocator_data;
    return LITTLE_RPC_FREE(pointer);
}
