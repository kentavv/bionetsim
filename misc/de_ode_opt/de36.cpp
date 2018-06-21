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

/***************************************************************
**                                                            **
**        D I F F E R E N T I A L     E V O L U T I O N       **
**                                                            **
** Program: de.c                                              **
** Version: 3.6                                               **
**                                                            **
** Authors: Dr. Rainer Storn                                  **
**          c/o ICSI, 1947 Center Street, Suite 600           **
**          Berkeley, CA 94707                                **
**          Tel.:   510-642-4274 (extension 192)              **
**          Fax.:   510-643-7684                              **
**          E-mail: storn@icsi.berkeley.edu                   **
**          WWW: http://http.icsi.berkeley.edu/~storn/        **
**          on leave from                                     **
**          Siemens AG, ZFE T SN 2, Otto-Hahn Ring 6          **
**          D-81739 Muenchen, Germany                         **
**          Tel:    636-40502                                 **
**          Fax:    636-44577                                 **
**          E-mail: rainer.storn@zfe.siemens.de               **
**                                                            **
**          Kenneth Price                                     **
**          836 Owl Circle                                    **
**          Vacaville, CA 95687                               **
**          E-mail: kprice@solano.community.net               ** 
**                                                            **
** This program implements some variants of Differential      **
** Evolution (DE) as described in part in the techreport      **
** tr-95-012.ps of ICSI. You can get this report either via   **
** ftp.icsi.berkeley.edu/pub/techreports/1995/tr-95-012.ps.Z  **
** or via WWW: http://http.icsi.berkeley.edu/~storn/litera.html*
** A more extended version of tr-95-012.ps is submitted for   **
** publication in the Journal Evolutionary Computation.       ** 
**                                                            **
** You may use this program for any purpose, give it to any   **
** person or change it according to your needs as long as you **
** are referring to Rainer Storn and Ken Price as the origi-  **
** nators of the the DE idea.                                 **
** If you have questions concerning DE feel free to contact   **
** us. We also will be happy to know about your experiences   **
** with DE and your suggestions of improvement.               **
**                                                            **
***************************************************************/
/**H*O*C**************************************************************
**                                                                  **
** No.!Version! Date ! Request !    Modification           ! Author **
** ---+-------+------+---------+---------------------------+------- **
**  1 + 3.1  +5/18/95+   -     + strategy DE/rand-to-best/1+  Storn **
**    +      +       +         + included                  +        **
**  1 + 3.2  +6/06/95+C.Fleiner+ change loops into memcpy  +  Storn **
**  2 + 3.2  +6/06/95+   -     + update comments           +  Storn **
**  1 + 3.3  +6/15/95+ K.Price + strategy DE/best/2 incl.  +  Storn **
**  2 + 3.3  +6/16/95+   -     + comments and beautifying  +  Storn **
**  3 + 3.3  +7/13/95+   -     + upper and lower bound for +  Storn **
**    +      +       +         + initialization            +        **
**  1 + 3.4  +2/12/96+   -     + increased printout prec.  +  Storn **
**  1 + 3.5  +5/28/96+   -     + strategies revisited      +  Storn **
**  2 + 3.5  +5/28/96+   -     + strategy DE/rand/2 incl.  +  Storn **
**  1 + 3.6  +8/06/96+ K.Price + Binomial Crossover added  +  Storn **
**  2 + 3.6  +9/30/96+ K.Price + cost variance output      +  Storn **
**  3 + 3.6  +9/30/96+   -     + alternative to ASSIGND    +  Storn **
**  4 + 3.6  +10/1/96+   -    + variable checking inserted +  Storn **
**  5 + 3.6  +10/1/96+   -     + strategy indic. improved  +  Storn **
**                                                                  **
***H*O*C*E***********************************************************/


#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "de.h"

#include "evolve_params.h"
#include "individual.h"
#include "log.h"
#include "random.h"
#include "version.h"

EvolveParams ep;
Random rng;
static int gene_len;

