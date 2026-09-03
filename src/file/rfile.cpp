#include "rfile.h"

/// @file rfile.cpp
/// @brief Base record-level file I/O and RPG-style printer output formatting.
///
/// Rfile is the common base for IBM i "record level access" files (physical/
/// logical database files and device files like printer/display), analogous
/// to RPG's F-spec file declarations. RrecordPrint additionally implements
/// RPG's O-spec (output spec) printer formatting rules: "edit words" (a
/// template string of digit positions, zero-suppression, commas, currency
/// symbols, etc., as typed literally on an O-spec) and "edit codes" (single
/// letters/digits like '1','J','Y' that RPG expands into an equivalent edit
/// word automatically - e.g. inserting commas, a floating '$', or a trailing
/// 'CR' for negative values).

Rfile::Rfile(const AS400 &as400, czstring sFileName)
 : error(false), found(false), eof(false), record(nullptr)
{
	fileName = sFileName;
	//strncpy(fileName, sFileName, sizeof(fileName));
	server = as400.url;
   password = as400.password;
   usrid = as400.usrid;
	//strncpy(server, as400.url, sizeof(server));
}
/*
Rfile::Rfile(const AS400 &as400, char *sFileName, Rrecord &format)
{
	Rfile(as400, sFileName);
	setRecord(format);
}
*/
Rfile::~Rfile()
{
	close();
}

void Rfile::setRecord(Rrecord &format)
{
	record = &format;
}

////////////////////////////////////////////////////////////////////////////////
// Print file

