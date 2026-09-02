#ifndef AS400_H
# define AS400_H

#include "compat/StringContracts.h"

class AS400
{
public:
   /// @brief Default, no-arg constructor which assumes connection to localhost using current credentials
   AS400() : url("localhost"), usrid("*CURRENT"), password("*CURRENT") {};
   /// @brief Connect to `sURL` using the current user ID's credentials.
   AS400(czstring sURL);
   /// @brief Connect to `sURL` using an explicit user ID and password.
   AS400(czstring sURL, czstring lusrid, czstring lpassword);

public:
   czstring	url;
   czstring	usrid;
   czstring	password;
};
#endif
