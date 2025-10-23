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

#ifdef _DEBUG
    void HelloAsync();
#endif
    bool S2D_Login(shared_ptr<PBSession> sessionRef, string id, string password);
    bool S2D_CreateAccount(shared_ptr<PBSession> sessionRef, string id, string password);
    bool S2D_RenewElos(shared_ptr<PlayerSession> playerSessionRef, int dbid);
    bool S2D_PlayerInfomation(shared_ptr<PlayerSession> playerSessionRef, int dbid);

    void AsyncCompleteRpc();

private:
    unique_ptr<S2D_Protocol::S2D_Service::Stub> _stub;
    unique_ptr<grpc::CompletionQueue> _cqRef;
    atomic<bool> _isConnected = false;
};

