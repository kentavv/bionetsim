// This file is part of GenoDYN.
//
// GenoDYN is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// GenoDYN is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with GenoDYN.  If not, see <http://www.gnu.org/licenses/>.
//
// Copyright 2008 Kent A. Vander Velden

#ifndef AST_NODE_H
#define AST_NODE_H

#include <vector>

#include "STEClasses.h"
#include "Value.h"
#include "SymTabMgr.h"

extern SymTabMgr symTabMgr;
extern int debugf;

extern ostream &ff_log;

enum NodeTag { VAR_DEF_NODE = 0,
	       VALUE_NODE,
	       REF_EXPR_NODE,

               METHOD_INVOKE_NODE,
	       METHOD_DEF_NODE,

               BINARY_OP_NODE,
               UNARY_OP_NODE, 
               PRINT_NODE,
	       
	       CAST_NODE, 
	       MATH_NODE,

               BREAK_NODE,
               CONTINUE_NODE,
	       RETURN_NODE,

               IF_NODE,
               WHILE_NODE,
               FOR_NODE,
               SWITCH_NODE,

               CASE_NODE, 
	       BLOCK_NODE,
               SEQUENCE_NODE,

	       SIMULATE_NODE,
	       INITIAL_NODE,
	       KC_NODE,
	       RESULT_NODE
};

class AstNode {
public:
  AstNode(int line, NodeTag tag, TypeTag t = ERROR_TYPE) 
    : line_(line),
      tag_(tag),
      type_(t) {
    if(debugf > 0) {
      ff_log << "Created AstNode type " << tag_ << " on line " << line_ << " with typetag " << type_ << '\n';
    }
  }
  virtual ~AstNode() {}

  NodeTag nodeTag() const { return tag_; }
  TypeTag type()    const { return type_; }
  int line()        const { return line_; }

  virtual void print(ostream& /*os*/, int /*indent*/ = 0) const {}
  virtual Value* eval() = 0;

protected:
  NodeTag tag_;
  TypeTag type_;
  int line_;
};

inline ostream& operator<<(ostream& os, const AstNode& an) {
  an.print(os);
  return os;
};

class VarNode : public AstNode {
public:
  VarNode(int line, const string &name, Modifier mod, TypeTag type, std::vector<int> *subscripts) 
    : AstNode(line, VAR_DEF_NODE, type),
      name_(name),
      mod_(mod),
      subscripts_(subscripts) {
  }

  void print(ostream& /*os*/, int /*indent*/ = 0) const {}
  Value* eval();

protected:
  VarNode(const VarNode &);

  string name_;
  Modifier mod_;
  std::vector<int> *subscripts_;
};

class ValueNode: public AstNode {
public:
  ValueNode(int line, Value *v) 
    : AstNode(line, VALUE_NODE, UNKNOWN_TYPE),
      val_(v) {
  }
  ~ValueNode() {}

  void print(ostream& /*os*/, int /*indent*/ = 0) const {}
  Value* eval();

private:
  Value* val_;
};

class SeqNode;

class RefExprNode: public AstNode {
public:
  RefExprNode(int line, const string &name) 
    : AstNode(line, REF_EXPR_NODE, UNKNOWN_TYPE),
      name_(name),
      sym_(NULL),
      ss_(NULL) {
  }
  ~RefExprNode() {}

  SymTabEntry* symTabEntry() { return sym_; }

  void setSubscripts(SeqNode *ss) { ss_ = ss; }

  void print(ostream& /*os*/, int /*indent*/ = 0) const {}
  Value* eval();
  string name() const { return name_; }

protected:
  RefExprNode(RefExprNode* base, NodeTag tag_);

  int calcIndex(SeqNode *ss, int dim, const vector<int> &);

private:
  string name_;

  SymTabEntry *sym_;
  SeqNode *ss_;
};

class SeqNode: public AstNode {
public:
  SeqNode(int line, AstNode *first, SeqNode *rest = NULL) 
    : AstNode(line, SEQUENCE_NODE, UNKNOWN_TYPE),
      head_(first),
      tail_(rest) {
  }
  ~SeqNode() {}

  SeqNode* append(AstNode*) {}
  int size() const {}
  void print(ostream& /*os*/, int /*indent*/ = 0) const {}
  AstNode **toArray() {}
  Value* eval();

  AstNode *head() { return head_; }
  SeqNode *tail() { return tail_; }

  bool populateSymTable();

private:
  AstNode *head_;
  SeqNode *tail_;  
};

class BlockNode: public AstNode {
public:
  BlockNode(int line, SeqNode *body) 
    : AstNode(line, BLOCK_NODE, UNKNOWN_TYPE),
      body_(body) {
  }
  ~BlockNode() {}

  void print(ostream& /*os*/, int /*indent*/ = 0) const {
  }
  Value* eval();

private:
  SeqNode* body_;
};

