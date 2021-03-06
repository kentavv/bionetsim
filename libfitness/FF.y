%{
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
%}

%start program
%union {
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
};

%token TOK_SYNTAX_ERROR

/* Keywords */
%token TOK_STRING
%token TOK_BOOLEAN    TOK_BREAK    TOK_CASE
%token TOK_CONTINUE   TOK_DEFAULT  TOK_DO       TOK_INITIAL
%token TOK_ELSE       TOK_EXPONENT TOK_FALSE    TOK_FLOAT
%token TOK_FOR        TOK_IF       TOK_INT      TOK_KC
%token TOK_PRINT      TOK_RESULT   TOK_RETURN   TOK_SIMULATE  
%token TOK_SWITCH     TOK_TRUE     TOK_VOID     TOK_WHILE

 /* Math functions */
%token TOK_ABS
%token TOK_ACOS TOK_ASIN TOK_ATAN 
%token TOK_COS  TOK_SIN  TOK_TAN
%token TOK_EXP  TOK_LOG  TOK_SQRT
%token TOK_SQR

   /* constants and identifiers */
%token TOK_INT_CONST TOK_FLOAT_CONST TOK_STRING_CONST TOK_ID

   /* miscellaneous operators */
%token TOK_COLON  TOK_COMMA  TOK_DOT    TOK_SEMICOLON
%token TOK_LBRACK TOK_RBRACK TOK_LPAREN TOK_RPAREN
%token TOK_LBRACE TOK_RBRACE
%token TOK_LEFT_SHIFT

   /* Arithmetic operators, with precedence info */
%right TOK_ASSIGN
%left TOK_OR
%left TOK_AND
%left TOK_EQ TOK_NE
/* %nonassoc TOK_EQ, TOK_NE */
%nonassoc TOK_LT TOK_GT TOK_LE TOK_GE
%left TOK_PLUS   TOK_MINUS
%left TOK_MULT   TOK_DIV TOK_MOD
%right TOK_NOT
%right TOK_EXPONENT
%left TOK_PLUS_PLUS TOK_MINUS_MINUS
%right TOK_UMINUS
%right TOK_UPLUS
%nonassoc TOK_LBRACK TOK_LPAREN
%left TOK_DOT

%type<seqNodeVal> program
%type<refExprNodeVal> name
%type<strVal> simpleName
%type<seqNodeVal> varDecl
%type<vectorIntVal> optArraySubScript
%type<vectorIntVal> arraySubScript
%type<methodDefNodeVal> funcDefn
%type<methodDefNodeVal> funcHdr
%type<blockNodeVal> funcBody
%type<seqNodeVal> formalParams
%type<seqNodeVal> neFormalParams
%type<varNodeVal> formalParam
%type<seqNodeVal> declOrStmts
%type<seqNodeVal> declOrStmtsVarHdr
%type<seqNodeVal> declOrStmtsStmtsHdr
%type<nodeVal> returnStmt
%type<nodeVal> simpleStmt
%type<nodeVal> emptyStmt
%type<nodeVal> neStmt
%type<typeVal> type
%type<typeVal> funcType
%type<intVal> blockStmtHdr;
%type<seqNodeVal> localVarDecl
%type<nodeVal> localVarDeclHdr
%type<nodeVal> stmt
%type<nodeVal> printStmt
%type<seqNodeVal> nePrintStmts
%type<nodeVal> ifThenStmt
%type<nodeVal> ifThenElseStmt
%type<nodeVal> whileStmt
%type<nodeVal> forStmt
%type<nodeVal> breakStmt
%type<nodeVal> continueStmt
%type<nodeVal> optExpr
%type<seqNodeVal> optInitalization
%type<seqNodeVal> optSimpleStmts
%type<seqNodeVal> simpleStmts
%type<seqNodeVal> neSimpleStmts
%type<nodeVal> blockStmt
%type<nodeVal> assignment
%type<refExprNodeVal> refExpr
%type<nodeVal> funcInvocation
%type<refExprNodeVal> subscriptRefExpr
%type<nodeVal> expr
%type<nodeVal> literal
%type<seqNodeVal> exprs
%type<seqNodeVal> neExprs
%type<nodeVal> castExpr;
%type<nodeVal> uminusExpr
%type<nodeVal> uplusExpr
%type<nodeVal> multExpr
%type<nodeVal> divExpr
%type<nodeVal> modExpr
%type<nodeVal> addExpr
%type<nodeVal> subExpr
%type<nodeVal> andExpr
%type<nodeVal> orExpr
%type<nodeVal> gtExpr
%type<nodeVal> ltExpr
%type<nodeVal> geExpr
%type<nodeVal> leExpr
%type<nodeVal> eqExpr
%type<nodeVal> neExpr
%type<nodeVal> parenExpr
%type<nodeVal> notExpr
%type<nodeVal> exponentExpr
%type<nodeVal> autoExpr
%type<nodeVal> mathExpr

