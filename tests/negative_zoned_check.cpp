// Regression test for negative zoned-decimal encode/decode (QXXITOZ/QXXDTOZ/QXXZTOI).
// Added because tests/smoke_core.cpp's Zoned<> case only covers a positive value with
// fraction > 0, which doesn't exercise the negative-sign decode path (fraction == 0
// negative values previously silently dropped their last digit -- see decodeSign() call
// site in xxcvt.cpp's QXXZTOI).
#include <cstdio>
#include <cstring>
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

	// toInt() goes through QXXZTOI, but every arithmetic/printing path goes
	// through zonedToChar()/QXXZTOD instead. With fraction == 0 that used to
	// decode the sign nibble off the '.' rather than the last digit, so any
	// negative whole-number Zoned read back as 0.
	Zoned<5,0> e(int(-5));
	if ((double)e != -5.0) { std::printf("FAIL: Zoned<5,0> int(-5) -> %f (QXXZTOD)\n", (double)e); ++failures; }
	else std::printf("PASS: Zoned<5,0> int(-5) round-trip via QXXZTOD\n");

	if (std::strcmp(zonedToChar((const_byte_ptr)e.overlay, 5, 0), "-00005.") != 0) {
		std::printf("FAIL: zonedToChar(Zoned<5,0>(-5)) -> %s\n",
			zonedToChar((const_byte_ptr)e.overlay, 5, 0));
		++failures;
	}
	else std::printf("PASS: zonedToChar renders fraction==0 negative correctly\n");

	// Signed arithmetic on a fraction == 0 operand.
	Zoned<11,2> f = Zoned<9,2>(-320.75) * Zoned<5,0>(int(-5));
	if ((double)f != 1603.75) { std::printf("FAIL: -320.75 * -5 -> %f\n", (double)f); ++failures; }
	else std::printf("PASS: negative * negative (fraction==0 operand) -> 1603.75\n");

	Zoned<7,3> d = -12.345;
	if ((double)d != -12.345) { std::printf("FAIL: Zoned<7,3> -12.345 -> %f\n", (double)d); ++failures; }
	else std::printf("PASS: Zoned<7,3> -12.345 round-trip via QXXDTOZ\n");

	Zoned<5,2> half = 1.50;
	if (half.inth() != 2) { std::printf("FAIL: Zoned inth(1.50) -> %d\n", half.inth()); ++failures; }
	else std::printf("PASS: Zoned inth(1.50) half-adjusts to 2\n");
	if (Zoned<5,2>(-1.50).inth() != -2) { std::printf("FAIL: Zoned inth(-1.50)\n"); ++failures; }
	else std::printf("PASS: Zoned inth(-1.50) half-adjusts away from zero\n");
	Zoned<5,1> scaled = Zoned<5,2>(12.35).dech<5,1>();
	if ((double)scaled != 12.4) { std::printf("FAIL: Zoned dech(12.35 -> 1 decimal) -> %f\n", (double)scaled); ++failures; }
	else std::printf("PASS: Zoned dech(12.35) -> 12.4\n");
	Zoned<5,1> roundedScale = Zoned<5,2>(12.35).round<1>();
	if ((double)roundedScale != 12.4) { std::printf("FAIL: Zoned round<1>(12.35) -> %f\n", (double)roundedScale); ++failures; }
	else std::printf("PASS: Zoned round<1>(12.35) -> 12.4\n");
	Zoned<5,0> roundedWhole = Zoned<5,2>(12.50).round<0>();
	if (roundedWhole.toInt() != 13) { std::printf("FAIL: Zoned round<0>(12.50) -> %d\n", roundedWhole.toInt()); ++failures; }
	else std::printf("PASS: Zoned round<0>(12.50) -> 13\n");
	Packed<5,2> packedHalf = 1.50;
	if (packedHalf.inth() != 2) { std::printf("FAIL: Packed inth(1.50) -> %d\n", packedHalf.inth()); ++failures; }
	else std::printf("PASS: Packed inth(1.50) half-adjusts to 2\n");
	Packed<5,1> packedScale = Packed<5,2>(12.35).round<1>();
	if ((double)packedScale != 12.4) { std::printf("FAIL: Packed round<1>(12.35) -> %f\n", (double)packedScale); ++failures; }
	else std::printf("PASS: Packed round<1>(12.35) -> 12.4\n");

	std::printf(failures == 0 ? "\nAll checks passed.\n" : "\n%d check(s) failed.\n", failures);
	return failures == 0 ? 0 : 1;
}
