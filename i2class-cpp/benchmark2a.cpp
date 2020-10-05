#include "xport.h"

struct {
	_DecimalT< 6, 0> CUSNO;               // Customer Number
													  // BCD class SPECIFIED IN DDS
	Fixed<25> CNAME;                       // Customer Name
	char CADD1[25];                       // Customer Address Line 1
	char CADD2[25];                       // Customer Address Line 2
	char CADD3[16];                       // Customer Address Line 3
	char CSTTE[2];                        // Customer State
	char CZIPC[10];                       // Customer Zip Code
	_DecimalT<10, 0> CPHON;               // Phone Number
													  // BCD class SPECIFIED IN DDS
	char CTYPE[2];                        // Customer Type
	_DecimalT< 9, 0> CRLIM;               // Credit Limit in Dollars
													  // BCD class SPECIFIED IN DDS
	double AMTDU;               // Outstanding A/R Balance
													  // BCD class SPECIFIED IN DDS
	_DecimalT< 9, 2> OROPN;               // Total Open Orders in Dollars
													  // BCD class SPECIFIED IN DDS
	_DecimalT< 9, 0> HIGHB;               // Highext A/R Balance
													  // BCD class SPECIFIED IN DDS
	_DecimalT<11, 2> PAYAM;               // Last Payment Amount
													  // BCD class SPECIFIED IN DDS
	Zoned<2,0> PAYMN;                        // Last Payment Date-Month
													  // ZONE SPECIFIED IN DDS
													  // REPLACED BY CHARACTER TYPE
	Zoned<2,0> PAYDY;                        // Last Payment Date-Day
													  // ZONE SPECIFIED IN DDS
													  // REPLACED BY CHARACTER TYPE
	Zoned<2,0> PAYYR;                        // Last Payment Date-Year
													  // ZONE SPECIFIED IN DDS
													  // REPLACED BY CHARACTER TYPE
	_DecimalT< 9, 2> MTDdC;               // Month to Date Sales
													  // BCD class SPECIFIED IN DDS
	_DecimalT<11, 2> YTDdC;               // Year to Date Sales
													  // BCD class SPECIFIED IN DDS
	_DecimalT< 3, 0> REGON;               // Sales Region
													  // BCD class SPECIFIED IN DDS
	_DecimalT< 3, 0> SALNO;               // Salesman Number
													  // BCD class SPECIFIED IN DDS
	char DFTWH[2];                        // Default Warehouse
	_DecimalT< 6, 0> PAYMDY;              // Last Payment Date MDY
													  // BCD class SPECIFIED IN DDS
	_DecimalT< 6, 0> PAYDMY;              // BCD class SPECIFIED IN DDS
	char PAYYMD[6];                       // Last Payment Date YMD
													  // ZONE SPECIFIED IN DDS
													  // REPLACED BY CHARACTER TYPE
	_DecimalT< 5, 0> PAYJUL;              // Last Payment Date JUL
} cusfmt;
double amttot=0;

void main()
{
	_RFILE *fp = _Ropen("TESTC/CUSTMASTB", "rr");
	for (int i=0; i<1000; i++)
	{
		_Rlocate(fp, NULL, 0, __FIRST | __PRIOR);
		_RIOFB_T *feedBack = _Rreadn(fp, &cusfmt, sizeof(cusfmt), __DFT);
		while (feedBack->num_bytes>0)
		{
			amttot = amttot + cusfmt.AMTDU;
			feedBack = _Rreadn(fp, &cusfmt, sizeof(cusfmt), __DFT);
		}
	}
	_Rclose(fp);
}

