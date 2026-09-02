#ifndef RPGTYPES_H
# define RPGTYPES_H

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <math.h>
#include <cstddef>
#include <iosfwd>

#include "xxcvt.h" // For zoned conversion
#include "i2compat.h"
#if !defined(NO_PACKED)
# include <MIH/CPYNV.h>
# include <bcd.h> // For packed decimal
#endif

/// @file RPGTypes.h
/// @brief Global RPG-compatible state and helper functions: figurative constants,
/// the U/DATE family of special date fields, the 99-element indicator array
/// (RPG's IN01-IN99/*INxx), zoned-decimal sign encoding, and the TESTN/TESTZ/
/// MOVE/MOVEL opcodes RPG programs use for validating and transferring data
/// between fields of possibly different type/length.
///
/// See: [RPG data-type keywords](https://www.ibm.com/docs/en/i/7.4.0?topic=statement-data-type-keywords#datatypekw)
/// (this file implements the runtime behavior behind ZONED, PACKED, IND, etc.)


// Widest decimal this build supports. bcd.h sets DFT_DEC_DIG to 63 on IBM i 7.2 and later
// (31 on 7.1 and earlier); everywhere else we use the current 63-digit maximum. Taking it
// from IBM's own constant rather than hardcoding keeps the figurative constants below in
// range on any release, and makes this the correct size for any scratch buffer receiving a
// converted decimal.
#if defined(DFT_DEC_DIG)
# define I2_MAX_DECIMAL_DIGITS DFT_DEC_DIG
#else
# define I2_MAX_DECIMAL_DIGITS 63
#endif
constexpr int MAX_DECIMAL_DIGITS = I2_MAX_DECIMAL_DIGITS;

constexpr double compile_pow(int base, int exp) {
    return exp == 0 ? 1 : base * compile_pow(base, exp - 1);
}
/// @brief The maximum value that a decimal(MAX_DECIMAL_DIGITS, 0) number can hold
constexpr double MAX_DECIMAL_VALUE = compile_pow(10, MAX_DECIMAL_DIGITS) - 1;
constexpr double MIN_DECIMAL_VALUE = -MAX_DECIMAL_VALUE;
// The preprocessor cannot stringify a computed constexpr value, so these are spelled out
// in groups of ten digits. They are arrays rather than pointers so the static_asserts can
// measure them with sizeof -- ILE C++ rejects calling a constexpr function on a pointer in
// a constant expression (CZP0016).
#if I2_MAX_DECIMAL_DIGITS == 31
static const char MAX_DECIMAL_STR[] = "9999999999" "9999999999" "9999999999" "9";
static const char MIN_DECIMAL_STR[] = "-" "9999999999" "9999999999" "9999999999" "9";
#else
static const char MAX_DECIMAL_STR[] =
	"9999999999" "9999999999" "9999999999" "9999999999" "9999999999" "9999999999" "999";
static const char MIN_DECIMAL_STR[] =
	"-" "9999999999" "9999999999" "9999999999" "9999999999" "9999999999" "9999999999" "999";
#endif
static_assert(sizeof(MAX_DECIMAL_STR) - 1 == MAX_DECIMAL_DIGITS,
	"MAX_DECIMAL_STR must hold exactly MAX_DECIMAL_DIGITS digits");
static_assert(sizeof(MIN_DECIMAL_STR) - 1 == MAX_DECIMAL_DIGITS + 1,
	"MIN_DECIMAL_STR must be MAX_DECIMAL_STR with a leading '-'");

////////////////////////////////////////////////////////////////////////////////
/// @brief Errors thrown by i2class
class CI2Err { };
/// @brief Range of subscript error
#if I2_RUNTIME_ENABLE_BOUNDS_CHECK
class CI2ErrSubscript : public CI2Err { };
#endif
/// @brief Thrown by Rfile::open()/close() (and derived-class overrides) when the
/// underlying file/database operation fails; message holds driver-specific detail.
class CI2ErrFile : public CI2Err
{
public:
	explicit CI2ErrFile(const char *msg = "")
	{
		strncpy(message, msg, sizeof(message) - 1);
		message[sizeof(message) - 1] = '\0';
	}
	char message[256];
};


////////////////////////////////////////////////////////////////////////////////
/// @brief Temporary fixed-length, (std::ranges::view-like)[https://cppreference.com/cpp/ranges/view]
/// value used for certain parameters and temporary results (e.g. Fixed<> substring and concatenation)
class FixedTemp
{
public:
	FixedTemp() : overlay(nullptr), sz(0) {}
	FixedTemp(const char *tBuf, int tsz) : sz(tsz), overlay((char *)tBuf) {}
	FixedTemp(const char &c) : sz(1), overlay((char *)&c) {}
	FixedTemp(const char *str) : overlay((char *)str)
		{ sz=static_cast<int>(strlen(str)); }
	inline int len() const
		{ return sz; }
	inline char &operator [] (const int i)
		{ return overlay[i]; }
//private:
	char *overlay;
	int	sz;
};

////////////////////////////////////////////////////////////////////////////////
/// @brief A dynamically allocated 'String' used by the + operators
class StringTemp : public FixedTemp
{
public:
	StringTemp(int tsz)
	{
		overlay=new char[tsz];
		sz=tsz;
	}
	inline StringTemp(const FixedTemp &fStr1, const FixedTemp &fStr2);
	// Copy constructor so that pointer member (overlay) doesn't get deleted
	inline StringTemp(const StringTemp &sStr);
	~StringTemp()
		{ delete[] overlay; }
};
// This allows assignment of A="B" + C as well as A=B + "C" (above)
StringTemp operator + (const FixedTemp &fStr1, const FixedTemp &fStr2);
StringTemp operator + (const FixedTemp &fStr1, const char *str);
StringTemp operator + (const char *str, const FixedTemp &fStr2);

#if defined(NO_PACKED)
////////////////////////////////////////////////////////////////////////////////
/// @brief Temporary zoned-decimal value used instead of Zoned<> for certain parameters
class ZonedTemp : public FixedTemp
{
public:
	ZonedTemp(char *toverlay, int tsz, int tprecision) : FixedTemp(toverlay, tsz)
		{ precision=tprecision; }
	int DigitsOf() const
   	{ return sz; }
	int PrecisionOf() const
   	{ return precision; }
	operator long double() const
		{ return QXXZTOD((unsigned char *)overlay, sz, precision); }
//private:
	int	precision;
};
#define _ConvertDecimal ZonedTemp
#endif

const char OFF='0';
const char ON='1';
#define MIN(a,b) a<b ? a : b

////////////////////////////////////////////////////////////////////////////////
/// @brief Special [Figurative Constant](https://www.ibm.com/docs/en/i/7.4.0?topic=constants-figurative#spfig)
/// *BLANKS, *ZEROS, *HIVAL, *LOVAL
class FigConst
{
public:
	FigConst(char c, int /*value*/) {fillChar=c;}
	operator char ()
		{ return fillChar; }
//private:
	char fillChar;
};
extern I2CLASS_API FigConst BLANKS;

/// @brief Numeric [Figurative Constant](https://www.ibm.com/docs/en/i/7.4.0?topic=constants-figurative#spfig)
/// (all but BLANKS)
class FigConstNum : public FigConst
{
public:
	FigConstNum(char c, int value) : FigConst(c, value) {}
	virtual ~FigConstNum() = default;
	// Cast to an int
	virtual operator short() const=0;
	virtual operator int() const=0;
	virtual operator long double() const=0;
#if !defined(NO_PACKED)
	virtual operator _ConvertDecimal() const=0;
#endif
};

/// @brief Numeric [Figurative Constant](https://www.ibm.com/docs/en/i/7.4.0?topic=constants-figurative#spfig)
/// representing zeros (*ZEROS)
class Zeros : public FigConstNum
{
public:
	Zeros() : FigConstNum('0', 0) {}
	inline operator short() const I2_OVERRIDE
		{ return 0; }
	inline operator int() const I2_OVERRIDE
		{ return 0; }
	inline operator long double() const I2_OVERRIDE
		{ return 0; }
#if !defined(NO_PACKED)
	inline operator _ConvertDecimal() const I2_OVERRIDE
		{ return (_ConvertDecimal)"0"; }
#endif
};
extern I2CLASS_API Zeros ZEROS;

/// @brief [Figurative Constant](https://www.ibm.com/docs/en/i/7.4.0?topic=constants-figurative#spfig)
/// representing the lowest possible value (*LOVAL)
class Loval : public FigConstNum
{
public:
	Loval() : FigConstNum('\0', -1) {}
	inline operator short() const I2_OVERRIDE
   	{ return SHRT_MIN; }
	inline operator int() const I2_OVERRIDE
   	{ return INT_MIN; }
	inline operator long double() const I2_OVERRIDE
   	{ return MIN_DECIMAL_VALUE; }
#if !defined(NO_PACKED)
	inline operator _ConvertDecimal() const I2_OVERRIDE
		{ return (_ConvertDecimal)MIN_DECIMAL_STR; }
#endif
};
extern I2CLASS_API Loval LOVAL;

/// @brief [Figurative Constant](https://www.ibm.com/docs/en/i/7.4.0?topic=constants-figurative#spfig) 
/// representing the highest possible value (*HIVAL)
class Hival : public FigConstNum
{
public:
	Hival() : FigConstNum('\xFF', 1) {}
	inline operator short() const I2_OVERRIDE
   	{ return SHRT_MAX; }
	inline operator int() const I2_OVERRIDE
   	{ return INT_MAX; }
	inline operator long double() const I2_OVERRIDE
   	{ return MAX_DECIMAL_VALUE; }
#if !defined(NO_PACKED)
	inline operator _ConvertDecimal() const I2_OVERRIDE
		{ return (_ConvertDecimal)MAX_DECIMAL_STR; }
#endif
};
extern I2CLASS_API Hival HIVAL;


