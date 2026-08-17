typedef union {
    char text[256];             /* char* name */
    Node *nPtr;             /* node pointer */
} YYSTYPE;
#define	NUMBER	258
#define	STRING	259
#define	IDENTIFIER	260
#define	PROCEDURE	261
#define	AND	262
#define	OR	263
#define	GE	264
#define	LE	265
#define	EQUALS	266
#define	NE	267
#define	TCAT	268
#define	TCAT	268
#define	BCAT	269
#define	UMINUS	270


extern YYSTYPE yylval;
