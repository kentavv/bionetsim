#ifndef lint
static const char yysccsid[] = "@(#)yaccpar	1.9 (Berkeley) 02/21/93";
#endif

#include <stdlib.h>
#include <string.h>

#define YYBYACC 1
#define YYMAJOR 1
#define YYMINOR 9
#define YYPATCH 20070509

#define YYEMPTY (-1)
#define yyclearin    (yychar = YYEMPTY)
#define yyerrok      (yyerrflag = 0)
#define YYRECOVERING (yyerrflag != 0)

extern int yyparse(void);

static int yygrowstack(void);
#define yyparse RPparse
#define yylex RPlex
#define yyerror RPerror
#define yychar RPchar
#define yyval RPval
#define yylval RPlval
#define yydebug RPdebug
#define yynerrs RPnerrs
#define yyerrflag RPerrflag
#define yyss RPss
#define yyssp RPssp
#define yyvs RPvs
#define yyvsp RPvsp
#define yylhs RPlhs
#define yylen RPlen
#define yydefred RPdefred
#define yydgoto RPdgoto
#define yysindex RPsindex
#define yyrindex RPrindex
#define yygindex RPgindex
#define yytable RPtable
#define yycheck RPcheck
#define yyname RPname
#define yyrule RPrule
#define YYPREFIX "RP"
#line 1 "RP.y"

#include <string>
#include <list>
#include <iostream>

#include <string.h>
#include <stdio.h>

#include "RP_AstNode.h"

using namespace std;

extern int verbosity;
extern int yylex();
extern void yyerror(char *s);
extern int RPyyline;

int RP_rxn_num = 0;

