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
            throw runtime_error("CreateQueryA2W: ���� ������ ���� �ʽ��ϴ�.");
        }

        wstring insertQuery = L"INSERT INTO ";

        // 1. ���� wstring�� �� ���� ��� (�� ���� ���ڴ� ����)
        size_t total_len = 0;
        for (const auto& uptr : wsRefs)
            if (uptr)
                total_len += wcslen(uptr.get()); // wcslen�� �� ���� ���ڸ� �������� �ʴ� ���� ��ȯ

        // 2. ���� wstring ���� �� �� ���ڿ� �̾� ���̱�
        wstring result;
        result.reserve(total_len);
        for (const auto& uptr : wsRefs)
            if (uptr)
                result.append(uptr.get()); // unique_ptr�� �����ϴ� wchar_t*�� append

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

