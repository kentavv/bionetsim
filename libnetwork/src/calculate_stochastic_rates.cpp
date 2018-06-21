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

#include "calculate_stochastic_rates.h"
#include "math_utils.h"
#include "matrix.h"
#include "pathway_matrix.h"

int 
rates(const PathwayMatrix &pem, const IMatrix &state, double t, Matrix &react_rates, double e) {
	const int nrxn = pem.kc.nrows();  
	const int nc = pem.cvs.count();
	for(int r=0; r<nrxn; r++) {
		const int nm = pem.ire(0, r);
		//int order = 0;
		react_rates(r, 0) = pem.kc(r, 0);
		for(int i=1; i<=nm; i++) {
			const int i2 = pem.ire(i, r);
			//order += i2;
			const int v = (i2 < nc) ? GenoDYN::roundToInt(pem.cvs[i2]->concentration(t)) : state(i2, 0);
			const int coeff = pem.re(i2, r);
			for(int k=0; k<coeff; k++) {
				react_rates(r, 0) *= v - k;
			}
		}
		//react_rates(r, 0) /= pow(v, order-1);    

		if(react_rates(r, 0) < e) react_rates(r, 0) = 0.0;
	}

	return 1;
}
