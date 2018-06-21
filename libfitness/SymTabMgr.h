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

#ifndef SYM_TAB_MGR_H
#define SYM_TAB_MGR_H

#include <list>

#include "SymTab.h"

using namespace std;

extern int debugf;

class ClassEntry;
class MethodEntry;

class SymTabMgr {
public:
  SymTabMgr(SymTab *globalScope) { 
    scopeStack_.push_back(globalScope);
  }

  virtual ~SymTabMgr() {}

  virtual SymTabEntry* lookUp(const string &name) {
    // Lookup name in current and preceeding scopes up and including
    // the last formals list
    list<SymTab*>::reverse_iterator i;
    for(i=scopeStack_.rbegin(); i!=scopeStack_.rend(); i++) {
      SymTabEntry *se = (*i)->lookUp(name);
      if(se) return se;
      if((*i)->tag() == FUNC_FORMALS) break;
    }
    // Finally check the global scope
    return globalScope()->lookUp(name);
  }

  virtual SymTabErr enter(SymTabEntry *se) {
    if(debugf > 0) {
      cout << "enterEntry\n";
    }
    return currentScope()->enter(se);
  }

  virtual SymTabErr enterScope(SymTabTag tag) { 
    if(debugf > 0) {
      cout << "SymTabMgr::enterScope(";
      switch(tag) {
      case GLOBAL:       cout << "global";           break;
      case FUNC_FORMALS: cout << "formals";          break;
      case BLOCK:        cout << "block";            break;
      default:           cout << "tag=" << (int)tag; break;
      }
      cout << ")\n";
    }
    scopeStack_.push_back(new SymTab(tag));
    return OK;
  }

  virtual SymTabErr leaveScope() { 
    if(debugf > 0) {
      cout << "leaveScope\n";
    }
    scopeStack_.pop_back();
    return OK;
  }

  SymTab* currentScope() { return scopeStack_.back(); }

  SymTab* globalScope() { return scopeStack_.front(); }

  void print(ostream &os) {
    int indent;
    list<SymTab*>::const_iterator i;
    for(indent=0, i=scopeStack_.begin(); i!=scopeStack_.end(); i++, indent+=2) {
      (*i)->print(os, indent);
    }
  }

private:
  const SymTabMgr& operator=(const SymTabMgr&);
  SymTabMgr(const SymTabMgr&);

  list<SymTab*> scopeStack_;
};

#endif
