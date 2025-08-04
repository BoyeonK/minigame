#include "pch.h"
#include "CallData.h"

void ReadyForCall(S2D_Protocol::S2D_Service::AsyncService* service, grpc::ServerCompletionQueue* cq) {
    for (int i = 0; i < 5; i++) {
        objectPool<HelloCallData>::alloc(service, cq);
    }
}
