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
                //한글이 포함되지 않으면서, 동시에 Windows환경이라서 제대로 동작한다.
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
        
        wchar_t* dbServer = nullptr;
        wchar_t* dbName = nullptr;
        wchar_t* connection = nullptr;
        size_t wSize;
        errno_t err;

        //환경 변수로 등록된 문자열(유니코드)들을 매핑
        err = _wdupenv_s(&dbServer, &wSize, L"DB_SERVER");
        if (err == 0 && dbServer != nullptr)
            wcout << L"DB_SERVER" << " = " << dbServer << endl;

        err = _wdupenv_s(&dbName, &wSize, L"DB_NAME");
        if (err == 0 && dbName != nullptr)
            wcout << L"DB_NAME" << " = " << dbName << endl;

        err = _wdupenv_s(&connection, &wSize, L"CONNECTION");
        if (err == 0 && connection != nullptr)
            wcout << L"CONNECTION" << " = " << connection << endl;

        // ODBC 환경 및 연결 초기화
        SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &_hEnv);
        CheckReturn(ret);

        // ODBC 버전 설정
        ret = SQLSetEnvAttr(_hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
        CheckReturn(ret);

        // 연결 핸들 할당
        ret = SQLAllocHandle(SQL_HANDLE_DBC, _hEnv, &_hDbc);
        CheckReturn(ret);

        if (dbServer && dbName && connection) {
            SQLWCHAR connStr[1024];
            _snwprintf_s(connStr, sizeof(connStr) / sizeof(SQLWCHAR),
                L"Driver={ODBC Driver 17 for SQL Server};Server=%ls;Database=%ls;%ls;",
                dbServer, dbName, connection);
            //기본 CMD에서 유니코드 한글이 출력되지 않음....
            wcout << L"connStr : " << connStr << endl;

            ret = SQLDriverConnectW(_hDbc, NULL, connStr, wcslen(connStr), NULL, 0, NULL, SQL_DRIVER_COMPLETE);
            CheckReturn(ret);
        }
        else
            cout << "환경 변수가 설정되지 않았습니다." << endl;

        delete[] dbServer;
        delete[] dbName;
        delete[] connection;
        cout << "DB 연결에 성공했습니다." << endl;
	}

    ~DBManager() {
        SQLFreeHandle(SQL_HANDLE_DBC, _hDbc);  // 연결 핸들 해제
        SQLFreeHandle(SQL_HANDLE_ENV, _hEnv);  // 환경 핸들 해제
    }

    bool CheckReturn(SQLRETURN& ret) {
        if (!SQL_SUCCEEDED(ret)) {
            SQLWCHAR SQLState[6];  // SQLSTATE는 5개의 문자를 사용 + null 종료 문자를 위해 6
            SQLWCHAR message[256]; // 메시지 버퍼 크기
            SQLINTEGER NativeError;
            SQLSMALLINT msgLength;

            // SQLGetDiagRecW에서 유니코드 오류 메시지 받아오기
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

