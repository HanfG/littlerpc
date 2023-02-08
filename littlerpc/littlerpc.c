#include "littlerpc.h"

#include <string.h>

static void _sendBuffer(LittleRPC_t *handle, uint8_t *buff, size_t len);
static void _processBuffer(LittleRPC_t *handle);
static void _callService(LittleRPC_t *handle, LittleRPCHeader_t *header, uint8_t *bodyBuff,
                         size_t buffLen);
static void _callClosure(LittleRPC_t *handle, LittleRPCHeader_t *header, uint8_t *bodyBuff,
                         size_t buffLen);

static int getServiveMethodIndex(const ProtobufCServiceDescriptor *service, const char *methodName);
static void *little_rpc_alloc(void *allocator_data, size_t size);
static void little_rpc_free(void *allocator_data, void *pointer);
static void serviceClosure(const ProtobufCMessage *msg, void *closure_data);

void LittleRPC_Init(LittleRPC_t *handle)
{
    handle->pbcAllocator.alloc = little_rpc_alloc;
    handle->pbcAllocator.free = little_rpc_free;
    handle->pbcAllocator.allocator_data = handle;

    handle->recvBufferAvailableSize = 0;
    handle->recvBuffer = (uint8_t *)LITTLE_RPC_ALLOC(LITTLE_RPC_CACHE_SIZE);

    handle->sendBufferCallback = LITTLE_RPC_NULLPTR;
    handle->sendBufferCallbackUserData = LITTLE_RPC_NULLPTR;

    LittleRPCSeqCallbackManager_Init(&handle->seqCallbackManager);
    LittleRPCProtobufCServicerManager_Init(&handle->serviceManager);
}

void LittleRPC_Destroy(LittleRPC_t *handle)
{

    LittleRPCProtobufCServicerManager_Destroy(&handle->serviceManager);
    LittleRPCSeqCallbackManager_Destroy(&handle->seqCallbackManager);
    LITTLE_RPC_FREE(handle->recvBuffer);
}

void LittleRPC_SetSendBufferCallback(LittleRPC_t *handle,
                                     LittleRPCSendBufferCallback sendBufferCallback,
                                     void *sendBufferCallbackUserData)
{
    handle->sendBufferCallback = sendBufferCallback;
    handle->sendBufferCallbackUserData = sendBufferCallbackUserData;
}


void LittleRPC_RegistService(LittleRPC_t *handle, LittleRPCServiceID serviceID,
                             ProtobufCService *service)
{
    LittleRPCProtobufCServicerManager_RegisteService(&handle->serviceManager, serviceID, service);
}

static void _sendBuffer(LittleRPC_t *handle, uint8_t *buff, size_t len)
{
    if (handle->sendBufferCallback != LITTLE_RPC_NULLPTR)
    {
        handle->sendBufferCallback(buff, len, handle->sendBufferCallbackUserData);
    }
}

void _processBuffer(LittleRPC_t *handle)
{
    if (handle->recvBufferAvailableSize < sizeof(LittleRPCHeader_t))
        return;

    LittleRPCHeader_t *header = (LittleRPCHeader_t *)handle->recvBuffer;
    size_t packageLen = sizeof(LittleRPCHeader_t) + header->contentBufferLen;

    if (handle->recvBufferAvailableSize < packageLen)
        return;
    if (header->msgType == LITTLERPC_MSG_TYPE_INPUT)
    {
        _callService(handle, header, handle->recvBuffer + sizeof(LittleRPCHeader_t),
                     header->contentBufferLen);
    }
    else if (header->msgType == LITTLERPC_MSG_TYPE_OUTPUT)
    {
        _callClosure(handle, header, handle->recvBuffer + sizeof(LittleRPCHeader_t),
                     header->contentBufferLen);
    }
    LITTLE_RPC_MEMCPY(handle->recvBuffer, handle->recvBuffer + packageLen,
                      handle->recvBufferAvailableSize - packageLen);
    handle->recvBufferAvailableSize -= packageLen;
}

