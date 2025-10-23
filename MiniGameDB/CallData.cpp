#include "pch.h"
#include "CallData.h"
#include "GlobalVariables.h"
#include <openssl/rand.h>
#include <openssl/evp.h>

void ReadyForCall(S2D_Protocol::S2D_Service::AsyncService* service, grpc::ServerCompletionQueue* cq) {
    for (int i = 0; i < 3; i++) {
        objectPool<HelloCallData>::alloc(service, cq);
        objectPool<DLoginCallData>::alloc(service, cq);
        objectPool<DCreateAccountCallData>::alloc(service, cq);
        objectPool<DRenewElosCallData>::alloc(service, cq);
    }
}

void HelloCallData::Proceed() {
    // 첫 번째 레슨 : 생성자에서 실행됨. CompletionQueue에 CallData를 등록한다.
    if (_status == CREATE) {
        _status = PROCESS;
        _service->RequestSayHello(&_ctx, &_request, &_responder, _completionQueueRef, _completionQueueRef, this);
    }
    // 두 번째 레슨: RPC 요청이 도착하여 처리 시작
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
    // 세번째 레슨: RPC가 완료됨 CallData를 Pool에 반환
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
        DLoginCallData* newCallData = objectPool<DLoginCallData>::alloc(_service, _completionQueueRef);

        string id = _request.id();
        string password = _request.password();
        grpc::Status stat = grpc::Status::OK;
        
        try {
            SQLHSTMT hStmt1 = nullptr;
            SQLHSTMT hStmt2 = nullptr;
            SQLHDBC hDbc = GDBManager->PopHDbc();

            //스코프를 벗어날때 자원을 해제해 줄 친구들
            Cleaner hDbcCleaner([=]() { GDBManager->ReturnHDbc(hDbc); });
            Cleaner hStmtCleaner([=]() {
                if (hStmt1 != nullptr)
                    SQLFreeHandle(SQL_HANDLE_STMT, hStmt1);
                if (hStmt2 != nullptr)
                    SQLFreeHandle(SQL_HANDLE_STMT, hStmt2);
            });

            SQLINTEGER dbid = -1;
            bool flag = false;
            bool flag2 = false;

            //해당 player_id로 dbid 조회. 해당 player_id를 가진 column이 없을경우, incorrect_id를 직렬화.
            ReadDbidFromPlayersTable(hDbc, hStmt1, flag, dbid, id);

            //fSQL1의 결과가 존재할 경우, 해당 dbid column의 Accounts Table을 Fetch.
            if (flag == true) ReadHashAndSaltFromAccountsTable(hDbc, hStmt2, flag2, dbid);

            //Account Table이 존재하는 경우(Fetch 성공), Table의 password_hash와 salt를 가져옴.
            //입력값 password와 해싱하여 비교 및 결과에 따라 _reply에 알맞는 dbid 직렬화
            if (flag2 == true) Compare_PBKDF2(hDbc, hStmt2, dbid, password);
        }
        catch (runtime_error& e) {
            cout << e.what() << endl;
        }

        _status = FINISH;
        _responder.Finish(_reply, stat, this);
    }
    // 마지막 단계: RPC가 완료됨 CallData를 Pool에 반환
    else {
#ifdef _DEBUG
        cout << "Server: Login Request sequence complete!" << endl;
#endif 
        objectPool<DLoginCallData>::dealloc(this);
    }
}

