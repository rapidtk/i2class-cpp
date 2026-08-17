#include "xport.h"
#pragma hdrstop

#include "PrintFile.h"


char XfileLPT::open()
{
	fp=fopen(fileName, "w");
	return '0';
}

char XfileLPT::close()
{
	((XrecordLPT*)record)->flush();
	fclose(fp);
	return '0';
}

// Print out a numeric value
void XfileLPT::write()
{
	record->output();
}

void XfilePrint::lsetRecordFormat(XrecordPrint &format)
{
   if (record)
   {
      column=((XrecordPrint*)record)->column;
      row=((XrecordPrint*)record)->row;
   }
   Xfile::setRecordFormat(format);
   format.column=column;
   format.row=row;
}

void XfileLPT::setRecordFormat(XrecordLPT &format)
{
	lsetRecordFormat(format);
   format.fp=fp;
}

void XrecordPrint::edit(char *buf, const NumericTemp &n, const char *edtWrd)
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
   if (precision==0)
   	precision = n.PrecisionOf();

	// Start from the back of the converted string and start copying in digits
	char *str;
   int dec, sign;
   str=fcvt(n.value, precision, &dec, &sign);
   int strI=strlen(str);
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
         	if (n.value<0)
	            memcpy(buf+wrdI, "CR", 2);
            break;
         }
         else
         	goto dft;
      case '-':
      	if (wrdI-lastBlank==1 || wrdI==0)
         {
         	if (n.value<0)
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
      	if (strI>=0 && (str[strI]!='0' || wrdI<zeroSuppress))
	         buf[wrdI]=str[strI];
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
void XrecordPrint::print(const FixedTemp &f, int col)
{
	setMaxColumn(col);

   int edtLen = f.Length();
   if (edtLen>col)
   	edtLen=col;
	// See if the positions in the record alrady have characters.  If they do,
   // then write out record.
   char *bufChar=buf+col-edtLen;
//	if (strncmp(bufChar, BLANKS, edtLen)!=0)
	if (strncmp(bufChar, "                                ", edtLen)!=0)
		flush();

   memcpy(bufChar, f.overlay, edtLen);
}

// Print out a numeric value
void XrecordPrint::print(const NumericTemp &n, int col, const char *edtWrd)
{
	setMaxColumn(col);

	int edtLen;
   if (edtWrd != NULL)
		edtLen=strlen(edtWrd);
   else
   	edtLen = n.Length();
   if (edtLen>col)
   	edtLen=col;
	// See if the positions in the record alrady have characters.  If they do,
   // then write out record.
   char *bufNum=buf+col-edtLen;
	//if (strncmp(bufNum, BLANKS, edtLen)!=0)
	if (strncmp(bufNum, "                                ", edtLen)!=0)
		flush();

	if (edtWrd != NULL)
   	edit(bufNum, n, edtWrd);
   else
   {
		char *str;
      int dec, sign;
      str=fcvt(n.value, n.PrecisionOf(), &dec, &sign);
      int strLen=strlen(str);
		int i=edtLen-strLen;
      if (i>0) {
      	memset(bufNum, '0', i);
         bufNum += i;
      }
      memcpy(bufNum, str, strLen);
   }
}

// Keep track of the right-most column written to
void XrecordPrint::setMaxColumn(int &col)
{
	if (col<=0)
   	col = column;
   else {
   	column = col;
      if (column > maxColumn)
      	maxColumn = column;
   }
}

void XrecordLPT::space(int rows)
{
	flush();
	while (rows>0)
   {
	   fputc('\n', fp);
   	rows--;
      row++;
   }
}

void XrecordLPT::flush()
{
	if (maxColumn>0)
   {
		fprintf(fp, "%s*", buf, maxColumn);
   	memset(buf, ' ', sizeof(buf));
   	maxColumn=0;
   	column=0;
   }
}


