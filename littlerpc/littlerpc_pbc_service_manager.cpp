#include "littlerpc_pbc_service_manager.h"

#define SERIVER_LIST_INIT_SIZE     8
#define SERIVER_LIST_INCREASE_STEP 4

LittleRPCProtobufCServicerManager::LittleRPCProtobufCServicerManager()
{
    this->services = LITTLE_RPC_NULLPTR;
    this->servicesNum = 0;
    this->servicesSize = 0;
}

LittleRPCProtobufCServicerManager::~LittleRPCProtobufCServicerManager()
{
    if (this->services != LITTLE_RPC_NULLPTR)
        LITTLE_RPC_FREE(this->services);
}


void LittleRPCProtobufCServicerManager::registService(LittleRPCServiceID serviceID,
                                                      ProtobufCService *service)
{

    if (this->findServiceByID(serviceID) != LITTLE_RPC_NULLPTR)
    {
        return;  // TODO return err
    }
    if (this->servicesNum >= this->servicesSize)
    {
        this->resizeServiceList(this->servicesSize + SERIVER_LIST_INCREASE_STEP);
    }
    this->services[this->servicesNum].serviceID = serviceID;
    this->services[this->servicesNum].service = service;
    this->servicesNum++;
}

void LittleRPCProtobufCServicerManager::unregistService(LittleRPCServiceID serviceID)
{
    int idx = this->getServiceIDIndex(serviceID);
    if (idx < 0)
        return;

    if (idx != this->servicesNum - 1)
        LITTLE_RPC_MEMCPY(&this->services[idx], &this->services[idx + 1], sizeof(PBCService_t));
    this->servicesNum--;
    if (this->servicesSize - this->servicesNum > SERIVER_LIST_INCREASE_STEP)
    {
        this->resizeServiceList(this->servicesNum - SERIVER_LIST_INCREASE_STEP);
    }
}

ProtobufCService *LittleRPCProtobufCServicerManager::findServiceByID(LittleRPCServiceID serviceID)
{
    int idx = this->getServiceIDIndex(serviceID);
    if (idx < 0)
        return LITTLE_RPC_NULLPTR;
    return this->services[idx].service;
}

int LittleRPCProtobufCServicerManager::getServiceIDIndex(LittleRPCServiceID serviceID)
{

    for (int i = 0; i < this->servicesNum; i++)
    {
        if (serviceID == this->services[i].serviceID)
        {
            return i;
        }
    }
    return -1;
}

void LittleRPCProtobufCServicerManager::resizeServiceList(size_t size)
{
    this->servicesSize = size;
    void *newMem = LITTLE_RPC_ALLOC(sizeof(PBCService_t) * this->servicesSize);
    if (this->services)
    {
        LITTLE_RPC_MEMCPY(newMem, this->services, sizeof(PBCService_t) * this->servicesNum);
        LITTLE_RPC_FREE(this->services);
    }
    this->services = (PBCService_t *)newMem;
}
