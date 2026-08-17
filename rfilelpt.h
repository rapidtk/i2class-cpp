#ifndef RFILELPT_H
#define RFILELPT_H

#include <stdio.h>

#include "as400.h"
#include "rfile.h"

////////////////////////////////////////////////////////////////////////////////
// Print file stuff
class RrecordLPT : public RrecordPrint
{
friend class RfileLPT;
public:
	void flush();
	void space(int rows=1);
	void skip(int row);
public:
	static int page;

private:
	FILE *fp;
};

class RfileLPT : public Rfile
{
public:
	RfileLPT(AS400 &as400, char *sFileName, int rcdLength) : Rfile(as400, sFileName) {}
	void setRecordFormat(RrecordLPT &format);
	char write();
	char write(RrecordLPT &format);
	char close();
	char open(const char *openType);

private:
	FILE *fp;
};
#endif
 