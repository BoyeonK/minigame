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

        string id = _request.id();
        string password = _request.password();

        //Players ���� �߰�
        {
            SQLHSTMT hStmt;
            SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, GDBManager->getHDbc(), &hStmt);
            if (!GDBManager->CheckReturn(ret)) {
                // TODO: Ʈ����� �ѹ� + �ڵ� ���� + throw ����
            }

            wstring query = L"INSERT INTO Players (player_id) VALUES (?)";
            ret = SQLPrepareW(hStmt, (SQLWCHAR*)query.c_str(), SQL_NTS);
            if (!GDBManager->CheckReturn(ret)) {
                // TODO: Ʈ����� �ѹ� + �ڵ� ���� + throw ����
            }

            wstring wId = GDBManager->a2wsRef(id);
            ret = SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, (SQLULEN)DBManager::pid_size, 0, (SQLPOINTER)wId.c_str(), 0, (SQLLEN*)SQL_NTS);
            if (!GDBManager->CheckReturn(ret)) {
                // TODO: Ʈ����� �ѹ� + �ڵ� ���� + throw ���� 
            }

            ret = SQLExecute(hStmt);
            if (!GDBManager->CheckReturn(ret)) {
                // TODO: Ʈ����� �ѹ� + �ڵ� ���� + throw ����
            }

            SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        }

        //��� �߰��� ������ dbid�� ����.
        SQLINTEGER dbid = -1;
        {
            SQLHSTMT hStmt;
            SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, GDBManager->getHDbc(), &hStmt);
            wstring query = L"SELECT dbid FROM Players WHERE player_id = ?";
            ret = SQLPrepareW(hStmt, (SQLWCHAR*)query.c_str(), SQL_NTS);
            if (!GDBManager->CheckReturn(ret)) {
                // TODO: Ʈ����� �ѹ� + �ڵ� ���� + throw ���� 
            }

            wstring wId = GDBManager->a2wsRef(id);
            ret = SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, (SQLULEN)DBManager::pid_size, 0, (SQLPOINTER)wId.c_str(), 0, (SQLLEN*)SQL_NTS);
            if (!GDBManager->CheckReturn(ret)) {
                // TODO: Ʈ����� �ѹ� + �ڵ� ���� + throw ���� 
            }

            ret = SQLExecute(hStmt);
            if (!GDBManager->CheckReturn(ret)) {
                // TODO: Ʈ����� �ѹ� + �ڵ� ���� + throw ���� 
            }

            SQLLEN dbid_ind = 0;

            ret = SQLFetch(hStmt);
            if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
                ret = SQLGetData(hStmt, 1, SQL_C_SLONG, &dbid, 0, &dbid_ind);
                if (!GDBManager->CheckReturn(ret)) {
                    // TODO: Ʈ����� �ѹ� + �ڵ� ���� + throw ���� 
                }
                // ���� ����
            }
            else if (ret == SQL_NO_DATA) {
                // TODO: Ʈ����� �ѹ� + �ڵ� ����
            }
            else {
                // TODO: Ʈ����� �ѹ� + �ڵ� ���� + throw ���� 
            }
        }

        {
            vector<unsigned char> salt(DBManager::salt_size);
            vector<unsigned char> hash(DBManager::hash_size);
            if (RAND_bytes(salt.data(), DBManager::salt_size) != 1) {
                // TODO: Ʈ����� �ѹ� + throw ����
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
                // TODO: Ʈ����� �ѹ� + throw ����
            }

            // �ؽ� ���� 16���� ���ڿ��� ��ȯ (���� �� ������ ����)
            wstringstream hashwordStream;
            for (const auto& byte : hash) {
                hashwordStream << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
            }

            wstring wHashword = hashwordStream.str();
            wstring wSalt(salt.begin(), salt.end());
            
            SQLHSTMT hStmt;
            SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, GDBManager->getHDbc(), &hStmt);
            if (!GDBManager->CheckReturn(ret)) {
                // TODO: Ʈ����� �ѹ� + �ڵ� ���� + throw ����
            }

            wstring query = L"INSERT INTO Accounts (dbid, password_hash, salt) VALUES (?, ?, ?)";
            ret = SQLPrepareW(hStmt, (SQLWCHAR*)query.c_str(), SQL_NTS);
            if (!GDBManager->CheckReturn(ret)) {
                // TODO: Ʈ����� �ѹ� + �ڵ� ���� + throw ����
            }

            ret = SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &dbid, 0, NULL);
            if (!GDBManager->CheckReturn(ret)) {
                // TODO: Ʈ����� �ѹ� + �ڵ� ���� + throw ���� 
            }
            ret = SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, (SQLULEN)DBManager::hash_size, 0, (SQLPOINTER)wHashword.c_str(), 0, (SQLLEN*)SQL_NTS);
            if (!GDBManager->CheckReturn(ret)) {
                // TODO: Ʈ����� �ѹ� + �ڵ� ���� + throw ���� 
            }
            ret = SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, (SQLULEN)DBManager::salt_size, 0, (SQLPOINTER)wSalt.c_str(), 0, (SQLLEN*)SQL_NTS);
            if (!GDBManager->CheckReturn(ret)) {
                // TODO: Ʈ����� �ѹ� + �ڵ� ���� + throw ���� 
            }

            ret = SQLExecute(hStmt);
            if (!GDBManager->CheckReturn(ret)) {
                // TODO: Ʈ����� �ѹ� + �ڵ� ���� + throw ����
            }

            SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        }

        {
            //����Ʈ ������ Elos Table����
            SQLHSTMT hStmt;
            SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, GDBManager->getHDbc(), &hStmt);
            if (!GDBManager->CheckReturn(ret)) {
                // TODO: Ʈ����� �ѹ� + �ڵ� ���� + throw ����
            }

            wstring query = L"INSERT INTO Elos dbid VALUES (?)";

            ret = SQLPrepareW(hStmt, (SQLWCHAR*)query.c_str(), SQL_NTS);
            if (!GDBManager->CheckReturn(ret)) {
                // TODO: Ʈ����� �ѹ� + �ڵ� ���� + throw ����
            }

            ret = SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &dbid, 0, NULL);
            if (!GDBManager->CheckReturn(ret)) {
                // TODO: Ʈ����� �ѹ� + �ڵ� ���� + throw ���� 
            }

            ret = SQLExecute(hStmt);
            if (!GDBManager->CheckReturn(ret)) {
                // TODO: Ʈ����� �ѹ� + �ڵ� ���� + throw ����
            }
        }

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
