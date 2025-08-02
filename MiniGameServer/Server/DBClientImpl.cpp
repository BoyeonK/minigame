#include "pch.h"
#include "DBClientImpl.h"

#ifdef _DEBUG
void DBClientImpl::HelloAsync() {
    HelloCall* call = objectPool<HelloCall>::alloc();
    string HandShake("HandShake!");

    // 1. 요청 메시지 객체를 먼저 생성하고 데이터를 설정
    S2D_Protocol::HelloRequest request;
    request.set_name(HandShake);

    // 2. 준비된 요청 객체를 PrepareAsyncSayHello 함수에 전달
    call->response_reader = _stub->PrepareAsyncSayHello(&call->context, request, _cqRef.get());
    call->response_reader->StartCall();

    // 3. 응답을 기다리며 CompletionQueue에 태그를 등록
    call->response_reader->Finish(&call->reply, &call->status, (void*)call);
}
#endif // _DEBUG