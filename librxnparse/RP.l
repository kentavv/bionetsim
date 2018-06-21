%option noyywrap
%option never-interactive

%{
#include <string.h>
#include "RP_AstNode.h"
#include "RP_yacc.h"

extern int RPyyinput(char *, int);

using namespace std;

#undef YY_INPUT
#define YY_INPUT(b, r, ms) (r = RPyyinput(b, ms))

int RPyyline = 1;
%}

yyDIGIT [0-9]
yyINTNUM {yyDIGIT}+

%x COMMENT
%x STRING

%%

"/*" BEGIN(COMMENT);

<COMMENT>[^*\n]*
<COMMENT>"*"+[^*/\n]*
<COMMENT>\n RPyyline++;
<COMMENT>"*"+"/" BEGIN(INITIAL);

"//".* {}

"\n" {
    RPyyline++;
    return TOK_NEW_LINE;
}

":" {
    return TOK_COLON;
}

"=" {
    return TOK_ASSIGN;
}

"[" {
    return TOK_LBRACE;
}

"]" {
    return TOK_RBRACE;
}

"," {
    return TOK_COMMA;
}

"+" {
    return TOK_PLUS;
}

"0" {
    return TOK_ZERO;
}

"-->" |
"->" {
    return TOK_RXN_ARROW;
}

"<-->" | 
"<->" {
    return TOK_RXN_ARROW_REV;
}

\(/[A-Za-z]([A-Za-z0-9_\-: ]*[A-Za-z0-9_\-]|[A-Za-z0-9_\-]*)\) {
    BEGIN(STRING);
}

<STRING>\) {
    BEGIN(INITIAL);
}

<STRING>[A-Za-z]([A-Za-z0-9_\-: ]*[A-Za-z0-9_\-:]|[A-Za-z0-9_\-:]*) {
    RPlval.strVal = new char[strlen(yytext)+1];
    strcpy(RPlval.strVal, yytext);
    return TOK_ID;
}

[A-Za-z]([A-Za-z0-9_\-:]*[A-Za-z0-9_\-]|[A-Za-z0-9_\-]*) {
    RPlval.strVal = new char[strlen(yytext)+1];
    strcpy(RPlval.strVal, yytext);
    return TOK_ID;
}

[1-9][0-9]* {
    RPlval.intVal = atol(yytext);
    return TOK_INT_CONST;
}

{yyINTNUM}\. |
{yyINTNUM}?\.{yyINTNUM} |
({yyINTNUM}?)\.({yyINTNUM}?)[eE]([-+]?){yyINTNUM} {
    sscanf(yytext, "%lf", &RPlval.floatVal);
    return TOK_FLOAT_CONST;
}

[^ \n\t\r] {
    RPlval.strVal=yytext;
    return TOK_SYNTAX_ERROR;
}

. {}

%%
