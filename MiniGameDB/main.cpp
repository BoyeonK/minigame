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

// 오류 상태 확인 함수
bool CheckReturn(SQLRETURN ret, SQLHDBC& hDbc) {
    if (!SQL_SUCCEEDED(ret)) {
        SQLWCHAR SQLState[6];  // SQLSTATE는 5개의 문자를 사용 + null 종료 문자를 위해 6
        SQLWCHAR message[256]; // 메시지 버퍼 크기
        SQLINTEGER NativeError;
        SQLSMALLINT msgLength;

        // SQLGetDiagRecW에서 유니코드 오류 메시지 받아오기
        SQLGetDiagRecW(SQL_HANDLE_DBC, hDbc, 1, SQLState, &NativeError, message, sizeof(message) / sizeof(SQLWCHAR), &msgLength);
        wcout << L"SQLState: " << SQLState << L", Message: " << message << endl;
        return false;
    }
    return true;
}

// ODBC 환경 초기화 및 연결 함수
void InitializeODBC(SQLHENV& hEnv, SQLHDBC& hDbc) {
    SQLRETURN ret;

    // 환경 핸들 할당
    ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
    CheckReturn(ret, hDbc);

    // ODBC 버전 설정
    ret = SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
    CheckReturn(ret, hDbc);

    // 연결 핸들 할당
    ret = SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
    CheckReturn(ret, hDbc);
}

// 데이터베이스 연결 함수
void ConnectToDatabase(SQLHDBC hDbc) {
    SQLWCHAR connStr[] = L"Driver={ODBC Driver 17 for SQL Server};Server=localhost\\SQLEXPRESS;Database=MyGameDB;Trusted_Connection=Yes;";
    SQLRETURN ret;

    ret = SQLDriverConnectW(hDbc, NULL, connStr, wcslen(connStr), NULL, 0, NULL, SQL_DRIVER_COMPLETE);
    CheckReturn(ret, hDbc);
}

void Cleanup(SQLHDBC hDbc, SQLHENV hEnv) {
    SQLFreeHandle(SQL_HANDLE_DBC, hDbc);  // 연결 핸들 해제
    SQLFreeHandle(SQL_HANDLE_ENV, hEnv);  // 환경 핸들 해제
}

wstring s2ws(const string& str) {
    wstring wstr(str.begin(), str.end());
    return wstr;
}

/*
//변환 문자열이 ASCII기반이 아닐 경우까지 감안할 경우.
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

    // 환경 변수 값 읽기
    errno_t err = _wdupenv_s(&value, &valueSize, varName);
    if (err == 0 && value != nullptr) {
        wcout << varName << L" = " << value << endl;
        return value;  // 성공적으로 값을 읽음
    }
    else {
        wcout << L"환경 변수 " << varName << L"을(를) 읽을 수 없습니다." << endl;
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
            // 공백이나 주석을 무시하고, "=" 기준으로 KEY=VALUE 쌍을 추출
            if (line.empty() || line[0] == '#') continue;

            // KEY=VALUE에서 VALUE가 따옴표로 감싸져 있다면 이를 제거
            size_t eqPos = line.find("=");
            if (eqPos != string::npos) {
                string key = line.substr(0, eqPos);
                string value = line.substr(eqPos + 1);

                // VALUE에서 따옴표 제거
                if (value.front() == '"' && value.back() == '"')
                    value = value.substr(1, value.length() - 2);

                wstring wkey = s2ws(key);
                wstring wvalue = s2ws(value);

                // 환경 변수 설정 (Windows에서는 SetEnvironmentVariableW 사용)
                if (SetEnvironmentVariableW(wkey.c_str(), wvalue.c_str()))
                    cout << "설정된 환경 변수: " << key << "=" << value << std::endl;
                else
                    cout << "환경 변수 설정 실패: " << key << " (에러 코드: " << GetLastError() << ")" << std::endl;
            }
        }
        envFile.close();
    }

    wchar_t* dbServer = GetEnvVar(L"DB_SERVER");
    wchar_t* dbName = GetEnvVar(L"DB_NAME");
    wchar_t* connection = GetEnvVar(L"CONNECTION");

    // ODBC 환경 및 연결 초기화
    ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
    CheckReturn(ret, hDbc);

    // ODBC 버전 설정
    ret = SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
    CheckReturn(ret, hDbc);

    // 연결 핸들 할당
    ret = SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
    CheckReturn(ret, hDbc);

    // 데이터베이스에 연결
    if (dbServer && dbName && connection) {
        SQLWCHAR connStr[1024];
        _snwprintf_s(connStr, sizeof(connStr) / sizeof(SQLWCHAR),
            L"Driver={ODBC Driver 17 for SQL Server};Server=%s;Database=%s;%s;",
            dbServer, dbName, connection);

        wcout << L"연결 문자열: " << connStr << endl;

        ret = SQLDriverConnectW(hDbc, NULL, connStr, wcslen(connStr), NULL, 0, NULL, SQL_DRIVER_COMPLETE);
        CheckReturn(ret, hDbc);
    }
    else
        wcout << L"환경 변수가 설정되지 않았습니다." << endl;

    // 리소스 해제
    delete[] dbServer;
    delete[] dbName;
    delete[] connection;
    Cleanup(hDbc, hEnv);

    return 0;
}