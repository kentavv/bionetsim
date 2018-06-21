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

#include <iostream>

#include <stdlib.h>
#include <math.h>

#include "AstNode.h"
#include "SymTab.h"
#include "STEClasses.h"
#include "SymTabMgr.h"

#include "log.h"
#include "matrix.h"
#include "pathway_matrix.h"
#include "result.h"
#include "calculate.h"

using namespace std;

SymTab global(GLOBAL);
SymTabMgr symTabMgr(&global);

//extern int symTabErrCheck(SymTabEntry *se);

#ifndef STAND_ALONE
extern PathwayMatrix FFpem;
extern Result FFresults;
#endif

int debugf = 0;
static ostream *pff_log = new ostream(new logbuf);
ostream &ff_log = *pff_log;

int
fitnessFunctionDebug() {
	return debugf;
}

void
setFitnessFunctionDebug(int v) {
	debugf = v;
}

#if 0
int symTabErrCheck(SymTabEntry *se)
{
	ff_log << "Checking for var: " << se->name() << endl;

	if (symTabMgr.enter(se) != OK) {
		ff_log << " line " << yyline << ": ";
		ff_log << "Error: Duplicate Symbol `" << se->name() << "'\n";
		return 0;
	}
	return 1;
}
#endif

Value * VarNode::eval() {
	if(debugf > 0) {
		ff_log << "VarNode::eval() >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n";
		ff_log << "Creating var " << name_ << '\n';
	}

	VarEntry *ve = new VarEntry(name_, mod_, type_, subscripts_);
	SymTabErr r = symTabMgr.enter(ve);
	if(r != OK) {
		if(r == DUPLICATE_SYM) {
			if(debugf > 0) {
				ff_log << "Duplicate symbol " << name_ << " on line " << line_ << '\n';
			}
			pneWarning("SCRIPT: Duplicate symbol %s detected on line %d.\n", name_.c_str(), line_);
		} else {
			if(debugf > 0) {
				ff_log << "Error on line " << line_ << '\n';
			}
			pneWarning("SCRIPT: Error on line %d.\n", line_);
		}
		return new Value(ERROR_TYPE);
	}

	if(debugf > 0) {
		ff_log << "VarNode::eval() <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n";
	}
	return ve->value();
}

Value* ValueNode::eval()
{
	if(debugf > 0) {
		ff_log << "ValueNode::eval() >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n";
	}
	if(debugf > 0) {
		ff_log << "ValueNode::eval() <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n";
	}
	return val_;
}

Value * RefExprNode::eval()
{
	if(debugf > 0) {
		ff_log << "RefExprNode::eval() >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n";
	}

	SymTabEntry *se = symTabMgr.lookUp(name_);

	if(debugf > 0) {
		ff_log << "Looking for " << name_ << " in:\n";
		symTabMgr.print(ff_log);
		ff_log << "===================\n";
	}

	Value *rv = NULL;
	if(se) {
		if(debugf > 0) {
			ff_log << "symbol found\n";
		}
		if(se->kind() == VAR) {
			if(debugf > 0) {
				ff_log << "symbol is a variable\n";
			}
			Value *v = ((VarEntry*)se)->value();
			if(v->type() == INT_MATRIX_TYPE ||
				v->type() == FLOAT_MATRIX_TYPE ||
				v->type() == STRING_MATRIX_TYPE ||
				v->type() == BOOL_MATRIX_TYPE) {
					if(debugf > 0) {
						ff_log << "symbol is a matrix\n";
					}
					if(!ss_) {
						rv = v;
					} else {
						int ind = calcIndex(ss_, v->subscripts().size(), v->subscripts());
						if(debugf > 0) {
							ff_log << "ind: " << ind << endl;
						}
						if(ind == -1) {
						} else {
							rv = v->index(ind);
						}
					}
			} else {
				if(ss_) {
				} else {
					rv = v;
				}
			}
		} else {
			if(debugf > 0) {
				ff_log << "symbol is not a variable\n";
			}
			if(debugf > 0) {
				ff_log << "Symbol " << name_ << " is not a variable on line " << line_ << '\n';
			}
			pneWarning("SCRIPT: Symbol %s is not a variable line line %d.\n", name_.c_str(), line_);
			rv = new Value(ERROR_TYPE);
		}
	} else {
		if(debugf > 0) {
			ff_log << "symbol not found\n";
		}
		if(debugf > 0) {
			ff_log << "Symbol " << name_ << " not found on line " << line_ << '\n';
		}
		pneWarning("SCRIPT: Symbol %s not found on line %d.\n", name_.c_str(), line_);
		rv = new Value(ERROR_TYPE);
	}

	if(debugf > 0) {
		if(rv) {
			ff_log << "value from ref: ";
			rv->print(ff_log);
			ff_log << '\n';
		} else {
			ff_log << "value == NULL\n";
		}
	}

	if(debugf > 0) {
		ff_log << "RefExprNode::eval() <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n";
	}

	return rv;
}

int
RefExprNode::calcIndex(SeqNode *ss, int dim, const vector<int> &subscripts) {
	if(!ss && dim < subscripts.size()) {
		ff_log << "Incorrect number of subscripts\n";
		return -1;
	}

	int rv = 0;
	int n = 0;

	Value *v;
	if(ss->tail()) {
		n = calcIndex(ss->tail(), dim-1, subscripts); 
		if(debugf > 0) {
			ff_log << "n: " << n << endl;
		}
		if(n == -1) {
			if(debugf > 0) {
				ff_log << "n == -1\n";
			}
			return -1;
		}
		rv = n;
	}

	if(ss->head()) {
		v = ss->head()->eval();
		if(!v) {
			if(debugf > 0) {
				ff_log << "v == NULL\n";
			}
			return -1;      
		}
		if(v->type() != INT_TYPE) {
			ff_log << "Only integers can be used in matrix indices\n";
			return -1;
		}
		int j = v->getIVal();
		if(j < 0 || subscripts[dim-1] <= j) {
			if(debugf > 0) {
				ff_log << "j: " << j << endl;
			}
			ff_log << "Index out of range for dim " << dim << " [0," << subscripts[dim-1] << "]\n";
			return -1;
		}
		if(dim == subscripts.size()) {
			rv += j;
		} else {
			int off = 1;
			for(int i=dim; i<subscripts.size(); i++) {
				off *= subscripts[i];
			}
			if(debugf > 0) {
				ff_log << "Using off: " << off << endl;
				ff_log << "rv += j * off: " << rv << ' ' << j << ' ' << off << '\n';
			}
			rv += j * off;
		}
		if(debugf > 0) {
			ff_log << "v: " << v->getIVal() << endl;
		}
	} else {
		if(debugf > 0) {
			ff_log << "head == NULL\n";
		}
		return -1;
	}
	if(debugf > 0) {
		ff_log << "rv: " << rv << endl;
	}
	return rv;
}

