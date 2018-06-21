%{
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
%}

%start equations

%union {
char *strVal;
  int   intVal;
  double floatVal;
  string *stringVal;
  pair<double, double> *RxnRateVal;
  ReactionNode *ReactionVal;
  list<ReactionNode> *ReactionListVal;
  list<pair<int, string> > *MoleculeSumVal;
}

%token TOK_NEW_LINE

%token TOK_ID
%token TOK_INT_CONST
%token TOK_FLOAT_CONST

%token TOK_LBRACE
%token TOK_RBRACE
%token TOK_RXN_ARROW
%token TOK_RXN_ARROW_REV
%token TOK_COLON
%token TOK_PLUS
%token TOK_COMMA
%token TOK_ASSIGN
%token TOK_ZERO

%token TOK_SYNTAX_ERROR

%type<stringVal> rxn_name
%type<MoleculeSumVal> mol_sum
%type<MoleculeSumVal> ne_mol_sum
%type<intVal> coeff
%type<stringVal> mol_name
%type<intVal> rxn_dir
%type<RxnRateVal> rxn_rate
%type<floatVal> number
%type<stringVal> name

%%

equations: 
	/* empty */ {}
	| equations equation {}

equation:
	| TOK_NEW_LINE {}
	| assignment TOK_NEW_LINE {}
	| reaction TOK_NEW_LINE {}

assignment:
	mol_name TOK_ASSIGN number {
	  RP_top2.push_back(make_pair(*$1, $3));
	  delete $1;
	}

reaction:
	rxn_name TOK_COLON ne_mol_sum rxn_dir mol_sum rxn_rate {
	  RP_top.push_back(ReactionNode(RPyyline, $1, $3, $4, $5, $6));
	  if($1) delete $1;
	  if($3) delete $3;
	  if($5) delete $5;
	  if($6) delete $6;
	}
	| rxn_name TOK_COLON mol_sum rxn_dir ne_mol_sum rxn_rate {
	  RP_top.push_back(ReactionNode(RPyyline, $1, $3, $4, $5, $6));
	  if($1) delete $1;
	  if($3) delete $3;
	  if($5) delete $5;
	  if($6) delete $6;
	}
	| rxn_name TOK_COLON ne_mol_sum TOK_RXN_ARROW TOK_ZERO rxn_rate {
	  RP_top.push_back(ReactionNode(RPyyline, $1, $3, 0, 0, $6));
	  if($1) delete $1;
	  if($3) delete $3;
	  if($6) delete $6;
	}



	| ne_mol_sum rxn_dir mol_sum rxn_rate {
	  char str[64];
	  sprintf(str, "R%d", ++RP_rxn_num);
	  string str2 = str;

	  RP_top.push_back(ReactionNode(RPyyline, &str2, $1, $2, $3, $4));
	  if($1) delete $1;
	  if($3) delete $3;
	  if($4) delete $4;
	}
	| mol_sum rxn_dir ne_mol_sum rxn_rate {
	  char str[64];
	  sprintf(str, "R%d", ++RP_rxn_num);
	  string str2 = str;

	  RP_top.push_back(ReactionNode(RPyyline, &str2, $1, $2, $3, $4));
	  if($1) delete $1;
	  if($3) delete $3;
	  if($4) delete $4;
	}
	| ne_mol_sum TOK_RXN_ARROW TOK_ZERO rxn_rate {
	  char str[64];
	  sprintf(str, "R%d", ++RP_rxn_num);
	  string str2 = str;

	  RP_top.push_back(ReactionNode(RPyyline, &str2, $1, 0, 0, $4));
	  if($1) delete $1;
	  if($4) delete $4;
	}

rxn_name:
	name {$$ = $1;}

mol_sum:
	/* empty */		{$$ = 0;}
        | ne_mol_sum                     { $$ = $1; }

ne_mol_sum:
        coeff mol_name {
	  list<pair<int, string> > *lst = new list<pair<int, string> >;
	  lst->push_back(make_pair($1, *$2));
	  $$ = lst; 
	}
        | mol_name {
	  list<pair<int, string> > *lst = new list<pair<int, string> >;
	  lst->push_back(make_pair(1, *$1));
	  $$ = lst; 
	}
        | ne_mol_sum TOK_PLUS coeff mol_name { 
	  $1->push_back(make_pair($3, *$4));
	  $$ = $1;
	}
        | ne_mol_sum TOK_PLUS mol_name{ 
	  $1->push_back(make_pair(1, *$3));
	  $$ = $1;
	}

coeff:
	TOK_INT_CONST		{$$ = yylval.intVal;}

mol_name:
	name { $$ = $1; }

rxn_dir:
	TOK_RXN_ARROW	{$$ = 0;}
	| TOK_RXN_ARROW_REV   {$$ = 1;}

rxn_rate:
	/* empty */	{$$ = new pair<double, double>(1., 1.);}
	| TOK_LBRACE number TOK_RBRACE	{$$ = new pair<double, double>($2, 1.);}
	| TOK_LBRACE number TOK_COMMA number TOK_RBRACE	{$$ = new pair<double, double>($2, $4);}

number:
	TOK_ZERO { $$ = 0.0; }
	| TOK_INT_CONST {$$ = double(yylval.intVal);}
	| TOK_FLOAT_CONST {$$ = yylval.floatVal;}

name:
	TOK_ID {
	  $$ = new string(yylval.strVal);
	}
%%
