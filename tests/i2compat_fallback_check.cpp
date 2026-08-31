// Verifies src/compat/stdlib.cpp's portable _itoa_s/_ecvt_s fallback (the non-MSVC branch),
// which this MSVC-only environment can't otherwise reach. #undef'ing _MSC_VER forces that
// branch to compile here so it gets exercised even without a second real compiler available.
// The names are aliased before including stdlib.cpp so its own definitions don't collide
// with MSVC's real ucrt.lib _itoa_s/_ecvt_s at link time -- a conflict that can only happen
// in this self-test (a real non-MSVC platform has no ucrt.lib to collide with). The system
// headers are pulled in *before* the alias so their real (unaliased) declarations are already
// past their include guards by the time stdlib.cpp re-includes them.
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#undef _MSC_VER
#define _itoa_s test_itoa_s
#define _ecvt_s test_ecvt_s
#include "../src/compat/stdlib.cpp"
#undef _itoa_s
#undef _ecvt_s

#include <cstdio>
#include <cstring>

static int failures = 0;

static void checkItoa(int value, const char *expected)
{
	char buf[64];
	int rc = test_itoa_s(value, buf, sizeof(buf), 10);
	if (rc != 0 || std::strcmp(buf, expected) != 0)
	{
		std::printf("FAIL: _itoa_s(%d) -> rc=%d buf=\"%s\" (expected \"%s\")\n", value, rc, buf, expected);
		++failures;
	}
	else
		std::printf("PASS: _itoa_s(%d) -> \"%s\"\n", value, buf);
}

static void checkEcvt(double value, int count, const char *expectedDigits, int expectedDec, int expectedSign)
{
	char buf[64];
	int dec = 0, sign = 0;
	int rc = test_ecvt_s(buf, sizeof(buf), value, count, &dec, &sign);
	if (rc != 0 || std::strcmp(buf, expectedDigits) != 0 || dec != expectedDec || sign != expectedSign)
	{
		std::printf("FAIL: _ecvt_s(%f, %d) -> rc=%d digits=\"%s\" dec=%d sign=%d (expected \"%s\" dec=%d sign=%d)\n",
			value, count, rc, buf, dec, sign, expectedDigits, expectedDec, expectedSign);
		++failures;
	}
	else
		std::printf("PASS: _ecvt_s(%f, %d) -> \"%s\" dec=%d sign=%d\n", value, count, buf, dec, sign);
}

int main()
{
	checkItoa(123, "123");
	checkItoa(-42, "-42");
	checkItoa(0, "0");

	checkEcvt(123.45, 7, "1234500", 3, 0);
	checkEcvt(-0.0123, 3, "123", -1, 1);
	checkEcvt(0.0, 5, "00000", 0, 0);
	checkEcvt(12.345, 7, "1234500", 2, 0);

	std::printf(failures == 0 ? "\nAll fallback checks passed.\n" : "\n%d fallback check(s) failed.\n", failures);
	return failures == 0 ? 0 : 1;
}
