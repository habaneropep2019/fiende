
/*  A Bison parser, made from ./ac-query-parse.y
    by GNU Bison version 1.28  */

#define YYBISON 1  /* Identify Bison output.  */

#define	P_CONST_STRING	257
#define	P_CONST_NUMBER	258
#define	P_CONST_BOOLEAN	259
#define	P_CONST_ID	260
#define	LPAREN	261
#define	RPAREN	262
#define	LBRACKET	263
#define	RBRACKET	264
#define	P_NOT	265
#define	P_NEGATE	266
#define	P_DOLLAR	267
#define	P_MULTIPLY	268
#define	P_DIVIDE	269
#define	P_ADD	270
#define	P_SUBTRACT	271
#define	P_EQ	272
#define	P_NEQ	273
#define	P_LEQ	274
#define	P_GEQ	275
#define	P_LT	276
#define	P_GT	277
#define	P_OR	278
#define	P_AND	279
#define	P_XOR	280
#define	COMMA	281
#define	PERIOD	282
#define	PARSE_ERROR	283

#line 25 "./ac-query-parse.y"

#include <glib.h>

#include "ac-query-expr.h"
#include <stdlib.h>

void yyerror(char *s);
int yylex ();
int yyparse (void);
void initFlex (const char *s);

static QueryExpr *parsed_expression;

#line 39 "./ac-query-parse.y"
typedef union
{
  char *val_string;
  char **val_stringv;
  gdouble val_number;
  gboolean val_boolean;
  QueryExpr *qexp;
  GSList *elist;
  int val_enum;
} YYSTYPE;
#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		60
#define	YYFLAG		-32768
#define	YYNTBASE	30

#define YYTRANSLATE(x) ((unsigned)(x) <= 283 ? yytranslate[x] : 44)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     1,     3,     4,     5,     6,
     7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
    17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
    27,    28,    29
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     2,     4,     6,     8,    12,    14,    16,    18,    20,
    22,    24,    28,    30,    32,    34,    36,    38,    40,    42,
    44,    46,    48,    50,    52,    54,    58,    61,    64,    66,
    68,    70,    72,    76,    78,    82,    85,    90,    94,   101,
   107
};

static const short yyrhs[] = {    31,
     0,    36,     0,    33,     0,    31,     0,    31,    27,    32,
     0,    37,     0,    34,     0,    38,     0,    41,     0,    42,
     0,    43,     0,     7,    31,     8,     0,    14,     0,    15,
     0,    17,     0,    16,     0,    18,     0,    19,     0,    20,
     0,    21,     0,    23,     0,    22,     0,    24,     0,    25,
     0,    26,     0,    33,    35,    31,     0,    11,    33,     0,
    17,    33,     0,     3,     0,     4,     0,     5,     0,    39,
     0,     9,    40,    10,     0,     3,     0,    40,    27,     3,
     0,    13,     6,     0,     6,     7,    32,     8,     0,     6,
     7,     8,     0,     6,    28,     6,     7,    32,     8,     0,
     6,    28,     6,     7,     8,     0,     6,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
    82,    84,    85,    87,    88,    90,    90,    90,    90,    90,
    90,    92,    94,    95,    96,    97,    98,    99,   100,   101,
   102,   103,   104,   105,   106,   108,   110,   111,   113,   119,
   125,   131,   138,   155,   156,   158,   160,   161,   162,   164,
   167
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","P_CONST_STRING",
"P_CONST_NUMBER","P_CONST_BOOLEAN","P_CONST_ID","LPAREN","RPAREN","LBRACKET",
"RBRACKET","P_NOT","P_NEGATE","P_DOLLAR","P_MULTIPLY","P_DIVIDE","P_ADD","P_SUBTRACT",
"P_EQ","P_NEQ","P_LEQ","P_GEQ","P_LT","P_GT","P_OR","P_AND","P_XOR","COMMA",
"PERIOD","PARSE_ERROR","whole_expression","expr","exprlist","expr_obvious","expr_sub",
"binop","expr_binop","expr_unop","expr_constant","expr_stringv","stringlist",
"expr_variable","expr_function","expr_id", NULL
};
#endif

