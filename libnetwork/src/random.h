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

#ifndef _RANDOM_H_
#define _RANDOM_H_

class Random {
public:
  Random(long seed = -1);
  Random(const Random &);
  ~Random();
  Random &operator = (const Random &);
  
  long seed();
  void setSeed(long seed = -1);

  // Generate a random value from the exponential distribution
  // with an average of av
  double exp(double av = 1.0);

  
  // Generate a random value from the Poisson distribution
  // with an average of av
  double poisson(double av = 1.0);


  // Generate a random value from the normal distribution
  // with an average of av and variance of sd
  double normal(double av = 0.0, double sd = 1.0);

  // Generate a random value from the uniform distribution
  // in the range [min, max]
  int integer(int min, int max);

  double real(double min, double max);
  
  /* generates a random Boolean value */ 
  bool boolean();
  
  /* generates a random number on [0,0xffffffff]-interval */
  unsigned long int32();
  
  /* generates a random number on [0,0x7fffffff]-interval */
  long int31();
  
  /* generates a random number on [0,1]-real-interval */
  double real1();
  
  /* generates a random number on [0,1)-real-interval */
  double real2();
  
  /* generates a random number on (0,1)-real-interval */
  double real3();
  
  /* generates a random number on [0,1) with 53-bit resolution*/
  double res53();

private:
  long seed_;

  int mt_n_;          // for mt19937
  unsigned long *mt_; // for mt19937
  int mti_;           // for mt19937

  double sq_, alxm_, g_, oldm_; // for poisson()

  int iset_;    // for normal()
  double gset_; // for normal()

  static long random_seed();

  void init_genrand(unsigned long s);
};

#endif
