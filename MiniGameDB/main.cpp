#include "GlobalVariables.h"
#include <iostream>
#include <iomanip>
#include <windows.h>

using namespace std;


void might_fail_function(int value) {
    if (value < 0) {
        throw runtime_error("Negative value not allowed!"); // 예외 발생
    }
    cout << "Function completed successfully with value: " << value << endl;
}

int main() {
    wcout.imbue(locale("korean"));

    string a = "alpha";
    string b = "베타";

    cout << a << endl;
    cout << b << endl;

    string uu = "나는문어";
    string aa = "Dreaming Octopus";

    try {
        unique_ptr<wchar_t[]> uuRef = GDBManager->a2ws(uu);
        if (uuRef.get() != nullptr) {
            wcout << uuRef.get() << endl;
        }
        else {
            wcout << L"Converted string is empty (nullptr)." << std::endl;
        }
    } catch (const runtime_error& e) {
        cout << e.what() << endl;
    }

    try {
        unique_ptr<wchar_t[]> aaRef = GDBManager->a2ws(aa);
        if (aaRef.get() != nullptr) {
            wcout << aaRef.get() << endl;
        }
        else {
            wcout << L"Converted string is empty (nullptr)." << std::endl;
        }
    } catch (const runtime_error& e) {
        cout << e.what() << endl;
    }
       
    return 0;
}