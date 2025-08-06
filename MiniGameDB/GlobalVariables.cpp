#include "pch.h"
#include "GlobalVariables.h"

DBManager* GDBManager = nullptr;
ThreadManager* GThreadManager = nullptr;
thread_local uint32_t MyThreadID = 0;
thread_local uint64_t LEndTickCount = 0;

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
    //ODBC 환경 및 연결 초기화
    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &_hEnv);
    CheckReturn(ret);

    // ODBC 버전 설정
    ret = SQLSetEnvAttr(_hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, SQL_IS_INTEGER);
    CheckReturn(ret);

    //DB연결을 위한 쿼리 작성을 위해서, 설정을 환경 변수에서 읽어냄 (다른사람이 보면 곤란)
    ifstream envFile(".env");
    if (envFile.is_open())
        cout << ".env Open Succeed!" << endl;
    else
        cout << ".env Open Faild" << endl;
    string line;

    while (getline(envFile, line)) {
        // 공백이나 주석을 무시하고, "=" 기준으로 KEY=VALUE 쌍을 추출
        if (line.empty() || line[0] == '#') continue;

        // KEY=VALUE에서 VALUE가 따옴표로 감싸져 있다면 이를 제거
        size_t eqPos = line.find("=");
        if (eqPos != string::npos) {
            string key = line.substr(0, eqPos);
            string value = line.substr(eqPos + 1);

            // VALUE가 따옴표로 묶여있을 경우, 제거
            if (value.front() == '"' && value.back() == '"')
                value = value.substr(1, value.length() - 2);

            //Unicode로 변환 (SetEnvironmentVariableW 사용을 위해서)
            //한글이 포함되지 않으면서(ascii 코드 내로 읽히는 문자들), 동시에 Windows환경이라서 제대로 동작한다.
            wstring wkey(key.begin(), key.end());
            wstring wvalue(value.begin(), value.end());

            // 환경 변수 설정
            if (SetEnvironmentVariableW(wkey.c_str(), wvalue.c_str()))
                cout << "환경 변수 : " << key << " 설정 성공." << std::endl;
            else
                cout << "환경 변수 설정 실패 : " << key << " (에러 코드: " << GetLastError() << ")" << std::endl;
        }
    }
    envFile.close();

    //2. 환경 변수로 등록된 문자열(유니코드)들을 매핑
    wchar_t* dbServer = nullptr;
    wchar_t* dbName = nullptr;
    wchar_t* connection = nullptr;
    size_t wSize;
    errno_t err;

    err = _wdupenv_s(&dbServer, &wSize, L"DB_SERVER");
    if (err == 0 && dbServer != nullptr)
        wcout << L"DB_SERVER" << " = " << dbServer << endl;

    err = _wdupenv_s(&dbName, &wSize, L"DB_NAME");
    if (err == 0 && dbName != nullptr)
        wcout << L"DB_NAME" << " = " << dbName << endl;

    err = _wdupenv_s(&connection, &wSize, L"CONNECTION");
    if (err == 0 && connection != nullptr)
        wcout << L"CONNECTION" << " = " << connection << endl;

    if (dbServer && dbName && connection) {
        SQLWCHAR connStr[1024];
        _snwprintf_s(connStr, sizeof(connStr) / sizeof(SQLWCHAR),
            L"Driver={ODBC Driver 17 for SQL Server};Server=%ls;Database=%ls;%ls;",
            dbServer, dbName, connection);
        //기본 CMD에서 유니코드 한글이 출력되지 않음....
        wcout << L"connStr : " << connStr << endl;

        //연결 핸들 pool에 연결핸들 3개 추가. 초기화단계에서는 1개의 스레드에서
        //실행되는 것이 자명하므로 굳이 mutex걸지 않음.
        for (int i = 0; i < 3; i++) {
            SQLHDBC hDbc = nullptr;

            ret = SQLAllocHandle(SQL_HANDLE_DBC, _hEnv, &hDbc);
            if (!CheckReturn(ret)) {
                cout << "핸들 할당 실패" << endl;
                continue;
            }

            ret = SQLDriverConnectW(hDbc, NULL, connStr, wcslen(connStr), NULL, 0, NULL, SQL_DRIVER_COMPLETE);
            if (CheckReturn(ret)) {
                _hDbcQ.push(hDbc);
                cout << "DB 연결 성공" << endl;
            }
            else {
                SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
            }
        }

    }
    else
        cout << "환경 변수가 설정되지 않아, DB연결을 수행하지 못함" << endl;

    delete[] dbServer;
    delete[] dbName;
    delete[] connection;

    try {
        InitialC();
        InitialR();
        InitialU();
        InitialD();
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
        }  // 연결 핸들 해제
    }
    SQLFreeHandle(SQL_HANDLE_ENV, _hEnv);  // 환경 핸들 해제
}

