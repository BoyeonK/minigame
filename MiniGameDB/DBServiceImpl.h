#pragma once
#include "CallData.h"

class GreeterServiceImpl final : public S2D_Protocol::S2D_Service::AsyncService {
public:
    GreeterServiceImpl() {}

    void setCompletionQueue(unique_ptr<grpc::ServerCompletionQueue> cqRef);

    grpc::ServerCompletionQueue* getPCompletionQueue() {
        return _completionQueueRef.get();
    }

    void HandleRpcs();

private:
    unique_ptr<grpc::ServerCompletionQueue> _completionQueueRef;
    atomic<bool> _isConnected = false;
};