%type<nodeVal> simulateInvocation
%type<nodeVal> initialExpr
%type<nodeVal> kcExpr
%type<nodeVal> resultExpr
%%

program: 
	/* empty */        { $$ = 0; }
	| program funcDefn {
	  FF_top = new SeqNode(yyline, $2, $1);
	  $$ = FF_top;
	};

type:
        TOK_INT       { $$ = INT_TYPE;    }
        | TOK_FLOAT   { $$ = FLOAT_TYPE;  }
        | TOK_STRING  { $$ = STRING_TYPE; }
        | TOK_BOOLEAN { $$ = BOOL_TYPE;   } ;

funcType:
	type          { $$ = $1;          }
        | TOK_VOID    { $$ = VOID_TYPE;   } ;

name:
        simpleName                { 
	  $$ = new RefExprNode(yyline, $1);
	  delete[] $1;
	};

simpleName:
        TOK_ID {
          char *c = new char[strlen(yylval.strVal)+1];
          strcpy(c, yylval.strVal);
	  delete[] yylval.strVal;
          $$ = c; } ;

varDecl:
        simpleName optArraySubScript {
	  TypeTag type = cur_type;
	  if($2) {
	    if(type == INT_TYPE)    type = INT_MATRIX_TYPE;
	    if(type == FLOAT_TYPE)  type = FLOAT_MATRIX_TYPE;
	    if(type == STRING_TYPE) type = STRING_MATRIX_TYPE;
	    if(type == BOOL_TYPE)   type = BOOL_MATRIX_TYPE;
	  }
	  VarNode *vn = new VarNode(yyline, $1, 0, type, $2);
	  delete[] $1;
	  $$ = new SeqNode(yyline, vn, 0); }
        | simpleName optArraySubScript TOK_ASSIGN expr {
	  TypeTag type = cur_type;
	  if($2) {
	    if(type == INT_TYPE)    type = INT_MATRIX_TYPE;
	    if(type == FLOAT_TYPE)  type = FLOAT_MATRIX_TYPE;
	    if(type == STRING_TYPE) type = STRING_MATRIX_TYPE;
	    if(type == BOOL_TYPE)   type = BOOL_MATRIX_TYPE;
	  }
	  VarNode *vn = new VarNode(yyline, $1, 0, cur_type, $2);
	  delete[] $1;
	  BinOpNode *bo = new BinOpNode(yyline, ASSIGN, vn, $4);
	  $$ = new SeqNode(yyline, bo, 0); }
        | varDecl TOK_COMMA simpleName optArraySubScript {
	  TypeTag type = cur_type;
	  if($4) {
	    if(type == INT_TYPE)    type = INT_MATRIX_TYPE;
	    if(type == FLOAT_TYPE)  type = FLOAT_MATRIX_TYPE;
	    if(type == STRING_TYPE) type = STRING_MATRIX_TYPE;
	    if(type == BOOL_TYPE)   type = BOOL_MATRIX_TYPE;
	  }
	  VarNode *vn = new VarNode(yyline, $3, 0, cur_type, $4);
	  delete[] $3;
	  $$ = new SeqNode(yyline, vn, $1); }
        | varDecl TOK_COMMA simpleName optArraySubScript TOK_ASSIGN expr {
	  TypeTag type = cur_type;
	  if($4) {
	    if(type == INT_TYPE)    type = INT_MATRIX_TYPE;
	    if(type == FLOAT_TYPE)  type = FLOAT_MATRIX_TYPE;
	    if(type == STRING_TYPE) type = STRING_MATRIX_TYPE;
	    if(type == BOOL_TYPE)   type = BOOL_MATRIX_TYPE;
	  }
	  VarNode *vn = new VarNode(yyline, $3, 0, cur_type, $4);
	  delete[] $3;
	  BinOpNode *bo = new BinOpNode(yyline, ASSIGN, vn, $6);
	  $$ = new SeqNode(yyline, bo, $1);
        } ;

