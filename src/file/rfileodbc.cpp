//#pragma hdrstop
//#include <string>

#include <cstdio>
#include <cstring>

#include "rfileodbc.h"

/// @file rfileodbc.cpp
/// @brief Concrete ODBC file access via SQLDriverConnect + "SELECT * FROM <fileName>".
/// AS400::url is used directly as the full ODBC connection string (e.g. targeting the
/// Windows-builtin Microsoft Text Driver for CSV files), matching the pattern already
/// proven out in izlib-cpp's OdbcFileHandle.

const char *READ_ONLY="rr";
const char *READ_WRITE="rr+";
const char *WRITE_ONLY="ar";

const char COMMIT_LOCK_LEVEL_NONE='n';
const char COMMIT_LOCK_LEVEL_DEFAULT='y';

namespace
{
bool odbcSucceeded(SQLRETURN ret)
{
	return ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO;
}
} // namespace

RfileODBC::~RfileODBC()
{
	close();
}

void RfileODBC::close()
{
	if (hstmt)
	{
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		hstmt = nullptr;
	}
	if (hdbc)
	{
		SQLDisconnect(hdbc);
		SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
		hdbc = nullptr;
	}
	if (henv)
	{
		SQLFreeHandle(SQL_HANDLE_ENV, henv);
		henv = nullptr;
	}
}

void RfileODBC::open(const char */*OpenType*/, int /*blockingFactor*/, char /*commitLockLevel*/)
{
	error=false;
	lastErrorText[0]='\0';

	if (!odbcSucceeded(SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv)))
	{
		error=true;
		throw CI2ErrFile("Failed to allocate ODBC environment handle");
	}
	SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (void *)SQL_OV_ODBC3, 0);

	if (!odbcSucceeded(SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc)))
	{
		SQLFreeHandle(SQL_HANDLE_ENV, henv);
		henv = nullptr;
		error=true;
		throw CI2ErrFile("Failed to allocate ODBC connection handle");
	}

	// server (== AS400::url) is the full ODBC connection string, passed as-is.
	SQLCHAR outConnStr[1024];
	SQLSMALLINT outLen=0;
	rc=SQLDriverConnect(hdbc, nullptr, (SQLCHAR *)server, SQL_NTS, outConnStr,
	 sizeof(outConnStr), &outLen, SQL_DRIVER_NOPROMPT);
	if (!odbcSucceeded(rc))
	{
		SQLGetDiagRec(SQL_HANDLE_DBC, hdbc, 1, nullptr, nullptr, (SQLCHAR *)lastErrorText,
		 sizeof(lastErrorText), nullptr);
		SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
		hdbc = nullptr;
		SQLFreeHandle(SQL_HANDLE_ENV, henv);
		henv = nullptr;
		error=true;
		throw CI2ErrFile(lastErrorText);
	}

	if (!odbcSucceeded(SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt)))
	{
		SQLDisconnect(hdbc);
		SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
		hdbc = nullptr;
		SQLFreeHandle(SQL_HANDLE_ENV, henv);
		henv = nullptr;
		error=true;
		throw CI2ErrFile("Failed to allocate ODBC statement handle");
	}

	char sql[512];
	std::snprintf(sql, sizeof(sql), "SELECT * FROM %s", fileName);
	rc=SQLExecDirect(hstmt, (SQLCHAR *)sql, SQL_NTS);
	if (odbcSucceeded(rc))
	{
		error=false;
		// setRecordFormat() may have been called before open() (before hstmt existed) --
		// (re-)sync the bound record's copy now that the real statement handle exists.
		if (record)
			static_cast<RrecordODBC *>(record)->hstmt = hstmt;
	}
	else
	{
		SQLGetDiagRec(SQL_HANDLE_STMT, hstmt, 1, nullptr, nullptr, (SQLCHAR *)lastErrorText,
		 sizeof(lastErrorText), nullptr);
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		hstmt = nullptr;
		SQLDisconnect(hdbc);
		SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
		hdbc = nullptr;
		SQLFreeHandle(SQL_HANDLE_ENV, henv);
		henv = nullptr;
		error=true;
		throw CI2ErrFile(lastErrorText);
	}
}

bool RfileODBC::read()
{
	rc=SQLFetch (hstmt);                      /* now execute the fetch        */
	switch (rc)
	{
		case SQL_SUCCESS:
		case SQL_SUCCESS_WITH_INFO:
			record->input();
			eof=false;
			return true;
		case SQL_ERROR:
		case SQL_INVALID_HANDLE:
			error=true; // fall through intentionally to set eof
		default:
			eof=true;
			return false;
	}
}


void RfileODBC::setRecordFormat(RrecordODBC &format)
{
	setRecord(format);
	format.hstmt = hstmt;
}

int RrecordODBC::readInt(SQLSMALLINT columnNumber)
{
   int i;
//	SQLGetCol(hstmt, columnNumber, SQL_INTEGER, (SQLPOINTER) &i,
	SQLGetData(hstmt, columnNumber, SQL_INTEGER, (SQLPOINTER) &i,
    sizeof(i), &rLength);
   return i;
}

double RrecordODBC::readDouble(SQLSMALLINT columnNumber)
{
	double d;
	SQLGetData(hstmt, columnNumber, SQL_DOUBLE, (SQLPOINTER) &d,
    sizeof(d), &rLength);
   return d;
}

char RrecordODBC::readChar(SQLSMALLINT columnNumber)
{
	char c;
	copyMem(&c, columnNumber, 1);
	return c;
}

bool RrecordODBC::copyMem(void *str, SQLSMALLINT columnNumber, int strLen)
{
	SQLGetData(hstmt, columnNumber, SQL_CHAR, (SQLPOINTER) str,
	 strLen, &rLength);
	return true;
}

bool RrecordODBC::readDecimal(SQLSMALLINT columnNumber, char *buf, int bufLen)
{
	SQLGetData(hstmt, columnNumber, SQL_C_CHAR, (SQLPOINTER) buf, bufLen, &rLength);
	return rLength != SQL_NULL_DATA;
}

/*
void RrecordODBC::fixedCpy(template <int sz> Fixed<sz> &fStr, SQLSMALLINT columnNumber)
{
   RrecordODBC::memCpy(fStr.buf, fStr.Length(), columnNumber);
}
*/