class MethodDefNode: public AstNode {
public:
  MethodDefNode(int line, const string &name, int, TypeTag type)
    : AstNode(line, METHOD_DEF_NODE, type),
      name_(name) {
  }
  ~MethodDefNode() {}

  void print(ostream& /*os*/, int /*indent*/ = 0) const {}
  Value* eval();
  
  string name() const { return name_; }

  void setFormals(SeqNode *v) {formals_ = v; }
  void setBody(BlockNode *v) { body_ = v; }
  
  SeqNode *formals() { return formals_; }

  BlockNode *body() { return body_; }

private:
  string name_;
  Modifier mod_;
  int arity_;
  AstNode **args_;
  SeqNode *formals_;
  BlockNode *body_;
};

class MethodNode: public AstNode {
public:
  MethodNode(int line, const string &name, SeqNode* args)
    : AstNode(line, METHOD_INVOKE_NODE, UNKNOWN_TYPE), 
      name_(name),
      args_(args) {
  }
  ~MethodNode() {}

  void print(ostream& /*os*/, int /*indent*/ = 0) const {}
  Value* eval();

private:
  string name_;
  int arity_;
  SeqNode *args_;
};

enum BinOpCode {MULT = 0, DIV, MOD, ADD, SUB, AND, OR, EQ, NE, GT, GE,
                LT, LE, EXPONENT, ASSIGN};

class BinOpNode: public AstNode {
public:
  BinOpNode(int line, BinOpCode code, AstNode *expr1, AstNode *expr2)
    : AstNode(line, BINARY_OP_NODE, UNKNOWN_TYPE),
      op_(code),
      arg1_(expr1),
      arg2_(expr2) {
  }
  ~BinOpNode() {}

  void print(ostream& /*os*/, int /*indent*/ = 0) const {}
  Value* eval();

private:
  BinOpCode op_;
  AstNode *arg1_;
  AstNode *arg2_;
};

enum UnaryOpCode {PRE_INCR = 0, PRE_DECR, POST_INCR, POST_DECR, NEG, POS, NOT};

class UnaryOpNode: public AstNode {
public:
  UnaryOpNode(int line, UnaryOpCode code, AstNode *expr)
    : AstNode(line, UNARY_OP_NODE, UNKNOWN_TYPE),
      op_(code),
      arg_(expr) {
  }
  ~UnaryOpNode() {}

  void print(ostream& /*os*/, int /*indent*/ = 0) const {}
  Value* eval();

private:
  UnaryOpCode op_;
  AstNode *arg_;
};

class ReturnNode: public AstNode {
public:
  ReturnNode(int line, AstNode *retVal = 0) : 
    AstNode(line, RETURN_NODE, UNKNOWN_TYPE),
    intToFloatCoerce_(false),
    rv_(retVal) {
  }
  ~ReturnNode() {}

  void print(ostream& /*os*/, int /*indent*/ = 0) const {}
  Value* eval();

private:
  bool intToFloatCoerce_;
  AstNode *rv_;
};

class BreakNode: public AstNode {
public:
  BreakNode(int line, int depth) : 
    AstNode(line, BREAK_NODE, UNKNOWN_TYPE),
    depth_(depth) {
  }
  ~BreakNode() {}

  void print(ostream& /*os*/, int /*indent*/ = 0) const {}
  Value* eval();

private:
  int depth_;
};

class ContinueNode: public AstNode {
public:
  ContinueNode(int line, int depth) : 
    AstNode(line, CONTINUE_NODE, UNKNOWN_TYPE),
    depth_(depth) {
  }
  ~ContinueNode() {}

  void print(ostream& /*os*/, int /*indent*/ = 0) const {}
  Value* eval();

private:
  int depth_;
};

class IfNode: public AstNode {
public:
  IfNode(int line, AstNode* cond, AstNode *then, AstNode *els = NULL) 
    : AstNode(line, IF_NODE, UNKNOWN_TYPE),
      cond_(cond),
      then_(then),
      else_(els) {
  }
  ~IfNode() {}

  void print(ostream& /*os*/, int /*indent*/ = 0) const {}
  Value* eval();

private:
  AstNode* cond_;
  AstNode *then_;
  AstNode *else_;
};

class WhileNode: public AstNode {
public:
  WhileNode(int line, AstNode *cond, AstNode *body) 
    : AstNode(line, WHILE_NODE, UNKNOWN_TYPE),
      cond_(cond),
      body_(body) {
  }
  ~WhileNode() {}

  void print(ostream& /*os*/, int /*indent*/ = 0) const {}
  Value* eval();
  
private:
  AstNode *cond_;
  AstNode *body_;
};

class ForNode: public AstNode {
public:
  ForNode(int line, AstNode *init, AstNode *cond, AstNode *step, AstNode *body) 
    : AstNode(line, FOR_NODE, UNKNOWN_TYPE),
      init_(init),
      cond_(cond),
      step_(step),
      body_(body) {
  }
  ~ForNode() {}

