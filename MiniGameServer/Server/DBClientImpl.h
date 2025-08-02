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

        // 1. ��û �޽��� ��ü�� ���� �����ϰ� �����͸� ����
        S2D_Protocol::HelloRequest request;
        request.set_name(user);

        // 2. �غ�� ��û ��ü�� PrepareAsyncSayHello �Լ��� ����
        call->response_reader = _stub->PrepareAsyncSayHello(&call->context, request, _cqRef.get());
        call->response_reader->StartCall();

        // 3. ������ ��ٸ��� CompletionQueue�� �±׸� ���
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
            delete call; // RPC�� �Ϸ�Ǹ� ��ü ����
        }
    }

private:
    unique_ptr<S2D_Protocol::Greeter::Stub> _stub;
    unique_ptr<grpc::CompletionQueue> _cqRef;
};

