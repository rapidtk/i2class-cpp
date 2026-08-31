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

Rfile::Rfile(const AS400 &as400, char *sFileName)
{
	fileName = (char *)sFileName;
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
void RrecordPrint::edit(char *buf, const _ConvertDecimal &n, const char *edtWrd)
{
	// Make a pass forwards through the edit word to accumulate information...
	int wrdI=static_cast<int>(strlen(edtWrd));
	char	fillChar=' ';
	int currency=0, decimal=0, precision=0;
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
	char strBuf[31];
	char *str=strBuf;
#if !defined(NO_PACKED) 
	cpynv(NUM_DESCR(_T_ZONED, n.DigitsOf(), n.PrecisionOf()), str, NUM_DESCR(_T_PACKED, n.DigitsOf(), n.PrecisionOf()), (char *)&n);
#else
	memcpy(strBuf, n.overlay, n.DigitsOf());
#endif
	bool negative=(isdigit(str[nlen-1]) == 0);
	if (negative)
		decodeSign(str+nlen-1);
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
			else if (wrdI>=zeroSuppress)
				buf[wrdI]='0';
			// Insert floating currency symbol
			else if (currency>0 && (strI==-1 || (strI<0 && wrdI==currency)))
				buf[wrdI]='$';
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

// Print out a character value
void RrecordPrint::printChar(const char *str, int edtLen, int col)
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
	{ printChar(f.overlay, f.len(), col); }
void RrecordPrint::print(const char *str, int col)
	{ printChar(str, static_cast<int>(strlen(str)), col); }

void RrecordPrint::printl(const FixedTemp &f, int col)
{
	if (col<=0)
		col=column;
	print(f, col+f.len()-1);
}
void RrecordPrint::printl(const char *str, int col)
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
void RrecordPrint::print(const _ConvertDecimal &n, int col, const char *edtWrd)
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
void RrecordPrint::printl(const _ConvertDecimal &n, int col, const char *edtWrd)
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
		//int nlen=n.len();
		int nlen=n.DigitsOf();
		int precision=n.PrecisionOf();
		//if (QXXZTOD((unsigned char *)n.overlay, nlen, precision) !=0 ||
		if (n !=0 || strchr("2BKO4DMQZ", edtCde)==NULL)
		{
			// Generate appropriate edit word from edit code
			char edtWrd[45];
			// Generate 'W'/'Y' date edit code
			if (edtCde=='Y')
			{
				if (nlen==7)
					strcpy(edtWrd, " 0 /  /  ");
				else
				{
					strcpy(edtWrd, "0 /  /    ");
					if (nlen<8)
						edtWrd[nlen+(nlen-1)/2]='\0';
				}
			}
			else if (edtCde=='W')
			{
				if (nlen==5)
					strcpy(edtWrd, "0 /   ");
				else if (nlen==7)
					strcpy(edtWrd, "  0 /  ");
				else
				{
					strcpy(edtWrd, "  0 /  /  ");
					if (nlen<8)
					{
						int i=nlen;
						if (i>2)
							i=i-2;
						edtWrd[nlen+(i-1)/2]='\0';
					}
				}
			}
			else {
				char *e=edtWrd;
				// Add leading minus
				if (edtCde>='N' && edtCde<='Q')
				{
					edtWrd[0]='-';
					e++;
				}
				// Figure out if commas are needed
			const char *comma;
				comma=strchr("12ABJKNO", edtCde);
				int scale=nlen-precision;
				int scale3=scale%3;
				if (scale3 > 0)
				{
					memset(e, ' ', scale3);
					e += scale3;
					scale -= scale3;
				}
				// If the length is >3, print out comma-separated 3 letter chunks
				while (scale>0)
				{
					if (comma) {
						*e=',';
						e++;
					}
					memset(e, ' ', 3);
					e += 3;
					scale -= 3;
				}
				// Add asterisk or floating currency symbol
				if (fillChar != ' ')
				{
					if (e>edtWrd)
						e--;
					*e=fillChar;
					e++;
					if (fillChar != '*')
					{
						*e='0';
						e++;
					}
				}
				// Add decimal point if needed
				if (precision>0)
				{
					*e='.';
					e++;
					memset(e, ' ', precision);
					e += precision;
				}
				// Add trailing CR
				if (edtCde>='A' && edtCde<='D')
					strcpy(e, "CR");
				else
				{
					// Add trailing minus
					if (edtCde>='J' && edtCde<='M')
					{
						*e='-';
						e++;
					}
					*e='\0';
				}
			}
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



