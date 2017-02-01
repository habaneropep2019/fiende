%option noyywrap
%option yylineno
%option stack

%{

// $Id: lexer.ll,v 1.18 2000/09/12 21:28:03 kenelson Exp $

//#define LEXER_TEST

#include <fstream>
#include <unistd.h>
#include <string>
#include <math.h>

#include "gtkmmproc.h"
#include "parser.h"

#ifdef LEXER_TEST

// We now get the symbols from the parse even though the lexer
// test does not use it.  However, remember to add new parser
// symbols to the lexer test program below.

YYSTYPE yylval;

extern "C"
{
void yyerror(char *s);
}

#else

extern "C"
{
void yyerror(char *s);
int yylex();
int yyparse();
}

#endif

/* initial code */
float GtkVersion=1.1;
int gtk_cond_if=false;
int gtk_cond_print=false;

int macro_depth = 0;
int par_depth = 0;
int brace_depth = 0;

/* pattern declarations */
%}

string  \"[^\n"]+\"
symbols \*|&|<|>|,|\.|=|\|

ws    [ \t]+
wsn [ \t]*

alpha   [A-Za-z]
dig     [0-9]

ccomment   \/\*
cppcomment \/\/.*$

 // Symbol name
symname [a-zA-Z_][a-zA-Z_0-9:]+|[a-zA-Z_]
gtkcast [A-Z_]+

num1    [-+]?{dig}+\.?([eE][-+]?{dig}+)?
num2    [-+]?{dig}*\.{dig}+([eE][-+]?{dig}+)?
num3    0x[0-9a-fA-F]+
number  {num1}|{num2}|{num3}
version {dig}+\.{dig}+

ifgtk ^{wsn}#ifgtk{ws}
elsegtk ^{wsn}#elsegtk
endifgtk ^{wsn}#endifgtk


 // Class definition/declaration
classdef class
classderivetype public|protected|private

typespecifier    int|bool|void|char|float|double|short|long|signed|unsigned

wrapclassdecl  ^{wsn}WRAP_CLASS
wrapmethoddecl ^{wsn}WRAP_METHOD
wrapmemberdecl ^{wsn}WRAP_MEMBER
wrapnomethdecl ^{wsn}WRAP_NO_METHOD

wrapctordefault ^{wsn}WRAP_CTOR_DEFAULT{wsn};
wrapctorcast ^{wsn}WRAP_CTOR_CAST{wsn};
wrapdtor ^{wsn}WRAP_DTOR{wsn};

signaldecl ^{wsn}SIGNAL_SPEC


 // Define states
%x CCOMMENT
%x CPPCOMMENT
%x MACROS
%x MACROLINE

%s GTK_COND_IF
%s GTK_COND_WEED

%x DECL_CHAR
%x DECL_STRING
%x WRAP_CHAR
%x WRAP_STRING

%s IN_WRAP
%s IN_DECL
%s IN_PASS
%s END_DECL

%s CLASS_SECTION
%s CLASS_SECTION_START
%s DOCUMENTATION_SECTION
%s PRIVATE_SECTION
%s IMPL_SECTION


%%


%{ // We need a line marking in the front of every line 
%}
^(.|{ws}) { yyless(0); fprintf(yyout,"#L %d __source__\n",yylineno); }

