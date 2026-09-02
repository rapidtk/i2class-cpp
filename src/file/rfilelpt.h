#ifndef RFILELPT_H
#define RFILELPT_H

#include <stdio.h>

#include "as400.h"
#include "rfile.h"

/// @file rfilelpt.h
/// @brief Stand in for IBM i [printer file](https://www.ibm.com/docs/en/i/7.4.0?topic=file-printer-overview) access,
/// which writes to a local text file instead of generating spooled output.

/// @brief An local-text-file specific version of the base [RrecordPrint]() format class
///
/// RrecordLPT accumulates one line at a time into outputBuffer via the
/// print()/printl() calls inherited from `RrecordPrint`, and
/// flush() emits the completed line, tracking page/row position the way RPG's
/// O-specs (space/skip/overflow) do for a print file.
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

/// @brief An local-text-file specific version of the base [Rfile]() class
class RfileLPT : public Rfile
{
public:
	RfileLPT(AS400 &as400, czstring sFileName, int rcdLength) : Rfile(as400, sFileName), fp(nullptr) {}
	void setRecordFormat(RrecordLPT &format);
	bool write();
	bool write(RrecordLPT &format);
	void close();
	void open(czstring openType);

private:
	FILE *fp;
};
#endif
 