/// @brief A [Fixed-length character](https://www.ibm.com/docs/en/i/7.4.0?topic=type-character-format) string data type
template <int sz> class Fixed
{
public:
	Fixed()
		{ memset(overlay, ' ', sz); } // Always initialize to blanks
	Fixed(const char *str)
   	{ assign(str, static_cast<int>(strlen(str))); }
	Fixed(char c)
   	{ assign(c); }
	Fixed(const FigConst &fc)
		{ operator =(fc); }
	Fixed(FixedTemp &tStr)
   	{ assign(tStr.overlay, tStr.len()); }
#ifndef NO_FUNCTION_TEMPLATE
	template <int fsz> inline Fixed<sz>& operator = (const Fixed<fsz>& fStr)
	{
		assign(fStr.overlay, fStr.len());
		return *this;
	}
#endif
	// Do assignment from a FixedTemp
	inline Fixed<sz>& operator = (const FixedTemp &tStr)
	{
		assign(tStr.overlay, tStr.len());
		return *this;
	}

	// Do assignment from a character string literal
	inline Fixed<sz>& operator = (const char *str)
	{
		assign(str, strlen(str));
		return *this;
	}

	/*
	// Do assignment from a character
	Fixed<sz>& operator = (char c)
	{
		assign(c);
		return *this;
	}
	*/

	// Do assignment from a FigConst
	inline Fixed<sz>& operator = (const FigConst &fc)
	{
		memset(overlay, fc.fillChar, sz);
		return *this;
	}

//#ifdef NO_FUNCTION_TEMPLATE
	// Cast to a FixedTemp
	inline operator FixedTemp()
		{ return FixedTemp(overlay, sz); }
/*
	inline operator CharTemp()
		{ return CharTemp(overlay, sz); }
*/
//#endif


	// Cast to an int
	inline int toInt()
		{ return atoi(c_str()); }

	//Cast to a char
	/*
	inline operator char() const
		{ return *overlay; }
	*/

#ifdef NO_FUNCTION_TEMPLATE
	bool operator == (const FixedTemp &tStr) const
		{ return (Compare(tStr.overlay, tStr.len()) == 0); }
	bool operator != (const FixedTemp &tStr) const
		{ return (Compare(tStr.overlay, tStr.len()) != 0); }
	bool operator <= (const FixedTemp &tStr) const
		{ return (Compare(tStr.overlay, tStr.len()) <= 0); }
	bool operator <  (const FixedTemp &tStr) const
		{ return (Compare(tStr.overlay, tStr.len()) <  0); }
	bool operator >= (const FixedTemp &tStr) const
		{ return (Compare(tStr.overlay, tStr.len()) >= 0); }
	bool operator >  (const FixedTemp &tStr) const
		{ return (Compare(tStr.overlay, tStr.len()) >  0); }
#else
	template <int fsz>
    bool operator == (const Fixed<fsz> &fStr) const
		{ return (Compare(fStr.overlay, fStr.len()) == 0); }
	template <int fsz>
    bool operator != (const Fixed<fsz> &fStr) const
		{ return (Compare(fStr.overlay, fStr.len()) != 0); }
	template <int fsz>
    bool operator <= (const Fixed<fsz> &fStr) const
		{ return (Compare(fStr.overlay, fStr.len()) <= 0); }
	template <int fsz>
    bool operator <  (const Fixed<fsz> &fStr) const
		{ return (Compare(fStr.overlay, fStr.len()) <  0); }
	template <int fsz>
    bool operator >= (const Fixed<fsz> &fStr) const
		{ return (Compare(fStr.overlay, fStr.len()) >= 0); }
	template <int fsz>
    bool operator >  (const Fixed<fsz> &fStr) const
		{ return (Compare(fStr.overlay, fStr.len()) >  0); }
#endif

	// Do comparisons to (char* null-terminated) strings
	bool operator == (const char *str) const
		{ return (Compare(str, static_cast<int>(strlen(str))) == 0); }
	bool operator != (const char *str) const
		{ return (Compare(str, static_cast<int>(strlen(str))) != 0); }
	bool operator <= (const char *str) const
		{ return (Compare(str, static_cast<int>(strlen(str))) <= 0); }
	bool operator <  (const char *str) const
		{ return (Compare(str, static_cast<int>(strlen(str))) <  0); }
	bool operator >= (const char *str) const
		{ return (Compare(str, static_cast<int>(strlen(str))) >= 0); }
	bool operator >  (const char *str) const
		{ return (Compare(str, static_cast<int>(strlen(str))) >  0); }

	// Do comparisons between Fixeds and char
	bool operator == (char c) const
		{ return (Compare(&c, 1) == 0); }
	bool operator != (char c) const
		{ return (Compare(&c, 1) != 0); }
	bool operator <= (char c) const
		{ return (Compare(&c, 1) <= 0); }
	bool operator <  (char c) const
		{ return (Compare(&c, 1) <  0); }
	bool operator >= (char c) const
		{ return (Compare(&c, 1) >= 0); }
	bool operator >  (char c) const
		{ return (Compare(&c, 1) >  0); }

	// Do comparisons between Fixed and FigConst
	bool operator == (const FigConst &fc) const
		{ return (FigCompare(fc.fillChar) == 0); }
	bool operator != (const FigConst &fc) const
		{ return (FigCompare(fc.fillChar) != 0); }
	bool operator <= (const FigConst &fc) const
		{ return (FigCompare(fc.fillChar) <= 0); }
	bool operator <  (const FigConst &fc) const
		{ return (FigCompare(fc.fillChar) <  0); }
	bool operator >= (const FigConst &fc) const
		{ return (FigCompare(fc.fillChar) >= 0); }
	bool operator >  (const FigConst &fc) const
		{ return (FigCompare(fc.fillChar) >  0); }

	/// @returns The character at the 0-based `index`
	/// @throws CI2ErrSubscript if <0 or >= `sz` size()
	char &operator [] (const int index)
   {
#if I2_RUNTIME_ENABLE_BOUNDS_CHECK
		if (index<0 || index>=sz)
      	throw CI2ErrSubscript();
#endif
		return overlay[index];
    }

	/// @returns The length of this field in bytes, equal to `sz` size()
	int len() const
		{ return sz; }

	/// @brief Check Characters [%CHECK](https://www.ibm.com/docs/en/i/7.4.0?topic=functions-check-check-characters) equivalent.
	/// @return 1-based position of the first character, scanning from
	/// `start`, that is not in `cStr`; 0 if every character examined matches.
	int check(const char *cStr, int start=1) const
		{ return checkStr(cStr, start-1, sz, 1); }
	int check(char c, int start=1) const
	{
#if I2_RUNTIME_ENABLE_BOUNDS_CHECK
		if (start>sz || start<=0)
      	throw CI2ErrSubscript();
#endif
      for (int i=start-1; i<sz; i++)
      {
      	if (overlay[i]!=c)
         	return i+1;
      }
      return 0;
	}

	/// @brief Check Reverse [%CHECKR](https://www.ibm.com/docs/en/i/7.4.0?topic=functions-checkr-check-characters-right-left) equivalent.
	///
	/// See: check()
	int checkr(const char *cStr, int start=sz) const
		{ return checkStr(cStr, start-1, -1, -1); }
	int checkr(char c, int start=sz) const
	{
#if I2_RUNTIME_ENABLE_BOUNDS_CHECK
		if (start>sz || start<=0)
      	throw CI2ErrSubscript();
#endif
      for (int i=start-1; i>=0; i--)
      {
      	if (overlay[i]!=c)
         	return i+1;
      }
      return 0;
	}

	/// [%TRIM](https://www.ibm.com/docs/en/i/7.4.0?topic=functions-trim-trim-characters-edges) BIF.
	/// @returns A FixedTemp<> view of this value
	/// with leading and trailing blanks removed
	FixedTemp trim() const
	{
		int offset=check(' ')-1;
		return FixedTemp(overlay+offset, checkr(' ')-offset);
	}
	/// Trim Left [%TRIML](https://www.ibm.com/docs/en/i/7.4.0?topic=functions-triml-trim-leading-characters) BIF.
	/// @returns A FixedTemp<> view of this value
	/// with leading blanks removed
	FixedTemp triml() const
	{
		int offset=check(' ')-1;
		return FixedTemp(overlay+offset, sz-offset);
	}
	/// Trim Right [%TRIMR](https://www.ibm.com/docs/en/i/7.4.0?topic=functions-trimr-trim-trailing-characters) equivalent.
	/// @returns A FixedTemp<> view of this value
	/// with trailing blanks removed
	FixedTemp trimr() const
		{ return FixedTemp(overlay, checkr(' ')); }

	/// @brief Substring [%SUBST](https://www.ibm.com/docs/en/i/7.4.0?topic=functions-subst-substring) equivalent.
	/// @returns A FixedTemp<> substring view starting at the 1-based `start` position
	/// @throws `CI2ErrSubscript` if `start` is <=0 or > `sz` size().
	FixedTemp subst(int start) const
		{ return subst(start, sz-start+1); }
	FixedTemp subst(int start, int length) const
   {
#if I2_RUNTIME_ENABLE_BOUNDS_CHECK
		if (start>sz || start<=0)
      	throw CI2ErrSubscript();
#endif
		return FixedTemp((char *)overlay+start-1, length);
   }

	/// @brief Scan for characters [%SCAN](https://www.ibm.com/docs/en/i/7.4.0?topic=functions-scan-scan-string) equivalent.
	/// @returns 1-based position of the first occurrence of any character in `this` value 
	/// that is also in `str`, or 0 if no match found.
	int scan(const char *str) const
	{
		return scanStr(str, static_cast<int>(strlen(str)));
	}
	int scan(char c) const
	{
		const char *s=(const char *)memchr(overlay, c, sz);
		if (s)
			return static_cast<int>(s-overlay)+1;
		return 0;
	}
	int scan(const FixedTemp &tStr) const
	{
		return scanStr (tStr.overlay, tStr.len());
	}

	/// @brief String concatenation [%CONCAT](https://www.ibm.com/docs/en/i/7.4.0?topic=codes-cat-concatenate) equivalent.
	/// @returns concatenation of `t1` and `t2`
	void cat(FixedTemp t1, FixedTemp t2)
		{ catStr(t1.overlay, t1.len(), t2.overlay, t2.len(), 0); }
	void cat(FixedTemp t1, FixedTemp t2, int blanks)
   {
   	catStr(t1.overlay, checkStr(t1.overlay, t1.len()-1, -1, -1), t2.overlay,
       checkStr(t2.overlay, t2.len()-1, -1, -1), blanks);
	}

	/// @brief BIF-like [MOVEL](https://www.ibm.com/docs/en/i/7.4.0?topic=codes-movel-move-left) opcode equivalent.
	///
	/// Copies a value (Fixed<>, literal, or decimal) into the left-most positions of `this` field
	Fixed<sz>& movel(const char *str)
	{
		memcpy(overlay, str, MIN(sz, strlen(str)));
		return *this;
	}
	Fixed<sz>& movel(FixedTemp tStr)
	{
		memcpy(overlay, tStr.overlay, MIN(sz, tStr.len()));
		return *this;
	}
#if !defined(NO_PACKED)
	Fixed<sz>& movel(const _ConvertDecimal p)
	{
		char buf[MAX_DECIMAL_DIGITS];
		cpynv(NUM_DESCR(_T_ZONED, p.DigitsOf(), p.PrecisionOf()), buf,
       NUM_DESCR(_T_PACKED, p.DigitsOf(), p.PrecisionOf()), (char *)&p);
		memcpy(overlay, buf, MIN(sz, p.DigitsOf()));
		return *this;
	}
#endif
	// Move a value to the right-most bytes of the string
	Fixed<sz>& move(FixedTemp tStr)
	{
		int minsz=MIN(sz, tStr.len());
		memcpy(overlay+sz-minsz, tStr.overlay+tStr.len()-minsz, minsz);
		return *this;
	}
#if !defined(NO_PACKED)
	Fixed<sz>& move(const _ConvertDecimal p)
	{
		char buf[MAX_DECIMAL_DIGITS];
		cpynv(NUM_DESCR(_T_ZONED, p.DigitsOf(), p.PrecisionOf()), buf,
       NUM_DESCR(_T_PACKED, p.DigitsOf(), p.PrecisionOf()), (char *)&p);
		int minsz=MIN(sz, p.DigitsOf());
		memcpy(overlay+sz-minsz, buf+p.DigitsOf()-minsz, minsz);
		return *this;
	}
#endif
	Fixed<sz>& move(const char *str)
	{
		int strLen=strlen(str);
		int minsz=MIN(sz, strLen);
		memcpy(overlay+sz-minsz, str+strLen-minsz, minsz);
		return *this;
	}
	/// @brief BIF-like [MOVEA](https://www.ibm.com/docs/en/i/7.4.0?topic=codes-movea-move-array) opcode
	/// equivalent for a single field: overwrites `this` starting at the 1-based `index`
	/// with `tStr`/`fc`/`c`, leaving bytes before `index` untouched.
	/// @throws CI2ErrSubscript if `index` is <=0 or > `sz` size().
	Fixed<sz>& movea(FixedTemp tStr, int index=1)
	{
#if I2_RUNTIME_ENABLE_BOUNDS_CHECK
		if (index<=0 || index>sz)
      	throw CI2ErrSubscript();
#endif
		memcpy(overlay+index-1, tStr.overlay, MIN(sz-(index-1), tStr.len()));
		return *this;
	}
	// Replace a portion of the string with a figurative constant
	Fixed<sz>& movea(const FigConst &fc, int index=1)
	{
#if I2_RUNTIME_ENABLE_BOUNDS_CHECK
		if (index<=0 || index>sz)
      	throw CI2ErrSubscript();
#endif
		memset(overlay+index-1, fc.fillChar, sz-(index-1));
		return *this;
	}
	Fixed<sz>& moveall(char c, int index=1)
	{
#if I2_RUNTIME_ENABLE_BOUNDS_CHECK
		if (index<=0 || index>sz)
      	throw CI2ErrSubscript();
#endif
		memset(overlay+index-1, c, sz-(index-1));
		return *this;
	}

	/// @brief BIF-like [MOVEA](https://www.ibm.com/docs/en/i/7.4.0?topic=codes-movea-move-array) opcode
	/// equivalent for an array source: packs `array`'s elements, starting at the 1-based
	/// `index`, as a contiguous stream of raw bytes into the start of `this` field,
	/// stopping when either `array` or `this` field is exhausted.
	///
	/// This works for any element type used elsewhere in this file (Fixed<>/Zoned<>/
	/// Packed<>, or a plain numeric type) with no per-type conversion, because each one's
	/// in-memory layout *is* its RPG storage representation -- the same guarantee the
	/// `static_assert(sizeof(...) == sz)` checks in this file enforce. `&array[i]` and
	/// `sizeof(T)` are already the bytes RPG's MOVEA would move.
	///
	/// @note Known divergence from real RPG: this is a raw byte copy, not the
	/// value-preserving range-checked move real MOVEA does between differently-signed
	/// numeric types of the same byte length (see legacy-uef/RPGLE-NOTES.md) --
	/// intentional simplification, not a bug.
	/// @throws CI2ErrSubscript if `index` is <=0 or > N
	template <class T, int N>
	Fixed<sz>& movea(T (&array)[N], int index=1)
	{
#if I2_RUNTIME_ENABLE_BOUNDS_CHECK
		if (index<=0 || index>N)
      	throw CI2ErrSubscript();
#endif
		char *dest = overlay;
		int remaining = sz;
		for (int i=index-1; i<N && remaining>0; i++)
		{
			int n = MIN(static_cast<int>(sizeof(T)), remaining);
			memcpy(dest, &array[i], n);
			dest += n;
			remaining -= n;
		}
		return *this;
	}

	// Return a null-terminated c-style string. Shared static buffer per sz, so only one
	// result is live at a time -- see README Known Limitations. Use c_str(buf, bufSize)
	// below when you need your own storage.
	char *c_str() const
	{
		static char str[sz+1];
		memcpy(str, overlay, sz);
		str[sz]='\0';
		return str;
	}

	// Copy into a caller-supplied buffer and null-terminate it, like strlcpy(); returns
	// buf, like strcpy(). Truncates if bufSize <= sz.
	char *c_str(char *buf, size_t bufSize) const
	{
		size_t len = bufSize>0 ? MIN((size_t)sz, bufSize-1) : 0;
		memcpy(buf, overlay, len);
		if (bufSize>0)
			buf[len]='\0';
		return buf;
	}

	// Set the contents to a single character
	void assign(char c)
	{
		*overlay=c;
		memset(overlay+1, ' ', sz-1);
	}

public:
	char overlay[sz];
	static_assert(sizeof(overlay) == static_cast<std::size_t>(sz),
		"Fixed<sz> must be exactly sz bytes for IBM i binary compatibility");

protected:
	// Set this Fixed to the contents of another Fixed/String
	void assign(const char *str)
		{ memcpy(overlay, str, sz); }
	void assign(const char *str, int strLen)
	{
		if (sz>strLen)
		{
			memcpy(overlay, str, strLen);
			memset(overlay+strLen, ' ', sz-strLen);
		}
		else
			assign(str);
	}

	// Compare the Fixed to another Fixed/String
	int  Compare(const char *str) const
		{ return memcmp(overlay, str, sz); }
	int  Compare(const char *str, int strLen) const
	{
		int cmp = memcmp(overlay, str, MIN(sz, strLen));
		if (cmp==0 && sz!=strLen)
		{
			// Allow for strings that are padded with blanks to return '='
			if (sz<strLen)
			{
				for (int i=sz; i<strLen; i++)
					if (str[i]!=' ')
						return -1;
			}
			else
			{
				for (int i=strLen; i<sz; i++)
					if (overlay[i]!=' ')
						return 1;
			}
		}
		return cmp;
	}

	// Compare the Fixed to a Figurative constant
	int  FigCompare(const char fc) const
	{
		int cmp;
		for (int i=0; i<sz; i++)
		{
			cmp=overlay[i]-fc;
			if (cmp != 0)
				break;
		}
		return cmp;
	}
private:
	// Return position of character that is not in cStr
	int checkStr(const char *cStr, int start, int end, int increment) const
	{
#if I2_RUNTIME_ENABLE_BOUNDS_CHECK
		if (start>=sz || start<0)
      	throw CI2ErrSubscript();
#endif
		for (int i=start; i!=end; i=i+increment)
		{
			bool inSet=false;
			for (const char *s=cStr; *s!='\0'; s++)
			{
				if (overlay[i]==*s)
				{
					inSet=true;
					break;
				}
			}
			// First character not in cStr wins; otherwise keep looking
			if (!inSet)
				return i+1;
		}
		// Every character examined was in cStr
		return 0;
	}

	// Concatenate two strings and assign to fixed
	void catStr(const char *s1, int s1Len, const char *s2, int s2Len, int blanks)
	{
		// Not static, and not shared: s1/s2 may alias overlay, so build the result
		// somewhere else first, then blank-fill whatever the sources did not cover.
		char buf[sz];
		int len=MIN(s1Len, sz);
		memcpy(buf, s1, len);
		blanks=MIN(blanks, sz-len);
		memset(buf+len, ' ', blanks);
		len=len+blanks;
		int len2=MIN(s2Len, sz-len);
		memcpy(buf+len, s2, len2);
		if (len+len2 < sz)
			memset(buf+len+len2, ' ', sz-len-len2);
		memcpy(overlay, buf, sz);
	}

	// Return position of string in fixed
	int scanStr(const char *str, int strLen) const
	{
		if (strLen<=0 || strLen>sz)
			return 0;
		// Last position a needle of strLen can still start at
		for (int i=0; i<=sz-strLen; i++)
		{
			if (memcmp(overlay+i, str, strLen)==0)
				return i+1;
		}
		return 0;
	}
};
#define fixed(sz) Fixed<sz>
template <int sz> inline
 FixedTemp subst(const Fixed<sz> &fStr, int start)
	{ return fStr.subst(start); }
