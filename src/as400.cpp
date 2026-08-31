#include "as400.h"

/// @file as400.cpp
/// @brief Connection/identity info for a remote IBM i (AS/400) system.
///
/// This is the "target system" descriptor used by the ODBC/ADO file backends
/// (see rfileodbc.cpp/rfileado.cpp) to know which IBM i host and user profile
/// to connect as when a program is compiled to run remotely from Windows
/// instead of natively on IBM i.

/// @brief Construct with just a host/URL; no credentials (e.g. relies on default/cached auth).
AS400::AS400(char *sURL)
{
   url=sURL;
}
/// @brief Construct with host/URL plus an IBM i user profile and password.
AS400::AS400(char *sURL, char *lusrid, char *lpassword)
{
   url=sURL;
   usrid=lusrid;
   password=lpassword;
}

