#pragma once
#include "DBServiceImpl.h"

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
    void ReadDbidFromPlayersTable(SQLHDBC& hDbc, SQLHSTMT& hStmt1, bool& flag, SQLINTEGER& dbid, const string& id);
    void ReadHashAndSaltFromAccountsTable(SQLHDBC& hDbc, SQLHSTMT& hStmt2, bool& flag2, SQLINTEGER& dbid);
    void Compare_PBKDF2(SQLHDBC& hDbc, SQLHSTMT& hStmt2, SQLINTEGER& dbid, const string& password);

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
    //void fSQL1(SQLHDBC& hDbc, SQLHSTMT& hStmt1, const string& id, bool& S1);
    void CreatePlayersTable(SQLHDBC& hDbc, SQLHSTMT& hStmt2, const string& id, bool& flag);
    void ReadDbidFromPlayersTable(SQLHDBC& hDbc, SQLHSTMT& hStmt3, const string& id, SQLINTEGER& dbid);
    void CreateAccountsTable(SQLHDBC& hDbc, SQLHSTMT& hStmt4, const string& password, SQLINTEGER& dbid);
    void CreateElosTable(SQLHDBC& hDbc, SQLHSTMT& hStmt5, SQLINTEGER& dbid);
    void CreatePersonalRecordsTable(SQLHDBC& hDbc, SQLHSTMT& hStmt6, SQLINTEGER& dbid);

    S2D_Protocol::S2D_CreateAccount _request;
    S2D_Protocol::D2S_CreateAccount _reply;
    grpc::ServerAsyncResponseWriter<S2D_Protocol::D2S_CreateAccount> _responder;
};

class DPlayerInfomationCallData final : public CallData {
public:
    DPlayerInfomationCallData(S2D_Protocol::S2D_Service::AsyncService* service, grpc::ServerCompletionQueue* cq)
        : CallData(service, cq), _responder(&_ctx) {

        Proceed();
    }

    void Proceed() override;
    void ReturnToPool() override { objectPool<DPlayerInfomationCallData>::dealloc(this); }

private:
    void ReadPlayerId(SQLHDBC& hDbc, SQLHSTMT& hStmt1, const int& dbid, wstring& playerId);
    void ReadElos(SQLHDBC& hDbc, SQLHSTMT& hStmt2, const int& dbid, vector<SQLINTEGER>& elos);
    void ReadPersonalRecords(SQLHDBC& hDbc, SQLHSTMT& hStmt2, const int& dbid, vector<SQLINTEGER>& personalRecords);

    S2D_Protocol::S2D_RequestPlayerInfomation _request;
    S2D_Protocol::D2C_ResponsePlayerInfomation _reply;
    grpc::ServerAsyncResponseWriter<S2D_Protocol::D2C_ResponsePlayerInfomation> _responder;
};