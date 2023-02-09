#ifndef __ROLE_2_H__
#define __ROLE_2_H__

#include <stdint.h>
#include <stdlib.h>

#include "littlerpc/littlerpc.h"

void Role2_Init(void);

size_t Role2_OnRecv(uint8_t * buff, size_t len);

void Role2_SetSendCallback(LittleRPCSendBufferCallback sendBufferCallback, void *sendBufferCallbackUserData);

void Role2_InvokeService1Method1(uint32_t x);

#endif