void
setupEnvironment(int argc, char *argv[]) {
  const char *filename = argv[1];
  if(!ep.load(filename)) {
    fprintf(stderr, "Unable to load %s\n", filename);
    exit(EXIT_FAILURE);
  }

  ep.cs_mode = argc == 3;
  if(ep.cs_mode) {
    ep.i_am_master = strcmp(argv[2], "master") == 0;
    ep.master_ip.s_addr = inet_addr(argv[2]);
  }
  
  rng.setSeed(ep.random_seed);

#if 0
  {
    QStringList unique_rxn_names = ep.pem.rxn_names;
    unique_rxn_names.sort();
    QStringList::Iterator it, it1, it2;
    for(it1 = unique_rxn_names.begin(), it2 = unique_rxn_names.begin(), ++it2;
        it2 != unique_rxn_names.end(); ++it1,  ++it2) {
      if(*it2 == *it1) {
        unique_rxn_names.erase(it2);
      }
    }
    for(it = unique_rxn_names.begin(); it != unique_rxn_names.end(); ++it) {
      printf("%s\n", (const char*)*it);
    }
  }
#endif

  gene_len = ep.param_ranges.nrows();

  ep.print();
  printf("gene_len: %d\n", gene_len);
  fflush(NULL);
}

/*------------------------Macros----------------------------------------*/

/*#define ASSIGND(a,b) memcpy((a),(b),sizeof(float)*D) */  /* quick copy by Claudio */
                                                           /* works only for small  */
                                                           /* arrays, but is faster.*/

/*------------------------Globals---------------------------------------*/

//long  rnd_uni_init;                 /* serves as a seed for rnd_uni()   */
//float c[MAXPOP][MAXDIM], d[MAXPOP][MAXDIM];
//float (*pold)[MAXPOP][MAXDIM], (*pnew)[MAXPOP][MAXDIM], (*pswap)[MAXPOP][MAXDIM];


/*---------Function declarations----------------------------------------*/

//void  assignd(int D, float a[], float b[]);
//float rnd_uni(long *idum);    /* uniform pseudo random number generator */
//float extern evaluate(int D, float tmp[], long *nfeval); /* obj. funct. */

/*---------Function definitions-----------------------------------------*/

#if 0
void  assignd(int D, float a[], float b[])
/**C*F****************************************************************
**                                                                  **
** Assigns D-dimensional vector b to vector a.                      **
** You might encounter problems with the macro ASSIGND on some      **
** machines. If yes, better use this function although it's slower. **
**                                                                  **
***C*F*E*************************************************************/
{
   int j;
   for (j=0; j<D; j++)
   {
      a[j] = b[j];
   }
}
#endif


#if 0
float rnd_uni(long *idum)
/**C*F****************************************************************
**                                                                  **
** SRC-FUNCTION   :rnd_uni()                                        **
** LONG_NAME      :random_uniform                                   **
** AUTHOR         :(see below)                                      **
**                                                                  **
** DESCRIPTION    :rnd_uni() generates an equally distributed ran-  **
**                 dom number in the interval [0,1]. For further    **
**                 reference see Press, W.H. et alii, Numerical     **
**                 Recipes in C, Cambridge University Press, 1992.  **
**                                                                  **
** FUNCTIONS      :none                                             **
**                                                                  **
** GLOBALS        :none                                             **
**                                                                  **
** PARAMETERS     :*idum    serves as a seed value                  **
**                                                                  **
** PRECONDITIONS  :*idum must be negative on the first call.        **
**                                                                  **
** POSTCONDITIONS :*idum will be changed                            **
**                                                                  **
***C*F*E*************************************************************/
{
  long j;
  long k;
  static long idum2=123456789;
  static long iy=0;
  static long iv[NTAB];
  float temp;

  if (*idum <= 0)
  {
    if (-(*idum) < 1) *idum=1;
    else *idum = -(*idum);
    idum2=(*idum);
    for (j=NTAB+7;j>=0;j--)
    {
      k=(*idum)/IQ1;
      *idum=IA1*(*idum-k*IQ1)-k*IR1;
      if (*idum < 0) *idum += IM1;
      if (j < NTAB) iv[j] = *idum;
    }
    iy=iv[0];
  }
  k=(*idum)/IQ1;
  *idum=IA1*(*idum-k*IQ1)-k*IR1;
  if (*idum < 0) *idum += IM1;
  k=idum2/IQ2;
  idum2=IA2*(idum2-k*IQ2)-k*IR2;
  if (idum2 < 0) idum2 += IM2;
  j=iy/NDIV;
  iy=iv[j]-idum2;
  iv[j] = *idum;
  if (iy < 1) iy += IMM1;
  if ((temp=AM*iy) > RNMX) return RNMX;
  else return temp;

}/*------End of rnd_uni()--------------------------*/
#endif