template <int sz>
 FixedTemp subst(const Fixed<sz> &fStr, int start, int length)
	{ return fStr.subst(start, length); }
template <int sz>
 FixedTemp trimr(const Fixed<sz> &fStr)
	{ return fStr.trimr(); }
template <int sz>
 FixedTemp triml(const Fixed<sz> &fStr)
	{ return fStr.triml(); }
template <int sz>
 FixedTemp trim(const Fixed<sz> &fStr)
	{ return fStr.trim(); }

////////////////////////////////////////////////////////////////////////////////
/// @brief A Fixed<1> ~boolean value containing either '0'=false, or '1'=true
/// @note If `this` value contains anything other than '0' or '1', 
/// then unpredictable results can occur.
class Indicator : public Fixed<1>
{
public:
	Indicator()
		{ *overlay='0'; }
	Indicator(bool b)
		{ operator=(b); }
	Indicator& operator = (bool b)
	{
		if (b)
			*overlay='1';
		else
			*overlay='0';
		return *this;
	}
#if !defined(bool)
	Indicator& operator = (int i)
	{
		bool b=i;
		return operator=(b);
	}
#endif
	Indicator& operator = (char c)
	{
		*overlay=c;
		return *this;
	}

	// Cast to a char
	operator char()
		{return *overlay; }

