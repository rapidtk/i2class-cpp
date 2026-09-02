#ifndef I2_COMPAT_STRING_CONTRACTS_H
#define I2_COMPAT_STRING_CONTRACTS_H

#include <cstddef>
#include <cstring>

// Plain aliases for "char *"/"const char *" that are documented, null-terminated
// C strings, as opposed to a raw fixed-length byte buffer (like Fixed<sz>::overlay)
// that may not be null-terminated at all. No namespace, no behavior change --
// purely so call sites can tell the two usages apart at a glance.
typedef char *zstring;
typedef const char *czstring;
typedef unsigned char *byte_ptr;
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
