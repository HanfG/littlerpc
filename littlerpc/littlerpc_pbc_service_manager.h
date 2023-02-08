#ifndef __LITTLE_RPC_PBC_SERVICE_MANAGER_H__
#define __LITTLE_RPC_PBC_SERVICE_MANAGER_H__
// PBC means Protobuf-C

#include "protobuf-c/protobuf-c.h"

#include "littlerpc_def.h"

typedef struct _LittleRPCPBCService
{
    LittleRPCServiceID serviceID;
    ProtobufCService *service;
} _LittleRPCPBCService_t;

typedef struct LittleRPCProtobufCServicerManager
{
    size_t servicesSize;
    size_t servicesNum;
    _LittleRPCPBCService_t *services;
} LittleRPCProtobufCServicerManager_t;

void LittleRPCProtobufCServicerManager_Init(LittleRPCProtobufCServicerManager_t *handle);

void LittleRPCProtobufCServicerManager_Destroy(LittleRPCProtobufCServicerManager_t *handle);

void LittleRPCProtobufCServicerManager_RegisteService(LittleRPCProtobufCServicerManager_t *handle,
                                                      LittleRPCServiceID serviceID,
                                                      ProtobufCService *service);

void LittleRPCProtobufCServicerManager_UnregistService(LittleRPCProtobufCServicerManager_t *handle,
                                                       LittleRPCServiceID serviceID);

ProtobufCService *
LittleRPCProtobufCServicerManager_FindServiceByID(LittleRPCProtobufCServicerManager_t *handle,
                                                  LittleRPCServiceID serviceID);
#endif
