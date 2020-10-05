#include "as400.h"

AS400::AS400(char *sURL)
{
   url=sURL;
}
AS400::AS400(char *sURL, char *lusrid, char *lpassword)
{
   url=sURL;
   usrid=lusrid;
   password=lpassword;
}

