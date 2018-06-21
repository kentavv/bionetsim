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

#include <stdio.h>

#ifdef GUI
#include <qapplication.h>
#endif

#include "calculate_gillespie_direct.h"
#include "calculate_stochastic_rates.h"
#include "log.h"
#include "math_utils.h"
#include "matrix.h"
#include "pathway_matrix.h"
#include "random.h"

static Random rng;

int
calculate_results_gillespie_direct(const PathwayMatrix &pem, Matrix &results) {  
	const double t0 = pem.simulation.timeBegin();
	double t = t0;
	const double tend = pem.simulation.timeEnd();
	const double sr = pem.simulation.timeStepSize();

	int i, j, m, k;

	const int ncvs = pem.cvs.count();
	const int nmol = pem.pro.nrows();
	const int nrxn = pem.pro.ncolumns();
	IMatrix state(nmol, 1);
	for(i=0; i<nmol; i++) {
		state(i, 0) = GenoDYN::roundToInt(pem.y0(i, 0));
	}

	const int nsteps = GenoDYN::roundToInt((tend - t0) / sr);
	results.resize(nsteps+1, pem.nPlotted()+1);

#if 0
#ifdef GUI
	if(pem.progress) {
		pem.progress->setTotalSteps(100);
	}
#endif
#endif

	results(0, 0) = t0;
	for(m=0, k=1; m<ncvs; m++) {
		if(pem.plotted(m)) {
			results(0, k) = pem.cvs[m]->concentration(t0);
			k++;
		}
	}
	for(m=ncvs; m<nmol; m++) {
		if(pem.plotted(m)) {
			results(0, k) = pem.y0(m, 0);
			k++;
		}
	}

	int ri = 1;
	const double e = 1e-8;

	Matrix react_rates(nrxn, 1);
	Matrix react_cdf(nrxn + 1, 1);

	unsigned long nl = 0;

	while(t <= tend){
		//I Compute the rates
		rates(pem, state, t, react_rates, e);

		double intensity = 0.0;
		for(i=0; i<nrxn; i++){
			intensity += react_rates(i, 0);
		}
		if(intensity == 0.0) {
			pneWarning("Reaching an absorbing state at t = %f\n", t);
			if(ncvs == 0) {
				t = tend + 1.0;
			} else {
				// If CVs are present continue sampling in case CV state value is 0
				// or the value is otherwise temporarily zero
				t += sr; // Could actually jump to t when CV state changes... would be much faster
			}
		} else {
			//II Waiting time before the next jump
			t += rng.exp(1.0 / intensity);
		}

		// Update results and GUI
		if(t >= ri*sr) {
			int tmp = GenoDYN::roundToInt(t / sr);
			int eri = nsteps < tmp ? nsteps : tmp;
			for(; ri<=eri; ri++) {
				results(ri, 0) = ri*sr;
				for(m=0, k=1; m<ncvs; m++) {
					if(pem.plotted(m)) {
						results(ri, k) = pem.cvs[m]->concentration(ri*sr);
						k++;
					}
				}
				for(m=ncvs; m<nmol; m++) {
					if(pem.plotted(m)) {
						results(ri, k) = state(m, 0);
						k++;
					}
				}
			}
			if(ri >= nsteps+1) break; 
#if 0
#ifdef GUI
			int prog = int(ri/(double)nsteps*100.0 + 0.5);
			if(pem.progress && prog > pem.progress->progress()) {
				pem.progress->setProgress(prog);
				qApp->processEvents(100);
			}
#endif
#endif
			if(pem.quit) break;
		}

		if(t > tend) break;

		// Normally this condition should only be true when ncvs > 0
		if(intensity == 0.0) continue;

		//III Next reaction
		double sum = 0.0;
		const double u_random = rng.real1();
		int next_react = -1;
		react_cdf(0, 0) = 0.0;
		for(i=1; i<nrxn+1; i++) {
			sum += react_rates(i - 1, 0) / intensity;
			react_cdf(i, 0) = sum;
		}
		for(i=0; i<nrxn; i++) {
			if(react_cdf(i, 0)-e/2.0 <= u_random && u_random <= react_cdf(i+1, 0)+e/2.0) {
				next_react = i;
				break;
			}
		}

		if(next_react == -1) {
			pneWarning("next_react == -1\n");
			pneWarning("u_random: %f\n", u_random);
			pneWarning("Intensity: %f\n", intensity);
			pneWarning("react_rates:\n");
			react_rates.print();
			pneWarning("react_cdf:\n");
			react_cdf.print();
			for(i=0; i<nrxn; i++) {
				printf("%03d: %f %f %f   %g %g %g      %f %f %f   %g %g %g\n",
					i,
					react_cdf(i, 0), u_random, react_cdf(i, 0) - u_random,
					react_cdf(i, 0), u_random, react_cdf(i, 0) - u_random,
					react_cdf(i+1, 0), u_random, react_cdf(i+1, 0) - u_random,
					react_cdf(i+1, 0), u_random, react_cdf(i+1, 0) - u_random);
			}

			if(u_random < 0.5) {
				for(i=0; i<nrxn; i++) {
					if(react_rates(i, 0) > 0) {
						next_react = i;
						break;
					}
				}
			} else {
				for(i=nrxn-1; i>=0; i--) {
					if(react_rates(i, 0) > 0) {
						next_react = i;
						break;
					}
				}
			}
			pneWarning("next_react == -1 avoided by using reaction %d\n", next_react);
		}

		// This should never happen but for safety we check    
		if(next_react == -1) return -1;

		//IV Update the state
		const int nm = pem.isto(0, next_react);
		for(i=1; i<=nm; i++) {
			int i2 = pem.isto(i, next_react);
			state(i2, 0) += pem.pro(i2, next_react) - pem.re(i2, next_react);
		}

#ifdef GUI
		if((nl++ % 1000) == 0) {
			qApp->processEvents(100);
		}
#endif
		if(pem.quit) break;
	}
#if 0
#ifdef GUI
	if(pem.progress) {
		pem.progress->reset();
	}
#endif
#endif
	if(pem.quit) return -1;

	return 1;
}
