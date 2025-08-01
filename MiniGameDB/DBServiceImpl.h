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
            // 첫 번째 단계: 새로운 RPC를 기다리도록 큐에 등록
            _status = PROCESS;
            _service->RequestSayHello(&_ctx, &_request, &_responder, _completionQueueRef, _completionQueueRef, this);
        }
        else if (_status == PROCESS) {
            // 두 번째 단계: RPC 요청이 도착하여 처리 시작
            // 다음 요청을 위해 새로운 CallData 객체를 등록
            new CallData(_service, _completionQueueRef);

            // 비즈니스 로직 수행
            string name = _request.name();
            _reply.set_message("Hello, " + name + " from async server!");

            cout << "Server: Received request from '" << string(name.begin(), name.end()) << "'" << endl;

            // 응답 전송을 비동기적으로 시작. 완료되면 FINISH 단계로 넘어갑니다.
            _status = FINISH;
            _responder.Finish(_reply, grpc::Status::OK, this);
        }
        else { // status_ == FINISH
         // 마지막 단계: RPC가 완전히 완료됨
            delete this; // CallData 객체 메모리 해제
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