	// Cast to a bool
	operator bool()
	{
		if (*overlay=='1')
			return true;
		else
			return false;
	}
	bool operator !()
	{
		if (*overlay!='1')
			return true;
		else
			return false;
	}
};
static_assert(sizeof(Indicator) == 1, "Indicator must be exactly 1 byte for IBM i binary compatibility");

////////////////////////////////////////////////////////////////////////////////
/// @brief A [Multi-occurrence](https://www.ibm.com/docs/en/i/7.4.0?topic=codes-occur-setget-occurrence-data-structure) data structure
template <int sz, int elem> class DS : public Fixed<sz>
{
public:
	DS() : Occur(1) {}
	// Do assignment from one DS to another
	DS<sz, elem>& operator = (const DS<sz, elem>& ds)
	{
		memcpy(Fixed<sz>::overlay, ds.overlay, sz);
		memcpy(buffer, ds.buffer, sz*(elem-1));
		return *this;
	}
	// Do assignment from a fixed
#ifndef NO_FUNCTION_TEMPLATE
	template <int fsz> DS<sz, elem>& operator = (const Fixed<fsz>& fStr)
	{
		assign(fStr.overlay, fStr.len());
		return *this;
	}
#endif
	// Do assignment from a character string literal
	DS<sz, elem>& operator = (const char *str)
	{
		Fixed<sz>::assign(str, static_cast<int>(strlen(str)));
		return *this;
	}

	// Do assignment from a FixedTemp
	DS<sz, elem>& operator = (const FixedTemp &tStr)
	{
		Fixed<sz>::assign(tStr.overlay, tStr.len());
		return *this;
	}
	/*
	// Do assignment from a character
	DS<sz, elem>& operator = (const char c)
	{
		overlay[0]=c;
		memset(overlay+1, ' ', sz-1);
		return *this;
	}
	*/
	// Do assignment from a FigConst
	DS<sz, elem>& operator = (const FigConst &fc)
	{
		memset(Fixed<sz>::overlay, fc.fillChar, sz);
		return *this;
	}

	/*
	// Cast to a Fixed
	operator Fixed<sz>()
	{
		Fixed<sz>	fix((char *)overlay, sz);
		return fix;
	}
	*/
	// A mult-occur DS is implemented as a Fixed with a buffer for all occurrences
	// except the current one.  buffer[Occur-2] holds the value of DS[0] and
	// overlay is the current (Occur) occurrence.
   // buffer[-1] is valid and actually points to overlay
	void occur(short lOccur)
	{
	#if I2_RUNTIME_ENABLE_BOUNDS_CHECK
		if (lOccur < 1 || lOccur > elem)
			throw CI2ErrSubscript();
	#endif
		if (Occur!=lOccur)
		{
			// Not static: these are per-call scratch, not per-instance state -- a
			// static local here was shared (and clobbered) by every DS<sz,elem>
			// object of the same size calling occur() concurrently or reentrantly.
			char	buffer0[sz];
			memcpy(buffer0, Fixed<sz>::overlay, sz);
			short i;
			if (lOccur==1)
			{
				i=Occur-2;
				goto swap;
			}
			else if (Occur==1)
			{
				i=lOccur-2;
			swap:
				memcpy(Fixed<sz>::overlay, buffer[i].overlay, sz);
				memcpy(buffer[i].overlay, buffer0, sz);
			}
			else
			{
				memcpy(Fixed<sz>::overlay, buffer[lOccur-2].overlay, sz);
				memcpy(buffer[lOccur-2].overlay, buffer[Occur-2].overlay, sz);
				memcpy(buffer[Occur-2].overlay, buffer0, sz);
			}
			Occur=lOccur;
		}
	}
	short Occur;
private:
	Fixed<sz>	buffer[elem-1];
};

////////////////////////////////////////////////////////////////////////////////
// Zoned decimal
void decodeSign(char *c);
void encodeSign(char *c);
char *zonedToChar(const char *zptr, int digits, int fraction);
void zonedFromChar(char *zptr, int digits, int fraction, const char *str);
/// @brief Format a zoned field as exact decimal text (e.g. "-123.45") into a
/// caller-supplied buffer, which must hold at least digits+3 bytes (sign, decimal
/// point, null). Leading zeros are suppressed. Unlike zonedToChar() there is no
/// shared static buffer, so this is safe to call from anywhere.
char *zonedFormat(char *buf, const char *zptr, int digits, int fraction);

////////////////////////////////////////////////////////////////////////////////
// Decimal literals -- __D()/__Z()
//
// On IBM i, bcd.h supplies __D(s) as (_ConvertDecimal)(s), so a decimal constant keeps
// its exact digits instead of going through a double. See:
// https://www.ibm.com/docs/en/i/7.4.0?topic=template-using-d-macro-simplify-code
// The string may only contain 0-9 . - +
#if defined(NO_PACKED)
/// @brief The non-IBM-i stand-in for a bcd.h _ConvertDecimal constant.
///
/// Holds the literal text rather than a converted value, so assigning it into a
/// Zoned<> reproduces the written digits exactly -- `z = __D("8.7")` can never pick up
/// the binary floating-point expansion of 8.7 the way `z = 8.7` potentially could.
/// The text must outlive the DecimalConst, which is automatic for the string literals
/// __D()/__Z() are meant to be used with.
class DecimalConst
{
public:
	explicit DecimalConst(const char *s) : text(s), digits(0), fraction(0)
	{
		const char *p = text;
		if (*p=='-' || *p=='+')
			p++;
		for (bool afterPoint=false; *p!='\0'; p++)
		{
			if (*p=='.')
				afterPoint = true;
			else
			{
				digits++;
				if (afterPoint)
					fraction++;
			}
		}
	}
	int DigitsOf() const { return digits; }
	int PrecisionOf() const { return fraction; }
	const char *str() const { return text; }
	operator long double() const { return strtod(text, nullptr); }
private:
	const char *text;
	int digits, fraction;
};
# define __D(DECIMAL_CONSTANT) DecimalConst(DECIMAL_CONSTANT)
#endif
/// @brief Zoned-decimal literal; the zoned counterpart of __D(). Same exact-digits
/// guarantee, so `z = __Z("8.7")` stores 8.7 rather than a rounded double.
#define __Z(DECIMAL_CONSTANT) __D(DECIMAL_CONSTANT)


