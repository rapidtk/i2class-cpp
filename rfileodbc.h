#ifndef RFILEODBC_H
#define RFILEODBC_H

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

#include "as400.h"
#include "rfile.h"
//#include "RPGtypes.h"

//#define getInt(col, ncol) col=readInt(ncol)
//#define getDouble(col, ncol) col=readDouble(ncol)
//#define getString(col, ncol) readString(col.overlay, ncol, col.len()) ? col : col
#define copyString(a, b) getMem(a.overlay, b, a.len())
#define setValue(a, b)
#define setText(a, b)

#define DEFINE_GET(e) \
int getInt(e fn) { return readInt(fn); } \
double getDouble(e fn) { return readDouble(fn); } \
char getChar(e fn) { return readChar(fn); } \
bool getMem(void *str, e fn, int len) { return copyMem(str, fn, len); }

//#define fixedCpy(a, b) memCpy(a.overlay, b, a.len())

class RrecordODBC : public Rrecord
{
friend class RfileODBC;
public:
	SQLHSTMT       hstmt;

public:
	int	readInt(SQLSMALLINT columnNumber);
	double	readDouble(SQLSMALLINT columnNumber);
	bool copyMem(void *str, SQLSMALLINT columnNumber, int strLen);
	char readChar(SQLSMALLINT columnNumber);
protected:
	int	keyLength;
private:
	SQLINTEGER		rLength;
};

class RfileODBC : public Rfile
{
public:
	RfileODBC(AS400 &as400, char *sFileName): Rfile(as400, sFileName){};
	/*
	RfileODBC(AS400 &as400, char *sFileName, Rrecord &format, const char *openType)
	 :Rfile(as400, sFileName, format)
	{
		open(openType);
	}
	*/
	~RfileODBC();
	char close();
	char open(const char *openType, int blockingFactor=0,
    char commitLockLevel=COMMIT_LOCK_LEVEL_NONE);
   void setRecordFormat(RrecordODBC &format);
	char read();
//	void template <int sz> fixedCpy(Fixed<sz> &fStr, SQLSMALLINT columnNumber);

private:
	static SQLHENV        henv;
	SQLHDBC        hdbc;
	SQLHSTMT       hstmt;
	SQLRETURN		rc;
};

/*
class RindexODBC : public RfileODBC {

	char chain();
	char chain(RrecordODBC r);
	char open(const char *openType, int blockingFactor=0,
    char commitLockLevel=COMMIT_LOCK_LEVEL_NONE);
	char reade();
	char reade(RrecordODBC r);
	char readEqual();
	char readpe();
	char readpe(RrecordODBC r);
	char readpEqual();
	char setgt();
	char setgt(RrecordODBC r);
	char setll();
	char setll(RrecordODBC r);

private:
	int checkKey(Object[] key);
	void readxe() throws SQLException
	boolean seekAfter(Object[] key, int start, int end) throws SQLException
	private boolean seekKey(Object[] key, int start, int end) throws SQLException
private:
	int lastRecord;
}
*/
#endif