list<ReactionNode> RP_top;
list<pair<string, double> > RP_top2;
#line 26 "RP.y"
typedef union {
char *strVal;
  int   intVal;
  double floatVal;
  string *stringVal;
  pair<double, double> *RxnRateVal;
  ReactionNode *ReactionVal;
  list<ReactionNode> *ReactionListVal;
  list<pair<int, string> > *MoleculeSumVal;
} YYSTYPE;
#line 80 "RP.tab.c"
#define TOK_NEW_LINE 257
#define TOK_ID 258
#define TOK_INT_CONST 259
#define TOK_FLOAT_CONST 260
#define TOK_LBRACE 261
#define TOK_RBRACE 262
#define TOK_RXN_ARROW 263
#define TOK_RXN_ARROW_REV 264
#define TOK_COLON 265
#define TOK_PLUS 266
#define TOK_COMMA 267
#define TOK_ASSIGN 268
#define TOK_ZERO 269
#define TOK_SYNTAX_ERROR 270
#define YYERRCODE 256
short RPlhs[] = {                                        -1,
    0,    0,   10,   10,   10,   10,   11,   12,   12,   12,
   12,   12,   12,    1,    2,    2,    3,    3,    3,    3,
    4,    5,    6,    6,    7,    7,    7,    8,    8,    8,
    9,
};
short RPlen[] = {                                         2,
    0,    2,    0,    1,    2,    2,    3,    6,    6,    6,
    4,    4,    4,    1,    0,    1,    2,    1,    4,    3,
    1,    1,    1,    1,    0,    3,    5,    1,    1,    1,
    1,
};
short RPdefred[] = {                                      1,
    0,    4,   31,   21,    0,    0,    0,    0,    0,    0,
    2,    0,    0,    0,   23,   24,    0,    0,    0,    0,
   17,   22,    0,    5,    6,    0,    0,   18,    0,    0,
    0,   20,    0,    0,   29,   30,   28,    7,    0,    0,
    0,    0,   12,   13,   19,   11,    0,    0,    0,    0,
    9,   10,    8,   26,    0,    0,   27,
};
short RPdgoto[] = {                                       1,
    5,    6,   34,    8,   28,   17,   43,   38,   22,   11,
   12,   13,
};
short RPsindex[] = {                                      0,
 -192,    0,    0,    0, -258, -195, -211, -243, -251,    0,
    0, -237, -235, -224,    0,    0, -224, -244, -224, -224,
    0,    0, -229,    0,    0, -195, -206,    0, -255, -216,
 -243,    0, -216, -227,    0,    0,    0,    0, -224, -232,
 -224, -229,    0,    0,    0,    0, -255, -216, -216, -248,
    0,    0,    0,    0, -229, -203,    0,
};
short RPrindex[] = {                                      0,
    1,    0,    0,    0,    0,    0,    0,    0, -202, -222,
    0,    0,    0, -193,    0,    0,    0, -210,    0, -225,
    0,    0,    0,    0,    0,    0,    0,    0, -235, -235,
    0,    0, -235, -207,    0,    0,    0,    0,    0, -210,
 -225,    0,    0,    0,    0,    0, -235, -235, -235,    0,
    0,    0,    0,    0,    0,    0,    0,
};
short RPgindex[] = {                                      0,
    0,  -12,   -1,   37,    2,   -3,  -21,  -37,   62,    0,
    0,    0,
};
#define YYTABLESIZE 265
short RPtable[] = {                                       7,
    3,   26,    9,   20,   50,   42,   14,   33,   44,   21,
   19,   46,   27,   54,    3,   29,   23,   56,   55,   24,
   32,   25,   39,   41,   30,   51,   52,   53,   49,   35,
   36,   15,   45,    3,    4,   15,   48,   47,   19,   37,
   22,   22,   14,   22,   42,   22,   23,   23,   23,   16,
   23,   18,   16,   16,   19,   31,   40,   16,   57,   19,
   18,   18,   10,   18,    2,    3,    4,   15,   16,   15,
   15,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    3,    3,
};
short RPcheck[] = {                                       1,
    0,   14,    1,    7,   42,  261,  265,   20,   30,    8,
  266,   33,   14,  262,  258,   17,  268,   55,  267,  257,
   19,  257,   26,   27,  269,   47,   48,   49,   41,  259,
  260,  257,   31,  258,  259,  261,  269,   39,  266,  269,
  263,  264,  265,  266,  261,  268,  257,  258,  259,  257,
  261,  263,  264,  261,  266,   19,  263,  264,  262,  266,
  263,  264,    1,  266,  257,  258,  259,  263,  264,  263,
  264,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,  263,  264,
};
#define YYFINAL 1
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 270
#if YYDEBUG
char *RPname[] = {
"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"TOK_NEW_LINE","TOK_ID",
"TOK_INT_CONST","TOK_FLOAT_CONST","TOK_LBRACE","TOK_RBRACE","TOK_RXN_ARROW",
"TOK_RXN_ARROW_REV","TOK_COLON","TOK_PLUS","TOK_COMMA","TOK_ASSIGN","TOK_ZERO",
"TOK_SYNTAX_ERROR",
};
char *RPrule[] = {
"$accept : equations",
"equations :",
"equations : equations equation",
"equation :",
"equation : TOK_NEW_LINE",
"equation : assignment TOK_NEW_LINE",
"equation : reaction TOK_NEW_LINE",
"assignment : mol_name TOK_ASSIGN number",
"reaction : rxn_name TOK_COLON ne_mol_sum rxn_dir mol_sum rxn_rate",
"reaction : rxn_name TOK_COLON mol_sum rxn_dir ne_mol_sum rxn_rate",
"reaction : rxn_name TOK_COLON ne_mol_sum TOK_RXN_ARROW TOK_ZERO rxn_rate",
"reaction : ne_mol_sum rxn_dir mol_sum rxn_rate",
"reaction : mol_sum rxn_dir ne_mol_sum rxn_rate",
"reaction : ne_mol_sum TOK_RXN_ARROW TOK_ZERO rxn_rate",
"rxn_name : name",
"mol_sum :",
"mol_sum : ne_mol_sum",
"ne_mol_sum : coeff mol_name",
"ne_mol_sum : mol_name",
"ne_mol_sum : ne_mol_sum TOK_PLUS coeff mol_name",
"ne_mol_sum : ne_mol_sum TOK_PLUS mol_name",
"coeff : TOK_INT_CONST",
"mol_name : name",
"rxn_dir : TOK_RXN_ARROW",
"rxn_dir : TOK_RXN_ARROW_REV",
"rxn_rate :",
"rxn_rate : TOK_LBRACE number TOK_RBRACE",
"rxn_rate : TOK_LBRACE number TOK_COMMA number TOK_RBRACE",
"number : TOK_ZERO",
"number : TOK_INT_CONST",
"number : TOK_FLOAT_CONST",
"name : TOK_ID",
};
#endif
#if YYDEBUG
#include <stdio.h>
#endif

