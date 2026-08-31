// i2class-cpp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

/// @file i2class-cpp.cpp
/// @brief Demo/manual smoke test exercising the core RPG-compatible data types.
///
/// Illustrates the IBM i/RPG data type wrappers this library provides:
///  - Fixed<N>: a fixed-length character field (like RPG CHAR(N)); comparisons
///    are blank-padded, matching RPG semantics rather than C-string semantics.
///  - Zoned<P,F>: a zoned-decimal numeric field with P total digits and F
///    digits after the decimal point (see xxcvt.cpp for the on-disk format).
///  - packed(P,F): a packed (BCD) decimal field - same numeric semantics as
///    Zoned but stored two digits per byte, as IBM i COBOL/RPG PACKED fields are.
///  - Figurative constants ZEROS/BLANKS/LOVAL/HIVAL mirror RPG's *ZEROS,
///    *BLANKS, *LOVAL, and *HIVAL special values.

#include <iostream>

#include "RPGTypes.h"

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
	Zoned<5, 2> znd52 = 123.45;
	double diff = (double)znd52 - 123.45;
	if (diff == 0.0)
	   std::cout <<	"zoned(5,2) 123.45 - 123.45 equal to 0.0\n";

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