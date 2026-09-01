#include "i2compat.h"
#include "rpgtypes.h"
#include "rglobal.h"

#ifdef __OS400__
#include "recio.h"
#include "xxfdbk.h"
#endif


////////////////////////////////////////////////////////////////////////////////
// Figurative constant declaration
#define FIGCONSTNUM_COMPARE(OP) \
 bool operator OP (short val1, const FigConstNum &val2) \
   { return val1 OP val2.operator short(); } \
 bool operator OP (int val1, const FigConstNum &val2) \
   { return val1 OP val2.operator int(); } \
 bool operator OP (long double val1, const FigConstNum &val2) \
   { return val1 OP val2.operator long double(); }
FIGCONSTNUM_COMPARE(==)
FIGCONSTNUM_COMPARE(!=)
FIGCONSTNUM_COMPARE(<=)
FIGCONSTNUM_COMPARE(<)
FIGCONSTNUM_COMPARE(>=)
FIGCONSTNUM_COMPARE(>)

////////////////////////////////////////////////////////////////////////////////
// StringTemp functions
inline StringTemp::StringTemp(const FixedTemp &fStr1, const FixedTemp &fStr2)
{
   int len1=fStr1.len();
   int len2=fStr2.len();
   sz=len1+len2;
   overlay = new char[sz];
   memcpy(overlay, fStr1.overlay, len1);
   memcpy(overlay+len1, fStr2.overlay, len2);
}
// Copy constructor so that pointer member (overlay) doesn't get deleted
inline StringTemp::StringTemp(const StringTemp &sStr)
{
   sz=sStr.len();
   overlay=new char[sz];
   memcpy(overlay, sStr.overlay, sz);
}

// This allows assignment of A="B" + C as well as A=B + "C" (above)
StringTemp operator + (const FixedTemp &fStr1, const FixedTemp &fStr2)
{
	return StringTemp(fStr1, fStr2);
}
StringTemp operator + (const char *str, const FixedTemp &fStr2)
{
	FixedTemp fStr1((char *)str, static_cast<int>(strlen(str)));
	return fStr1 + fStr2;
}
StringTemp operator + (const FixedTemp &fStr1, const char *str)
{
	FixedTemp fStr2((char *)str, static_cast<int>(strlen(str)));
	return fStr1 + fStr2;
}

////////////////////////////////////////////////////////////////////////////////
/// [Figurative constant](https://www.ibm.com/docs/en/i/7.4.0?topic=constants-figurative#spfig) declarations

FigConst	BLANKS(' ', 0);
Zeros ZEROS;
Loval LOVAL;
Hival HIVAL;

////////////////////////////////////////////////////////////////////////////////
// 'Special' date (U/*DATE, U/*YEAR, U/*MONTH, U/*DAY declarations
/// RPG's U/DATE (job/system date) is exposed as several overlapping numeric
/// "views" of the same 8-byte YYYYMMDD storage: YEAR/MONTH/DAY (4/2/2 digits)
/// and their 2-digit UYEAR/UMONTH/UDAY counterparts. These are references
/// into YYMD.overlay so all views always stay in sync with each other.
FmtDate<8>	YYMD("%Y%m%d");
	Zoned<4,0>	&YEAR=(Zoned<4,0> &)YYMD.overlay[0];
		Zoned<2,0>	&UYEAR=(Zoned<2,0> &)YEAR.overlay[2];
	Zoned<2,0>	&MONTH=(Zoned<2,0> &)YYMD.overlay[4];
		Zoned<2,0>	&UMONTH=MONTH;
	Zoned<2,0>	&DAY=(Zoned<2,0> &)YYMD.overlay[6];
		Zoned<2,0>	&UDAY=DAY;

FmtDate<6>	XUDATE(_DATFMT);
FmtDate<8>	XDATE(_DATFMT);