/* define the initial stack-sizes */
#ifdef YYSTACKSIZE
#undef YYMAXDEPTH
#define YYMAXDEPTH  YYSTACKSIZE
#else
#ifdef YYMAXDEPTH
#define YYSTACKSIZE YYMAXDEPTH
#else
#define YYSTACKSIZE 500
#define YYMAXDEPTH  500
#endif
#endif

#define YYINITSTACKSIZE 500

int      yydebug;
int      yynerrs;
int      yyerrflag;
int      yychar;
short   *yyssp;
YYSTYPE *yyvsp;
YYSTYPE  yyval;
YYSTYPE  yylval;

/* variables for the parser stack */
static short   *yyss;
static short   *yysslim;
static YYSTYPE *yyvs;
static int      yystacksize;
#line 187 "RP.y"

#line 287 "RP.tab.c"
/* allocate initial stack or double stack size, up to YYMAXDEPTH */
static int yygrowstack(void)
{
    int newsize, i;
    short *newss;
    YYSTYPE *newvs;

    if ((newsize = yystacksize) == 0)
        newsize = YYINITSTACKSIZE;
    else if (newsize >= YYMAXDEPTH)
        return -1;
    else if ((newsize *= 2) > YYMAXDEPTH)
        newsize = YYMAXDEPTH;

    i = yyssp - yyss;
    newss = (yyss != 0)
          ? (short *)realloc(yyss, newsize * sizeof(*newss))
          : (short *)malloc(newsize * sizeof(*newss));
    if (newss == 0)
        return -1;

    yyss  = newss;
    yyssp = newss + i;
    newvs = (yyvs != 0)
          ? (YYSTYPE *)realloc(yyvs, newsize * sizeof(*newvs))
          : (YYSTYPE *)malloc(newsize * sizeof(*newvs));
    if (newvs == 0)
        return -1;

    yyvs = newvs;
    yyvsp = newvs + i;
    yystacksize = newsize;
    yysslim = yyss + newsize - 1;
    return 0;
}

#define YYABORT goto yyabort
#define YYREJECT goto yyabort
#define YYACCEPT goto yyaccept
#define YYERROR goto yyerrlab
int
yyparse(void)
{
    register int yym, yyn, yystate;
#if YYDEBUG
    register const char *yys;

    if ((yys = getenv("YYDEBUG")) != 0)
    {
        yyn = *yys;
        if (yyn >= '0' && yyn <= '9')
            yydebug = yyn - '0';
    }
#endif

    yynerrs = 0;
    yyerrflag = 0;
    yychar = YYEMPTY;

    if (yyss == NULL && yygrowstack()) goto yyoverflow;
    yyssp = yyss;
    yyvsp = yyvs;
    *yyssp = yystate = 0;

yyloop:
    if ((yyn = yydefred[yystate]) != 0) goto yyreduce;
    if (yychar < 0)
    {
        if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("%sdebug: state %d, reading %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
    }
    if ((yyn = yysindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: state %d, shifting to state %d\n",
                    YYPREFIX, yystate, yytable[yyn]);
#endif
        if (yyssp >= yysslim && yygrowstack())
        {
            goto yyoverflow;
        }
        *++yyssp = yystate = yytable[yyn];
        *++yyvsp = yylval;
        yychar = YYEMPTY;
        if (yyerrflag > 0)  --yyerrflag;
        goto yyloop;
    }
    if ((yyn = yyrindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
        yyn = yytable[yyn];
        goto yyreduce;
    }
    if (yyerrflag) goto yyinrecovery;

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
            if ((yyn = yysindex[*yyssp]) && (yyn += YYERRCODE) >= 0 &&
                    yyn <= YYTABLESIZE && yycheck[yyn] == YYERRCODE)
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: state %d, error recovery shifting\
 to state %d\n", YYPREFIX, *yyssp, yytable[yyn]);
#endif
                if (yyssp >= yysslim && yygrowstack())
                {
                    goto yyoverflow;
                }
                *++yyssp = yystate = yytable[yyn];
                *++yyvsp = yylval;
                goto yyloop;
            }
            else
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: error recovery discarding state %d\n",
                            YYPREFIX, *yyssp);
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
            printf("%sdebug: state %d, error recovery discards token %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
        yychar = YYEMPTY;
        goto yyloop;
    }

yyreduce:
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: state %d, reducing by rule %d (%s)\n",
                YYPREFIX, yystate, yyn, yyrule[yyn]);
