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
#include <fcntl.h>
#include <math.h>

#ifndef WIN32
#include <sys/time.h>
#include <unistd.h>
#else
#include <sys/timeb.h>
#include <time.h>
#define M_PI 3.14159265358979323846
#endif

#include "nr.h"
#include "random.h"


/*------------------------------*/

Random::Random(long seed)
  : seed_(seed),
    
    mt_n_(624),
    mt_(NULL),
    mti_(0),

    sq_(0.0),
    alxm_(0.0),
    g_(0.0),
    oldm_(-1.0),

    iset_(0),
    gset_(0.0)
{
  mt_ = new unsigned long[mt_n_];
  setSeed(seed);
}

Random::Random(const Random &r)
  : seed_(r.seed_),
    
    mt_n_(r.mt_n_),
    mt_(NULL),
    mti_(r.mti_),

    sq_(r.sq_),
    alxm_(r.alxm_),
    g_(r.g_),
    oldm_(r.oldm_),

    iset_(r.iset_),
    gset_(r.gset_)
{
  mt_ = new unsigned long[mt_n_];
  for(int i=0; i<mt_n_; i++) {
    mt_[i] = r.mt_[i];
  }
}

Random::~Random() {
  if(mt_) delete[] mt_;
}

Random &
Random::operator = (const Random &r) {
  seed_ = r.seed_;
  
  mt_n_ = r.mt_n_;
  if(mt_) delete[] mt_;
  mt_ = NULL;
  mti_ = r.mti_;
  
  sq_ = r.sq_;
  alxm_ = r.alxm_;
  g_ = r.g_;
  oldm_ = r.oldm_;
  
  iset_ = r.iset_;
  gset_ = r.gset_;

  mt_ = new unsigned long[mt_n_];
  for(int i=0; i<mt_n_; i++) {
    mt_[i] = r.mt_[i];
  }

  return *this;
}
  
long
Random::seed() {
  return seed_;
}

void
Random::setSeed(long seed) {
  seed_ = seed;
  if(seed_ < 0) {
    seed_ = random_seed();
  }
  init_genrand(seed_);
}

double
Random::exp(double av) {
  return -log(real3()) * av;
}

// The following is a modified version of poidev() from Numerical Recipies
double
Random::poisson(double av) {
  double em, t, y;
  double xm = av;
  
  if(xm < 12.0) {
    if(xm != oldm_) {
      oldm_ = xm;
      g_ = exp(-xm);
    }
    em = -1;
    t = 1.0;
    do {
      ++em;
      t *= real3();
    } while(t > g_);
  } else {
    if(xm != oldm_) {
      oldm_ = xm;
      sq_ = sqrt(2.0 * xm);
      alxm_ = log(xm);
      g_ = xm * alxm_ - gammln(xm + 1.0);
    }
    do {
      do {
	y = tan(M_PI * real3());
	em = sq_ * y + xm;
      } while(em < 0.0);
      em = floor(em);
      t = 0.9 * (1.0 + y*y) * exp(em * alxm_ - gammln(em + 1.0) - g_);
    } while(real3() > t);
  }
  return em;
}


// The following is a modified version of gasdev() from Numerical Recipies
double
Random::normal(double av, double sd) {
  double fac, rsq, v1, v2;

  if(iset_ == 0) {
    do {
      v1 = 2.0 * real1() - 1.0;
      v2 = 2.0 * real1() - 1.0;
      rsq = v1*v1 + v2*v2;
    } while(rsq >= 1.0 || rsq == 0.0);
    fac = sqrt(-2.0 * log(rsq) / rsq);
    gset_ = v1*fac*sd+av;
    iset_ = 1;
    return v2*fac*sd+av;
  } else {
    iset_ = 0;
    return gset_;
  }
}

int
Random::integer(int min, int max) {
  return (int32() % (max - min + 1)) + min;
}

double
Random::real(double min, double max) {
  return real1() * (max - min) + min;
}

bool
Random::boolean() {
  return real1() < 0.5;
}