Value * SeqNode::eval()
{
	if(debugf > 0) {
		ff_log << "SeqNode::eval() >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n";
	}

	Value *v;

	if(tail_) {
		if(debugf > 0) {
			ff_log << "processing tail\n";
		}

		v = tail_->eval();

		if(debugf > 0) {
			if(v) {
				ff_log << "value from tail:\n";
				v->print(ff_log);
				ff_log << '\n';
			} else {
				ff_log << "tail return value == NULL\n";
			}
		}

		if(v && v->type() == ERROR_TYPE) return v;

		if(v) {
			if((v->type() == LOOP_CONTROL_BREAK ||
				v->type() == LOOP_CONTROL_CONTINUE) &&
				v->getDepth() > 0) return v;
		}
	}

	if(head_) {
		if(debugf > 0) {
			ff_log << "processing head\n";
		}

		v = head_->eval();

		if(debugf > 0) {
			if(v) {
				ff_log << "value from head:\n";
				v->print(ff_log);
				ff_log << '\n';
			} else {
				ff_log << "head return value == NULL\n";
			}
		}

		if(v && v->type() == ERROR_TYPE) return v;

		if(v) {
			if((v->type() == LOOP_CONTROL_BREAK ||
				v->type() == LOOP_CONTROL_CONTINUE) &&
				v->getDepth() > 0) return v;
		}
	}

	if(debugf > 0) {
		ff_log << "SeqNode::eval() <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n";
	}

	return v;
}

bool
SeqNode::populateSymTable() { 
	if(debugf > 0) {
		ff_log << "SeqNode::populateSymTable() >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n";
	}

	if(head_) {
		if(debugf > 0) {
			ff_log << "Processing head...\n";
		}
		if(head_->nodeTag() == METHOD_DEF_NODE) {
			MethodEntry *me = new MethodEntry(((MethodDefNode*)head_)->name(), 0, head_->type());
			me->method((MethodDefNode*)head_);
			SymTabErr r = global.enter(me);
			if(r != OK) {
				if(r == DUPLICATE_SYM) {
					ff_log << "Duplicate symbol " << ((MethodDefNode*)head_)->name() << " on line " << line_ << '\n';
				} else {
					ff_log << "Unable to add symbol " << ((MethodDefNode*)head_)->name() << " on line " << line_ << '\n';
				}
				return false;
			}
		} else {
			if(debugf > 0) {
				ff_log << "head does not refer to a function\n";
			}
			ff_log << "Head does not refer to a function for symbol " << ((MethodDefNode*)head_)->name() << " on line " << line_ << '\n';
			return false;
		}
	}
	if(tail_) {
		if(debugf > 0) {
			ff_log << "Processing tail...\n";
		}
		if(!tail_->populateSymTable()) return false;
	}

	if(debugf > 0) {
		ff_log << "SeqNode::populateSymTable() <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n";
	}

	return true;
}

Value * BlockNode::eval()
{
	if(debugf > 0) {
		ff_log << "BlockNode::eval() >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n";
	}

	symTabMgr.enterScope(BLOCK);

	Value * rv = body_ ? body_->eval() : NULL;

	symTabMgr.leaveScope();

	if(debugf > 0) {
		if(rv) {
			ff_log << "value from body: ";
			rv->print(ff_log);
			ff_log << '\n';
		} else {
			ff_log << "value == NULL\n";
		}
	}

	if(debugf > 0) {
		ff_log << "BlockNode::eval() <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n";
	}

	return rv;
}

Value * MethodDefNode::eval()
{
	if(debugf > 0) {
		ff_log << "MethodDefNode::eval() >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n";
	}

	Value *rv = NULL;

	if(body_) {
		rv = body_->eval();
	} else {
		if(debugf > 0) {
			ff_log << "body_ == NULL\n";
		}
	}

	if(debugf > 0) {
		if(rv) {
			ff_log << "Value: ";
			rv->print(ff_log);
			ff_log << '\n';
		} else {
			ff_log << "value == NULL\n";
		}
	}

	if(debugf > 0) {
		ff_log << "MethodDefNode::eval() <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n";
	}

	return rv ? rv : new Value(ERROR_TYPE);
}

Value * MethodNode::eval()
{
	if(debugf > 0) {
		ff_log << "MethodNode::eval() >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n";
	}

	SymTabEntry *se = symTabMgr.globalScope()->lookUp(name_);

	if(debugf > 0) {
		ff_log << "Looking for " << name_ << " in:\n";
		symTabMgr.globalScope()->print(ff_log);
		ff_log << "===================" << '\n';
	}

	Value *rv=NULL;

	if(se) {
		if(debugf > 0) {
			ff_log << "symbol found\n";
		}
		if(se->kind() == GLOBAL_FUN) {
			if(debugf > 0) {
				ff_log << "symbol is a function\n";
			}

			MethodEntry * me = (MethodEntry*)se;

			// process expressions for arguments in current scope
			if(debugf > 0) {
				ff_log << "Processing expression for arguments...\n";
			}
			list<Value*> values;
			SeqNode *i = args_;
			while(i) {
				Value *v = i->head()->eval();

				if(debugf > 0) {
					if(v) {
						ff_log << "value: ";
						v->print(ff_log);
						ff_log << '\n';
					} else {
						ff_log << "v == NULL\n";
					}
				}
				values.push_back(v);

				i = i->tail();
			}

			// Setup new scope to construct formals in
			symTabMgr.enterScope(FUNC_FORMALS);

			// Assign previously evaluated expressions to values of formals
			// formals is a SeqNode of VarNodes, args_ is a SeqNode of AstNode (nodeVals)
			i = me->method()->formals();
			list<Value*>::iterator j = values.begin();
			// the parameter count will already have been completed by this point by a
			// prepass process

			if(debugf > 0) {
				ff_log << "processing function formals assigning values from expressions...\n";
			}

			while(i && j!=values.end()) {
				Value *v = i->eval();

				if(v) {
					v->set((*j)->getIVal());
				}

				if(debugf > 0) {
					if(v) {
						ff_log << "final arg value: ";
						v->print(ff_log);
						ff_log << '\n';
					} else {
						ff_log << "arg == NULL\n";
					}
				}

				i = i->tail();
				j++;
			}

			// Execute body... first node will be a BlockNode so new scope will be
			// setup
			if(debugf > 0) {
				ff_log << "Executing body of function...\n";
			}
			rv = me->method()->eval();

			if(debugf > 0) {
				if(rv) {
					ff_log << "value from function: ";
					rv->print(ff_log);
					ff_log << '\n';
				} else {
					ff_log << "return value == NULL\n";
				}
			}

			symTabMgr.leaveScope();
		} else {
			if(debugf > 0) {
				ff_log << "symbol is not a function\n";
			}
		}
	} else {
		if(debugf > 0) {
			ff_log << "symbol not found\n";
		}
	}

	if(debugf > 0) {
		ff_log << "MethodNode::eval() <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n";
	}

	return rv;
}

