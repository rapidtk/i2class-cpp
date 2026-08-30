// Minimal smoke tests for src/ (Fixed, Zoned, Indicator, DS, FigConst, AS400).
// No test framework dependency - build and run directly, e.g.:
//   cl.exe /EHsc /nologo /Isrc /Isrc/file /Iutil /Iparsers ^
//     tests/smoke_core.cpp src/RPGTypes.cpp src/xxcvt.cpp src/xxdtaa.cpp src/as400.cpp
#include <cstdio>
#include <cstring>
#include <cmath>

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
