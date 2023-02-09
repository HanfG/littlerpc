#include "littlerpc.h"

#include <string.h>

#define _START_FLAG_COUNT            2
#define _START_FLAG_0                0x55
#define _START_FLAG_1                0xAA
#define _START_FLAG_MATCH(buff, idx) (buff[idx] == _START_FLAG_0 && buff[idx + 1] == _START_FLAG_1)

static const unsigned int crc8Table[256] = {
    // X^8+X^2+X^1+1
    0x00, 0x07, 0x0E, 0x09, 0x1C, 0x1B, 0x12, 0x15, 0x38, 0x3F, 0x36, 0x31, 0x24, 0x23, 0x2A, 0x2D,
    0x70, 0x77, 0x7E, 0x79, 0x6C, 0x6B, 0x62, 0x65, 0x48, 0x4F, 0x46, 0x41, 0x54, 0x53, 0x5A, 0x5D,
    0xE0, 0xE7, 0xEE, 0xE9, 0xFC, 0xFB, 0xF2, 0xF5, 0xD8, 0xDF, 0xD6, 0xD1, 0xC4, 0xC3, 0xCA, 0xCD,
    0x90, 0x97, 0x9E, 0x99, 0x8C, 0x8B, 0x82, 0x85, 0xA8, 0xAF, 0xA6, 0xA1, 0xB4, 0xB3, 0xBA, 0xBD,
    0xC7, 0xC0, 0xC9, 0xCE, 0xDB, 0xDC, 0xD5, 0xD2, 0xFF, 0xF8, 0xF1, 0xF6, 0xE3, 0xE4, 0xED, 0xEA,
    0xB7, 0xB0, 0xB9, 0xBE, 0xAB, 0xAC, 0xA5, 0xA2, 0x8F, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9D, 0x9A,
    0x27, 0x20, 0x29, 0x2E, 0x3B, 0x3C, 0x35, 0x32, 0x1F, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0D, 0x0A,
    0x57, 0x50, 0x59, 0x5E, 0x4B, 0x4C, 0x45, 0x42, 0x6F, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7D, 0x7A,
    0x89, 0x8E, 0x87, 0x80, 0x95, 0x92, 0x9B, 0x9C, 0xB1, 0xB6, 0xBF, 0xB8, 0xAD, 0xAA, 0xA3, 0xA4,
    0xF9, 0xFE, 0xF7, 0xF0, 0xE5, 0xE2, 0xEB, 0xEC, 0xC1, 0xC6, 0xCF, 0xC8, 0xDD, 0xDA, 0xD3, 0xD4,
    0x69, 0x6E, 0x67, 0x60, 0x75, 0x72, 0x7B, 0x7C, 0x51, 0x56, 0x5F, 0x58, 0x4D, 0x4A, 0x43, 0x44,
    0x19, 0x1E, 0x17, 0x10, 0x05, 0x02, 0x0B, 0x0C, 0x21, 0x26, 0x2F, 0x28, 0x3D, 0x3A, 0x33, 0x34,
    0x4E, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5C, 0x5B, 0x76, 0x71, 0x78, 0x7F, 0x6A, 0x6D, 0x64, 0x63,
    0x3E, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2C, 0x2B, 0x06, 0x01, 0x08, 0x0F, 0x1A, 0x1D, 0x14, 0x13,
    0xAE, 0xA9, 0xA0, 0xA7, 0xB2, 0xB5, 0xBC, 0xBB, 0x96, 0x91, 0x98, 0x9F, 0x8A, 0x8D, 0x84, 0x83,
    0xDE, 0xD9, 0xD0, 0xD7, 0xC2, 0xC5, 0xCC, 0xCB, 0xE6, 0xE1, 0xE8, 0xEF, 0xFA, 0xFD, 0xF4, 0xF3};

inline void _sendBuffer(LittleRPC_t *handle, uint8_t *buff, size_t len);
inline void _processBuffer(LittleRPC_t *handle);
inline void _callService(LittleRPC_t *handle, LittleRPCHeader_t *header, uint8_t *bodyBuff,
                         size_t buffLen);
inline void _callClosure(LittleRPC_t *handle, LittleRPCHeader_t *header, uint8_t *bodyBuff,
                         size_t buffLen);
inline void _moveBuffer(LittleRPC_t *handle, size_t moveLen);
inline void _sendPackage(LittleRPC_t *handle, LittleRPCHeader_t *header, const ProtobufCMessage *input);

inline uint8_t _calcHeaderCRC(LittleRPCHeader_t *header);
inline bool _checkHeaderCRC(LittleRPCHeader_t *header);