Value * BinOpNode::eval()
{
	if(debugf > 0) {
		ff_log << "BinOpNode::eval() >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n";
	}

	Value *v1=NULL, *v2=NULL, *rv=NULL;

	if(debugf > 0) {
		if(arg1_) {
			ff_log << "arg1 defined\n";
		} else {
			ff_log << "arg1 == NULL\n";
		}
		if(arg2_) {
			ff_log << "arg2 defined\n";
		} else {
			ff_log << "arg2 == NULL\n";
		}
	}

	if(arg1_) v1=arg1_->eval();
	if(arg2_) v2=arg2_->eval();

	if(debugf > 0) {
		if(v1) {
			ff_log << "v1 value: ";
			v1->print(ff_log);
			ff_log << '\n';
		} else {
			ff_log << "v1 == NULL\n";
		}
		if(v2) {
			ff_log << "v2 value: ";
			v2->print(ff_log);
			ff_log << '\n';
		} else {
			ff_log << "v2 == NULL\n";
		}
	}

	if(v1 && v2) {
		if(debugf > 0) {
			v1->print(ff_log);
			switch(op_) {
			case MULT:     ff_log << "\t*\t";  break;
			case DIV:      ff_log << "\t/\t";  break;
			case MOD:      ff_log << "\t%\t";  break;
			case ADD:      ff_log << "\t+\t";  break;
			case SUB:      ff_log << "\t-\t";  break;
			case AND:      ff_log << "\t&&\t"; break;
			case OR:       ff_log << "\t||\t"; break;
			case EQ:       ff_log << "\t==\t"; break;
			case NE:       ff_log << "\t!=\t"; break;
			case GT:       ff_log << "\t>\t";  break;
			case GE:       ff_log << "\t>=\t"; break;
			case LT:       ff_log << "\t<\t";  break;
			case LE:       ff_log << "\t<=\t"; break;
			case EXPONENT: ff_log << "\t^\t";  break;
			case ASSIGN:   ff_log << "\t=\t";  break;
			default:       ff_log << "\top=" << op_ << '\t'; break;
			}
			v2->print(ff_log);
			ff_log << '\n';
		}

		TypeTag t1 = v1->type();
		TypeTag t2 = v2->type();

		switch(op_) {
		case MULT:
			if(t1==INT_TYPE && t2==INT_TYPE) {
				rv = new Value(v1->getIVal() * v2->getIVal());
			} else if(t1==FLOAT_TYPE && t2==FLOAT_TYPE) {
				rv = new Value(v1->getFVal() * v2->getFVal());
			} else if(t1==INT_TYPE && t2==FLOAT_TYPE) {
				rv = new Value(v1->getIVal() * v2->getFVal());
			} else if(t1==FLOAT_TYPE && t2==INT_TYPE) {
				rv = new Value(v1->getFVal() * v2->getIVal());
			}
			break;
		case DIV:
			if(t1==INT_TYPE && t2==INT_TYPE) {
				rv = new Value(v1->getIVal() / v2->getIVal());
			} else if(t1==FLOAT_TYPE && t2==FLOAT_TYPE) {
				rv = new Value(v1->getFVal() / v2->getFVal());
			} else if(t1==INT_TYPE && t2==FLOAT_TYPE) {
				rv = new Value(v1->getIVal() / v2->getFVal());
			} else if(t1==FLOAT_TYPE && t2==INT_TYPE) {
				rv = new Value(v1->getFVal() / v2->getIVal());
			}
			break;
		case MOD:
			if(t1==INT_TYPE && t2==INT_TYPE) {
				rv = new Value(v1->getIVal() % v2->getIVal());
			}
			break;
		case ADD:
			if(t1==INT_TYPE && t2==INT_TYPE) {
				rv = new Value(v1->getIVal() + v2->getIVal());
			} else if(t1==FLOAT_TYPE && t2==FLOAT_TYPE) {
				rv = new Value(v1->getFVal() + v2->getFVal());
			} else if(t1==INT_TYPE && t2==FLOAT_TYPE) {
				rv = new Value(v1->getIVal() + v2->getFVal());
			} else if(t1==FLOAT_TYPE && t2==INT_TYPE) {
				rv = new Value(v1->getFVal() + v2->getIVal());
			} else if(t1==STRING_TYPE && t2==STRING_TYPE) {
				rv = new Value(v1->getSVal() + v2->getSVal());
			}
			break;
		case SUB:
			if(t1==INT_TYPE && t2==INT_TYPE) {
				rv = new Value(v1->getIVal() - v2->getIVal());
			} else if(t1==FLOAT_TYPE && t2==FLOAT_TYPE) {
				rv = new Value(v1->getFVal() - v2->getFVal());
			} else if(t1==INT_TYPE && t2==FLOAT_TYPE) {
				rv = new Value(v1->getIVal() - v2->getFVal());
			} else if(t1==FLOAT_TYPE && t2==INT_TYPE) {
				rv = new Value(v1->getFVal() - v2->getIVal());
			}
			break;
		case AND:
			if(t1==BOOL_TYPE && t2==BOOL_TYPE) {
				rv = new Value(v1->getBVal() && v2->getBVal());
			}
			break;
		case OR:
			if(t1==BOOL_TYPE && t2==BOOL_TYPE) {
				rv = new Value(v1->getBVal() || v2->getBVal());
			}
			break;
		case EQ:
			if(t1==INT_TYPE && t2==INT_TYPE) {
				rv = new Value(v1->getIVal() == v2->getIVal());
			} else if(t1==FLOAT_TYPE && t2==FLOAT_TYPE) {
				rv = new Value(v1->getFVal() == v2->getFVal());
			} else if(t1==INT_TYPE && t2==FLOAT_TYPE) {
				rv = new Value(v1->getIVal() == v2->getFVal());
			} else if(t1==FLOAT_TYPE && t2==INT_TYPE) {
				rv = new Value(v1->getFVal() == v2->getIVal());
			} else if(t1==BOOL_TYPE && t2==BOOL_TYPE) {
				rv = new Value(v1->getBVal() == v2->getBVal());
			} else if(t1==STRING_TYPE && t2==STRING_TYPE) {
				rv = new Value(v1->getSVal() == v2->getSVal());
			}
			break;
		case NE: 
			if(t1==INT_TYPE && t2==INT_TYPE) {
				rv = new Value(v1->getIVal() != v2->getIVal());
			} else if(t1==FLOAT_TYPE && t2==FLOAT_TYPE) {
				rv = new Value(v1->getFVal() != v2->getFVal());
			} else if(t1==INT_TYPE && t2==FLOAT_TYPE) {
				rv = new Value(v1->getIVal() != v2->getFVal());
			} else if(t1==FLOAT_TYPE && t2==INT_TYPE) {
				rv = new Value(v1->getFVal() != v2->getIVal());
			} else if(t1==BOOL_TYPE && t2==BOOL_TYPE) {
				rv = new Value(v1->getBVal() != v2->getBVal());
			} else if(t1==STRING_TYPE && t2==STRING_TYPE) {
				rv = new Value(v1->getSVal() != v2->getSVal());
			}
			break;
		case GT: 
			if(t1==INT_TYPE && t2==INT_TYPE) {
				rv = new Value(v1->getIVal() > v2->getIVal());
			} else if(t1==FLOAT_TYPE && t2==FLOAT_TYPE) {
				rv = new Value(v1->getFVal() > v2->getFVal());
			} else if(t1==INT_TYPE && t2==FLOAT_TYPE) {
				rv = new Value(v1->getIVal() > v2->getFVal());
			} else if(t1==FLOAT_TYPE && t2==INT_TYPE) {
				rv = new Value(v1->getFVal() > v2->getIVal());
			} else if(t1==STRING_TYPE && t2==STRING_TYPE) {
				rv = new Value(v1->getSVal() > v2->getSVal());
			}
			break;
		case GE:
			if(t1==INT_TYPE && t2==INT_TYPE) {
				rv = new Value(v1->getIVal() >= v2->getIVal());
			} else if(t1==FLOAT_TYPE && t2==FLOAT_TYPE) {
				rv = new Value(v1->getFVal() >= v2->getFVal());
			} else if(t1==INT_TYPE && t2==FLOAT_TYPE) {
				rv = new Value(v1->getIVal() >= v2->getFVal());
			} else if(t1==FLOAT_TYPE && t2==INT_TYPE) {
				rv = new Value(v1->getFVal() >= v2->getIVal());
			} else if(t1==STRING_TYPE && t2==STRING_TYPE) {
				rv = new Value(v1->getSVal() >= v2->getSVal());
			}
			break;
		case LT:
			if(t1==INT_TYPE && t2==INT_TYPE) {
				rv = new Value(v1->getIVal() < v2->getIVal());
			} else if(t1==FLOAT_TYPE && t2==FLOAT_TYPE) {
				rv = new Value(v1->getFVal() < v2->getFVal());
			} else if(t1==INT_TYPE && t2==FLOAT_TYPE) {
				rv = new Value(v1->getIVal() < v2->getFVal());
			} else if(t1==FLOAT_TYPE && t2==INT_TYPE) {
				rv = new Value(v1->getFVal() < v2->getIVal());
			} else if(t1==STRING_TYPE && t2==STRING_TYPE) {
				rv = new Value(v1->getSVal() < v2->getSVal());
			}
			break;
		case LE:
			if(t1==INT_TYPE && t2==INT_TYPE) {
				rv = new Value(v1->getIVal() <= v2->getIVal());
			} else if(t1==FLOAT_TYPE && t2==FLOAT_TYPE) {
				rv = new Value(v1->getFVal() <= v2->getFVal());
			} else if(t1==INT_TYPE && t2==FLOAT_TYPE) {
				rv = new Value(v1->getIVal() <= v2->getFVal());
			} else if(t1==FLOAT_TYPE && t2==INT_TYPE) {
				rv = new Value(v1->getFVal() <= v2->getIVal());
			} else if(t1==STRING_TYPE && t2==STRING_TYPE) {
				rv = new Value(v1->getSVal() <= v2->getSVal());
			}
			break;
		case EXPONENT:
			if(t1==INT_TYPE && t2==INT_TYPE) {
				int a = v1->getIVal();
				int b = 1;
				int n = v2->getIVal();
				for(int i=0; i<n; i++) {
					b *= a;
				}
				rv = new Value(b);
			} else if(t1==FLOAT_TYPE && t2==FLOAT_TYPE) {
				rv = new Value(pow(v1->getFVal(), v2->getFVal()));
			} else if(t1==FLOAT_TYPE && t2==INT_TYPE) {
				rv = new Value(pow(v1->getFVal(), v2->getIVal()));
			} else if(t1==INT_TYPE && t2==FLOAT_TYPE) {
				rv = new Value(pow((double)v1->getIVal(), v2->getFVal()));
			} else if(t1==STRING_TYPE && t2==INT_TYPE) {
				string a = v1->getSVal();
				string b;
				int n = v2->getIVal();
				for(int i=0; i<n; i++) {
					b += a;
				}
				rv = new Value(b);
			}
			break;
		case ASSIGN:
			if(t1==INT_TYPE && t2==INT_TYPE) {
				v1->set(v2->getIVal());
				rv = v1;
			} else if(t1==FLOAT_TYPE && t2==FLOAT_TYPE) {
				v1->set(v2->getFVal());
				rv = v1;
			} else if(t1==FLOAT_TYPE && t2==INT_TYPE) {
				v1->set((float) v2->getIVal());
				rv = v1;
			} else if(t1==BOOL_TYPE && t2==BOOL_TYPE) {
				v1->set(v2->getBVal());
				rv = v1;
			} else if(t1==STRING_TYPE && t2==STRING_TYPE) {
				v1->set(v2->getSVal());
				rv = v1;
			} else {
				if(debugf > 0) {
					ff_log << "Unable to assign " << typeStr(t2) << " to " << typeStr(t1) << " on line " << line_ << '\n';
				}
				pneWarning("SCRIPT: Unable to assign %s to %s on line %d.\n", typeStr(t2).c_str(), typeStr(t1).c_str(), line_);
				rv = NULL;
			}
			break;
		default:
			break;
		}
	}

	if(debugf > 0) {
		if(rv) {
			ff_log << "value = ";
			rv->print(ff_log);
			ff_log << '\n';
		} else {
			ff_log << "rv == NULL\n";
		}
	}

	if(debugf > 0) {
		ff_log << "BinOpNode::eval() <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n";
	}

	return rv ? rv : new Value(ERROR_TYPE);
}