  void print(ostream& /*os*/, int /*indent*/ = 0) const {}
  Value* eval();

private:
  AstNode *init_;
  AstNode *cond_;
  AstNode *step_;
  AstNode *body_;
};

class CaseNode: public AstNode {
public:
  CaseNode(int line, AstNode *cas, BlockNode *caseBody)
    : AstNode(line, CASE_NODE, UNKNOWN_TYPE),
      case_(cas),
      body_(caseBody) {
  }
  CaseNode(int line, BlockNode *caseBody)
    : AstNode(line, CASE_NODE, UNKNOWN_TYPE),
      case_(NULL),
      body_(caseBody) {
  }    
  ~CaseNode() {}

  void print(ostream& /*os*/, int /*indent*/ = 0) const {}
  // Value* eval();

private:
  AstNode *case_;
  BlockNode *body_;
};

class CaseList {
public:
  CaseList(AstNode *first) 
    : head_(first),
      tail_(NULL) {
  }
  ~CaseList() {}

  CaseList *append(CaseNode *) {}
  void print(ostream& /*os*/, int /*indent*/ = 0) const {}
  // Value* eval();

private:
  AstNode *head_;
  CaseList *tail_;
};

class SwitchNode: public AstNode {
public:
  SwitchNode(int line, AstNode *expr, CaseList *cases) 
    : AstNode(line, SWITCH_NODE, UNKNOWN_TYPE),
      expr_(expr),
      cases_(cases) {
  }
  ~SwitchNode() {}

  void print(ostream& /*os*/, int /*indent*/ = 0) const {}
  // Value* eval();

private:
  AstNode *expr_;
  CaseList *cases_;
};

class PrintNode: public AstNode {
public:
  PrintNode(int line, SeqNode* exprs)
    : AstNode(line, PRINT_NODE, UNKNOWN_TYPE),
      exprs_(exprs) {
  }
  ~PrintNode() {}

  void print(ostream& /*os*/, int /*indent*/ = 0) const {}
  Value* eval();

private:
  SeqNode *exprs_;
  int arity_;

  bool evalExprs(SeqNode *s);
};

class CastNode : public AstNode {
public:
  CastNode(int line, TypeTag type, AstNode *expr)
    : AstNode(line, CAST_NODE, type),
      expr_(expr) {
  }
  ~CastNode() {}

  void print(ostream& /*os*/, int /*indent*/ = 0) const {}
  Value* eval();

private:
  AstNode *expr_;
};

class InitialNode : public AstNode {
public:
  InitialNode(int line, AstNode *expr)
    : AstNode(line, INITIAL_NODE, VOID_TYPE),
      expr_(expr),
      expr_value_(NULL) {
  }
  ~InitialNode() {}

  void print(ostream& /*os*/, int /*indent*/ = 0) const {}
  Value* eval();

  void setAssignment(AstNode *v) { expr_value_ = v; }

private:
  AstNode *expr_;
  AstNode *expr_value_;
};

class KCNode : public AstNode {
public:
  KCNode(int line, AstNode *expr)
    : AstNode(line, KC_NODE, VOID_TYPE),
      expr_(expr),
      expr_value_(NULL) {
  }
  ~KCNode() {}

  void print(ostream& /*os*/, int /*indent*/ = 0) const {}
  Value* eval();

  void setAssignment(AstNode *v) { expr_value_ = v; }

private:
  AstNode *expr_;
  AstNode *expr_value_;
};

class ResultNode : public AstNode {
public:
  ResultNode(int line, AstNode *name_expr, AstNode *time_expr)
    : AstNode(line, RESULT_NODE, VOID_TYPE),
      name_expr_(name_expr),
      time_expr_(time_expr) {
  }
  ~ResultNode() {}

  void print(ostream& /*os*/, int /*indent*/ = 0) const {}
  Value* eval();

private:
  AstNode *name_expr_;
  AstNode *time_expr_;
};

class SimulateNode : public AstNode {
public:
  SimulateNode(int line)
    : AstNode(line, SIMULATE_NODE, VOID_TYPE) {
  }
  ~SimulateNode() {}

  void print(ostream& /*os*/, int /*indent*/ = 0) const {}
  Value* eval();

private:
};

enum MathOp {MATH_ABS,
             MATH_ACOS, MATH_ASIN, MATH_ATAN,
	     MATH_COS,  MATH_SIN,  MATH_TAN,
	     MATH_EXP,  MATH_LOG,  MATH_SQRT,
             MATH_SQR };

class MathNode : public AstNode {
public:
  MathNode(int line, MathOp op, AstNode *expr)
    : AstNode(line, MATH_NODE, UNKNOWN_TYPE),
      op_(op),
      expr_(expr) {
  }
  ~MathNode() {}

  void print(ostream& /*os*/, int /*indent*/ = 0) const {}
  Value* eval();

private:
  MathOp op_;
  AstNode *expr_;
};

#endif
