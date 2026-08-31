// Regression test for negative zoned-decimal encode/decode (QXXITOZ/QXXDTOZ/QXXZTOI).
// Added because tests/smoke_core.cpp's Zoned<> case only covers a positive value with
// fraction > 0, which doesn't exercise the negative-sign decode path (fraction == 0
// negative values previously silently dropped their last digit -- see decodeSign() call
// site in xxcvt.cpp's QXXZTOI).
#include <cstdio>
#include "RPGTypes.h"

int main()
{
	int failures = 0;

	Zoned<5,2> a = -42.00;
	if ((double)a != -42.00) { std::printf("FAIL: Zoned<5,2> -42.00 -> %f\n", (double)a); ++failures; }
	else std::printf("PASS: Zoned<5,2> -42.00 round-trip via QXXDTOZ\n");

	Zoned<5,2> b(int(-7));
	if (b.toInt() != -7) { std::printf("FAIL: Zoned<5,2> int(-7) -> %d\n", b.toInt()); ++failures; }
	else std::printf("PASS: Zoned<5,2> int(-7) round-trip via QXXITOZ\n");

	Zoned<9,0> c(int(-123456789));
	if (c.toInt() != -123456789) { std::printf("FAIL: Zoned<9,0> int(-123456789) -> %d\n", c.toInt()); ++failures; }
	else std::printf("PASS: Zoned<9,0> int(-123456789) round-trip via QXXITOZ\n");

	Zoned<7,3> d = -12.345;
	if ((double)d != -12.345) { std::printf("FAIL: Zoned<7,3> -12.345 -> %f\n", (double)d); ++failures; }
	else std::printf("PASS: Zoned<7,3> -12.345 round-trip via QXXDTOZ\n");

	std::printf(failures == 0 ? "\nAll checks passed.\n" : "\n%d check(s) failed.\n", failures);
	return failures == 0 ? 0 : 1;
}