////////////////////////////////////////////////////////////////////////////////
// Global Indicator space
/// RPG's 99 numbered indicators (*IN01-*IN99), a global array of boolean flags
/// traditionally used to signal conditions (end-of-file, record found, etc.)
/// between a program's calculation and output specs. IN01..IN99 are named
/// references into the xIN[] array; IN is a Fixed<99> view of the whole array
/// (so it can be read/written as a single 99-character '0'/'1' string, as RPG
/// programs sometimes do via *INxx or the LR/KA-KY/L1-L9/OA-OV aliases below).
IND	xIN[99];
	IND &IN01=xIN[0],&IN02=xIN[1],&IN03=xIN[2],&IN04=xIN[3],&IN05=xIN[4];
	IND &IN06=xIN[5],&IN07=xIN[6],&IN08=xIN[7],&IN09=xIN[8],&IN10=xIN[9];
	IND &IN11=xIN[10],&IN12=xIN[11],&IN13=xIN[12],&IN14=xIN[13],&IN15=xIN[14];
	IND &IN16=xIN[15],&IN17=xIN[16],&IN18=xIN[17],&IN19=xIN[18],&IN20=xIN[19];
	IND &IN21=xIN[20],&IN22=xIN[21],&IN23=xIN[22],&IN24=xIN[23],&IN25=xIN[24];
	IND &IN26=xIN[25],&IN27=xIN[26],&IN28=xIN[27],&IN29=xIN[28],&IN30=xIN[29];
	IND &IN31=xIN[30],&IN32=xIN[31],&IN33=xIN[32],&IN34=xIN[33],&IN35=xIN[34];
	IND &IN36=xIN[35],&IN37=xIN[36],&IN38=xIN[37],&IN39=xIN[38],&IN40=xIN[39];
	IND &IN41=xIN[40],&IN42=xIN[41],&IN43=xIN[42],&IN44=xIN[43],&IN45=xIN[44];
	IND &IN46=xIN[45],&IN47=xIN[46],&IN48=xIN[47],&IN49=xIN[48],&IN50=xIN[49];
	IND &IN51=xIN[50],&IN52=xIN[51],&IN53=xIN[52],&IN54=xIN[53],&IN55=xIN[54];
	IND &IN56=xIN[55],&IN57=xIN[56],&IN58=xIN[57],&IN59=xIN[58],&IN60=xIN[59];
	IND &IN61=xIN[60],&IN62=xIN[61],&IN63=xIN[62],&IN64=xIN[63],&IN65=xIN[64];
	IND &IN66=xIN[65],&IN67=xIN[66],&IN68=xIN[67],&IN69=xIN[68],&IN70=xIN[69];
	IND &IN71=xIN[70],&IN72=xIN[71],&IN73=xIN[72],&IN74=xIN[73],&IN75=xIN[74];
	IND &IN76=xIN[75],&IN77=xIN[76],&IN78=xIN[77],&IN79=xIN[78],&IN80=xIN[79];
	IND &IN81=xIN[80],&IN82=xIN[81],&IN83=xIN[82],&IN84=xIN[83],&IN85=xIN[84];
	IND &IN86=xIN[85],&IN87=xIN[86],&IN88=xIN[87],&IN89=xIN[88],&IN90=xIN[89];
	IND &IN91=xIN[90],&IN92=xIN[91],&IN93=xIN[92],&IN94=xIN[93],&IN95=xIN[94];
	IND &IN96=xIN[95],&IN97=xIN[96],&IN98=xIN[97],&IN99=xIN[98];
Fixed<99>	&IN=(Fixed<99> &)xIN[0];

IND INKA,INKB,INKC,INKD,INKE,INKF,INKG,INKH,INKI,INKJ,INKK,INKL,INKM,INKN;
IND INKP,INKQ,INKR,INKS,INKT,INKU,INKV,INKW,INKX,INKY;

IND INLR, INL1, INL2, INL3, INL4, INL5, INL6, INL7, INL8, INL9;
IND INOA, INOB, INOC, INOD, INOE, INOF, INOG, INOV;

