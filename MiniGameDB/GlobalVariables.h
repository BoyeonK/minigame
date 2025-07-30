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

    wstring combineWideStrings(initializer_list<unique_ptr<wchar_t[]>> wsRefs) {
        if (wsRefs.size() < 3 or (wsRefs.size() / 2) == 0) {
            throw runtime_error("CreateQueryA2W: 인자 갯수가 맞지 않습니다.");
        }

        wstring insertQuery = L"INSERT INTO ";

        // 1. 최종 wstring의 총 길이 계산 (널 종료 문자는 제외)
        size_t total_len = 0;
        for (const auto& uptr : wsRefs)
            if (uptr)
                total_len += wcslen(uptr.get()); // wcslen은 널 종료 문자를 포함하지 않는 길이 반환

        // 2. 최종 wstring 생성 및 각 문자열 이어 붙이기
        wstring result;
        result.reserve(total_len);
        for (const auto& uptr : wsRefs)
            if (uptr)
                result.append(uptr.get()); // unique_ptr이 관리하는 wchar_t*를 append

        return result;
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

