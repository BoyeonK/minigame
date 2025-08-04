#include "pch.h"
#include "DBServiceImpl.h"

void GreeterServiceImpl::setCompletionQueue(unique_ptr<grpc::ServerCompletionQueue> cqRef) {
    if (cqRef) {
        _completionQueueRef = move(cqRef);
        _isConnected.store(true);
        return;
    }
    _isConnected.store(false);
    throw runtime_error("invalid completionQueue pointer");
}

void GreeterServiceImpl::HandleRpcs() {
    if (_isConnected.load()) {
        void* tag;
        bool ok;
        grpc::CompletionQueue::NextStatus status = _completionQueueRef.get()->AsyncNext(&tag, &ok, chrono::system_clock::now() + chrono::milliseconds(20));

        switch (status) {
        case (grpc::CompletionQueue::GOT_EVENT):
            if (ok)
                // 성공 시 CallData 객체의 Proceed() 호출 (다음 단계로 진행)
                static_cast<CallData*>(tag)->Proceed();
            else
                // 실패 시 pool로 반환.
                static_cast<CallData*>(tag)->ReturnToPool();
            break;

        case grpc::CompletionQueue::TIMEOUT:
            // 타임아웃 시 다른 작업을 수행 (이후 루프의 다음 순서로 계속 진행)
            break;

        case grpc::CompletionQueue::SHUTDOWN:
            _isConnected.store(false);
            break;
        }
    }
}