static const short yyr1[] = {     0,
    30,    31,    31,    32,    32,    33,    33,    33,    33,    33,
    33,    34,    35,    35,    35,    35,    35,    35,    35,    35,
    35,    35,    35,    35,    35,    36,    37,    37,    38,    38,
    38,    38,    39,    40,    40,    41,    42,    42,    42,    42,
    43
};

static const short yyr2[] = {     0,
     1,     1,     1,     1,     3,     1,     1,     1,     1,     1,
     1,     3,     1,     1,     1,     1,     1,     1,     1,     1,
     1,     1,     1,     1,     1,     3,     2,     2,     1,     1,
     1,     1,     3,     1,     3,     2,     4,     3,     6,     5,
     1
};

static const short yydefact[] = {     0,
    29,    30,    31,    41,     0,     0,     0,     0,     0,     1,
     3,     7,     2,     6,     8,    32,     9,    10,    11,     0,
     0,     0,    34,     0,    27,    36,    28,    13,    14,    16,
    15,    17,    18,    19,    20,    22,    21,    23,    24,    25,
     0,    38,     4,     0,     0,    12,    33,     0,    26,     0,
    37,     0,    35,     5,    40,     0,    39,     0,     0,     0
};

static const short yydefgoto[] = {    58,
    43,    44,    11,    12,    41,    13,    14,    15,    16,    24,
    17,    18,    19
};

static const short yypact[] = {    39,
-32768,-32768,-32768,    -6,    39,     1,    39,    -3,    39,-32768,
    43,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,     3,
    11,    10,-32768,    -8,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
    39,-32768,     7,    13,    28,-32768,-32768,    33,-32768,    39,
-32768,    20,-32768,-32768,-32768,    30,-32768,    40,    47,-32768
};

static const short yypgoto[] = {-32768,
     0,   -37,    23,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768
};


#define	YYLAST		69


static const short yytable[] = {    10,
    20,    47,    26,    23,    22,     1,     2,     3,     4,     5,
    42,     6,    54,     7,    56,     8,    45,    46,    48,     9,
    51,    21,     1,     2,     3,     4,     5,    55,     6,    25,
     7,    27,     8,    50,    52,    53,     9,    57,     0,    59,
    49,     1,     2,     3,     4,     5,    60,     6,     0,     7,
     0,     8,     0,     0,     0,     9,    28,    29,    30,    31,
    32,    33,    34,    35,    36,    37,    38,    39,    40
};

static const short yycheck[] = {     0,
     7,    10,     6,     3,     5,     3,     4,     5,     6,     7,
     8,     9,    50,    11,    52,    13,     6,     8,    27,    17,
     8,    28,     3,     4,     5,     6,     7,     8,     9,     7,
    11,     9,    13,    27,     7,     3,    17,     8,    -1,     0,
    41,     3,     4,     5,     6,     7,     0,     9,    -1,    11,
    -1,    13,    -1,    -1,    -1,    17,    14,    15,    16,    17,
    18,    19,    20,    21,    22,    23,    24,    25,    26
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/usr/lib/bison.simple"
/* This file comes from bison-1.28.  */

/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

#ifndef YYSTACK_USE_ALLOCA
#ifdef alloca
#define YYSTACK_USE_ALLOCA
#else /* alloca not defined */
#ifdef __GNUC__
#define YYSTACK_USE_ALLOCA
#define alloca __builtin_alloca
#else /* not GNU C.  */
#if (!defined (__STDC__) && defined (sparc)) || defined (__sparc__) || defined (__sparc) || defined (__sgi) || (defined (__sun) && defined (__i386))
#define YYSTACK_USE_ALLOCA
#include <alloca.h>
#else /* not sparc */
/* We think this test detects Watcom and Microsoft C.  */
/* This used to test MSDOS, but that is a bad idea
   since that symbol is in the user namespace.  */
#if (defined (_MSDOS) || defined (_MSDOS_)) && !defined (__TURBOC__)
#if 0 /* No need for malloc.h, which pollutes the namespace;
	 instead, just don't use alloca.  */
#include <malloc.h>
#endif
#else /* not MSDOS, or __TURBOC__ */
#if defined(_AIX)
/* I don't know what this was needed for, but it pollutes the namespace.
   So I turned it off.   rms, 2 May 1997.  */
/* #include <malloc.h>  */
 #pragma alloca
#define YYSTACK_USE_ALLOCA
#else /* not MSDOS, or __TURBOC__, or _AIX */
#if 0
#ifdef __hpux /* haible@ilog.fr says this works for HPUX 9.05 and up,
		 and on HPUX 10.  Eventually we can turn this on.  */
#define YYSTACK_USE_ALLOCA
#define alloca __builtin_alloca
#endif /* __hpux */
#endif
#endif /* not _AIX */
#endif /* not MSDOS, or __TURBOC__ */
#endif /* not sparc */
#endif /* not GNU C */
#endif /* alloca not defined */
#endif /* YYSTACK_USE_ALLOCA not defined */

#ifdef YYSTACK_USE_ALLOCA
#define YYSTACK_ALLOC alloca
#else
#define YYSTACK_ALLOC malloc
#endif

/* Note: there must be only one dollar sign in this file.
   It is replaced by the list of actions, each action
   as one case of the switch.  */

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	goto yyacceptlab
#define YYABORT 	goto yyabortlab
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(token, value) \
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    { yychar = (token), yylval = (value);			\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { yyerror ("syntax error: cannot back up"); YYERROR; }	\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

#ifndef YYPURE
#define YYLEX		yylex()
#endif

#ifdef YYPURE
#ifdef YYLSP_NEEDED
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, &yylloc, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval, &yylloc)
#endif
#else /* not YYLSP_NEEDED */
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval)
#endif
#endif /* not YYLSP_NEEDED */
#endif

