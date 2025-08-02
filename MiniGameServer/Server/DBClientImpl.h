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

#ifdef _DEBUG
    void HelloAsync();
#endif

    void AsyncCompleteRpc() {
        void* tag;
        bool ok;
        if (_cqRef->Next(&tag, &ok)) {
            S2D_CallData* pCallData = reinterpret_cast<S2D_CallData*>(tag);

            if (ok && pCallData->status.ok())
                pCallData->OnSucceed();
            else {
                pCallData->OnFailed();
                throw runtime_error("HandShake Failed");
            }
            pCallData->ReturnToPool(); // RPC가 완료되면 객체 해제
        }
    }

private:
    unique_ptr<S2D_Protocol::Greeter::Stub> _stub;
    unique_ptr<grpc::CompletionQueue> _cqRef;
};

