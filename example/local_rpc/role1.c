#include "role1.h"
#include "test.pb-c.h"

#include "service_id_def.h"

static void service1__test_method1(TestService1_Service *service, const TestReq *input,
                                   TestRsp_Closure closure, void *closure_data);

static void service2__test_method2_callback(LittleRPCInvokeResult_t invokeResult,
                                            const ProtobufCMessage *msg, void *closure_data);

static TestService1_Service testService1 = TEST_SERVICE1__INIT(service1__);

static LittleRPC_t rpc1;

void Role1_Init(void)
{
    LittleRPC_Init(&rpc1);
    LittleRPC_RegistService(&rpc1, LRPC_SERVICE_ID_TEST_SERVICE_1,
                            (ProtobufCService *)&testService1);
}

size_t Role1_OnRecv(uint8_t *buff, size_t len)
{
    LittleRPC_OnRecv(&rpc1, buff, len);
}

void Role1_SetSendCallback(LittleRPCSendBufferCallback sendBufferCallback,
                           void *sendBufferCallbackUserData)
{
    LittleRPC_SetSendBufferCallback(&rpc1, sendBufferCallback, LITTLE_RPC_NULLPTR);
}

void Role1_InvokeService2Method2(uint32_t x)
{

    TestReq req = TEST_REQ__INIT;
    req.msg = "!..Call From Role1..!";
    req.x = x;
    LittleRPC_RpcInvoke(&rpc1, LRPC_SERVICE_ID_TEST_SERVICE_2, &test_service2__descriptor,
                        "testMethod2", (ProtobufCMessage *)&req, service2__test_method2_callback,
                        LITTLE_RPC_NULLPTR);
}

static void service1__test_method1(TestService1_Service *service, const TestReq *input,
                                   TestRsp_Closure closure, void *closure_data)
{
    printf("Role1: "
           "run service1 method1 -> ");
    printf("Role1: "
           "input: msg: %s, x: %d\r\n",
           input->msg, input->x);
    TestRsp rsp = TEST_RSP__INIT;
    rsp.retcode = 1024;
    closure(&rsp, closure_data);
}
static void service2__test_method2_callback(LittleRPCInvokeResult_t invokeResult,
                                            const ProtobufCMessage *msg, void *closure_data)
{
    TestRsp *rsp = (TestRsp *)msg;
    if (invokeResult == INVOKE_RESULT_FINISH)
    {
        printf("Role1: "
               "invokeResult: %d, service2 method2 -> rsp retCode: %d\r\n",
               invokeResult, rsp->retcode);
    }
    else
    {
        printf("Role1: "
               "invokeResult: %d\r\n",
               invokeResult);
    }
}
