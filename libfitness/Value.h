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

#ifndef VALUE_H
#define VALUE_H

#include <iostream>
#include <string>
#include <vector>

#include <string.h>
#include <stdlib.h>

using namespace std;

enum TypeTag { ERROR_TYPE,  VOID_TYPE,
	       INT_TYPE,    INT_MATRIX_TYPE,
	       FLOAT_TYPE,  FLOAT_MATRIX_TYPE,
	       STRING_TYPE, STRING_MATRIX_TYPE,
	       BOOL_TYPE,   BOOL_MATRIX_TYPE,
	       LOOP_CONTROL_BREAK, LOOP_CONTROL_CONTINUE, UNKNOWN_TYPE};

enum LoopControl { LOOP_BREAK, LOOP_CONTINUE };

inline string
typeStr(TypeTag t) {
  string str = "(unknonw type id)";

  switch(t) {
  case ERROR_TYPE:            str = "(error)";    break;
  case INT_TYPE:              str = "(int)";      break;
  case INT_MATRIX_TYPE:       str = "(int[])";    break;
  case FLOAT_TYPE:            str = "(float)";    break;
  case FLOAT_MATRIX_TYPE:     str = "(float[])";  break;
  case STRING_TYPE:           str = "(string)";   break;
  case STRING_MATRIX_TYPE:    str = "(string[])"; break;
  case BOOL_TYPE:             str = "(bool)";     break;
  case BOOL_MATRIX_TYPE:      str = "(bool[])";   break;
  case VOID_TYPE:             str = "(void)";     break;
  case LOOP_CONTROL_BREAK:    str = "(break)";    break;
  case LOOP_CONTROL_CONTINUE: str = "(continue)"; break;
  case UNKNOWN_TYPE:          str = "(unknown)";  break;
  }

  return str;
}

class Value {
public:
  Value(TypeTag t=ERROR_TYPE, std::vector<int> *subscripts=NULL) 
    : subscripts_(subscripts),
      type_(t),
      mat_(NULL) { 
			if(type_ == ERROR_TYPE) {
			}
    if(type_ == INT_MATRIX_TYPE ||
       type_ == FLOAT_MATRIX_TYPE ||
       type_ == STRING_MATRIX_TYPE ||
       type_ == BOOL_MATRIX_TYPE) {
      if(numSubscripts() == 0) {
      } else {
	int n = numElements();
	if(n <= 0) {
	} else {
	  switch(type_) {
	  case INT_MATRIX_TYPE:
	    mat_ = new Value[n];
	    for(int i=0; i<n; i++) {
	      mat_[i].set(0);
	    }
	    break;
	  case FLOAT_MATRIX_TYPE:
	    mat_ = new Value[n];
	    for(int i=0; i<n; i++) {
	      mat_[i].set(0.0f);
	    }
	    break;
	  case STRING_MATRIX_TYPE:
	    mat_ = new Value[n];
	    for(int i=0; i<n; i++) {
	      mat_[i].set(string(""));
	    }
	    break;
	  case BOOL_MATRIX_TYPE:
	    mat_ = new Value[n];
	    for(int i=0; i<n; i++) {
	      mat_[i].set(false);
	    }
	    break;
	  }
	}
      }
    } else {
      if(numSubscripts() > 0) {
      } else {
	switch(type_) {
	case INT_TYPE:    set(0);          break;
	case FLOAT_TYPE:  set(0.0f);       break;
	case STRING_TYPE: set(string("")); break;
	case BOOL_TYPE:   set(false);      break;
	}
      }
    }
  }

  Value(int i)             : subscripts_(NULL), mat_(NULL) { set(i); }
  Value(double f)          : subscripts_(NULL), mat_(NULL) { set(f); }
  Value(const string &str) : subscripts_(NULL), mat_(NULL) { set(str); }
  Value(bool b)            : subscripts_(NULL), mat_(NULL) { set(b); }
  Value(LoopControl c, int d) { setLoopControl(c, d); }
  ~Value() {}

  TypeTag type()    const { return type_; }
  int    getIVal()  const { return iVal_; }
  double getFVal()  const { return fVal_; }
  string getSVal()  const { return sVal_; }
  bool   getBVal()  const { return bVal_; }
  int    getDepth() const { return depth_; }

  Value *index(int ind) { return &mat_[ind]; }

  int numSubscripts() const { return !subscripts_ ? 0 : (*subscripts_).size(); }

  void set(int v) {
    type_ = INT_TYPE;
    if(mat_) delete[] mat_;
    mat_ = NULL;
    iVal_ = v;
  }
  void set(double v) {
    type_ = FLOAT_TYPE;
    if(mat_) delete[] mat_;
    mat_ = NULL;
    fVal_ = v;
  }
  void set(bool v) {
    type_ = BOOL_TYPE;
    if(mat_) delete[] mat_;
    mat_ = NULL;
    bVal_ = v;
  }
  void set(const string &v) {
    type_ = STRING_TYPE;
    if(mat_) delete[] mat_;
    mat_ = NULL;
    sVal_ = v;
  }
  void setLoopControl(LoopControl c, int d) { 
    switch(c) {
    case LOOP_BREAK:    type_ = LOOP_CONTROL_BREAK;    break;
    case LOOP_CONTINUE: type_ = LOOP_CONTROL_CONTINUE; break;
    }
    depth_ = d;
  }
  void decLoopDepth() { depth_--; }

  void print(ostream& os) const {
    switch(type_) {
    case ERROR_TYPE:         os << "(error)";            break;
    case UNKNOWN_TYPE:       os << "(unknown)";          break;
    case INT_TYPE:           os << "(int) "    << iVal_; break;
    case INT_MATRIX_TYPE:    os << "(int[]) ";           break;
    case FLOAT_TYPE:         os << "(float) "  << fVal_; break;
    case FLOAT_MATRIX_TYPE:  os << "(float[]) ";         break;
    case STRING_TYPE:        os << "(string) " << sVal_; break;
    case STRING_MATRIX_TYPE: os << "(string[]) ";        break;
    case BOOL_TYPE:          os << "(bool) "   << bVal_; break;
    case BOOL_MATRIX_TYPE:   os << "(bool[]) ";                   break;
    case LOOP_CONTROL_BREAK: os << "(loop control break) "    << depth_; break;
    case LOOP_CONTROL_CONTINUE: os << "(loop control continue) " << depth_; break;
    }
  }

  vector<int> subscripts() const { return *subscripts_; }

private:
  std::vector<int> *subscripts_;
  /* data members. Use a union to store the value. */
  TypeTag type_;
  union {
    int iVal_;
    double fVal_;
    bool bVal_;
    int depth_;
  };
  string sVal_;
  Value *mat_;

  int numElements() const {
    int n=1;
    for(int i=0; i<numSubscripts(); i++) {
      n *= (*subscripts_)[i];
    }
    return n;
  }

  const Value& operator=(const Value& v);
  // Assume that the type of this object and v match -- typeCheck phase 
  // would have ensured this. Simply copy the type_ and appropriate Val_
  // field from the source to the target.
  bool operator==(const Value&) const;

  Value(const Value&); // Disable
};

inline ostream& operator<<(ostream& os, Value* v) {
  v->print(os);
  return os;
};

#endif /* VALUE_H */
