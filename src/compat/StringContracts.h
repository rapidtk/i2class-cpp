#ifndef I2_COMPAT_STRING_CONTRACTS_H
#define I2_COMPAT_STRING_CONTRACTS_H

#include <cstddef>
#include <cstring>

// Plain aliases for "char *"/"const char *" that are documented, null-terminated
// C strings, as opposed to a raw fixed-length byte buffer (like Fixed<sz>::overlay)
// that may not be null-terminated at all. 

/// @brief Pointer to null-terminated string (aka "zero-terminated string") data
///
/// See: gsl::czstring @link https://github.com/microsoft/gsl @endlink
typedef char *zstring;
/// @brief Pointer to constant null-terminated string (aka "zero-terminated string") data
///
/// See: gsl::czstring @link https://github.com/microsoft/gsl @endlink
typedef const char *czstring;

// Plain aliases for "unsigned char *"/"const unsigned char *" i.e. raw fixed-length byte buffers

/// @brief Pointer to data of an unknown type
///
/// See: std::byte @link https://en.cppreference.com/cpp/types/byte @endlink
typedef unsigned char *byte_ptr;
/// @brief Pointer to constant data of an unknown type
///
/// See: std::byte @link https://en.cppreference.com/cpp/types/byte @endlink
typedef const unsigned char *const_byte_ptr;

inline czstring nonnull_or_empty(czstring value)
{
	return (value == nullptr) ? "" : value;
}

inline std::size_t zstring_length(czstring value)
{
	return (value == nullptr) ? static_cast<std::size_t>(0) : std::strlen(value);
}

#endif
