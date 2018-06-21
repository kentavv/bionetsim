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

#include <ostream>
#include <string>
#include <list>

using namespace std;

class ReactionNode {
public:
  ReactionNode(int line, const string *rxn_name, const list<pair<int, string> > *left_side, int rxn_dir, const list<pair<int, string> > *right_side, const pair<double, double> *rxn_rate);
    ~ReactionNode() {}

    //NodeTag nodeTag() const { return tag_; }
    //  TypeTag type()    const { return type_; }
  int line()        const { return line_; }

  void print(ostream &os, int /*indent*/ = 0) const {
    os << line_ << '\t' << rxn_name_ << '\t';
    list<pair<int, string> >::const_iterator i;
    for(i=left_side_.begin(); i!=left_side_.end(); i++) {
      os << '{' << i->first << '\t' << i->second << '}';
    }

    if(rxn_dir_ == 0) {
      os << "-->";
    } else if(rxn_dir_ == 1) {
      os << "<-->";
    }

    for(i=right_side_.begin(); i!=right_side_.end(); i++) {
      os << '{' << i->first << '\t' << i->second << '}';
    }

    os << '[' << rxn_rate_.first << ", " << rxn_rate_.second << "]\n";
  }
  //virtual Value* eval() = 0;

  string label() const { return rxn_name_; }
  double Kf() const { return rxn_rate_.first; }
  double Kr() const { return rxn_rate_.second; }
  bool reversible() const { return rxn_dir_ != 0; }

	const list<pair<int, string> > &leftSide() const { return left_side_; }
	const list<pair<int, string> > &rightSide() const { return right_side_; }

protected:
  int line_;
  string rxn_name_;
  list<pair<int, string> > left_side_;
  int rxn_dir_; 
  list<pair<int, string> > right_side_;
  pair<double, double> rxn_rate_;
};

//inline ostream& operator<<(ostream& os, const AstNode& an) {
//  an.print(os);
//  return os;
//};

#endif
