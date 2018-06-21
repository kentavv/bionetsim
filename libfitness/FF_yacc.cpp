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
#define yyparse FFparse
#define yylex FFlex
#define yyerror FFerror
#define yychar FFchar
#define yyval FFval
#define yylval FFlval
#define yydebug FFdebug
#define yynerrs FFnerrs
#define yyerrflag FFerrflag
#define yyss FFss
#define yyssp FFssp
#define yyvs FFvs
#define yyvsp FFvsp
#define yylhs FFlhs
#define yylen FFlen
#define yydefred FFdefred
#define yydgoto FFdgoto
#define yysindex FFsindex
#define yyrindex FFrindex
#define yygindex FFgindex
#define yytable FFtable
#define yycheck FFcheck
#define yyname FFname
#define yyrule FFrule
#define YYPREFIX "FF"
#line 2 "FF.y"
#include <vector>
#include <iostream>

#include <string.h>
#include <stdio.h>

#include "AstNode.h"

using namespace std;

extern int verbosity;
extern int yylex();
extern void yyerror(char *s);
extern int yyline;

TypeTag cur_type=ERROR_TYPE;
bool addThisToSymTab = false;
bool dup_method = false;

SeqNode *FF_top = NULL;
#line 25 "FF.y"
typedef union {
  char* strVal;
  int   intVal;
  double floatVal;
  vector<int> *vectorIntVal;
  AstNode *nodeVal; 
  TypeTag typeVal;
  SeqNode *seqNodeVal;
  BlockNode *blockNodeVal;
  RefExprNode *refExprNodeVal;
  VarNode *varNodeVal;
  MethodDefNode *methodDefNodeVal;
} YYSTYPE;
#line 82 "FF.tab.c"
#define TOK_SYNTAX_ERROR 257
#define TOK_STRING 258
#define TOK_BOOLEAN 259
#define TOK_BREAK 260
#define TOK_CASE 261
#define TOK_CONTINUE 262
#define TOK_DEFAULT 263
#define TOK_DO 264
#define TOK_INITIAL 265
#define TOK_ELSE 266
#define TOK_EXPONENT 267
#define TOK_FALSE 268
#define TOK_FLOAT 269
#define TOK_FOR 270
#define TOK_IF 271
#define TOK_INT 272
#define TOK_KC 273
#define TOK_PRINT 274
#define TOK_RESULT 275
#define TOK_RETURN 276
#define TOK_SIMULATE 277
#define TOK_SWITCH 278
#define TOK_TRUE 279
#define TOK_VOID 280
#define TOK_WHILE 281
#define TOK_ABS 282
#define TOK_ACOS 283
#define TOK_ASIN 284
#define TOK_ATAN 285
#define TOK_COS 286
#define TOK_SIN 287
#define TOK_TAN 288
#define TOK_EXP 289
#define TOK_LOG 290
#define TOK_SQRT 291
#define TOK_SQR 292
#define TOK_INT_CONST 293
#define TOK_FLOAT_CONST 294
#define TOK_STRING_CONST 295
#define TOK_ID 296
#define TOK_COLON 297
#define TOK_COMMA 298
#define TOK_DOT 299
#define TOK_SEMICOLON 300
#define TOK_LBRACK 301
#define TOK_RBRACK 302
#define TOK_LPAREN 303
#define TOK_RPAREN 304
#define TOK_LBRACE 305
#define TOK_RBRACE 306
#define TOK_LEFT_SHIFT 307
#define TOK_ASSIGN 308
#define TOK_OR 309
#define TOK_AND 310
#define TOK_EQ 311
#define TOK_NE 312
#define TOK_LT 313
#define TOK_GT 314
#define TOK_LE 315
#define TOK_GE 316
#define TOK_PLUS 317
#define TOK_MINUS 318
#define TOK_MULT 319
#define TOK_DIV 320
#define TOK_MOD 321
#define TOK_NOT 322
#define TOK_PLUS_PLUS 323
#define TOK_MINUS_MINUS 324
#define TOK_UMINUS 325
#define TOK_UPLUS 326
#define YYERRCODE 256
short FFlhs[] = {                                        -1,
    0,    0,   19,   19,   19,   19,   20,   20,    1,    2,
    3,    3,    3,    3,    4,    4,    5,    5,    6,    7,
   72,    8,    9,    9,   10,   10,   11,   13,   13,   14,
   14,   14,   14,   12,   12,   12,   12,   22,   23,   24,
   24,   18,   18,   18,   18,   18,   18,   18,   18,   18,
   18,   18,   68,   38,   21,   25,   26,   26,   27,   28,
   29,   30,   34,   34,   33,   33,   35,   36,   36,   37,
   37,   15,   15,   16,   17,   39,   40,   40,   41,   42,
   45,   45,   46,   46,   43,   43,   43,   43,   43,   43,
   43,   43,   43,   43,   43,   43,   43,   43,   43,   43,
   43,   43,   43,   43,   43,   43,   43,   43,   43,   43,
   43,   43,   43,   43,   43,   67,   67,   67,   67,   67,
   67,   67,   67,   67,   67,   67,   69,   70,   71,   71,
   44,   44,   44,   44,   44,   48,   49,   50,   51,   52,
   53,   54,   55,   56,   57,   58,   59,   60,   61,   62,
   63,   64,   65,   66,   66,   66,   66,   47,   31,   31,
   32,   32,
};
short FFlen[] = {                                         2,
    0,    2,    1,    1,    1,    1,    1,    1,    1,    1,
    2,    4,    4,    6,    0,    3,    1,    3,    5,    3,
    1,    2,    0,    1,    1,    3,    3,    2,    3,    1,
    1,    2,    2,    0,    1,    1,    2,    2,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    2,    1,
    1,    2,    3,    3,    1,    3,    2,    3,    7,   11,
    5,    9,    1,    1,    0,    1,    1,    0,    1,    1,
    3,    3,    2,    1,    1,    3,    1,    1,    4,    4,
    0,    1,    1,    3,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    3,    3,
    1,    1,    1,    1,    1,    4,    4,    4,    4,    4,
    4,    4,    4,    4,    4,    4,    4,    4,    6,    4,
    1,    1,    1,    1,    1,    2,    2,    3,    3,    3,
    3,    3,    3,    3,    3,    3,    3,    3,    3,    3,
    3,    2,    3,    2,    2,    2,    2,    4,    2,    3,
    2,    3,
};
short FFdefred[] = {                                      1,
    0,    5,    6,    4,    3,    8,    2,    0,    7,    0,
    0,    0,   25,    0,    0,    0,    0,   10,    0,   20,
   21,    0,   26,    0,   27,    0,    0,    0,    0,  135,
    0,    0,    0,    0,    0,    0,    0,  134,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  131,  133,  132,   75,    0,   55,    0,    0,    0,    0,
    0,    0,    0,   19,    0,    0,    0,   50,    0,   30,
   31,    0,    0,    0,    0,   42,   43,   44,   45,   46,
   47,   48,   51,  108,    0,   89,   78,    0,   85,   87,
  104,  105,   91,   92,   93,   94,   95,   96,   97,  101,
  100,  103,  102,   98,   99,  111,  106,  107,   88,  115,
   90,    0,    0,  114,   17,    0,   52,    0,  159,    0,
  161,    0,    0,    0,    0,    0,    0,    0,   73,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,  137,  136,    0,    9,  156,
  157,    0,    0,   22,    0,    0,   32,   33,   49,    0,
    0,   28,    0,    0,    0,  154,  155,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,   16,  160,  162,    0,   70,   63,
    0,   64,   67,    0,    0,    0,    0,   56,    0,    0,
   72,   53,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  151,    0,    0,    0,    0,   29,
    0,   54,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,   18,  127,    0,    0,    0,  128,    0,    0,  130,
    0,  116,  117,  118,  119,  120,  121,  122,  123,  124,
  125,  126,   80,    0,   79,  158,    0,    0,    0,    0,
   71,    0,    0,   61,    0,    0,    0,    0,    0,  129,
    0,    0,    0,    0,    0,    0,   62,    0,    0,   60,
};
short FFdgoto[] = {                                       1,
   62,   63,  164,   25,  116,    7,    8,   64,   11,   12,
   13,   65,   66,   67,   68,   69,   70,   71,  130,   10,
   73,   74,   75,    0,   76,  127,   77,   78,   79,   80,
   81,   82,  269,  191,  192,  193,  194,   83,   84,   85,
   86,   87,   88,   89,  217,  218,   90,   91,   92,   93,
   94,   95,   96,   97,   98,   99,  100,  101,  102,  103,
  104,  105,  106,  107,  108,  109,  110,  111,  112,  113,
  114,   22,
};
short FFsindex[] = {                                      0,
 -244,    0,    0,    0,    0,    0,    0, -240,    0, -242,
 -266, -182,    0, -239, -183, -181, -240,    0, -245,    0,
    0,   76,    0, -165,    0, -169, -260, -258, -171,    0,
 -151, -138, -134, -145, -131,  -98, -130,    0, -120, -104,
 -103, -100,  -99,  -87,  -85,  -82,  -81,  -80,  -76,  -75,
    0,    0,    0,    0,  137,    0,  137,  137,  137, -239,
 -239,  -90,  -73,    0,  -72,   76,   76,    0,  -69,    0,
    0,  -70,   76,  -65, -239,    0,    0,    0,    0,    0,
    0,    0,    0,    0, -297,    0,    0,  785,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,  -67,  -63,    0,    0, -268,    0,  -64,    0,  -52,
    0,  137,  137,  137,  137,  137, -249,  137,    0,  -70,
  277,  -42,  137,  137,  137,  137,  137,  137,  137,  137,
  137,  137,  137,  137,  305,    0,    0,  -30,    0,    0,
    0,  137,  137,    0,   76,  -36,    0,    0,    0,  137,
  -33,    0, -245,  -55,  137,    0,    0,  137,  137,  137,
  137,  137,  137,  137,  137,  137,  137,  137,  137,  137,
  137,  137,  137,  -54,    0,    0,    0,  332,    0,    0,
  -26,    0,    0,  -28,  360,  387,  785,    0,  137, -236,
    0,    0,  415,  442,  470,  497,  525,  552,  580,  607,
  635,  662,  690,  717,    0,  785,  -17,    6,    2,    0,
  745,    0,   -3, -239,  785,  -30,  800,  130,    1,    1,
 -206, -206, -206, -206, -250, -250,  -30,  -30,  -30,  785,
  785,    0,    0,  137,  137,    3,    0,  785,  137,    0,
    4,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,  137,    0,    0,  137, -245,   11,  785,
    0,   76,  772,    0,  785,  785,   15,  137,   20,    0,
  137,   23,   62,  785,    4,   28,    0,   76,   31,    0,
};
short FFrindex[] = {                                      0,
    0,    0,    0,    0,    0,    0,    0,   35,    0,    0,
    0,   36,    0,    0,    0,    0,    0,    0, -141,    0,
    0,   37,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,  195,  167,    0,    0,   48,   50,    0,    0,    0,
    0,   46,   37,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  -60,    0,    0, -212,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,  222,  250,    0,    0,    0,    0,    0,    0,    0,
    0,    0,   73,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  824,    0,    0,
    0,   72,   71,    0,   77,    0,    0,    0,    0,    0,
    0,    0, -253,   85,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0, -237,    0,    0, -248,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0, -175,    0, -207,    0,    0,
    0,    0, -179,    0, -149,  848, -261,  303,   80,  986,
 1001, 1016, 1031, 1046,  944,  965,  872,  896,  920,  -92,
  194,    0,    0,   86,    0,    0,    0, -247,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0, -253,    0,   88,
    0,   37,    0,    0, -148, -132, -122,   97,    0,    0,
    0,    0,    7, -118,    0,    0,    0,   37,    0,    0,
};
short FFgindex[] = {                                      0,
    0,  -11,    0, -158,    0,    0,    0,    0,    0,    0,
  386,  -71,    0,  311,    0, -119,  -61,  -59,   -1,    0,
    0,  -57,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  126,    0,    0, -241,    0,  -48,
    0,    0,  -35,    0,  254,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,
};
#define YYTABLESIZE 1358
short FFtable[] = {                                       9,
  131,  161,   19,  189,  223,  157,   14,  158,  156,  274,
  165,  150,  151,    2,    3,   14,  168,    2,    3,  145,
   72,  146,  147,  148,    4,  166,  167,    5,    4,  184,
  168,    5,  118,  185,  120,    6,  144,   16,  144,  119,
  144,  121,  144,  287,   15,  144,   15,  144,  149,  149,
  198,   57,   58,   15,   15,   24,   18,  199,   57,   58,
  168,  249,   69,  163,   72,  190,   69,  250,  179,  180,
  181,   72,  169,  170,  171,  172,  173,  174,  175,  176,
  177,  178,  179,  180,  181,   74,  188,   74,  195,  196,
  197,   74,  200,  157,   82,  158,   82,  203,  204,  205,
  206,  207,  208,  209,  210,  211,  212,  213,  214,  277,
  177,  178,  179,  180,  181,   17,  216,  216,   11,   20,
   11,   72,   83,   21,  221,  271,   83,  115,   83,  225,
  117,  122,  226,  227,  228,  229,  230,  231,  232,  233,
  234,  235,  236,  237,  238,  239,  240,  241,   76,   84,
   76,  123,   76,   84,   76,   84,   15,   76,  189,    2,
    3,  126,   15,  248,  124,   12,   29,   12,  125,   30,
    4,  128,  132,    5,   33,   13,   35,   13,   37,   14,
   38,   14,  133,   40,   41,   42,   43,   44,   45,   46,
   47,   48,   49,   50,   51,   52,   53,   18,  134,  135,
  279,  129,  136,  137,   55,  109,   86,  109,  270,  109,
  152,  109,  268,  273,  109,  138,  289,  139,   57,   58,
  140,  141,  142,   59,   60,   61,  143,  144,  275,  153,
  159,  276,  160,  154,  162,  186,  168,   86,  242,   86,
  182,   86,  224,   86,  183,  284,   86,  187,   86,   86,
   86,   86,   86,   86,   86,   86,   86,   86,   86,   86,
   86,  202,   59,  220,   59,   59,   59,  168,   59,  245,
   72,   59,  222,  244,   59,   59,   59,   59,   59,   59,
   59,   59,   59,   59,  263,   59,   72,   59,   59,   59,
   59,   59,   59,   59,   59,   59,   59,   59,   59,   59,
   59,   59,   59,  264,  267,  265,   59,  272,   56,   59,
  278,   59,   59,  173,  174,  175,  176,  177,  178,  179,
  180,  181,  281,   59,   59,  283,  285,  286,   59,   59,
   59,   26,  288,    2,    3,   27,  290,   28,   23,   24,
   29,   39,   34,   30,    4,   31,   32,    5,   33,   34,
   35,   36,   37,   35,   38,   36,   39,   40,   41,   42,
   43,   44,   45,   46,   47,   48,   49,   50,   51,   52,
   53,   18,   68,   81,   81,   54,  155,  149,   55,  149,
   56,  149,   37,  149,   38,   65,  149,   66,  149,  149,
  149,  149,   57,   58,    2,    3,  168,   59,   60,   61,
   68,   29,   23,  282,   30,    4,  219,    0,    5,   33,
    0,   35,    0,   37,    0,   38,    0,    0,   40,   41,
   42,   43,   44,   45,   46,   47,   48,   49,   50,   51,
   52,   53,   18,    9,    0,    0,    0,    0,    0,   55,
  171,  172,  173,  174,  175,  176,  177,  178,  179,  180,
  181,    0,    0,   57,   58,    0,    0,    0,   59,   60,
   61,   77,    0,    0,    9,    0,    9,    9,    9,    0,
    9,    0,    0,    9,    9,    9,    9,    9,    9,    9,
    9,    9,    9,    9,    9,    9,    9,    9,  112,    9,
    9,  110,   77,  110,   77,  110,   77,  110,   77,    0,
  110,   77,   77,   77,   77,   77,   77,   77,   77,   77,
   77,   77,   77,   77,   77,   77,  113,   77,   77,  112,
    0,  112,    0,  112,    0,  112,    0,    0,  112,    0,
  112,  112,  112,  112,  112,  112,  112,  112,  112,  112,
  112,  112,  112,  168,    0,    0,    0,  113,    0,  113,
    0,  113,    0,  113,    0,    0,  113,    0,  113,  113,
  113,  113,  113,  113,  113,  113,  113,  113,  113,  113,
  113,  168,    0,    0,    0,    0,  201,    0,    0,    0,
    0,    0,    0,    0,    0,  169,  170,  171,  172,  173,
  174,  175,  176,  177,  178,  179,  180,  181,  168,    0,
  143,    0,  143,    0,  143,    0,  143,    0,  215,  143,
    0,  143,  143,  169,  170,  171,  172,  173,  174,  175,
  176,  177,  178,  179,  180,  181,  168,    0,    0,    0,
    0,    0,    0,    0,    0,  243,    0,    0,    0,    0,
  169,  170,  171,  172,  173,  174,  175,  176,  177,  178,
  179,  180,  181,  168,    0,    0,    0,    0,    0,    0,
    0,    0,    0,  246,    0,    0,    0,    0,  169,  170,
  171,  172,  173,  174,  175,  176,  177,  178,  179,  180,
  181,  168,    0,    0,    0,    0,    0,    0,    0,    0,
  247,    0,    0,    0,    0,  169,  170,  171,  172,  173,
  174,  175,  176,  177,  178,  179,  180,  181,  168,    0,
    0,    0,    0,    0,    0,    0,    0,    0,  251,    0,
    0,    0,    0,  169,  170,  171,  172,  173,  174,  175,
  176,  177,  178,  179,  180,  181,  168,    0,    0,    0,
    0,    0,    0,    0,    0,  252,    0,    0,    0,    0,
  169,  170,  171,  172,  173,  174,  175,  176,  177,  178,
  179,  180,  181,  168,    0,    0,    0,    0,    0,    0,
    0,    0,    0,  253,    0,    0,    0,    0,  169,  170,
  171,  172,  173,  174,  175,  176,  177,  178,  179,  180,
  181,  168,    0,    0,    0,    0,    0,    0,    0,    0,
  254,    0,    0,    0,    0,  169,  170,  171,  172,  173,
  174,  175,  176,  177,  178,  179,  180,  181,  168,    0,
    0,    0,    0,    0,    0,    0,    0,    0,  255,    0,
    0,    0,    0,  169,  170,  171,  172,  173,  174,  175,
  176,  177,  178,  179,  180,  181,  168,    0,    0,    0,
    0,    0,    0,    0,    0,  256,    0,    0,    0,    0,
  169,  170,  171,  172,  173,  174,  175,  176,  177,  178,
  179,  180,  181,  168,    0,    0,    0,    0,    0,    0,
    0,    0,    0,  257,    0,    0,    0,    0,  169,  170,
  171,  172,  173,  174,  175,  176,  177,  178,  179,  180,
  181,  168,    0,    0,    0,    0,    0,    0,    0,    0,
  258,    0,    0,    0,    0,  169,  170,  171,  172,  173,
  174,  175,  176,  177,  178,  179,  180,  181,  168,    0,
    0,    0,    0,    0,    0,    0,    0,    0,  259,    0,
    0,    0,    0,  169,  170,  171,  172,  173,  174,  175,
  176,  177,  178,  179,  180,  181,  168,    0,    0,    0,
    0,    0,    0,    0,    0,  260,    0,    0,    0,    0,
  169,  170,  171,  172,  173,  174,  175,  176,  177,  178,
  179,  180,  181,  168,    0,    0,    0,    0,    0,    0,
    0,    0,    0,  261,    0,    0,    0,    0,  169,  170,
  171,  172,  173,  174,  175,  176,  177,  178,  179,  180,
  181,  168,    0,    0,    0,    0,    0,    0,    0,    0,
  262,    0,    0,    0,    0,  169,  170,  171,  172,  173,
  174,  175,  176,  177,  178,  179,  180,  181,  168,    0,
    0,    0,    0,    0,    0,    0,    0,    0,  266,    0,
    0,  168,    0,  169,  170,  171,  172,  173,  174,  175,
  176,  177,  178,  179,  180,  181,  168,    0,    0,    0,
    0,    0,    0,    0,    0,  280,    0,    0,    0,    0,
  169,  170,  171,  172,  173,  174,  175,  176,  177,  178,
  179,  180,  181,  169,  170,  171,  172,  173,  174,  175,
  176,  177,  178,  179,  180,  181,    0,    0,    0,  170,
  171,  172,  173,  174,  175,  176,  177,  178,  179,  180,
  181,  152,    0,  152,    0,  152,    0,  152,    0,    0,
  152,    0,  152,  152,  152,  152,  152,  152,  152,  152,
  152,  152,  152,  152,  152,  153,    0,  153,    0,  153,
    0,  153,    0,    0,  153,    0,  153,  153,  153,  153,
  153,  153,  153,  153,  153,  153,  153,  153,  153,  138,
    0,  138,    0,  138,    0,  138,    0,    0,  138,    0,
  138,  138,  138,  138,  138,  138,  138,  138,  138,  138,
  138,  138,  138,  139,    0,  139,    0,  139,    0,  139,
    0,    0,  139,    0,  139,  139,  139,  139,  139,  139,
  139,  139,  139,  139,  139,  139,  139,  140,    0,  140,
    0,  140,    0,  140,    0,    0,  140,    0,  140,  140,
  140,  140,  140,  140,  140,  140,  140,  140,  140,  140,
  140,  141,    0,  141,    0,  141,    0,  141,    0,    0,
  141,    0,  141,  141,  141,  141,  141,  141,  141,  141,
  141,  141,  142,    0,  142,    0,  142,    0,  142,    0,
    0,  142,    0,  142,  142,  142,  142,  142,  142,  142,
  142,  142,  142,  150,    0,  150,    0,  150,    0,  150,
    0,    0,  150,    0,  150,  150,  150,  150,  146,    0,
  146,    0,  146,    0,  146,    0,    0,  146,    0,  146,
  146,  146,  146,  145,    0,  145,    0,  145,    0,  145,
    0,    0,  145,    0,  145,  145,  145,  145,  148,    0,
  148,    0,  148,    0,  148,    0,    0,  148,    0,  148,
  148,  148,  148,  147,    0,  147,    0,  147,    0,  147,
    0,    0,  147,    0,  147,  147,  147,  147,
};
short FFcheck[] = {                                       1,
   36,   73,   14,  123,  163,   67,    8,   67,   66,  251,
  308,   60,   61,  258,  259,   17,  267,  258,  259,   55,
   22,   57,   58,   59,  269,  323,  324,  272,  269,  298,
  267,  272,  293,  302,  293,  280,  298,  304,  300,  300,
  302,  300,  304,  285,  298,  307,  300,  309,   60,   61,
  300,  300,  300,  296,  308,  301,  296,  307,  307,  307,
  267,  298,  300,   75,   66,  123,  304,  304,  319,  320,
  321,   73,  309,  310,  311,  312,  313,  314,  315,  316,
  317,  318,  319,  320,  321,  298,  122,  300,  124,  125,
  126,  304,  128,  155,  302,  155,  304,  133,  134,  135,
  136,  137,  138,  139,  140,  141,  142,  143,  144,  268,
  317,  318,  319,  320,  321,  298,  152,  153,  298,  303,
  300,  123,  298,  305,  160,  245,  302,  293,  304,  165,
  300,  303,  168,  169,  170,  171,  172,  173,  174,  175,
  176,  177,  178,  179,  180,  181,  182,  183,  298,  298,
  300,  303,  302,  302,  304,  304,  298,  307,  278,  258,
  259,  307,  304,  199,  303,  298,  265,  300,  303,  268,
  269,  303,  303,  272,  273,  298,  275,  300,  277,  298,
  279,  300,  303,  282,  283,  284,  285,  286,  287,  288,
  289,  290,  291,  292,  293,  294,  295,  296,  303,  303,
  272,  300,  303,  303,  303,  298,  267,  300,  244,  302,
  301,  304,  224,  249,  307,  303,  288,  303,  317,  318,
  303,  303,  303,  322,  323,  324,  303,  303,  264,  303,
  300,  267,  303,  306,  300,  300,  267,  298,  293,  300,
  308,  302,  298,  304,  308,  281,  307,  300,  309,  310,
  311,  312,  313,  314,  315,  316,  317,  318,  319,  320,
  321,  304,  256,  300,  258,  259,  260,  267,  262,  298,
  272,  265,  306,  300,  268,  269,  270,  271,  272,  273,
  274,  275,  276,  277,  302,  279,  288,  281,  282,  283,
  284,  285,  286,  287,  288,  289,  290,  291,  292,  293,
  294,  295,  296,  298,  308,  304,  300,  305,  305,  303,
  300,  305,  306,  313,  314,  315,  316,  317,  318,  319,
  320,  321,  308,  317,  318,  306,  304,  266,  322,  323,
  324,  256,  305,  258,  259,  260,  306,  262,  304,  304,
  265,  296,  306,  268,  269,  270,  271,  272,  273,  274,
  275,  276,  277,  306,  279,  306,  281,  282,  283,  284,
  285,  286,  287,  288,  289,  290,  291,  292,  293,  294,
  295,  296,  300,  302,  304,  300,   66,  298,  303,  300,
  305,  302,  306,  304,  300,  300,  307,  300,  309,  310,
  311,  312,  317,  318,  258,  259,  267,  322,  323,  324,
  304,  265,   17,  278,  268,  269,  153,   -1,  272,  273,
   -1,  275,   -1,  277,   -1,  279,   -1,   -1,  282,  283,
  284,  285,  286,  287,  288,  289,  290,  291,  292,  293,
  294,  295,  296,  267,   -1,   -1,   -1,   -1,   -1,  303,
  311,  312,  313,  314,  315,  316,  317,  318,  319,  320,
  321,   -1,   -1,  317,  318,   -1,   -1,   -1,  322,  323,
  324,  267,   -1,   -1,  298,   -1,  300,  301,  302,   -1,
  304,   -1,   -1,  307,  308,  309,  310,  311,  312,  313,
  314,  315,  316,  317,  318,  319,  320,  321,  267,  323,
  324,  298,  298,  300,  300,  302,  302,  304,  304,   -1,
  307,  307,  308,  309,  310,  311,  312,  313,  314,  315,
  316,  317,  318,  319,  320,  321,  267,  323,  324,  298,
   -1,  300,   -1,  302,   -1,  304,   -1,   -1,  307,   -1,
  309,  310,  311,  312,  313,  314,  315,  316,  317,  318,
  319,  320,  321,  267,   -1,   -1,   -1,  298,   -1,  300,
   -1,  302,   -1,  304,   -1,   -1,  307,   -1,  309,  310,
  311,  312,  313,  314,  315,  316,  317,  318,  319,  320,
  321,  267,   -1,   -1,   -1,   -1,  300,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,  309,  310,  311,  312,  313,
  314,  315,  316,  317,  318,  319,  320,  321,  267,   -1,
  298,   -1,  300,   -1,  302,   -1,  304,   -1,  304,  307,
   -1,  309,  310,  309,  310,  311,  312,  313,  314,  315,
  316,  317,  318,  319,  320,  321,  267,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,  304,   -1,   -1,   -1,   -1,
  309,  310,  311,  312,  313,  314,  315,  316,  317,  318,
  319,  320,  321,  267,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,  304,   -1,   -1,   -1,   -1,  309,  310,
  311,  312,  313,  314,  315,  316,  317,  318,  319,  320,
  321,  267,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  304,   -1,   -1,   -1,   -1,  309,  310,  311,  312,  313,
  314,  315,  316,  317,  318,  319,  320,  321,  267,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  304,   -1,
   -1,   -1,   -1,  309,  310,  311,  312,  313,  314,  315,
  316,  317,  318,  319,  320,  321,  267,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,  304,   -1,   -1,   -1,   -1,
  309,  310,  311,  312,  313,  314,  315,  316,  317,  318,
  319,  320,  321,  267,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,  304,   -1,   -1,   -1,   -1,  309,  310,
  311,  312,  313,  314,  315,  316,  317,  318,  319,  320,
  321,  267,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  304,   -1,   -1,   -1,   -1,  309,  310,  311,  312,  313,
  314,  315,  316,  317,  318,  319,  320,  321,  267,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  304,   -1,
   -1,   -1,   -1,  309,  310,  311,  312,  313,  314,  315,
  316,  317,  318,  319,  320,  321,  267,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,  304,   -1,   -1,   -1,   -1,
  309,  310,  311,  312,  313,  314,  315,  316,  317,  318,
  319,  320,  321,  267,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,  304,   -1,   -1,   -1,   -1,  309,  310,
  311,  312,  313,  314,  315,  316,  317,  318,  319,  320,
  321,  267,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  304,   -1,   -1,   -1,   -1,  309,  310,  311,  312,  313,
  314,  315,  316,  317,  318,  319,  320,  321,  267,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  304,   -1,
   -1,   -1,   -1,  309,  310,  311,  312,  313,  314,  315,
  316,  317,  318,  319,  320,  321,  267,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,  304,   -1,   -1,   -1,   -1,
  309,  310,  311,  312,  313,  314,  315,  316,  317,  318,
  319,  320,  321,  267,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,  304,   -1,   -1,   -1,   -1,  309,  310,
  311,  312,  313,  314,  315,  316,  317,  318,  319,  320,
  321,  267,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  304,   -1,   -1,   -1,   -1,  309,  310,  311,  312,  313,
  314,  315,  316,  317,  318,  319,  320,  321,  267,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  304,   -1,
   -1,  267,   -1,  309,  310,  311,  312,  313,  314,  315,
  316,  317,  318,  319,  320,  321,  267,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,  304,   -1,   -1,   -1,   -1,
  309,  310,  311,  312,  313,  314,  315,  316,  317,  318,
  319,  320,  321,  309,  310,  311,  312,  313,  314,  315,
  316,  317,  318,  319,  320,  321,   -1,   -1,   -1,  310,
  311,  312,  313,  314,  315,  316,  317,  318,  319,  320,
  321,  298,   -1,  300,   -1,  302,   -1,  304,   -1,   -1,
  307,   -1,  309,  310,  311,  312,  313,  314,  315,  316,
  317,  318,  319,  320,  321,  298,   -1,  300,   -1,  302,
   -1,  304,   -1,   -1,  307,   -1,  309,  310,  311,  312,
  313,  314,  315,  316,  317,  318,  319,  320,  321,  298,
   -1,  300,   -1,  302,   -1,  304,   -1,   -1,  307,   -1,
  309,  310,  311,  312,  313,  314,  315,  316,  317,  318,
  319,  320,  321,  298,   -1,  300,   -1,  302,   -1,  304,
   -1,   -1,  307,   -1,  309,  310,  311,  312,  313,  314,
  315,  316,  317,  318,  319,  320,  321,  298,   -1,  300,
   -1,  302,   -1,  304,   -1,   -1,  307,   -1,  309,  310,
  311,  312,  313,  314,  315,  316,  317,  318,  319,  320,
  321,  298,   -1,  300,   -1,  302,   -1,  304,   -1,   -1,
  307,   -1,  309,  310,  311,  312,  313,  314,  315,  316,
  317,  318,  298,   -1,  300,   -1,  302,   -1,  304,   -1,
   -1,  307,   -1,  309,  310,  311,  312,  313,  314,  315,
  316,  317,  318,  298,   -1,  300,   -1,  302,   -1,  304,
   -1,   -1,  307,   -1,  309,  310,  311,  312,  298,   -1,
  300,   -1,  302,   -1,  304,   -1,   -1,  307,   -1,  309,
  310,  311,  312,  298,   -1,  300,   -1,  302,   -1,  304,
   -1,   -1,  307,   -1,  309,  310,  311,  312,  298,   -1,
  300,   -1,  302,   -1,  304,   -1,   -1,  307,   -1,  309,
  310,  311,  312,  298,   -1,  300,   -1,  302,   -1,  304,
   -1,   -1,  307,   -1,  309,  310,  311,  312,
};
#define YYFINAL 1
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 326
#if YYDEBUG
char *FFname[] = {
"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"TOK_SYNTAX_ERROR","TOK_STRING",
"TOK_BOOLEAN","TOK_BREAK","TOK_CASE","TOK_CONTINUE","TOK_DEFAULT","TOK_DO",
"TOK_INITIAL","TOK_ELSE","TOK_EXPONENT","TOK_FALSE","TOK_FLOAT","TOK_FOR",
"TOK_IF","TOK_INT","TOK_KC","TOK_PRINT","TOK_RESULT","TOK_RETURN",
"TOK_SIMULATE","TOK_SWITCH","TOK_TRUE","TOK_VOID","TOK_WHILE","TOK_ABS",
"TOK_ACOS","TOK_ASIN","TOK_ATAN","TOK_COS","TOK_SIN","TOK_TAN","TOK_EXP",
"TOK_LOG","TOK_SQRT","TOK_SQR","TOK_INT_CONST","TOK_FLOAT_CONST",
"TOK_STRING_CONST","TOK_ID","TOK_COLON","TOK_COMMA","TOK_DOT","TOK_SEMICOLON",
"TOK_LBRACK","TOK_RBRACK","TOK_LPAREN","TOK_RPAREN","TOK_LBRACE","TOK_RBRACE",
"TOK_LEFT_SHIFT","TOK_ASSIGN","TOK_OR","TOK_AND","TOK_EQ","TOK_NE","TOK_LT",
"TOK_GT","TOK_LE","TOK_GE","TOK_PLUS","TOK_MINUS","TOK_MULT","TOK_DIV",
"TOK_MOD","TOK_NOT","TOK_PLUS_PLUS","TOK_MINUS_MINUS","TOK_UMINUS","TOK_UPLUS",
};
char *FFrule[] = {
"$accept : program",
"program :",
"program : program funcDefn",
"type : TOK_INT",
"type : TOK_FLOAT",
"type : TOK_STRING",
"type : TOK_BOOLEAN",
"funcType : type",
"funcType : TOK_VOID",
"name : simpleName",
"simpleName : TOK_ID",
"varDecl : simpleName optArraySubScript",
"varDecl : simpleName optArraySubScript TOK_ASSIGN expr",
"varDecl : varDecl TOK_COMMA simpleName optArraySubScript",
"varDecl : varDecl TOK_COMMA simpleName optArraySubScript TOK_ASSIGN expr",
"optArraySubScript :",
"optArraySubScript : TOK_LBRACK arraySubScript TOK_RBRACK",
"arraySubScript : TOK_INT_CONST",
"arraySubScript : arraySubScript TOK_COMMA TOK_INT_CONST",
"funcDefn : funcHdr formalParams TOK_RPAREN funcHdr2 funcBody",
"funcHdr : funcType TOK_ID TOK_LPAREN",
"funcHdr2 : TOK_LBRACE",
"funcBody : declOrStmts TOK_RBRACE",
"formalParams :",
"formalParams : neFormalParams",
"neFormalParams : formalParam",
"neFormalParams : neFormalParams TOK_COMMA formalParam",
"formalParam : type simpleName optArraySubScript",
"declOrStmtsVarHdr : localVarDecl TOK_SEMICOLON",
"declOrStmtsVarHdr : declOrStmtsVarHdr localVarDecl TOK_SEMICOLON",
"declOrStmtsStmtsHdr : emptyStmt",
"declOrStmtsStmtsHdr : neStmt",
"declOrStmtsStmtsHdr : declOrStmtsStmtsHdr emptyStmt",
"declOrStmtsStmtsHdr : declOrStmtsStmtsHdr neStmt",
"declOrStmts :",
"declOrStmts : declOrStmtsVarHdr",
"declOrStmts : declOrStmtsStmtsHdr",
"declOrStmts : declOrStmtsVarHdr declOrStmtsStmtsHdr",
"localVarDecl : localVarDeclHdr varDecl",
"localVarDeclHdr : type",
"stmt : neStmt",
"stmt : emptyStmt",
"neStmt : printStmt",
"neStmt : ifThenStmt",
"neStmt : ifThenElseStmt",
"neStmt : whileStmt",
"neStmt : forStmt",
"neStmt : breakStmt",
"neStmt : continueStmt",
"neStmt : simpleStmt TOK_SEMICOLON",
"neStmt : returnStmt",
"neStmt : blockStmt",
"neStmt : error TOK_SEMICOLON",
"simulateInvocation : TOK_SIMULATE TOK_LPAREN TOK_RPAREN",
"blockStmt : blockStmtHdr declOrStmts TOK_RBRACE",
"blockStmtHdr : TOK_LBRACE",
"printStmt : TOK_PRINT nePrintStmts TOK_SEMICOLON",
"nePrintStmts : TOK_LEFT_SHIFT expr",
"nePrintStmts : nePrintStmts TOK_LEFT_SHIFT expr",
"ifThenStmt : TOK_IF TOK_LPAREN expr TOK_RPAREN TOK_LBRACE declOrStmts TOK_RBRACE",
"ifThenElseStmt : TOK_IF TOK_LPAREN expr TOK_RPAREN TOK_LBRACE declOrStmts TOK_RBRACE TOK_ELSE TOK_LBRACE declOrStmts TOK_RBRACE",
"whileStmt : TOK_WHILE TOK_LPAREN expr TOK_RPAREN blockStmt",
"forStmt : TOK_FOR TOK_LPAREN optInitalization TOK_SEMICOLON optExpr TOK_SEMICOLON optSimpleStmts TOK_RPAREN blockStmt",
"optInitalization : localVarDecl",
"optInitalization : optSimpleStmts",
"optExpr :",
"optExpr : expr",
"optSimpleStmts : simpleStmts",
"simpleStmts :",
"simpleStmts : neSimpleStmts",
"neSimpleStmts : simpleStmt",
"neSimpleStmts : neSimpleStmts TOK_COMMA simpleStmt",
"returnStmt : TOK_RETURN expr TOK_SEMICOLON",
"returnStmt : TOK_RETURN TOK_SEMICOLON",
"simpleStmt : expr",
"emptyStmt : TOK_SEMICOLON",
"assignment : refExpr TOK_ASSIGN expr",
"refExpr : name",
"refExpr : subscriptRefExpr",
"funcInvocation : simpleName TOK_LPAREN exprs TOK_RPAREN",
"subscriptRefExpr : name TOK_LBRACK exprs TOK_RBRACK",
"exprs :",
"exprs : neExprs",
"neExprs : expr",
"neExprs : neExprs TOK_COMMA expr",
"expr : literal",
"expr : refExpr",
"expr : castExpr",
"expr : autoExpr",
"expr : funcInvocation",
"expr : simulateInvocation",
"expr : multExpr",
"expr : divExpr",
"expr : modExpr",
"expr : addExpr",
"expr : subExpr",
"expr : andExpr",
"expr : orExpr",
"expr : eqExpr",
"expr : neExpr",
"expr : ltExpr",
"expr : gtExpr",
"expr : leExpr",
"expr : geExpr",
"expr : uminusExpr",
"expr : uplusExpr",
"expr : notExpr",
"expr : exponentExpr",
"expr : assignment",
"expr : initialExpr TOK_ASSIGN expr",
"expr : kcExpr TOK_ASSIGN expr",
"expr : parenExpr",
"expr : initialExpr",
"expr : kcExpr",
"expr : resultExpr",
"expr : mathExpr",
"mathExpr : TOK_ABS TOK_LPAREN expr TOK_RPAREN",
"mathExpr : TOK_ACOS TOK_LPAREN expr TOK_RPAREN",
"mathExpr : TOK_ASIN TOK_LPAREN expr TOK_RPAREN",
"mathExpr : TOK_ATAN TOK_LPAREN expr TOK_RPAREN",
"mathExpr : TOK_COS TOK_LPAREN expr TOK_RPAREN",
"mathExpr : TOK_SIN TOK_LPAREN expr TOK_RPAREN",
"mathExpr : TOK_TAN TOK_LPAREN expr TOK_RPAREN",
"mathExpr : TOK_EXP TOK_LPAREN expr TOK_RPAREN",
"mathExpr : TOK_LOG TOK_LPAREN expr TOK_RPAREN",
"mathExpr : TOK_SQRT TOK_LPAREN expr TOK_RPAREN",
"mathExpr : TOK_SQR TOK_LPAREN expr TOK_RPAREN",
"initialExpr : TOK_INITIAL TOK_LPAREN expr TOK_RPAREN",
"kcExpr : TOK_KC TOK_LPAREN expr TOK_RPAREN",
"resultExpr : TOK_RESULT TOK_LPAREN expr TOK_COMMA expr TOK_RPAREN",
"resultExpr : TOK_RESULT TOK_LPAREN expr TOK_RPAREN",
"literal : TOK_INT_CONST",
"literal : TOK_STRING_CONST",
"literal : TOK_FLOAT_CONST",
"literal : TOK_TRUE",
"literal : TOK_FALSE",
"uminusExpr : TOK_MINUS expr",
"uplusExpr : TOK_PLUS expr",
"multExpr : expr TOK_MULT expr",
"divExpr : expr TOK_DIV expr",
"modExpr : expr TOK_MOD expr",
"addExpr : expr TOK_PLUS expr",
"subExpr : expr TOK_MINUS expr",
"andExpr : expr TOK_AND expr",
"orExpr : expr TOK_OR expr",
"gtExpr : expr TOK_GT expr",
"ltExpr : expr TOK_LT expr",
"geExpr : expr TOK_GE expr",
"leExpr : expr TOK_LE expr",
"eqExpr : expr TOK_EQ expr",
"neExpr : expr TOK_NE expr",
"parenExpr : TOK_LPAREN expr TOK_RPAREN",
"notExpr : TOK_NOT expr",
"exponentExpr : expr TOK_EXPONENT expr",
"autoExpr : refExpr TOK_PLUS_PLUS",
"autoExpr : refExpr TOK_MINUS_MINUS",
"autoExpr : TOK_PLUS_PLUS refExpr",
"autoExpr : TOK_MINUS_MINUS refExpr",
"castExpr : type TOK_LPAREN expr TOK_RPAREN",
"breakStmt : TOK_BREAK TOK_SEMICOLON",
"breakStmt : TOK_BREAK TOK_INT_CONST TOK_SEMICOLON",
"continueStmt : TOK_CONTINUE TOK_SEMICOLON",
"continueStmt : TOK_CONTINUE TOK_INT_CONST TOK_SEMICOLON",
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
#line 159 "FF.y"
{ yyval.seqNodeVal = 0; }
break;
case 2:
#line 160 "FF.y"
{
	  FF_top = new SeqNode(yyline, yyvsp[0].methodDefNodeVal, yyvsp[-1].seqNodeVal);
	  yyval.seqNodeVal = FF_top;
	}
break;
case 3:
#line 166 "FF.y"
{ yyval.typeVal = INT_TYPE;    }
break;
case 4:
#line 167 "FF.y"
{ yyval.typeVal = FLOAT_TYPE;  }
break;
case 5:
#line 168 "FF.y"
{ yyval.typeVal = STRING_TYPE; }
break;
case 6:
#line 169 "FF.y"
{ yyval.typeVal = BOOL_TYPE;   }
break;
case 7:
#line 172 "FF.y"
{ yyval.typeVal = yyvsp[0].typeVal;          }
break;
case 8:
#line 173 "FF.y"
{ yyval.typeVal = VOID_TYPE;   }
break;
case 9:
#line 176 "FF.y"
{ 
	  yyval.refExprNodeVal = new RefExprNode(yyline, yyvsp[0].strVal);
	  delete[] yyvsp[0].strVal;
	}
break;
case 10:
#line 182 "FF.y"
{
          char *c = new char[strlen(yylval.strVal)+1];
          strcpy(c, yylval.strVal);
	  delete[] yylval.strVal;
          yyval.strVal = c; }
break;
case 11:
#line 189 "FF.y"
{
	  TypeTag type = cur_type;
	  if(yyvsp[0].vectorIntVal) {
	    if(type == INT_TYPE)    type = INT_MATRIX_TYPE;
	    if(type == FLOAT_TYPE)  type = FLOAT_MATRIX_TYPE;
	    if(type == STRING_TYPE) type = STRING_MATRIX_TYPE;
	    if(type == BOOL_TYPE)   type = BOOL_MATRIX_TYPE;
	  }
	  VarNode *vn = new VarNode(yyline, yyvsp[-1].strVal, 0, type, yyvsp[0].vectorIntVal);
	  delete[] yyvsp[-1].strVal;
	  yyval.seqNodeVal = new SeqNode(yyline, vn, 0); }
break;
case 12:
#line 200 "FF.y"
{
	  TypeTag type = cur_type;
	  if(yyvsp[-2].vectorIntVal) {
	    if(type == INT_TYPE)    type = INT_MATRIX_TYPE;
	    if(type == FLOAT_TYPE)  type = FLOAT_MATRIX_TYPE;
	    if(type == STRING_TYPE) type = STRING_MATRIX_TYPE;
	    if(type == BOOL_TYPE)   type = BOOL_MATRIX_TYPE;
	  }
	  VarNode *vn = new VarNode(yyline, yyvsp[-3].strVal, 0, cur_type, yyvsp[-2].vectorIntVal);
	  delete[] yyvsp[-3].strVal;
	  BinOpNode *bo = new BinOpNode(yyline, ASSIGN, vn, yyvsp[0].nodeVal);
	  yyval.seqNodeVal = new SeqNode(yyline, bo, 0); }
break;
case 13:
#line 212 "FF.y"
{
	  TypeTag type = cur_type;
	  if(yyvsp[0].vectorIntVal) {
	    if(type == INT_TYPE)    type = INT_MATRIX_TYPE;
	    if(type == FLOAT_TYPE)  type = FLOAT_MATRIX_TYPE;
	    if(type == STRING_TYPE) type = STRING_MATRIX_TYPE;
	    if(type == BOOL_TYPE)   type = BOOL_MATRIX_TYPE;
	  }
	  VarNode *vn = new VarNode(yyline, yyvsp[-1].strVal, 0, cur_type, yyvsp[0].vectorIntVal);
	  delete[] yyvsp[-1].strVal;
	  yyval.seqNodeVal = new SeqNode(yyline, vn, yyvsp[-3].seqNodeVal); }
break;
case 14:
#line 223 "FF.y"
{
	  TypeTag type = cur_type;
	  if(yyvsp[-2].vectorIntVal) {
	    if(type == INT_TYPE)    type = INT_MATRIX_TYPE;
	    if(type == FLOAT_TYPE)  type = FLOAT_MATRIX_TYPE;
	    if(type == STRING_TYPE) type = STRING_MATRIX_TYPE;
	    if(type == BOOL_TYPE)   type = BOOL_MATRIX_TYPE;
	  }
	  VarNode *vn = new VarNode(yyline, yyvsp[-3].strVal, 0, cur_type, yyvsp[-2].vectorIntVal);
	  delete[] yyvsp[-3].strVal;
	  BinOpNode *bo = new BinOpNode(yyline, ASSIGN, vn, yyvsp[0].nodeVal);
	  yyval.seqNodeVal = new SeqNode(yyline, bo, yyvsp[-5].seqNodeVal);
        }
break;
case 15:
#line 238 "FF.y"
{ yyval.vectorIntVal = NULL; }
break;
case 16:
#line 239 "FF.y"
{ yyval.vectorIntVal = yyvsp[-1].vectorIntVal;   }
break;
case 17:
#line 242 "FF.y"
{ 
	  vector<int> *dims = new vector<int>;
	  dims->push_back(yylval.intVal);
	  yyval.vectorIntVal = dims;
	}
break;
case 18:
#line 246 "FF.y"
{ 
	  yyvsp[-2].vectorIntVal->push_back(yylval.intVal);
	  yyval.vectorIntVal = yyvsp[-2].vectorIntVal;
        }
break;
case 19:
#line 252 "FF.y"
{
          yyvsp[-4].methodDefNodeVal->setFormals(yyvsp[-3].seqNodeVal);
          yyvsp[-4].methodDefNodeVal->setBody(yyvsp[0].blockNodeVal);
	  yyval.methodDefNodeVal = yyvsp[-4].methodDefNodeVal;
        }
break;
case 20:
#line 259 "FF.y"
{
          MethodDefNode *md = new MethodDefNode(yyline, yylval.strVal, 0, yyvsp[-2].typeVal);
	  delete[] yylval.strVal;
	  yyval.methodDefNodeVal = md;
        }
break;
case 21:
#line 266 "FF.y"
{
        }
break;
case 22:
#line 270 "FF.y"
{ yyval.blockNodeVal = new BlockNode(yyline, yyvsp[-1].seqNodeVal); }
break;
case 23:
#line 273 "FF.y"
{ yyval.seqNodeVal = 0;  }
break;
case 24:
#line 274 "FF.y"
{ yyval.seqNodeVal = yyvsp[0].seqNodeVal; }
break;
case 25:
#line 277 "FF.y"
{
          yyval.seqNodeVal = new SeqNode(yyline, yyvsp[0].varNodeVal, 0); }
break;
case 26:
#line 279 "FF.y"
{
	  yyval.seqNodeVal = new SeqNode(yyline, yyvsp[0].varNodeVal, yyvsp[-2].seqNodeVal);
	}
break;
case 27:
#line 284 "FF.y"
{
	  VarNode *vn = new VarNode(yyline, yyvsp[-1].strVal, 0, yyvsp[-2].typeVal, yyvsp[0].vectorIntVal);
	  delete[] yyvsp[-1].strVal;
	  yyval.varNodeVal = vn;
        }
break;
case 28:
#line 291 "FF.y"
{ yyval.seqNodeVal = yyvsp[-1].seqNodeVal; }
break;
case 29:
#line 292 "FF.y"
{ yyval.seqNodeVal = new SeqNode(yyline, yyvsp[-1].seqNodeVal, yyvsp[-2].seqNodeVal); }
break;
case 30:
#line 295 "FF.y"
{ yyval.seqNodeVal = 0; }
break;
case 31:
#line 297 "FF.y"
{
                  yyval.seqNodeVal = new SeqNode(yyline, yyvsp[0].nodeVal, 0);
                }
break;
case 32:
#line 300 "FF.y"
{ yyval.seqNodeVal = yyvsp[-1].seqNodeVal; }
break;
case 33:
#line 301 "FF.y"
{ yyval.seqNodeVal = yyvsp[0].nodeVal ? new SeqNode(yyline, yyvsp[0].nodeVal, yyvsp[-1].seqNodeVal) : yyvsp[-1].seqNodeVal; }
break;
case 34:
#line 305 "FF.y"
{ yyval.seqNodeVal = 0; }
break;
case 35:
#line 306 "FF.y"
{ yyval.seqNodeVal = yyvsp[0].seqNodeVal; }
break;
case 36:
#line 307 "FF.y"
{ yyval.seqNodeVal = yyvsp[0].seqNodeVal; }
break;
case 37:
#line 308 "FF.y"
{ yyval.seqNodeVal = new SeqNode(yyline, yyvsp[0].seqNodeVal, yyvsp[-1].seqNodeVal); }
break;
case 38:
#line 311 "FF.y"
{ yyval.seqNodeVal = yyvsp[0].seqNodeVal; }
break;
case 39:
#line 314 "FF.y"
{
          cur_type = yyvsp[0].typeVal;
        }
break;
case 40:
#line 319 "FF.y"
{ yyval.nodeVal = yyvsp[0].nodeVal; }
break;
case 41:
#line 320 "FF.y"
{ yyval.nodeVal = yyvsp[0].nodeVal; }
break;
case 42:
#line 323 "FF.y"
{ yyval.nodeVal = yyvsp[0].nodeVal; }
break;
case 43:
#line 324 "FF.y"
{ yyval.nodeVal = yyvsp[0].nodeVal; }
break;
case 44:
#line 325 "FF.y"
{ yyval.nodeVal = yyvsp[0].nodeVal; }
break;
case 45:
#line 326 "FF.y"
{ yyval.nodeVal = yyvsp[0].nodeVal; }
break;
case 46:
#line 327 "FF.y"
{ yyval.nodeVal = yyvsp[0].nodeVal; }
break;
case 47:
#line 328 "FF.y"
{ yyval.nodeVal = yyvsp[0].nodeVal; }
break;
case 48:
#line 329 "FF.y"
{ yyval.nodeVal = yyvsp[0].nodeVal; }
break;
case 49:
#line 330 "FF.y"
{ yyval.nodeVal = new SeqNode(yyline, yyvsp[-1].nodeVal, 0); }
break;
case 50:
#line 331 "FF.y"
{ yyval.nodeVal = new SeqNode(yyline, yyvsp[0].nodeVal, 0); }
break;
case 51:
#line 332 "FF.y"
{ yyval.nodeVal = yyvsp[0].nodeVal; }
break;
case 52:
#line 333 "FF.y"
{ yyerrok; yyval.nodeVal = 0; }
break;
case 53:
#line 336 "FF.y"
{ 
	  yyval.nodeVal = new SimulateNode(yyline); 
	}
break;
case 54:
#line 341 "FF.y"
{
         yyval.nodeVal = new BlockNode(yyvsp[-2].intVal, yyvsp[-1].seqNodeVal);
       }
break;
case 55:
#line 346 "FF.y"
{
	  yyval.intVal = yyline;
        }
break;
case 56:
#line 351 "FF.y"
{
	  yyval.nodeVal = new PrintNode(yyline, yyvsp[-1].seqNodeVal);
	}
break;
case 57:
#line 356 "FF.y"
{ 
	  yyval.seqNodeVal = new SeqNode(yyline, yyvsp[0].nodeVal, 0 );
        }
break;
case 58:
#line 359 "FF.y"
{ 
	  yyval.seqNodeVal = new SeqNode(yyline, yyvsp[0].nodeVal, yyvsp[-2].seqNodeVal);
	}
break;
case 59:
#line 364 "FF.y"
{
          yyval.nodeVal = new IfNode(yyline, yyvsp[-4].nodeVal, yyvsp[-1].seqNodeVal);
        }
break;
case 60:
#line 369 "FF.y"
{
          yyval.nodeVal = new IfNode(yyline, yyvsp[-8].nodeVal, yyvsp[-5].seqNodeVal, yyvsp[-1].seqNodeVal);
        }
break;
case 61:
#line 375 "FF.y"
{ yyval.nodeVal = new WhileNode(yyline, yyvsp[-2].nodeVal, yyvsp[0].nodeVal); }
break;
case 62:
#line 379 "FF.y"
{ 
                  yyval.nodeVal = new ForNode(yyline, yyvsp[-6].seqNodeVal, yyvsp[-4].nodeVal, yyvsp[-2].seqNodeVal, yyvsp[0].nodeVal); 
                }
break;
case 63:
#line 384 "FF.y"
{ yyval.seqNodeVal = yyvsp[0].seqNodeVal; }
break;
case 64:
#line 385 "FF.y"
{ yyval.seqNodeVal = yyvsp[0].seqNodeVal; }
break;
case 65:
#line 388 "FF.y"
{ yyval.nodeVal = 0;  }
break;
case 66:
#line 389 "FF.y"
{ yyval.nodeVal = yyvsp[0].nodeVal; }
break;
case 67:
#line 392 "FF.y"
{ yyval.seqNodeVal = yyvsp[0].seqNodeVal; }
break;
case 68:
#line 395 "FF.y"
{ yyval.seqNodeVal = 0;  }
break;
case 69:
#line 396 "FF.y"
{ yyval.seqNodeVal = yyvsp[0].seqNodeVal; }
break;
case 70:
#line 399 "FF.y"
{ yyval.seqNodeVal = new SeqNode(yyline, yyvsp[0].nodeVal, 0);  }
break;
case 71:
#line 400 "FF.y"
{ yyval.seqNodeVal = new SeqNode(yyline, yyvsp[0].nodeVal, yyvsp[-2].seqNodeVal); }
break;
case 72:
#line 403 "FF.y"
{ yyval.nodeVal = new ReturnNode(yyline, yyvsp[-1].nodeVal); }
break;
case 73:
#line 404 "FF.y"
{ yyval.nodeVal = new ReturnNode(yyline);     }
break;
case 74:
#line 407 "FF.y"
{ yyval.nodeVal = yyvsp[0].nodeVal; }
break;
case 75:
#line 410 "FF.y"
{ yyval.nodeVal = 0; }
break;
case 76:
#line 413 "FF.y"
{ yyval.nodeVal = new BinOpNode(yyline, ASSIGN, yyvsp[-2].refExprNodeVal, yyvsp[0].nodeVal); }
break;
case 77:
#line 416 "FF.y"
{ yyval.refExprNodeVal = yyvsp[0].refExprNodeVal; }
break;
case 78:
#line 417 "FF.y"
{ yyval.refExprNodeVal = yyvsp[0].refExprNodeVal; }
break;
case 79:
#line 420 "FF.y"
{
          yyval.nodeVal = new MethodNode(yyline, yyvsp[-3].strVal, yyvsp[-1].seqNodeVal);
	  delete[] yyvsp[-3].strVal;
        }
break;
case 80:
#line 426 "FF.y"
{
	  yyvsp[-3].refExprNodeVal->setSubscripts(yyvsp[-1].seqNodeVal);
	  yyval.refExprNodeVal = yyvsp[-3].refExprNodeVal;
	}
break;
case 81:
#line 432 "FF.y"
{ yyval.seqNodeVal = 0;  }
break;
case 82:
#line 433 "FF.y"
{ yyval.seqNodeVal = yyvsp[0].seqNodeVal; }
break;
case 83:
#line 436 "FF.y"
{ yyval.seqNodeVal = new SeqNode(yyline, yyvsp[0].nodeVal, 0);  }
break;
case 84:
#line 437 "FF.y"
{ yyval.seqNodeVal = new SeqNode(yyline, yyvsp[0].nodeVal, yyvsp[-2].seqNodeVal); }
break;
case 85:
#line 440 "FF.y"
{ yyval.nodeVal = yyvsp[0].nodeVal; }
break;
case 86:
#line 441 "FF.y"
{ yyval.nodeVal = yyvsp[0].refExprNodeVal; }
break;
case 87:
#line 442 "FF.y"
{ yyval.nodeVal = yyvsp[0].nodeVal; }
break;
case 88:
#line 443 "FF.y"
{ yyval.nodeVal = yyvsp[0].nodeVal; }
break;
case 89:
#line 444 "FF.y"
{ yyval.nodeVal = yyvsp[0].nodeVal; }
break;
case 90:
#line 445 "FF.y"
{ yyval.nodeVal = yyvsp[0].nodeVal; }
break;
case 91:
#line 446 "FF.y"
{ yyval.nodeVal = yyvsp[0].nodeVal; }
break;
case 92:
#line 447 "FF.y"
{ yyval.nodeVal = yyvsp[0].nodeVal; }
break;
case 93:
#line 448 "FF.y"
{ yyval.nodeVal = yyvsp[0].nodeVal; }
break;
case 94:
#line 449 "FF.y"
{ yyval.nodeVal = yyvsp[0].nodeVal; }
break;
case 95:
#line 450 "FF.y"
{ yyval.nodeVal = yyvsp[0].nodeVal; }
break;
case 96:
#line 451 "FF.y"
{ yyval.nodeVal = yyvsp[0].nodeVal; }
break;
case 97:
#line 452 "FF.y"
{ yyval.nodeVal = yyvsp[0].nodeVal; }
break;
case 98:
#line 453 "FF.y"
{ yyval.nodeVal = yyvsp[0].nodeVal; }
break;
case 99:
#line 454 "FF.y"
{ yyval.nodeVal = yyvsp[0].nodeVal; }
break;
case 100:
#line 455 "FF.y"
{ yyval.nodeVal = yyvsp[0].nodeVal; }
break;
case 101:
#line 456 "FF.y"
{ yyval.nodeVal = yyvsp[0].nodeVal; }
break;
case 102:
#line 457 "FF.y"
{ yyval.nodeVal = yyvsp[0].nodeVal; }
break;
case 103:
#line 458 "FF.y"
{ yyval.nodeVal = yyvsp[0].nodeVal; }
break;
case 104:
#line 459 "FF.y"
{ yyval.nodeVal = yyvsp[0].nodeVal; }
break;
case 105:
#line 460 "FF.y"
{ yyval.nodeVal = yyvsp[0].nodeVal; }
break;
case 106:
#line 461 "FF.y"
{ yyval.nodeVal = yyvsp[0].nodeVal; }
break;
case 107:
#line 462 "FF.y"
{ yyval.nodeVal = yyvsp[0].nodeVal; }
break;
case 108:
#line 463 "FF.y"
{ yyval.nodeVal = yyvsp[0].nodeVal; }
break;
case 109:
#line 464 "FF.y"
{ ((InitialNode*)yyvsp[-2].nodeVal)->setAssignment(yyvsp[0].nodeVal); yyval.nodeVal = yyvsp[-2].nodeVal; }
break;
case 110:
#line 465 "FF.y"
{ ((KCNode*)yyvsp[-2].nodeVal)->setAssignment(yyvsp[0].nodeVal); yyval.nodeVal = yyvsp[-2].nodeVal; }
break;
case 111:
#line 466 "FF.y"
{ yyval.nodeVal = yyvsp[0].nodeVal; }
break;
case 112:
#line 467 "FF.y"
{ yyval.nodeVal = yyvsp[0].nodeVal; }
break;
case 113:
#line 468 "FF.y"
{ yyval.nodeVal = yyvsp[0].nodeVal; }
break;
case 114:
#line 469 "FF.y"
{ yyval.nodeVal = yyvsp[0].nodeVal; }
break;
case 115:
#line 470 "FF.y"
{ yyval.nodeVal = yyvsp[0].nodeVal; }
break;
case 116:
#line 473 "FF.y"
{
	  yyval.nodeVal = new MathNode(yyline, MATH_ABS, yyvsp[-1].nodeVal);
	}
break;
case 117:
#line 476 "FF.y"
{
	  yyval.nodeVal = new MathNode(yyline, MATH_ACOS, yyvsp[-1].nodeVal);
	}
break;
case 118:
#line 479 "FF.y"
{
	  yyval.nodeVal = new MathNode(yyline, MATH_ASIN, yyvsp[-1].nodeVal);
	}
break;
case 119:
#line 482 "FF.y"
{
	  yyval.nodeVal = new MathNode(yyline, MATH_ATAN, yyvsp[-1].nodeVal);
	}
break;
case 120:
#line 485 "FF.y"
{
	  yyval.nodeVal = new MathNode(yyline, MATH_COS, yyvsp[-1].nodeVal);
	}
break;
case 121:
#line 488 "FF.y"
{
	  yyval.nodeVal = new MathNode(yyline, MATH_SIN, yyvsp[-1].nodeVal);
	}
break;
case 122:
#line 491 "FF.y"
{
	  yyval.nodeVal = new MathNode(yyline, MATH_TAN, yyvsp[-1].nodeVal);
	}
break;
case 123:
#line 494 "FF.y"
{
	  yyval.nodeVal = new MathNode(yyline, MATH_EXP, yyvsp[-1].nodeVal);
	}
break;
case 124:
#line 497 "FF.y"
{
	  yyval.nodeVal = new MathNode(yyline, MATH_LOG, yyvsp[-1].nodeVal);
	}
break;
case 125:
#line 500 "FF.y"
{
	  yyval.nodeVal = new MathNode(yyline, MATH_SQRT, yyvsp[-1].nodeVal);
	}
break;
case 126:
#line 503 "FF.y"
{
	  yyval.nodeVal = new MathNode(yyline, MATH_SQR, yyvsp[-1].nodeVal);
	}
break;
case 127:
#line 508 "FF.y"
{
	  yyval.nodeVal = new InitialNode(yyline, yyvsp[-1].nodeVal);
	}
break;
case 128:
#line 513 "FF.y"
{
	  yyval.nodeVal = new KCNode(yyline, yyvsp[-1].nodeVal);
	}
break;
case 129:
#line 518 "FF.y"
{
	  yyval.nodeVal = new ResultNode(yyline, yyvsp[-3].nodeVal, yyvsp[-1].nodeVal);
	}
break;
case 130:
#line 521 "FF.y"
{
	  yyval.nodeVal = new ResultNode(yyline, yyvsp[-1].nodeVal, NULL);
	}
break;
case 131:
#line 526 "FF.y"
{ yyval.nodeVal = new ValueNode(yyline, new Value(yylval.intVal)); }
break;
case 132:
#line 527 "FF.y"
{
		char *c = new char[strlen(yylval.strVal)+1];
		strcpy(c, yylval.strVal);
		yyval.nodeVal = new ValueNode(yyline, new Value(string(c))); }
break;
case 133:
#line 531 "FF.y"
{ yyval.nodeVal = new ValueNode(yyline, new Value(yylval.floatVal)); }
break;
case 134:
#line 532 "FF.y"
{ yyval.nodeVal = new ValueNode(yyline, new Value(true)); }
break;
case 135:
#line 533 "FF.y"
{ yyval.nodeVal = new ValueNode(yyline, new Value(false)); }
break;
case 136:
#line 536 "FF.y"
{ yyval.nodeVal = new UnaryOpNode(yyline, NEG, yyvsp[0].nodeVal); }
break;
case 137:
#line 539 "FF.y"
{ yyval.nodeVal = new UnaryOpNode(yyline, POS, yyvsp[0].nodeVal); }
break;
case 138:
#line 542 "FF.y"
{ yyval.nodeVal = new BinOpNode(yyline, MULT, yyvsp[-2].nodeVal, yyvsp[0].nodeVal); }
break;
case 139:
#line 545 "FF.y"
{ yyval.nodeVal = new BinOpNode(yyline, DIV, yyvsp[-2].nodeVal, yyvsp[0].nodeVal); }
break;
case 140:
#line 548 "FF.y"
{ yyval.nodeVal = new BinOpNode(yyline, MOD, yyvsp[-2].nodeVal, yyvsp[0].nodeVal); }
break;
case 141:
#line 551 "FF.y"
{ yyval.nodeVal = new BinOpNode(yyline, ADD, yyvsp[-2].nodeVal, yyvsp[0].nodeVal); }
break;
case 142:
#line 554 "FF.y"
{ yyval.nodeVal = new BinOpNode(yyline, SUB, yyvsp[-2].nodeVal, yyvsp[0].nodeVal); }
break;
case 143:
#line 557 "FF.y"
{ yyval.nodeVal = new BinOpNode(yyline, AND, yyvsp[-2].nodeVal, yyvsp[0].nodeVal); }
break;
case 144:
#line 560 "FF.y"
{ yyval.nodeVal = new BinOpNode(yyline, OR, yyvsp[-2].nodeVal, yyvsp[0].nodeVal); }
break;
case 145:
#line 563 "FF.y"
{ yyval.nodeVal = new BinOpNode(yyline, GT, yyvsp[-2].nodeVal, yyvsp[0].nodeVal); }
break;
case 146:
#line 566 "FF.y"
{ yyval.nodeVal = new BinOpNode(yyline, LT, yyvsp[-2].nodeVal, yyvsp[0].nodeVal); }
break;
case 147:
#line 569 "FF.y"
{ yyval.nodeVal = new BinOpNode(yyline, GE, yyvsp[-2].nodeVal, yyvsp[0].nodeVal); }
break;
case 148:
#line 572 "FF.y"
{ yyval.nodeVal = new BinOpNode(yyline, LE, yyvsp[-2].nodeVal, yyvsp[0].nodeVal); }
break;
case 149:
#line 575 "FF.y"
{ yyval.nodeVal = new BinOpNode(yyline, EQ, yyvsp[-2].nodeVal, yyvsp[0].nodeVal); }
break;
case 150:
#line 578 "FF.y"
{ yyval.nodeVal = new BinOpNode(yyline, NE, yyvsp[-2].nodeVal, yyvsp[0].nodeVal); }
break;
case 151:
#line 581 "FF.y"
{ yyval.nodeVal = yyvsp[-1].nodeVal; }
break;
case 152:
#line 584 "FF.y"
{ yyval.nodeVal = new UnaryOpNode(yyline, NOT, yyvsp[0].nodeVal); }
break;
case 153:
#line 587 "FF.y"
{ yyval.nodeVal = new BinOpNode(yyline, EXPONENT, yyvsp[-2].nodeVal, yyvsp[0].nodeVal); }
break;
case 154:
#line 590 "FF.y"
{ yyval.nodeVal = new UnaryOpNode(yyline, POST_INCR, yyvsp[-1].refExprNodeVal); }
break;
case 155:
#line 591 "FF.y"
{ yyval.nodeVal = new UnaryOpNode(yyline, POST_DECR, yyvsp[-1].refExprNodeVal); }
break;
case 156:
#line 592 "FF.y"
{ yyval.nodeVal = new UnaryOpNode(yyline, PRE_INCR, yyvsp[0].refExprNodeVal); }
break;
case 157:
#line 593 "FF.y"
{ yyval.nodeVal = new UnaryOpNode(yyline, PRE_DECR, yyvsp[0].refExprNodeVal); }
break;
case 158:
#line 596 "FF.y"
{
	  yyval.nodeVal = new CastNode(yyline, yyvsp[-3].typeVal, yyvsp[-1].nodeVal);
	}
break;
case 159:
#line 601 "FF.y"
{
	  yyval.nodeVal = new BreakNode(yyline, 1);
	}
break;
case 160:
#line 604 "FF.y"
{
	  yyval.nodeVal = new BreakNode(yyline, yylval.intVal);
	}
break;
case 161:
#line 609 "FF.y"
{
	  yyval.nodeVal = new ContinueNode(yyline, 1);
	}
break;
case 162:
#line 612 "FF.y"
{
	  yyval.nodeVal = new ContinueNode(yyline, yylval.intVal);
	}
break;
#line 1782 "FF.tab.c"
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
