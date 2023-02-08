#include "littlerpc_pbc_service_manager.h"

#define SERIVER_LIST_INIT_SIZE     8
#define SERIVER_LIST_INCREASE_STEP 4

static int _getServiceIDIndex(LittleRPCProtobufCServicerManager_t *handle,
                              LittleRPCServiceID serviceID);
static void _resizeServiceList(LittleRPCProtobufCServicerManager_t *handle, size_t size);


void LittleRPCProtobufCServicerManager_Init(LittleRPCProtobufCServicerManager_t *handle)
{
    handle->services = LITTLE_RPC_NULLPTR;
    handle->servicesNum = 0;
    handle->servicesSize = 0;
}

void LittleRPCProtobufCServicerManager_Destroy(LittleRPCProtobufCServicerManager_t *handle)
{
    if (handle->services != LITTLE_RPC_NULLPTR)
    {
        LITTLE_RPC_FREE(handle->services);
    }
}

void LittleRPCProtobufCServicerManager_RegisteService(LittleRPCProtobufCServicerManager_t *handle,
                                                      LittleRPCServiceID serviceID,
                                                      ProtobufCService *service)
{
    if (LittleRPCProtobufCServicerManager_FindServiceByID(handle, serviceID) != LITTLE_RPC_NULLPTR)
    {
        return;  // TODO return err
    }
    if (handle->servicesNum >= handle->servicesSize)
    {
        _resizeServiceList(handle, handle->servicesSize + SERIVER_LIST_INCREASE_STEP);
    }
    handle->services[handle->servicesNum].serviceID = serviceID;
    handle->services[handle->servicesNum].service = service;
    handle->servicesNum++;
}

void LittleRPCProtobufCServicerManager_UnregistService(LittleRPCProtobufCServicerManager_t *handle,
                                                       LittleRPCServiceID serviceID)
{
    int idx = _getServiceIDIndex(handle, serviceID);
    if (idx < 0)
    {
        return;
    }

    if (idx != handle->servicesNum - 1)
    {
        LITTLE_RPC_MEMCPY(&handle->services[idx], &handle->services[idx + 1], sizeof(_LittleRPCPBCService_t));
    }
    handle->servicesNum--;
    if (handle->servicesSize - handle->servicesNum > SERIVER_LIST_INCREASE_STEP)
    {
        _resizeServiceList(handle, handle->servicesNum - SERIVER_LIST_INCREASE_STEP);
    }
}

ProtobufCService *LittleRPCProtobufCServicerManager_FindServiceByID(LittleRPCProtobufCServicerManager_t *handle,
                                   LittleRPCServiceID serviceID)
{
    int idx = _getServiceIDIndex(handle, serviceID);
    if (idx < 0)
    {
        return LITTLE_RPC_NULLPTR;
    }
    return handle->services[idx].service;
}

int _getServiceIDIndex(LittleRPCProtobufCServicerManager_t *handle, LittleRPCServiceID serviceID)
{
    for (int i = 0; i < handle->servicesNum; i++)
    {
        if (serviceID == handle->services[i].serviceID)
        {
            return i;
        }
    }
    return -1;
}

void _resizeServiceList(LittleRPCProtobufCServicerManager_t *handle, size_t size)
{
    void *newMem = LITTLE_RPC_ALLOC(sizeof(_LittleRPCPBCService_t) * size);
    if (!newMem)
    {
        return; // XXX return err
    }
    handle->servicesSize = size;
    if (handle->services != LITTLE_RPC_NULLPTR)
    {
        LITTLE_RPC_MEMCPY(newMem, handle->services, sizeof(_LittleRPCPBCService_t) * handle->servicesNum);
        LITTLE_RPC_FREE(handle->services);
    }
    handle->services = (_LittleRPCPBCService_t *)newMem;
}
