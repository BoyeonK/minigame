#include "pch.h"
#include "CallData.h"

void ReadyForCall(S2D_Protocol::S2D_Service::AsyncService* service, grpc::ServerCompletionQueue* cq) {
    for (int i = 0; i < 3; i++) {
        objectPool<HelloCallData>::alloc(service, cq);
        objectPool<DLoginCallData>::alloc(service, cq);
        objectPool<DCreateAccountCallData>::alloc(service, cq);
    }
}

void HelloCallData::Proceed() {
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
        cout << "Server: Response sequence complete!" << endl;
        objectPool<HelloCallData>::dealloc(this);
    }
}

void DLoginCallData::Proceed() {
    if (_status == CREATE) {
        _status = PROCESS;
        _service->RequestLoginRequest(&_ctx, &_request, &_responder, _completionQueueRef, _completionQueueRef, this);
    }
    else if (_status == PROCESS) {
        // 새로운 CallData를 CompletionQueue에 등록.
        DLoginCallData* newCallData = objectPool<DLoginCallData>::alloc(_service, _completionQueueRef);

        //TODO: DB를 조회해서 해당 ID를 조회.
            //ID가 없는경우, D2S_Login에 err 담아서 전송.
            //ID가 있는경우, 서버로 부터 받은 password값을, 해당 ID의 salt와 조합하여 해싱.
            //이후 DB의 password값과 비교.
                //맞는경우, D2S_Login의 dbid에 해당 유저의 dbid를 넣어 전송.
                //틀린경우, D2S_Login의 dbid에 0을 넣어 전송.

        _status = FINISH;
        _responder.Finish(_reply, grpc::Status::OK, this);
    }
    // 마지막 단계: RPC가 완료됨 CallData를 Pool에 반환
    else {
#ifdef _DEBUG
        cout << "Server: Login Request sequence complete!" << endl;
#endif 
        objectPool<DLoginCallData>::dealloc(this);
    }
}

void DCreateAccountCallData::Proceed() {
    if (_status == CREATE) {
        _status = PROCESS;
        _service->RequestCreateAccountRequest(&_ctx, &_request, &_responder, _completionQueueRef, _completionQueueRef, this);
    }
    else if (_status == PROCESS) {
        // 새로운 CallData를 CompletionQueue에 등록.
        DCreateAccountCallData* newCallData = objectPool<DCreateAccountCallData>::alloc(_service, _completionQueueRef);
        //TODO: 해당 아이디, 패스워드로 계정생성 시도.
        //성공한경우 D2S_CreateAccount에 true담아 전송.
        //실패한경우 false 담아 전송.

        _status = FINISH;
        _responder.Finish(_reply, grpc::Status::OK, this);
    }
    // 마지막 단계: RPC가 완료됨 CallData를 Pool에 반환
    else {
#ifdef _DEBUG
        cout << "Server: CreateAccount Request sequence complete!" << endl;
#endif 
        objectPool<DCreateAccountCallData>::dealloc(this);
    }
}
