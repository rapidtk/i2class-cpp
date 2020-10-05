#include "xPort.h"
AS400 as400("ASC406");

double amtdu;
double amttot;
char INLR;
#include "custmastB.h"
class Cusfmt : public CUSTMAST_CUSFMT
{
	void input()
	{
		amtdu = getDouble(AMTDU);
		//fixedCpy(cname, CNAME);
	}
} cusfmt;
XFILE custmast(as400, "TESTC/CUSTMASTB");

void main()
{
	custmast.setRecordFormat(cusfmt);
	custmast.open(READ_ONLY);
	for (int i=0; i<1000; i++)
	{
		cusfmt.setKey(0);
		custmast.setll();
		INLR=custmast.read();
		while (INLR==OFF)
		{
			amttot = amttot + amtdu;
			INLR=custmast.read();
		}
	}
}

