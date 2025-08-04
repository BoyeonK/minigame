#pragma once

void ReadyForCall(S2D_Protocol::S2D_Service::AsyncService* service, grpc::ServerCompletionQueue* cq);

class CallData {
public:
    CallData(S2D_Protocol::S2D_Service::AsyncService* service, grpc::ServerCompletionQueue* cq)
        : _service(service), _completionQueueRef(cq), _status(CREATE), _ctx() {}
    virtual ~CallData() {}
    virtual void Proceed() = 0;
    virtual void ReturnToPool() = 0;

protected:
    // 모든 CallData 객체가 공통적으로 사용하는 멤버 변수
    S2D_Protocol::S2D_Service::AsyncService* _service;
    grpc::ServerCompletionQueue* _completionQueueRef;
    grpc::ServerContext _ctx;
    enum CallStatus { CREATE, PROCESS, FINISH };
    CallStatus _status;
};

class HelloCallData final : public CallData {
public:
    HelloCallData(S2D_Protocol::S2D_Service::AsyncService* service, grpc::ServerCompletionQueue* cq) 
        : CallData(service, cq), _responder(&_ctx) {

        Proceed();
    }

    void Proceed() override;

    void ReturnToPool() override { objectPool<HelloCallData>::dealloc(this); }

private:
    S2D_Protocol::HelloRequest _request;
    S2D_Protocol::HelloReply _reply;
    grpc::ServerAsyncResponseWriter<S2D_Protocol::HelloReply> _responder;
};

class DLoginCallData final : public CallData {
public:
    DLoginCallData(S2D_Protocol::S2D_Service::AsyncService* service, grpc::ServerCompletionQueue* cq)
        : CallData(service, cq), _responder(&_ctx) {

        Proceed();
    }

    void Proceed() override;

    void ReturnToPool() override { objectPool<DLoginCallData>::dealloc(this); }

private:
    S2D_Protocol::S2D_Login _request;
    S2D_Protocol::D2S_Login _reply;
    grpc::ServerAsyncResponseWriter<S2D_Protocol::D2S_Login> _responder;
};

class DCreateAccountCallData final : public CallData {
public:
    DCreateAccountCallData(S2D_Protocol::S2D_Service::AsyncService* service, grpc::ServerCompletionQueue* cq)
        : CallData(service, cq), _responder(&_ctx) {

        Proceed();
    }

    void Proceed() override;

    void ReturnToPool() override { objectPool<DCreateAccountCallData>::dealloc(this); }

private:
    S2D_Protocol::S2D_CreateAccount _request;
    S2D_Protocol::D2S_CreateAccount _reply;
    grpc::ServerAsyncResponseWriter<S2D_Protocol::D2S_CreateAccount> _responder;
};