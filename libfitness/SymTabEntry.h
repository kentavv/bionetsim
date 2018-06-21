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

#ifndef SYM_TAB_ENTRY_H
#define SYM_TAB_ENTRY_H

#include <iostream>

using namespace std;

class SymTab;
class Type;

/* pretty-printing related constants and functions */

const int STEP_INDENT = 2;
inline void prtSpace(ostream& os, int n) {
  for (int i = 0; i < n; i++)
	os << ' ';
}

/* Auxiliary type definitions */

enum Kind {UNKNOWN_KIND, VAR, UNNAMED_BLOCK, GLOBAL_FUN};
enum ModifierFlag {UNKNOWN_MOD = 0x0, PRIVATE = 0x1, PUBLIC = 0x2, 
                   STATIC = 0x4, FINAL = 0x8};

typedef int Modifier; 
/* stored as a bitmask, obtained by or'ing all applicable modifiers */

/* The base class */

// Arity = the number of arguments that a function takes

class SymTabEntry {
public:
  SymTabEntry(const string &name, int /*size*/, int arity,  Modifier mFlags, Kind kind)
    : name_(name),
      arity_(arity),
      mflags_(mFlags),
      kind_(kind) {
  }
  virtual ~SymTabEntry() {}

  int operator==(const SymTabEntry&);

  // Read-Accessors
  int arity()       const { return arity_; }
  string name()     const { return name_; }
  Kind kind()       const { return kind_; }
  Modifier mflags() const { return mflags_; }
  const SymTab* symTab() const { return symTab_; }
  const SymTabEntry* children() const { return next_; }
  /* pointer to an array containing the SymTabEntr'ies in symTab_ member */

  // Write-Accessors
  void setName(const string &v) { name_ = v; }
  void kind(Kind v)             { kind_ = v; }
  void arity(int v)             { arity_ = v; }
  void mflags(Modifier v)       { mflags_ = v; }
  SymTab* symTab()              { return symTab_; }
  void symTab(SymTab*v)         { symTab_ = v; }

  // Output
  virtual void print(ostream& /*os*/, int /*indent*/ = 0) const {}

  // TypeCheck all subcomponents
  virtual void typeCheck() {}; /* Leave empty for the base class */

  SymTabEntry* next() const { return next_; }
  void next(SymTabEntry *v) { next_ = v; }

private:
  const SymTabEntry& operator=(const SymTabEntry&);
  SymTabEntry(const SymTabEntry&);

private:
  string name_;
  int arity_;
  Kind kind_;
  Modifier mflags_;
  SymTab * symTab_;
  SymTabEntry *next_;
};

/* Global operator defined using the base class. Need not be redefined
   for subclasses, since this function is implemented using the virtual
   function "print" */

inline ostream& operator<<(ostream& os, SymTabEntry& ste) {
  ste.print(os);
  return os;
};

#endif
