#include "../i2compat.h"

// Portable (non-MSVC) fallback implementations of the Microsoft Secure CRT
// functions this codebase calls (_itoa_s, _ecvt_s) -- see i2compat.h for the
// declarations and why this exists. Add any future compatibility functions
// here, following the same "declare in i2compat.h, define here, guard on the
// compiler/platform that's missing the native version" pattern.
#if !defined(_MSC_VER)

#include <cstdio>
#include <cstdlib>
#include <cstring>

int _itoa_s(int value, char *buffer, std::size_t sizeInCharacters, int radix)
{
	if (buffer == nullptr || radix != 10) // only base 10 is used anywhere in this codebase
		return 22; // EINVAL
	int n = std::snprintf(buffer, sizeInCharacters, "%d", value);
	if (n < 0 || static_cast<std::size_t>(n) >= sizeInCharacters)
		return 34; // ERANGE
	return 0;
}

int _ecvt_s(char *buffer, std::size_t sizeInBytes, double value, int count, int *dec, int *sign)
{
	if (buffer == nullptr || dec == nullptr || sign == nullptr || count < 0
		|| sizeInBytes < static_cast<std::size_t>(count) + 1)
		return 22; // EINVAL

	*sign = (value < 0) ? 1 : 0;
	double magnitude = (value < 0) ? -value : value;

	if (magnitude == 0.0)
	{
		std::memset(buffer, '0', static_cast<std::size_t>(count));
		buffer[count] = '\0';
		*dec = 0;
		return 0;
	}

	// Scientific notation gives exactly `count` significant digits directly:
	// "d.ddd...e+NN" has 1 + (count-1) = count digits before the 'e'.
	char sci[64];
	std::snprintf(sci, sizeof(sci), "%.*e", (count > 0 ? count - 1 : 0), magnitude);

	std::size_t n = 0;
	for (const char *p = sci; *p != '\0' && *p != 'e' && *p != 'E' && n < static_cast<std::size_t>(count); ++p)
	{
		if (*p != '.')
			buffer[n++] = *p;
	}
	buffer[n] = '\0';

	const char *expPtr = std::strchr(sci, 'e');
	*dec = (expPtr != nullptr ? std::atoi(expPtr + 1) : 0) + 1;
	return 0;
}

#endif // !_MSC_VER