int
main(int argc, char *argv[])
/**C*F****************************************************************
**                                                                  **
** SRC-FUNCTION   :main()                                           **
** LONG_NAME      :main program                                     **
** AUTHOR         :Rainer Storn, Kenneth Price                      **
**                                                                  **
** DESCRIPTION    :driver program for differential evolution.       **
**                                                                  **
** FUNCTIONS      :rnd_uni(), evaluate(), printf(), fprintf(),      **
**                 fopen(), fclose(), fscanf().                     **
**                                                                  **
** GLOBALS        :rnd_uni_init    input variable for rnd_uni()     **
**                                                                  **
** PARAMETERS     :argc            #arguments = 3                   **
**                 argv            pointer to argument strings      **
**                                                                  **
** PRECONDITIONS  :main must be called with three parameters        **
**                 e.g. like de1 <input-file> <output-file>, if     **
**                 the executable file is called de1.               **
**                 The input file must contain valid inputs accor-  **
**                 ding to the fscanf() section of main().          **
**                                                                  **
** POSTCONDITIONS :main() produces consecutive console outputs and  **
**                 writes the final results in an output file if    **
**                 the program terminates without an error.         **
**                                                                  **
***C*F*E*************************************************************/

{
  //char  chr;             /* y/n choice variable                */
   const char  *strat[20];   /* strategy-indicator                 */

   int   i, j, L, n;      /* counting variables                 */
   int   r1, r2, r3, r4;  /* placeholders for random indexes    */
   int   r5;              /* placeholders for random indexes    */
   int   D;               /* Dimension of parameter vector      */
   int   NP;              /* number of population members       */
   int   imin;            /* index to member with lowest energy */
   int   refresh;         /* refresh rate of screen output      */
   int   strategy;        /* choice parameter for screen output */
   int   gen, genmax; //, seed;   

   long  nfeval;          /* number of function evaluations     */

   float trial_cost;      /* buffer variable                    */
   //float inibound_h;      /* upper parameter bound              */
   //float inibound_l;      /* lower parameter bound              */
   //float tmp[MAXDIM], best[MAXDIM], bestit[MAXDIM]; /* members  */
   Individual tmp, best, bestit;
   //float cost[MAXPOP];    /* obj. funct. values                 */
   float cvar;            /* computes the cost variance         */
   float cmean;           /* mean cost                          */
   float F,CR;            /* control variables of DE            */
   float cmin;            /* help variables                     */

   //FILE  *fpin_ptr;
   FILE  *fpout_ptr;

   if(argc < 2) {
    fprintf(stderr, "%s: <parameter file> [<master IP address> | \"master\"]\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  appname = "DE ODE Opt";
  appversion = "0.01 devel";

  // Disable debug and warning output
  setLogFP(stdout, NULL, NULL, NULL);
  
  setupEnvironment(argc, argv);

/*------Initializations----------------------------*/

#if 0
  if (argc != 3)                                 /* number of arguments */
 {
    printf("\nUsage : de <input-file> <output-file>\n");
    exit(1);
 }
  

 fpout_ptr = fopen(argv[2],"r");          /* Open Output file for writing */

 if ( fpout_ptr != NULL )
 {
    printf("\nOutput file %s does already exist, \ntype y if you ",argv[2]);
    printf("want to overwrite it, \nanything else if you want to exit.\n");
    chr = (char)getchar();
    if ((chr != 'y') && (chr != 'Y'))
    {
      exit(1);
    }
    fclose(fpout_ptr);
 }
#endif

 
 strat[1]  = "DE/best/1/exp          ";
 strat[2]  = "DE/rand/1/exp          ";
 strat[3]  = "DE/rand-to-best/1/exp  ";
 strat[4]  = "DE/best/2/exp          ";
 strat[5]  = "DE/rand/2/exp          ";
 strat[6]  = "DE/best/1/bin          ";
 strat[7]  = "DE/rand/1/bin          ";
 strat[8]  = "DE/rand-to-best/1/bin  ";
 strat[9]  = "DE/best/2/bin          ";
 strat[10] = "DE/rand/2/bin          ";

/*-----Read input data------------------------------------------------*/

#if 0
 fpin_ptr   = fopen(argv[1],"r");

 if (fpin_ptr == NULL)
 {
    printf("\nCannot open input file\n");
    exit(1);                                 /* input file is necessary */
 }
#endif
 
 //fscanf(fpin_ptr,"%d",&strategy);       /*---choice of strategy-----------------*/
 strategy = 7;
 strategy = 3;
 //fscanf(fpin_ptr,"%d",&genmax);         /*---maximum number of generations------*/
 genmax = ep.ngen;
 //fscanf(fpin_ptr,"%d",&refresh);        /*---output refresh cycle---------------*/
 refresh = 1;
 //fscanf(fpin_ptr,"%d",&D);              /*---number of parameters---------------*/
 D = gene_len;
 //fscanf(fpin_ptr,"%d",&NP);             /*---population size.-------------------*/
 NP = ep.pop_size;
 //fscanf(fpin_ptr,"%f",&inibound_h);     /*---upper parameter bound for init-----*/
 //fscanf(fpin_ptr,"%f",&inibound_l);     /*---lower parameter bound for init-----*/
 //fscanf(fpin_ptr,"%f",&F);              /*---weight factor----------------------*/
 F = 0.9;
 //fscanf(fpin_ptr,"%f",&CR);             /*---crossing over factor---------------*/
 CR = 1.0;
 //fscanf(fpin_ptr,"%d",&seed);           /*---random seed------------------------*/
 //seed = ep.random_seed;


 //fclose(fpin_ptr);

/*-----Checking input variables for proper range----------------------------*/

#if 0
  if (D > MAXDIM)
  {
     printf("\nError! D=%d > MAXDIM=%d\n",D,MAXDIM);
     exit(1);
  }
  if (D <= 0)
  {
     printf("\nError! D=%d, should be > 0\n",D);
     exit(1);
  }
  if (NP > MAXPOP)
  {
     printf("\nError! NP=%d > MAXPOP=%d\n",NP,MAXPOP);
     exit(1);
  }
  if (NP <= 0)
  {
     printf("\nError! NP=%d, should be > 0\n",NP);
     exit(1);
  }
#endif
  if ((CR < 0) || (CR > 1.0))
  {
     printf("\nError! CR=%f, should be ex [0,1]\n",CR);
     exit(1);
  }
#if 0
  if (seed <= 0)
  {
     printf("\nError! seed=%d, should be > 0\n",seed);
     exit(1);
  }
#endif
  if (refresh <= 0)
  {
     printf("\nError! refresh=%d, should be > 0\n",refresh);
     exit(1);
  }
  if (genmax <= 0)
  {
     printf("\nError! genmax=%d, should be > 0\n",genmax);
     exit(1);
  }
  if ((strategy < 0) || (strategy > 10))
  {
     printf("\nError! strategy=%d, should be ex {1,2,3,4,5,6,7,8,9,10}\n",strategy);
     exit(1);
  }
#if 0
  if (inibound_h < inibound_l)
  {
     printf("\nError! inibound_h=%f < inibound_l=%f\n",inibound_h, inibound_l);
     exit(1);
  }
#endif
  

/*-----Open output file-----------------------------------------------*/

#if 0
  fpout_ptr   = fopen(argv[2],"w");

   if (fpout_ptr == NULL)
   {
      printf("\nCannot open output file\n");
      exit(1);
   }
#endif
   fpout_ptr = stdout;
   

/*-----Initialize random number generator-----------------------------*/

   //rnd_uni_init = -(long)seed;  /* initialization of rnd_uni() */
 nfeval       =  0;  /* reset number of function evaluations */



/*------Initialization------------------------------------------------*/
/*------Right now this part is kept fairly simple and just generates--*/
/*------random numbers in the range [-initfac, +initfac]. You might---*/
/*------want to extend the init part such that you can initialize-----*/
/*------each parameter separately.------------------------------------*/

 Individual *c = new Individual[ep.pop_size](gene_len);
 Individual *d = new Individual[ep.pop_size];
 Individual **pold, **pnew, **pswap;
 double *cost = new double[ep.pop_size];
				      
   for (i=0; i<NP; i++)
   {
     c[i].random();
#if 0
     for (j=0; j<D; j++) /* spread initial population members */
      {
	//c[i][j] = inibound_l + rnd_uni(&rnd_uni_init)*(inibound_h - inibound_l);
	c[i][j] = ranges[j][0] + rnd_uni(&rnd_uni_init)*(ranges[j][1] - ranges[j][0]);
      }
#endif
     cost[i] = c[i].fitness(); //evaluate(D,c[i],&nfeval); /* obj. funct. value */
   }
   cmin = cost[0];
   imin = 0;
   for (i=1; i<NP; i++)
   {
      if (cost[i]<cmin)
      {
	 cmin = cost[i];
	 imin = i;
      }
   }

   //assignd(D,best,c[imin]);            /* save best member ever          */
   //assignd(D,bestit,c[imin]);          /* save best member of generation */
   best = c[imin];
   bestit = c[imin];
   
   pold = &c; /* old population (generation G)   */
   pnew = &d; /* new population (generation G+1) */
   
/*=======================================================================*/
/*=========Iteration loop================================================*/
/*=======================================================================*/

   gen = 0;                          /* generation counter reset */
   while ((gen < genmax) /*&& (kbhit() == 0)*/) /* remove comments if conio.h */
   {                                            /* is accepted by compiler    */
      gen++;
      imin = 0;

      for (i=0; i<NP; i++)         /* Start of loop through ensemble  */
      {
	 do                        /* Pick a random population member */
	 {                         /* Endless loop for NP < 2 !!!     */
	   r1 = rng.integer(0, NP-1); //(int)(rnd_uni(&rnd_uni_init)*NP);
	 }while(r1==i);            

	 do                        /* Pick a random population member */
	 {                         /* Endless loop for NP < 3 !!!     */
	   r2 = rng.integer(0, NP-1); //(int)(rnd_uni(&rnd_uni_init)*NP);
	 }while((r2==i) || (r2==r1));

	 do                        /* Pick a random population member */
	 {                         /* Endless loop for NP < 4 !!!     */
	   r3 = rng.integer(0, NP-1); //(int)(rnd_uni(&rnd_uni_init)*NP);
	 }while((r3==i) || (r3==r1) || (r3==r2));

	 do                        /* Pick a random population member */
	 {                         /* Endless loop for NP < 5 !!!     */
	   r4 = rng.integer(0, NP-1); //(int)(rnd_uni(&rnd_uni_init)*NP);
	 }while((r4==i) || (r4==r1) || (r4==r2) || (r4==r3));

	 do                        /* Pick a random population member */
	 {                         /* Endless loop for NP < 6 !!!     */
	   r5 = rng.integer(0, NP-1); //(int)(rnd_uni(&rnd_uni_init)*NP);
	 }while((r5==i) || (r5==r1) || (r5==r2) || (r5==r3) || (r5==r4));


/*=======Choice of strategy===============================================================*/
/*=======We have tried to come up with a sensible naming-convention: DE/x/y/z=============*/
/*=======DE :  stands for Differential Evolution==========================================*/
/*=======x  :  a string which denotes the vector to be perturbed==========================*/
/*=======y  :  number of difference vectors taken for perturbation of x===================*/
/*=======z  :  crossover method (exp = exponential, bin = binomial)=======================*/
/*                                                                                        */
/*=======There are some simple rules which are worth following:===========================*/
/*=======1)  F is usually between 0.5 and 1 (in rare cases > 1)===========================*/
/*=======2)  CR is between 0 and 1 with 0., 0.3, 0.7 and 1. being worth to be tried first=*/
/*=======3)  To start off NP = 10*D is a reasonable choice. Increase NP if misconvergence=*/
/*           happens.                                                                     */
/*=======4)  If you increase NP, F usually has to be decreased============================*/
/*=======5)  When the DE/best... schemes fail DE/rand... usually works and vice versa=====*/


/*=======EXPONENTIAL CROSSOVER============================================================*/

/*-------DE/best/1/exp--------------------------------------------------------------------*/
/*-------Our oldest strategy but still not bad. However, we have found several------------*/
/*-------optimization problems where misconvergence occurs.-------------------------------*/
	 if (strategy == 1) /* strategy DE0 (not in our paper) */
	 {
	   //assignd(D,tmp,(*pold)[i]);
	   tmp = (*pold)[i];
	   n = rng.integer(0, D-1); //(int)(rnd_uni(&rnd_uni_init)*D);
	   L = 0;
	   do
	   {                       
	     tmp.ref(n) = bestit(n) + F*((*pold)[r2](n)-(*pold)[r3](n));
	     n = (n+1)%D;
	     L++;
	     //}while((rnd_uni(&rnd_uni_init) < CR) && (L < D));
	   }while((rng.real1() < CR) && (L < D));
	 }
/*-------DE/rand/1/exp-------------------------------------------------------------------*/
/*-------This is one of my favourite strategies. It works especially well when the-------*/
/*-------"bestit[]"-schemes experience misconvergence. Try e.g. F=0.7 and CR=0.5---------*/
/*-------as a first guess.---------------------------------------------------------------*/
	 else if (strategy == 2) /* strategy DE1 in the techreport */
	 {
	   //assignd(D,tmp,(*pold)[i]);
	   tmp = (*pold)[i];
	   n = rng.integer(0, D-1); //(int)(rnd_uni(&rnd_uni_init)*D);
	   L = 0;
	   do
	   {                       
	     tmp.ref(n) = (*pold)[r1](n) + F*((*pold)[r2](n)-(*pold)[r3](n));
	     n = (n+1)%D;
	     L++;
	     //}while((rnd_uni(&rnd_uni_init) < CR) && (L < D));
	   }while((rng.real1() < CR) && (L < D));
	 }
/*-------DE/rand-to-best/1/exp-----------------------------------------------------------*/
/*-------This strategy seems to be one of the best strategies. Try F=0.85 and CR=1.------*/
/*-------If you get misconvergence try to increase NP. If this doesn't help you----------*/
/*-------should play around with all three control variables.----------------------------*/
	 else if (strategy == 3) /* similiar to DE2 but generally better */
	 { 
	   //assignd(D,tmp,(*pold)[i]);
	   tmp = (*pold)[i];
	   n = rng.integer(0, D-1); //(int)(rnd_uni(&rnd_uni_init)*D); 
	   L = 0;
	   do
	   {                       
	     tmp.ref(n) = tmp(n) + F*(bestit(n) - tmp(n)) + F*((*pold)[r1](n)-(*pold)[r2](n));
	     n = (n+1)%D;
	     L++;
	     //}while((rnd_uni(&rnd_uni_init) < CR) && (L < D));
	   }while((rng.real1() < CR) && (L < D));
	 }
/*-------DE/best/2/exp is another powerful strategy worth trying--------------------------*/
	 else if (strategy == 4)
	 { 
	   //assignd(D,tmp,(*pold)[i]);
	   tmp = (*pold)[i];
	   n = rng.integer(0, D-1); //(int)(rnd_uni(&rnd_uni_init)*D); 
	   L = 0;
	   do
	   {                           
	     tmp.ref(n) = bestit(n) + 
		      ((*pold)[r1](n)+(*pold)[r2](n)-(*pold)[r3](n)-(*pold)[r4](n))*F;
	     n = (n+1)%D;
	     L++;
	     //}while((rnd_uni(&rnd_uni_init) < CR) && (L < D));
	   }while((rng.real1() < CR) && (L < D));
	 }
/*-------DE/rand/2/exp seems to be a robust optimizer for many functions-------------------*/
	 else if (strategy == 5)
	 { 
	   //assignd(D,tmp,(*pold)[i]);
	   tmp = (*pold)[i];
	   n = rng.integer(0, D-1); //(int)(rnd_uni(&rnd_uni_init)*D); 
	   L = 0;
	   do
	   {                           
	     tmp.ref(n) = (*pold)[r5](n) + 
		      ((*pold)[r1](n)+(*pold)[r2](n)-(*pold)[r3](n)-(*pold)[r4](n))*F;
	     n = (n+1)%D;
	     L++;
	     //}while((rnd_uni(&rnd_uni_init) < CR) && (L < D));
	   }while((rng.real1() < CR) && (L < D));
	 }

/*=======Essentially same strategies but BINOMIAL CROSSOVER===============================*/

/*-------DE/best/1/bin--------------------------------------------------------------------*/
	 else if (strategy == 6) 
	 {
	   //assignd(D,tmp,(*pold)[i]);
	   tmp = (*pold)[i];
	   n = rng.integer(0, D-1); //(int)(rnd_uni(&rnd_uni_init)*D); 
           for (L=0; L<D; L++) /* perform D binomial trials */
           {
	     //if ((rnd_uni(&rnd_uni_init) < CR) || L == (D-1)) /* change at least one parameter */
	     if ((rng.real1() < CR) || L == (D-1)) /* change at least one parameter */
	     {                       
	       tmp.ref(n) = bestit(n) + F*((*pold)[r2](n)-(*pold)[r3](n));
	     }
	     n = (n+1)%D;
           }
	 }
/*-------DE/rand/1/bin-------------------------------------------------------------------*/
	 else if (strategy == 7) 
	 {
	   //assignd(D,tmp,(*pold)[i]);
	   tmp = (*pold)[i];
	   n = rng.integer(0, D-1); //(int)(rnd_uni(&rnd_uni_init)*D); 
           for (L=0; L<D; L++) /* perform D binomial trials */
           {
	     //if ((rnd_uni(&rnd_uni_init) < CR) || L == (D-1)) /* change at least one parameter */
	     if ((rng.real1() < CR) || L == (D-1)) /* change at least one parameter */
	     {                       
	       tmp.ref(n) = (*pold)[r1](n) + F*((*pold)[r2](n)-(*pold)[r3](n));
	     }
	     n = (n+1)%D;
           }
	 }
/*-------DE/rand-to-best/1/bin-----------------------------------------------------------*/
	 else if (strategy == 8) 
	 { 
	   //assignd(D,tmp,(*pold)[i]);
	   tmp = (*pold)[i];
	   n = rng.integer(0, D-1); //(int)(rnd_uni(&rnd_uni_init)*D); 
           for (L=0; L<D; L++) /* perform D binomial trials */
           {
	     //if ((rnd_uni(&rnd_uni_init) < CR) || L == (D-1)) /* change at least one parameter */
	     if ((rng.real1() < CR) || L == (D-1)) /* change at least one parameter */
	     {                       
	       tmp.ref(n) = tmp(n) + F*(bestit(n) - tmp(n)) + F*((*pold)[r1](n)-(*pold)[r2](n));
	     }
	     n = (n+1)%D;
           }
	 }
/*-------DE/best/2/bin--------------------------------------------------------------------*/
	 else if (strategy == 9)
	 { 
	   //assignd(D,tmp,(*pold)[i]);
	   tmp = (*pold)[i];
	   n = rng.integer(0, D-1); //(int)(rnd_uni(&rnd_uni_init)*D); 
           for (L=0; L<D; L++) /* perform D binomial trials */
           {
	     //if ((rnd_uni(&rnd_uni_init) < CR) || L == (D-1)) /* change at least one parameter */
	     if ((rng.real1() < CR) || L == (D-1)) /* change at least one parameter */
	     {                       
	       tmp.ref(n) = bestit(n) + 
		      ((*pold)[r1](n)+(*pold)[r2](n)-(*pold)[r3](n)-(*pold)[r4](n))*F;
	     }
	     n = (n+1)%D;
           }
	 }
/*-------DE/rand/2/bin--------------------------------------------------------------------*/
	 else
	 { 
	   //assignd(D,tmp,(*pold)[i]);
	   tmp = (*pold)[i];
	   n = rng.integer(0, D-1); //(int)(rnd_uni(&rnd_uni_init)*D); 
           for (L=0; L<D; L++) /* perform D binomial trials */
           {
	     //if ((rnd_uni(&rnd_uni_init) < CR) || L == (D-1)) /* change at least one parameter */
	     if ((rng.real1() < CR) || L == (D-1)) /* change at least one parameter */
	     {                       
	       tmp.ref(n) = (*pold)[r5](n) + 
		      ((*pold)[r1](n)+(*pold)[r2](n)-(*pold)[r3](n)-(*pold)[r4](n))*F;
	     }
	     n = (n+1)%D;
           }
	 }


/*=======Trial mutation now in tmp[]. Test how good this choice really was.==================*/

	 //trial_cost = evaluate(D,tmp,&nfeval);  /* Evaluate new vector in tmp[] */
	 trial_cost = tmp.fitness();

	 if (trial_cost <= cost[i])   /* improved objective function value ? */
	 {                                  
	    cost[i]=trial_cost;         
	    //assignd(D,(*pnew)[i],tmp);
	    (*pnew)[i] = tmp;
	    if (trial_cost<cmin)          /* Was this a new minimum? */
	    {                               /* if so...*/
	       cmin=trial_cost;           /* reset cmin to new low...*/
	       imin=i;
	       //assignd(D,best,tmp);
	       best = tmp;
	    }                           
	 }                            
	 else
	 {
	   //assignd(D,(*pnew)[i],(*pold)[i]); /* replace target with old value */
	   (*pnew)[i] = (*pold)[i];
	 }
      }   /* End mutation loop through pop. */
					   
      //assignd(D,bestit,best);  /* Save best population member of current iteration */
      bestit = best;

      /* swap population arrays. New generation becomes old one */

      pswap = pold;
      pold  = pnew;
      pnew  = pswap;

/*----Compute the energy variance (just for monitoring purposes)-----------*/

      cmean = 0.;          /* compute the mean value first */
      for (j=0; j<NP; j++)
      {
         cmean += cost[j];
      }
      cmean = cmean/NP;

      cvar = 0.;           /* now the variance              */
      for (j=0; j<NP; j++)
      {
         cvar += (cost[j] - cmean)*(cost[j] - cmean);
      }
      cvar = cvar/(NP-1);


/*----Output part----------------------------------------------------------*/

      if (1 /*gen%refresh==1)*/)   /* display after every refresh generations */
      { /* ABORT works only if conio.h is accepted by your compiler */
	printf("\n\n                         PRESS ANY KEY TO ABORT"); 
	printf("\n\n\n Best-so-far cost funct. value=%-15.10g\n",cmin);

	for (j=0;j<D;j++)
	{
	  printf("\n best[%d]=%-15.10g",j,best(j));
	}
	printf("\n\n Generation=%d  NFEs=%ld   Strategy: %s    ",gen,nfeval,strat[strategy]);
	printf("\n NP=%d    F=%-4.2g    CR=%-4.2g   cost-variance=%-10.5g\n",
               NP,F,CR,cvar);
      }

      fprintf(fpout_ptr,"%ld   %-15.10g\n",nfeval,cmin);
   }
/*=======================================================================*/
/*=========End of iteration loop=========================================*/
/*=======================================================================*/

/*-------Final output in file-------------------------------------------*/


   fprintf(fpout_ptr,"\n\n\n Best-so-far obj. funct. value = %-15.10g\n",cmin);

   for (j=0;j<D;j++)
   {
     fprintf(fpout_ptr,"\n best[%d]=%-15.10g",j,best(j));
   }
   fprintf(fpout_ptr,"\n\n Generation=%d  NFEs=%ld   Strategy: %s    ",gen,nfeval,strat[strategy]);
   fprintf(fpout_ptr,"\n NP=%d    F=%-4.2g    CR=%-4.2g    cost-variance=%-10.5g\n",
           NP,F,CR,cvar); 

   best.print();
   
   //fclose(fpout_ptr);

   return(0);
}

/*-----------End of main()------------------------------------------*/





