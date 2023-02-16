#ifndef __LITTLE_RPC_CONF_H__
#define __LITTLE_RPC_CONF_H__

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>

#define LITTLE_RPC_CONF_ALLOC(size) malloc(size)
#define LITTLE_RPC_CONF_FREE(pointer) free(pointer)
#define LITTLE_RPC_CONF_MEMCPY(dest, src, size) memcpy(dest, src, size)

#define LITTLE_RPC_CONF_MAX_SERVICE_NUM (8)
#define LITTLE_RPC_CONF_PENDDING_RPC_NUM (10)
#define LITTLE_RPC_CONF_CACHE_SIZE (16 * 1024)

#define LITTLE_RPC_CONF_NULLPTR NULL

#define LITTLE_RPC_CONF_PACK_VERIFY 0

#define LITTLE_RPC_CONF_ENABLE_TIMEOUT 1

#if LITTLE_RPC_CONF_ENABLE_TIMEOUT == 1

#define LITTLE_RPC_CONF_TIMEOUT_RECV_PACKAGE 1000
#define LITTLE_RPC_CONF_TIMEOUT_WAIT_RESPONSE 5000

#define LITTLE_RPC_CONF_TICK_TYPE uint64_t

#include <sys/time.h>
#define LITTLE_RPC_CONF_TIMEOUT_GET_TICK(pTick) \
    do {                                        \
        struct timeval tv;                      \
        gettimeofday(&tv, NULL);                \
        *pTick = tv.tv_sec * 1000 + tv.tv_usec; \
    } while (0)
#endif // if LITTLE_RPC_CONF_ENABLE_TIMEOUT == 1

#define LITTLE_RPC_CONF_THREAD_SAFE 0

#if LITTLE_RPC_CONF_THREAD_SAFE != 0
#define LITTLE_RPC_CONF_LOCK_ACQUIRE() acquire()
#define LITTLE_RPC_CONF_LOCK_RELEASE(lock) release(lock)
#endif // if LITTLE_RPC_CONF_THREAD_SAFE != 0

// Service ID can not be ZERO
#define LITTLE_RPC_CONF_GET_SERVICE_ID(serviceDesc, pID) \
    do {                                                 \
        if (serviceDesc == service1) {                   \
            *pID = 0x01;                                 \
        } else if (serviceDesc == service2) {            \
            *pID = 0x02;                                 \
        }                                                \
    } while (0)

#endif // ifndef __LITTLE_RPC_CONF_H__