/* If nonreentrant, generate the variables here */

#ifndef YYPURE

int	yychar;			/*  the lookahead symbol		*/
YYSTYPE	yylval;			/*  the semantic value of the		*/
				/*  lookahead symbol			*/

#ifdef YYLSP_NEEDED
YYLTYPE yylloc;			/*  location data for the lookahead	*/
				/*  symbol				*/
#endif

int yynerrs;			/*  number of parse errors so far       */
#endif  /* not YYPURE */

#if YYDEBUG != 0
int yydebug;			/*  nonzero means print parse trace	*/
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif

/*  YYINITDEPTH indicates the initial size of the parser's stacks	*/

#ifndef	YYINITDEPTH
#define YYINITDEPTH 200
#endif

/*  YYMAXDEPTH is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#if YYMAXDEPTH == 0
#undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif

/* Define __yy_memcpy.  Note that the size argument
   should be passed with type unsigned int, because that is what the non-GCC
   definitions require.  With GCC, __builtin_memcpy takes an arg
   of type size_t, but it can handle unsigned int.  */

#if __GNUC__ > 1		/* GNU C and GNU C++ define this.  */
#define __yy_memcpy(TO,FROM,COUNT)	__builtin_memcpy(TO,FROM,COUNT)
#else				/* not GNU C or C++ */
#ifndef __cplusplus

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (to, from, count)
     char *to;
     char *from;
     unsigned int count;
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#else /* __cplusplus */

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (char *to, char *from, unsigned int count)
{
  register char *t = to;
  register char *f = from;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#endif
#endif

#line 217 "/usr/lib/bison.simple"

/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
#ifdef __cplusplus
#define YYPARSE_PARAM_ARG void *YYPARSE_PARAM
#define YYPARSE_PARAM_DECL
#else /* not __cplusplus */
#define YYPARSE_PARAM_ARG YYPARSE_PARAM
#define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
#endif /* not __cplusplus */
#else /* not YYPARSE_PARAM */
#define YYPARSE_PARAM_ARG
#define YYPARSE_PARAM_DECL
#endif /* not YYPARSE_PARAM */

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
#ifdef YYPARSE_PARAM
int yyparse (void *);
#else
int yyparse (void);
#endif
#endif

int
yyparse(YYPARSE_PARAM_ARG)
     YYPARSE_PARAM_DECL
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YYSTYPE *yyvsp;
  int yyerrstatus;	/*  number of tokens to shift before error messages enabled */
  int yychar1 = 0;		/*  lookahead token as an internal (translated) token number */

  short	yyssa[YYINITDEPTH];	/*  the state stack			*/
  YYSTYPE yyvsa[YYINITDEPTH];	/*  the semantic value stack		*/

  short *yyss = yyssa;		/*  refer to the stacks thru separate pointers */
  YYSTYPE *yyvs = yyvsa;	/*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YYLSP_NEEDED
  YYLTYPE yylsa[YYINITDEPTH];	/*  the location stack			*/
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  int yystacksize = YYINITDEPTH;
  int yyfree_stacks = 0;

#ifdef YYPURE
  int yychar;
  YYSTYPE yylval;
  int yynerrs;
#ifdef YYLSP_NEEDED
  YYLTYPE yylloc;
#endif
#endif

  YYSTYPE yyval;		/*  the variable used to return		*/
				/*  semantic values from the action	*/
				/*  routines				*/

  int yylen;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Starting parse\n");
#endif

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
#ifdef YYLSP_NEEDED
  yylsp = yyls;
#endif

/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
yynewstate:

  *++yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YYSTYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YYLSP_NEEDED
      YYLTYPE *yyls1 = yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
#ifdef YYLSP_NEEDED
      /* This used to be a conditional around just the two extra args,
	 but that might be undefined if yyoverflow is a macro.  */
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yyls1, size * sizeof (*yylsp),
		 &yystacksize);
