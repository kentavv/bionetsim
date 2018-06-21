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

#ifdef GUI
#include <qapplication.h>
#endif

#include "calculate.h"
#include "calculate_cvode.h"
#include "calculate_gillespie_direct.h"

const int n_simulationMethodNames = 2;
const char * const simulationMethodNames[2] = {
  "ODE",
  "Stochastic"
};

bool
run_simulation(PathwayMatrix &pem, Result &result) {
  int i, j;
  int ncvs = pem.cvs.count();
  for(i=0; i<ncvs; i++) {
    pem.cvs[i]->init();
  }

  result.column_labels = pem.plottedMoleculeNames();

  result.cont.resize(result.column_labels.count(), false);
  for(int k=0; k<ncvs; k++) {
    result.cont[k] = true;
  }

  result.env = pem.simulation.environmentLabel(pem.curEnvironment());

  //pem.setCurExperiment(i);
  //result.exp = pem.simulation.experimentLabel(i);

  int flag = -1;
  switch(pem.simulation.simulationMethod()) {
  case 0:
		flag = calculate_results_cvode(pem, result.m, result.jacobian);
		break;
	case 1:
		if(pem.simulation.singleTrajectory()) {
			flag = calculate_results_gillespie_direct(pem, result.m);
		} else {
#ifdef GUI
			if(pem.progress) {
				pem.progress->setTotalSteps(pem.simulation.numReplicates());
			}
#endif
			for(int k=0; k<pem.simulation.numReplicates(); k++) {
				flag = calculate_results_gillespie_direct(pem, result.m);
				//result.m.print();
				result.tel.append(new Matrix(result.m));
				if(flag != 1) break;
				if(pem.quit) break;
#ifdef GUI
				if(pem.progress) {
					pem.progress->setProgress(k+1);
					qApp->processEvents(100);
				}
#endif
			}
#ifdef GUI
			if(pem.progress) {
				pem.progress->reset();
			}
#endif
		}
		break;
	}

  result.success = (flag == 1);

  return result.success;
}

bool
run_simulation(PathwayMatrix &pem, QValueList<Result> &results) {
  int i, j;
  int ncvs = pem.cvs.count();
  for(i=0; i<ncvs; i++) {
    pem.cvs[i]->init();
  }

  results.clear();

  for(i=0; i<pem.simulation.numEnvironment(); i++) {
    //for(j=0; j<pem.simulation.numExperiments(); j++) {
      pem.setCurEnvironment(i);
      //pem.setCurExperiment(i);

      Result r;
      run_simulation(pem, r);
      results.append(r);
      if(pem.quit) break;
      //}
  }

  return true;
}
