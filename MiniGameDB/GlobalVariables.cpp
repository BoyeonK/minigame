#include "pch.h"
#include "GlobalVariables.h"

DBManager* GDBManager = nullptr;
ThreadManager* GThreadManager = nullptr;
thread_local uint32_t MyThreadID = 0;
thread_local uint64_t LEndTickCount = 0;
thread_local SQLHDBC LhDbc = nullptr;

class DBGlobal {
public:
	DBGlobal() {
		GDBManager = new DBManager();
        GThreadManager = new ThreadManager();
	}

	~DBGlobal() {
		delete GDBManager;
        delete GThreadManager;
	}

} GDBGlobal;

DBManager::DBManager() : _hEnv(nullptr) {
    //ODBC ȯ�� �� ���� �ʱ�ȭ
    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &_hEnv);
    if (!SQL_SUCCEEDED(ret)) {
        cout << "ȯ�� �ڵ� �Ҵ� ����." << endl;
        return;
    }

    // ODBC ���� ����
    ret = SQLSetEnvAttr(_hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, SQL_IS_INTEGER);

    //ȯ�溯�� ����
    SetEnv();

    //���� �ڵ� Ǯ��, ����� ������ �ڵ��� 3�� �߰�.
    for (int i = 0; i < 3; i++) {
        ReturnHDbc(ConnectNewHDbc());
    }

    //CRUD �׽�Ʈ
    try {
        InitialC();
        InitialR();
        InitialU();
        InitialD();
        ScandinavianFlick();
    }
    catch (const runtime_error& e) {
        cerr << e.what() << endl;
    } 
}

DBManager::~DBManager() {
    {
        lock_guard<mutex> lock(_mtx);
        while (!_hDbcQ.empty()) {
            SQLHDBC hDbc = _hDbcQ.front();
            SQLFreeHandle(SQL_HANDLE_DBC, &hDbc);
            _hDbcQ.pop();
        }  // ���� �ڵ� ����
    }
    SQLFreeHandle(SQL_HANDLE_ENV, _hEnv);  // ȯ�� �ڵ� ����
}

void DBManager::SetEnv() {
    //DB������ ���� ���� �ۼ��� ���ؼ�, ������ ȯ�� �������� �о (�ٸ������ ���� ���)
    ifstream envFile(".env");
    if (envFile.is_open())
        cout << ".env Open Succeed!" << endl;
    else
        cout << ".env Open Faild" << endl;
    string line;

    while (getline(envFile, line)) {
        // �����̳� �ּ��� �����ϰ�, "=" �������� KEY=VALUE ���� ����
        if (line.empty() || line[0] == '#') continue;

        // KEY=VALUE���� VALUE�� ����ǥ�� ������ �ִٸ� �̸� ����
        size_t eqPos = line.find("=");
        if (eqPos != string::npos) {
            string key = line.substr(0, eqPos);
            string value = line.substr(eqPos + 1);

            // VALUE�� ����ǥ�� �������� ���, ����
            if (value.front() == '"' && value.back() == '"')
                value = value.substr(1, value.length() - 2);

            //Unicode�� ��ȯ (SetEnvironmentVariableW ����� ���ؼ�)
            //�ѱ��� ���Ե��� �����鼭(ascii �ڵ� ���� ������ ���ڵ�), ���ÿ� Windowsȯ���̶� ����� �����Ѵ�.
            wstring wkey(key.begin(), key.end());
            wstring wvalue(value.begin(), value.end());

            // ȯ�� ���� ����
            if (SetEnvironmentVariableW(wkey.c_str(), wvalue.c_str()))
                cout << "ȯ�� ���� : " << key << " ���� ����." << std::endl;
            else
                cout << "ȯ�� ���� ���� ���� : " << key << " (���� �ڵ�: " << GetLastError() << ")" << std::endl;
        }
    }
    envFile.close();
}

