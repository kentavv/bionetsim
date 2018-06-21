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

#include <math.h>

#include "calculate.h"
#include "fitness.h"
#include "fitness_functions.h"
#include "log.h"
#include "result.h"

#include "fitness_lang.h"

QStringList Fitness::function_names_;

static int n_names = 3;
static const char *names[] = {
  "Null",
  "Script",
  "Generic"
};

class FitnessStaticLoader {
public:
  FitnessStaticLoader();
};

FitnessStaticLoader::FitnessStaticLoader() {
  for(int i=0; i<n_names; i++) {
    Fitness::function_names_.push_back(names[i]);
  }
}

static FitnessStaticLoader fsl_;

Fitness::Fitness()
  : ff_(0)
{}

Fitness::Fitness(const Fitness &f)
  : ff_(f.ff_),
    molecules_(f.molecules_),
    targets_(f.targets_),
    script_(f.script_)
{}

Fitness::~Fitness() {
}

Fitness &
Fitness::operator=(const Fitness &f) {
  ff_ = f.ff_;
  molecules_ = f.molecules_;
  targets_ = f.targets_;
  script_ = f.script_;

  return *this;
}

void
Fitness::clear() {
  ff_ = 0;
  molecules_.clear();
  targets_.clear();
  script_ = "";
}

QStringList
Fitness::functionNames() {
  return function_names_;
}

void
Fitness::setFunction(const QString &name) {
  ff_ = function_names_.findIndex(name);
  if(ff_ == -1) ff_ = 0;
}

QString
Fitness::function() const {
  return function_names_[ff_];
}

void
Fitness::setNumEnvironment(int ne) {
  if(targets_.ncolumns() != ne) {
    Matrix nt(targets_.nrows(), ne);
    int nr = targets_.nrows();
    int nc = targets_.ncolumns() < ne ? targets_.ncolumns() : ne;
    for(int i=0; i<nr; i++) {
      for(int j=0; j<nc; j++) {
	nt(i, j) = targets_(i, j);
      }
    }
    targets_ = nt;
  }
}

int
Fitness::numEnvironment() const {
  return targets_.ncolumns();
}

void
Fitness::setGeneric(const QStringList &mol, const Matrix &mat) {
  molecules_ = mol;
  targets_ = mat;
}

void
Fitness::genericTarget(QStringList &mol, Matrix &mat) const {
  mol = molecules_;
  mat = targets_;
}

void
Fitness::setScript(const QString &s) {
  script_ = s;
}

QString 
Fitness::script() const {
  return script_;
}

bool
Fitness::calculate(PathwayMatrix &pem, double &fitness) {
  Matrix mat;
  return calculate(pem, fitness, mat);
}
  
bool
Fitness::calculate(PathwayMatrix &pem, double &fitness, Matrix &mat) {
  double f = 0.0;
  if(ff_ == 0) {
  } else if(ff_ == 1) {
    // Script
    if(!fitness_lang_exec(script_, pem, f, mat)) {
      pneDebug("script execution failed\n");
      return false;
    }
  } else if(ff_ == 2) {
		int i, j;
		int ne = targets_.ncolumns();
		int nm = targets_.nrows();
		if(pem.simulation.numEnvironment() != ne) {
			pneDebug("pem.simulation.numEnvironment() != ne");
			return false;
		}

		IMatrix inds(nm, 1);
		for(i=0; i<nm; i++) {
			inds(i, 0) = pem.moleculeLookup(molecules_[i]);
			if(inds(i, 0) == -1) {
				pneDebug(QString("Unable to locate molecule ") + molecules_[i]);
				return false;
			}
		}

		mat.resize(nm, ne);
		int oe = pem.curEnvironment();
		Result r;
		for(i=0; i<ne; i++) {
			pem.setCurEnvironment(i);
			if(!run_simulation(pem, r)) {
				pem.setCurEnvironment(oe);
				return false;
			}
			for(j=0; j<nm; j++) {
				mat(j, i) = r.m(r.m.nrows()-1, inds(j, 0));
			}
		}
		pem.setCurEnvironment(oe);    

		pneDebug("Final results:");
		for(i=0; i<nm; i++) {
			QString str, str2;
			for(j=0; j<ne; j++) {
				if(j > 0) str += "\t";
				str2.sprintf("%g", mat(i, j));
				str += str2;
			}
			pneDebug("%s", (const char*)str);
		}

		pneDebug("");

		pneDebug("Distance from target (results - targets):");
		for(i=0; i<nm; i++) {
			QString str, str2;
			for(j=0; j<ne; j++) {
				if(j > 0) str += "\t";
				str2.sprintf("%g", mat(i, j) - targets_(i, j));
				str += str2;
			}
			pneDebug("%s", (const char*)str);
		}

		pneDebug("");

		for(i=0; i<nm; i++) {
			for(j=0; j<ne; j++) {
				double d = mat(i, j) - targets_(i, j);
				f += d * d;
			}
		}
		f = sqrt(f);
  }
  fitness = f;
  return true;
}

void
Fitness::save(QDomDocument &doc, QDomElement &n) const {
  QDomElement e, e2, e3;
  QDomText t;

  QDomElement fit = doc.createElement("Fitness");
  n.appendChild(fit);

  e = doc.createElement("Function");
  t = doc.createTextNode(function());
  fit.appendChild(e); e.appendChild(t);

  if(!script_.isEmpty()) {
    e = doc.createElement("Script");
    t = doc.createTextNode(script());
    fit.appendChild(e); e.appendChild(t);
  }

  if(!targets_.empty()) {
    e = doc.createElement("Generic");
    fit.appendChild(e);
    
    int ne = targets_.ncolumns();
    int nm = targets_.nrows();
    for(int i=0; i<nm; i++) {
      e2 = doc.createElement("Target");
      e.appendChild(e2);

      e3 = doc.createElement("Molecule");
      t = doc.createTextNode(molecules_[i]);
      e2.appendChild(e3); e3.appendChild(t);

      e3 = doc.createElement("Values");
      QString str, str2;
      for(int j=0; j<ne; j++) {
	if(j > 0) {
	  str2.sprintf("\t%g", targets_(i, j));
	  str += str2;
	} else {
	  str2.sprintf("%g", targets_(i, j));
	  str += str2;
	}
      }
      t = doc.createTextNode(str);
      e2.appendChild(e3); e3.appendChild(t);
    }
  }
}

QString
Fitness::genReport() const {
  QString text;

  return text;
}