optArraySubScript:
        /* empty */                            { $$ = NULL; }
        | TOK_LBRACK arraySubScript TOK_RBRACK { $$ = $2;   } ;

arraySubScript:
        TOK_INT_CONST                            { 
	  vector<int> *dims = new vector<int>;
	  dims->push_back(yylval.intVal);
	  $$ = dims;
	} | arraySubScript TOK_COMMA TOK_INT_CONST { 
	  $1->push_back(yylval.intVal);
	  $$ = $1;
        } ;

funcDefn:
        funcHdr formalParams TOK_RPAREN funcHdr2 funcBody {
          $1->setFormals($2);
          $1->setBody($5);
	  $$ = $1;
        };

funcHdr:
        funcType TOK_ID TOK_LPAREN {
          MethodDefNode *md = new MethodDefNode(yyline, yylval.strVal, 0, $1);
	  delete[] yylval.strVal;
	  $$ = md;
        };

funcHdr2:
        TOK_LBRACE {
        }

funcBody:
        declOrStmts TOK_RBRACE { $$ = new BlockNode(yyline, $1); };

formalParams:
        /* empty */      { $$ = 0;  }
        | neFormalParams { $$ = $1; } ;

neFormalParams:
        formalParam {
          $$ = new SeqNode(yyline, $1, 0); }
        | neFormalParams TOK_COMMA formalParam {
	  $$ = new SeqNode(yyline, $3, $1);
	};

formalParam:
        type simpleName optArraySubScript {
	  VarNode *vn = new VarNode(yyline, $2, 0, $1, $3);
	  delete[] $2;
	  $$ = vn;
        };

declOrStmtsVarHdr:
        localVarDecl TOK_SEMICOLON                     { $$ = $1; }
        | declOrStmtsVarHdr localVarDecl TOK_SEMICOLON { $$ = new SeqNode(yyline, $2, $1); };

declOrStmtsStmtsHdr:
	emptyStmt { $$ = 0; }
	| neStmt
                {
                  $$ = new SeqNode(yyline, $1, 0);
                }
	| declOrStmtsStmtsHdr emptyStmt { $$ = $1; }
	| declOrStmtsStmtsHdr neStmt { $$ = $2 ? new SeqNode(yyline, $2, $1) : $1; };


declOrStmts:
	/* empty */ { $$ = 0; }
        | declOrStmtsVarHdr                     { $$ = $1; }
	| declOrStmtsStmtsHdr { $$ = $1; }
        | declOrStmtsVarHdr declOrStmtsStmtsHdr { $$ = new SeqNode(yyline, $2, $1); };

localVarDecl:
        localVarDeclHdr varDecl { $$ = $2; }

localVarDeclHdr:
        type {
          cur_type = $1;
        };

stmt:
        neStmt      { $$ = $1; }
        | emptyStmt { $$ = $1; };

neStmt:
        printStmt                           { $$ = $1; }
        | ifThenStmt                        { $$ = $1; }
        | ifThenElseStmt                    { $$ = $1; }
        | whileStmt                         { $$ = $1; }
        | forStmt                           { $$ = $1; }
	| breakStmt			    { $$ = $1; }
	| continueStmt			    { $$ = $1; }
	| simpleStmt TOK_SEMICOLON { $$ = new SeqNode(yyline, $1, 0); }
	| returnStmt             { $$ = new SeqNode(yyline, $1, 0); }
	| blockStmt              { $$ = $1; }
	| error TOK_SEMICOLON { yyerrok; $$ = 0; };