bool DBManager::CheckReturn(SQLRETURN ret, SQLSMALLINT handleType, SQLHANDLE handle) {
    if (!SQL_SUCCEEDED(ret)) {
        SQLWCHAR sqlState[6];
        SQLINTEGER nativeError;
        SQLWCHAR messageText[1024];
        SQLSMALLINT textLength;
        SQLSMALLINT recNumber = 1;

        // �� �̻� ������ �����Ͱ� ���� ��(SQL_NO_DATA)���� �ݺ�
        while (SQLGetDiagRecW(handleType, handle, recNumber, sqlState, &nativeError,
            messageText, sizeof(messageText) / sizeof(SQLWCHAR), &textLength) != SQL_NO_DATA)
        {
            wcout << L"ODBC Error:" << endl;
            wcout << L"  SQLSTATE: " << sqlState << endl;
            wcout << L"  Native Error: " << nativeError << endl;
            //wcout << L"  Message: " << messageText << endl;

            recNumber++;
        }
        return false;
    }
    return true;
}

wstring DBManager::a2wsRef(const string& in_cp949) {
    if (in_cp949.empty())
        return L"";

    // 1. �ʿ��� wchar_t ���� ũ�� ��� (NULL ���� ���� ����)
    int w_len = MultiByteToWideChar(CP_ACP, 0, in_cp949.c_str(), -1, NULL, 0);
    if (w_len == 0)
        throw runtime_error("a2ws: Failed to get required length. LastError: " + to_string(GetLastError()));

    // 2. �Ҵ�
    wstring ws;
    ws.resize(w_len - 1);

    //�� ä���
    if (!MultiByteToWideChar(CP_ACP, 0, in_cp949.c_str(), -1, &ws[0], w_len))
        throw runtime_error("a2ws: Failed to convert string to wchar. LastError: " + std::to_string(GetLastError()));

    return ws;
}

wstring DBManager::s2wsRef(const string& in_u8s) {
    if (in_u8s.empty())
        return L"";

    // 1. �ʿ��� wchar_t ���� ũ�� ��� (NULL ���� ���� ����)
    int w_len = MultiByteToWideChar(CP_UTF8, 0, in_u8s.c_str(), -1, NULL, 0);
    if (w_len == 0)
        throw runtime_error("s2ws: Failed to get required length. LastError: " + to_string(GetLastError()));

    // 2. �Ҵ�
    wstring ws;
    ws.resize(w_len - 1);

    //�� ä���
    if (!MultiByteToWideChar(CP_UTF8, 0, in_u8s.c_str(), -1, &ws[0], w_len))
        throw runtime_error("s2ws: Failed to convert string to wchar. LastError: " + std::to_string(GetLastError()));

    return ws;
}

wstring DBManager::v2wsRef(const vector<unsigned char>& in_binary) {
    wstringstream ws;
    for (const auto& byte : in_binary) {
        ws << hex << setw(2) << setfill(L'0') << static_cast<int>(byte);
    }
    return ws.str();
}

wstring DBManager::CreateQuery(const wstring& tableName, initializer_list<wstring> wstrs) {
    if (wstrs.size() < 2 or wstrs.size() % 2 != 0)
        throw runtime_error("CreateQuery: ���� ������ ���� �ʽ��ϴ�.");

    vector<wstring> args(wstrs);

    wstringstream ss;
    ss << L"INSERT INTO " << tableName << L" (";
    size_t halfCol = wstrs.size() / 2;

    for (int i = 0; i < halfCol; i++) {
        ss << args[i];
        if (i != (halfCol - 1))
            ss << L", ";
    }
    ss << L") VALUES (";
    for (int i = 0; i < halfCol; i++) {
        ss << args[i + halfCol];
        if (i != (halfCol - 1))
            ss << L", ";
    }
    ss << L")";

    return ss.str();
}

