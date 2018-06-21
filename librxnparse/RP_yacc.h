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
extern YYSTYPE RPlval;