simulateInvocation:
	TOK_SIMULATE TOK_LPAREN TOK_RPAREN { 
	  $$ = new SimulateNode(yyline); 
	};

blockStmt:
       blockStmtHdr declOrStmts TOK_RBRACE {
         $$ = new BlockNode($1, $2);
       };

blockStmtHdr:
        TOK_LBRACE {
	  $$ = yyline;
        };

printStmt:
	TOK_PRINT nePrintStmts TOK_SEMICOLON {
	  $$ = new PrintNode(yyline, $2);
	};

nePrintStmts:
	TOK_LEFT_SHIFT expr { 
	  $$ = new SeqNode(yyline, $2, 0 );
        }
	| nePrintStmts TOK_LEFT_SHIFT expr { 
	  $$ = new SeqNode(yyline, $3, $1);
	};

ifThenStmt:
        TOK_IF TOK_LPAREN expr TOK_RPAREN TOK_LBRACE declOrStmts TOK_RBRACE {
          $$ = new IfNode(yyline, $3, $6);
        } ;

ifThenElseStmt:
        TOK_IF TOK_LPAREN expr TOK_RPAREN TOK_LBRACE declOrStmts TOK_RBRACE TOK_ELSE TOK_LBRACE declOrStmts TOK_RBRACE {
          $$ = new IfNode(yyline, $3, $6, $10);
        } ;

whileStmt:
        TOK_WHILE TOK_LPAREN expr TOK_RPAREN blockStmt
                { $$ = new WhileNode(yyline, $3, $5); } ;

forStmt:
        TOK_FOR TOK_LPAREN optInitalization TOK_SEMICOLON optExpr  
                TOK_SEMICOLON optSimpleStmts TOK_RPAREN blockStmt { 
                  $$ = new ForNode(yyline, $3, $5, $7, $9); 
                } ;

optInitalization:
	localVarDecl { $$ = $1; }
        | optSimpleStmts { $$ = $1; }

optExpr:
        /* empty */  { $$ = 0;  }
        | expr       { $$ = $1; } ;

optSimpleStmts:
        simpleStmts { $$ = $1; } ;

simpleStmts:
        /* empty */     { $$ = 0;  }
        | neSimpleStmts { $$ = $1; }

neSimpleStmts:
        simpleStmt                           { $$ = new SeqNode(yyline, $1, 0);  }
        | neSimpleStmts TOK_COMMA simpleStmt { $$ = new SeqNode(yyline, $3, $1); };

returnStmt:
        TOK_RETURN expr TOK_SEMICOLON { $$ = new ReturnNode(yyline, $2); }
        | TOK_RETURN TOK_SEMICOLON    { $$ = new ReturnNode(yyline);     }

simpleStmt:
       expr { $$ = $1; };

emptyStmt:
        TOK_SEMICOLON { $$ = 0; };

assignment:
        refExpr TOK_ASSIGN expr { $$ = new BinOpNode(yyline, ASSIGN, $1, $3); } ;

refExpr:
	name                             { $$ = $1; }
	| subscriptRefExpr               { $$ = $1; } ;

funcInvocation:
        simpleName TOK_LPAREN exprs TOK_RPAREN {
          $$ = new MethodNode(yyline, $1, $3);
	  delete[] $1;
        } ;

subscriptRefExpr:
	name TOK_LBRACK exprs TOK_RBRACK {
	  $1->setSubscripts($3);
	  $$ = $1;
	};

exprs:
	/* empty */ { $$ = 0;  }
	| neExprs   { $$ = $1; } ;

neExprs:
	expr                     { $$ = new SeqNode(yyline, $1, 0);  }
	| neExprs TOK_COMMA expr { $$ = new SeqNode(yyline, $3, $1); } ;