void DLoginCallData::ReadDbidFromPlayersTable(SQLHDBC& hDbc, SQLHSTMT& hStmt1, bool& flag, SQLINTEGER& dbid, const string& id) {
    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt1);
    if (!GDBManager->CheckReturn(ret, SQL_HANDLE_DBC, hDbc)) {
        throw runtime_error("S2D_Login : hStmt1 Alloc Failed");
    }

    wstring query = L"SELECT dbid FROM Players WHERE player_id = ?";
    ret = SQLPrepareW(hStmt1, (SQLWCHAR*)query.c_str(), SQL_NTS);
    if (!GDBManager->CheckReturn(ret, SQL_HANDLE_STMT, hStmt1)) {
        throw runtime_error("S2D_Login : S1 Query Setting Failed");
    }

    wstring wId = GDBManager->s2wsRef(id);

    SQLLEN idLen = SQL_NTS;
    ret = SQLBindParameter(hStmt1, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, wId.size(), 0, (SQLPOINTER)wId.c_str(), 0, &idLen);
    if (!GDBManager->CheckReturn(ret, SQL_HANDLE_STMT, hStmt1)) {
        throw runtime_error("S2D_Login : S1 Bind Parameter Failed");
    }

    ret = SQLExecute(hStmt1);
    if (!GDBManager->CheckReturn(ret, SQL_HANDLE_STMT, hStmt1)) {
        throw runtime_error("S2D_Login : S1 Execute Failed");
    }

    ret = SQLFetch(hStmt1);
    // SQL문 성공은 했다 (데이터 없음) or SQL문 성공해서 결과를 들고왔다
    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
        ret = SQLGetData(hStmt1, 1, SQL_C_SLONG, &dbid, sizeof(dbid), NULL);
        // 정상 동작 1, 가져오는데 성공
        if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
            flag = true;
        }
        //? 있는데 보여주긴 싫음? 이건 무슨상황
        else {
            throw runtime_error("S2D_Login : S1 GetData Failed");
        }
    }
    // 정상 동작 2, 일치하는 데이터가 없음. DB잘못이 아님. 사람 잘못.
    else if (ret == SQL_NO_DATA) {
        _reply.set_incorrect_id(true);
    }
    // SQL문 Fetch에 실패함.
    else {
        throw runtime_error("S2D_Login : S1 Fetch Failed");
    }
}

void DLoginCallData::ReadHashAndSaltFromAccountsTable(SQLHDBC& hDbc, SQLHSTMT& hStmt2, bool& flag2, SQLINTEGER& dbid) {
    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt2);
    if (!GDBManager->CheckReturn(ret, SQL_HANDLE_DBC, hDbc)) {
        throw runtime_error("S2D_Login : hStmt2 Alloc Failed");
    }

    wstring query = L"SELECT password_hash, salt FROM Accounts WHERE dbid = ?";
    ret = SQLPrepareW(hStmt2, (SQLWCHAR*)query.c_str(), SQL_NTS);
    if (!GDBManager->CheckReturn(ret, SQL_HANDLE_STMT, hStmt2)) {
        throw runtime_error("S2D_Login : S2 Query Setting Failed");
    }

    int targetDbid = dbid;
    ret = SQLBindParameter(hStmt2, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &targetDbid, 0, NULL);
    if (!GDBManager->CheckReturn(ret, SQL_HANDLE_STMT, hStmt2)) {
        throw runtime_error("S2D_Login : S2 Bind Parameter Failed");
    }

    ret = SQLExecute(hStmt2);
    if (!GDBManager->CheckReturn(ret, SQL_HANDLE_STMT, hStmt2)) {
        throw runtime_error("S2D_Login : S2 Execute Failed");
    }

    ret = SQLFetch(hStmt2);
    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
        flag2 = true;
    }
    else if (ret == SQL_NO_DATA) {
        //방금 조회한 db가.. 없어?
        throw runtime_error("S2D_Login : Table Disappeared. Maybe, by Other Thread.");
    }
    else {
        throw runtime_error("S2D_Login : S2 Fetch Failed.");
    }
}

