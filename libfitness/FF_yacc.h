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
extern YYSTYPE FFlval;
