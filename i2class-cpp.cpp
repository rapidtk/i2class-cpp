// i2class-cpp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include "RPGTypes.h"

int main()
{
    Fixed<10> str = "abcdef";
    Fixed<3> str2 = "ghi";
    Zoned<3, 1> znd1 = 12.1;
    if (str2 == "ghi")
        std::cout << "equal to ghi";

    std::cout << str.c_str() << str2.c_str() << znd1;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
