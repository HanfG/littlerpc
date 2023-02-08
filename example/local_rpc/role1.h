#ifndef __ROLE_1_H__
#define __ROLE_1_H__

#include <stdint.h>
#include <stdlib.h>

#include "littlerpc/littlerpc.h"

void Role1_Init(void);

size_t Role1_OnRecv(uint8_t * buff, size_t len);

void Role1_SetSendCallback(LittleRPCSendBufferCallback sendBufferCallback, void *sendBufferCallbackUserData);

void Role1_InvokeService2Method2(uint32_t x);

#endif