bool DBManager::CheckReturn(SQLRETURN& ret) {
    if (!SQL_SUCCEEDED(ret)) {
        SQLWCHAR SQLState[6];  // SQLSTATE는 5개의 문자를 사용 + null 종료 문자를 위해 6
        SQLWCHAR message[256]; // 메시지 버퍼 크기
        SQLINTEGER NativeError;
        SQLSMALLINT msgLength;

        SQLHDBC hDbc = popHDbc();
        // SQLGetDiagRecW에서 유니코드 오류 메시지 받아오기
        SQLRETURN rreett = SQLGetDiagRecW(SQL_HANDLE_DBC, hDbc, 1, SQLState, &NativeError, message, sizeof(message) / sizeof(SQLWCHAR), &msgLength);

        if (rreett == SQL_SUCCESS || rreett == SQL_SUCCESS_WITH_INFO) {
            wcout << L"SQLState: " << SQLState << L", Message: " << message << endl;
        }
        else {
            wcout << L"Error retrieving diagnostic information" << endl;
        }
        returnHDbc(hDbc);
        return false;
    }
    return true;
}

bool DBManager::CheckReturn(SQLRETURN& ret, SQLHANDLE hHandle, SQLSMALLINT HandleType) {
    if (SQL_SUCCEEDED(ret)) {
        return true;
    }

    SQLWCHAR sqlState[6];
    SQLWCHAR messageText[SQL_MAX_MESSAGE_LENGTH + 1]; // ODBC 표준 최대 메시지 길이
    SQLINTEGER nativeError;
    SQLSMALLINT messageLength;
    SQLSMALLINT recNumber = 1; // 첫 번째 진단 레코드부터 시작

    // 모든 진단 레코드를 가져오기 위해 루프를 사용합니다.
    while (SQLGetDiagRecW(HandleType, hHandle, recNumber, sqlState, &nativeError,
        messageText, SQL_MAX_MESSAGE_LENGTH + 1, &messageLength) == SQL_SUCCESS) {

        wcerr << L"ODBC Error - Type: " << (HandleType == SQL_HANDLE_ENV ? L"ENV" :
            HandleType == SQL_HANDLE_DBC ? L"DBC" :
            HandleType == SQL_HANDLE_STMT ? L"STMT" :
            HandleType == SQL_HANDLE_DESC ? L"DESC" : L"UNKNOWN")
            << L", Rec#: " << recNumber
            << L", SQLState: " << sqlState
            << L", NativeError: " << nativeError
            << L", Message: " << messageText << endl;

        recNumber++;
    }

    // 예외 던질 수 있음.
    // throw runtime_error("");
    return false;
}

wstring DBManager::a2wsRef(const string& in_cp949) {
    if (in_cp949.empty())
        return L"";

    // 1. 필요한 wchar_t 버퍼 크기 계산 (NULL 종료 문자 포함)
    int w_len = MultiByteToWideChar(CP_ACP, 0, in_cp949.c_str(), -1, NULL, 0);
    if (w_len == 0)
        throw runtime_error("a2ws: Failed to get required length. LastError: " + to_string(GetLastError()));

    // 2. 할당
    wstring ws;
    ws.resize(w_len - 1);

    //값 채우기
    if (!MultiByteToWideChar(CP_ACP, 0, in_cp949.c_str(), -1, &ws[0], w_len))
        throw runtime_error("a2ws: Failed to convert string to wchar. LastError: " + std::to_string(GetLastError()));

    return ws;
}

