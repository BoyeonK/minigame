#pragma once
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

extern class DBManager* GDBManager;

class DBManager {
public:
	DBManager() : _hEnv(nullptr), _hDbc(nullptr) {
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
            CheckReturn(ret);
        }
        else
            cout << "ȯ�� ������ �������� �ʾҽ��ϴ�." << endl;

        delete[] dbServer;
        delete[] dbName;
        delete[] connection;
        cout << "DB ���ῡ �����߽��ϴ�." << endl;
	}

    ~DBManager() {
        SQLFreeHandle(SQL_HANDLE_DBC, _hDbc);  // ���� �ڵ� ����
        SQLFreeHandle(SQL_HANDLE_ENV, _hEnv);  // ȯ�� �ڵ� ����
    }

    bool CheckReturn(SQLRETURN& ret) {
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

private:
	SQLHENV _hEnv;
	SQLHDBC _hDbc;
};