/// @brief A numeric [zoned-decimal](https://www.ibm.com/docs/en/i/7.4.0?topic=type-zoned-decimal-format) value 
/// backed by an unformatted (no sign or decimal point) fixed-length string of characters. 
/// @tparam sz The total number of digits
/// @tparam precision The number of digits to the right of the decimal point
template <int sz, int precision> class Zoned
{
public:
	Zoned()
		{ memset(overlay, '0', sz); } // Always initialize to 0
#if !defined(NO_PACKED)
	Zoned(const _ConvertDecimal p)				{ operator=(p); };
	//Zoned(const _DecimalT<sz, precision> p)	{ operator=(p); };
#endif
	Zoned(short s)
   	{ assignInt(s); }
	Zoned(int i)
   	{ assignInt(i); }
	Zoned(long l)
   	{ assign(l); }
	Zoned(float f)
   	{ assign(f); }
	Zoned(double d)
   	{ assign(d); }
	Zoned(long double d)
   	{ assign(d); }
	Zoned(const FigConstNum &fc)
   	{ assign(fc); }
#if defined(NO_PACKED)
	Zoned(const DecimalConst &d)
   	{ assign(d); }
#endif

#if !defined(NO_PACKED)
	// Do assignment from a packed
	Zoned<sz, precision>& operator = (const _ConvertDecimal &p)
	{
		cpynv(NUM_DESCR(_T_ZONED, sz, precision), overlay,
       NUM_DESCR(_T_PACKED, p.DigitsOf(), p.PrecisionOf()), (char *)&p);
		return *this;
	}
	Zoned<sz, precision>& operator = (const char *)
	{
		memset (overlay, '0', sz);
		return *this;
	}
	// The assignment operator defined above is a dummy operator used to
	// eliminate ambiguity in the following situation :
	//      Zoned <10,2> dec; dec = 0;
#else
	// Do assignment from a long double
	Zoned<sz, precision>& operator = (long double d)
	{
		assign(d);
		return *this;
	}
	// Do assignment from a decimal literal -- __D("8.7") keeps the written digits
	Zoned<sz, precision>& operator = (const DecimalConst &d)
	{
		assign(d);
		return *this;
	}
	bool operator == (const DecimalConst &d) const { return CompareConst(d) == 0; }
	bool operator != (const DecimalConst &d) const { return CompareConst(d) != 0; }
	bool operator <= (const DecimalConst &d) const { return CompareConst(d) <= 0; }
	bool operator <  (const DecimalConst &d) const { return CompareConst(d) <  0; }
	bool operator >= (const DecimalConst &d) const { return CompareConst(d) >= 0; }
	bool operator >  (const DecimalConst &d) const { return CompareConst(d) >  0; }
#endif
   // Do assignment from a FigConstNum
	Zoned<sz, precision>& operator = (const FigConstNum &fc)
	{
      assign(fc);
		return *this;
	}

	// Do cast from zoned to long double
	operator long double() const
		{ return QXXZTOD((unsigned char *)overlay, sz, precision); }

	// Do cast from zoned to int
//	operator int() const
	int toInt() const
		{ return QXXZTOI((unsigned char *)overlay, sz, precision); }

	// Add/subtract one rather than incrementing toPacked() -- it returns by value, and in
	// NO_PACKED builds that value is a long double, which cannot be incremented.
	Zoned<sz,precision> &operator ++ () // increment (prefix)
	{
		*this = toPacked() + 1;
		return *this;
	}
	Zoned<sz,precision> operator ++ (int) // increment (postfix)
	{
		Zoned<sz, precision> OrigVal = *this;
		*this = toPacked() + 1;
		return (OrigVal);
	}
	Zoned<sz,precision> &operator -- () // decrement (prefix)
	{
		*this = toPacked() - 1;
		return *this;
	}
	Zoned<sz,precision> operator -- (int) // decrement (postfix)
	{
		Zoned<sz, precision> OrigVal = *this;
		*this = toPacked() - 1;
		return (OrigVal);
	}
	// Add another Zoned (any size/precision) in place, matching RPG's ADD/+= semantics --
	// the native IBM i packed-decimal type already supports +=; this is the equivalent for
	// both that type and the portable (non-IBM-i) Zoned fallback.
	template <int l2, int p2>
	Zoned<sz,precision> &operator += (const Zoned<l2,p2> &val)
	{
		*this = toPacked() + val.toPacked();
		return *this;
	}
	Zoned<sz,precision> &operator += (long double val)
	{
		assign((long double)*this + val);
		return *this;
	}
	// Subtract another Zoned (any size/precision) in place, matching RPG's SUB/-= semantics --
	// see operator+=() above.
	template <int l2, int p2>
	Zoned<sz,precision> &operator -= (const Zoned<l2,p2> &val)
	{
		*this = toPacked() - val.toPacked();
		return *this;
	}
	Zoned<sz,precision> &operator -= (long double val)
	{
		assign((long double)*this - val);
		return *this;
	}
	// Multiply by another Zoned (any size/precision) in place, matching RPG's MULT/*=
	// semantics -- see operator+=() above.
	template <int l2, int p2>
	Zoned<sz,precision> &operator *= (const Zoned<l2,p2> &val)
	{
		*this = toPacked() * val.toPacked();
		return *this;
	}
	Zoned<sz,precision> &operator *= (long double val)
	{
		assign((long double)*this * val);
		return *this;
	}
	// Divide by another Zoned (any size/precision) in place, matching RPG's DIV//= semantics --
	// see operator+=() above.
	template <int l2, int p2>
	Zoned<sz,precision> &operator /= (const Zoned<l2,p2> &val)
	{
		*this = toPacked() / val.toPacked();
		return *this;
	}
	Zoned<sz,precision> &operator /= (long double val)
	{
		assign((long double)*this / val);
		return *this;
	}
#if !defined(NO_PACKED)
	// Explicit packed overloads. Without these a packed operand reaches operator OP=(long
	// double) and (via Zoned's operator _ConvertDecimal) the _ConvertDecimal form equally
	// well -- one user-defined conversion each, so the call is ambiguous. These match
	// exactly, so they win outright and keep the arithmetic in decimal rather than binary
	// floating point.
# define GEN_ZONED_PACKED_ASSIGN_OP(OP) \
	template <int d, int p> \
	Zoned<sz,precision> &operator OP##= (const _DecimalT<d,p> &val) \
		{ *this = toPacked() OP val; return *this; } \
	Zoned<sz,precision> &operator OP##= (const _ConvertDecimal &val) \
		{ *this = toPacked() OP val; return *this; }
	GEN_ZONED_PACKED_ASSIGN_OP(+)
	GEN_ZONED_PACKED_ASSIGN_OP(-)
	GEN_ZONED_PACKED_ASSIGN_OP(*)
	GEN_ZONED_PACKED_ASSIGN_OP(/)
# undef GEN_ZONED_PACKED_ASSIGN_OP

	// Cast to a packed
	_DecimalT<sz, precision> toPacked() const
	{
		_DecimalT<sz, precision> p;
		cpynv(NUM_DESCR(_T_PACKED, sz, precision), &p,
       NUM_DESCR(_T_ZONED, sz, precision), (_SPCPTRCN)overlay);
		return p;
	}
	operator _ConvertDecimal() const
	{
		_ConvertDecimal p(sz, precision);
		cpynv(NUM_DESCR(_T_PACKED, sz, precision), &p,
       NUM_DESCR(_T_ZONED, sz, precision), (_SPCPTRCN)overlay);
		return p;
	}
#else
	long double toPacked() const
		{ return operator long double(); }
#endif

#if defined(NO_PACKED)
	// Cast to a ZonedTemp
	operator ZonedTemp()
	{
		ZonedTemp z(overlay, sz, precision);
		return z;
	}
#endif

	int operator % (int val2)
	{
		int val1=toInt();
		//int val1=operator int();
		return val1-((long)(val1/val2))*val2;
	}

	int len() const {return sz;}
	int PrecisionOf() const {return precision;}

	// Move a value to the left-most bytes of the number
	Zoned<sz, precision>& movel(const FixedTemp &tStr)
	{
		memcpy(overlay, tStr.overlay, MIN(sz, tStr.len()));
		return *this;
	}
#if !defined(NO_PACKED)
	Zoned<sz, precision>& movel(const _ConvertDecimal p)
	{
		char buf[MAX_DECIMAL_DIGITS];
		cpynv(NUM_DESCR(_T_ZONED, p.DigitsOf(), p.PrecisionOf()), buf,
       NUM_DESCR(_T_PACKED, p.DigitsOf(), p.PrecisionOf()), (char *)&p);
		memcpy(overlay, buf, MIN(sz, p.DigitsOf()));
		return *this;
	}
#endif
	Zoned<sz, precision>& move(const FixedTemp &tStr)
	{
		int minsz=MIN(sz, tStr.len());
		memcpy(overlay+sz-minsz, tStr.overlay+tStr.len()-minsz, minsz);
		return *this;
	}
#if !defined(NO_PACKED)
	Zoned<sz, precision>& move(const _ConvertDecimal p)
	{
		char buf[MAX_DECIMAL_DIGITS];
		cpynv(NUM_DESCR(_T_ZONED, p.DigitsOf(), p.PrecisionOf()), buf,
       NUM_DESCR(_T_PACKED, p.DigitsOf(), p.PrecisionOf()), (char *)&p);
		int minsz=MIN(sz, p.DigitsOf());
		memcpy(overlay+sz-minsz, buf+p.DigitsOf()-minsz, minsz);
		return *this;
	}
#endif
	void assign(const FigConstNum &fc)
   {
   	if (fc.fillChar=='0')
			memset(overlay, '0', sz);
      else
      {
			memset(overlay, '9', sz);
         if (fc.fillChar==0x00)
		      overlay[sz-1]='R';
      }
   }
	// Do assignment from one Zoned to another
	void assign(const Zoned<sz, precision>& znd)
		{ memcpy(overlay, znd.overlay, sz); }
	// Do assignment from a long double
	void assign(long double d)
		{ QXXDTOZ((unsigned char *)overlay, sz, precision, static_cast<double>(d)); }
#if defined(NO_PACKED)
	// Do assignment from a decimal literal, digit for digit (never via double)
	void assign(const DecimalConst &d)
		{ zonedFromChar(overlay, sz, precision, d.str()); }
#endif

//protected:
	char overlay[sz];
	static_assert(sizeof(overlay) == static_cast<std::size_t>(sz),
		"Zoned<sz,precision> must be exactly sz bytes for IBM i binary compatibility");

private:

	// Do assignment from an int
	void assignInt(int i)
		{ QXXITOZ((unsigned char *)overlay, sz, precision, i); }
#if defined(NO_PACKED)
	// Compare against a decimal literal by re-scaling it to this field's shape, so the
	// comparison stays exact instead of going through a double
	int CompareConst(const DecimalConst &d) const
	{
		Zoned<sz, precision> tmp;
		tmp.assign(d);
		return Compare(tmp);
	}
#endif
	// Do comparisons between Zoned types
	int Compare(const Zoned<sz, precision> &znd) const
	{
		int cmp=(unsigned char)overlay[sz-1]-(unsigned char)znd.overlay[sz-1];
		if (cmp==0)
		{
			cmp = memcmp(overlay, znd.overlay, znd.len());
			// If both numbers are negative, then 'flip' the comparison
			if (cmp !=0 && !isdigit(overlay[sz-1]))
				cmp = cmp * -1;
		}
		return cmp;
	}
};

/// @brief Stream a Zoned as exact decimal text.
///
/// Without this, `std::cout << zonedValue` falls back to Zoned's implicit conversion to
/// long double and picks up the stream's default precision of 6 *significant* digits, so
/// e.g. 60145.76 prints as "60145.8". This mirrors what bcd.h provides for _DecimalT.
template <class charT, class traits, int sz, int precision>
inline std::basic_ostream<charT, traits> &
 operator << (std::basic_ostream<charT, traits> &os, const Zoned<sz, precision> &znd)
{
	char buf[sz + 3]; // sign, decimal point, null terminator
	return os << zonedFormat(buf, znd.overlay, sz, precision);
}