/// @brief Apply an RPG-style [edit word](https://www.ibm.com/docs/en/i/7.4.0?topic=name-edit-words#opfedit) 
/// to a numeric value, producing the formatted digits/punctuation RPG would print for an O-spec field.
///
/// An edit word is a template string with special characters: '0'/'*' mark
/// zero-suppressed/fill positions, ' ' marks digit positions, '$' marks a
/// (possibly floating) currency symbol, '.' the decimal point, ',' a comma
/// insertion point, '-'/"CR" trailing sign indicators. This walks the edit
/// word right-to-left, consuming digits from the source value as it goes.
/// @param buf destination buffer (same length as edtWrd) to receive formatted output
/// @param n the numeric (zoned/packed) value being formatted
/// @param edtWrd the edit word template, as it would appear on an RPG O-spec
void RrecordPrint::edit(zstring buf, const _ConvertDecimal &n, czstring edtWrd)
{
	// Make a pass forwards through the edit word to accumulate information...
	int wrdI=static_cast<int>(strlen(edtWrd));
	char	fillChar=' ';
	// -1 means "no floating currency symbol"; a '$' in the leftmost position is a
	// fixed symbol that prints in place, so it deliberately never sets this.
	int currency=-1, decimal=0, precision=0;
	int lastBlank=-1000; // no blank/zero-suppress position seen yet; keeps wrdI-lastBlank from
	                      // ever accidentally matching 1 or 2 below if edtWrd has none
	int zeroSuppress=wrdI;
	for (int i=0; i<wrdI; i++)
	{
		switch (edtWrd[i])
		{
		case '0':
		case '*':
			if (i>zeroSuppress)
				break;
			zeroSuppress=i;
			if (edtWrd[i]=='*')
				fillChar='*';
			// intentionally fall through here and count the zero suppress character
		case ' ':
			lastBlank=i;
			if (decimal>0)
				precision++;
			break;
		case '$':
			if (i>0)
				currency=i;
			break;
		case '.':
			decimal=i;
		}
	}
	int nlen=n.DigitsOf();
	if (precision==0)
		precision = nlen;

	// Start from the back of the converted string and start copying in digits
	/*
	char *str;
	str=n.overlay;
	*/
	char strBuf[MAX_DECIMAL_DIGITS];
	char *str=strBuf;
#if !defined(NO_PACKED) 
	cpynv(NUM_DESCR(_T_ZONED, n.DigitsOf(), n.PrecisionOf()), str, NUM_DESCR(_T_PACKED, n.DigitsOf(), n.PrecisionOf()), (char *)&n);
#else
	memcpy(strBuf, n.overlay, n.DigitsOf());
#endif
	bool negative=(isdigit(str[nlen-1]) == 0);
	if (negative)
		decodeSign(reinterpret_cast<byte_ptr>(str+nlen-1));
	// Loop past any leading zeros
	int j=0;
	for ( ; j<nlen-n.PrecisionOf(); j++)
	{
		if (*str=='0')
			str++;
		else
			break;
	}
	int strI=nlen-j;
	bool currencyPlaced=false;
	// Edit codes N-Q put the minus in front, and the reference shows it hugging
	// the first significant digit (-.125, not "-     .125"), so a leading '-'
	// floats the same way a currency symbol does. With a currency symbol present
	// both would want the same slot, so there it stays put.
	bool floatMinus = negative && edtWrd[0]=='-' && currency<0;
	bool minusPlaced=false;
	while (wrdI>0)
	{
		wrdI--;
		switch (edtWrd[wrdI])
		{
		case '&':
			buf[wrdI]=' ';
			break;
		// Handle CR (credit)
		case 'R':
			if (wrdI>0 && edtWrd[wrdI-1]=='C' && (wrdI-lastBlank==2 || wrdI==1))
			{
				wrdI--;
				if (negative)
					memcpy(buf+wrdI, "CR", 2);
				break;
			}
			else
				goto dft;
		case '-':
			if (wrdI==0 && floatMinus)
			{
				// Placed against the digits below unless they reached this far, in
				// which case there was nowhere to float to and it belongs here.
				if (!minusPlaced)
					buf[wrdI]='-';
				break;
			}
			if (wrdI-lastBlank==1 || wrdI==0)
			{
				if (negative)
					buf[wrdI]='-';
			}
			break;
		case '$':
			if (wrdI!=currency)
				goto dft;
		case '0':
		case '*':
			if (wrdI>zeroSuppress)
				goto dft;
			// fall through here intentionally
		case ' ':
			strI--;
			if (strI>=0 && (str[strI]!='0' || wrdI<=zeroSuppress))
				buf[wrdI]=str[strI];
			// wrdI==zeroSuppress is the '0'/'*' marker itself -- it must always show a
			// digit (typically '0' for an all-zero value), not fall through to blank.
			// Asterisk protection is the exception: a zero balance is a full field of
			// asterisks, so the marker takes the fill instead.
			else if (wrdI>=zeroSuppress && fillChar!='*')
				buf[wrdI]='0';
			// Insert floating currency symbol. strI==-1 is the position just left of
			// the leftmost digit; wrdI==currency is the fallback for when that position
			// was already claimed (an all-zero value takes it for the '0' marker).
			// Only ever one symbol, whichever comes first walking right to left.
			else if (currency>=0 && !currencyPlaced && (strI==-1 || wrdI==currency))
			{
				buf[wrdI]='$';
				currencyPlaced=true;
			}
			else if (floatMinus && !minusPlaced && strI==-1)
			{
				buf[wrdI]='-';
				minusPlaced=true;
			}
			else
				buf[wrdI]=fillChar;
			break;
		case ',':
			if (strI<=0 && wrdI<zeroSuppress)
			{
				buf[wrdI]=fillChar;
				break;
			} // Fall through here intentionally
		default:
		dft:
			buf[wrdI]=edtWrd[wrdI];
		}
	}
}

std::string EDITWRD(const _ConvertDecimal &value, czstring editWord)
{
	std::string result(strlen(editWord), ' ');
	RrecordPrint::edit(&result[0], value, editWord);
	return result;
}

