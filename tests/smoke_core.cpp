// Minimal smoke tests for src/ (Fixed, Zoned, Indicator, DS, FigConst, AS400).
// No test framework dependency - build and run directly, e.g.:
//   cl.exe /EHsc /nologo /Isrc /Isrc/file /Iutil /Iparsers ^
//     tests/smoke_core.cpp src/RPGTypes.cpp src/xxcvt.cpp src/xxdtaa.cpp src/as400.cpp
#include <cstdio>
#include <cstring>
#include <cmath>
#include <sstream>
#include <string>

#include "RPGTypes.h"
#include "as400.h"

static int failures = 0;

#define CHECK(cond) \
	do { \
		if (!(cond)) { \
			std::fprintf(stderr, "FAIL: %s (smoke_core.cpp:%d)\n", #cond, __LINE__); \
			++failures; \
		} \
	} while (0)

static void test_fixed()
{
	Fixed<10> a = "hello";
	CHECK(a == "hello");
	CHECK(a != "world");

	// trimr()/subst() return FixedTemp; bind to a named lvalue before
	// constructing a Fixed<> from it (Fixed's FixedTemp ctor takes a
	// non-const reference).
	FixedTemp trimmed = a.trimr();
	Fixed<5> trimmedFixed = trimmed;
	CHECK(trimmedFixed == "hello");

	Fixed<6> c = "abcdef";
	FixedTemp middle = c.subst(2, 3);
	Fixed<3> sub = middle;
	CHECK(sub == "bcd");

	Fixed<5> blank;
	CHECK(blank == BLANKS);
}

static void test_indicator()
{
	Indicator ind;
	CHECK((bool)ind == false);
	ind = true;
	CHECK((bool)ind == true);
	CHECK(!ind == false);
	ind = false;
	CHECK(!ind == true);
}

static void test_zoned()
{
	Zoned<5, 2> num = 123.45;
	CHECK(num.toInt() == 123);
	CHECK(std::fabs((double)num - 123.45) < 0.001);

	Zoned<5, 2> zero = ZEROS;
	CHECK(zero.toInt() == 0);

	Zoned<5, 2> total = 100.25;
	total += 23.50;
	CHECK(std::fabs((double)total - 123.75) < 0.001);
	Zoned<3, 1> other = 1.5;
	total += other;
	CHECK(std::fabs((double)total - 125.25) < 0.001);
	total -= 25.25;
	CHECK(std::fabs((double)total - 100.00) < 0.001);
	total -= other;
	CHECK(std::fabs((double)total - 98.50) < 0.001);
	total *= 2.0;
	CHECK(std::fabs((double)total - 197.00) < 0.001);
	total *= other;
	CHECK(std::fabs((double)total - 295.50) < 0.001);
	total /= 2.0;
	CHECK(std::fabs((double)total - 147.75) < 0.001);
	total /= other;
	CHECK(std::fabs((double)total - 98.50) < 0.001);
}

static void test_zoned_stream()
{
	// Streaming must show every digit -- converting to double instead picks up the
	// stream's default 6-significant-digit precision (60145.76 would print "60145.8").
	std::ostringstream os;
	Zoned<9, 2> total = 60145.76;
	os << total;
	CHECK(os.str() == "60145.76");

	std::ostringstream negOs;
	Zoned<5, 2> neg = -123.45;
	negOs << neg;
	CHECK(negOs.str() == "-123.45");

	std::ostringstream intOs;
	Zoned<4, 0> whole = 1234;
	intOs << whole;
	CHECK(intOs.str() == "1234");

	// Leading zeros suppressed, but one integer digit always kept
	std::ostringstream smallOs;
	Zoned<5, 2> small = 0.25;
	smallOs << small;
	CHECK(smallOs.str() == "0.25");
}

static void test_decimal_literal()
{
	// __D()/__Z() keep the written digits, so no binary-floating-point expansion can
	// leak in -- unlike assigning the double 8.7 to a wide field.
	Zoned<18, 1> wide = __Z("8.7");
	std::ostringstream os;
	os << wide;
	CHECK(os.str() == "8.7");

	Zoned<9, 2> total;
	total = __D("60145.76");
	std::ostringstream totalOs;
	totalOs << total;
	CHECK(totalOs.str() == "60145.76");

	Zoned<7, 2> neg = __D("-12.34");
	std::ostringstream negOs;
	negOs << neg;
	CHECK(negOs.str() == "-12.34");

	// Comparison against a literal is exact, and needs no cast
	Zoned<3, 1> z = __Z("32.1");
	CHECK(z == __Z("32.1"));
	CHECK(z != __Z("32.2"));
	CHECK(z > __Z("32.0"));
	CHECK(z < __Z("32.2"));

	// Past the old 31-digit ceiling, and far past what a double could carry (~15
	// significant digits) -- only exact because the literal never becomes a double.
	Zoned<40, 2> huge = __Z("12345678901234567890123456789012345678.99");
	std::ostringstream hugeOs;
	hugeOs << huge;
	CHECK(hugeOs.str() == "12345678901234567890123456789012345678.99");
	CHECK(MAX_DECIMAL_DIGITS == 63);
	CHECK(sizeof(MAX_DECIMAL_STR) - 1 == MAX_DECIMAL_DIGITS);
}

static void test_zoned_arithmetic()
{
	// Zoned OP Zoned must resolve without ambiguity (it collided with the FixedTemp
	// string-concatenation operator+ before Zoned got its own operators).
	Zoned<9, 2> a = __D("100.25");
	Zoned<7, 2> b = __D("23.50");
	CHECK(std::fabs((double)(a + b) - 123.75) < 0.001);
	CHECK(std::fabs((double)(a - b) - 76.75) < 0.001);
	CHECK(a > b);
	CHECK(b < a);
	CHECK(a != b);
}

static void test_packed()
{
	// packed(n,p) is Packed<n,p> on every platform now, wrapping the native packed
	// decimal on IBM i and Zoned<> elsewhere -- same interface either way.
	packed(9, 2) p = __D("60145.76");
	std::ostringstream os;
	os << p;
	CHECK(os.str() == "60145.76");
	CHECK(p.len() == 9);
	CHECK(p.DigitsOf() == 9);
	CHECK(p.PrecisionOf() == 2);
	CHECK(p.toInt() == 60145);

	packed(5, 2) fromInt = 123;
	CHECK(fromInt.toInt() == 123);
	packed(5, 2) fromDouble = 123.45;
	CHECK(std::fabs((double)fromDouble - 123.45) < 0.001);

	// assign() is the exact path; the double constructor is the convenience one
	packed(7, 2) assigned;
	assigned.assign(__D("-12.34"));
	std::ostringstream negOs;
	negOs << assigned;
	CHECK(negOs.str() == "-12.34");

	// Packed <-> Packed and Packed <-> Zoned comparison and arithmetic
	packed(9, 2) x = __D("100.25");
	packed(7, 2) y = __D("23.50");
	Zoned<7, 2> z = __D("23.50");
	CHECK(x > y);
	CHECK(y == z);
	CHECK(z == y);
	CHECK(x != z);
	CHECK(std::fabs((double)(x + y) - 123.75) < 0.001);
	CHECK(std::fabs((double)(x - z) - 76.75) < 0.001);

	// Figurative constants and round-tripping through the zoned form
	packed(4, 2) hi;
	hi.assign(HIVAL);
	CHECK(hi == HIVAL);
	packed(5, 2) zeros;
	zeros.assign(ZEROS);
	CHECK(zeros.toInt() == 0);

	// Must stay the same size as what it wraps, so it can overlay a record buffer
	CHECK(sizeof(packed(9, 2)) == sizeof(packed(9, 2)::Backing));
}

static void test_ds()
{
	// A 3-occurrence data structure; verify each occurrence keeps its own value.
	DS<4, 3> ds;
	ds.occur(1);
	ds = "aaaa";
	ds.occur(2);
	ds = "bbbb";
	ds.occur(3);
	ds = "cccc";

	ds.occur(1);
	CHECK(ds == "aaaa");
	ds.occur(2);
	CHECK(ds == "bbbb");
	ds.occur(3);
	CHECK(ds == "cccc");
}

static void test_as400()
{
	AS400 conn((char *)"myserver", (char *)"user", (char *)"pass");
	CHECK(std::strcmp(conn.url, "myserver") == 0);
	CHECK(std::strcmp(conn.usrid, "user") == 0);
	CHECK(std::strcmp(conn.password, "pass") == 0);
}

int main()
{
	test_fixed();
	test_indicator();
	test_zoned();
	test_zoned_stream();
	test_decimal_literal();
	test_zoned_arithmetic();
	test_packed();
	test_ds();
	test_as400();

	if (failures == 0)
	{
		std::printf("All core smoke tests passed.\n");
		return 0;
	}
	std::printf("%d core smoke test(s) failed.\n", failures);
	return 1;
}
