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

#ifndef _INDIVIDUAL_H_
#define _INDIVIDUAL_H_

#include "matrix.h"

class Individual {
public:
  Individual();
  Individual(const Individual &);
  Individual& operator = (const Individual &);

  void addGene();
  
  void mutate();
  double fitness();
  int toggle() { (void)fitness(); return t_; } // t_ is computed in fitness() so we call fitness() to ensure t_ is current
  bool save(const char *filename);
  bool save2(const char *filename);
  bool convertToItems(QList<PathwayCanvasItem> &items, bool includeCVs=true, bool easyCVs=true);
  int nmols() const;
  int nrxns() const;
  
  bool convertToPM();
    
  double f_;
  int t_;
  bool current_;
  Matrix rxns_;
  IMatrix mols_;
  //1=gene     gene_ind
  //2=protein  gene_ind
  //3=protein-complex  ind1  ind2
  //4=gene:protein-complex  gene-mol-ind ind1  ind2
  //5=modified-protein ind1
  Matrix y0_;
  int gene_ind_;
};

#endif
