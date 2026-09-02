#include <stdlib.h>
//#include <mem.h>
#include <ctype.h>
#include <float.h>
#include <string.h>
#include "rpgtypes.h"

/// @file xxcvt.cpp
/// @brief Conversions between IBM i "zoned decimal" storage and native C++ numeric types.
///
/// Zoned decimal is the storage format RPG uses for numeric fields declared with
/// `S` (zoned) in the DDS/DSPEC: one byte per digit, each byte holding the digit
/// value in its low nibble (0x30-0x39 in ASCII / 0xF0-0xF9 in EBCDIC) and, for the
/// *last* digit only, the sign encoded in the high nibble instead of the usual
/// digit-family nibble (see decodeSign()/encodeSign() in RPGTypes.cpp). These
/// QXX* functions mirror the IBM i system APIs of the same name so that ported
/// RPG numeric assignments/conversions behave identically on Windows.
///
/// See: [Zoned-decimal format](https://www.ibm.com/docs/en/i/7.4.0?topic=type-zoned-decimal-format#zodecfo),

/// @brief Convert a zoned-decimal field to an integer (truncates any fractional digits).
/// @param zptr pointer to the first byte of the zoned field
/// @param digits total number of digits (precision) in the field
/// @param fraction number of digits to the right of the implied decimal point
///
/// See: @see [QXXITOZ()](https://www.ibm.com/docs/en/i/7.4.0?topic=q-convert-integer-zoned-decimal-qxxitoz)
int QXXZTOI(byte_ptr zptr, int digits, int fraction)
{
	int i=digits-fraction;
	if (i<=0)
   	return 0;
	char buf[MAX_DECIMAL_DIGITS+1];
	memcpy(buf, zptr, i);
	buf[i]='\0';
	// A negative zoned value is 0xD0-D9 (}, J-R)
	bool positive=isdigit(zptr[digits-1]);
	if (!positive && fraction==0)
		decodeSign(reinterpret_cast<byte_ptr>(buf+digits-1)); // last digit byte, not the null terminator at buf[digits]
	i=atoi(buf);
	if (!positive)
		i = i*-1;
	return i;
}
/// @brief Convert a zoned-decimal field to a double, preserving the implied decimal point/sign.
/// @param zptr pointer to the first byte of the zoned field
/// @param digits total number of digits (precision) in the field
/// @param fraction number of digits to the right of the implied decimal point
///
/// See: @see [QXXZTOD()](https://www.ibm.com/docs/en/i/7.4.0?topic=q-convert-zoned-decimal-double-qxxztod)
double QXXZTOD(byte_ptr zptr, int digits, int fraction)
{
	return atof(zonedToChar(zptr, digits, fraction));
}

/// @brief Convert a double into an in-place zoned-decimal field (rounds/truncates to fit).
/// @param zptr pointer to the first byte of the destination zoned field
/// @param digits total number of digits (precision) in the field
/// @param fraction number of digits to the right of the implied decimal point
/// @param value the value to encode; negative values set the sign nibble via encodeSign()
///
/// See: @see [QXXDTOZ()](https://www.ibm.com/docs/en/i/7.4.0?topic=q-convert-double-zoned-decimal-qxxdtoz)
void QXXDTOZ(byte_ptr zptr, int digits, int fraction, double value)
{
	// Copy in everything to the left of the decimal point
	char buf[MAX_DECIMAL_DIGITS+1];
	int dec, sign;
	// Only ask for as many significant digits as a double can actually carry (DBL_DIG).
	// Asking for the field's full width re-exposes the binary representation's noise --
	// 8.7 at 18 significant digits is 8.69999999999999929, and the truncating copy below
	// would then store 8.6 rather than 8.7.
	int sigDigits = digits<DBL_DIG ? digits : DBL_DIG;
	_ecvt_s(buf, sizeof(buf), value, sigDigits, &dec, &sign);
	// Pad so the copy below can still take up to `digits` characters
	if (digits>sigDigits)
		memset(buf+sigDigits, '0', digits-sigDigits);
	int i=digits-fraction; // number of digits to left of decimal point
	int j=i-dec;
	if (j<0)
		j=0; // integer part wider than the field -- truncate rather than write before zptr[0]
	// Set leading bytes (if any) to 0
	if (j>0)
		memset(zptr, '0', j);
	memcpy(zptr+j, buf, digits-j);
	// Negative values set the last digit high-order nibble to 0xD
	if (value<0)
		encodeSign(zptr+digits-1);
}

/// @brief Convert an integer into an in-place zoned-decimal field.
/// @param zptr pointer to the first byte of the destination zoned field
/// @param digits total number of digits (precision) in the field
/// @param fraction number of digits to the right of the implied decimal point (padded with zeros)
/// @param value the integer value to encode; negative values set the sign nibble via encodeSign()
void QXXITOZ(byte_ptr zptr, int digits, int fraction, int value)
{
	// Copy in everything to the left of the decimal point
	char	buf[MAX_DECIMAL_DIGITS+1];
	_itoa_s(value, buf, sizeof(buf), 10);
	// Skip the leading '-' (if any) -- it's not a digit, and encodeSign() below
	// records the sign separately.
	const char *digitsStr = (value < 0) ? buf+1 : buf;
	int i=digits-fraction; // number of digits to left of decimal point
	int j=i-static_cast<int>(strlen(digitsStr));
	// Set leading bytes (if any) to 0
	if (j>0)
		memset(zptr, '0', j);
	memcpy(zptr+j, digitsStr, i-j);
	// Set trailing decimal (if any) to 0
	if (fraction>0)
		memset(zptr+i, '0', fraction);
	// Negative values set the last digit high-order nibble to 0xD
	if (value<0)
		encodeSign(zptr+digits-1);
}

