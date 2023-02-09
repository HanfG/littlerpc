#include "role2.h"

#include "test.pb-c.h"

#include "service_id_def.h"

static void service2__test_method2(TestService2_Service *service, const TestReq *input,
                                   TestRsp_Closure closure, void *closure_data);

static void service1__test_method1_callback(LittleRPCInvokeResult_t invokeResult,
                                            const ProtobufCMessage *msg, void *closure_data);

static TestService2_Service testService2 = TEST_SERVICE2__INIT(service2__);

static LittleRPC_t rpc2;

void Role2_Init(void)
{
    LittleRPC_Init(&rpc2);

    LittleRPC_RegistService(&rpc2, LRPC_SERVICE_ID_TEST_SERVICE_2,
                            (ProtobufCService *)&testService2);
}

size_t Role2_OnRecv(uint8_t *buff, size_t len)
{
    LittleRPC_OnRecv(&rpc2, buff, len);
}

void Role2_SetSendCallback(LittleRPCSendBufferCallback sendBufferCallback,
                           void *sendBufferCallbackUserData)
{
    LittleRPC_SetSendBufferCallback(&rpc2, sendBufferCallback, LITTLE_RPC_NULLPTR);
}

void Role2_InvokeService1Method1(uint32_t x)
{

    TestReq req = TEST_REQ__INIT;
    req.msg = "!..Call From Role2..!";
    req.x = x;
    LittleRPC_RpcInvoke(&rpc2, LRPC_SERVICE_ID_TEST_SERVICE_1, &test_service1__descriptor,
                        "testMethod1", (ProtobufCMessage *)&req, service1__test_method1_callback,
                        LITTLE_RPC_NULLPTR);
}

void service1__test_method1_callback(LittleRPCInvokeResult_t invokeResult,
                                     const ProtobufCMessage *msg, void *closure_data)
{
    TestRsp *rsp = (TestRsp *)msg;
    if (invokeResult == INVOKE_RESULT_FINISH)
    {
        printf("Role2: "
               "invokeResult:%d, service1 method1 -> rsp retCode: %d\r\n",
               invokeResult, rsp->retcode);
    }
    else
    {
        printf("Role2: "
               "invokeResult:%d\r\n",
               invokeResult);
    }
}


static void service2__test_method2(TestService2_Service *service, const TestReq *input,
                                   TestRsp_Closure closure, void *closure_data)
{
    printf("Role2: "
           "run service2 method2 -> ");
    printf("Role2: "
           "input: msg: %s, x: %d\r\n",
           input->msg, input->x);
    TestRsp rsp = TEST_RSP__INIT;
    rsp.retcode = input->x;
    closure(&rsp, closure_data);
}
