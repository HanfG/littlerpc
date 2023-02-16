#include "littlerpc_pbc_service_manager.h"

#define SERIVER_LIST_INIT_SIZE 8
#define SERIVER_LIST_INCREASE_STEP 4

static int _getServiceIDIndex(LittleRPCProtobufCServicerManager_t* handle,
    LittleRPCServiceID serviceID);

void LittleRPCProtobufCServicerManager_Init(LittleRPCProtobufCServicerManager_t* handle)
{
    handle->servicesNum = 0;
}

void LittleRPCProtobufCServicerManager_Destroy(LittleRPCProtobufCServicerManager_t* handle)
{
}

int LittleRPCProtobufCServicerManager_RegisteService(LittleRPCProtobufCServicerManager_t* handle,
    LittleRPCServiceID serviceID,
    ProtobufCService* service)
{
    if (LittleRPCProtobufCServicerManager_FindServiceByID(handle, serviceID) != LITTLE_RPC_NULLPTR) {
        return 2;
    }
    if (handle->servicesNum >= LITTLE_RPC_MAX_SERVICE_NUM) {
        return 1;
    }
    handle->services[handle->servicesNum].serviceID = serviceID;
    handle->services[handle->servicesNum].service = service;
    handle->servicesNum++;
    return 0;
}

void LittleRPCProtobufCServicerManager_UnregistService(LittleRPCProtobufCServicerManager_t* handle,
    LittleRPCServiceID serviceID)
{
    int idx = _getServiceIDIndex(handle, serviceID);
    if (idx < 0) {
        return;
    }

    if (idx != handle->servicesNum - 1) {
        LITTLE_RPC_MEMCPY(&handle->services[idx], &handle->services[idx + 1], (handle->servicesNum - idx - 1) * sizeof(_LittleRPCPBCService_t));
    }
    handle->servicesNum--;
}

ProtobufCService* LittleRPCProtobufCServicerManager_FindServiceByID(LittleRPCProtobufCServicerManager_t* handle,
    LittleRPCServiceID serviceID)
{
    int idx = _getServiceIDIndex(handle, serviceID);
    if (idx < 0) {
        return LITTLE_RPC_NULLPTR;
    }
    return handle->services[idx].service;
}

int _getServiceIDIndex(LittleRPCProtobufCServicerManager_t* handle, LittleRPCServiceID serviceID)
{
    for (int i = 0; i < handle->servicesNum; i++) {
        if (serviceID == handle->services[i].serviceID) {
            return i;
        }
    }
    return -1;
}
