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

#ifndef STE_CLASSES_H
#define STE_CLASSES_H

#include <vector>

#include "SymTabEntry.h"
#include "Value.h"
#include "misc.h"

class MethodDefNode;
class BlockNode;
class SeqNode;

extern int debugf;

/* Subclasses of SymTabEntry */

class BlockEntry: public SymTabEntry {
public:
  BlockEntry(int line) : SymTabEntry("", 0, 0, 0, UNNAMED_BLOCK), line_(line) {}
  ~BlockEntry() {}

  int line() const { return line_;}
  void print(ostream& os, int indent = 0) const { 
    print_ws(os, indent);
    os << "block{\n";
  }
  virtual void typeCheck() {}

private:
  int line_;
};

class VarEntry: public SymTabEntry {
public:
  VarEntry(const string &name, Modifier mFlags, TypeTag type,
           std::vector<int> *subscripts)
    : SymTabEntry(name, 0, 0, mFlags, VAR),
      type_(type)
  {
    val_ = new Value(type, subscripts);
  }
  ~VarEntry() {}

  TypeTag type() const { return type_; }

  void print(ostream& os, int indent = 0) const {
    print_ws(os, indent);
    os << "var " << name() << '\n';
  }
  virtual void typeCheck() {}

  Value *value() { return val_; }

private:
  TypeTag type_;

  Value *val_;
  VarEntry(const VarEntry&);
};

class MethodEntry: public SymTabEntry {
public:
  MethodEntry(const string &name, Modifier mFlags, TypeTag type)
    : SymTabEntry(name, 0, 0, mFlags, GLOBAL_FUN),
      method_(NULL),
      formals_(NULL),
      type_(type) {}
  ~MethodEntry(){}

  // Accessors

  TypeTag type() const { return returnType_; }
  const VarEntry** formals() const { return (const VarEntry**) formals_; }

  // Modifiers

  MethodDefNode* method() { return method_; }
  void method(MethodDefNode* m) { method_ = m; }
  void formals(VarEntry**v) { formals_ = v; }

  void print(ostream& os, int indent = 0) const { 
    print_ws(os, indent);
    os << "func " << name() << '\n'; 
  }
  virtual void typeCheck() {}

private:
  TypeTag returnType_;
  MethodDefNode *method_;
  VarEntry **formals_;
  TypeTag type_;
};

#endif /* STE_CLASSES_H */