/// @brief On native IBM i, bind a display/printer file's INDARA (separate
/// indicator area) to the global indicator array so RPG-style *INxx settings
/// made by the program are reflected on the next I/O operation for this file.
/// Only meaningful for files opened with a separate indicator area; a no-op
/// (and not compiled) elsewhere since indicator areas are an IBM i I/O concept.
#ifdef __OS400__
void setIndara(_RFILE *fp)
{
	_XXOPFB_T *opfb=_Ropnfbk(fp);
	if (opfb->sep_indic_area_flag > 0)
		_Rindara(fp, (char *)xIN);
}
#endif

////////////////////////////////////////////////////////////////////////////////
// Global time functions used by FMTDATE, U/TIME, U/TIMESTAMP etc.
int time6()
{
	FmtDate<6> ltime("%H%M%S");
	return ltime.toInt();
}
long double time12()
{
	FmtDate<6> ltime("%H%M%S");
	FmtDate<6> ldate(_DATFMT);
	Zoned<12, 0> ltime12;
	memcpy(ltime12.overlay, ltime.overlay, 6);
	memcpy(ltime12.overlay+6, ldate.overlay, 6);
	return ltime12;
}
long double time14()
{
	FmtDate<6> ltime("%H%M%S");
	FmtDate<8> ldate(_DATFMT);
	Zoned<14, 0> ltime14;
	memcpy(ltime14.overlay, ltime.overlay, 6);
	memcpy(ltime14.overlay+6, ldate.overlay, 8);
	return ltime14;
}
Time6 TIME;
Time12 UTIMESTAMP;
Time14 TIMESTAMP;

////////////////////////////////////////////////////////////////////////////////
// Zoned support functions

/// @brief Decode the sign nibble of a zoned decimal's last digit into a plain ASCII digit.
///
/// A zoned-decimal field stores one digit per byte, but the *last* digit's
/// byte doubles up as the sign: for negative values the digit is instead one
/// of EBCDIC '}' (zero) or 'J'-'R' (digits 1-9), e.g. 'J' means "digit 1,
/// negative". This restores the ordinary digit character ('0'-'9') in place.
/// @param c pointer to the last-digit byte of a zoned field known to be negative
void decodeSign(char *c)
{
	if (*c=='}')
		*c='0';
	else
		*c='1' + (*c-'J');
}

/// @brief Inverse of decodeSign(): encode a negative zoned decimal's last digit
/// by replacing the plain digit character with its sign-bearing equivalent
/// ('0'->'}', '1'-'9'->'J'-'R').
/// @param c pointer to the last-digit byte of a zoned field to mark negative
void encodeSign(char *c)
{
	if (*c=='0')
		*c='}';
	else
		*c='J' + (*c-'1');
}
/// @brief Render a zoned-decimal field as a printable/parseable C string (e.g. "-123.45").
/// @param zptr pointer to the first byte of the zoned field
/// @param digits total number of digits (precision)
/// @param fraction number of digits after the implied decimal point
/// @return pointer to a static internal buffer holding the formatted number
char *zonedToChar(const char *zptr, int digits, int fraction)
{
    static char	buf[MAX_DECIMAL_DIGITS + 3]; // +3 for sign, decimal point, and null terminator
	char *bufPtr = buf;
	// A negative zoned value is 0xD0-D9 (}, J-R)
	bool positive=isdigit(zptr[digits-1]);
	if (!positive)
	{
		// The sign-bearing digit is decoded in place further down, once it has actually
		// been copied into buf -- doing it here read uninitialized memory.
		*buf='-';
		bufPtr++;
	}

	// Copy in everything to the left of the decimal point
	int precision = digits - fraction;
	memcpy(bufPtr, zptr, precision);
	bufPtr += precision;
	// Add '.' to buffer
	*bufPtr='.';
	bufPtr++;
	// Copy in everything to the right of the decimal point
	memcpy(bufPtr, zptr + precision, fraction);
	// Decode the sign of the last digit if negative  
	bufPtr += fraction - 1;
	if (!positive)
	   decodeSign(bufPtr);
	// Null terminate the string
    bufPtr++;
    *bufPtr='\0';
	return buf;
}