Value * UnaryOpNode::eval()
{
	if(debugf > 0) {
		ff_log << "UnaryOpNode::eval() >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n";
	}

	Value *v=NULL, *rv=NULL;

	if(debugf > 0) {
		if(arg_) {
			ff_log << "arg_ defined\n";
		} else {
			ff_log << "arg_ == NULL\n";
		}
	}

	if(arg_) v=arg_->eval();

	if(debugf > 0) {
		if(v) {
			ff_log << "v value: ";
			v->print(ff_log);
			ff_log << '\n';
		} else {
			ff_log << "v == NULL\n";
		}
	}

	if(v) {
		if(debugf > 0) {
			v->print(ff_log);
			switch(op_) {
			case PRE_INCR:  ff_log << "\t (pre)++"; break;
			case PRE_DECR:  ff_log << "\t (pre)--"; break;
			case POST_INCR: ff_log << "\t (post)++"; break;
			case POST_DECR: ff_log << "\t (post)--"; break;
			case NEG:       ff_log << "\t-";  break;
			case POS:       ff_log << "\t+";  break;
			case NOT:       ff_log << "\t!";  break;
			default:        ff_log << "\top="<< op_ << '\t'; break;
			}
			ff_log << '\n';
		}

		TypeTag t = v->type();

		switch(op_) {
		case PRE_INCR:
			if(t==INT_TYPE) {
				v->set(v->getIVal()+1);
				rv = new Value(v->getIVal());
			}
			break;
		case PRE_DECR:
			if(t==INT_TYPE) {
				v->set(v->getIVal()-1);
				rv = new Value(v->getIVal());
			}
			break;
		case POST_INCR:
			if(t==INT_TYPE) {
				rv = new Value(v->getIVal());
				v->set(v->getIVal()+1);
			}
			break;
		case POST_DECR:
			if(t==INT_TYPE) {
				rv = new Value(v->getIVal());
				v->set(v->getIVal()-1);
			}
			break;
		case NEG:
			if(t==INT_TYPE) {
				rv = new Value(- v->getIVal());
			} else if(t==FLOAT_TYPE) {
				rv = new Value(- v->getFVal());
			}
			break;
		case POS:
			if(t==INT_TYPE) {
				rv = new Value(+ v->getIVal());
			} else if(t==FLOAT_TYPE) {
				rv = new Value(+ v->getFVal());
			}
			break;
		case NOT:
			if(t==BOOL_TYPE) {
				rv = new Value(! v->getBVal());
			}
			break;
		}
	}

	if(debugf > 0) {
		if(rv) {
			ff_log << "value = ";
			rv->print(ff_log);
			ff_log << '\n';
		} else {
			ff_log << "rv == NULL\n";
		}
	}

	if(debugf > 0) {
		ff_log << "UnaryOpNode::eval() <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n";
	}

	return rv;
}

