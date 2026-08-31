#ifndef AS400_H
# define AS400_H
class AS400
{
public:
   /// @brief Default, no-arg constructor which assumes connection to localhost using current credentials
   AS400() : url("localhost"), usrid("*CURRENT"), password("*CURRENT") {};
   AS400(const char *sURL);
   AS400(const char *sURL, const char *lusrid, const char *lpassword);

public:
   const char	*url;
   const char	*usrid;
   const char	*password;
};
#endif
