#ifndef __LITTLE_RPC_DEF_H__
#define __LITTLE_RPC_DEF_H__

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>

#define LITTLE_RPC_ALLOC(size)             malloc(size)
#define LITTLE_RPC_FREE(pointer)           free(pointer)
#define LITTLE_RPC_MEMCPY(dest, src, size) memcpy(dest, src, size)

#define LITTLE_RPC_PENDDING_RPC_NUM 10
#define LITTLE_RPC_CACHE_SIZE       (16 * 1024)

typedef uint8_t LittleRPCServiceID;
typedef uint8_t LittleRPCMessageID;
typedef uint8_t LittleRPCSequence;
typedef int LittleRPCMethodIndex;

#endif