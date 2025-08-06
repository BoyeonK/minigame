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

        string id = _request.id();
        string password = _request.password();
        SQLHDBC hDbc = GDBManager->popHDbc();

        bool attrErr = false;

        try {
            auto hStmt1 = unique_ptr<SQLHANDLE, SQLHandleDeleter>(new SQLHANDLE(SQL_NULL_HANDLE), { SQL_HANDLE_STMT, hDbc });
            auto hStmt2 = unique_ptr<SQLHANDLE, SQLHandleDeleter>(new SQLHANDLE(SQL_NULL_HANDLE), { SQL_HANDLE_STMT, hDbc });
            auto hStmt3 = unique_ptr<SQLHANDLE, SQLHandleDeleter>(new SQLHANDLE(SQL_NULL_HANDLE), { SQL_HANDLE_STMT, hDbc });
            auto hStmt4 = unique_ptr<SQLHANDLE, SQLHandleDeleter>(new SQLHANDLE(SQL_NULL_HANDLE), { SQL_HANDLE_STMT, hDbc });

            SQLRETURN ret = SQLSetConnectAttr(hDbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_OFF, SQL_IS_UINTEGER);
            if (!GDBManager->CheckReturn(ret)) {
                attrErr = true;
                throw runtime_error("0"); 
            }

            //Player Table INSERT 작업
            ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, hStmt1.get());
            if (!GDBManager->CheckReturn(ret)) {
                throw runtime_error("1");
            }

            wstring query1 = L"INSERT INTO Players (player_id) VALUES (?)";
            ret = SQLPrepareW(hStmt1.get(), (SQLWCHAR*)query1.c_str(), SQL_NTS);
            if (!GDBManager->CheckReturn(ret)) {
                throw runtime_error("2");
            }

            wstring wId = GDBManager->a2wsRef(id);
            ret = SQLBindParameter(hStmt1.get(), 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)wId.c_str(), 0, (SQLLEN*)SQL_NTS);
            if (!GDBManager->CheckReturn(ret)) {
                throw runtime_error("3");
            }

            ret = SQLExecute(hStmt1.get());
            if (!GDBManager->CheckReturn(ret)) {
                throw runtime_error("4");
            }

            //Player Table SELECT 작업
            //방금 추가한 계정의 dbid를 얻어옴.
            SQLINTEGER dbid = -1;

            ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, hStmt2.get());
            if (!GDBManager->CheckReturn(ret)) {
                throw runtime_error("5");
            }

            wstring query2 = L"SELECT dbid FROM Players WHERE player_id = ?";
            ret = SQLPrepareW(hStmt2.get(), (SQLWCHAR*)query2.c_str(), SQL_NTS);
            if (!GDBManager->CheckReturn(ret)) {
                throw runtime_error("7");
            }

            ret = SQLBindParameter(hStmt2.get(), 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)wId.c_str(), 0, (SQLLEN*)SQL_NTS);
            if (!GDBManager->CheckReturn(ret)) {
                throw runtime_error("8");
            }

            ret = SQLExecute(hStmt2.get());
            if (!GDBManager->CheckReturn(ret)) {
                throw runtime_error("9");
            }

            SQLLEN dbid_ind = 0;

            ret = SQLFetch(hStmt2.get());
            if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
                ret = SQLGetData(hStmt2.get(), 1, SQL_C_SLONG, &dbid, sizeof(dbid), &dbid_ind);
                if (!GDBManager->CheckReturn(ret)) {
                    throw runtime_error("9");
                }
                // 정상 동작
            }
            else if (ret == SQL_NO_DATA) {
                throw runtime_error("10");
            }
            else {
                throw runtime_error("11");
            }

            //Accounts Table INSERT 작업
            ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, hStmt3.get());
            if (!GDBManager->CheckReturn(ret)) {
                throw runtime_error("12");
            }

            wstring query3 = L"INSERT INTO Accounts (dbid, password_hash, salt) VALUES (?, ?, ?)";
            ret = SQLPrepareW(hStmt3.get(), (SQLWCHAR*)query3.c_str(), SQL_NTS);
            if (!GDBManager->CheckReturn(ret)) {
                throw runtime_error("13");
            }

            vector<unsigned char> salt(DBManager::salt_size);
            vector<unsigned char> hash(DBManager::hash_size);
            if (RAND_bytes(salt.data(), DBManager::salt_size) != 1) {
                throw runtime_error("14");
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
                throw runtime_error("15");
            }

            // 해시 값을 16진수 문자열로 변환 (저장 및 관리를 위해)
            wstring wHashword = GDBManager->v2wsRef(hash);
            wstring wSalt = GDBManager->v2wsRef(salt);

            ret = SQLBindParameter(hStmt3.get(), 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &dbid, 0, NULL);
            if (!GDBManager->CheckReturn(ret)) {
                throw runtime_error("16");
            }
            ret = SQLBindParameter(hStmt3.get(), 2, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)wHashword.c_str(), 0, (SQLLEN*)SQL_NTS);
            if (!GDBManager->CheckReturn(ret)) {
                throw runtime_error("17");
            }
            ret = SQLBindParameter(hStmt3.get(), 3, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)wSalt.c_str(), 0, (SQLLEN*)SQL_NTS);
            if (!GDBManager->CheckReturn(ret)) {
                throw runtime_error("18");
            }

            ret = SQLExecute(hStmt3.get());
            if (!GDBManager->CheckReturn(ret)) {
                throw runtime_error("19");
            }

            //Elos Table INSERT작업
            ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, hStmt4.get());
            if (!GDBManager->CheckReturn(ret)) {
                throw runtime_error("20");
            }

            wstring query = L"INSERT INTO Elos (dbid) VALUES (?)";

            ret = SQLPrepareW(hStmt4.get(), (SQLWCHAR*)query.c_str(), SQL_NTS);
            if (!GDBManager->CheckReturn(ret)) {
                throw runtime_error("21");
            }

            ret = SQLBindParameter(hStmt4.get(), 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &dbid, 0, NULL);
            if (!GDBManager->CheckReturn(ret)) {
                throw runtime_error("22");
            }

            ret = SQLExecute(hStmt4.get());
            if (!GDBManager->CheckReturn(ret)) {
                throw runtime_error("23");
            }

            SQLEndTran(SQL_HANDLE_DBC, hDbc, SQL_COMMIT);
            cout << "Transaction committed successfully." << std::endl;
        }
        catch (const runtime_error& e) {
            cout << e.what() << endl;
            if (!attrErr) SQLEndTran(SQL_HANDLE_DBC, hDbc, SQL_ROLLBACK);
            std::cout << "Transaction rolled back due to an error." << std::endl;
        }

        SQLSetConnectAttr(hDbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_ON, SQL_IS_UINTEGER);

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