/// @brief Format a zoned field as exact decimal text into a caller-supplied buffer.
///
/// Same digits zonedToChar() produces, but with leading zeros suppressed and without the
/// shared static buffer -- this is what operator<<(ostream&, Zoned) uses so that printing
/// a Zoned shows every digit, instead of implicitly converting to long double and picking
/// up the stream's default 6-significant-digit precision.
/// @param buf destination, must hold at least digits+3 bytes (sign, '.', null)
/// @param zptr pointer to the first byte of the zoned field
/// @param digits total number of digits (precision)
/// @param fraction number of digits after the implied decimal point
/// @return buf
char *zonedFormat(char *buf, const char *zptr, int digits, int fraction)
{
	bool negative = (isdigit(zptr[digits-1]) == 0);
	int intLen = digits - fraction;

	// Suppress leading zeros, but always leave at least one integer digit
	int start = 0;
	while (start < intLen-1 && zptr[start]=='0')
		start++;

	char *out = buf;
	if (negative)
		*out++ = '-';
	if (intLen <= 0)
		*out++ = '0'; // e.g. Zoned<2,2>: no integer digits are stored at all
	else
	{
		memcpy(out, zptr+start, intLen-start);
		out += intLen-start;
	}
	if (fraction > 0)
	{
		*out++ = '.';
		memcpy(out, zptr+intLen, fraction);
		out += fraction;
	}
	*out = '\0';
	// The last digit carries the sign nibble; restore it to a plain digit
	if (negative)
		decodeSign(out-1);
	return buf;
}

/// @brief Inverse of zonedToChar(): parse a decimal string (e.g. "-123.45", as returned
/// by ODBC's exact-text retrieval of a NUMERIC/DECIMAL column) directly into a zoned-decimal
/// field, without going through a double -- this is what preserves the full-precision
/// exactness Java's ResultSet.getBigDecimal()/RecordJDBC.getDecimal() give you (see
/// RrecordODBC::readDecimal()).
/// @param zptr pointer to the first byte of the destination zoned field
/// @param digits total number of digits (precision) in the field
/// @param fraction number of digits to the right of the implied decimal point
/// @param str a decimal string, optionally signed ('-'/'+'), optionally containing a
/// single '.'; integer/fraction digits that don't fit are zero-padded or truncated from
/// the high/low order end respectively, matching RPG's MOVE/assignment truncation rules.
void zonedFromChar(char *zptr, int digits, int fraction, const char *str)
{
	while (*str==' ')
		str++;
	bool negative = (*str=='-');
	if (negative || *str=='+')
		str++;

	const char *dot=strchr(str, '.');
	int intLen = dot ? static_cast<int>(dot-str) : static_cast<int>(strlen(str));
	const char *fracStr = dot ? dot+1 : "";
	int fracLen = static_cast<int>(strlen(fracStr));

	int intDigits=digits-fraction;
	// Right-justify the integer part, padding/truncating high-order digits
	if (intLen>=intDigits)
		memcpy(zptr, str+(intLen-intDigits), intDigits);
	else
	{
		memset(zptr, '0', intDigits-intLen);
		memcpy(zptr+(intDigits-intLen), str, intLen);
	}
	// Left-justify the fraction part, padding/truncating low-order digits
	if (fraction>0)
	{
		int fracCopy = fracLen<fraction ? fracLen : fraction;
		memcpy(zptr+intDigits, fracStr, fracCopy);
		if (fracCopy<fraction)
			memset(zptr+intDigits+fracCopy, '0', fraction-fracCopy);
	}
	if (negative)
		encodeSign(zptr+digits-1);
}

