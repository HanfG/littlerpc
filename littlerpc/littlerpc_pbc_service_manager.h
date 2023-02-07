#ifndef __LITTLE_RPC_PBC_SERVICE_MANAGER_H__
#define __LITTLE_RPC_PBC_SERVICE_MANAGER_H__
// PBC means Protobuf-C

#include "protobuf-c/protobuf-c.h"

#include "littlerpc_def.h"


class LittleRPCProtobufCServicerManager
{
public:
    typedef struct
    {
        LittleRPCServiceID serviceID;
        ProtobufCService *service;
    } PBCService_t;

    LittleRPCProtobufCServicerManager();
    ~LittleRPCProtobufCServicerManager();

    void registService(LittleRPCServiceID serviceID, ProtobufCService *service);
    void unregistService(LittleRPCServiceID serviceID);
    ProtobufCService *findServiceByID(LittleRPCServiceID serviceID);

private:
    void resizeServiceList(size_t size);
    int getServiceIDIndex(LittleRPCServiceID serviceID);

    size_t servicesSize;
    size_t servicesNum;
    PBCService_t *services;
};

#endif
