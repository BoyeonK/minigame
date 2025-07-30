#include "GlobalVariables.h"

DBManager* GDBManager = nullptr;

class DBGlobal {
public:
	DBGlobal() {
		GDBManager = new DBManager();
	}

	~DBGlobal() {
		delete GDBManager;
	}

} GDBGlobal;

DBManager::DBManager() : _hEnv(nullptr), _hDbc(nullptr) {
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
            //�ѱ��� ���Ե��� �����鼭, ���ÿ� Windowsȯ���̶� ����� �����Ѵ�.
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

    wchar_t* dbServer = nullptr;
    wchar_t* dbName = nullptr;
    wchar_t* connection = nullptr;
    size_t wSize;
    errno_t err;

    //ȯ�� ������ ��ϵ� ���ڿ�(�����ڵ�)���� ����
    err = _wdupenv_s(&dbServer, &wSize, L"DB_SERVER");
    if (err == 0 && dbServer != nullptr)
        wcout << L"DB_SERVER" << " = " << dbServer << endl;

    err = _wdupenv_s(&dbName, &wSize, L"DB_NAME");
    if (err == 0 && dbName != nullptr)
        wcout << L"DB_NAME" << " = " << dbName << endl;

    err = _wdupenv_s(&connection, &wSize, L"CONNECTION");
    if (err == 0 && connection != nullptr)
        wcout << L"CONNECTION" << " = " << connection << endl;

    // ODBC ȯ�� �� ���� �ʱ�ȭ
    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &_hEnv);
    CheckReturn(ret);

    // ODBC ���� ����
    ret = SQLSetEnvAttr(_hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
    CheckReturn(ret);

    // ���� �ڵ� �Ҵ�
    ret = SQLAllocHandle(SQL_HANDLE_DBC, _hEnv, &_hDbc);
    CheckReturn(ret);

    if (dbServer && dbName && connection) {
        SQLWCHAR connStr[1024];
        _snwprintf_s(connStr, sizeof(connStr) / sizeof(SQLWCHAR),
            L"Driver={ODBC Driver 17 for SQL Server};Server=%ls;Database=%ls;%ls;",
            dbServer, dbName, connection);
        //�⺻ CMD���� �����ڵ� �ѱ��� ��µ��� ����....
        wcout << L"connStr : " << connStr << endl;

        ret = SQLDriverConnectW(_hDbc, NULL, connStr, wcslen(connStr), NULL, 0, NULL, SQL_DRIVER_COMPLETE);
        if (CheckReturn(ret)) {
            cout << "DB ���ῡ �����߽��ϴ�." << endl;
        }
    }
    else
        cout << "ȯ�� ������ �������� �ʾҽ��ϴ�." << endl;

    delete[] dbServer;
    delete[] dbName;
    delete[] connection;
}

DBManager::~DBManager() {
    SQLFreeHandle(SQL_HANDLE_DBC, _hDbc);  // ���� �ڵ� ����
    SQLFreeHandle(SQL_HANDLE_ENV, _hEnv);  // ȯ�� �ڵ� ����
}

bool DBManager::CheckReturn(SQLRETURN& ret) {
    if (!SQL_SUCCEEDED(ret)) {
        SQLWCHAR SQLState[6];  // SQLSTATE�� 5���� ���ڸ� ��� + null ���� ���ڸ� ���� 6
        SQLWCHAR message[256]; // �޽��� ���� ũ��
        SQLINTEGER NativeError;
        SQLSMALLINT msgLength;

        // SQLGetDiagRecW���� �����ڵ� ���� �޽��� �޾ƿ���
        SQLGetDiagRecW(SQL_HANDLE_DBC, _hDbc, 1, SQLState, &NativeError, message, sizeof(message) / sizeof(SQLWCHAR), &msgLength);
        wcout << L"SQLState: " << SQLState << L", Message: " << message << endl;
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