void DLoginCallData::Compare_PBKDF2(SQLHDBC& hDbc, SQLHSTMT& hStmt2, SQLINTEGER& dbid, const string& password) {
    const size_t HASH_SIZE = 64;
    const size_t SALT_SIZE = 32;
    wstring password_hash(HASH_SIZE, L' ');
    wstring salt(SALT_SIZE, L' ');
    SQLLEN hash_ind, salt_ind;

    // password_hash와 salt 데이터를 가져옴
    SQLRETURN ret = SQLGetData(hStmt2, 1, SQL_C_WCHAR, password_hash.data(), (HASH_SIZE + 1) * sizeof(wchar_t), &hash_ind);
    if (!GDBManager->CheckReturn(ret, SQL_HANDLE_STMT, hStmt2)) {
        throw runtime_error("Failed to get password_hash data.");
    }
    password_hash.resize(hash_ind / sizeof(wchar_t));

    ret = SQLGetData(hStmt2, 2, SQL_C_WCHAR, salt.data(), (SALT_SIZE + 1) * sizeof(wchar_t), &salt_ind);
    if (!GDBManager->CheckReturn(ret, SQL_HANDLE_STMT, hStmt2)) {
        throw runtime_error("Failed to get salt data.");
    }
    salt.resize(salt_ind / sizeof(wchar_t));

    vector<unsigned char> requestedVHash(DBManager::hash_size);
    vector<unsigned char> vSalt = GDBManager->ws2vRef(salt);

    if (vSalt.size() != DBManager::salt_size) {
        throw runtime_error("S2D_Login : Salt size is different from DB Setting");
    }

    if (PKCS5_PBKDF2_HMAC(
        password.c_str(),
        password.length(),
        vSalt.data(),
        vSalt.size(),
        DBManager::pbkdf2_iter,
        EVP_sha256(),
        DBManager::hash_size,
        requestedVHash.data()
    ) != 1) {
        throw runtime_error("S2D_Login : PBKDF5 Failed");
    }

    wstring requestedWHash = GDBManager->v2wsRef(requestedVHash);
    if (requestedWHash == password_hash) {
        _reply.set_dbid(dbid);
    }
    else {
        _reply.set_dbid(0);
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

        string id = _request.id();
        string password = _request.password();      
        
        try {
            SQLHDBC hDbc = GDBManager->PopHDbc();
            if (hDbc == nullptr) {
                throw runtime_error("Invalid hDbc");
            }
            bool attr = false;
            Cleaner hDbcCleaner([&]() {
                if (attr) SQLEndTran(SQL_HANDLE_DBC, hDbc, SQL_ROLLBACK);
                GDBManager->ReturnHDbc(hDbc);
            });

            SQLHSTMT hStmt1 = nullptr , hStmt2 = nullptr, hStmt3 = nullptr, hStmt4 = nullptr, hStmt5 = nullptr;
            Cleaner hStmtCleaner([&]() {
                if (hStmt1 != nullptr) SQLFreeHandle(SQL_HANDLE_STMT, hStmt1);
                if (hStmt2 != nullptr) SQLFreeHandle(SQL_HANDLE_STMT, hStmt2);
                if (hStmt3 != nullptr) SQLFreeHandle(SQL_HANDLE_STMT, hStmt3);
                if (hStmt4 != nullptr) SQLFreeHandle(SQL_HANDLE_STMT, hStmt4);
                if (hStmt5 != nullptr) SQLFreeHandle(SQL_HANDLE_STMT, hStmt5);
            });

            bool isSuccess = false;
            
            //트랜잭션 설정
            SQLRETURN ret = SQLSetConnectAttr(hDbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_OFF, SQL_IS_UINTEGER);
            if (!GDBManager->CheckReturn(ret, SQL_HANDLE_DBC, hDbc)) {
                throw runtime_error("Transaction Setting Failed");
            }
            attr = true;

            //중앙 테이블인 Players row INSERT
            CreatePlayersTable(hDbc, hStmt2, id, isSuccess);

            //INSERT에 성공한 경우.
            if (isSuccess) {
                //해당 테이블의 dbid를 가져옴.
                SQLINTEGER dbid = -1;
                ReadDbidFromPlayersTable(hDbc, hStmt3, id, dbid);

                //dbid로서 Accounts row INSERT
                CreateAccountsTable(hDbc, hStmt4, password, dbid);

                //dbid로서 Elos row INSERT
                CreateElosTable(hDbc, hStmt5, dbid);

                ret = SQLEndTran(SQL_HANDLE_DBC, hDbc, SQL_COMMIT);
                if (!GDBManager->CheckReturn(ret, SQL_HANDLE_DBC, hDbc)) {
                    throw runtime_error("Transaction Commit Failed");
                }
                attr = false;
                _reply.set_success(true);
            }
            else {
                _reply.set_success(false);
            }

            SQLSetConnectAttr(hDbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_ON, SQL_IS_UINTEGER);
            if (!GDBManager->CheckReturn(ret, SQL_HANDLE_DBC, hDbc)) {
                //hDbc가 null인 경우, pool에 반환되지 않게 동작하므로
                hDbc = nullptr;
                throw runtime_error("Failed to hDbc Auto Commit Setting");
            }
        }
        catch (const runtime_error& e) {
            cout << e.what() << endl;
        }

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
/*
void DCreateAccountCallData::fSQL1(SQLHDBC& hDbc, SQLHSTMT& hStmt1, const string& id, bool& S1) {
    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt1);
    if (!GDBManager->CheckReturn(ret, SQL_HANDLE_DBC, hDbc)) {
        throw runtime_error("D2S_CreateAccount : hStmt1 Alloc Failed");
    }

    wstring query = L"SELECT 1 FROM Players WHERE player_id = ?";
    ret = SQLPrepareW(hStmt1, (SQLWCHAR*)query.c_str(), SQL_NTS);
    if (!GDBManager->CheckReturn(ret, SQL_HANDLE_STMT, hStmt1)) {
        throw runtime_error("D2S_CreateAccount : S1 Query Setting Failed");
    }

    wstring wId = GDBManager->a2wsRef(id);
    SQLLEN idLen = SQL_NTS;
    ret = SQLBindParameter(hStmt1, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, wId.size(), 0, (SQLPOINTER)wId.c_str(), 0, &idLen);
    if (!GDBManager->CheckReturn(ret, SQL_HANDLE_STMT, hStmt1)) {
        throw runtime_error("D2S_CreateAccount : S1 Bind Parameter Failed");
    }

    ret = SQLExecute(hStmt1);
    if (!GDBManager->CheckReturn(ret, SQL_HANDLE_STMT, hStmt1)) {
        throw runtime_error("D2S_CreateAccount : S1 Execute Failed");
    }

    ret = SQLFetch(hStmt1);
    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
        _reply.set_success(false);
    }
    else if (ret == SQL_NO_DATA) {
        S1 = true;
    }
    else {
        throw runtime_error("D2S_CreateAccount : S1 Fetch Failed");
    }
}
*/
void DCreateAccountCallData::CreatePlayersTable(SQLHDBC& hDbc, SQLHSTMT& hStmt2, const string& id, bool& flag) {
    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt2);
    if (!GDBManager->CheckReturn(ret, SQL_HANDLE_DBC, hDbc)) {
        cout << "hStmt2 할당 실패;" << endl;
        throw runtime_error("D2S_CreateAccount : hStmt2 Alloc Failed");
    }

    wstring query = L"INSERT INTO Players (player_id) VALUES (?)";
    ret = SQLPrepareW(hStmt2, (SQLWCHAR*)query.c_str(), SQL_NTS);
    if (!GDBManager->CheckReturn(ret, SQL_HANDLE_STMT, hStmt2)) {
        throw runtime_error("D2S_CreateAccount : S2 Query Setting Failed");
    }

    wstring wId = GDBManager->s2wsRef(id);
    SQLLEN idLen = SQL_NTS;
    ret = SQLBindParameter(hStmt2, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, wId.size(), 0, (SQLPOINTER)wId.c_str(), 0, &idLen);
    if (!GDBManager->CheckReturn(ret, SQL_HANDLE_STMT, hStmt2)) {
        throw runtime_error("D2S_CreateAccount : S2 Bind Parameter Failed");
    }

    ret = SQLExecute(hStmt2);
    if (!SQL_SUCCEEDED(ret)) {
        SQLWCHAR sqlState[6] = { 0 };
        SQLINTEGER nativeError;
        SQLWCHAR messageText[1024] = { 0 };
        SQLSMALLINT textLength;
        SQLSMALLINT recNumber = 1;

        SQLGetDiagRecW(SQL_HANDLE_STMT, hStmt2, recNumber, sqlState, &nativeError, messageText, sizeof(messageText) / sizeof(SQLWCHAR), &textLength);
        if (wstring(sqlState) == L"23000") {
            flag = false;
            cout << 23000 << endl;
            return;
        }
        else {
            throw runtime_error("D2S_CreateAccount : S2 Execute Failed");
        }
    }
    flag = true;
}

void DCreateAccountCallData::ReadDbidFromPlayersTable(SQLHDBC& hDbc, SQLHSTMT& hStmt3, const string& id, SQLINTEGER& dbid) {
    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt3);
    if (!GDBManager->CheckReturn(ret, SQL_HANDLE_DBC, hDbc)) {
        throw runtime_error("D2S_CreateAccount : S3 hStmt Alloc Failed");
    }

    wstring query = L"SELECT dbid FROM Players WHERE player_id = ?";
    ret = SQLPrepareW(hStmt3, (SQLWCHAR*)query.c_str(), SQL_NTS);
    if (!GDBManager->CheckReturn(ret, SQL_HANDLE_STMT, hStmt3)) {
        throw runtime_error("D2S_CreateAccount : S3 Query Setting Failed");
    }

    wstring wId = GDBManager->s2wsRef(id);
    SQLLEN idLen = SQL_NTS;
    ret = SQLBindParameter(hStmt3, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, wId.size(), 0, (SQLPOINTER)wId.c_str(), 0, &idLen);
    if (!GDBManager->CheckReturn(ret, SQL_HANDLE_STMT, hStmt3)) {
        throw runtime_error("D2S_CreateAccount : S3 Bind Parameter Failed");
    }

    ret = SQLExecute(hStmt3);
    if (!GDBManager->CheckReturn(ret, SQL_HANDLE_STMT, hStmt3)) {
        throw runtime_error("D2S_CreateAccount : S3 Execute Failed");
    }

    SQLLEN dbid_ind = 0;

    ret = SQLFetch(hStmt3);
    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
        ret = SQLGetData(hStmt3, 1, SQL_C_SLONG, &dbid, sizeof(dbid), &dbid_ind);
        cout << dbid << endl;
        // 정상 동작
    }
    else if (ret == SQL_NO_DATA) {
        throw runtime_error("D2S_CreateAccount : S3 Fetch No Data");
    }
    else {
        throw runtime_error("D2S_CreateAccount : S3 Fetch Failed");
    }
}