#if !defined(NO_PACKED)
# define GEN_ZONED_OP(OP, RTNTYP) \
	template <int l1, int p1, int l2, int p2> \
	 RTNTYP operator OP (Zoned<l1, p1> val1, Zoned<l2, p2> val2) \
	 {return val1.operator _ConvertDecimal() OP val2.operator _ConvertDecimal();} \
	template <int l1, int p1, int l2, int p2> \
	 RTNTYP operator OP (Zoned<l1, p1> val1, _DecimalT<l2, p2> val2) \
	  { return val1.operator _ConvertDecimal() OP val2; } \
	template <int l1, int p1, int l2, int p2> \
	 RTNTYP operator OP (_DecimalT<l1, p1> val1, Zoned<l2, p2> val2) \
	  { return val1 OP val2.operator _ConvertDecimal(); }

	GEN_ZONED_OP(==, bool)
	GEN_ZONED_OP(!=, bool)
	GEN_ZONED_OP(<=, bool)
	GEN_ZONED_OP(<, bool)
	GEN_ZONED_OP(>=, bool)
	GEN_ZONED_OP(>, bool)

	GEN_ZONED_OP(+, _ConvertDecimal)
	GEN_ZONED_OP(-, _ConvertDecimal)
	GEN_ZONED_OP(/, _ConvertDecimal)
	GEN_ZONED_OP(*, _ConvertDecimal)

// packed OP= Zoned. bcd.h's own OP= overloads only reach a Zoned through a user-defined
// conversion (to _ConvertDecimal, long double, int, ...), and several are equally good, so
// without these `packedVal += zonedVal` is ambiguous.
# define GEN_PACKED_ZONED_ASSIGN_OP(OP) \
	template <int l1, int p1, int l2, int p2> \
	 inline _DecimalT<l1, p1> &operator OP##= (_DecimalT<l1, p1> &val1, const Zoned<l2, p2> &val2) \
	  { val1 = val1 OP val2.toPacked(); return val1; }
	GEN_PACKED_ZONED_ASSIGN_OP(+)
	GEN_PACKED_ZONED_ASSIGN_OP(-)
	GEN_PACKED_ZONED_ASSIGN_OP(*)
	GEN_PACKED_ZONED_ASSIGN_OP(/)
#else
// Zoned OP Zoned on the non-IBM-i path. These have to exist explicitly: Zoned converts to
// ZonedTemp, which derives from FixedTemp, so without them `a + b` is ambiguous between
// built-in long double arithmetic and operator+(FixedTemp, FixedTemp) -- string
// concatenation. GCC rejects it outright; MSVC quietly picks one.
# define GEN_ZONED_OP(OP, RTNTYP) \
	template <int l1, int p1, int l2, int p2> \
	 inline RTNTYP operator OP (const Zoned<l1, p1> &val1, const Zoned<l2, p2> &val2) \
	  { return (long double)val1 OP (long double)val2; }

	GEN_ZONED_OP(==, bool)
	GEN_ZONED_OP(!=, bool)
	GEN_ZONED_OP(<=, bool)
	GEN_ZONED_OP(<, bool)
	GEN_ZONED_OP(>=, bool)
	GEN_ZONED_OP(>, bool)

	GEN_ZONED_OP(+, long double)
	GEN_ZONED_OP(-, long double)
	GEN_ZONED_OP(/, long double)
	GEN_ZONED_OP(*, long double)
#endif
#define zoned(sz, precision) Zoned<sz, precision>

#define FIGCONSTNUM_COMP1(OP) \
 bool operator OP (short val1, const FigConstNum &val2); \
 bool operator OP (int val1, const FigConstNum &val2); \
 bool operator OP (long double val1, const FigConstNum &val2); \
 template<int sz, int precision> \
  bool operator OP (Zoned<sz, precision> val1, const FigConstNum &val2) \
	{ return val1 OP (Zoned<sz, precision>)val2; }
#ifndef NO_PACKED
# define FIGCONSTNUM_COMP(OP) FIGCONSTNUM_COMP1(OP) \
 template<int sz, int precision> \
  bool operator OP (_DecimalT<sz, precision> val1, const FigConstNum &val2) \
	{ return val1 OP (Zoned<sz, precision>)val2; }
#else
# define FIGCONSTNUM_COMP(OP) FIGCONSTNUM_COMP1(OP)
#endif
FIGCONSTNUM_COMP(==)
FIGCONSTNUM_COMP(!=)
FIGCONSTNUM_COMP(<=)
FIGCONSTNUM_COMP(<)
FIGCONSTNUM_COMP(>=)
FIGCONSTNUM_COMP(>)


////////////////////////////////////////////////////////////////////////////////
/// Packed decimal
/// @brief A numeric [packed-decimal](https://www.ibm.com/docs/en/i/7.4.0?topic=type-packed-decimal-format) value
/// where each 4-bit nibble stores a BCD digit (0-9) and the final trailing nibble holds
/// the hexadecimal sign indicator (C/D/F).
///
/// A thin wrapper over whatever packed-decimal representation the platform provides
/// (_DecimalT from bcd.h on IBM i, Zoned<> elsewhere) so that packed(n,p) is one type with
/// one interface and one set of conversion rules everywhere, and so it offers the same
/// assign()/move()/movel()/toInt() surface as Zoned<> rather than bcd.h's separate one.
///
/// The single `overlay` member and absence of virtuals keep this layout-compatible with
/// the backing type, so a Packed field can still be overlaid directly on a record buffer.
/// @tparam sz The total number of digits
/// @tparam precision The number of digits to the right of the decimal point
template <int sz, int precision> class Packed
{
public:
#if !defined(NO_PACKED)
	typedef _DecimalT<sz, precision> Backing;
#else
	typedef Zoned<sz, precision> Backing;
#endif

	Packed()							{ assignInt(0); }
	Packed(short s)						{ assignInt(s); }
	Packed(int i)						{ assignInt(i); }
	Packed(long l)						{ assignInt(static_cast<int>(l)); }
	Packed(float f)						{ assign(static_cast<long double>(f)); }
	Packed(double d)					{ assign(static_cast<long double>(d)); }
	Packed(long double d)				{ assign(d); }
	Packed(const FigConstNum &fc)		{ assign(fc); }
	Packed(const Backing &b)			{ overlay = b; }
#if defined(NO_PACKED)
	Packed(const DecimalConst &d)		{ assign(d); }
#else
	// Exact matches for bcd.h's types. Without these a _ConvertDecimal reaches both
	// Packed(long double) and Packed(const Backing &) through one user-defined conversion
	// apiece, which is ambiguous -- and _ConvertDecimal is exactly what the arithmetic
	// operators below return.
	Packed(const _ConvertDecimal &d)	{ assign(d); }
	template <int d2, int p2>
	Packed(const _DecimalT<d2, p2> &d)	{ overlay = d; }
#endif

	Packed<sz, precision>& operator = (long double d)			{ assign(d); return *this; }
	Packed<sz, precision>& operator = (const FigConstNum &fc)	{ assign(fc); return *this; }
	Packed<sz, precision>& operator = (const Backing &b)		{ overlay = b; return *this; }
#if defined(NO_PACKED)
	Packed<sz, precision>& operator = (const DecimalConst &d)	{ assign(d); return *this; }
#else
	Packed<sz, precision>& operator = (const _ConvertDecimal &d) { assign(d); return *this; }
	template <int d2, int p2>
	Packed<sz, precision>& operator = (const _DecimalT<d2, p2> &d) { overlay = d; return *this; }
#endif
	template <int l2, int p2>
	Packed<sz, precision>& operator = (const Zoned<l2, p2> &z)	{ assign(z); return *this; }

	/// @brief Value as a double. Exact for fields inside a double's ~15 significant
	/// digits; use toChars()/operator<< when every digit has to survive.
	operator long double() const		{ return overlay; }
	int toInt() const;
	int len() const						{ return sz; }
	int DigitsOf() const				{ return sz; }
	int PrecisionOf() const				{ return precision; }

	/// @brief The native packed representation, for interop with bcd.h on IBM i.
	const Backing &toPacked() const		{ return overlay; }
	Zoned<sz, precision> toZoned() const;
	void fromZoned(const Zoned<sz, precision> &z);
#if !defined(NO_PACKED)
	/// @brief Convert to bcd.h's intermediate type so a Packed can be handed straight to
	/// the native packed-decimal operators. Copies first because _DecimalT's conversion
	/// operator is non-const and `overlay` is const here.
	operator _ConvertDecimal() const		{ Backing tmp = overlay; return tmp; }
#endif

	void assign(long double d);
	void assign(const FigConstNum &fc);
	template <int l2, int p2>
	void assign(const Zoned<l2, p2> &z)	{ fromZoned(Zoned<sz, precision>(static_cast<long double>(z))); }
#if defined(NO_PACKED)
	void assign(const DecimalConst &d)	{ overlay.assign(d); }
#else
	void assign(const _ConvertDecimal &d) { overlay = d; }
#endif

	// RPG MOVE/MOVEL operate on the digit positions, so they route through the zoned form
	Packed<sz, precision>& move(const FixedTemp &tStr)
	{
		Zoned<sz, precision> z = toZoned();
		z.move(tStr);
		fromZoned(z);
		return *this;
	}
	Packed<sz, precision>& movel(const FixedTemp &tStr)
	{
		Zoned<sz, precision> z = toZoned();
		z.movel(tStr);
		fromZoned(z);
		return *this;
	}

	int operator % (int val2) const
	{
		int val1 = toInt();
		return val1 - ((long)(val1 / val2)) * val2;
	}

public:
	Backing overlay;

private:
	void assignInt(int i);
};

template <int sz, int precision>
inline int Packed<sz, precision>::toInt() const
{
#if !defined(NO_PACKED)
	return static_cast<int>(overlay);
#else
	return overlay.toInt();
#endif
}

template <int sz, int precision>
inline void Packed<sz, precision>::assign(long double d)
{
#if !defined(NO_PACKED)
	overlay = d;
#else
	overlay.assign(d);
#endif
}

template <int sz, int precision>
inline void Packed<sz, precision>::assignInt(int i)
{
#if !defined(NO_PACKED)
	overlay = i;
#else
	overlay = Zoned<sz, precision>(i);
#endif
}