Value * ReturnNode::eval()
{
	if(debugf > 0) {
		ff_log << "ReturnNode::eval() >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n";
	}

	Value *rv = NULL;

	if(rv_) {
		if(debugf > 0) {
			ff_log << "rv_ defined\n";
		}
		rv = rv_->eval();
	} else {
		if(debugf > 0) {
			ff_log << "rv_ == NULL\n";
		}
	}

	if(debugf > 0) {
		if(rv) {
			ff_log << "value = ";
			rv->print(ff_log);
			ff_log << '\n';
		} else {
			ff_log << "rv == NULL\n";
		}
	}

	if(debugf > 0) {
		ff_log << "ReturnNode::eval() <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n";
	}

	return rv;
}

Value * BreakNode::eval()
{
	if(debugf > 0) {
		ff_log << "BreakNode::eval() >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n";
	}

	Value *rv = new Value(LOOP_BREAK, depth_);

	if(debugf > 0) {
		ff_log << "BreakNode::eval() <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n";
	}

	return rv;
}

Value * ContinueNode::eval()
{
	if(debugf > 0) {
		ff_log << "ContinueNode::eval() >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n";
	}

	Value *rv = new Value(LOOP_CONTINUE, depth_);

	if(debugf > 0) {
		ff_log << "ContinueNode::eval() <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n";
	}

	return rv;
}

Value * IfNode::eval()
{
	if(debugf > 0) {
		ff_log << "IfNode::eval() >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n";
	}

	Value *rv=NULL;
	Value *cond_val=NULL;

	if(cond_) {
		cond_val=cond_->eval();
	}

	if(cond_val) {
		TypeTag t=cond_val->type();
		if(t==BOOL_TYPE) {
			if(cond_val->getBVal()) { 
				if(then_) {
					rv=then_->eval();
				}
			} else {
				if(else_) {
					rv=else_->eval();
				}
			}
		} else {
			if(debugf > 0) {
				ff_log << "if() expects Boolean conditional on line " << line_ << '\n';    
			}
			pneWarning("SCRIPT: if() expects Boolean conditional on line %d.\n", line_);
			return new Value(ERROR_TYPE);
		}
	}

	if(debugf > 0) {
		ff_log << "IfNode::eval() <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n";
	}

	return rv ? rv : new Value(VOID_TYPE);;
}

