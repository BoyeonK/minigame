#define _WIN32_WINNT 0x0601
#define NOMINMAX

#pragma once

#include <WinSock2.h>
#include <windows.h>

#include <vector>
#include <memory>
#include <string>
#include <fstream>
#include <sstream>

#include <iostream>
#include <locale>
#include <codecvt>
#include <cwchar>
#include <sqlext.h>
#include <sqltypes.h> 
#include <sql.h> 
#include <cstdlib> 

using namespace std;

extern class DBManager* GDBManager;

class DBManager {
public:
    DBManager();
    ~DBManager();
    
    bool CheckReturn(SQLRETURN& ret);
    bool CheckReturn(SQLRETURN& ret, SQLHANDLE hHandle, SQLSMALLINT HandleType);

    wstring a2wsRef(const string& in_cp949);
    wstring s2wsRef(const string& in_u8s);

    //이대로 사용하면 코드인젝션에 굉장히 취약함!
    wstring CreateQuery(const wstring& tableName, initializer_list<wstring> wstrs);

    /*
    wchar_t* ReadQueryA2W();
    wchar_t* UpdateQueryA2W();
    wchar_t* DeleteQueryA2W();
    */

private:
	SQLHENV _hEnv;
	SQLHDBC _hDbc;

    //생성자에서 CRUD 테스트를 위해 사용할 함수들
    void InitialC();
    void InitialR();
    void InitialU();
    void InitialD();
};

