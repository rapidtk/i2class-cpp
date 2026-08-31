// Regression test proving I2_RUNTIME_ENABLE_BOUNDS_CHECK is actually wired up in Debug
// builds (see CMakeLists.txt) -- this macro previously existed in RPGTypes.h but was never
// defined anywhere, so out-of-range Fixed<>/DS<> access silently did nothing.
#include <cstdio>

#include "RPGTypes.h"

static int failures = 0;

#define CHECK(cond) \
	do { \
		if (!(cond)) { \
			std::fprintf(stderr, "FAIL: %s (bounds_check.cpp:%d)\n", #cond, __LINE__); \
			++failures; \
		} \
	} while (0)

int main()
{
#if I2_RUNTIME_ENABLE_BOUNDS_CHECK
	Fixed<5> f = "hello";
	bool threw = false;
	try
	{
		char &c = f[10]; // out of range
		(void)c;
	}
	catch (const CI2ErrSubscript &)
	{
		threw = true;
	}
	CHECK(threw);
	std::printf("I2_RUNTIME_ENABLE_BOUNDS_CHECK is ON: out-of-range access threw as expected.\n");
#else
	std::printf("I2_RUNTIME_ENABLE_BOUNDS_CHECK is OFF in this build config -- nothing to check.\n");
#endif

	if (failures == 0)
	{
		std::printf("All bounds-check tests passed.\n");
		return 0;
	}
	std::printf("%d bounds-check test(s) failed.\n", failures);
	return 1;
}
