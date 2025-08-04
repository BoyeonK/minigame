#pragma once
#include "S2D_CallData.h"

class DBClientImpl {
public:
    DBClientImpl(shared_ptr<grpc::Channel> channel) : 
        _stub(S2D_Protocol::Greeter::NewStub(channel)),
        _cqRef(make_unique<grpc::CompletionQueue>()) 
    {
        if (_cqRef) {
            _isConnected.store(true);
        }
    }

    ~DBClientImpl() {
        _cqRef->Shutdown();
    }

#ifdef _DEBUG
    void HelloAsync();
#endif

    void AsyncCompleteRpc() {
        if (_isConnected.load()) {
            void* tag;
            bool ok;

            grpc::CompletionQueue::NextStatus status = _cqRef->AsyncNext(&tag, &ok, chrono::system_clock::now() + chrono::milliseconds(2));

            switch (status) {
            case (grpc::CompletionQueue::GOT_EVENT): {
                S2D_CallData* pCallData = reinterpret_cast<S2D_CallData*>(tag);
                if (ok && pCallData->status.ok())
                    pCallData->OnSucceed();
                else
                    pCallData->OnFailed();
                pCallData->ReturnToPool();
                break;
            }
            case (grpc::CompletionQueue::TIMEOUT):
                break;
            case (grpc::CompletionQueue::SHUTDOWN): 
                break;
            }
        }
    }

private:
    unique_ptr<S2D_Protocol::Greeter::Stub> _stub;
    unique_ptr<grpc::CompletionQueue> _cqRef;
    atomic<bool> _isConnected = false;
};

