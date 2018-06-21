!  This is a simple example of mixed equality and inequality
!  constraints, one of which is binding at the solution.

PROGRAM REPRESS

  USE CODELIST_CREATION
  IMPLICIT NONE

      TYPE(CDLVAR), DIMENSION(11):: X
      TYPE(CDLLHS):: PHI
      TYPE(CDLEQ), DIMENSION(11) :: C

      TYPE(CDLVAR), DIMENSION(11) :: C_TMP
      TYPE(CDLVAR) :: SUM
      INTEGER :: I


      CALL INITIALIZE_CODELIST(X)

      CALL SET_RESIDUALS( X(1), X(2), X(3), X(4), X(5), X(6), X(7), X(8), X(9), X(10), X(11) )

      SUM = 0
      DO I = 1,11
         C(I) = C_TMP(I)
         SUM = SUM + C_TMP(I)**2
      END DO
      PHI = SUM

      CALL FINISH_CODELIST

CONTAINS

   SUBROUTINE SET_RESIDUALS( CI, CIg, CIm, GFP, GFPg, lacI, lacIg, lacIm, tetR, tetRg, tetRm )
      IMPLICIT NONE
      TYPE(CDLVAR) :: CI, CIg, CIm, GFP, GFPg, lacI, lacIg, lacIm, tetR, tetRg, tetRm
      DOUBLE PRECISION :: k1, k2, k3, k4, k5, k6, k7, k8, k9, k10, k11, k12, k12r, k13, k13r, k14, k14r, k15, k15r, k16, k17, k18

    ! Constants
  k1 = 1
  k2 = 0.1
  k3 = 0.1
  k4 = 0.1
  k5 = 1
  k6 = 1
  k7 = 1
  k8 = 10
  k9 = 1000
  k10 = 10
  k11 = 10
  k12 = 0.1
  k12r = 100
  k13 = 100
  k13r = 0.1
  k14 = 100
  k14r = 0.1
  k15 = 0.1
  k15r = 100
  k16 = 10
  k17 = 10
  k18 = 10
    ! Polynomials
  C_TMP(1) = -2*k15*lacIg*CI**2+2*k15r*(1-lacIg)+k18*CIm-k2*CI
  C_TMP(2) = -k11*CIg+k11*CIg+k14*(1-CIg)-k14r*CIg*tetR**2
  C_TMP(3) = k11*CIg+k18*CIm-k18*CIm-k7*CIm
  C_TMP(4) = k9*GFPg-k1*GFP
  C_TMP(5) = -k9*GFPg+k9*GFPg-k12*GFPg*tetR**2+k12r*(1-GFPg)
  C_TMP(6) = 2*k13*(1-tetRg)-2*k13r*tetRg*lacI**2+k16*lacIm-k4*lacI
  C_TMP(7) = -k10*lacIg+k10*lacIg-k15*lacIg*CI**2+k15r*(1-lacIg)
  C_TMP(8) = k10*lacIg-k16*lacIm+k16*lacIm-k5*lacIm
  C_TMP(9) = -2*k12*GFPg*tetR**2+2*k12r*(1-GFPg)+2*k14*(1-CIg)-2*k14r*CIg*tetR**2+k17*tetRm-k3*tetR
  C_TMP(10) = -k8*tetRg+k8*tetRg+k13*(1-tetRg)-k13r*tetRg*lacI**2
  C_TMP(11) = k8*tetRg-k17*tetRm+k17*tetRm-k6*tetRm

   END SUBROUTINE SET_RESIDUALS

END PROGRAM REPRESS
