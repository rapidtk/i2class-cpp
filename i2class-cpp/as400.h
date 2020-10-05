#ifndef AS400_H
# define AS400_H
class AS400
{
public:
   AS400(char *sURL);
   AS400(char *sURL, char *lusrid, char *lpassword);

public:
   char	*url;
   char	*usrid;
   char  *password;
};
#endif