////////////////////////////////////////////////////////////////////////////////
//Global TESTx functions
/// @brief RPG TESTN opcode: classify a character as blank, a valid numeric
/// digit (including zoned sign-encoded 'A'-'R'), or invalid.
/// @return 0 for blank, 1 for a valid numeric character, -1 if not numeric.
int testn(char c)
{
	if (c==' ')
		return 0;
	if (isdigit(c) || (c>'A' && c<'R'))
		return 1;
	return -1;
}
/// @brief RPG TESTN opcode applied to a whole field: verify every non-blank
/// character is a plain digit ('0'-'9').
/// @return -1 if any non-blank, non-digit character is found; otherwise the
/// index of the last non-blank digit seen (0 if the field is entirely blank).
int testn(const FixedTemp &fStr)
{
	int numIndex=0;
	for (int i=0; i<fStr.len(); i++)
	{
		char c=fStr.overlay[i];
		if (c!=' ')
		{
			if (!isdigit(c))
				return -1;
			numIndex = i;
		}
	}
	return numIndex;
}

/// @brief RPG TESTZ opcode: check whether a character is a valid zoned-decimal
/// digit, including the sign-encoded forms ('&'/'A'-'I' for positive,
/// '-'/'J'-'R' for negative) described in decodeSign()/encodeSign().
/// @return 1 if it's a valid zoned digit character, 0 otherwise.
int testz(char c)
{
	if (c=='&' || (c>='A' && c<='I'))
		return 1;
	if (c=='-' || (c>='J' && c<='R'))
		return 1;
	return 0;
}
int testz(const FixedTemp &fStr)
	{ return testz(*fStr.overlay); }

////////////////////////////////////////////////////////////////////////////////
// Global MOVEx functions
/// @brief RPG MOVEL opcode: copy characters left-justified (from the start of
/// the source into the start of the destination), truncating/leaving the
/// remainder of a longer destination untouched - unlike a normal C++ 'assign'.
void movel(const FixedTemp &f1, const FixedTemp &f2)
	{ memcpy(f2.overlay, f1.overlay, MIN(f1.len(), f2.len())); }
/// @brief RPG MOVE opcode: copy characters right-justified (aligning the end
/// of the source with the end of the destination), leaving any leading bytes
/// of a longer destination untouched.
void move(const FixedTemp &f1, const FixedTemp &f2)
{
	int minsz=MIN(f1.len(), f2.len());
	memcpy(f2.overlay+f2.len()-minsz, f1.overlay+f1.len()-minsz, minsz);
}

char move(const FixedTemp &source, char &c)
{
   Fixed<1> fxd;
   fxd.move(source);
   c=fxd[0];
   return c;
}
char move(const _ConvertDecimal &source, char &c)
{
   Fixed<1> fxd;
   fxd.move(source);
   c=fxd[0];
   return c;
}

char movel(const FixedTemp &source, char &c)
{
	Fixed<1> fxd;
   fxd.movel(source);
   c=fxd[0];
   return c;
}
char movel(const _ConvertDecimal &source, char &c)
{
	Fixed<1> fxd;
   fxd.movel(source);
   c=fxd[0];
   return c;
}

short move(const FixedTemp &source, short &dest)
{
   Zoned<4, 0> znd(dest);
   znd.move(source);
   dest=(short)znd;
   return dest;
}

short move(const _ConvertDecimal &source, short &dest)
{
   Zoned<9, 0> znd(dest);
   znd.move(source);
   dest=(short)znd;
   return dest;
}

int move(const FixedTemp &source, int &dest)
{
   Zoned<9, 0> znd(dest);
   znd.move(source);
   dest=(int)znd;
   return dest;
}

int move(const _ConvertDecimal &source, int &dest)
{
   Zoned<9, 0> znd(dest);
   znd.move(source);
   dest=(int)znd;
   return dest;
}