template <int sz, int precision>
inline void Packed<sz, precision>::assign(const FigConstNum &fc)
{
	Zoned<sz, precision> z;
	z.assign(fc);
	fromZoned(z);
}

template <int sz, int precision>
inline Zoned<sz, precision> Packed<sz, precision>::toZoned() const
{
#if !defined(NO_PACKED)
	Zoned<sz, precision> z;
	cpynv(NUM_DESCR(_T_ZONED, sz, precision), z.overlay,
	 NUM_DESCR(_T_PACKED, sz, precision), (char *)&overlay);
	return z;
#else
	return overlay;
#endif
}

template <int sz, int precision>
inline void Packed<sz, precision>::fromZoned(const Zoned<sz, precision> &z)
{
#if !defined(NO_PACKED)
	cpynv(NUM_DESCR(_T_PACKED, sz, precision), &overlay,
	 NUM_DESCR(_T_ZONED, sz, precision), (_SPCPTRCN)z.overlay);
#else
	overlay = z;
#endif
}

static_assert(sizeof(Packed<9, 2>) == sizeof(Packed<9, 2>::Backing),
	"Packed<> must stay layout-compatible with its backing type so it can overlay a record buffer");

/// @brief Stream a Packed as exact decimal text -- see operator<<(ostream, Zoned).
template <class charT, class traits, int sz, int precision>
inline std::basic_ostream<charT, traits> &
 operator << (std::basic_ostream<charT, traits> &os, const Packed<sz, precision> &pkd)
{
	return os << pkd.toZoned();
}

// Arithmetic and comparison. On IBM i these forward to the native packed type so the
// arithmetic stays exact decimal (and picks up bcd.h's RPG-style intermediate result
// widening); elsewhere Packed is zoned-backed and forwards to Zoned's operators.
#if !defined(NO_PACKED)
# define GEN_PACKED_OP(OP, RTNTYP) \
	template <int l1, int p1, int l2, int p2> \
	 inline RTNTYP operator OP (const Packed<l1, p1> &val1, const Packed<l2, p2> &val2) \
	  { return val1.overlay OP val2.overlay; } \
	template <int l1, int p1, int l2, int p2> \
	 inline RTNTYP operator OP (const Packed<l1, p1> &val1, const Zoned<l2, p2> &val2) \
	  { return val1.overlay OP val2.toPacked(); } \
	template <int l1, int p1, int l2, int p2> \
	 inline RTNTYP operator OP (const Zoned<l1, p1> &val1, const Packed<l2, p2> &val2) \
	  { return val1.toPacked() OP val2.overlay; }
	GEN_PACKED_OP(==, bool)
	GEN_PACKED_OP(!=, bool)
	GEN_PACKED_OP(<=, bool)
	GEN_PACKED_OP(<, bool)
	GEN_PACKED_OP(>=, bool)
	GEN_PACKED_OP(>, bool)
	GEN_PACKED_OP(+, _ConvertDecimal)
	GEN_PACKED_OP(-, _ConvertDecimal)
	GEN_PACKED_OP(*, _ConvertDecimal)
	GEN_PACKED_OP(/, _ConvertDecimal)
#else
# define GEN_PACKED_OP(OP, RTNTYP) \
	template <int l1, int p1, int l2, int p2> \
	 inline RTNTYP operator OP (const Packed<l1, p1> &val1, const Packed<l2, p2> &val2) \
	  { return val1.overlay OP val2.overlay; } \
	template <int l1, int p1, int l2, int p2> \
	 inline RTNTYP operator OP (const Packed<l1, p1> &val1, const Zoned<l2, p2> &val2) \
	  { return val1.overlay OP val2; } \
	template <int l1, int p1, int l2, int p2> \
	 inline RTNTYP operator OP (const Zoned<l1, p1> &val1, const Packed<l2, p2> &val2) \
	  { return val1 OP val2.overlay; }
	GEN_PACKED_OP(==, bool)
	GEN_PACKED_OP(!=, bool)
	GEN_PACKED_OP(<=, bool)
	GEN_PACKED_OP(<, bool)
	GEN_PACKED_OP(>=, bool)
	GEN_PACKED_OP(>, bool)
	GEN_PACKED_OP(+, long double)
	GEN_PACKED_OP(-, long double)
	GEN_PACKED_OP(*, long double)
	GEN_PACKED_OP(/, long double)
#endif

template<int sz, int precision>
 inline bool operator == (const Packed<sz, precision> &val1, const FigConstNum &val2)
	{ return val1.toZoned() == (Zoned<sz, precision>)val2; }
template<int sz, int precision>
 inline bool operator != (const Packed<sz, precision> &val1, const FigConstNum &val2)
	{ return val1.toZoned() != (Zoned<sz, precision>)val2; }

#define packed(sz, precision) Packed<sz, precision>

////////////////////////////////////////////////////////////////////////////////
// Comparing a decimal field directly against a floating-point literal is deleted.
// Most decimal values have no exact binary representation, so `z == 32.1` asks whether a
// decimal equals an approximation of 32.1 -- which is rarely the intent and fails for
// reasons that are hard to see. Say what you mean instead:
//     (double)z == 32.1     compare as floating point
//     z == __D("32.1")      compare exactly, digit for digit
// Assignment and conversion are unaffected: `double d = z;` and `z = 3.14;` still work,
// the latter storing the written digits (see QXXDTOZ).
//
// The integer overloads are not optional. Deleting only the floating-point ones would
// capture `z == 42` as well, because int converts to double by a standard conversion while
// the built-in candidate needs a user-defined conversion on the decimal -- so the deleted
// overload would win. Declaring the integer forms explicitly keeps them exact matches.
// Integers convert to decimal without losing decimal precision, which is why they stay.
#define GEN_DECIMAL_CMP_DELETED(TYPE, OP, FLT) \
	template <int sz, int precision> \
	 bool operator OP (const TYPE<sz, precision> &, FLT) = delete; \
	template <int sz, int precision> \
	 bool operator OP (FLT, const TYPE<sz, precision> &) = delete;
#define GEN_DECIMAL_CMP_INT(TYPE, OP, INT) \
	template <int sz, int precision> \
	 inline bool operator OP (const TYPE<sz, precision> &val1, INT val2) \
	  { return (long double)val1 OP (long double)val2; } \
	template <int sz, int precision> \
	 inline bool operator OP (INT val1, const TYPE<sz, precision> &val2) \
	  { return (long double)val1 OP (long double)val2; }
#define GEN_DECIMAL_CMP(TYPE, OP) \
	GEN_DECIMAL_CMP_DELETED(TYPE, OP, float) \
	GEN_DECIMAL_CMP_DELETED(TYPE, OP, double) \
	GEN_DECIMAL_CMP_DELETED(TYPE, OP, long double) \
	GEN_DECIMAL_CMP_INT(TYPE, OP, short) \
	GEN_DECIMAL_CMP_INT(TYPE, OP, int) \
	GEN_DECIMAL_CMP_INT(TYPE, OP, long)
#define GEN_DECIMAL_CMPS(TYPE) \
	GEN_DECIMAL_CMP(TYPE, ==) \
	GEN_DECIMAL_CMP(TYPE, !=) \
	GEN_DECIMAL_CMP(TYPE, <=) \
	GEN_DECIMAL_CMP(TYPE, <) \
	GEN_DECIMAL_CMP(TYPE, >=) \
	GEN_DECIMAL_CMP(TYPE, >)

GEN_DECIMAL_CMPS(Zoned)
GEN_DECIMAL_CMPS(Packed)

#ifndef NO_PACKED
////////////////////////////////////////////////////////////////////////////////
/// @brief Generic numeric class
template <int sz, int precision> class Numeric
{
public:
	Numeric(_DecimalT<sz, precision> d) { overlay=d; }
	Numeric(Zoned<sz, precision> z) { overlay=z; }
	Numeric(short s) { overlay=s; }
	Numeric(int i) { overlay=i; }
	Numeric(long l) { overlay=l; }
	Numeric(float f) { overlay=f; }
	Numeric(double d) { overlay=d; }
	Numeric(long double ld) { overlay=ld; }
	Numeric(FigConstNum fc) { overlay=(zoned(sz,precision))fc; }
	operator _ConvertDecimal()
	 { return overlay.operator _ConvertDecimal(); }
public:
	packed(sz, precision) overlay;
};
# define numeric(sz,precision) Numeric<sz, precision>
#else
# define numeric(sz,precision) Zoned<sz, precision>
#endif

////////////////////////////////////////////////////////////////////////////////
/// @brief The [User Date Special Words](https://www.ibm.com/docs/en/i/7.4.0?topic=words-user-date-special)
// U/*DATE, U/*YEAR, and U/*MONTH
template <int sz> class FmtDate : public Zoned<sz, 0>
{
public:
	FmtDate(const char *fmt)
	{
		char *jfmt;
		if (*fmt=='*')
		{
			if (strcmp(fmt, "*YMD")==0)
				jfmt=(char*)"%y%m%d";
			else if (strcmp(fmt, "*DMY")==0)
				jfmt=(char*)"%d%m%y";
			else
				jfmt=(char*)"%m%d%y";
			static int i=0;
			if (sz+i>=8)
			{
				char jbuf[7];
				strcpy(jbuf, jfmt);
				*(strchr(jbuf, 'y'))='Y';
				jfmt=jbuf;
			}
		}
		else
			jfmt=(char *)fmt;

		time_t timet;
		timet=time(NULL);
		char overlay0[sz+1];
		strftime(overlay0, sz+1, jfmt, localtime(&timet));
		memcpy(Zoned<sz,0>::overlay, overlay0, sz);
	}
};
extern I2CLASS_API Zoned<4,0>	&YEAR;
extern I2CLASS_API Zoned<2,0>	&UYEAR;
extern I2CLASS_API Zoned<2,0>	&MONTH;
extern I2CLASS_API Zoned<2,0>	&DAY;

