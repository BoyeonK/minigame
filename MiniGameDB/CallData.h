#pragma once

class CallData {
public:
    CallData(S2D_Protocol::Greeter::AsyncService* service, grpc::ServerCompletionQueue* cq)
        : _service(service), _completionQueueRef(cq), _status(CREATE), _ctx() {}
    virtual ~CallData() {}
    virtual void Proceed() = 0;
    virtual void ReturnToPool() = 0;

protected:
    // 모든 CallData 객체가 공통적으로 사용하는 멤버 변수
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
        // 첫 번째 단계 : 생성자에서 실행됨. CompletionQueue에 CallData를 등록한다.
        if (_status == CREATE) {
            _status = PROCESS;
            _service->RequestSayHello(&_ctx, &_request, &_responder, _completionQueueRef, _completionQueueRef, this);
        }
        // 두 번째 단계: RPC 요청이 도착하여 처리 시작
        else if (_status == PROCESS) {
            // 새로운 CallData를 CompletionQueue에 등록.
            HelloCallData* newCallData = objectPool<HelloCallData>::alloc(_service, _completionQueueRef);

            // 비즈니스 로직 수행
            string name = _request.name();
            _reply.set_message("Hello, " + name + " from async server!");

            cout << "Server: Received request from '" << string(name.begin(), name.end()) << "'" << endl;

            // 응답 전송을 비동기적으로 시작.
            _status = FINISH;
            _responder.Finish(_reply, grpc::Status::OK, this);
        }
        // 마지막 단계: RPC가 완료됨 CallData를 Pool에 반환
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