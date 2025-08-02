#include "pch.h"
#include "DBClientImpl.h"

#ifdef _DEBUG
void DBClientImpl::HelloAsync() {
    HelloCall* call = objectPool<HelloCall>::alloc();
    string HandShake("HandShake!");

    // 1. ��û �޽��� ��ü�� ���� �����ϰ� �����͸� ����
    S2D_Protocol::HelloRequest request;
    request.set_name(HandShake);

    // 2. �غ�� ��û ��ü�� PrepareAsyncSayHello �Լ��� ����
    call->response_reader = _stub->PrepareAsyncSayHello(&call->context, request, _cqRef.get());
    call->response_reader->StartCall();

    // 3. ������ ��ٸ��� CompletionQueue�� �±׸� ���
    call->response_reader->Finish(&call->reply, &call->status, (void*)call);
}
#endif // _DEBUG