#else
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yystacksize);
#endif

      yyss = yyss1; yyvs = yyvs1;
#ifdef YYLSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	{
	  yyerror("parser stack overflow");
	  if (yyfree_stacks)
	    {
	      free (yyss);
	      free (yyvs);
#ifdef YYLSP_NEEDED
	      free (yyls);
#endif
	    }
	  return 2;
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
#ifndef YYSTACK_USE_ALLOCA
      yyfree_stacks = 1;
#endif
      yyss = (short *) YYSTACK_ALLOC (yystacksize * sizeof (*yyssp));
      __yy_memcpy ((char *)yyss, (char *)yyss1,
		   size * (unsigned int) sizeof (*yyssp));
      yyvs = (YYSTYPE *) YYSTACK_ALLOC (yystacksize * sizeof (*yyvsp));
      __yy_memcpy ((char *)yyvs, (char *)yyvs1,
		   size * (unsigned int) sizeof (*yyvsp));
#ifdef YYLSP_NEEDED
      yyls = (YYLTYPE *) YYSTACK_ALLOC (yystacksize * sizeof (*yylsp));
      __yy_memcpy ((char *)yyls, (char *)yyls1,
		   size * (unsigned int) sizeof (*yylsp));
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YYLSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Entering state %d\n", yystate);
#endif

  goto yybackup;
 yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Reading a token: ");
#endif
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(yychar);

