#include "pch.h"
#include "DBClientImpl.h"

#ifdef _DEBUG
void DBClientImpl::HelloAsync() {
    HelloCall* call = objectPool<HelloCall>::alloc();
    string HandShake("HandShake!");

    // 1. ��û �޽��� ��ü�� ���� �����ϰ� �����͸� ����
    S2D_Protocol::HelloRequest request;
    request.set_name(HandShake);

    // 2. �غ�� ��û ��ü�� PrepareAsyncSayHello �Լ��� ����
    call->response_reader = _stub->PrepareAsyncSayHello(&call->context, request, _cqRef.get());
    call->response_reader->StartCall();

    // 3. ������ ��ٸ��� CompletionQueue�� �±׸� ���
    call->response_reader->Finish(&call->reply, &call->status, (void*)call);
}
#endif

bool DBClientImpl::S2D_Login(shared_ptr<PBSession> sessionRef, string id, string password) {
    SLoginCall* call = objectPool<SLoginCall>::alloc(sessionRef);
    S2D_Protocol::S2D_Login request = S2DPacketMaker::Make_S2D_Login(id, password);

    //Call�� ó���ϴ� �� �� ����, FM����ϸ� CallData�� ��� �Լ��μ� ĸ��ȭ�ؾ���.
    call->response_reader = _stub->PrepareAsyncLoginRequest(&call->context, request, _cqRef.get());
    call->response_reader->StartCall();

    call->response_reader->Finish(&call->reply, &call->status, (void*)call);
    return true;
}

bool DBClientImpl::S2D_CreateAccount(shared_ptr<PBSession> sessionRef, string id, string password) {
    SCreateAccountCall* call = objectPool<SCreateAccountCall>::alloc(sessionRef);
    S2D_Protocol::S2D_CreateAccount request = S2DPacketMaker::Make_S2D_CreateAccount(id, password);

    call->response_reader = _stub->PrepareAsyncCreateAccountRequest(&call->context, request, _cqRef.get());
    call->response_reader->StartCall();

    call->response_reader->Finish(&call->reply, &call->status, (void*)call);
    return true;
}

bool DBClientImpl::S2D_PlayerInfomation(shared_ptr<PlayerSession> playerSessionRef, int32_t dbid) {
    SPlayerInformationCall* call = objectPool<SPlayerInformationCall>::alloc(playerSessionRef);
    S2D_Protocol::S2D_RequestPlayerInfomation request = S2DPacketMaker::Make_S2D_RequestPlayerInfomation(dbid);

    call->response_reader = _stub->PrepareAsyncPlayerInfomation(&call->context, request, _cqRef.get());
    call->response_reader->StartCall();

    call->response_reader->Finish(&call->reply, &call->status, (void*)call);
    return true;
}

bool DBClientImpl::S2D_RenewElo(shared_ptr<PlayerSession> playerSessionRef, int32_t dbid, int32_t gameId, int32_t elo) {
    SRenewEloCall* call = objectPool<SRenewEloCall>::alloc(playerSessionRef);
    S2D_Protocol::S2D_TryRenewElo request = S2DPacketMaker::Make_S2D_TryRenewElo(dbid, gameId, elo);

    call->response_reader = _stub->PrepareAsyncRenewElo(&call->context, request, _cqRef.get());
    call->response_reader->StartCall();

    call->response_reader->Finish(&call->reply, &call->status, (void*)call);
    return true;
}

bool DBClientImpl::S2D_RenewPersonalRecord(shared_ptr<PlayerSession> playerSessionRef, int32_t dbid, int32_t gameId, int32_t score) {
    SRenewPersonalRecordCall* call = objectPool<SRenewPersonalRecordCall>::alloc(playerSessionRef);
    S2D_Protocol::S2D_TryRenewPersonalRecord request = S2DPacketMaker::Make_S2D_TryRenewPersonalRecord(dbid, gameId, score);

    call->response_reader = _stub->PrepareAsyncRenewPersonalRecord(&call->context, request, _cqRef.get());
    call->response_reader->StartCall();

    call->response_reader->Finish(&call->reply, &call->status, (void*)call);
    return true;
}

bool DBClientImpl::S2D_PublicRecord(int32_t gameId) {
    SPublicRecordCall* call = objectPool<SPublicRecordCall>::alloc();
    S2D_Protocol::S2D_RequestPublicRecord request = S2DPacketMaker::Make_S2D_RequestPublicRecord(gameId);

    call->response_reader = _stub->PrepareAsyncPublicRecord(&call->context, request, _cqRef.get());
    call->response_reader->StartCall();

    call->response_reader->Finish(&call->reply, &call->status, (void*)call);
    return true;
}

void DBClientImpl::AsyncCompleteRpc() {
    if (_isConnected.load()) {
        void* tag;
        bool ok;

        grpc::CompletionQueue::NextStatus status = _cqRef->AsyncNext(&tag, &ok, chrono::system_clock::now() + chrono::milliseconds(2));

        switch (status) {
        case (grpc::CompletionQueue::GOT_EVENT): {
            S2D_CallData* pCallData = reinterpret_cast<S2D_CallData*>(tag);
            if (ok && pCallData->status.ok())
                pCallData->OnSucceed();
            else
                pCallData->OnFailed();
            pCallData->ReturnToPool();
            break;
        }
        case (grpc::CompletionQueue::TIMEOUT):
            break;
        case (grpc::CompletionQueue::SHUTDOWN):
            break;
        }
    }
}