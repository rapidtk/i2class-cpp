#ifndef I2COMPAT_H
#define I2COMPAT_H

#if __cplusplus >= 202002L
# define I2_OVERRIDE override
#else
  // No override support
# define I2_OVERRIDE
#endif

// Built-in packed decimal support
#ifndef __OS400__
# define NO_PACKED
#endif

////////////////////////////////////////////////////////////////////////////////
// Export macro for the handful of extern global objects (BLANKS, ZEROS, LOVAL,
// HIVAL, TIME, ...) that a Windows DLL doesn't export automatically -- MSVC's
// WINDOWS_EXPORT_ALL_SYMBOLS (used by CMakeLists.txt) only auto-exports functions,
// not data. I2CLASS_BUILDING_DLL is defined by CMakeLists.txt only for the i2class
// target itself, so consumers of the header automatically get the dllimport side.
// No-op everywhere else (non-Windows, or linking the sources directly like the
// original .vcxproj does), so this changes nothing for existing non-DLL builds.
#if defined(_WIN32) && defined(I2CLASS_BUILDING_DLL)
# define I2CLASS_API __declspec(dllexport)
#elif defined(_WIN32) && defined(I2CLASS_USING_DLL)
# define I2CLASS_API __declspec(dllimport)
#else
# define I2CLASS_API
#endif

////////////////////////////////////////////////////////////////////////////////
// Portable fallbacks for the Microsoft Secure CRT functions this codebase calls
// (_itoa_s, _ecvt_s). MSVC already provides real, native implementations of both
// via <stdlib.h>; every other standard C++11+ compiler gets these instead (see
// src/compat/stdlib.cpp for the implementations), so call sites can use
// _itoa_s()/_ecvt_s() uniformly everywhere without #ifdef'ing each call.
#if !defined(_MSC_VER)

#include <cstddef>

int _itoa_s(int value, char *buffer, std::size_t sizeInCharacters, int radix);

// Portable equivalent of the classic ecvt()/_ecvt_s() contract: writes exactly
// `count` significant decimal digits (no sign, no decimal point) to buffer;
// *dec = position of the decimal point relative to the first digit (0 for a
// zero value, negative if the value has leading zeros before its first
// significant digit); *sign = 1 if value is negative, else 0.
int _ecvt_s(char *buffer, std::size_t sizeInBytes, double value, int count, int *dec, int *sign);

#endif // !_MSC_VER

#endif