expr:
        literal      { $$ = $1; }
	| refExpr    { $$ = $1; }
	| castExpr    { $$ = $1; }
        | autoExpr   { $$ = $1; }
	| funcInvocation    { $$ = $1; }
        | simulateInvocation { $$ = $1; }
        | multExpr   { $$ = $1; }
        | divExpr    { $$ = $1; }
        | modExpr    { $$ = $1; }
        | addExpr    { $$ = $1; }
        | subExpr    { $$ = $1; }
        | andExpr    { $$ = $1; }
        | orExpr     { $$ = $1; }
        | eqExpr     { $$ = $1; }
        | neExpr     { $$ = $1; }
        | ltExpr     { $$ = $1; }
        | gtExpr     { $$ = $1; }
        | leExpr     { $$ = $1; }
        | geExpr     { $$ = $1; }
        | uminusExpr { $$ = $1; }
        | uplusExpr  { $$ = $1; }
        | notExpr    { $$ = $1; }
        | exponentExpr    { $$ = $1; }
	| assignment { $$ = $1; }
        | initialExpr TOK_ASSIGN expr { ((InitialNode*)$1)->setAssignment($3); $$ = $1; }
        | kcExpr TOK_ASSIGN expr { ((KCNode*)$1)->setAssignment($3); $$ = $1; }
        | parenExpr  { $$ = $1; } 
	| initialExpr     { $$ = $1; }
	| kcExpr     { $$ = $1; }
	| resultExpr { $$ = $1; }
        | mathExpr   { $$ = $1; } ;

mathExpr:
	TOK_ABS TOK_LPAREN expr TOK_RPAREN {
	  $$ = new MathNode(yyline, MATH_ABS, $3);
	}
	| TOK_ACOS TOK_LPAREN expr TOK_RPAREN {
	  $$ = new MathNode(yyline, MATH_ACOS, $3);
	}
	| TOK_ASIN TOK_LPAREN expr TOK_RPAREN {
	  $$ = new MathNode(yyline, MATH_ASIN, $3);
	}
	| TOK_ATAN TOK_LPAREN expr TOK_RPAREN {
	  $$ = new MathNode(yyline, MATH_ATAN, $3);
	}
	| TOK_COS TOK_LPAREN expr TOK_RPAREN {
	  $$ = new MathNode(yyline, MATH_COS, $3);
	}
	| TOK_SIN TOK_LPAREN expr TOK_RPAREN {
	  $$ = new MathNode(yyline, MATH_SIN, $3);
	}
	| TOK_TAN TOK_LPAREN expr TOK_RPAREN {
	  $$ = new MathNode(yyline, MATH_TAN, $3);
	}
	| TOK_EXP TOK_LPAREN expr TOK_RPAREN {
	  $$ = new MathNode(yyline, MATH_EXP, $3);
	}
	| TOK_LOG TOK_LPAREN expr TOK_RPAREN {
	  $$ = new MathNode(yyline, MATH_LOG, $3);
	}
	| TOK_SQRT TOK_LPAREN expr TOK_RPAREN {
	  $$ = new MathNode(yyline, MATH_SQRT, $3);
	}
	| TOK_SQR TOK_LPAREN expr TOK_RPAREN {
	  $$ = new MathNode(yyline, MATH_SQR, $3);
	};

initialExpr:
	TOK_INITIAL TOK_LPAREN expr TOK_RPAREN {
	  $$ = new InitialNode(yyline, $3);
	};

kcExpr:
	TOK_KC TOK_LPAREN expr TOK_RPAREN {
	  $$ = new KCNode(yyline, $3);
	};

resultExpr:
	TOK_RESULT TOK_LPAREN expr TOK_COMMA expr TOK_RPAREN {
	  $$ = new ResultNode(yyline, $3, $5);
	}
	| TOK_RESULT TOK_LPAREN expr TOK_RPAREN {
	  $$ = new ResultNode(yyline, $3, NULL);
	}

