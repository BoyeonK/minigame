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
                // ���� �� CallData ��ü�� Proceed() ȣ�� (���� �ܰ�� ����)
                static_cast<CallData*>(tag)->Proceed();
            else
                // ���� �� pool�� ��ȯ.
                static_cast<CallData*>(tag)->ReturnToPool();
            break;

        case grpc::CompletionQueue::TIMEOUT:
            // Ÿ�Ӿƿ� �� �ٸ� �۾��� ���� (���� ������ ���� ������ ��� ����)
            break;

        case grpc::CompletionQueue::SHUTDOWN:
            _isConnected.store(false);
            break;
        }
    }
}