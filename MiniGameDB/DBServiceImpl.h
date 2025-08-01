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

private:
    // 워커 스레드가 실행할 함수
    void HandleRpcs() {
        void* tag;
        bool ok;

        while (_completionQueueRef->Next(&tag, &ok)) { // CompletionQueue에서 이벤트가 올 때까지 대기
            if (ok) {
                // 이벤트가 성공적으로 발생하면 CallData 객체의 Proceed() 호출
                static_cast<CallData*>(tag)->Proceed();
            }
            else {
                // 비동기 작업 실패 시 (예: 클라이언트 연결 끊김)
                delete static_cast<CallData*>(tag);
            }
        }
    }

    unique_ptr<grpc::ServerCompletionQueue> _completionQueueRef;
};