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
    string a = "alpha";
    string b = "베타";

    cout << a << endl;
    cout << b << endl;

    std::cout << std::endl;

    std::cout << "--- Attempt 1: Successful call ---" << std::endl;
    try {
        might_fail_function(10);
    }
    catch (const std::runtime_error& e) {
        std::cerr << "Caught an exception: " << e.what() << std::endl;
    }
    std::cout << "Main function continues after successful call." << std::endl;

    std::cout << "\n--- Attempt 2: Failing call ---" << std::endl;
    try {
        might_fail_function(-5); // 여기서 예외가 던져짐
        std::cout << "This line will NOT be printed if exception is thrown." << std::endl;
    }
    catch (const std::runtime_error& e) { // 여기에서 예외를 잡음
        std::cerr << "Caught an exception: " << e.what() << std::endl;
    }
    std::cout << "Main function continues after catching exception." << std::endl;

    return 0;
}