/// @brief Build the edit word that an RPG edit code stands for.
/// @param buf destination, must hold at least 45 characters
/// @param edtCde the single-character RPG edit code
/// @param fillChar zero-suppression fill override ('*' for check protection, a
///        currency symbol to float it, ' ' for none)
/// @param nlen total digits in the field
/// @param precision digits to the right of the decimal point
///
/// Shared by EDITC() and RrecordPrint::print(n, col, edtCde, fillChar) so the
/// two cannot drift apart.
static void editCodeToWord(zstring buf, char edtCde, char fillChar, int nlen, int precision)
{
	if (edtCde=='Y')
	{
		if (nlen==7)
			strcpy(buf, " 0 /  /  ");
		else
		{
			strcpy(buf, "0 /  /    ");
			if (nlen<8)
				buf[nlen+(nlen-1)/2]='\0';
		}
		return;
	}
	if (edtCde=='W')
	{
		if (nlen==5)
			strcpy(buf, "0 /   ");
		else if (nlen==7)
			strcpy(buf, "  0 /  ");
		else
		{
			strcpy(buf, "  0 /  /  ");
			if (nlen<8)
			{
				int i=nlen;
				if (i>2)
					i=i-2;
				buf[nlen+(i-1)/2]='\0';
			}
		}
		return;
	}

	char *out=buf;
	if (edtCde>='N' && edtCde<='Q')
		*out++='-';
	// A currency symbol reserves the leftmost position and is not a digit
	// position, so it cannot displace the comma groups. Asterisk protection
	// works differently -- it replaces the zero-suppression stop, below.
	if (fillChar!=' ' && fillChar!='*')
		*out++=fillChar;
	const char *comma=strchr("12ABJKNO", edtCde);
	// Where the digit positions start, so the leading group can be told apart
	// from the rest once the prefixes above have been written.
	char *digitsStart=out;
	int scale=nlen-precision;
	int scale3=scale%3;
	if (scale3>0)
	{
		memset(out, ' ', scale3);
		out+=scale3;
		scale-=scale3;
	}
	while (scale>0)
	{
		// A separator goes *between* groups of three, never before the first
		// one -- an exact multiple of 3 has no partial leading group to absorb it.
		if (comma && out>digitsStart)
			*out++=',';
		memset(out, ' ', 3);
		out+=3;
		scale-=3;
	}
	if (out>digitsStart)
	{
		if (fillChar=='*')
			out[-1]='*';
		else if (fillChar!=' ')
			out[-1]='0';
		else if (precision==0 && strchr("13ACJLNP", edtCde)!=NULL)
			// Codes that show a zero balance need a stop-zero-suppression
			// character at the units digit, or an all-zero value edits to
			// blanks. With decimals the fraction digits already force ".00".
			out[-1]='0';
	}
	if (precision>0)
	{
		*out++='.';
		memset(out, ' ', precision);
		out+=precision;
	}
	if (edtCde>='A' && edtCde<='D')
		strcpy(out, "CR");
	else
	{
		if (edtCde>='J' && edtCde<='M')
			*out++='-';
		*out='\0';
	}
}

std::string EDITC(const _ConvertDecimal &value, char editCode, char fillChar)
{
	int nlen=value.DigitsOf();
	int precision=value.PrecisionOf();
	if (editCode=='X')
		return EDITWRD(value, std::string(nlen, ' ').c_str());
	if (value==0 && strchr("2BKO4DMQZ", editCode)!=NULL)
		return std::string(nlen, ' ');

	char editWord[45];
	editCodeToWord(editWord, editCode, fillChar, nlen, precision);
	return EDITWRD(value, editWord);
}

// Print out a character value
void RrecordPrint::printChar(const_byte_ptr str, int edtLen, int col)
{
	setMaxColumn(col);

	if (edtLen>col)
		edtLen=col;
	// See if the positions in the record alrady have characters.  If they do,
	// then write out record.
	char *bufChar=outputBuffer+col-edtLen+1;
//	if (strncmp(bufChar, BLANKS, edtLen)!=0)
//	if (strncmp(bufChar, "                                ", edtLen)!=0)
//		flush();

	memcpy(bufChar, str, edtLen);
	column++;
}
void RrecordPrint::print(const FixedTemp &f, int col)
	{ printChar(reinterpret_cast<const_byte_ptr>(f.overlay), f.len(), col); }
void RrecordPrint::print(czstring str, int col)
	{ printChar(reinterpret_cast<const_byte_ptr>(str), static_cast<int>(strlen(str)), col); }

void RrecordPrint::printl(const FixedTemp &f, int col)
{
	if (col<=0)
		col=column;
	print(f, col+f.len()-1);
}
void RrecordPrint::printl(czstring str, int col)
{
	FixedTemp cStr((char *)str, static_cast<int>(strlen(str)));
	printl(cStr, col);
}

