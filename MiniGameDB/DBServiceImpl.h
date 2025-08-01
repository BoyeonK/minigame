#pragma once
#include "CallData.h"

class GreeterServiceImpl final : public S2D_Protocol::Greeter::AsyncService {
public:
    GreeterServiceImpl() {}

    void setCompletionQueue(unique_ptr<grpc::ServerCompletionQueue> cqRef) {
        if (cqRef) {
            _completionQueueRef = move(cqRef);
            return;
        }
        throw runtime_error("invalid completionQueue pointer");
    }

    grpc::ServerCompletionQueue* getPCompletionQueue () {
        return _completionQueueRef.get();
    }

    void HandleRpcs() {
        void* tag;
        bool ok;

        while (_completionQueueRef->Next(&tag, &ok)) { // CompletionQueue에서 이벤트가 올 때까지 대기
            if (ok) {
                // 성공 시 CallData 객체의 Proceed() 호출 (다음 단계로 진행)
                static_cast<CallData*>(tag)->Proceed();
            }
            else {
                // 실패 시 (예: 클라이언트 연결 끊김) pool로 반환.
                static_cast<CallData*>(tag)->ReturnToPool();
            }
        }
    }

private:
    unique_ptr<grpc::ServerCompletionQueue> _completionQueueRef;
};