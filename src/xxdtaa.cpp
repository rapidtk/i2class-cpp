#include "rpgtypes.h"
#pragma hdrstop

#include "xxdtaa.h"
#include <stdio.h>

/// @file xxdtaa.cpp
/// @brief Stand-in for IBM i [data area](https://www.ibm.com/docs/en/i/7.4.0?topic=procedures-using-data-areas)
/// (QXXRTVDA/QXXCHGDA system APIs).
///
/// On IBM i, a data area is a small, named object in a library
/// used to persist a value (e.g. a string, counter, flag, or date) across job or program
/// invocations - roughly analogous to a single-value shared/global variable
/// stored outside the program. RPG's `IN`/`OUT` operations on data areas map to
/// the system APIs QXXRTVDA (retrieve) / QXXCHGDA (change). 
///
/// On non-IBM i systems, each data area is emulated here as a flat file
/// named "<library>/<name>.dtaara" (or just "<name>.dtaara" when the library is
/// *LIBL) holding the raw bytes of the value.
///
/// See: [QXXRTVDA() — Retrieve Data Area](https://www.ibm.com/docs/en/i/7.4.0?topic=q-retrieve-data-area-qxxrtvda)
/// and [QXXCHGDA() — Change Data Area](https://www.ibm.com/docs/en/i/7.4.0?topic=q-change-data-area-qxxchgda)

// Build a data area file name of DATAAREA.dtaara
namespace {
	/// @brief Compose the on-disk file name used to emulate a given data area.
	/// @param dn qualified data area name (library + object name, IBM i style)
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
/// @brief Emulates IBM i QXXRTVDA - retrieve (read) bytes from a data area.
/// @param dtaname qualified data area name
/// @param offset byte offset within the data area to start reading from
/// @param len number of bytes to read into dtaptr
/// @param dtaptr destination buffer, at least len bytes
///
/// See: [QXXRTVDA() — Retrieve Data Area](https://www.ibm.com/docs/en/i/7.4.0?topic=q-retrieve-data-area-qxxrtvda)
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
/// @brief Emulates IBM i QXXCHGDA - change (write) bytes into a data area.
/// @param dtaname qualified data area name
/// @param offset byte offset within the data area to start writing at
/// @param len number of bytes to write from dtaptr
/// @param dtaptr source buffer, at least len bytes
///
/// See: [QXXCHGDA() — Change Data Area](https://www.ibm.com/docs/en/i/7.4.0?topic=q-change-data-area-qxxchgda)
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