void _callService(LittleRPC_t *handle, LittleRPCHeader_t *header, uint8_t *bodyBuff, size_t buffLen)
{
    ProtobufCService *service = LittleRPCProtobufCServicerManager_FindServiceByID(
        &handle->serviceManager, header->serviceID);

    if (service == LITTLE_RPC_NULLPTR)
    {
        return;
    }
    if (header->methodIndex >= service->descriptor->n_methods)
    {
        return;
    }

    const ProtobufCMessageDescriptor *reqMsgDesc =
        service->descriptor->methods[header->methodIndex].input;
    ProtobufCMessage *msg =
        protobuf_c_message_unpack(reqMsgDesc, &handle->pbcAllocator, buffLen, bodyBuff);

    LittleRPCClosureContext_t closureContext;
    closureContext.server = handle;
    closureContext.reqHeader = header;

    service->invoke(service, header->methodIndex, msg, serviceClosure, &closureContext);
    protobuf_c_message_free_unpacked(msg, &handle->pbcAllocator);
}

void _callClosure(LittleRPC_t *handle, LittleRPCHeader_t *header, uint8_t *bodyBuff, size_t buffLen)
{
    LittleRPCSeqCallback_t sc;

    if (!LittleRPCSeqCallbackManager_PopSeqCallbackBySeq(&handle->seqCallbackManager, header->seq,
                                                         &sc))
    {
        return;
    }
    const ProtobufCMessageDescriptor *outputMsgDesc =
        sc.serviceDescriptor->methods[header->methodIndex].output;

    ProtobufCMessage *msg =
        protobuf_c_message_unpack(outputMsgDesc, &handle->pbcAllocator, buffLen, bodyBuff);

    sc.closure(msg, sc.closure_data);
    protobuf_c_message_free_unpacked(msg, &handle->pbcAllocator);
}

size_t LittleRPC_OnRecv(LittleRPC_t *handle, uint8_t *buff, size_t len)
{
    size_t copyLen = len < (LITTLE_RPC_CACHE_SIZE - handle->recvBufferAvailableSize)
                         ? len
                         : (LITTLE_RPC_CACHE_SIZE - handle->recvBufferAvailableSize);
    size_t shiftLen = len - copyLen;

    LITTLE_RPC_MEMCPY(handle->recvBuffer + handle->recvBufferAvailableSize, buff, copyLen);
    handle->recvBufferAvailableSize += copyLen;

    _processBuffer(handle);
    return shiftLen;
}


LittleRPCInvokeRet_t LittleRPC_RpcInvoke(LittleRPC_t *handle, LittleRPCServiceID serviceID,
                                         const ProtobufCServiceDescriptor *serviceDescriptor,
                                         const char *methodName, const ProtobufCMessage *input,
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

    header.seq = LittleRPCSeqCallbackManager_GenerateSeq(&handle->seqCallbackManager);
    header.serviceID = serviceID;
    header.msgType = LITTLERPC_MSG_TYPE_INPUT;
    header.methodIndex = methidIndex;
    header.contentBufferLen = bodySize;

    if (closure != LITTLE_RPC_NULLPTR && LittleRPCSeqCallbackManager_PushSeqCallback(
                                             &handle->seqCallbackManager, header.seq,
                                             serviceDescriptor, closure, closure_data) == false)
    {
        LITTLE_RPC_FREE(bodyBuffer);
        return INVOKE_TOO_MANY_PEDDING_RPC;
    }

    _sendBuffer(handle, (uint8_t *)&header, sizeof(LittleRPCHeader_t));
    _sendBuffer(handle, bodyBuffer, bodySize);
    LITTLE_RPC_FREE(bodyBuffer);
    return INVOKE_SUCC;
}

void serviceClosure(const ProtobufCMessage *msg, void *closure_data)
{
    LittleRPCClosureContext_t *context = (LittleRPCClosureContext_t *)closure_data;

    size_t bodySize = protobuf_c_message_get_packed_size(msg);
    uint8_t *buff = (uint8_t *)LITTLE_RPC_ALLOC(bodySize);
    protobuf_c_message_pack(msg, buff);

    LittleRPCHeader_t header;
    header.seq = context->reqHeader->seq;
    header.serviceID = context->reqHeader->serviceID;
    header.methodIndex = context->reqHeader->methodIndex;
    header.msgType = LITTLERPC_MSG_TYPE_OUTPUT;
    header.contentBufferLen = bodySize;

    _sendBuffer(context->server, (uint8_t *)&header, sizeof(header) / sizeof(uint8_t));
    _sendBuffer(context->server, buff, bodySize);
    LITTLE_RPC_FREE(buff);
}

int getServiveMethodIndex(const ProtobufCServiceDescriptor *serviceDescriptor,
                          const char *methodName)
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
    LITTLE_RPC_FREE(pointer);
}