#endif
    yym = yylen[yyn];
    if (yym)
        yyval = yyvsp[1-yym];
    else
        memset(&yyval, 0, sizeof yyval);
    switch (yyn)
    {
case 1:
#line 68 "RP.y"
{}
break;
case 2:
#line 69 "RP.y"
{}
break;
case 4:
#line 72 "RP.y"
{}
break;
case 5:
#line 73 "RP.y"
{}
break;
case 6:
#line 74 "RP.y"
{}
break;
case 7:
#line 77 "RP.y"
{
	  RP_top2.push_back(make_pair(*yyvsp[-2].stringVal, yyvsp[0].floatVal));
	  delete yyvsp[-2].stringVal;
	}
break;
case 8:
#line 83 "RP.y"
{
	  RP_top.push_back(ReactionNode(RPyyline, yyvsp[-5].stringVal, yyvsp[-3].MoleculeSumVal, yyvsp[-2].intVal, yyvsp[-1].MoleculeSumVal, yyvsp[0].RxnRateVal));
	  if(yyvsp[-5].stringVal) delete yyvsp[-5].stringVal;
	  if(yyvsp[-3].MoleculeSumVal) delete yyvsp[-3].MoleculeSumVal;
	  if(yyvsp[-1].MoleculeSumVal) delete yyvsp[-1].MoleculeSumVal;
	  if(yyvsp[0].RxnRateVal) delete yyvsp[0].RxnRateVal;
	}
break;
case 9:
#line 90 "RP.y"
{
	  RP_top.push_back(ReactionNode(RPyyline, yyvsp[-5].stringVal, yyvsp[-3].MoleculeSumVal, yyvsp[-2].intVal, yyvsp[-1].MoleculeSumVal, yyvsp[0].RxnRateVal));
	  if(yyvsp[-5].stringVal) delete yyvsp[-5].stringVal;
	  if(yyvsp[-3].MoleculeSumVal) delete yyvsp[-3].MoleculeSumVal;
	  if(yyvsp[-1].MoleculeSumVal) delete yyvsp[-1].MoleculeSumVal;
	  if(yyvsp[0].RxnRateVal) delete yyvsp[0].RxnRateVal;
	}
break;
case 10:
#line 97 "RP.y"
{
	  RP_top.push_back(ReactionNode(RPyyline, yyvsp[-5].stringVal, yyvsp[-3].MoleculeSumVal, 0, 0, yyvsp[0].RxnRateVal));
	  if(yyvsp[-5].stringVal) delete yyvsp[-5].stringVal;
	  if(yyvsp[-3].MoleculeSumVal) delete yyvsp[-3].MoleculeSumVal;
	  if(yyvsp[0].RxnRateVal) delete yyvsp[0].RxnRateVal;
	}
break;
case 11:
#line 106 "RP.y"
{
	  char str[64];
	  sprintf(str, "R%d", ++RP_rxn_num);
	  string str2 = str;

	  RP_top.push_back(ReactionNode(RPyyline, &str2, yyvsp[-3].MoleculeSumVal, yyvsp[-2].intVal, yyvsp[-1].MoleculeSumVal, yyvsp[0].RxnRateVal));
	  if(yyvsp[-3].MoleculeSumVal) delete yyvsp[-3].MoleculeSumVal;
	  if(yyvsp[-1].MoleculeSumVal) delete yyvsp[-1].MoleculeSumVal;
	  if(yyvsp[0].RxnRateVal) delete yyvsp[0].RxnRateVal;
	}
break;
case 12:
#line 116 "RP.y"
{
	  char str[64];
	  sprintf(str, "R%d", ++RP_rxn_num);
	  string str2 = str;

	  RP_top.push_back(ReactionNode(RPyyline, &str2, yyvsp[-3].MoleculeSumVal, yyvsp[-2].intVal, yyvsp[-1].MoleculeSumVal, yyvsp[0].RxnRateVal));
	  if(yyvsp[-3].MoleculeSumVal) delete yyvsp[-3].MoleculeSumVal;
	  if(yyvsp[-1].MoleculeSumVal) delete yyvsp[-1].MoleculeSumVal;
	  if(yyvsp[0].RxnRateVal) delete yyvsp[0].RxnRateVal;
	}
break;
case 13:
#line 126 "RP.y"
{
	  char str[64];
	  sprintf(str, "R%d", ++RP_rxn_num);
	  string str2 = str;

	  RP_top.push_back(ReactionNode(RPyyline, &str2, yyvsp[-3].MoleculeSumVal, 0, 0, yyvsp[0].RxnRateVal));
	  if(yyvsp[-3].MoleculeSumVal) delete yyvsp[-3].MoleculeSumVal;
	  if(yyvsp[0].RxnRateVal) delete yyvsp[0].RxnRateVal;
	}
break;
case 14:
#line 137 "RP.y"
{yyval.stringVal = yyvsp[0].stringVal;}
break;
case 15:
#line 140 "RP.y"
{yyval.MoleculeSumVal = 0;}
break;
case 16:
#line 141 "RP.y"
{ yyval.MoleculeSumVal = yyvsp[0].MoleculeSumVal; }
break;
case 17:
#line 144 "RP.y"
{
	  list<pair<int, string> > *lst = new list<pair<int, string> >;
	  lst->push_back(make_pair(yyvsp[-1].intVal, *yyvsp[0].stringVal));
	  yyval.MoleculeSumVal = lst; 
	}
break;
case 18:
#line 149 "RP.y"
{
	  list<pair<int, string> > *lst = new list<pair<int, string> >;
	  lst->push_back(make_pair(1, *yyvsp[0].stringVal));
	  yyval.MoleculeSumVal = lst; 
	}
break;
case 19:
#line 154 "RP.y"
{ 
	  yyvsp[-3].MoleculeSumVal->push_back(make_pair(yyvsp[-1].intVal, *yyvsp[0].stringVal));
	  yyval.MoleculeSumVal = yyvsp[-3].MoleculeSumVal;
	}
break;
case 20:
#line 158 "RP.y"
{ 
	  yyvsp[-2].MoleculeSumVal->push_back(make_pair(1, *yyvsp[0].stringVal));
	  yyval.MoleculeSumVal = yyvsp[-2].MoleculeSumVal;
	}
break;
case 21:
#line 164 "RP.y"
{yyval.intVal = yylval.intVal;}
break;
case 22:
#line 167 "RP.y"
{ yyval.stringVal = yyvsp[0].stringVal; }
break;
case 23:
#line 170 "RP.y"
{yyval.intVal = 0;}
break;
case 24:
#line 171 "RP.y"
{yyval.intVal = 1;}
break;
case 25:
#line 174 "RP.y"
{yyval.RxnRateVal = new pair<double, double>(1., 1.);}
break;
case 26:
#line 175 "RP.y"
{yyval.RxnRateVal = new pair<double, double>(yyvsp[-1].floatVal, 1.);}
break;
case 27:
#line 176 "RP.y"
{yyval.RxnRateVal = new pair<double, double>(yyvsp[-3].floatVal, yyvsp[-1].floatVal);}
break;
case 28:
#line 179 "RP.y"
{ yyval.floatVal = 0.0; }
break;
case 29:
#line 180 "RP.y"
{yyval.floatVal = double(yylval.intVal);}
break;
case 30:
#line 181 "RP.y"
{yyval.floatVal = yylval.floatVal;}
break;
case 31:
#line 184 "RP.y"
{
	  yyval.stringVal = new string(yylval.strVal);
	}
break;
#line 650 "RP.tab.c"
    }
    yyssp -= yym;
    yystate = *yyssp;
    yyvsp -= yym;
    yym = yylhs[yyn];
    if (yystate == 0 && yym == 0)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: after reduction, shifting from state 0 to\
 state %d\n", YYPREFIX, YYFINAL);
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
                printf("%sdebug: state %d, reading %d (%s)\n",
                        YYPREFIX, YYFINAL, yychar, yys);
            }
#endif
        }
        if (yychar == 0) goto yyaccept;
        goto yyloop;
    }
    if ((yyn = yygindex[yym]) && (yyn += yystate) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yystate)
        yystate = yytable[yyn];
    else
        yystate = yydgoto[yym];
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: after reduction, shifting from state %d \
to state %d\n", YYPREFIX, *yyssp, yystate);
#endif
    if (yyssp >= yysslim && yygrowstack())
    {
        goto yyoverflow;
    }
    *++yyssp = yystate;
    *++yyvsp = yyval;
    goto yyloop;

yyoverflow:
    yyerror("yacc stack overflow");

yyabort:
    return (1);

yyaccept:
    return (0);
}
