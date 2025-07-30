#include "GlobalVariables.h"
#include <iostream>
#include <iomanip>
#include <windows.h>

using namespace std;

int main() {
    wcout.imbue(locale("korean"));

    string a = "alpha";
    string b = "º£Å¸";

    cout << a << endl;
    cout << b << endl;

    wstring tableName = L"HandShake";
    wstring colName = L"value";
    wstring value = L"10";

    try {
        wstring q = GDBManager->CreateQuery(tableName, { L"value1", L"value2", L"'10'", L"'20'" });
        wcout << q << endl;
    } catch (const runtime_error& e) {
        cout << e.what() << endl;
    }
   
    return 0;
}