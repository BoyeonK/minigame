#include "pch.h"
#include "CallData.h"

void ReadyForCall(S2D_Protocol::Greeter::AsyncService* service, grpc::ServerCompletionQueue* cq) {
    for (int i = 0; i < 5; i++) {
        objectPool<HelloCallData>::alloc(service, cq);
    }
}
