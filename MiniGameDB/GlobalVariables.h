#pragma once
#include <windows.h>
#include <sqlext.h>
#include <sqltypes.h> 
#include <sql.h> 
#include <cstdlib> 
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <locale>
#include <codecvt>
#include <cwchar>
#include <vector>
#include <memory>

using namespace std;

extern class DBManager* GDBManager;

class DBManager {
public:
    DBManager();
    ~DBManager();

    bool CheckReturn(SQLRETURN& ret);

    wstring a2wsRef(const string& in_cp949);
    wstring s2wsRef(const string& in_u8s);

    //이대로 사용하면 코드인젝션에 굉장히 취약함!
    wstring CreateQuery(const wstring& tableName, initializer_list<wstring> wstrs) {
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

    /*
    wchar_t* ReadQueryA2W();
    wchar_t* UpdateQueryA2W();
    wchar_t* DeleteQueryA2W();
    */

private:
	SQLHENV _hEnv;
	SQLHDBC _hDbc;
};

