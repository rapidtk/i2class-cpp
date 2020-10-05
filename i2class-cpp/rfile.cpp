#include "rfile.h"

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
// Print file stuff
void RrecordPrint::edit(char *buf, const _ConvertDecimal &n, const char *edtWrd)
{
	// Make a pass forwards through the edit word to accumulate information...
	int wrdI=strlen(edtWrd);
	char	fillChar=' ';
	int currency=0, decimal=0, precision=0, lastBlank;
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
			else if (wrdI>zeroSuppress)
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
	{ printChar(str, strlen(str), col); }

void RrecordPrint::printl(const FixedTemp &f, int col)
{
	if (col<=0)
		col=column;
	print(f, col+f.len()-1);
}
void RrecordPrint::printl(const char *str, int col)
{
	FixedTemp cStr((char *)str, strlen(str));
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
void RrecordPrint::print(const _ConvertDecimal &n, int col, const char *edtWrd)
{
	setMaxColumn(col);

	int edtLen;
	if (edtWrd != NULL)
		edtLen=strlen(edtWrd);
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
		edtLen=strlen(edtWrd);
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
				char *comma;
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