Value * WhileNode::eval()
{
	if(debugf > 0) {
		ff_log << "WhileNode::eval() >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n";
	}

	Value *rv=NULL;
	Value *cond_val=NULL;

	if(cond_) {
		cond_val=cond_->eval();
	}

	if(cond_val) {
		TypeTag t=cond_val->type();
		if(t==BOOL_TYPE) {
			while(cond_val->getBVal()) {      
				if(body_) {
					rv=body_->eval();

					if(rv) {
						if(rv->type() == LOOP_CONTROL_BREAK ||
							rv->type() == LOOP_CONTROL_CONTINUE) {
								rv->decLoopDepth();
								if(rv->getDepth() == 0) {
									if(rv->type() == LOOP_CONTROL_BREAK) break;
									if(rv->type() == LOOP_CONTROL_CONTINUE) {} // condition is eval'd next anyway
								} else {
									return rv;
								}
						}
					}
				}

				if(cond_) {
					cond_val=cond_->eval();
				}
			}      
		} else {
			if(debugf > 0) {
				ff_log << "while() expects Boolean conditional on line " << line_ << '\n';    
			}
			pneWarning("SCRIPT: while() expects Boolean conditional on line %d.\n", line_);
			return new Value(ERROR_TYPE);
		}
	}

	if(debugf > 0) {
		ff_log << "WhileNode::eval() <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n";
	}

	return rv;
}

Value * ForNode::eval()
{
	if(debugf > 0) {
		ff_log << "ForNode::eval() >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n";
	}

	symTabMgr.enterScope(BLOCK);

	Value *rv=NULL;
	if(init_) {
		init_->eval();
	}

	Value *cond_val=NULL;

	if(cond_) {
		cond_val=cond_->eval();
	}

	if(cond_val) {
		TypeTag t=cond_val->type();
		if(t==BOOL_TYPE) {
			while(cond_val->getBVal()) {      
				if(body_) {
					rv=body_->eval();

					if(rv) {
						if(rv->type() == LOOP_CONTROL_BREAK ||
							rv->type() == LOOP_CONTROL_CONTINUE) {
								rv->decLoopDepth();
								if(rv->getDepth() == 0) {
									if(rv->type() == LOOP_CONTROL_BREAK) break;
									if(rv->type() == LOOP_CONTROL_CONTINUE) {} // step is eval'd next anyway
								} else {
									return rv;
								}
						} else if(rv->type() == ERROR_TYPE) {
							return new Value(ERROR_TYPE);
						}
					}
				}

				if(step_) {
					step_->eval();
				}

				if(cond_) {
					cond_val=cond_->eval();
				}
			} 
		} else {
			if(debugf > 0) {
				ff_log << "for() expects Boolean conditional on line " << line_ << '\n';    
			}
			pneWarning("SCRIPT: for() expects Boolean conditional on line %d.\n", line_);
			return new Value(ERROR_TYPE);
		}
	}

	symTabMgr.leaveScope();

	if(debugf > 0) {
		ff_log << "ForNode::eval() <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n";
	}

	return rv;
}

Value *PrintNode::eval() {
	if(debugf > 0) {
		ff_log << "PrintNode::eval() >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n";
	}

	SeqNode *s = exprs_;
	if(!evalExprs(s)) return new Value(ERROR_TYPE);

	if(debugf > 0) {
		ff_log << "PrintNode::eval() <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n";
	}

	//  return NULL;
	return new Value(VOID_TYPE);
}

bool
PrintNode::evalExprs(SeqNode *s) {
	if(s) {
		AstNode *head = s->head();
		SeqNode *tail = s->tail();

		if(!evalExprs(tail)) return false;

		Value *v;

		if(head) {
			//ff_log << "processing head\n";

			v = head->eval();

			if(v) {
				//ff_log << "value from head:\n";
				//v->print(ff_log);
				//ff_log << '\n';
#ifdef STAND_ALONE
				switch(v->type()) {
				case INT_TYPE:    ff_log << v->getIVal();                      break;
				case FLOAT_TYPE:  ff_log << v->getFVal();                      break;
				case STRING_TYPE: ff_log << v->getSVal();                      break;
				case BOOL_TYPE:   ff_log << (v->getBVal() ? "true" : "false"); break;
				case ERROR_TYPE:  return false;
				case UNKNOWN_TYPE:
				case VOID_TYPE:
					break;
				}
#else
				switch(v->type()) {
				case INT_TYPE:    pnePrint("%d", v->getIVal());              break;
				case FLOAT_TYPE:  pnePrint("%f", v->getFVal());              break;
				case STRING_TYPE: pnePrint("%s", v->getSVal().c_str());      break;
				case BOOL_TYPE:   pnePrint("%s", v->getBVal() ? "true" : "false"); break;
				case ERROR_TYPE:  return false;
				case UNKNOWN_TYPE:
				case VOID_TYPE:
					break;
				}
#endif
			} else {
				//ff_log << "head return value == NULL\n";
				ff_log << "(null)";
				return false;
			}
		} else {
			return false;
		}
	}

	return true;
}

Value * CastNode::eval()
{
	if(debugf > 0) {
		ff_log << "CastNode::eval() >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n";
	}

	Value *v=NULL, *rv=NULL;

	if(debugf > 0) {
		if(expr_) {
			ff_log << "expr_ defined\n";
		} else {
			ff_log << "expr_ == NULL\n";
		}
	}

	if(expr_) v=expr_->eval();

	if(!expr_ || !v || v->type() == ERROR_TYPE) return new Value(ERROR_TYPE);

	if(debugf > 0) {
		if(v) {
			ff_log << "v value: ";
			v->print(ff_log);
			ff_log << '\n';
		} else {
			ff_log << "v == NULL\n";
		}
	}

	if(v) {
		if(debugf > 0) {
			v->print(ff_log);
		}

		switch(v->type()) {
		case INT_TYPE:
			switch(type_) {
		case INT_TYPE:
			rv = new Value((int)v->getIVal()); break;
		case BOOL_TYPE:
			rv = new Value(v->getIVal() != 0); break;
		case STRING_TYPE:
			rv = NULL;
		case FLOAT_TYPE:
			rv = new Value((float)v->getIVal()); break;
			}
			break;
		case BOOL_TYPE:
			switch(type_) {
		case INT_TYPE:
			rv = new Value(v->getBVal() ? 1 : 0); break;
		case BOOL_TYPE:
			rv = new Value(v->getBVal()); break;
		case STRING_TYPE:
			rv = new Value(v->getBVal() ? string("true") : string("false")); break;
		case FLOAT_TYPE:
			rv = new Value(v->getBVal() ? 1.0f : 0.0f); break;
			}
			break;
		case STRING_TYPE:
			break;
		case FLOAT_TYPE:
			switch(type_) {
		case INT_TYPE:
			rv = new Value((int)v->getFVal()); break;
		case BOOL_TYPE:
			rv = new Value(v->getFVal() != 0.0); break;
		case STRING_TYPE:
			rv = NULL;
		case FLOAT_TYPE:
			rv = new Value((float)v->getFVal()); break;
			}
			break;
		}
	}

	if(debugf > 0) {
		if(rv) {
			ff_log << "value = ";
			rv->print(ff_log);
			ff_log << '\n';
		} else {
			ff_log << "rv == NULL\n";
		}
	}

	if(debugf > 0) {
		ff_log << "CastNode::eval() <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n";
	}

	return rv ? rv : new Value(ERROR_TYPE);
}

