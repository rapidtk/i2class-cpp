/* bison clp.y -d -o clp_tab.cpp --no-lines */
%{
#include "node.h"
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>

extern int yylex();
void yyerror(char *s);

%}

%union {
    char text[256];             /* char* name */
    Node *nPtr;             /* node pointer */
};

%token <text> NUMBER
%token <text> STRING
%token <text> IDENTIFIER
//%token <value> FUNCTION
//%token <value> PROCEDURE
%token PROCEDURE

%left '='
%left AND OR
%left GE LE EQUALS NE '>' '<'
%left '+' '-' TCAT BCAT
%left '*' '/'
%nonassoc UMINUS '!'

%type <nPtr> stmt expr parm_list
%%

function:
          function stmt              { walkResults($2); delete $2; }
        | /* NULL */
        ;

stmt:
          expr                       { $$ = $1; }
        | expr '=' expr              { $$ = opr('=', $1, $3); }
        ;

parm_list: 
          expr                  { $$ = $1; } 
        | parm_list expr	{ $$ = opr(',', $1, $2); } 
        | /* NULL */		{ $$ = NULL; }
        ; 

expr:
          NUMBER                { $$ = id(NUMBER, $1); }
        | STRING                { $$ = id(STRING, $1); }
        | IDENTIFIER            { $$ = id(IDENTIFIER, $1); }
        | IDENTIFIER '(' parm_list ')' { $$ = procedure($1, $3); } 
        | '-' expr %prec UMINUS { $$ = opr(UMINUS, $2, NULL); }
        | expr '+' expr         { $$ = opr('+', $1, $3); }
        | expr '-' expr         { $$ = opr('-', $1, $3); }
        | expr TCAT expr	{ $$ = opr(TCAT, $1, $3); }
        | expr BCAT expr	{ $$ = opr(BCAT, $1, $3); }
        | expr '-' expr         { $$ = opr('-', $1, $3); }
        | expr '*' expr         { $$ = opr('*', $1, $3); }
        | expr '/' expr         { $$ = opr('/', $1, $3); }
        | expr '<' expr         { $$ = opr('<', $1, $3); }
        | expr '>' expr         { $$ = opr('>', $1, $3); }
        | expr GE expr          { $$ = opr(GE, $1, $3); }
        | expr LE expr          { $$ = opr(LE, $1, $3); }
        | expr NE expr          { $$ = opr(NE, $1, $3); }
        | expr EQUALS expr      { $$ = opr(EQUALS, $1, $3); }
        | expr OR expr          { $$ = opr(OR, $1, $3); }
        | expr AND expr         { $$ = opr(AND, $1, $3); }
        | '!' expr              { $$ = opr('!', $2, NULL); }
        | '(' expr ')'          { $2->_isBracketed = true; $$ = $2;}
        ;

%%

void yyerror(char *s) {
    fprintf(stdout, "%s\n", s);
}

int main(void) {
    yyparse();
    return 0;
}