wstring DBManager::s2wsRef(const string& in_u8s) {
    if (in_u8s.empty())
        return L"";

    // 1. 필요한 wchar_t 버퍼 크기 계산 (NULL 종료 문자 포함)
    int w_len = MultiByteToWideChar(CP_UTF8, 0, in_u8s.c_str(), -1, NULL, 0);
    if (w_len == 0)
        throw runtime_error("s2ws: Failed to get required length. LastError: " + to_string(GetLastError()));

    // 2. 할당
    wstring ws;
    ws.resize(w_len - 1);

    //값 채우기
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
        throw runtime_error("CreateQuery: 인자 갯수가 맞지 않습니다.");

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

SQLHDBC DBManager::connectNewHDbc() {
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
            if (!CheckReturn(ret)) {
                throw runtime_error("핸들 할당 실패");
            }

            ret = SQLDriverConnectW(hDbc, NULL, connStr, wcslen(connStr), NULL, 0, NULL, SQL_DRIVER_COMPLETE);
            if (CheckReturn(ret)) {
                cout << "DB 연결 성공" << endl;
            }
            else {
                SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
                throw runtime_error("DB 연결 실패");
            }
        }
        else
            throw runtime_error("환경 변수에 값이 올바르지 않음");
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

SQLHDBC DBManager::popHDbc() {
    SQLHDBC hDbc = nullptr;
    {
        lock_guard<mutex> lock(_mtx);
        if (!_hDbcQ.empty()) {
            hDbc = _hDbcQ.front();
            _hDbcQ.pop();
            return hDbc;
        }
    }
    hDbc = connectNewHDbc();
    return hDbc;
}

void DBManager::returnHDbc(SQLHDBC hDbc) {
    lock_guard<mutex> lock(_mtx);
    _hDbcQ.push(hDbc);
}

void DBManager::InitialC() {
    SQLHSTMT hStmt;
    SQLHDBC hDbc = popHDbc();
    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);

    wstring query = L"INSERT INTO CRUD (id, value) VALUES (?, ?)";
    ret = SQLPrepareW(hStmt, (SQLWCHAR*)query.c_str(), SQL_NTS);
    if (!CheckReturn(ret)) {
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        throw runtime_error("InitialC Failed.");
    }

    int id = 0, value = 10;
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &id, 0, NULL);
    SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &value, 0, NULL);

    ret = SQLExecute(hStmt);
    if (!CheckReturn(ret)) {
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
    SQLHDBC hDbc = popHDbc();
    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
    wstring query = L"SELECT id, value FROM CRUD";
    ret = SQLExecDirectW(hStmt, (SQLWCHAR*)query.c_str(), SQL_NTS);
    if (!CheckReturn(ret)) {
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        returnHDbc(hDbc);
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
    returnHDbc(hDbc);
}

void DBManager::InitialU() {
    SQLHSTMT hStmt;
    SQLHDBC hDbc = popHDbc();
    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
    wstring query = L"UPDATE CRUD SET value = ? WHERE id = ?";
    ret = SQLPrepareW(hStmt, (SQLWCHAR*)query.c_str(), SQL_NTS);
    if (!CheckReturn(ret, hStmt, SQL_HANDLE_STMT)) {
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        returnHDbc(hDbc);
        throw runtime_error("InitialU Failed.");
    }

    int id = 0;
    int uvalue = 20;
    ret = SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &uvalue, 0, NULL);
    if (!CheckReturn(ret, hStmt, SQL_HANDLE_STMT)) {
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        returnHDbc(hDbc);
        throw runtime_error("InitialU Failed.");
    }

    ret = SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &id, 0, NULL);
    if (!CheckReturn(ret, hStmt, SQL_HANDLE_STMT)) {
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        returnHDbc(hDbc);
        throw runtime_error("InitialU Failed.");
    }

    ret = SQLExecute(hStmt);
    if (!CheckReturn(ret, hStmt, SQL_HANDLE_STMT)) {
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        returnHDbc(hDbc);
        throw runtime_error("InitialU Failed.");
    }
    else {
        cout << "U sequence is done. value: " << uvalue << endl;
    }
    returnHDbc(hDbc);
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}

void DBManager::InitialD() {
    SQLHSTMT hStmt;
    SQLHDBC hDbc = popHDbc();
    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
    wstring query = L"DELETE FROM CRUD WHERE id = ?";
    ret = SQLPrepareW(hStmt, (SQLWCHAR*)query.c_str(), SQL_NTS);
    if (!CheckReturn(ret, hStmt, SQL_HANDLE_STMT)) {
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        throw runtime_error("InitialD Failed.");
    }

    int id = 0;
    ret = SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &id, 0, NULL);
    if (!CheckReturn(ret, hStmt, SQL_HANDLE_STMT)) {
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        returnHDbc(hDbc);
        throw runtime_error("InitialD Failed.");
    }

    ret = SQLExecute(hStmt);
    if (!CheckReturn(ret, hStmt, SQL_HANDLE_STMT)) {
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        returnHDbc(hDbc);
        throw runtime_error("InitialD Failed.");
    }
    else {
        cout << "D sequence is done." << endl;
    }
    returnHDbc(hDbc);
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}

ThreadManager::ThreadManager() {
    InitTLS();
}

ThreadManager::~ThreadManager() {
    Join();
}

void ThreadManager::InitTLS() {
    //NxtThreadID는 오로지 InitTLS함수로만 관리되어야함.
    //따라서 static함수 스코프 내에 static변수로 선언
    //이 함수 스코프를 벗어나도 NxtThreadID 변수는 살아있다.
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

