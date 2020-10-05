#ifndef BISON__RPG_TAB_HPP
# define BISON__RPG_TAB_HPP

#ifndef YYSTYPE
typedef union {
    char text[256];             /* char* name */
    Node *nPtr;             /* node pointer */
} yystype;
# define YYSTYPE yystype
# define YYSTYPE_IS_TRIVIAL 1
#endif
# define	NUMBER	257
# define	STRING	258
# define	IDENTIFIER	259
# define	PROCEDURE	260
# define	AND	261
# define	OR	262
# define	GE	263
# define	LE	264
# define	EQUALS	265
# define	NE	266
# define	UMINUS	267


extern YYSTYPE yylval;

#endif /* not BISON__RPG_TAB_HPP */