void DCreateAccountCallData::CreateAccountsTable(SQLHDBC& hDbc, SQLHSTMT& hStmt4, const string& password, SQLINTEGER& dbid) {
    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt4);
    if (!GDBManager->CheckReturn(ret, SQL_HANDLE_DBC, hDbc)) {
        throw runtime_error("D2S_CreateAccount : hStmt4 Alloc Failed");
    }

    wstring query = L"INSERT INTO Accounts (dbid, password_hash, salt) VALUES (?, ?, ?)";
    ret = SQLPrepareW(hStmt4, (SQLWCHAR*)query.c_str(), SQL_NTS);
    if (!GDBManager->CheckReturn(ret, SQL_HANDLE_STMT, hStmt4)) {
        throw runtime_error("D2S_CreateAccount : S4 Query Setting Failed");
    }
    vector<unsigned char> salt(DBManager::salt_size);
    vector<unsigned char> hash(DBManager::hash_size);
    if (RAND_bytes(salt.data(), DBManager::salt_size) != 1) {
        throw runtime_error("D2S_CreateAccount : S4 Salt Setting Failed");
    }

    if (PKCS5_PBKDF2_HMAC(
        password.c_str(),
        password.length(),
        salt.data(),
        salt.size(),
        DBManager::pbkdf2_iter,
        EVP_sha256(),
        DBManager::hash_size,
        hash.data()
    ) != 1) {
        throw runtime_error("D2S_CreateAccount : S4 PBKDF2 Failed");
    }

    wstring wHashword = GDBManager->v2wsRef(hash);
    wstring wSalt = GDBManager->v2wsRef(salt);
    ret = SQLBindParameter(hStmt4, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &dbid, 0, NULL);
    if (!GDBManager->CheckReturn(ret, SQL_HANDLE_STMT, hStmt4)) {
        throw runtime_error("D2S_CreateAccount : S4 Bind dbid Failed");
    }
    SQLLEN wHashLen = SQL_NTS;
    ret = SQLBindParameter(hStmt4, 2, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, wHashword.size(), 0, (SQLPOINTER)wHashword.c_str(), 0, &wHashLen);
    if (!GDBManager->CheckReturn(ret, SQL_HANDLE_STMT, hStmt4)) {
        throw runtime_error("D2S_CreateAccount : S4 Bind password_hash Failed");
    }
    SQLLEN wSaltLen = SQL_NTS;
    ret = SQLBindParameter(hStmt4, 3, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, wSalt.size(), 0, (SQLPOINTER)wSalt.c_str(), 0, &wSaltLen);
    if (!GDBManager->CheckReturn(ret, SQL_HANDLE_STMT, hStmt4)) {
        throw runtime_error("D2S_CreateAccount : S4 Bind salt Failed");
    }

    ret = SQLExecute(hStmt4);
    if (!GDBManager->CheckReturn(ret, SQL_HANDLE_STMT, hStmt4)) {
        throw runtime_error("D2S_CreateAccount : S4 Execute Failed");
    }
}

