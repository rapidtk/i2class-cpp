#include "rpgtypes.h"
#pragma hdrstop

#include "xxdtaa.h"
#include <stdio.h>

// Build a data area file name of DATAAREA.dtaara
namespace {
	char *getDtaaraFileName(_DTAA_NAME_T dn)
	{
		DtaaName	dtaname;
		memcpy(&dtaname, &dn, sizeof(dtaname));
		static char buf[30];
		int i=0;
		if (dtaname.dtaa_lib != "*LIBL")
		{
			i=dtaname.dtaa_lib.checkr(" ");
			memcpy(buf, dtaname.dtaa_lib.overlay, i);
			buf[i]='/';
			i++;
		}
		int len=dtaname.dtaa_name.checkr(" ");
		memcpy(buf+i, dtaname.dtaa_name.overlay, len);
		i=i+len;
		strcpy(buf+i, ".dtaara");
		return buf;
	}
}
// Read values from a "data area"
void QXXRTVDA(_DTAA_NAME_T dtaname, short offset, short len, char *dtaptr)
/*
	{ return QXXRTVDA((DtaaName)dtaname, offset, len, dtaptr); }
void QXXRTVDA(DtaaName dtaname, short offset, short len, char *dtaptr)
*/
{
	FILE *fp=fopen(getDtaaraFileName(dtaname), "r");
	if (fp)
	{
		if (offset>0)
			fseek(fp, offset, SEEK_SET);
		fread(dtaptr, 1, len, fp);
		fclose(fp);
	}
}
// Write values to a "data area"
void QXXCHGDA(_DTAA_NAME_T dtaname, short offset, short len, char *dtaptr)
/*
	{ return QXXCHGDA((DtaaName)dtaname, offset, len, dtaptr); }
void QXXCHGDA(DtaaName dtaname, short offset, short len, char *dtaptr)
*/
{
	FILE *fp=fopen(getDtaaraFileName(dtaname), "w");
	if (fp)
	{
		if (offset>0)
			fseek(fp, offset, SEEK_SET);
		fwrite(dtaptr, 1, len, fp);
		fclose(fp);
	}
}//---------------------------------------------------------------------------
#pragma package(smart_init)