Value* 
InitialNode::eval() {
	if(debugf > 0) {
		ff_log << "InitialNode::eval() >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n";
	}

	Value *v = NULL, *rv = NULL;

	if(debugf > 0) {
		if(expr_) {
			ff_log << "expr_ defined\n";
		} else {
			ff_log << "expr_ == NULL\n";
		}
	}

	if(expr_) v=expr_->eval();

#ifndef STAND_ALONE

	if(!expr_ || !v || v->type() == ERROR_TYPE) {
		if(debugf > 0) {
			ff_log << "Failed to evaulate name passed to initial() on line " << line_ << '\n';    
		}
		pneWarning("SCRIPT: Failed to evaulate name passed to initial() on line %d.\n", line_);
		return new Value(ERROR_TYPE);
	}

	if(v) {
		if(v->type() != STRING_TYPE) {
			if(debugf > 0) {
				ff_log << "Name passed to initial() must be string type on line " << line_ << '\n';
			}
			pneWarning("SCRIPT: Name passed to initial() must be string type on line %d.\n", line_);
			return new Value(ERROR_TYPE);
		}
		string mol = v->getSVal();
		// lookup value in Initial matrix using name
		int ind = FFpem.moleculeLookup(mol.c_str());
		if(ind == -1) {
			if(debugf > 0) {
				ff_log << "Name passed to initial() not valid on line " << line_ << '\n';
			}
			pneWarning("SCRIPT: Name passed to initial() not valid on line %d.", line_);
			return new Value(ERROR_TYPE);
		}
		double res = FFpem.y0(ind, 0);
		if(expr_value_) {
			Value *v2 = expr_value_->eval();
			if(!v2) {
				return new Value(ERROR_TYPE);
			}
			if(v2->type() != INT_TYPE && v2->type() != FLOAT_TYPE) {
				if(debugf > 0) {
					ff_log << "Assignment to initial requires numerical value on line " << line_ << '\n';
				}
				pneWarning("SCRIPT: Assignment to initial requires numerical value on line %d.", line_);
				return new Value(ERROR_TYPE);
			}
			v2->print(ff_log);
			if(debugf > 0) {
				ff_log << "\n";
				ff_log << v2->getIVal() << endl;
				ff_log << v2->getFVal() << endl;
			}
			res = v2->type() == INT_TYPE ? v2->getIVal() : v2->getFVal();
			// assign v2 to initial
			FFpem.y0(ind, 0) = res;
		}
		rv = new Value(res);
	}

	if(debugf > 0) {
		ff_log << "Initial========\n";
		FFpem.y0.print(ff_log);
		ff_log << "Initial========\n";
	}

#endif

	if(debugf > 0) {
		if(rv) {
			ff_log << "value = ";
			rv->print(ff_log);
			ff_log << '\n';
		} else {
			ff_log << "rv == NULL\n";
		}
	}

	if(debugf > 0) {
		ff_log << "InitialNode::eval() <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n";
	}

	return rv ? rv : new Value(ERROR_TYPE);
}

Value* 
KCNode::eval() {
	if(debugf > 0) {
		ff_log << "KCNode::eval() >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n";
	}

	Value *v = NULL, *rv = NULL;

	if(debugf > 0) {
		if(expr_) {
			ff_log << "expr_ defined\n";
		} else {
			ff_log << "expr_ == NULL\n";
		}
	}

	if(expr_) v=expr_->eval();

#ifndef STAND_ALONE

	if(!expr_ || !v || v->type() == ERROR_TYPE) {
		if(debugf > 0) {
			ff_log << "Failed to evaulate name passed to kc() on line " << line_ << '\n';    
		}
		pneWarning("SCRIPT: Failed to evaulate name passed to kc() on line %d.\n", line_);
		return new Value(ERROR_TYPE);
	}

	if(v) {
		if(v->type() != STRING_TYPE) {
			if(debugf > 0) {
				ff_log << "Name passed to kc() must be string type on line " << line_ << '\n';
			}
			pneWarning("SCRIPT: Name passed to kc() must be string type on line %d.\n", line_);
			return new Value(ERROR_TYPE);
		}
		string rxn = v->getSVal();
		// lookup value in KC matrix using name
		int ind = FFpem.reactionLookup(rxn.c_str());
		if(ind == -1) {
			if(debugf > 0) {
				ff_log << "Name passed to kc() not valid on line " << line_ << '\n';
			}
			pneWarning("SCRIPT: Name passed to kc() not valid on line %d.", line_);
			return new Value(ERROR_TYPE);
		}
		double res = FFpem.kc(ind, 0);
		if(expr_value_) {
			Value *v2 = expr_value_->eval();
			if(!v2) {
				return new Value(ERROR_TYPE);
			}
			if(v2->type() != INT_TYPE && v2->type() != FLOAT_TYPE) {
				if(debugf > 0) {
					ff_log << "Assignment to KC requires numerical value on line " << line_ << '\n';
				}
				pneWarning("SCRIPT: Assignment to KC requires numerical value on line %d.", line_);
				return new Value(ERROR_TYPE);
			}
			v2->print(ff_log);
			if(debugf > 0) {
				ff_log << '\n'
					<< v2->getIVal() << '\n'
					<< v2->getFVal() << '\n';
			}
			res = v2->type() == INT_TYPE ? v2->getIVal() : v2->getFVal();
			// assign v2 to kc
			FFpem.kc(ind, 0) = res;
		}
		rv = new Value(res);
	}

	if(debugf > 0) {
		ff_log << "KC========\n";
		FFpem.kc.print(ff_log);
		ff_log << "KC========\n";
	}

#endif

	if(debugf > 0) {
		if(rv) {
			ff_log << "value = ";
			rv->print(ff_log);
			ff_log << '\n';
		} else {
			ff_log << "rv == NULL\n";
		}
	}

	if(debugf > 0) {
		ff_log << "KCNode::eval() <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n";
	}

	return rv ? rv : new Value(ERROR_TYPE);
}

