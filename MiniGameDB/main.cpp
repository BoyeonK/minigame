#include "GlobalVariables.h"
#include <iostream>
#include <iomanip>
#include <windows.h>

using namespace std;

int main() {
    wcout.imbue(locale("korean"));

    string a = "alpha";
    string b = "베타";

    cout << a << endl;
    cout << b << endl;

    string uu = "나는문어";
    string aa = "Dreaming Octopus";

    try {
        wstring uuRef = GDBManager->a2wsRef(uu);
        wcout << uuRef << endl;
    } catch (const runtime_error& e) {
        cout << e.what() << endl;
    }

    try {
        wstring aaRef = GDBManager->a2wsRef(aa);
        wcout << aaRef << endl;
    } catch (const runtime_error& e) {
        cout << e.what() << endl;
    }
   
    return 0;
}