%{ // Macro escape
%}
<MACROS> 
  {
    ^"#m4end" { yy_pop_state(); }
    \`.*\'   { ECHO; }
    .+ { ECHO; }
  }

<MACROLINE>
  {
   [^\n]*        ECHO;
   "\n"          ECHO; yy_pop_state(); 
  }

^"#m4 " {
    yy_push_state(MACROLINE);
  }

^"#m4start" {
    yy_push_state(MACROS);
  }

%{ // Trouble character  (crashes m4)
%}
"`"  { fputc('\'',yyout); }


%{ // Comment handling  
%}

{ccomment}  {
       ECHO; 
       yy_push_state(CCOMMENT);
     }

<CCOMMENT>
  {
   "`"            fputc('\'',yyout);
   [^*`]*         ECHO;
   "*"+[^*`/]*    ECHO;
   "*"+"/"        ECHO; yy_pop_state();
  }

{cppcomment} {
       ECHO;
       yy_push_state(CPPCOMMENT);
     }

<CPPCOMMENT>
  {
   "`"            fputc('\'',yyout);
   [^`\n]*        ECHO;
   "\n"           ECHO; yy_pop_state(); yylineno--;
  }



%{ // Version control
%}

<GTK_COND_IF>
  {
   {version}  {
       gtk_cond_if=true;
       yy_pop_state();
       if ( fabs(atof(yytext)-GtkVersion) < 0.001 )
         {
           gtk_cond_print=true; 
         }
       else
         {
           yy_push_state(GTK_COND_WEED);    
         }
      }
   .             yyerror("bad gtk version");
  }

<GTK_COND_WEED>
  {
   {ifgtk}    {yy_pop_state(); yy_push_state(GTK_COND_IF);}
   {elsegtk}  {if (!gtk_cond_print) {yy_pop_state(); gtk_cond_print=true;} }
   {endifgtk} {
       yy_pop_state();  
       gtk_cond_if=false;  
       gtk_cond_print=false;
     }
   "\n"       ECHO;
   [^\n]*     
  }

{ifgtk}   {yy_push_state(GTK_COND_IF);}

{elsegtk} {
    if (!gtk_cond_if)  yyerror("#elsegtk encountered without #ifgtk");
    yy_push_state(GTK_COND_WEED);
  }

{endifgtk} {
    if (!gtk_cond_if)  yyerror("#endgtk encountered without #ifgtk");
    gtk_cond_if=false;
    gtk_cond_print=false;
  }



%{ // Declarations
%}
<DECL_STRING>{
    {symname} {
       yylval.sval = new string(yytext);
       return SYMNAME;
     }
    \\.  {printf(yytext);}
    "\"" {yy_pop_state();return '\"';}
    .    {printf(yytext);}
  }

<DECL_CHAR>{
    \\.  {printf(yytext);}
    "\'" {yy_pop_state();return '\'';}
    .    {printf(yytext);}
  }

<IN_PASS>{
   \{|\} {yyerror("braces in declaration.");}

   "(" {
        ECHO;
        if (!par_depth)
          {
           fprintf(yyout,"`");
          }
        par_depth++;
       }

   "," {
        if (par_depth==1)
          {
           fprintf(yyout,"',`");
          }
        else
          ECHO;
       }

   ")" {
       par_depth--;
       if (!par_depth) 
         {yy_pop_state();
          yy_push_state(END_DECL);
          fprintf(yyout,"')");
         }
       else
         ECHO;
     }
   
   . {ECHO;}
  }

<IN_DECL>{
   \{|\} {yyerror("braces in declaration.");}

   "(" {
       par_depth++;
       return '(';
     }

   ")" {
       par_depth--;
       if (!par_depth) 
         {yy_pop_state();
          yy_push_state(END_DECL);
          return ')';
         }
       return ')';
     }
   
   "\"" {
       yy_push_state(DECL_STRING);
       return '\"';
     }

   "const"    { return CONST; }
   "volatile" { return VOLATILE; }
   "static"   { return STATIC; }
   "fixmegtkconst" { return FIXMEGTKCONST; }
 
   {typespecifier} {
      yylval.sval = new string(yytext);
      return TYPESPECIFIER;
     }

   {symbols} {
       return yytext[0];
     }

   {symname} {
       yylval.sval = new string(yytext);
       return SYMNAME;
     }

   {number} {
       yylval.sval = new string(yytext);
       return NUMBER;
     }

   {ws}
   "\n"  ECHO;

   . {printf(yytext);}
  }

<END_DECL>{
    ";" {yy_pop_state();}
    {ws}
    .   {yyerror("declaration missing ;");}
  }



%{ // Declarations
%}
<WRAP_STRING>{
    \\.  {ECHO;}
    "\"" {ECHO; yy_pop_state();}
    .    {ECHO;}
  }

<WRAP_CHAR>{
    \\.  {ECHO;}
    "\'" {ECHO; yy_pop_state();}
    .    {ECHO;}
  }

