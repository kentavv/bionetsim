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

#ifndef _FITNESS_H_
#define _FITNESS_H_

#include <qdom.h>
#include <qstringlist.h>

#include "matrix.h"
#include "pathway_matrix.h"

class FitnessStaticLoader;

class Fitness {
public:
  Fitness();
  Fitness(const Fitness &);
  ~Fitness();

  Fitness &operator=(const Fitness &);

  void clear();
  
  void save(QDomDocument &doc, QDomElement &n) const;
  
  static QStringList functionNames();
  
  void setFunction(const QString &);
  QString function() const;
  
  void setNumEnvironment(int);
  int numEnvironment() const;
  
  void setGeneric(const QStringList &, const Matrix &);
  void genericTarget(QStringList &names, Matrix &mat) const;

  void setScript(const QString &);
  QString script() const;

  //! The environment of PathwayMatrix is altered during calculate but returned to original value afterwards
  bool calculate(PathwayMatrix &, double &);
  bool calculate(PathwayMatrix &, double &, Matrix &);

  QString genReport() const;
  
protected:
  int ff_;
  QStringList molecules_;
  Matrix targets_;
  QString script_;
  static QStringList function_names_;

  friend class FitnessStaticLoader;
};

#endif
