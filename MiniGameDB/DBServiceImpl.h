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
    // ��Ŀ �����尡 ������ �Լ�
    void HandleRpcs() {
        void* tag;
        bool ok;

        while (_completionQueueRef->Next(&tag, &ok)) { // CompletionQueue���� �̺�Ʈ�� �� ������ ���
            if (ok) {
                // �̺�Ʈ�� ���������� �߻��ϸ� CallData ��ü�� Proceed() ȣ��
                static_cast<CallData*>(tag)->Proceed();
            }
            else {
                // �񵿱� �۾� ���� �� (��: Ŭ���̾�Ʈ ���� ����)
                delete static_cast<CallData*>(tag);
            }
        }
    }

    unique_ptr<grpc::ServerCompletionQueue> _completionQueueRef;
};