void DCreateAccountCallData::CreateElosTable(SQLHDBC& hDbc, SQLHSTMT& hStmt5, SQLINTEGER& dbid) {
    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt5);
    if (!GDBManager->CheckReturn(ret, SQL_HANDLE_DBC, hDbc)) {
        throw runtime_error("D2S_CreateAccount : S5 hStmt5 Alloc Failed");
    }

    wstring query = L"INSERT INTO Elos (dbid) VALUES (?)";
    ret = SQLPrepareW(hStmt5, (SQLWCHAR*)query.c_str(), SQL_NTS);
    if (!GDBManager->CheckReturn(ret, SQL_HANDLE_STMT, hStmt5)) {
        throw runtime_error("D2S_CreateAccount : S5 Query Setting Failed");
    }

    ret = SQLBindParameter(hStmt5, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &dbid, 0, NULL);
    if (!GDBManager->CheckReturn(ret, SQL_HANDLE_STMT, hStmt5)) {
        throw runtime_error("D2S_CreateAccount : S5 Bind Parameter Failed");
    }

    ret = SQLExecute(hStmt5);
    if (!GDBManager->CheckReturn(ret, SQL_HANDLE_STMT, hStmt5)) {
        throw runtime_error("D2S_CreateAccount : S5 Execute Failed");
    }
}

