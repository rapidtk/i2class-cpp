#include <stdlib.h>
//#include <mem.h>
#include <ctype.h>
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
int QXXZTOI(unsigned char *zptr, int digits, int fraction)
{
	int i=digits-fraction;
	if (i<=0)
   	return 0;
	static char buf[32];
	memcpy(buf, zptr, i);
	buf[i]='\0';
	// A negative zoned value is 0xD0-D9 (}, J-R)
	bool positive=isdigit(zptr[digits-1]);
	if (!positive && fraction==0)
		decodeSign(buf+digits);
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
double QXXZTOD(unsigned char *zptr, int digits, int fraction)
{
	return atof(zonedToChar((const char*)zptr, digits, fraction));
}

/// @brief Convert a double into an in-place zoned-decimal field (rounds/truncates to fit).
/// @param zptr pointer to the first byte of the destination zoned field
/// @param digits total number of digits (precision) in the field
/// @param fraction number of digits to the right of the implied decimal point
/// @param value the value to encode; negative values set the sign nibble via encodeSign()
///
/// See: @see [QXXDTOZ()](https://www.ibm.com/docs/en/i/7.4.0?topic=q-convert-double-zoned-decimal-qxxdtoz)
void QXXDTOZ(unsigned char *zptr, int digits, int fraction, double value)
{
	// Copy in everything to the left of the decimal point
	char	*buf;
	int dec, sign;
	buf=_ecvt(value, digits, &dec, &sign);
	int i=digits-fraction; // number of digits to left of decimal point
	int j=i-dec;
	// Set leading bytes (if any) to 0
	if (j>0)
		memset(zptr, '0', j);
	memcpy(zptr+j, buf, digits-j);
	// Negative values set the last digit high-order nibble to 0xD
	if (value<0)
		encodeSign((char *)zptr+digits-1);
}

/// @brief Convert an integer into an in-place zoned-decimal field.
/// @param zptr pointer to the first byte of the destination zoned field
/// @param digits total number of digits (precision) in the field
/// @param fraction number of digits to the right of the implied decimal point (padded with zeros)
/// @param value the integer value to encode; negative values set the sign nibble via encodeSign()
void QXXITOZ(unsigned char *zptr, int digits, int fraction, int value)
{
	// Copy in everything to the left of the decimal point
	static char	buf[32];
	_itoa(value, (char*)buf, 10);
	int i=digits-fraction; // number of digits to left of decimal point
	int j=i-strlen(buf);
	// Set leading bytes (if any) to 0
	if (j>0)
		memset(zptr, '0', j);
	memcpy(zptr+j, buf, i-j);
	// Set trailing decimal (if any) to 0
	if (fraction>0)
		memset(zptr+i, '0', fraction);
	// Negative values set the last digit high-order nibble to 0xD
	if (value<0)
		encodeSign((char *)zptr+digits-1);
}

