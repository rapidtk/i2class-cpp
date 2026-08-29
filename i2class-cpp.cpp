// i2class-cpp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include "i2class.h"

int main()
{
    Fixed<10> fxd10;
    fxd10 = "abcdef";
	std::cout << "char(10): " << fxd10.c_str() << '\n' ;
	Fixed<3> fxd3 = "abc";
	if (fxd10 != fxd3)
	   std::cout << "char(10) 'abcdef' not equal to char(3) 'abc' \n";
	if (fxd3 == "abc")
	   std::cout << "char(3) 'abc' equal to 'abc'\n";
	fxd3 = ZEROS;
	if (fxd3 == "000")
	   std::cout << "char(3) '000' equal to *ZEROS\n";

	Zoned<3, 1> znd31 = 12.1;
	std::cout << "zoned(3,1): " << (double)znd31 << '\n';
	znd31.movel('3');
	if (znd31 == 32.1)
	   std::cout << "zoned(3,1) 32.1 equal to 32.1\n";

	packed(4, 2) pkd42;
	pkd42.assign(znd31);
	if (pkd42 == znd31)
	   std::cout << "packed(4,2) 32.10 equal to zoned(3,1) 32.1\n";


	znd31 = LOVAL;
	if (znd31 == -99.9)
	   std::cout << "zoned(3,1) -99.9 equal to *LOVAL\n";
	pkd42 = 99.99;
	if (pkd42 == HIVAL)
	   std::cout << "packed(4,2) 99.99 equal to *HIVAL\n";

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