void DRenewElosCallData::Proceed() {
    if (_status == CREATE) {
        _status = PROCESS;
        _service->RequestRenewElosRequest(&_ctx, &_request, &_responder, _completionQueueRef, _completionQueueRef, this);
    }
    else if (_status == PROCESS) {
        // 새로운 CallData를 CompletionQueue에 등록.
        DRenewElosCallData* newCallData = objectPool<DRenewElosCallData>::alloc(_service, _completionQueueRef);
        int dbid = _request.dbid();

        try {
            SQLHDBC hDbc = GDBManager->PopHDbc();
            if (hDbc == nullptr) {
                throw runtime_error("Invalid hDbc");
            }

            Cleaner hDbcCleaner([&]() {
                GDBManager->ReturnHDbc(hDbc);
            });

            SQLHSTMT hStmt1 = nullptr;
            Cleaner hStmtCleaner([&]() {
                if (hStmt1 != nullptr) SQLFreeHandle(SQL_HANDLE_STMT, hStmt1);
            });

            SQLINTEGER elo1 = 0, elo2 = 0, elo3 = 0;
            ReadElosFromElosTable(hDbc, hStmt1, dbid, elo1, elo2, elo3);

            if (elo1 != 0 || elo2 != 0 || elo3 != 0) {
                _reply.set_elo1(elo1);
                _reply.set_elo2(elo2);
                _reply.set_elo3(elo3);
            }
            else {
                throw runtime_error("wrong Elos");
            }
        }
        catch (runtime_error& e) {
            cerr << e.what() << endl;
        }
        
        _status = FINISH;
        _responder.Finish(_reply, grpc::Status::OK, this);
    }
    // 마지막 단계: RPC가 완료됨 CallData를 Pool에 반환
    else {
#ifdef _DEBUG
        cout << "Server: CreateAccount Request sequence complete!" << endl;
#endif 
        ReturnToPool();
    }
}

void DRenewElosCallData::ReadElosFromElosTable(SQLHDBC& hDbc, SQLHSTMT& hStmt1, const int& dbid, SQLINTEGER& elo1, SQLINTEGER& elo2, SQLINTEGER& elo3) {
    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt1);
    if (!GDBManager->CheckReturn(ret, SQL_HANDLE_DBC, hDbc)) {
        throw runtime_error("S2D_RenewElos : hStmt1 Alloc Failed");
    }

    wstring query = L"SELECT elo1, elo2, elo3 FROM Elos WHERE dbid = ?";
    ret = SQLPrepareW(hStmt1, (SQLWCHAR*)query.c_str(), SQL_NTS);
    if (!GDBManager->CheckReturn(ret, SQL_HANDLE_STMT, hStmt1)) {
        throw runtime_error("S2D_RenewElos : S1 Query Setting Failed");
    }

    int objectId = dbid;
    ret = SQLBindParameter(hStmt1, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &objectId, 0, NULL);
    if (!GDBManager->CheckReturn(ret, SQL_HANDLE_STMT, hStmt1)) {
        throw runtime_error("S2D_RenewElos : S1 Bind Parameter Failed");
    }

    ret = SQLExecute(hStmt1);
    if (!GDBManager->CheckReturn(ret, SQL_HANDLE_STMT, hStmt1)) {
        throw runtime_error("S2D_RenewElos : S1 Execute Failed");
    }

    ret = SQLFetch(hStmt1);
    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
        SQLGetData(hStmt1, 1, SQL_C_SLONG, &elo1, sizeof(elo1), NULL);
        SQLGetData(hStmt1, 2, SQL_C_SLONG, &elo2, sizeof(elo2), NULL);
        SQLGetData(hStmt1, 3, SQL_C_SLONG, &elo3, sizeof(elo3), NULL);
    }
    else if (ret == SQL_NO_DATA) {
        throw runtime_error("S2D_RenewElos : Player not found with the given dbid.");
    }
    else {
        throw runtime_error("S2D_RenewElos : SQLFetch Failed");
    }
}

