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

#ifndef SYMTAB_H
#define SYMTAB_H

#include <iostream>
#include <list>

#include "SymTabEntry.h"
#include "misc.h"

using namespace std;

enum SymTabTag { GLOBAL, FUNC_FORMALS, BLOCK };

enum SymTabErr {OK, SYM_NOT_FOUND_OR_INACCESSIBLE, DUPLICATE_SYM, 
                INVALID_SCOPE};
class SymTab {
public:
  SymTab(SymTabTag tag) : tag_(tag) {}
  virtual ~SymTab() {}

  void clear() {
    tab_.clear();
  }

  SymTabTag tag() const { return tag_; }

  virtual SymTabEntry* lookUp(const string &name) const {
    list<SymTabEntry*>::const_iterator i;
    for(i=tab_.begin(); i!=tab_.end(); i++) {
      if((*i)->name() == name) return *i;
    }
    return NULL;
  }
  // The above functions look up the entry in this symbol table object
  // corresponding to name and kind. A NULL pointer is returned if the 
  // entry is not found.

  virtual SymTabErr enter(SymTabEntry *se) {
    if(lookUp(se->name())) return DUPLICATE_SYM;
    tab_.push_back(se);
    return OK;
  }
  // Enters a pointer to "se" into this symbol table. Returns OK if the 
  // entry was successfully inserted. If a duplicate entry already exists in
  // the table, DUPLICATE_SYM is returned.

  virtual void print(ostream& os, int indent = 0) const {
    print_ws(os, indent);
    switch(tag_) {
    case GLOBAL:       os << "global\n";  break;
    case FUNC_FORMALS: os << "formals\n"; break;
    case BLOCK:        os << "block\n";   break;
    }
    list<SymTabEntry*>::const_iterator i;
    for(i=tab_.begin(); i!=tab_.end(); i++) {
      (*i)->print(os, indent);
    }
  }

private:
  SymTabTag tag_;
  list<SymTabEntry*> tab_;
  
  SymTab(const SymTab&);
  const SymTab& operator=(const SymTab&);
  int operator==(const SymTab&) const;
};

#endif
