#ifndef __LITTLE_RPC_CONF_H__
#define __LITTLE_RPC_CONF_H__

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>

#define LITTLE_RPC_CONF_ALLOC(size)             malloc(size)
#define LITTLE_RPC_CONF_FREE(pointer)           free(pointer)
#define LITTLE_RPC_CONF_MEMCPY(dest, src, size) memcpy(dest, src, size)

#define LITTLE_RPC_CONF_PENDDING_RPC_NUM  (10)
#define LITTLE_RPC_CONF_INIT_CACHE_STATIC 1
#define LITTLE_RPC_CONF_CACHE_SIZE        (16 * 1024)

#define LITTLE_RPC_CONF_NULLPTR NULL

#define LITTLE_RPC_CONF_PACK_VERIFY 0

#endif  // ifndef __LITTLE_RPC_CONF_H__