SQLHDBC DBManager::ConnectNewHDbc() {
    SQLHDBC hDbc = nullptr;

    wchar_t* dbServer = nullptr;
    wchar_t* dbName = nullptr;
    wchar_t* connection = nullptr;
    size_t wSize;
    errno_t err;
    try {
        err = _wdupenv_s(&dbServer, &wSize, L"DB_SERVER");
        err = _wdupenv_s(&dbName, &wSize, L"DB_NAME");
        err = _wdupenv_s(&connection, &wSize, L"CONNECTION");

        if (dbServer && dbName && connection) {
            SQLWCHAR connStr[1024];
            _snwprintf_s(connStr, sizeof(connStr) / sizeof(SQLWCHAR),
                L"Driver={ODBC Driver 17 for SQL Server};Server=%ls;Database=%ls;%ls;",
                dbServer, dbName, connection);

            SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_DBC, _hEnv, &hDbc);
            ret = SQLDriverConnectW(hDbc, NULL, connStr, wcslen(connStr), NULL, 0, NULL, SQL_DRIVER_COMPLETE);
            if (!SQL_SUCCEEDED(ret)) {
                SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
                throw runtime_error("DB ���� ����");
            }
            else {
                cout << "DB ���� ����" << endl;
            }
        }
        else
            throw runtime_error("ȯ�� ������ ���� �ùٸ��� ����");
    }
    catch (runtime_error& e) {
        cout << e.what() << endl;
        hDbc = nullptr;
    }
    
    if (dbServer) delete[] dbServer;
    if (dbName) delete[] dbName;
    if (connection) delete[] connection;

    return hDbc;
}

SQLHDBC DBManager::PopHDbc() {
    SQLHDBC hDbc = nullptr;
    {
        lock_guard<mutex> lock(_mtx);
        if (!_hDbcQ.empty()) {
            hDbc = _hDbcQ.front();
            _hDbcQ.pop();
            return hDbc;
        }
    }
    hDbc = ConnectNewHDbc();
    return hDbc;
}

void DBManager::ReturnHDbc(SQLHDBC hDbc) {
    lock_guard<mutex> lock(_mtx);
    _hDbcQ.push(hDbc);
}

void DBManager::InitialC() {
    SQLHSTMT hStmt;
    SQLHDBC hDbc = PopHDbc();
    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);

    wstring query = L"INSERT INTO CRUD (id, value) VALUES (?, ?)";
    ret = SQLPrepareW(hStmt, (SQLWCHAR*)query.c_str(), SQL_NTS);
    if (!CheckReturn(ret, SQL_HANDLE_STMT, hStmt)) {
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        throw runtime_error("InitialC Failed.");
    }

    int id = 0, value = 10;
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &id, 0, NULL);
    SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &value, 0, NULL);

    ret = SQLExecute(hStmt);
    if (!CheckReturn(ret, SQL_HANDLE_STMT, hStmt)) {
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        throw runtime_error("InitialC Failed.");
    }
    else {
        cout << "C sequence is done." << endl;
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}

