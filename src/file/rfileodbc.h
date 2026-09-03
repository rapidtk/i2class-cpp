#ifndef RFILEODBC_H
#define RFILEODBC_H

#ifdef __OS400__
# include <sqlcli.h>
#else
# ifdef _WIN32
#  include <windows.h>
# endif
# include <sql.h>
# include <sqlext.h> // SQLDriverConnect, SQL_ATTR_ODBC_VERSION, SQL_OV_ODBC3, ...
# define SQL_TXN_NO_COMMIT 1
# define SQL_ATTR_COMMIT 0
#endif

#include "as400.h"
#include "rfile.h"
//#include "RPGtypes.h"

#define getInt(ncol) readInt(ncol)
#define getDouble(ncol) readDouble(ncol)
//#define getString(ncol) readString(col.overlay, ncol, col.len()) ? col : col
#define copyString(a, b) copyMem(a.overlay, b, a.len())
// Write not implemented yet, so these are no-ops for now...
#define setValue(a, b)
#define setText(a, b)

//#define DEFINE_GET(e) \
//int getInt(e fn) { return readInt(fn); } \
//double getDouble(e fn) { return readDouble(fn); } \
//char getChar(e fn) { return readChar(fn); } \
//bool getMem(void *str, e fn, int len) { return copyMem(str, fn, len); }
//#define fixedCpy(a, b) memCpy(a.overlay, b, a.len())

/// @brief An ODBC-specific version of the base [Rrecord]() format class
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

	/// @brief Fetch a column as exact decimal text (e.g. "1234.56"), avoiding the binary
	/// floating-point round-trip readDouble() goes through -- the ODBC/C++ analog of
	/// Java's RecordJDBC.getBigDecimal()/getDecimal()/getNumeric(). Returns false if the
	/// driver reported no data (NULL) for this column.
	bool readDecimal(SQLSMALLINT columnNumber, zstring buf, int bufLen);
	/// @brief Fetch a column directly into a Zoned<sz,precision> field, exactly -- see
	/// readDecimal(SQLSMALLINT, char*, int).
	template <int sz, int precision>
	bool readDecimal(SQLSMALLINT columnNumber, Zoned<sz, precision> &dest)
	{
		char buf[MAX_DECIMAL_DIGITS + 3];
		if (!readDecimal(columnNumber, buf, sizeof(buf)))
			return false;
		zonedFromChar(reinterpret_cast<byte_ptr>(dest.overlay), sz, precision, buf);
		return true;
	}
protected:
	int	keyLength;
private:
	SQLLEN		rLength;
};

/// @brief An ODBC-specific version of the base [Rfile]() class
class RfileODBC : public Rfile
{
public:
	// server (AS400::url) is a full ODBC connection string passed directly to
	// SQLDriverConnect -- e.g. "Driver={Microsoft Access Text Driver (*.txt, *.csv)};
	// Dbq=C:\\path\\to\\folder;Extensions=asc,csv,tab,txt;HDR=Yes;FMT=Delimited;"
	RfileODBC(const AS400 &as400, czstring sFileName)
	 : Rfile(as400, sFileName), henv(0), hdbc(0), hstmt(0), rc(0), lastErrorText()
	{};
	/*
	RfileODBC(AS400 &as400, char *sFileName, Rrecord &format, const char *openType)
	 :Rfile(as400, sFileName, format)
	{
		open(openType);
	}
	*/
	~RfileODBC();
	/// @brief Close the file. Never throws (freeing ODBC handles cannot meaningfully fail here).
	void close();
	/// @brief Open the file. Throws CI2ErrFile (see lastError()) if the ODBC connection or query fails.
	void open(czstring openType, int blockingFactor=0,
    char commitLockLevel=COMMIT_LOCK_LEVEL_NONE);
   void setRecordFormat(RrecordODBC &format);
	/// @brief Fetch the next row. Returns true if a row was read, false at end-of-file (sets eof).
	bool read();
//	void template <int sz> fixedCpy(Fixed<sz> &fStr, SQLSMALLINT columnNumber);

	// Text of the most recent ODBC diagnostic record, or empty if nothing failed.
	czstring lastError() const { return lastErrorText; }

private:
	// Not static: a static SQLHENV was shared (and freed out from under) every RfileODBC
	// instance, not just repeated per file the way the old single-file design assumed.
	SQLHENV        henv;
	SQLHDBC        hdbc;
	SQLHSTMT       hstmt;
	SQLRETURN		rc;
	char           lastErrorText[256];
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

