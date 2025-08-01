#pragma once
#include "S2D_Protocol.grpc.pb.h"

class CallData {
public:
    CallData(S2D_Protocol::Greeter::AsyncService* service, grpc::ServerCompletionQueue* cq)
        : _service(service), _completionQueueRef(cq), _responder(&_ctx), _status(CREATE) {
        Proceed();
    }

    void Proceed() {
        if (_status == CREATE) {
            // ù ��° �ܰ�: ���ο� RPC�� ��ٸ����� ť�� ���
            _status = PROCESS;
            _service->RequestSayHello(&_ctx, &_request, &_responder, _completionQueueRef, _completionQueueRef, this);
        }
        else if (_status == PROCESS) {
            // �� ��° �ܰ�: RPC ��û�� �����Ͽ� ó�� ����
            // ���� ��û�� ���� ���ο� CallData ��ü�� ���
            new CallData(_service, _completionQueueRef);

            // ����Ͻ� ���� ����
            string name = _request.name();
            _reply.set_message("Hello, " + name + " from async server!");

            cout << "Server: Received request from '" << string(name.begin(), name.end()) << "'" << endl;

            // ���� ������ �񵿱������� ����. �Ϸ�Ǹ� FINISH �ܰ�� �Ѿ�ϴ�.
            _status = FINISH;
            _responder.Finish(_reply, grpc::Status::OK, this);
        }
        else { // status_ == FINISH
         // ������ �ܰ�: RPC�� ������ �Ϸ��
            delete this; // CallData ��ü �޸� ����
        }
    }

private:
    S2D_Protocol::Greeter::AsyncService* _service;
    grpc::ServerCompletionQueue* _completionQueueRef;
    grpc::ServerContext _ctx;
    S2D_Protocol::HelloRequest _request;
    S2D_Protocol::HelloReply _reply;
    grpc::ServerAsyncResponseWriter<S2D_Protocol::HelloReply> _responder;
    enum CallStatus { CREATE, PROCESS, FINISH };
    CallStatus _status;
};

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