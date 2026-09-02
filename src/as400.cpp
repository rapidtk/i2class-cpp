#include "as400.h"

/// @file as400.cpp
/// @brief Connection/identity info for a target system/database server.
///
/// This is the "target host" descriptor used by various
/// access classes - defaults to:
/// 
/// system="localhost", userid="*CURRENT", password="*CURRENT"
///
/// which means connect to the local host using the current user id/password.

/// @brief Construct with just a host/URL; no credentials (e.g. relies on default/cached auth).
AS400::AS400(czstring sURL)
{
   url=sURL;
   usrid="*CURRENT";
   password="*CURRENT";
}
/// @brief Construct with host/URL plus a user id and password.
AS400::AS400(czstring sURL, czstring lusrid, czstring lpassword)
{
   url=sURL;
   usrid=lusrid;
   password=lpassword;
}