// Print out a numeric value
/*
void RrecordPrint::print(int i, int col, const char *edtWrd)
{
	Zoned<9,0> dec(i);
   print ((_ConvertDecimal)dec, col, edtWrd);
}
*/
/// @brief Print a numeric field at a given column, optionally applying an
/// explicit RPG edit word (see edit()); with no edit word the raw digits are used.
void RrecordPrint::print(const _ConvertDecimal &n, int col, czstring edtWrd)
{
	setMaxColumn(col);

	int edtLen;
	if (edtWrd != NULL)
		edtLen=static_cast<int>(strlen(edtWrd));
	else
		//edtLen = n.len();
		edtLen = n.DigitsOf();
	if (edtLen>col)
		edtLen=col;
	// See if the positions in the record alrady have characters.  If they do,
	// then write out record.
	char *bufNum=outputBuffer+col-edtLen+1;
	//if (strncmp(bufNum, BLANKS, edtLen)!=0)
	//if (strncmp(bufNum, "                                ", edtLen)!=0)
	//	flush();

	if (edtWrd != NULL)
		edit(bufNum, n, edtWrd);
	else
	{
		/*
		char *str;
		str=zonedToChar(n.overlay, n.len(), n.DigitsOf());
		int strLen=strlen(str);
		int i=edtLen-strLen;
		if (i>0) {
			memset(bufNum, '0', i);
			bufNum += i;
		}
		*/
#if !defined(NO_PACKED)
		cpynv(NUM_DESCR(_T_ZONED, n.DigitsOf(), n.PrecisionOf()), bufNum, NUM_DESCR(_T_PACKED, n.DigitsOf(), n.PrecisionOf()), (char *)&n);
#else
		memcpy(bufNum, n.overlay, edtLen);
#endif
	}
	column++;
}
void RrecordPrint::printl(const _ConvertDecimal &n, int col, czstring edtWrd)
{
	if (col<=0)
		col=column;
	//print(n, col+n.len()-1, edtWrd);
	int edtLen;
	if (edtWrd != NULL)
		edtLen=static_cast<int>(strlen(edtWrd));
	else
		edtLen = n.DigitsOf();
	print(n, col+edtLen-1, edtWrd);
}

// Print out a numeric value with an edit code
/*
void RrecordPrint::print(int i, int col, char edtCde, char fillChar)
{
	Zoned<9,0> dec(i);
   print ((_ConvertDecimal)dec, col, edtCde, fillChar);
}
*/
/// @brief Print a numeric field using an RPG [edit code](https://www.ibm.com/docs/en/i/7.4.0?topic=fields-edit-codes) 
/// (O-spec shorthand for a common edit word) instead of a literal edit word.
///
/// Mirrors the standard RPG edit codes: '1'/'2'/'A'/'B' insert commas,
/// 'J'-'Q' add sign handling, 'A'-'D' append trailing "CR" for negative
/// values, 'Y'/'W' format the value as a slash-separated date, and 'X' means
/// "no editing" (print raw digits). Values that are zero and use a
/// zero-suppressing code (per RPG rules) are skipped/left blank, matching
/// native RPG O-spec behavior.
/// @param n the numeric value to print
/// @param col rightmost print column
/// @param edtCde the single-character RPG edit code
/// @param fillChar zero-suppression fill character override ('*' for check-protection, etc.)
void RrecordPrint::print(const _ConvertDecimal &n, int col, char edtCde, char fillChar)
{
	// Edtcde(X) means no editing
	if (edtCde=='X')
		print(n, col);
	else
	{
		// If the value of n is 0, then skip this entirely for edit codes that
		// don't print a zero balance.
		if (n !=0 || strchr("2BKO4DMQZ", edtCde)==NULL)
		{
			char edtWrd[45];
			editCodeToWord(edtWrd, edtCde, fillChar, n.DigitsOf(), n.PrecisionOf());
			print(n, col, edtWrd);
		}
	}
}

//void RrecordPrint::printl(const ZonedTemp &n, int col, char edtCde, char fillChar)
void RrecordPrint::printl(const _ConvertDecimal &n, int col, char edtCde, char fillChar)
{
	if (col<=0)
		col=column;
	//print(n, col+n.len()-1, edtCde, fillChar);
	print(n, col+n.DigitsOf()-1, edtCde, fillChar);
}

// Keep track of the right-most column written to
void RrecordPrint::setMaxColumn(int &col)
{
	if (col<=0)
		col = column;
	else {
		column = col;
		if (column > maxColumn)
			maxColumn = column;
	}
}