void DPlayerInfomationCallData::Proceed() {
    if (_status == CREATE) {
        _status = PROCESS;
        _service->RequestPlayerInfomation(&_ctx, &_request, &_responder, _completionQueueRef, _completionQueueRef, this);
    }
    else if (_status == PROCESS) {
        DPlayerInfomationCallData* newCallData = objectPool<DPlayerInfomationCallData>::alloc(_service, _completionQueueRef);

        int dbid = _request.dbid();
        grpc::Status stat = grpc::Status::OK;

        try {
            SQLHDBC hDbc = GDBManager->PopHDbc();
            if (hDbc == nullptr) {
                throw runtime_error("Invalid hDbc");
            }
            Cleaner hDbcCleaner([&]() {
                GDBManager->ReturnHDbc(hDbc);
            });

            SQLHSTMT hStmt1 = nullptr, hStmt2 = nullptr, hStmt3 = nullptr;
            Cleaner hStmtCleaner([&]() {
                if (hStmt1 != nullptr) SQLFreeHandle(SQL_HANDLE_STMT, hStmt1);
                if (hStmt2 != nullptr) SQLFreeHandle(SQL_HANDLE_STMT, hStmt2);
                if (hStmt3 != nullptr) SQLFreeHandle(SQL_HANDLE_STMT, hStmt3);
            });

            wstring playerId;
            playerId = L"";
            vector<SQLINTEGER> elos;
            vector<SQLINTEGER> records;
            ReadPlayerId(hDbc, hStmt1, dbid, playerId);
            ReadElos(hDbc, hStmt2, dbid, elos);
            ReadPersonalRecords(hDbc, hStmt3, dbid, records);

            string u8playerId = GDBManager->ws2sRef(playerId);
            _reply.set_playerid(u8playerId);
            for (SQLINTEGER elo : elos) {
                _reply.add_elos(elo);
            }
            for (SQLINTEGER record : records) {
                _reply.add_personalrecords(record);
            }
        }
        catch (runtime_error& e) {
            cout << e.what() << endl;
        }

        _status = FINISH;
        _responder.Finish(_reply, stat, this);
    }
    // 마지막 단계: RPC가 완료됨 CallData를 Pool에 반환
    else {
        objectPool<DPlayerInfomationCallData>::dealloc(this);
    }
}

void DPlayerInfomationCallData::ReadPlayerId(SQLHDBC& hDbc, SQLHSTMT& hStmt1, const int& dbid, wstring& playerId) {
    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt1);
    if (!GDBManager->CheckReturn(ret, SQL_HANDLE_DBC, hDbc)) {
        throw runtime_error("S2D_PlayerInformation : hStmt1 Alloc Failed");
    }

    wstring query = L"SELECT player_id FROM Players WHERE dbid = ?";
    ret = SQLPrepareW(hStmt1, (SQLWCHAR*)query.c_str(), SQL_NTS);
    if (!GDBManager->CheckReturn(ret, SQL_HANDLE_STMT, hStmt1)) {
        throw runtime_error("S2D_PlayerInformation : Query Setting Failed");
    }

    int objectId = dbid;
    ret = SQLBindParameter(hStmt1, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &objectId, 0, NULL);
    if (!GDBManager->CheckReturn(ret, SQL_HANDLE_STMT, hStmt1)) {
        throw runtime_error("S2D_PlayerInformation : Bind Parameter Failed");
    }

    ret = SQLExecute(hStmt1);
    if (!GDBManager->CheckReturn(ret, SQL_HANDLE_STMT, hStmt1)) {
        throw runtime_error("S2D_PlayerInformation : Execute Failed");
    }

    ret = SQLFetch(hStmt1);
    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
        SQLWCHAR playerIdBuffer[101]; // 예: 최대 100자 + NULL 종결 문자
        SQLLEN indicator = 0; // 실제 데이터 길이 또는 NULL 여부를 받을 변수
        ret = SQLGetData(hStmt1, 1, SQL_C_WCHAR, playerIdBuffer, sizeof(playerIdBuffer), &indicator);
        if (SQL_SUCCEEDED(ret)) {
            if (indicator == SQL_NULL_DATA) {
                throw runtime_error("S2D_PlayerInformation : Player ID is NULL.");
            }
            else {
                playerId = wstring(playerIdBuffer);
            }
        }
        else {
            throw runtime_error("S2D_PlayerInformation : SQLGetData Failed");
        }
    }
    else if (ret == SQL_NO_DATA) {
        throw runtime_error("S2D_PlayerInformation : Player not found with the given dbid.");
    }
    else {
        throw runtime_error("S2D_PlayerInformation : SQLFetch Failed");
    }
}