#if YYDEBUG != 0
      if (yydebug)
	{
	  fprintf (stderr, "Next token is %d (%s", yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
#endif
	  fprintf (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting token %d (%s), ", yychar, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (yyerrstatus) yyerrstatus--;

  yystate = yyn;
  goto yynewstate;

/* Do the default action for the current state.  */
yydefault:

  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;

/* Do a reduction.  yyn is the number of a rule to reduce with.  */
yyreduce:
  yylen = yyr2[yyn];
  if (yylen > 0)
    yyval = yyvsp[1-yylen]; /* implement default value of the action */

#if YYDEBUG != 0
  if (yydebug)
    {
      int i;

      fprintf (stderr, "Reducing via rule %d (line %d), ",
	       yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (i = yyprhs[yyn]; yyrhs[i] > 0; i++)
	fprintf (stderr, "%s ", yytname[yyrhs[i]]);
      fprintf (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif


  switch (yyn) {

case 1:
#line 82 "./ac-query-parse.y"
{ parsed_expression = yyvsp[0].qexp; ;
    break;}
case 4:
#line 87 "./ac-query-parse.y"
{ yyval.elist = g_slist_prepend(NULL, yyvsp[0].qexp); ;
    break;}
case 5:
#line 88 "./ac-query-parse.y"
{ yyval.elist = g_slist_prepend(yyvsp[0].elist, yyvsp[-2].qexp); ;
    break;}
case 12:
#line 92 "./ac-query-parse.y"
{ yyval.qexp = yyvsp[-1].qexp; ;
    break;}
case 13:
#line 94 "./ac-query-parse.y"
{ yyval.val_enum = P_MULTIPLY; ;
    break;}
case 14:
#line 95 "./ac-query-parse.y"
{ yyval.val_enum = P_DIVIDE; ;
    break;}
case 15:
#line 96 "./ac-query-parse.y"
{ yyval.val_enum = P_SUBTRACT; ;
    break;}
case 16:
#line 97 "./ac-query-parse.y"
{ yyval.val_enum = P_ADD; ;
    break;}
case 17:
#line 98 "./ac-query-parse.y"
{ yyval.val_enum = P_EQ; ;
    break;}
case 18:
#line 99 "./ac-query-parse.y"
{ yyval.val_enum = P_NEQ; ;
    break;}
case 19:
#line 100 "./ac-query-parse.y"
{ yyval.val_enum = P_LEQ; ;
    break;}
case 20:
#line 101 "./ac-query-parse.y"
{ yyval.val_enum = P_GEQ; ;
    break;}
case 21:
#line 102 "./ac-query-parse.y"
{ yyval.val_enum = P_GT; ;
    break;}
case 22:
#line 103 "./ac-query-parse.y"
{ yyval.val_enum = P_LT; ;
    break;}
case 23:
#line 104 "./ac-query-parse.y"
{ yyval.val_enum = P_OR; ;
    break;}
case 24:
#line 105 "./ac-query-parse.y"
{ yyval.val_enum = P_AND; ;
    break;}
case 25:
#line 106 "./ac-query-parse.y"
{ yyval.val_enum = P_XOR; ;
    break;}
case 26:
#line 108 "./ac-query-parse.y"
{ yyval.qexp = qexp_binop_new (yyvsp[-2].qexp, yyvsp[-1].val_enum, yyvsp[0].qexp); ;
    break;}
case 27:
#line 110 "./ac-query-parse.y"
{ yyval.qexp = qexp_unop_new (P_NOT, yyvsp[0].qexp); ;
    break;}
case 28:
#line 111 "./ac-query-parse.y"
{ yyval.qexp = qexp_unop_new (P_NEGATE, yyvsp[0].qexp); ;
    break;}
case 29:
#line 113 "./ac-query-parse.y"
{
	  QueryExprConst qctmp;
	  qctmp.type = CONST_STRING;
	  qctmp.u.v_string = yyvsp[0].val_string;
	  yyval.qexp = qexp_constant_new (qctmp);
        ;
    break;}
case 30:
#line 119 "./ac-query-parse.y"
{
	  QueryExprConst qctmp;
	  qctmp.type = CONST_NUMBER;
	  qctmp.u.v_number = yyvsp[0].val_number;
	  yyval.qexp = qexp_constant_new (qctmp);
	;
    break;}
case 31:
#line 125 "./ac-query-parse.y"
{
	  QueryExprConst qctmp;
	  qctmp.type = CONST_BOOLEAN;
	  qctmp.u.v_boolean = yyvsp[0].val_boolean;
	  yyval.qexp = qexp_constant_new (qctmp);
	;
    break;}
case 32:
#line 131 "./ac-query-parse.y"
{
	  QueryExprConst qctmp;
	  qctmp.type = CONST_STRINGV;
	  qctmp.u.v_stringv = yyvsp[0].val_stringv;
	  yyval.qexp = qexp_constant_new (qctmp);
	;
    break;}
case 33:
#line 138 "./ac-query-parse.y"
{
  char **new_stringv;
  int i, n;
  GSList *cur;

  n = g_slist_length(yyvsp[-1].elist);
  new_stringv = g_new (char *, n + 1);
  for (cur = yyvsp[-1].elist, i = 0; i < n; i++, cur = cur->next) {
    new_stringv[i] = cur->data;
  }
  new_stringv[i] = NULL;

  g_slist_free (yyvsp[-1].elist);

  yyval.val_stringv = new_stringv;
;
    break;}
case 34:
#line 155 "./ac-query-parse.y"
{ yyval.elist = g_slist_prepend (NULL, yyvsp[0].val_string); ;
    break;}
case 35:
#line 156 "./ac-query-parse.y"
{ yyval.elist = g_slist_append (yyvsp[-2].elist, yyvsp[0].val_string); ;
    break;}
case 36:
#line 158 "./ac-query-parse.y"
{ yyval.qexp = qexp_variable_new (yyvsp[0].val_string); ;
    break;}
case 37:
#line 160 "./ac-query-parse.y"
{ yyval.qexp = qexp_function_new (yyvsp[-3].val_string, yyvsp[-1].elist); ;
    break;}
case 38:
#line 161 "./ac-query-parse.y"
{ yyval.qexp = qexp_function_new (yyvsp[-2].val_string, NULL); ;
    break;}
case 39:
#line 162 "./ac-query-parse.y"
{
	yyval.qexp = qexp_function_new(yyvsp[-3].val_string, g_slist_prepend (yyvsp[-1].elist, qexp_id_new (yyvsp[-5].val_string))); ;
    break;}
case 40:
#line 164 "./ac-query-parse.y"
{
	yyval.qexp = qexp_function_new(yyvsp[-2].val_string, g_slist_prepend (NULL, qexp_id_new (yyvsp[-4].val_string))); ;
    break;}
case 41:
#line 167 "./ac-query-parse.y"
{ yyval.qexp = qexp_id_new (yyvsp[0].val_string); ;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 543 "/usr/lib/bison.simple"

  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YYLSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = yylloc.first_line;
      yylsp->first_column = yylloc.first_column;
      yylsp->last_line = (yylsp-1)->last_line;
      yylsp->last_column = (yylsp-1)->last_column;
      yylsp->text = 0;
    }
  else
    {
      yylsp->last_line = (yylsp+yylen-1)->last_line;
      yylsp->last_column = (yylsp+yylen-1)->last_column;
    }
#endif

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;

yyerrlab:   /* here on detecting error */

  if (! yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  int size = 0;
	  char *msg;
	  int x, count;

	  count = 0;
	  /* Start X at -yyn if nec to avoid negative indexes in yycheck.  */
	  for (x = (yyn < 0 ? -yyn : 0);
	       x < (sizeof(yytname) / sizeof(char *)); x++)
	    if (yycheck[x + yyn] == x)
	      size += strlen(yytname[x]) + 15, count++;
	  msg = (char *) malloc(size + 15);
	  if (msg != 0)
	    {
	      strcpy(msg, "parse error");

	      if (count < 5)
		{
		  count = 0;
		  for (x = (yyn < 0 ? -yyn : 0);
		       x < (sizeof(yytname) / sizeof(char *)); x++)
		    if (yycheck[x + yyn] == x)
		      {
			strcat(msg, count == 0 ? ", expecting `" : " or `");
			strcat(msg, yytname[x]);
			strcat(msg, "'");
			count++;
		      }
		}
	      yyerror(msg);
	      free(msg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror("parse error");
    }

  goto yyerrlab1;
yyerrlab1:   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Discarding token %d (%s).\n", yychar, yytname[yychar1]);
#endif

      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;

yyerrdefault:  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (yyn) goto yydefault;
#endif

yyerrpop:   /* pop the current state because it cannot handle the error token */

  if (yyssp == yyss) YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#ifdef YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

yyerrhandle:

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;

 yyacceptlab:
  /* YYACCEPT comes here.  */
  if (yyfree_stacks)
    {
      free (yyss);
      free (yyvs);
#ifdef YYLSP_NEEDED
      free (yyls);
#endif
    }
  return 0;

 yyabortlab:
  /* YYABORT comes here.  */
  if (yyfree_stacks)
    {
      free (yyss);
      free (yyvs);
#ifdef YYLSP_NEEDED
      free (yyls);
#endif
    }
  return 1;
}
#line 169 "./ac-query-parse.y"


static GString *parse_errors = NULL;

void yyerror (char *s)
{
  g_string_append (parse_errors, s);
  g_string_append_c (parse_errors, '\n');
}

const char *qexp_parse (const char *_code, 
			QueryExpr **retme)
{
  parsed_expression = NULL;

  g_assert (retme);

  if (!parse_errors)
    parse_errors = g_string_new (NULL);
  else
    g_string_truncate (parse_errors, 0);

  initFlex (_code);
  yyparse();

  *retme = parsed_expression;

  if (parse_errors->len)
    return parse_errors->str;
  else
    return NULL;
}