void DBManager::InitialR() {
    SQLHSTMT hStmt;
    SQLHDBC hDbc = PopHDbc();
    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
    wstring query = L"SELECT id, value FROM CRUD";
    ret = SQLExecDirectW(hStmt, (SQLWCHAR*)query.c_str(), SQL_NTS);
    if (!CheckReturn(ret, SQL_HANDLE_STMT, hStmt)) {
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        ReturnHDbc(hDbc);
        throw runtime_error("InitialC Failed.");
    }

    SQLINTEGER sid;
    SQLINTEGER svalue;

    while (SQLFetch(hStmt) == SQL_SUCCESS) {
        SQLGetData(hStmt, 1, SQL_C_SLONG, &sid, 0, NULL);
        SQLGetData(hStmt, 2, SQL_C_SLONG, &svalue, 0, NULL);
        cout << "R sequence is done. id : " << sid << " value: " << svalue << endl;
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    ReturnHDbc(hDbc);
}

void DBManager::InitialU() {
    SQLHSTMT hStmt;
    SQLHDBC hDbc = PopHDbc();
    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
    wstring query = L"UPDATE CRUD SET value = ? WHERE id = ?";
    ret = SQLPrepareW(hStmt, (SQLWCHAR*)query.c_str(), SQL_NTS);
    if (!CheckReturn(ret, SQL_HANDLE_STMT, hStmt)) {
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        ReturnHDbc(hDbc);
        throw runtime_error("InitialU Failed.");
    }

    int id = 0;
    int uvalue = 20;
    ret = SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &uvalue, 0, NULL);
    if (!CheckReturn(ret, SQL_HANDLE_STMT, hStmt)) {
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        ReturnHDbc(hDbc);
        throw runtime_error("InitialU Failed.");
    }

    ret = SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &id, 0, NULL);
    if (!CheckReturn(ret, SQL_HANDLE_STMT, hStmt)) {
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        ReturnHDbc(hDbc);
        throw runtime_error("InitialU Failed.");
    }

    ret = SQLExecute(hStmt);
    if (!CheckReturn(ret, SQL_HANDLE_STMT, hStmt)) {
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        ReturnHDbc(hDbc);
        throw runtime_error("InitialU Failed.");
    }
    else {
        cout << "U sequence is done. value: " << uvalue << endl;
    }
    ReturnHDbc(hDbc);
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}

void DBManager::InitialD() {
    SQLHSTMT hStmt;
    SQLHDBC hDbc = PopHDbc();
    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
    wstring query = L"DELETE FROM CRUD WHERE id = ?";
    ret = SQLPrepareW(hStmt, (SQLWCHAR*)query.c_str(), SQL_NTS);
    if (!CheckReturn(ret, SQL_HANDLE_STMT, hStmt)) {
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        throw runtime_error("InitialD Failed.");
    }

    int id = 0;
    ret = SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &id, 0, NULL);
    if (!CheckReturn(ret, SQL_HANDLE_STMT, hStmt)) {
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        ReturnHDbc(hDbc);
        throw runtime_error("InitialD Failed.");
    }

    ret = SQLExecute(hStmt);
    if (!CheckReturn(ret, SQL_HANDLE_STMT, hStmt)) {
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        ReturnHDbc(hDbc);
        throw runtime_error("InitialD Failed.");
    }
    else {
        cout << "D sequence is done." << endl;
    }
    ReturnHDbc(hDbc);
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}