static int _getServiveMethodIndex(const ProtobufCServiceDescriptor *service,
                                  const char *methodName);
static void *_little_rpc_alloc(void *allocator_data, size_t size);
static void _little_rpc_free(void *allocator_data, void *pointer);
static void _serviceClosure(const ProtobufCMessage *msg, void *closure_data);


void LittleRPC_Init(LittleRPC_t *handle)
{
    handle->pbcAllocator.alloc = _little_rpc_alloc;
    handle->pbcAllocator.free = _little_rpc_free;
    handle->pbcAllocator.allocator_data = handle;

    handle->recvBufferAvailableSize = 0;
#if LITTLE_RPC_MEM_STATIC != 1
    handle->recvBuffer = (uint8_t *)LITTLE_RPC_ALLOC(LITTLE_RPC_CACHE_SIZE);
#endif

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

size_t LittleRPC_OnRecv(LittleRPC_t *handle, uint8_t *buff, size_t len)
{
#if LITTLE_RPC_ENABLE_TIMEOUT != 0
    LittleRPCSeqCallback_t sc;
    while(LittleRPCSeqCallbackManager_PopTimeoutSeqCallback(&handle->seqCallbackManager, &sc))
    {
        sc.callback(INVOKE_RESULT_TIMEOUT, LITTLE_RPC_NULLPTR, sc.user_data);
    }
    LITTLE_RPC_TICK_TYPE nowTick = 0;
    LITTLE_RPC_TIMEOUT_GET_TICK(&nowTick);
    if (nowTick - handle->lastOnRecvTick > LITTLE_RPC_TIMEOUT_RECV_PACKAGE)
    {
        handle->recvBufferAvailableSize = 0; // clear buffer
    }
    handle->lastOnRecvTick = nowTick;
#endif  // if LITTLE_RPC_ENABLE_TIMEOUT != 0

    size_t copyLen = len < (LITTLE_RPC_CACHE_SIZE - handle->recvBufferAvailableSize)
                         ? len
                         : (LITTLE_RPC_CACHE_SIZE - handle->recvBufferAvailableSize);

    LITTLE_RPC_MEMCPY(handle->recvBuffer + handle->recvBufferAvailableSize, buff, copyLen);
    handle->recvBufferAvailableSize += copyLen;

    _processBuffer(handle);
    return copyLen;
}


LittleRPCInvokeRet_t LittleRPC_RpcInvoke(LittleRPC_t *handle, LittleRPCServiceID serviceID,
                                         const ProtobufCServiceDescriptor *serviceDescriptor,
                                         const char *methodName, const ProtobufCMessage *input,
                                         LittleRPCInvokeCallback callback, void *user_data)
{

    int methidIndex = _getServiveMethodIndex(serviceDescriptor, methodName);
    if (methidIndex < 0)
    {
        return INVOKE_RET_MEHTOD_NO_EXISTS;
    }
    LittleRPCSequence seq = LittleRPCSeqCallbackManager_GenerateSeq(&handle->seqCallbackManager);
    
    if (callback != LITTLE_RPC_NULLPTR && LittleRPCSeqCallbackManager_PushSeqCallback(
                                             &handle->seqCallbackManager, seq,
                                             serviceDescriptor, callback, user_data) == false)
    {
        return INVOKE_RET_TOO_MANY_PEDDING_RPC;
    }

    LittleRPCHeader_t header = {0};
    header.seq = seq;
    header.serviceID = serviceID;
    header.msgType = LITTLERPC_MSG_TYPE_INPUT;
    header.methodIndex = methidIndex;

    _sendPackage(handle, &header, input);

    return INVOKE_RET_SUCC;
}

void _sendBuffer(LittleRPC_t *handle, uint8_t *buff, size_t len)
{
    if (handle->sendBufferCallback != LITTLE_RPC_NULLPTR)
    {
        handle->sendBufferCallback(buff, len, handle->sendBufferCallbackUserData);
    }
}

void _moveBuffer(LittleRPC_t *handle, size_t moveLen)
{
    if (moveLen < 1)
    {
        return;
    }
    LITTLE_RPC_MEMCPY(handle->recvBuffer, handle->recvBuffer + moveLen,
                      handle->recvBufferAvailableSize - moveLen);
    handle->recvBufferAvailableSize -= moveLen;
}

void _stripBuffer(LittleRPC_t *handle)
{
    size_t startIndex = 0;
    while (startIndex < handle->recvBufferAvailableSize &&
           (!_START_FLAG_MATCH(handle->recvBuffer, startIndex)))
    {
        startIndex += 1;
    }
    _moveBuffer(handle, startIndex);
}

void _processBuffer(LittleRPC_t *handle)
{

    LittleRPCHeader_t *header = LITTLE_RPC_NULLPTR;
    do
    {
        _stripBuffer(handle);
        if (handle->recvBufferAvailableSize < (sizeof(LittleRPCHeader_t) + _START_FLAG_COUNT))
            return;

        header = (LittleRPCHeader_t *)(handle->recvBuffer + _START_FLAG_COUNT);
#if LITTLE_RPC_PACK_VERIFY != 0
        if (_checkHeaderCRC(header))
        {
            break;
        }
        _moveBuffer(handle, 2);
    } while (1);
#else
    } while (0);
#endif

    size_t packageLen = _START_FLAG_COUNT + sizeof(LittleRPCHeader_t) + header->contentBufferLen;

    if (handle->recvBufferAvailableSize < packageLen)
        return;
    if (header->msgType == LITTLERPC_MSG_TYPE_INPUT)
    {
        _callService(handle, header,
                     handle->recvBuffer + sizeof(LittleRPCHeader_t) + _START_FLAG_COUNT,
                     header->contentBufferLen);
    }
    else if (header->msgType == LITTLERPC_MSG_TYPE_OUTPUT)
    {
        _callClosure(handle, header,
                     handle->recvBuffer + sizeof(LittleRPCHeader_t) + _START_FLAG_COUNT,
                     header->contentBufferLen);
    }
    _moveBuffer(handle, packageLen);
}

