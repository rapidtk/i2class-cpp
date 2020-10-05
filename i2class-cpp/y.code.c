#if defined(__STDC__) || defined(__cplusplus)
#define YYCONST const
#define YYPARAMS(x) x
#define YYDEFUN(name, arglist, args) name(args)
#define YYAND ,
#define YYPTR void *
#else
#define YYCONST
#define YYPARAMS(x) ()
#define YYDEFUN(name, arglist, args) name arglist args;
#define YYAND ;
#define YYPTR char *
#endif
#ifndef lint
YYCONST static char yysccsid[] = "@(#)yaccpar	1.8 (Berkeley +Cygnus.28) 01/20/91";
#endif
#define YYBYACC 1
#ifndef YYDONT_INCLUDE_STDIO
#include <stdio.h>
#endif
#ifdef __cplusplus
#include <stdlib.h> /* for malloc/realloc/free */
#endif
#line 1 "rpg.y"

#include "node.h"
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>

extern int yylex();
void yyerror(char *s);

#line 13 "rpg.y"
typedef union {
    char text[256];             /* char* name */
    Node *nPtr;             /* node pointer */
} YYSTYPE;
#line 40 "y.code.c"
#define NUMBER 257
#define STRING 258
#define IDENTIFIER 259
#define PROCEDURE 260
#define AND 261
#define OR 262
#define GE 263
#define LE 264
#define EQUALS 265
#define NE 266
#define UMINUS 267
#define YYERRCODE 256
#define YYTABLESIZE 457
#define YYFINAL 1
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 267
#if YYDEBUG
#endif
#define YYLEX yylex()
#define YYEMPTY -1
#define yyclearin (yychar=(YYEMPTY))
#define yyerrok (yyerrflag=0)
#ifndef YYINITDEPTH
#define YYINITDEPTH 200
#endif
#ifdef YYSTACKSIZE
#ifndef YYMAXDEPTH
#define YYMAXDEPTH YYSTACKSIZE
#endif
#else
#ifdef YYMAXDEPTH
#define YYSTACKSIZE YYMAXDEPTH
#else
#define YYSTACKSIZE 500
#define YYMAXDEPTH 500
#endif
#endif
#ifndef YYMAXSTACKSIZE
#define YYMAXSTACKSIZE 10000
#endif
int yydebug;
int yynerrs;
int yyerrflag;
int yychar;
YYSTYPE yyval;
YYSTYPE yylval;
static short *yyss;
static YYSTYPE *yyvs;
static int yystacksize;
#define yyfree(x) free(x)
extern int yylex();

static YYPTR
YYDEFUN (yymalloc, (bytes), unsigned bytes)
{
    YYPTR ptr = (YYPTR) malloc (bytes);
    if (ptr != 0) return (ptr);
    yyerror ("yyparse: memory exhausted");
    return (0);
}

static YYPTR
YYDEFUN (yyrealloc, (old, bytes), YYPTR old YYAND unsigned bytes)
{
    YYPTR ptr = (YYPTR) realloc (old, bytes);
    if (ptr != 0) return (ptr);
    yyerror ("yyparse: memory exhausted");
    return (0);
}

static int
#ifdef __GNUC__
__inline__
#endif
yygrow ()
{
    int old_stacksize = yystacksize;
    short *new_yyss;
    YYSTYPE *new_yyvs;

    if (yystacksize == YYMAXSTACKSIZE)
        return (1);
    yystacksize += (yystacksize + 1 ) / 2;
    if (yystacksize > YYMAXSTACKSIZE)
        yystacksize = YYMAXSTACKSIZE;
#if YYDEBUG
    if (yydebug)
        printf("yydebug: growing stack size from %d to %d\n",
               old_stacksize, yystacksize);
#endif
    new_yyss = (short *) yyrealloc ((char *)yyss, yystacksize * sizeof (short));
    if (new_yyss == 0)
        return (1);
    new_yyvs = (YYSTYPE *) yyrealloc ((char *)yyvs, yystacksize * sizeof (YYSTYPE));
    if (new_yyvs == 0)
    {
        yyfree (new_yyss);
        return (1);
    }
    yyss = new_yyss;
    yyvs = new_yyvs;
    return (0);
}
#line 74 "rpg.y"


void yyerror(char *s) {
    fprintf(stdout, "%s\n", s);
}

int main(void) {
    yyparse();
    return 0;
}

#line 158 "y.code.c"
#define YYABORT goto yyabort
#define YYACCEPT goto yyaccept
#define YYERROR goto yyerrlab

#if YYDEBUG
#ifdef __cplusplus
extern "C" char *getenv();
#else
extern char *getenv();
#endif
#endif

