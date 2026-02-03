#pragma once
#include "S2D_CallData.h"
#include "S2DPacketMaker.h"
#include "PlayerSession.h"

class DBClientImpl {
public:
    DBClientImpl(shared_ptr<grpc::Channel> channel) : 
        _stub(S2D_Protocol::S2D_Service::NewStub(channel)),
        _cqRef(make_unique<grpc::CompletionQueue>()) 
    {
        if (_cqRef) {
            _isConnected.store(true);
        }
    }

    ~DBClientImpl() {
        _cqRef->Shutdown();
    }

    void HelloAsync();
    bool S2D_Login(shared_ptr<PBSession> sessionRef, string id, string password);
    bool S2D_CreateAccount(shared_ptr<PBSession> sessionRef, string id, string password);
    bool S2D_PlayerInfomation(shared_ptr<PlayerSession> playerSessionRef, int32_t dbid);
    bool S2D_UpdateElo(int32_t dbid, int32_t gameId, int32_t elo);
    bool S2D_UpdatePersonalRecord(shared_ptr<PlayerSession> playerSessionRef, int32_t dbid, int32_t gameId, int32_t score);
    bool S2D_PublicRecord(int32_t gameId);
    bool S2D_UpdatePublicRecord(int32_t gameId, int32_t dbid, int32_t score);

    void AsyncCompleteRpc();

private:
    unique_ptr<S2D_Protocol::S2D_Service::Stub> _stub;
    unique_ptr<grpc::CompletionQueue> _cqRef;
    atomic<bool> _isConnected = false;
};

