//---------------------------------------------------------------------------

#ifndef XXDTAA_H
#define XXDTAA_H

struct _DTAA_NAME_T {
	char	dtaa_name[10];
	char	dtaa_lib[10];
};
void QXXRTVDA(_DTAA_NAME_T dtaname, short offset, short len, char *dtaptr);
//void QXXRTVDA(DtaaName dtaname, short offset, short len, char *dtaptr);
void QXXCHGDA(_DTAA_NAME_T dtaname, short offset, short len, char *dtaptr);
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