int
yyparse()
{
    register int yym, yyn, yystate;
    register YYSTYPE *yyvsp;
    register short *yyssp;
    short *yysse;
#if YYDEBUG
    register YYCONST char *yys;

    if (yys = getenv("YYDEBUG"))
    {
        yyn = *yys;
        if (yyn >= '0' && yyn <= '9')
            yydebug = yyn - '0';
    }
#endif

    yynerrs = 0;
    yyerrflag = 0;
    yychar = (-1);

    if (yyss == 0)
    {
        yyss = (short *) yymalloc (YYSTACKSIZE * sizeof (short));
        if (yyss == 0)
          goto yyabort;
        yyvs = (YYSTYPE *) yymalloc (YYSTACKSIZE * sizeof (YYSTYPE));
        if (yyvs == 0)
        {
            yyfree (yyss);
            goto yyabort;
        }
        yystacksize = YYSTACKSIZE;
    }
    yysse = yyss + yystacksize - 1;
    yyssp = yyss;
    yyvsp = yyvs;
    *yyssp = yystate = 0;
    goto yyloop;

yypush_lex:
    yyval = yylval;
    yystate = yytable[yyn];
yypush:
    if (yyssp >= yysse)
    {
        int depth = yyssp - yyss;
        if (yygrow() != 0)
             goto yyoverflow;
        yysse = yyss + yystacksize -1;
        yyssp = depth + yyss;
        yyvsp = depth + yyvs;
    }
    *++yyssp = yystate;
    *++yyvsp = yyval;

yyloop:
    if (yyn = yydefred[yystate]) goto yyreduce;
    yyn = yysindex[yystate];
    if (yychar < 0)
    {
        if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("yydebug: state %d, reading %d (%s)\n", yystate,
                    yychar, yys);
        }
#endif
    }
    if (yyn != 0
        && ((yyn += yychar), ((unsigned)yyn <= (unsigned)YYTABLESIZE))
        && yycheck[yyn] == yychar)
    {
#if YYDEBUG
        if (yydebug)
            printf("yydebug: state %d, shifting to state %d\n",
                    yystate, yytable[yyn]);
#endif
        if (yyerrflag > 0)  --yyerrflag;
        yychar = (-1);
        goto yypush_lex;
    }
    yyn = yyrindex[yystate];
    if (yyn != 0
        && ((yyn += yychar), ((unsigned)yyn <= (unsigned)YYTABLESIZE))
        && yycheck[yyn] == yychar)
    {
        yyn = yytable[yyn];
        goto yyreduce;
    }
    if (yyerrflag) goto yyinrecovery;
#ifdef lint
    goto yynewerror;
#endif
yynewerror:
    yyerror("syntax error");
#ifdef lint
    goto yyerrlab;
#endif
yyerrlab:
    ++yynerrs;
yyinrecovery:
    if (yyerrflag < 3)
    {
        yyerrflag = 3;
        for (;;)
        {
            yyn = yysindex[*yyssp];
            if (yyn != 0
                && ((yyn += YYERRCODE), ((unsigned)yyn <= (unsigned)YYTABLESIZE))
                && yycheck[yyn] == YYERRCODE)
            {
#if YYDEBUG
                if (yydebug)
                    printf("yydebug: state %d, error recovery shifting\
 to state %d\n", *yyssp, yytable[yyn]);
#endif
                goto yypush_lex;
            }
            else
            {
#if YYDEBUG
                if (yydebug)
                    printf("yydebug: error recovery discarding state %d\n",
                            *yyssp);
#endif
                if (yyssp <= yyss) goto yyabort;
                --yyssp;
                --yyvsp;
            }
        }
    }
    else
    {
        if (yychar == 0) goto yyabort;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("yydebug: state %d, error recovery discards token %d (%s)\n",
                    yystate, yychar, yys);
        }
#endif
        yychar = (-1);
        goto yyloop;
    }
yyreduce:
#if YYDEBUG
    if (yydebug)
        printf("yydebug: state %d, reducing by rule %d (%s)\n",
                yystate, yyn, yyrule[yyn]);
