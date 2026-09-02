//---------------------------------------------------------------------------

#ifndef XXDTAA_H
#define XXDTAA_H

#include "StringContracts.h"

/// @file xxdtaa.h
/// @brief This header and the accompanying `xxdtaa.cpp` are a thin compatibility
/// layer to add data area support on platforms other than IBM i
/// where xxdta.h is part of the operating system QSYSINC/H includes.
/// @note There is a `legacy/xxdta.hpp` which is a renamed copy of the IBM i
/// version of `xxdta.h` and what this copy was based on at the time.

struct _DTAA_NAME_T {
	char	dtaa_name[10];
	char	dtaa_lib[10];
};
/// @brief Retrieve data area [QXXRTVDA](https://www.ibm.com/docs/en/i/7.4.0?topic=q-retrieve-data-area-qxxrtvda) API equivalent
void QXXRTVDA(_DTAA_NAME_T dtaname, short offset, short len, byte_ptr dtaptr);
//void QXXRTVDA(DtaaName dtaname, short offset, short len, char *dtaptr);
/// @brief Change data area [QXXCHGDA](https://www.ibm.com/docs/en/i/7.4.0?topic=q-change-data-area-qxxchgda) API equivalent
void QXXCHGDA(_DTAA_NAME_T dtaname, short offset, short len, byte_ptr dtaptr);
//void QXXCHGDA(DtaaName dtaname, short offset, short len, char *dtaptr);


///////////////////////////////////////////////////////////////////////////////
// Data area
/*
template <int sz> class Dtaara : public Fixed<sz>
{
public:
	Dtaara(char *name) {dtaaraName=name; }
	// Retrieve the content of a Dtaara
	void in()
		{ QXXRTVDA(getLocation(), 0, sz, overlay); }
	void out()
		{ QXXCHGDA(getLocation(), 0, sz, overlay); }
private:
	DtaaName getLocation()
	{
		DtaaName location;
		location.dtaa_name=dtaaraName;
		if (*dtaaraName!='*')
			location.dtaa_lib="*LIBL";
		return location;
	}
private:
	char	*dtaaraName;
};
*/
//---------------------------------------------------------------------------
#endif
