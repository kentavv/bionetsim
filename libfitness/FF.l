%option noyywrap
%option never-interactive

%{
#include <vector>

#include <string.h>
#include "AstNode.h"
#include "FF_yacc.h"

extern int my_yyinput(char *, int);

using namespace std;

#undef YY_INPUT
#define YY_INPUT(b, r, ms) (r = my_yyinput(b, ms))

int yyline = 1;
char str_buf[1024];
char *str_buf_p;

%}

yyDIGIT [0-9]
yyINTNUM {yyDIGIT}+

%x STRING
%x COMMENT

%%

"/*" BEGIN(COMMENT);

<COMMENT>[^*\n]*
<COMMENT>"*"+[^*/\n]*
<COMMENT>\n yyline++;
<COMMENT>"*"+"/" BEGIN(INITIAL);

"//".* {}

\"/(.*[^\\])?\" {
    str_buf_p=str_buf;
    *str_buf_p='\0';
    BEGIN (STRING);
}

<STRING>\" {
  *str_buf_p='\0';
  FFlval.strVal=str_buf;
  BEGIN(INITIAL);
  return TOK_STRING_CONST;
}

<STRING>\n {
  BEGIN(INITIAL);
}

<STRING>\\n {
  *str_buf_p++='\n';
}

<STRING>\\t {
  *str_buf_p++='\t';
}

<STRING>\\. {
  *str_buf_p++=yytext[1];
}

<STRING>[^\\\n\"]+ {
  char *p=yytext;
  while(*p)
    *str_buf_p++=*p++;
}

abs {
   return TOK_ABS;
}

acos {
   return TOK_ACOS;
}

asin {
   return TOK_ASIN;
}

atan {
   return TOK_ATAN;
}

cos {
   return TOK_COS;
}

sin {
   return TOK_SIN;
}

tan {
   return TOK_TAN;
}

exp {
   return TOK_EXP;
}

log {
   return TOK_LOG;
}

sqrt {
   return TOK_SQRT;
}

sqr {
   return TOK_SQR;
}

initial {
   return TOK_INITIAL;
}

kc {
   return TOK_KC;
}

result {
   return TOK_RESULT;
}

simulate {
   return TOK_SIMULATE;
}

boolean {
    return TOK_BOOLEAN;
}

break {
    return TOK_BREAK;
}

case {
    return TOK_CASE;
}

string {
    return TOK_STRING;
}

continue {
    return TOK_CONTINUE;
}

default {
    return TOK_DEFAULT;
}

do {
    return TOK_DO;
}

else {
    return TOK_ELSE;
}

false {
    return TOK_FALSE;
}

float {
    return TOK_FLOAT;
}

for {
    return TOK_FOR;
}

if {
    return TOK_IF;
}

int {
    return TOK_INT;
}

return {
    return TOK_RETURN;
}

switch {
    return TOK_SWITCH;
}

true {
    return TOK_TRUE;
}

void {
    return TOK_VOID;
}

while {
    return TOK_WHILE;
}

print {
    return TOK_PRINT;
}

"<<" {
    return TOK_LEFT_SHIFT;
}

"&&" {
    return TOK_AND;
}

"||" {
    return TOK_OR;
}

"++" {
    return TOK_PLUS_PLUS;
}

"--" {
    return TOK_MINUS_MINUS;
}

"==" {
    return TOK_EQ;
}

"!=" {
    return TOK_NE;
}

">=" {
    return TOK_GE;
}

"<=" {
    return TOK_LE;
}

\+   {
        return TOK_PLUS;
     }

\-   {
        return TOK_MINUS;
     }

\/   {
        return TOK_DIV;
     }

\*   {
        return TOK_MULT;
     }

\%   {
        return TOK_MOD;
     }

=    {
        return TOK_ASSIGN;
     }

\<   {
        return TOK_LT;
     }

\>   {
        return TOK_GT;
     }

\!   {
        return TOK_NOT;
     }

\^   {
       return TOK_EXPONENT;
     }

"{" {
    return TOK_LBRACE;
}

"}" {
    return TOK_RBRACE;
}

"[" {
    return TOK_LBRACK;
}

"]" {
    return TOK_RBRACK;
}

"(" {
    return TOK_LPAREN;
}

")" {
    return TOK_RPAREN;
}

"," {
    return TOK_COMMA;
}

[.] {
    return TOK_DOT;
}

[;] {
    return TOK_SEMICOLON;
}

[:] {
    return TOK_COLON;
}

"\n" {
    yyline++;
}

[A-Za-z][A-Za-z0-9_]* {
    FFlval.strVal = new char[strlen(yytext)+1];
    strcpy(FFlval.strVal, yytext);
    return TOK_ID;
}

{yyINTNUM} {
    FFlval.intVal = atol(yytext);
    return TOK_INT_CONST;
}

{yyINTNUM}\. |
{yyINTNUM}?\.{yyINTNUM} |
({yyINTNUM}?)\.({yyINTNUM}?)[eE]([-+]?){yyINTNUM} {
    sscanf(yytext, "%lf", &FFlval.floatVal);
    return TOK_FLOAT_CONST;
}

[^ \n\t\r] {
    FFlval.strVal=yytext;
    return TOK_SYNTAX_ERROR;
}

. {}

%%