<IN_WRAP>{
    "\'" {
        ECHO; 
        yy_push_state(WRAP_CHAR);
      }

    "\"" {
        ECHO; 
        yy_push_state(WRAP_STRING);
      }

    "{" {
        brace_depth++;
        ECHO; 
      }

    "}" {
        if (brace_depth) brace_depth--;
        if (!brace_depth)
          {
           yy_pop_state();
           return ENDCLASSDEF;
          }
        ECHO; 
      }

    {wrapnomethdecl}   {
       fprintf(yyout,"GTKMM_METHOD_PASS");
       yy_push_state(IN_PASS);
      }

    {signaldecl}        {yy_push_state(IN_DECL);return SIGNALDECL;} 
    {wrapmethoddecl}    {yy_push_state(IN_DECL);return WRAPMETHODDECL;}
    {wrapmemberdecl}    {yy_push_state(IN_DECL);return WRAPMEMBERDECL;} 

    {wrapctorcast}      {return WRAPCTORCAST;}
    {wrapctordefault}   {return WRAPCTORDEFAULT;}
    {wrapdtor}          {return WRAPDTOR;}
    {wrapclassdecl}     {yyerror("duplicate WRAP_CLASS");}
  }


%{ // Section specific marks
%}
<CLASS_SECTION_START>{
   "("  { return '('; }
   ")"  { return ')'; }
   ";\n"  { yy_pop_state(); yy_push_state(CLASS_SECTION); }
 
   {wsn}  {}

   {symname} {
       yylval.sval = new string(yytext);
       return SYMNAME;
     }
    
  }


<CLASS_SECTION>{wrapclassdecl} {
   yy_push_state(IN_WRAP);
   yy_push_state(IN_DECL);
   par_depth=0;
   brace_depth=1;
   return WRAPCLASSDECL;
  }

%{ // Whitespace
%}
{wsn}\n { fputc('\n',yyout); } // kill trailing ws
{ws}       {ECHO;}            // rest falls through


%{ // Handle the various xx_START marks
%}
PRIVATE_START{wsn};\n       {
    yy_push_state(PRIVATE_SECTION);
    return PRIVATESECTION;
  }
IMPL_START{wsn};\n          {
    yy_push_state(IMPL_SECTION);
    return IMPLSECTION;
  }
DOCUMENTATION_START{wsn};\n {
    yy_push_state(DOCUMENTATION_SECTION);
    return DOCSECTION;
  }
CLASS_START         {
    yy_push_state(CLASS_SECTION_START);
    return CLASSSECTION;
  } 


%%



void yyerror(char *s)
{
  fprintf(stderr, "error at line %u : %s\n", yylineno, s);
  exit(EXIT_FAILURE);
}

void yywarning(char *s)
{
  fprintf(stderr, "warning at line %u : %s\n", yylineno, s);
}


#ifdef LEXER_TEST
int main( int  argc, char** argv )
  {
    int token;
    if (argc!=3) exit(-1);
    yyin=fopen(argv[1],"r");
    yyout=fopen(argv[2],"w");

    while((token=yylex()) != 0)
      {
       switch (token)
         {
          case CLASSSECTION:
            printf("CLASSSECTION:\n");
            break;
          case DOCSECTION:
            printf("DOCSECTION:\n");
            break;
          case PRIVATESECTION:
            printf("PRIVATESECTION:\n");
            break;
          case IMPLSECTION:
            printf("IMPLSECTION:\n");
            break;

          case WRAPCLASSDECL: 
            printf("WRAPCLASSDECL ");
            break;
          case WRAPMETHODDECL: 
            printf("WRAPMETHODDECL ");
            break;

          case WRAPCTORCAST:
            printf("WRAPCTORCAST\n");
            break;
          case WRAPCTORDEFAULT:
            printf("WRAPCTORDEFAULT\n");
            break;
          case WRAPDTOR:
            printf("WRAPDTOR\n");
            break;

          case SIGNALDECL: 
            printf("SIGNALDECL ");
            break;
          case CONST: 
            printf("CONST ");
            break;
          case FIXMEGTKCONST:
            printf("FIXMEGTKCONST ");
            break;
          case TYPESPECIFIER: 
            printf("TYPE ");
            break;
          case SYMNAME: 
            printf("SYM ");
            break;
          case NUMBER: 
            printf("NUM ");
            break;
          case ')': 
            printf(") ");
            if (!par_depth) printf("\n");
            break; 
          case ENDCLASSDEF:
            printf("ENDCLASS\n");
            break; 
          default:
            if (token<255)
              printf("%c ",token);
            else 
              printf("<<%d>> ",token);
         }
      };
    return 0;
  }
#endif