void _sendPackage(LittleRPC_t *handle, LittleRPCHeader_t *header, const ProtobufCMessage *input)
{

    size_t bodySize = protobuf_c_message_get_packed_size(input);
    uint8_t *bodyBuffer = (uint8_t *)LITTLE_RPC_ALLOC(bodySize);

    protobuf_c_message_pack(input, bodyBuffer);

    header->contentBufferLen = bodySize;
#if LITTLE_RPC_PACK_VERIFY != 0
    header->headerCRC8 = _calcHeaderCRC(header);
#endif

    const static uint8_t startFlag[] = {_START_FLAG_0, _START_FLAG_1};
    _sendBuffer(handle, (uint8_t *)startFlag, sizeof(startFlag));
    _sendBuffer(handle, (uint8_t *)header, sizeof(LittleRPCHeader_t));
    _sendBuffer(handle, bodyBuffer, bodySize);
    LITTLE_RPC_FREE(bodyBuffer);
}

uint8_t _calcHeaderCRC(LittleRPCHeader_t *header)
{
    uint8_t headerCRCBack = header->headerCRC8;
    uint8_t crc8 = 0;
    header->headerCRC8 = 0;
    for (int i = 0; i < sizeof(LittleRPCHeader_t); i++)
    {
        crc8 = crc8 ^ ((uint8_t *)header)[i];
        crc8 = crc8Table[crc8];
    }
    header->headerCRC8 = headerCRCBack;
    return crc8;
}

bool _checkHeaderCRC(LittleRPCHeader_t *header)
{
    return _calcHeaderCRC(header) == header->headerCRC8;
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

    service->invoke(service, header->methodIndex, msg, _serviceClosure, &closureContext);
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

    sc.callback(INVOKE_RESULT_FINISH, msg, sc.user_data);
    protobuf_c_message_free_unpacked(msg, &handle->pbcAllocator);
}


void _serviceClosure(const ProtobufCMessage *msg, void *closure_data)
{
    LittleRPCClosureContext_t *context = (LittleRPCClosureContext_t *)closure_data;

    LittleRPCHeader_t header = {0};
    header.seq = context->reqHeader->seq;
    header.serviceID = context->reqHeader->serviceID;
    header.methodIndex = context->reqHeader->methodIndex;
    header.msgType = LITTLERPC_MSG_TYPE_OUTPUT;

    _sendPackage(context->server, &header, msg);
}

int _getServiveMethodIndex(const ProtobufCServiceDescriptor *serviceDescriptor,
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

void *_little_rpc_alloc(void *allocator_data, size_t size)
{
    // LittleRPC *rpcHandler = (LittleRPC *)allocator_data;
    return LITTLE_RPC_ALLOC(size);
}

void _little_rpc_free(void *allocator_data, void *pointer)
{
    // LittleRPC *rpcHandler = (LittleRPC *)allocator_data;
    LITTLE_RPC_FREE(pointer);
}
