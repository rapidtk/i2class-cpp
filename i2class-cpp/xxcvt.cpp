#include <stdlib.h>
#include <mem.h>
#include <ctype.h>
#include <string.h>
#include "rpgtypes.h"

// Convert zoned to integer
int QXXZTOI(unsigned char *zptr, int digits, int fraction)
{
	int i=digits-fraction;
	if (i<=0)
   	return 0;
	static unsigned char buf[32];
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
// Convert zoned to double
double QXXZTOD(unsigned char *zptr, int digits, int fraction)
{
	return atof(zonedToChar(zptr, digits, fraction));
}

// Convert double to zoned
void QXXDTOZ(unsigned char *zptr, int digits, int fraction, double value)
{
	// Copy in everything to the left of the decimal point
	char	*buf;
	int dec, sign;
	buf=ecvt(value, digits, &dec, &sign);
	int i=digits-fraction; // number of digits to left of decimal point
	int j=i-dec;
	// Set leading bytes (if any) to 0
	if (j>0)
		memset(zptr, '0', j);
	memcpy(zptr+j, buf, digits-j);
	// Negative values set the last digit high-order nibble to 0xD
	if (value<0)
		encodeSign(zptr+digits-1);
}

// Convert integer to zoned
void QXXITOZ(unsigned char *zptr, int digits, int fraction, int value)
{
	// Copy in everything to the left of the decimal point
	static unsigned char	buf[32];
	itoa(value, buf, 10);
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
		encodeSign(zptr+digits-1);
}