void DPlayerInfomationCallData::ReadElos(SQLHDBC& hDbc, SQLHSTMT& hStmt2, const int& dbid, vector<SQLINTEGER>& elos) {
    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt2);
    if (!GDBManager->CheckReturn(ret, SQL_HANDLE_DBC, hDbc)) {
        throw runtime_error("S2D_PlayerInformation : hStmt2 Alloc Failed");
    }

    wstring query = L"SELECT elo1, elo2, elo3 FROM Elos WHERE dbid = ?";
    ret = SQLPrepareW(hStmt2, (SQLWCHAR*)query.c_str(), SQL_NTS);
    if (!GDBManager->CheckReturn(ret, SQL_HANDLE_STMT, hStmt2)) {
        throw runtime_error("S2D_PlayerInformation : Query Setting Failed");
    }

    int objectId = dbid;
    ret = SQLBindParameter(hStmt2, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &objectId, 0, NULL);
    if (!GDBManager->CheckReturn(ret, SQL_HANDLE_STMT, hStmt2)) {
        throw runtime_error("S2D_PlayerInformation : Bind Parameter Failed");
    }

    ret = SQLExecute(hStmt2);
    if (!GDBManager->CheckReturn(ret, SQL_HANDLE_STMT, hStmt2)) {
        throw runtime_error("S2D_PlayerInformation : Execute Failed");
    }

    SQLINTEGER elo1 = 0, elo2 = 0, elo3 = 0;
    ret = SQLFetch(hStmt2);
    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
        SQLGetData(hStmt2, 1, SQL_C_SLONG, &elo1, sizeof(elo1), NULL);
        SQLGetData(hStmt2, 2, SQL_C_SLONG, &elo2, sizeof(elo2), NULL);
        SQLGetData(hStmt2, 3, SQL_C_SLONG, &elo3, sizeof(elo3), NULL);
        elos.clear();
        elos.push_back(elo1);
        elos.push_back(elo2);
        elos.push_back(elo3);
    }
    else if (ret == SQL_NO_DATA) {
        throw runtime_error("S2D_PlayerInformation : Player not found with the given dbid.");
    }
    else {
        throw runtime_error("S2D_PlayerInformation : SQLFetch Failed");
    }
}

void DPlayerInfomationCallData::ReadPersonalRecords(SQLHDBC& hDbc, SQLHSTMT& hStmt3, const int& dbid, vector<SQLINTEGER>& personalRecords) {
    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt3);
    if (!GDBManager->CheckReturn(ret, SQL_HANDLE_DBC, hDbc)) {
        throw runtime_error("S2D_PlayerInformation : hStmt3 Alloc Failed");
    }

    wstring query = L"SELECT score1, score2, score3 FROM PersonalRecords WHERE dbid = ?";
    ret = SQLPrepareW(hStmt3, (SQLWCHAR*)query.c_str(), SQL_NTS);
    if (!GDBManager->CheckReturn(ret, SQL_HANDLE_STMT, hStmt3)) {
        throw runtime_error("S2D_PlayerInformation : Query Setting Failed");
    }

    int objectId = dbid;
    ret = SQLBindParameter(hStmt3, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &objectId, 0, NULL);
    if (!GDBManager->CheckReturn(ret, SQL_HANDLE_STMT, hStmt3)) {
        throw runtime_error("S2D_PlayerInformation : Bind Parameter Failed");
    }

    ret = SQLExecute(hStmt3);
    if (!GDBManager->CheckReturn(ret, SQL_HANDLE_STMT, hStmt3)) {
        throw runtime_error("S2D_PlayerInformation : Execute Failed");
    }

    SQLINTEGER personalRecord1 = 0, personalRecord2 = 0, personalRecord3 = 0;
    ret = SQLFetch(hStmt3);
    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
        SQLGetData(hStmt3, 1, SQL_C_SLONG, &personalRecord1, sizeof(personalRecord1), NULL);
        SQLGetData(hStmt3, 2, SQL_C_SLONG, &personalRecord2, sizeof(personalRecord2), NULL);
        SQLGetData(hStmt3, 3, SQL_C_SLONG, &personalRecord3, sizeof(personalRecord3), NULL);
        personalRecords.clear();
        personalRecords.push_back(personalRecord1);
        personalRecords.push_back(personalRecord2);
        personalRecords.push_back(personalRecord3);
    }
    else if (ret == SQL_NO_DATA) {
        throw runtime_error("S2D_PlayerInformation : Player not found with the given dbid.");
    }
    else {
        throw runtime_error("S2D_PlayerInformation : SQLFetch Failed");
    }
}
