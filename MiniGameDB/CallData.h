#pragma once

class CallData {
public:
    CallData(S2D_Protocol::Greeter::AsyncService* service, grpc::ServerCompletionQueue* cq)
        : _service(service), _completionQueueRef(cq), _status(CREATE), _ctx() {}
    virtual ~CallData() {}
    virtual void Proceed() = 0;
    virtual void ReturnToPool() = 0;

protected:
    // ��� CallData ��ü�� ���������� ����ϴ� ��� ����
    S2D_Protocol::Greeter::AsyncService* _service;
    grpc::ServerCompletionQueue* _completionQueueRef;
    grpc::ServerContext _ctx;
    enum CallStatus { CREATE, PROCESS, FINISH };
    CallStatus _status;
};

class HelloCallData final : public CallData {
public:
    HelloCallData(S2D_Protocol::Greeter::AsyncService* service, grpc::ServerCompletionQueue* cq) 
        : CallData(service, cq), _responder(&_ctx) {

        Proceed();
    }

    void Proceed() override {
        // ù ��° �ܰ� : �����ڿ��� �����. CompletionQueue�� CallData�� ����Ѵ�.
        if (_status == CREATE) {
            _status = PROCESS;
            _service->RequestSayHello(&_ctx, &_request, &_responder, _completionQueueRef, _completionQueueRef, this);
        }
        // �� ��° �ܰ�: RPC ��û�� �����Ͽ� ó�� ����
        else if (_status == PROCESS) {
            // ���ο� CallData�� CompletionQueue�� ���.
            HelloCallData* newCallData = objectPool<HelloCallData>::alloc(_service, _completionQueueRef);

            // ����Ͻ� ���� ����
            string name = _request.name();
            _reply.set_message("Hello, " + name + " from async server!");

            cout << "Server: Received request from '" << string(name.begin(), name.end()) << "'" << endl;

            // ���� ������ �񵿱������� ����.
            _status = FINISH;
            _responder.Finish(_reply, grpc::Status::OK, this);
        }
        // ������ �ܰ�: RPC�� �Ϸ�� CallData�� Pool�� ��ȯ
        else {
            cout << "Server: Response sqeunce complete!" << endl;
            objectPool<HelloCallData>::dealloc(this);
        }
            
    }

    void ReturnToPool() override {
        objectPool<HelloCallData>::dealloc(this);
    }

private:
    S2D_Protocol::HelloRequest _request;
    S2D_Protocol::HelloReply _reply;
    grpc::ServerAsyncResponseWriter<S2D_Protocol::HelloReply> _responder;
};

void ReadyForCall(S2D_Protocol::Greeter::AsyncService* service, grpc::ServerCompletionQueue* cq);