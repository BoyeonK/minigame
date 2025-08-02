#pragma once
#include "S2D_CallData.h"

class DBClientImpl {
public:
    DBClientImpl(shared_ptr<grpc::Channel> channel) : 
        _stub(S2D_Protocol::Greeter::NewStub(channel)),
        _cqRef(make_unique<grpc::CompletionQueue>())
    { }

    ~DBClientImpl() {
        _cqRef->Shutdown();
    }

    void SayHelloAsync(const string& user) {
        HelloCall* call = new HelloCall();

        // 1. 요청 메시지 객체를 먼저 생성하고 데이터를 설정
        S2D_Protocol::HelloRequest request;
        request.set_name(user);

        // 2. 준비된 요청 객체를 PrepareAsyncSayHello 함수에 전달
        call->response_reader = _stub->PrepareAsyncSayHello(&call->context, request, _cqRef.get());
        call->response_reader->StartCall();

        // 3. 응답을 기다리며 CompletionQueue에 태그를 등록
        call->response_reader->Finish(&call->reply, &call->status, (void*)call);
    }

    void AsyncCompleteRpc() {
        void* tag;
        bool ok;
        while (_cqRef->Next(&tag, &ok)) {
            S2D_CallData* call = reinterpret_cast<S2D_CallData*>(tag);

            if (ok && call->status.ok()) {
                call->OnSucceed();
            }
            else {
                call->OnFailed();
            }
            delete call; // RPC가 완료되면 객체 해제
        }
    }

private:
    unique_ptr<S2D_Protocol::Greeter::Stub> _stub;
    unique_ptr<grpc::CompletionQueue> _cqRef;
};

