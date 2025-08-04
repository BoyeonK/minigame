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
        // ���ο� CallData�� CompletionQueue�� ���.
        DLoginCallData* newCallData = objectPool<DLoginCallData>::alloc(_service, _completionQueueRef);

        //TODO: DB�� ��ȸ�ؼ� �ش� ID�� ��ȸ.
            //ID�� ���°��, D2S_Login�� err ��Ƽ� ����.
            //ID�� �ִ°��, ������ ���� ���� password����, �ش� ID�� salt�� �����Ͽ� �ؽ�.
            //���� DB�� password���� ��.
                //�´°��, D2S_Login�� dbid�� �ش� ������ dbid�� �־� ����.
                //Ʋ�����, D2S_Login�� dbid�� 0�� �־� ����.

        _status = FINISH;
        _responder.Finish(_reply, grpc::Status::OK, this);
    }
    // ������ �ܰ�: RPC�� �Ϸ�� CallData�� Pool�� ��ȯ
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
        // ���ο� CallData�� CompletionQueue�� ���.
        DCreateAccountCallData* newCallData = objectPool<DCreateAccountCallData>::alloc(_service, _completionQueueRef);
        //TODO: �ش� ���̵�, �н������ �������� �õ�.
        //�����Ѱ�� D2S_CreateAccount�� true��� ����.
        //�����Ѱ�� false ��� ����.

        _status = FINISH;
        _responder.Finish(_reply, grpc::Status::OK, this);
    }
    // ������ �ܰ�: RPC�� �Ϸ�� CallData�� Pool�� ��ȯ
    else {
#ifdef _DEBUG
        cout << "Server: CreateAccount Request sequence complete!" << endl;
#endif 
        objectPool<DCreateAccountCallData>::dealloc(this);
    }
}
