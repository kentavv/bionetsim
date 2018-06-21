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

#ifndef _PATHWAY_MATRIX_H_
#define _PATHWAY_MATRIX_H_

#include <stdio.h>

#include <qlist.h>
#include <qprogressbar.h>
#include <qstringlist.h>

#include "editor.h"
#include "matrix.h"
#include "pathway_canvas_items.h"
#include "simulation.h"

class PathwayMatrix {
public:
  PathwayMatrix();
  PathwayMatrix(const PathwayMatrix &);
  ~PathwayMatrix();
  
  PathwayMatrix &operator=(const PathwayMatrix &);
  
  void setup(const Editor &, const Simulation &, const QList<PathwayCanvasItem> &, QProgressBar *progress = NULL);
  void print(FILE *fp=stdout) const;

  int moleculeLookup(const QString &) const;
  int moleculeLookupAll(const QString &) const;
  QStringList moleculeNames() const;
  QStringList plottedMoleculeNames() const;
  int nPlotted() const;
  bool plotted(int) const;
  
  void setCurEnvironment(int);
  int curEnvironment() const;

  // For the evolve programs
  void setupSparseMatrices();
  void setupPlottedMoleculesList(QList<MoleculeItem> &);

  void applyKCToItems();

  Editor editor;
  Simulation simulation;
  
  Matrix kc;
  IMatrix pro;
  IMatrix re;
  IMatrix ipro;  // Indexed version of pro
  IMatrix ire;   // Indexed version of re
  IMatrix isto;  // A simplified version of ipro and ire for calculate.cpp
  IMatrix jmat;
  QList<PathwayCanvasItem> items;
  QArray<ControlVariableItem*> cvs;
  
  Matrix y0;
  Matrix atol;

  int reactionLookup(const QString &) const;
  QStringList rxn_names;
  QStringList rxn_equations;

  QProgressBar *progress;
  bool quit;

  BMatrix plotted_;

protected:
  static void initializeMoleculeIds(const QList<PathwayCanvasItem> &);
  static int countControlVariables(const QList<PathwayCanvasItem> &);
  static void buildControlVariableArray(const QList<PathwayCanvasItem> &,
					QArray<ControlVariableItem*> &,
					QList<MoleculeItem> &,
					QStringList &);

  static void buildControlVariableArray(const QList<PathwayCanvasItem> &,
					const QString &,
					int &,
					QArray<ControlVariableItem*> &,
					QList<MoleculeItem> &,
					QStringList &);
  static void buildMoleculeList(const QList<PathwayCanvasItem> &,
				QList<MoleculeItem> &,
				QStringList &);
  static void buildMoleculeList(const QList<PathwayCanvasItem> &,
				const QString &,
				int &,
				QList<MoleculeItem> &,
				QStringList &);
  static void buildReactionList(const QList<PathwayCanvasItem> &,
				QList<ReactionItem> &,
				QStringList &,
				QStringList &);
  static void buildReactionList(const QList<PathwayCanvasItem> &,
				const QString &,
				int &,
				QList<ReactionItem> &,
				QStringList &,
				QStringList &);

  void applyKCToItems(const QList<PathwayCanvasItem> &, int &);

private:
  QStringList mol_names_, plotted_mol_names_;
};

#endif