#endif
    yym = yylen[yyn];
    yyval = yyvsp[1-yym];
    switch (yyn)
    {
case 1:
#line 36 "rpg.y"
{ walkResults(yyvsp[0].nPtr); delete yyvsp[0].nPtr; }
break;
case 3:
#line 41 "rpg.y"
{ yyval.nPtr = yyvsp[0].nPtr; }
break;
case 4:
#line 42 "rpg.y"
{ yyval.nPtr = opr('=', yyvsp[-2].nPtr, yyvsp[0].nPtr); }
break;
case 5:
#line 46 "rpg.y"
{ yyval.nPtr = yyvsp[0].nPtr; }
break;
case 6:
#line 47 "rpg.y"
{ yyval.nPtr = opr(',', yyvsp[-2].nPtr, yyvsp[0].nPtr); }
break;
case 7:
#line 48 "rpg.y"
{ yyval.nPtr = NULL; }
break;
case 8:
#line 52 "rpg.y"
{ yyval.nPtr = id(NUMBER, yyvsp[0].text); }
break;
case 9:
#line 53 "rpg.y"
{ yyval.nPtr = id(STRING, yyvsp[0].text); }
break;
case 10:
#line 54 "rpg.y"
{ yyval.nPtr = id(IDENTIFIER, yyvsp[0].text); }
break;
case 11:
#line 55 "rpg.y"
{ yyval.nPtr = procedure(yyvsp[-3].text, yyvsp[-1].nPtr); }
break;
case 12:
#line 56 "rpg.y"
{ yyval.nPtr = opr(UMINUS, yyvsp[0].nPtr, NULL); }
break;
case 13:
#line 57 "rpg.y"
{ yyval.nPtr = opr('+', yyvsp[-2].nPtr, yyvsp[0].nPtr); }
break;
case 14:
#line 58 "rpg.y"
{ yyval.nPtr = opr('-', yyvsp[-2].nPtr, yyvsp[0].nPtr); }
break;
case 15:
#line 59 "rpg.y"
{ yyval.nPtr = opr('*', yyvsp[-2].nPtr, yyvsp[0].nPtr); }
break;
case 16:
#line 60 "rpg.y"
{ yyval.nPtr = opr('/', yyvsp[-2].nPtr, yyvsp[0].nPtr); }
break;
case 17:
#line 61 "rpg.y"
{ yyval.nPtr = opr('<', yyvsp[-2].nPtr, yyvsp[0].nPtr); }
break;
case 18:
#line 62 "rpg.y"
{ yyval.nPtr = opr('>', yyvsp[-2].nPtr, yyvsp[0].nPtr); }
break;
case 19:
#line 63 "rpg.y"
{ yyval.nPtr = opr(GE, yyvsp[-2].nPtr, yyvsp[0].nPtr); }
break;
case 20:
#line 64 "rpg.y"
{ yyval.nPtr = opr(LE, yyvsp[-2].nPtr, yyvsp[0].nPtr); }
break;
case 21:
#line 65 "rpg.y"
{ yyval.nPtr = opr(NE, yyvsp[-2].nPtr, yyvsp[0].nPtr); }
break;
case 22:
#line 67 "rpg.y"
{ yyval.nPtr = opr(EQUALS, yyvsp[-2].nPtr, yyvsp[0].nPtr); }
break;
case 23:
#line 68 "rpg.y"
{ yyval.nPtr = opr(OR, yyvsp[-2].nPtr, yyvsp[0].nPtr); }
break;
case 24:
#line 69 "rpg.y"
{ yyval.nPtr = opr(AND, yyvsp[-2].nPtr, yyvsp[0].nPtr); }
break;
case 25:
#line 70 "rpg.y"
{ yyval.nPtr = opr('!', yyvsp[0].nPtr, NULL); }
break;
case 26:
#line 71 "rpg.y"
{ yyvsp[-1].nPtr->_isBracketed = true; yyval.nPtr = yyvsp[-1].nPtr;}
break;
#line 434 "y.code.c"
    }
    yyssp -= yym;
    yyvsp -= yym;
    yym = yylhs[yyn];
    yystate = *yyssp;
    if (yystate == 0 && yym == 0)
    {
#if YYDEBUG
        if (yydebug)
            printf("yydebug: after reduction, shifting from state 0 to\
 state %d\n", YYFINAL);
#endif
        yystate = YYFINAL;
        *++yyssp = YYFINAL;
        *++yyvsp = yyval;
        if (yychar < 0)
        {
            if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
            if (yydebug)
            {
                yys = 0;
                if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
                if (!yys) yys = "illegal-symbol";
                printf("yydebug: state %d, reading %d (%s)\n",
                        YYFINAL, yychar, yys);
            }
#endif
        }
        if (yychar == 0) goto yyaccept;
        goto yyloop;
    }
    yyn = yygindex[yym];
	 if (yyn != 0
        && ((yyn += yystate), ((unsigned)yyn <= (unsigned)YYTABLESIZE))
        && yycheck[yyn] == yystate)
        yystate = yytable[yyn];
    else
        yystate = yydgoto[yym];
#if YYDEBUG
    if (yydebug)
        printf("yydebug: after reduction, shifting from state %d \
to state %d\n", *yyssp, yystate);
#endif
    goto yypush;
yyoverflow:
    yyerror("yacc stack overflow");
yyabort:
    return (1);
yyaccept:
    return (0);
}
