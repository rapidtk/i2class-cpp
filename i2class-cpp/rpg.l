/* flex -olex.yy.cpp rpg.l */
%{
#include "node.h"
#include "rpg_tab.cpp.h"
#include <stdio.h>
#include <string.h>

int tokenText(int type);
%}

digit 	[0-9]
letter	[A-Za-z#$@_]

%%

{letter}({letter}|{digit})* 	{return tokenText(IDENTIFIER); }
{digit}+				 				{return tokenText(NUMBER); }
\'[^\']?\'							{return tokenText(STRING); }
\"[^\"]*\"							{return tokenText(STRING); }

[-()<>=+*/,!]						{return *yytext;}

">="									{return GE;}
"<="									{return LE;}
"!="									{return NE;}
"=="									{return EQUALS;}
"&&"									{return AND;}
"||"									{return OR;}

[ \t\n\r]								; /* ignore whitespace */
.										{puts("Unknown character");}

%%
int yywrap(void) { return 1; }
int tokenText(int type)
{
	strcpy(yylval.text, yytext);
   return type;
}

char *GET_NFUNC(char *identifier, bool &isClass)
{
   if (strcmp(identifier, "cname")==0)
   {
      isClass=true;
      return "fixed";
   }
   if (strcmp(identifier, "cusno")==0)
   {
      isClass=true;
      return "int";
   }
   isClass=false;
	return "int";
}