long
Random::random_seed() {
  long seed;
#ifdef __linux__
  int fd = open("/dev/random", O_RDONLY|O_NONBLOCK), n;
  if(fd != -1) {
    if((n = read(fd, &seed, sizeof(seed))) != sizeof(seed)) {
      close(fd);
      fd = -1;
    } else {
      close(fd);
    }
  }
  if(fd == -1) {
    puts("Trying /dev/urandom");
    fd = open("/dev/urandom", O_RDONLY|O_NONBLOCK);
    if(fd != -1) {
      if((n = read(fd, &seed, sizeof(seed))) != sizeof(seed)) {
        close(fd);
        fd = -1;
      } else {
        close(fd);
      }
    }
  }
  if(fd == -1) {
    puts("/dev/*random devices failed\n");
#endif
#ifndef WIN32
    struct timeval t;
    gettimeofday(&t, NULL);
    seed = (t.tv_sec << 8) ^ t.tv_usec ^ (getpid() << 16);
#else
    struct _timeb t;
    _ftime(&t);
    seed = (t.time << 10) ^ t.millitm;
#endif
#ifdef __linux__
  }
#endif
  return seed >= 0 ? seed : -seed;
}

/*------------------------------*/



//  What follows is a version of mt19937.c modified to be thread safe.


/* 
   A C-program for MT19937, with initialization improved 2002/1/26.
   Coded by Takuji Nishimura and Makoto Matsumoto.

   Before using, initialize the state by using init_genrand(seed)  
   or init_by_array(init_key, key_length).

   Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
   All rights reserved.                          

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

     1. Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.

     2. Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

     3. The names of its contributors may not be used to endorse or promote 
        products derived from this software without specific prior written 
        permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


   Any feedback is very welcome.
   http://www.math.keio.ac.jp/matumoto/emt.html
   email: matumoto@math.keio.ac.jp
*/

#include <stdio.h>

/* Period parameters */  
#define M 397
#define MATRIX_A 0x9908b0dfUL   /* constant vector a */
#define UPPER_MASK 0x80000000UL /* most significant w-r bits */
#define LOWER_MASK 0x7fffffffUL /* least significant r bits */

/* initializes mt[N] with a seed */
void
Random::init_genrand(unsigned long s)
{
    mt_[0]= s & 0xffffffffUL;
    for (mti_=1; mti_<mt_n_; mti_++) {
        mt_[mti_] = 
	    (1812433253UL * (mt_[mti_-1] ^ (mt_[mti_-1] >> 30)) + mti_);
        /* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
        /* In the previous versions, MSBs of the seed affect   */
        /* only MSBs of the array mt[].                        */
        /* 2002/01/09 modified by Makoto Matsumoto             */
        mt_[mti_] &= 0xffffffffUL;
        /* for >32 bit machines */
    }
}

/* generates a random number on [0,0xffffffff]-interval */
unsigned long
Random::int32()
{
    unsigned long y;
    static unsigned long mag01[2]={0x0UL, MATRIX_A};
    /* mag01[x] = x * MATRIX_A  for x=0,1 */

    if (mti_ >= mt_n_) { /* generate N words at one time */
        int kk;

#if 0
        if (mti_ == N+1)   /* if init_genrand() has not been called, */
            init_genrand(5489UL); /* a default initial seed is used */
#endif

        for (kk=0;kk<mt_n_-M;kk++) {
            y = (mt_[kk]&UPPER_MASK)|(mt_[kk+1]&LOWER_MASK);
            mt_[kk] = mt_[kk+M] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        for (;kk<mt_n_-1;kk++) {
            y = (mt_[kk]&UPPER_MASK)|(mt_[kk+1]&LOWER_MASK);
            mt_[kk] = mt_[kk+(M-mt_n_)] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        y = (mt_[mt_n_-1]&UPPER_MASK)|(mt_[0]&LOWER_MASK);
        mt_[mt_n_-1] = mt_[M-1] ^ (y >> 1) ^ mag01[y & 0x1UL];

        mti_ = 0;
    }
  
    y = mt_[mti_++];

    /* Tempering */
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= (y >> 18);

    return y;
}

/* generates a random number on [0,0x7fffffff]-interval */
long
Random::int31()
{
    return (long)(int32()>>1);
}

/* generates a random number on [0,1]-real-interval */
double
Random::real1()
{
    return int32()*(1.0/4294967295.0); 
    /* divided by 2^32-1 */ 
}

/* generates a random number on [0,1)-real-interval */
double
Random::real2()
{
    return int32()*(1.0/4294967296.0); 
    /* divided by 2^32 */
}

/* generates a random number on (0,1)-real-interval */
double
Random::real3()
{
    return (((double)int32()) + 0.5)*(1.0/4294967296.0); 
    /* divided by 2^32 */
}

/* generates a random number on [0,1) with 53-bit resolution*/
double
Random::res53() 
{ 
    unsigned long a=int32()>>5, b=int32()>>6; 
    return(a*67108864.0+b)*(1.0/9007199254740992.0); 
} 
/* These real versions are due to Isaku Wada, 2002/01/09 added */
