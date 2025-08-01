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

        while (_completionQueueRef->Next(&tag, &ok)) { // CompletionQueue���� �̺�Ʈ�� �� ������ ���
            if (ok) {
                // ���� �� CallData ��ü�� Proceed() ȣ�� (���� �ܰ�� ����)
                static_cast<CallData*>(tag)->Proceed();
            }
            else {
                // ���� �� (��: Ŭ���̾�Ʈ ���� ����) pool�� ��ȯ.
                static_cast<CallData*>(tag)->ReturnToPool();
            }
        }
    }

private:
    unique_ptr<grpc::ServerCompletionQueue> _completionQueueRef;
};