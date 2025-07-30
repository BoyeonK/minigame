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

using namespace std;

extern class DBManager* GDBManager;

class DBManager {
public:
    DBManager();
    ~DBManager();

    bool CheckReturn(SQLRETURN& ret);
    wchar_t* s2ws(const string& in_u8s) {
        if (in_u8s.empty())
            return nullptr;

        // 1. �ʿ��� wchar_t ���� ũ�� ��� (NULL ���� ���� ����)
        int wide_char_len = MultiByteToWideChar(CP_UTF8, 0, in_u8s.c_str(), -1, NULL, 0);
        if (wide_char_len == 0)
            throw runtime_error("s2ws: Failed to get required length. LastError: " + to_string(GetLastError()));

        // 2. �Ҵ�
        wchar_t* ws = new wchar_t[wide_char_len];

        //�� ä���
        if (!MultiByteToWideChar(CP_UTF8, 0, in_u8s.c_str(), -1, ws, wide_char_len)) {
            delete[] ws;
            throw runtime_error("s2ws: Failed to convert string to wchar. LastError: " + std::to_string(GetLastError()));
        }

        return ws;
    }

    /*
    wchar_t* CreateQueryA2W(const Args&... args)
    wchar_t* ReadQueryA2W();
    wchar_t* UpdateQueryA2W();
    wchar_t* DeleteQueryA2W();
    */
private:
	SQLHENV _hEnv;
	SQLHDBC _hDbc;
};

