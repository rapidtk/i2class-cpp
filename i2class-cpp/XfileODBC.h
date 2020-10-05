#ifndef ODBCFILE_H
#define ODBCFILE_H

#ifdef __OS400__
# include <sqlcli.h>
#else
# ifdef __WIN32__
#  include <windows.h>
# endif
# include <sql.h>
# define SQL_TXN_NO_COMMIT 1
# define SQL_ATTR_COMMIT 0
#endif

#include "xFile.h"
#include "stdio.h"
//#include "RPGtypes.h"

#define fixedCpy(a, b) memCpy(a.overlay, b, a.len())

class XrecordODBC : public Xrecord
{
friend class XfileODBC;
public:
	SQLHSTMT       hstmt;

public:
	int	getInt(SQLSMALLINT columnNumber);
	double	getDouble(SQLSMALLINT columnNumber);
	void memCpy(void *str, SQLSMALLINT columnNumber, int strLen);
protected:
	int	keyLength;
private:
	SQLINTEGER		rLength;
};

class XfileODBC : public Xfile
{
public:
	XfileODBC(AS400 &as400, char *sFileName):Xfile(as400, sFileName){};
	/*
	XfileODBC(AS400 &as400, char *sFileName, Xrecord &format, const char *openType)
	 :Xfile(as400, sFileName, format)
	{
		open(openType);
	}
	*/
	~XfileODBC();
	char close();
	char open(const char *openType, int blockingFactor=0,
    char commitLockLevel=COMMIT_LOCK_LEVEL_NONE);
   void setRecordFormat(XrecordODBC &format);
	char read();
//	void template <int sz> fixedCpy(Fixed<sz> &fStr, SQLSMALLINT columnNumber);

private:
	static SQLHENV        henv;
	SQLHDBC        hdbc;
	SQLHSTMT       hstmt;
	SQLRETURN		rc;
};

////////////////////////////////////////////////////////////////////////////////
// Print file stuff
class XrecordLPT : public XrecordPrint
{
friend class XfileLPT;
public:
	void flush();
	void space(int rows=1);

private:
	FILE *fp;
};

class XfileLPT : public Xfile
{
public:
	XfileLPT(AS400 &as400, char *sFileName) : Xfile(as400, sFileName) {}
	void setRecordFormat(XrecordLPT &format);
	void write();
	void write(XrecordLPT &format);
	char close();
	char open(const char *openType);

private:
	FILE *fp;
};
#endif