void DBManager::ScandinavianFlick() {
    string myId = "tetepiti149";
    wstring wId = GDBManager->a2wsRef(myId);
    string password = "qwe123";
    bool exists;

    SQLHSTMT hStmt = nullptr;
    SQLHDBC hDbc = PopHDbc();
    if (hDbc == nullptr) {
        cout << "�ҷ� hDbc �̽�" << endl;
        return;
    }

    Cleaner hDbcCleaner([=]() {
        ReturnHDbc(hDbc);
    });

    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
    if (!CheckReturn(ret, SQL_HANDLE_DBC, hDbc)) {
        cout << "hStmt �Ҵ� ����;" << endl;
        return;
    }

    Cleaner hStmtCleaner([=]() {
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    });

    wstring query = L"SELECT 1 FROM Players WHERE player_id = ?";
    ret = SQLPrepareW(hStmt, (SQLWCHAR*)query.c_str(), SQL_NTS);
    if (!CheckReturn(ret, SQL_HANDLE_STMT, hStmt)) {
        return;
    }

    SQLLEN idLen = SQL_NTS;
    ret = SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, wId.size(), 0, (SQLPOINTER)wId.c_str(), 0, &idLen);
    if (!CheckReturn(ret, SQL_HANDLE_STMT, hStmt)) {
        return;
    }

    ret = SQLExecute(hStmt);
    if (!CheckReturn(ret, SQL_HANDLE_STMT, hStmt)) {
        return;
    }

    ret = SQLFetch(hStmt);
    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
        exists = true;
        cout << "�ִ� ���̵� ������" << endl;
        return;
    }
    else if (ret == SQL_NO_DATA) {
        exists = false;
    }
    else {
        return;
    }

    ret = SQLSetConnectAttr(hDbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_OFF, SQL_IS_UINTEGER);
    if (!CheckReturn(ret, SQL_HANDLE_DBC, hDbc)) {
        cout << "���� ����Ʈ ����. ������." << endl;
        return;
    }
    Cleaner Lovely_Labrynth_Of_The_Silver_Castle([=]() {
        SQLEndTran(SQL_HANDLE_DBC, hDbc, SQL_ROLLBACK);
    });

    SQLHSTMT hStmt2 = nullptr;
    ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt2);
    if (!CheckReturn(ret, SQL_HANDLE_DBC, hDbc)) {
        cout << "hStmt2 �Ҵ� ����;" << endl;
        return;
    }
    Cleaner hStmt2Cleaner([=]() {
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt2);
    });

    query = L"INSERT INTO Players (player_id) VALUES (?)";
    ret = SQLPrepareW(hStmt2, (SQLWCHAR*)query.c_str(), SQL_NTS);
    if (!CheckReturn(ret, SQL_HANDLE_STMT, hStmt2)) {
        return;
    }

    ret = SQLBindParameter(hStmt2, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, wId.size(), 0, (SQLPOINTER)wId.c_str(), 0, &idLen);
    if (!CheckReturn(ret, SQL_HANDLE_STMT, hStmt2)) {
        return;
    }

    ret = SQLExecute(hStmt2);
    if (!CheckReturn(ret, SQL_HANDLE_STMT, hStmt2)) {
        return;
    }

    //Player Table SELECT �۾�
    //��� �߰��� ������ dbid�� ����.
    SQLINTEGER dbid = -1;

    SQLHSTMT hStmt3 = nullptr;
    ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt3);
    if (!CheckReturn(ret, SQL_HANDLE_DBC, hDbc)) {
        cout << "hStmt3 �Ҵ� ����;" << endl;
        return;
    }
    Cleaner hStmt3Cleaner([=]() {
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt3);
    });

    query = L"SELECT dbid FROM Players WHERE player_id = ?";
    ret = SQLPrepareW(hStmt3, (SQLWCHAR*)query.c_str(), SQL_NTS);
    if (!CheckReturn(ret, SQL_HANDLE_STMT, hStmt3)) {
        return;
    }

    ret = SQLBindParameter(hStmt3, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, wId.size(), 0, (SQLPOINTER)wId.c_str(), 0, &idLen);
    if (!CheckReturn(ret, SQL_HANDLE_STMT, hStmt3)) {
        return;
    }

    ret = SQLExecute(hStmt3);
    if (!CheckReturn(ret, SQL_HANDLE_STMT, hStmt3)) {
        return;
    }

    SQLLEN dbid_ind = 0;

    ret = SQLFetch(hStmt3);
    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
        ret = SQLGetData(hStmt3, 1, SQL_C_SLONG, &dbid, sizeof(dbid), &dbid_ind);
        cout << dbid << endl;
        // ���� ����
    }
    else if (ret == SQL_NO_DATA) {
        return;
    }
    else {
        return;
    }

}

ThreadManager::ThreadManager() {
    InitTLS();
}

ThreadManager::~ThreadManager() {
    Join();
}

void ThreadManager::InitTLS() {
    //NxtThreadID�� ������ InitTLS�Լ��θ� �����Ǿ����.
    //���� static�Լ� ������ ���� static������ ����
    //�� �Լ� �������� ����� NxtThreadID ������ ����ִ�.
    static atomic<uint32_t> NxtThreadID = 1;
    MyThreadID = NxtThreadID.fetch_add(1);
}

void ThreadManager::Launch(function<void(void)> callback) {
    lock_guard<mutex> guard(_mutex);
    _threads.push_back(thread([=] {
        InitTLS();
        callback();
        DestroyTLS();
    }));
}

void ThreadManager::Join() {
    for (thread& t : _threads) {
        if (t.joinable())
            t.join();
    }
    _threads.clear();
}

