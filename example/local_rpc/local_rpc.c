#include <string.h>

#include "littlerpc/littlerpc.h"
#include "role1.h"
#include "role2.h"



static void sendToRole1(uint8_t *buff, size_t len, void *userData)
{
    Role1_OnRecv(buff, len);
}

static void sendToRole2(uint8_t *buff, size_t len, void *userData)
{
    Role2_OnRecv(buff, len);

}


int main(void)
{
    Role1_Init();
    Role1_SetSendCallback(sendToRole2, NULL);

    Role2_Init();
    Role2_SetSendCallback(sendToRole1, NULL);

    for (int i = 0; i < 100000; i ++){
        Role1_InvokeService2Method2(i);
        Role2_InvokeService1Method1(i);
    }

}