////////////////////////////////////////////////////////////////////////////////
/// @brief A [Data area](https://www.ibm.com/docs/en/i/7.4.0?topic=procedures-using-data-areas) helper class
struct DtaaName {
	Fixed<10>	dtaa_name;
	Fixed<10>	dtaa_lib;
};
#include "xxdtaa.h"
template <class T> class Dtaara : public T
{
public:
	Dtaara(char *name) {dtaaraName=name; }
	// Retrieve the content of a Dtaara
	void in()
		{QXXRTVDA(getLocation(),0,sizeof(*this)-sizeof(dtaaraName),(char *)this);}
	void out()
		{QXXCHGDA(getLocation(),0,sizeof(*this)-sizeof(dtaaraName),(char *)this);}
private:
	_DTAA_NAME_T getLocation()
	{
		DtaaName location;
		location.dtaa_name=dtaaraName;
		if (*dtaaraName!='*')
			location.dtaa_lib="*LIBL";
		_DTAA_NAME_T loc;
		memcpy(&loc, &location, sizeof(loc));
		return loc;
	}
private:
	char	*dtaaraName;
};

/// @brief Get Number of Elements [%ELEM](https://www.ibm.com/docs/en/i/7.4.0?topic=functions-elem-get-number-elements)
/// built-in function
#define ELEM(a) (int)(sizeof(a)/sizeof(a[0]))

// Do a (shell) sort for sorta
template<class T> void sorta(T *ary, int aryCnt)
{
	for (int gap=aryCnt/2; 0<gap; gap/=2)
		for (int i=gap; i<aryCnt; i++)
			for (int j=i-gap; 0<=j; j-=gap)
				if (ary[j+gap]<ary[j])
				{
					T temp = ary[j];
					ary[j]=ary[j+gap];
					ary[j+gap]=temp;
				}
}
/// @brief Sort array [SORTA](https://www.ibm.com/docs/en/i/7.4.0?topic=codes-sorta-sort-array) support
#define SORTA(a) sorta(a, ELEM(a))

// Sum all of the element in the array
template<class T> long double xfoot(T *ary, int aryCnt)
{
	long double sum=0;
	for (int i=0; i<aryCnt; i++)
		sum = sum + ary[i];
	return sum;
}
/// @brief Sum array elements [%XFOOT](https://www.ibm.com/docs/en/i/7.4.0?topic=functions-xfoot-sum-array-expression-elements#bbxfoot)
/// built-in function
#define XFOOT(a) xfoot(a, ELEM(a))

////////////////////////////////////////////////////////////////////////////////
// Look up a value in an array
enum lookupCompare {EQ, GE, GT, LE, LT};
// Look up a value in an array
template<class T, class A>
 int lookupxx(T arg, A *ary, int startIndex, int numElems, lookupCompare compare)
{
	for (int j=0; j<numElems; j++)
	{
		bool bcomp;
		if (compare==EQ)
			bcomp=(ary[startIndex-1]==arg);
		else if (compare==GT)
			bcomp=(ary[startIndex-1]>arg);
		else if (compare==LT)
			bcomp=(ary[startIndex-1]<arg);
		else if (compare==GE)
			bcomp=(ary[startIndex-1]>=arg);
		else //if (compare==LE)
			bcomp=(ary[startIndex-1]<=arg);
		if (bcomp)
			//return '1';
			return startIndex;
		startIndex++;
	}
	//startIndex=1;
	//return '0';
	return 0;
}
template<class T, class A>
 int lookup(T arg, A *ary, int startIndex, int numElems)
	{ return lookupxx(arg, ary, startIndex, numElems, EQ); }
template<class T, class A>
 int lookupgt(T arg, A *ary, int startIndex, int numElems)
	{ return lookupxx(arg, ary, startIndex, numElems, GT); }
template<class T, class A>
 int lookupge(T arg, A *ary, int startIndex, int numElems)
	{ return lookupxx(arg, ary, startIndex, numElems, GE); }
template<class T, class A>
 int lookuplt(T arg, A *ary, int startIndex, int numElems)
	{ return lookupxx(arg, ary, startIndex, numElems, LT); }
template<class T, class A>
 int lookuple(T arg, A *ary, int startIndex, int numElems)
	{ return lookupxx(arg, ary, startIndex, numElems, LE); }

template<int sz>
 int lookup(char arg, const Fixed<sz> &ary, int startIndex, int numElems)
	{ return lookupxx(arg, ary.overlay, startIndex, numElems, EQ); }
template<int sz>
 int lookupgt(char arg, const Fixed<sz> &ary, int startIndex, int numElems)
	{ return lookupxx(arg, ary.overlay, startIndex, numElems, GT); }
template<int sz>
 int lookupge(char arg, const Fixed<sz> &ary, int startIndex, int numElems)
	{ return lookupxx(arg, ary.overlay, startIndex, numElems, GE); }
template<int sz>
 int lookuplt(char arg, const Fixed<sz> &ary, int startIndex, int numElems)
	{ return lookupxx(arg, ary.overlay, startIndex, numElems, LT); }
template<int sz>
 int lookuple(char arg, const Fixed<sz> &ary, int startIndex, int numElems)
	{ return lookupxx(arg, ary.overlay, startIndex, numElems, LE); }

/// @brief Lookup values in an array [%LOOKUP](https://www.ibm.com/docs/en/i/7.4.0?topic=functions-lookupxx-look-up-array-element) built-in function
#define LOOKUP(arg, ary, s) lookup(arg, ary, (int)s, (int)(ELEM(ary)-s+1))
#define LOOKUPGT(arg, ary, s) lookupgt(arg, ary, (int)s, (int)(ELEM(ary)-s+1))
#define LOOKUPGE(arg, ary, s) lookupge(arg, ary, (int)s, (int)(ELEM(ary)-s+1))
#define LOOKUPLT(arg, ary, s) lookuplt(arg, ary, (int)s, (int)(ELEM(ary)-s+1))
#define LOOKUPLE(arg, ary, s) lookuple(arg, ary, (int)s, (int)(ELEM(ary)-s+1))

////////////////////////////////////////////////////////////////////////////////
/// @brief Generic array class for RPG-style handling of arrays
template <class C, int sz> class Array
{
public:
	C &operator[] (const int i)
		{ return elem[i]; }
	void sorta()
		{ ::sorta(elem, sz); }
	/*long double xfoot() const
		{ return ::xfoot(elem, sz); }*/
#ifndef NO_FUNCTION_TEMPLATE
	// Element count is measured from startIndex, matching the LOOKUP() macro
	template<class T> int lookup(T arg, int startIndex=1)
		{ return lookupxx(arg, elem, startIndex, sz-startIndex+1, EQ); }
	template<class T> int lookup(T arg, int startIndex, int numElems)
		{ return lookupxx(arg, elem, startIndex, numElems, EQ); }
#endif
private:
	C elem[sz];
};

/// @brief Move (copy) values to an array [MOVEA](https://www.ibm.com/docs/en/i/7.4.0?topic=codes-movea-move-array) support
/// @param a1 the array to move values from
/// @param i1 the index of the from-array to begin moving values from
/// @param a2 the array to move values to
/// @param i2 the index of the to-array to move values to
#define MOVEA(a1, i1, a2, i2) \
	{	int j=i2-1; \
		for (int i=i1-1; i<ELEM(a1) && j<ELEM(a2); i++) { \
			a2[j]=a1[i]; \
			j++; \
		} \
	}
/// @brief Move all values to an array [MOVEA](https://www.ibm.com/docs/en/i/7.4.0?topic=codes-movea-move-array) support
/// @param the value to copy
/// @param a1 the array to move values to
/// @param i1 the index of the to-array to move values to
#define MOVEALL(c, a1, i1) \
	{	for (int i=i1-1; i<ELEM(a1); i++) \
			a1[i]=c; \
	}

////////////////////////////////////////////////////////////////////////////////
// TIME, UTIMESTAMP, and TIMESTAMP support
int time6();
long double time12();
long double time14();

class Time6
{
public:
	// Cast to an int
	operator int()
		{return time6(); }
#if !defined(NO_PACKED)
	operator _ConvertDecimal()
		{ return (_DecimalT<6,0>)time6(); }
#else
	operator long double()
		{ return (long double)time6(); }
#endif
};
class Time12
{
public:
	operator long double()
		{return time12(); }
#if !defined(NO_PACKED)
	operator _ConvertDecimal()
		{ return (_DecimalT<12,0>)time12(); }
#endif
};
class Time14
{
public:
	operator long double()
		{return time14(); }
#if !defined(NO_PACKED)
	operator _ConvertDecimal()
		{ return (_DecimalT<14,0>)time14(); }
#endif
};
extern I2CLASS_API Time6 TIME;
extern I2CLASS_API Time12 UTIMESTAMP;
extern I2CLASS_API Time14 TIMESTAMP;

////////////////////////////////////////////////////////////////////////////////
// Global functions
int testn(char c);
int testn(const FixedTemp &fStr);
int testz(char c);
int testz(const FixedTemp &fStr);

char move(const FixedTemp &source, char &c);
char move(const _ConvertDecimal &source, char &c);
char movel(const FixedTemp &source, char &c);
char movel(const _ConvertDecimal &source, char &c);
short move(const FixedTemp &source, short &dest);
short move(const _ConvertDecimal &source, short &dest);
int move(const FixedTemp &source, int &dest);
int move(const _ConvertDecimal &source, int &dest);

#ifndef NO_PACKED
template <int sz, int precision>
 _DecimalT<sz,precision> move(const FixedTemp &source, _DecimalT<sz,precision> &dest)
{
	Zoned<sz,precision> znd(dest);
   znd.move(source);
   dest=znd;
   return dest;
}
template <int sz, int precision>
 _DecimalT<sz, precision> move(const _ConvertDecimal &source, _DecimalT<sz, precision> &dest)
{
	Zoned<sz, precision> znd(dest);
   znd.movel(source);
   dest=znd;
   return dest;
}

#endif // NO_PACKED
#endif // RPORT_H




