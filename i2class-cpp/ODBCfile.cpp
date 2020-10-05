#define XFILE XfileODBC
#include "xport.h"
#pragma hdrstop
//#include <string>

#include "ODBCfile.h"

const char *READ_ONLY="rr";
const char *READ_WRITE="rr+";
const char *WRITE_ONLY="ar";

const char COMMIT_LOCK_LEVEL_NONE='n';
const char COMMIT_LOCK_LEVEL_DEFAULT='y';


SQLHENV XfileODBC::henv;

XfileODBC::~XfileODBC()
{
	close();
}

// Initialize the class to point to the (already allocated) structure
char XfileODBC::close()
{
   SQLFreeStmt (hstmt, SQL_DROP);         /* free the statement handle    */

   /* EXEC SQL DISCONNECT;                                                */
   SQLDisconnect (hdbc);                  /* disconnect from the database */

   SQLFreeConnect (hdbc);                 /* free the connection handle   */
   SQLFreeEnv (henv);                     /* free the environment handle  */
   return '0';
}

char XfileODBC::open(const char *OpenType, int blockingFactor, char commitLockLevel)
{
	error='1';
   rc=SQLAllocEnv (&henv);                 /* allocate an environment handle */
	if (rc==SQL_SUCCESS)
   {
      rc=SQLAllocConnect (henv, &hdbc);       /* allocate a connection handle   */
      if (rc==SQL_SUCCESS)
      {
         SQLINTEGER	vParam=SQL_TXN_NO_COMMIT;
         rc=SQLSetConnectAttr(hdbc, SQL_ATTR_COMMIT, &vParam, 0);
         if (rc==SQL_SUCCESS)
         {
            rc=SQLConnect (hdbc, server, SQL_NTS, "ANDREWC", SQL_NTS, "SP8DS",
             SQL_NTS);
            if (rc==SQL_SUCCESS)
            {
               rc=SQLAllocStmt (hdbc, &hstmt);         /* allocate a statement handle    */
               if (rc==SQL_SUCCESS)
               {
                  char SQL[256]="SELECT * FROM ";
                  strcat(SQL, fileName);
                  //strcat(SQL, record->keyList);
                  rc=SQLExecDirect(hstmt, SQL, SQL_NTS);
	               if (rc==SQL_SUCCESS)
                  	error='0';
               }
            }
         }
      }
   }
   return error;
}

char XfileODBC::read()
{
	noData='0';
	rc=SQLFetch (hstmt);                      /* now execute the fetch        */
	switch (rc)
	{
		case SQL_SUCCESS:
		case SQL_SUCCESS_WITH_INFO:
			record->input();
			break;
		case SQL_ERROR:
		case SQL_INVALID_HANDLE:
			error='1'; // fall through intentionally to set EOF
		default:
			noData='1';
	}

	return noData;
}

void XfileODBC::setRecordFormat(XrecordODBC &format)
{
	Xfile::setRecordFormat(format);
	format.hstmt = hstmt;
}

int XrecordODBC::getInt(SQLSMALLINT columnNumber)
{
   int i;
//	SQLGetCol(hstmt, columnNumber, SQL_INTEGER, (SQLPOINTER) &i,
	SQLGetData(hstmt, columnNumber, SQL_INTEGER, (SQLPOINTER) &i,
    sizeof(i), &rLength);
   return i;
}

double XrecordODBC::getDouble(SQLSMALLINT columnNumber)
{
   double d;
	SQLGetData(hstmt, columnNumber, SQL_DOUBLE, (SQLPOINTER) &d,
    sizeof(d), &rLength);
   return d;
}

void XrecordODBC::memCpy(void *str, SQLSMALLINT columnNumber, int strLen)
{
	SQLGetData(hstmt, columnNumber, SQL_CHAR, (SQLPOINTER) str,
    strLen, &rLength);
}

/*
void XrecordODBC::fixedCpy(template <int sz> Fixed<sz> &fStr, SQLSMALLINT columnNumber)
{
   XrecordODBC::memCpy(fStr.buf, fStr.Length(), columnNumber);
}
*/





