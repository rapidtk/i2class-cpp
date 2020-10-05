//---------------------------------------------------------------------------

#pragma hdrstop

//---------------------------------------------------------------------------

#pragma argsused

#include <stdio.h>
extern void yyparse();
extern FILE *yyin;


int main(int argc, char* argv[])
{
	yyin = fopen("yyin.txt", "r");
   yyparse();
	return 0;
}
//---------------------------------------------------------------------------
 