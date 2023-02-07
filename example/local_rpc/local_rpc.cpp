#include <iostream>
#include <string.h>

#include "littlerpc/littlerpc.h"

#include "test.pb-c.h"

using namespace std;

#define LRPC_SERVICE_ID_TEST_SERVICE_1 1
#define LRPC_SERVICE_ID_TEST_SERVICE_2 2

static LittleRPC rpc1, rpc2;

static void service1__test_method1(TestService1_Service *service, const TestReq *input,
                                   TestRsp_Closure closure, void *closure_data);
static void service2__test_method2(TestService2_Service *service, const TestReq *input,
                                   TestRsp_Closure closure, void *closure_data);

static void service1__test_method1_on_return(const ProtobufCMessage *msg, void *closure_data);
static void service2__test_method2_on_return(const ProtobufCMessage *msg, void *closure_data);

static TestService1_Service testService1 = TEST_SERVICE1__INIT(service1__);
static TestService2_Service testService2 = TEST_SERVICE2__INIT(service2__);


static void sendToRpc1(uint8_t *buff, size_t len, void *userData) { rpc1.onRecv(buff, len); }

static void sendToRpc2(uint8_t *buff, size_t len, void *userData) { rpc2.onRecv(buff, len); }


int main(void)
{
    rpc1.setSendBufferCallback(sendToRpc2, LITTLE_RPC_NULLPTR);
    rpc2.setSendBufferCallback(sendToRpc1, LITTLE_RPC_NULLPTR);

    rpc1.registService(LRPC_SERVICE_ID_TEST_SERVICE_1, &testService1.base);
    rpc2.registService(LRPC_SERVICE_ID_TEST_SERVICE_2, &testService2.base);

    char c_chr[256];

    strcpy(c_chr, "Hello world");
    TestReq req = TEST_REQ__INIT;
    req.msg = c_chr;
    req.x = 123;

    LittleRPC::RPCInvokeRet_t ret;
    for (int i = 0; i < 11; i++)
    {
        req.x = i;
        ret = rpc1.RpcInvoke(LRPC_SERVICE_ID_TEST_SERVICE_2, &testService2.base, "testMethod2",
                             (ProtobufCMessage *)&req, service1__test_method1_on_return, LITTLE_RPC_NULLPTR);
        printf("rpc1 ret: %d\r\n", ret);
    }
    strcpy(c_chr, "Funk");
    TestReq req2 = TEST_REQ__INIT;
    req2.msg = c_chr;
    req2.x = 19901;

    rpc2.RpcInvoke(LRPC_SERVICE_ID_TEST_SERVICE_1, &testService1.base, "testMethod1",
                   (ProtobufCMessage *)&req2, service2__test_method2_on_return, LITTLE_RPC_NULLPTR);
}

void service1__test_method1_on_return(const ProtobufCMessage *msg, void *closure_data)
{
    TestRsp *rsp = (TestRsp *)msg;
    printf("service1 method1 -> rsp retCode: %d\r\n", rsp->retcode);
}

void service2__test_method2_on_return(const ProtobufCMessage *msg, void *closure_data)
{
    TestRsp *rsp = (TestRsp *)msg;
    printf("service2 method2 -> rsp retCode: %d\r\n", rsp->retcode);
}

static void service1__test_method1(TestService1_Service *service, const TestReq *input,
                                   TestRsp_Closure closure, void *closure_data)
{
    printf("run service1 method1 -> ");
    printf("input: msg: %s, x: %d\r\n", input->msg, input->x);
    TestRsp rsp = TEST_RSP__INIT;
    rsp.retcode = 1024;
    closure(&rsp, closure_data);
}

static void service2__test_method2(TestService2_Service *service, const TestReq *input,
                                   TestRsp_Closure closure, void *closure_data)
{
    printf("run service2 method2 -> ");
    printf("input: msg: %s, x: %d\r\n", input->msg, input->x);
    TestRsp rsp = TEST_RSP__INIT;
    closure(&rsp, closure_data);
}