literal:
        TOK_INT_CONST      { $$ = new ValueNode(yyline, new Value(yylval.intVal)); }
        | TOK_STRING_CONST {
		char *c = new char[strlen(yylval.strVal)+1];
		strcpy(c, yylval.strVal);
		$$ = new ValueNode(yyline, new Value(string(c))); }
	| TOK_FLOAT_CONST  { $$ = new ValueNode(yyline, new Value(yylval.floatVal)); }
	| TOK_TRUE         { $$ = new ValueNode(yyline, new Value(true)); } 
	| TOK_FALSE        { $$ = new ValueNode(yyline, new Value(false)); } ;

uminusExpr:
	TOK_MINUS expr %prec TOK_UMINUS { $$ = new UnaryOpNode(yyline, NEG, $2); } ;

uplusExpr:
	TOK_PLUS expr %prec TOK_UPLUS { $$ = new UnaryOpNode(yyline, POS, $2); } ;

multExpr:
	expr TOK_MULT expr { $$ = new BinOpNode(yyline, MULT, $1, $3); } ;

divExpr:
	expr TOK_DIV expr { $$ = new BinOpNode(yyline, DIV, $1, $3); } ;

modExpr:
	expr TOK_MOD expr { $$ = new BinOpNode(yyline, MOD, $1, $3); } ;

addExpr:
	expr TOK_PLUS expr { $$ = new BinOpNode(yyline, ADD, $1, $3); } ;

subExpr:
	expr TOK_MINUS expr { $$ = new BinOpNode(yyline, SUB, $1, $3); } ;

andExpr:
	expr TOK_AND expr { $$ = new BinOpNode(yyline, AND, $1, $3); } ;

orExpr:
	expr TOK_OR expr { $$ = new BinOpNode(yyline, OR, $1, $3); } ;

gtExpr:
	expr TOK_GT expr { $$ = new BinOpNode(yyline, GT, $1, $3); } ;

ltExpr:
	expr TOK_LT expr { $$ = new BinOpNode(yyline, LT, $1, $3); } ;

geExpr:
	expr TOK_GE expr { $$ = new BinOpNode(yyline, GE, $1, $3); } ;

leExpr:
	expr TOK_LE expr { $$ = new BinOpNode(yyline, LE, $1, $3); } ;

eqExpr:
	expr TOK_EQ expr { $$ = new BinOpNode(yyline, EQ, $1, $3); } ;

neExpr:
	expr TOK_NE expr { $$ = new BinOpNode(yyline, NE, $1, $3); } ;

parenExpr:
	TOK_LPAREN expr TOK_RPAREN { $$ = $2; } ;

notExpr:
	TOK_NOT expr { $$ = new UnaryOpNode(yyline, NOT, $2); } ;

exponentExpr:
	expr TOK_EXPONENT expr { $$ = new BinOpNode(yyline, EXPONENT, $1, $3); } ;

autoExpr:
	refExpr TOK_PLUS_PLUS     { $$ = new UnaryOpNode(yyline, POST_INCR, $1); }
	| refExpr TOK_MINUS_MINUS { $$ = new UnaryOpNode(yyline, POST_DECR, $1); }
	| TOK_PLUS_PLUS refExpr   { $$ = new UnaryOpNode(yyline, PRE_INCR, $2); }
	| TOK_MINUS_MINUS refExpr { $$ = new UnaryOpNode(yyline, PRE_DECR, $2); } ;

castExpr:
	type TOK_LPAREN expr TOK_RPAREN {
	  $$ = new CastNode(yyline, $1, $3);
	}

breakStmt:
	TOK_BREAK TOK_SEMICOLON {
	  $$ = new BreakNode(yyline, 1);
	}
	| TOK_BREAK TOK_INT_CONST TOK_SEMICOLON {
	  $$ = new BreakNode(yyline, yylval.intVal);
	};

continueStmt:
	TOK_CONTINUE TOK_SEMICOLON {
	  $$ = new ContinueNode(yyline, 1);
	}
	| TOK_CONTINUE TOK_INT_CONST TOK_SEMICOLON {
	  $$ = new ContinueNode(yyline, yylval.intVal);
	};
%%
