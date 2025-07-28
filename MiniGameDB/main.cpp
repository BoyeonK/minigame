#include <windows.h>
#include <sqlext.h>
#include <sqltypes.h> 
#include <sql.h> 
#include <cstdlib> 
#include <string>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

using namespace std;

// ���� ���� Ȯ�� �Լ�
bool CheckReturn(SQLRETURN ret, SQLHDBC& hDbc) {
    if (!SQL_SUCCEEDED(ret)) {
        SQLWCHAR SQLState[6];  // SQLSTATE�� 5���� ���ڸ� ��� + null ���� ���ڸ� ���� 6
        SQLWCHAR message[256]; // �޽��� ���� ũ��
        SQLINTEGER NativeError;
        SQLSMALLINT msgLength;

        // SQLGetDiagRecW���� �����ڵ� ���� �޽��� �޾ƿ���
        SQLGetDiagRecW(SQL_HANDLE_DBC, hDbc, 1, SQLState, &NativeError, message, sizeof(message) / sizeof(SQLWCHAR), &msgLength);
        wcout << L"SQLState: " << SQLState << L", Message: " << message << endl;
        return false;
    }
    return true;
}

// ODBC ȯ�� �ʱ�ȭ �� ���� �Լ�
void InitializeODBC(SQLHENV& hEnv, SQLHDBC& hDbc) {
    SQLRETURN ret;

    // ȯ�� �ڵ� �Ҵ�
    ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
    CheckReturn(ret, hDbc);

    // ODBC ���� ����
    ret = SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
    CheckReturn(ret, hDbc);

    // ���� �ڵ� �Ҵ�
    ret = SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
    CheckReturn(ret, hDbc);
}

// �����ͺ��̽� ���� �Լ�
void ConnectToDatabase(SQLHDBC hDbc) {
    SQLWCHAR connStr[] = L"Driver={ODBC Driver 17 for SQL Server};Server=localhost\\SQLEXPRESS;Database=MyGameDB;Trusted_Connection=Yes;";
    SQLRETURN ret;

    ret = SQLDriverConnectW(hDbc, NULL, connStr, wcslen(connStr), NULL, 0, NULL, SQL_DRIVER_COMPLETE);
    CheckReturn(ret, hDbc);
}

void Cleanup(SQLHDBC hDbc, SQLHENV hEnv) {
    SQLFreeHandle(SQL_HANDLE_DBC, hDbc);  // ���� �ڵ� ����
    SQLFreeHandle(SQL_HANDLE_ENV, hEnv);  // ȯ�� �ڵ� ����
}

wstring s2ws(const string& str) {
    wstring wstr(str.begin(), str.end());
    return wstr;
}

/*
//��ȯ ���ڿ��� ASCII����� �ƴ� ������ ������ ���.
wstring s2ws(const string& s) {
    int len;
    int slength = (int)s.length() + 1;
    len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
    wchar_t* buf = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
    wstring r(buf);
    delete[] buf;
    return r;
}
*/

wchar_t* GetEnvVar(const wchar_t* varName) {
    wchar_t* value = nullptr;
    size_t valueSize = 0;

    // ȯ�� ���� �� �б�
    errno_t err = _wdupenv_s(&value, &valueSize, varName);
    if (err == 0 && value != nullptr) {
        wcout << varName << L" = " << value << endl;
        return value;  // ���������� ���� ����
    }
    else {
        wcout << L"ȯ�� ���� " << varName << L"��(��) ���� �� �����ϴ�." << endl;
        return nullptr;
    }
}

int main() {
    SQLHENV hEnv;
    SQLHDBC hDbc;
    SQLRETURN ret;
    string line;

    ifstream envFile(".env");

    if (envFile.is_open()) {
        cout << ".env Open Succeed!" << endl;
        while (getline(envFile, line)) {
            // �����̳� �ּ��� �����ϰ�, "=" �������� KEY=VALUE ���� ����
            if (line.empty() || line[0] == '#') continue;

            // KEY=VALUE���� VALUE�� ����ǥ�� ������ �ִٸ� �̸� ����
            size_t eqPos = line.find("=");
            if (eqPos != string::npos) {
                string key = line.substr(0, eqPos);
                string value = line.substr(eqPos + 1);

                // VALUE���� ����ǥ ����
                if (value.front() == '"' && value.back() == '"')
                    value = value.substr(1, value.length() - 2);

                wstring wkey = s2ws(key);
                wstring wvalue = s2ws(value);

                // ȯ�� ���� ���� (Windows������ SetEnvironmentVariableW ���)
                if (SetEnvironmentVariableW(wkey.c_str(), wvalue.c_str()))
                    cout << "������ ȯ�� ����: " << key << "=" << value << std::endl;
                else
                    cout << "ȯ�� ���� ���� ����: " << key << " (���� �ڵ�: " << GetLastError() << ")" << std::endl;
            }
        }
        envFile.close();
    }

    wchar_t* dbServer = GetEnvVar(L"DB_SERVER");
    wchar_t* dbName = GetEnvVar(L"DB_NAME");
    wchar_t* connection = GetEnvVar(L"CONNECTION");

    // ODBC ȯ�� �� ���� �ʱ�ȭ
    ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
    CheckReturn(ret, hDbc);

    // ODBC ���� ����
    ret = SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
    CheckReturn(ret, hDbc);

    // ���� �ڵ� �Ҵ�
    ret = SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
    CheckReturn(ret, hDbc);

    // �����ͺ��̽��� ����
    if (dbServer && dbName && connection) {
        SQLWCHAR connStr[1024];
        _snwprintf_s(connStr, sizeof(connStr) / sizeof(SQLWCHAR),
            L"Driver={ODBC Driver 17 for SQL Server};Server=%s;Database=%s;%s;",
            dbServer, dbName, connection);

        wcout << L"���� ���ڿ�: " << connStr << endl;

        ret = SQLDriverConnectW(hDbc, NULL, connStr, wcslen(connStr), NULL, 0, NULL, SQL_DRIVER_COMPLETE);
        CheckReturn(ret, hDbc);
    }
    else
        wcout << L"ȯ�� ������ �������� �ʾҽ��ϴ�." << endl;

    // ���ҽ� ����
    delete[] dbServer;
    delete[] dbName;
    delete[] connection;
    Cleanup(hDbc, hEnv);

    return 0;
}