Value* 
ResultNode::eval() {
	if(debugf > 0) {
		ff_log << "ResultNode::eval() >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n";
	}

	if(debugf > 0) {
		if(name_expr_) {
			ff_log << "name_expr_ defined\n";
		} else {
			ff_log << "name_expr_ == NULL\n";
		}
		if(time_expr_) {
			ff_log << "time_expr_ defined\n";
		} else {
			ff_log << "time_expr_ == NULL\n";
		}
	}

	Value *ne=NULL, *te=NULL, *rv=NULL;

	if(name_expr_) ne = name_expr_->eval();
	if(time_expr_) te = time_expr_->eval();

	if(!name_expr_ || !ne || ne->type() == ERROR_TYPE) {
		ff_log << "Failed to evaulate name passed to result() on line " << line_ << '\n';    
		return new Value(ERROR_TYPE);
	}

	if((time_expr_ && !te) || (te && te->type() == ERROR_TYPE)) {
		ff_log << "Failed to evaulate time expression passed to result() on line " << line_ << '\n';    
		return new Value(ERROR_TYPE);
	}

	if(ne->type() != STRING_TYPE) {
		ff_log << "Name passed to result() must be string type on line " << line_ << '\n';
		return new Value(ERROR_TYPE);
	}

	int tt;
	if(te) {
		double t;
		if(te->type() == INT_TYPE) {
			t = te->getIVal();
		} else if(te->type() == FLOAT_TYPE) {
			t = te->getFVal();
		} else {
			if(debugf > 0) {
				ff_log << "Time value passed to result() must be numeric type on line " << line_ << '\n';
			}
			pneWarning("SCRIPT: Non-numeric time value passed to result() on line %d.\n", line_);
			return new Value(ERROR_TYPE);
		}

		double sr = FFpem.simulation.timeStepSize();
		tt = t/sr + 1;
	} else {
		tt = FFresults.m.nrows() - 1;
	}

#ifndef STAND_ALONE

	string n = ne->getSVal();
	int ind  = FFpem.moleculeLookup(n.c_str());
	if(ind == -1) {
		if(debugf > 0) {
			ff_log << "Molecule not found\n";
		}
		pneWarning("SCRIPT: Molecule %s not found.\n", n.c_str());
		return new Value(ERROR_TYPE);
	}
	if(FFresults.m.nrows() == 0) {
		if(debugf > 0) {
			ff_log << "Results are empty\n";
		}
		pneWarning("SCRIPT: Results are empty.\n");
		return new Value(ERROR_TYPE);
	}
	if(tt < 0 || tt >= FFresults.m.nrows()) {
		if(debugf > 0) {
			ff_log << "Time out of bounds\n";
		}
		pneWarning("SCRIPT: Referenced time index %d is not in range [%d %d].\n", tt, 0, FFresults.m.nrows()-1);
		return new Value(ERROR_TYPE);
	}
	double res=FFresults.m(tt, ind);
	// calculate t by dividing by sampling rate and interpolating if needed
	// look up value in results matrix using name and t
	// check bounds
	rv = new Value(res);

#endif

	if(debugf > 0) {
		if(rv) {
			ff_log << "value = ";
			rv->print(ff_log);
			ff_log << '\n';
		} else {
			ff_log << "rv == NULL\n";
		}
	}

	if(debugf > 0) {
		ff_log << "ResultNode::eval() <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n";
	}

	return rv ? rv : new Value(ERROR_TYPE);
}

Value *SimulateNode::eval() {
	if(debugf > 0) {
		ff_log << "SimulateNode::eval() >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n";
	}

	bool rv = true;

#ifndef STAND_ALONE

	FFpem.setCurEnvironment(0);
	rv = run_simulation(FFpem, FFresults);

#endif

	if(debugf > 0) {
		ff_log << "SimulateNode::eval() <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n";
	}

	return new Value(rv);
}

Value *
MathNode::eval() {
	if(debugf > 0) {
		ff_log << "MathNode::eval() >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n";
	}

	if(debugf > 0) {
		if(expr_) {
			ff_log << "expr_ defined\n";
		} else {
			ff_log << "expr_ == NULL\n";
		}
	}

	Value *e=NULL, *rv=NULL;

	if(expr_) e = expr_->eval();

	if(!expr_ || !e || e->type() == ERROR_TYPE) {
		if(debugf > 0) {
			ff_log << "Failed to evaulate expression passed to math-op on line " << line_ << '\n';    
		}
		pneWarning("SCRIPT: Failed to evaulate expression passed to math-op on line %d.\n", line_);
		return new Value(ERROR_TYPE);
	}

	if(e->type() != INT_TYPE && e->type() != FLOAT_TYPE) {
		if(debugf > 0) {
			ff_log << "Values passed to math-op must be numeric type on line " << line_ << '\n';
		}
		pneWarning("SCRIPT: Non-numeric values passed to math-op on line %d.\n", line_);
		return new Value(ERROR_TYPE);
	}

	double t;
	if(e->type() == INT_TYPE) {
		t = e->getIVal();
	}
	if(e->type() == FLOAT_TYPE) {
		t = e->getFVal();
	}

	switch(op_) {
	case MATH_ABS:  rv = new Value(e->type() == INT_TYPE ? abs(e->getIVal()) : fabs(e->getFVal())); break;
	case MATH_ACOS: rv = new Value(acos(t)); break;
	case MATH_ASIN: rv = new Value(asin(t)); break;
	case MATH_ATAN: rv = new Value(atan(t)); break;
	case MATH_COS:  rv = new Value(cos(t));  break;
	case MATH_SIN:  rv = new Value(sin(t));  break;
	case MATH_TAN:  rv = new Value(tan(t));  break;
	case MATH_EXP:  rv = new Value(exp(t));  break;
	case MATH_LOG:  rv = new Value(log(t));  break;
	case MATH_SQRT: rv = new Value(sqrt(t)); break;
	case MATH_SQR:  rv = new Value(t*t); break;
	}

	if(debugf > 0) {
		if(rv) {
			ff_log << "value = ";
			rv->print(ff_log);
			ff_log << '\n';
		} else {
			ff_log << "rv == NULL\n";
		}
	}

	if(debugf > 0) {
		ff_log << "ResultNode::eval() <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n";
	}

	return rv ? rv : new Value(ERROR_TYPE);
}
