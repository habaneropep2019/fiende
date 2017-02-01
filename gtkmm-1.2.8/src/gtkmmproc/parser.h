typedef union {
  int ival;
  string* sval;
  Argument* vval;
  ArgList* aval;
  FunctionDef* fval;
} YYSTYPE;
#define	CLASS	257
#define	WRAPCLASSDECL	258
#define	WRAPMETHODDECL	259
#define	SIGNALDECL	260
#define	ENDCLASSDEF	261
#define	CLASSSECTION	262
#define	DOCSECTION	263
#define	PRIVATESECTION	264
#define	IMPLSECTION	265
#define	CONST	266
#define	VOLATILE	267
#define	STATIC	268
#define	FIXMEGTKCONST	269
#define	WRAPCTORCAST	270
#define	WRAPCTORDEFAULT	271
#define	WRAPDTOR	272
#define	WRAPMEMBERDECL	273
#define	SYMNAME	274
#define	NUMBER	275
#define	TYPESPECIFIER	276
#define	DUMMY	277


extern YYSTYPE yylval;
