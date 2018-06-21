!  This is a simple example of mixed equality and inequality
!  constraints, one of which is binding at the solution.

PROGRAM VELDEN1

  USE CODELIST_CREATION
  IMPLICIT NONE

      TYPE(CDLVAR), DIMENSION(4):: X
      TYPE(CDLLHS):: PHI
      TYPE(CDLEQ), DIMENSION(4) :: C

      TYPE(CDLVAR), DIMENSION(4) :: C_TMP
      TYPE(CDLVAR) :: SUM
      INTEGER :: I


      CALL INITIALIZE_CODELIST(X)

      CALL SET_RESIDUALS( X(1), X(2), X(3), X(4) )

      SUM = 0
      DO I = 1,4
         C(I) = C_TMP(I)
         SUM = SUM + C_TMP(I)**2
      END DO
      PHI = SUM

      CALL FINISH_CODELIST

CONTAINS

   SUBROUTINE SET_RESIDUALS( a, b, BB, d )
      IMPLICIT NONE
      TYPE(CDLVAR) :: a, b, BB, d
      DOUBLE PRECISION :: k1, k2, k3, k4, k5, k6, k7, k7r, k8

    ! Constants
      k1  = 0.2D0
      k2  = 0.0085D0
      k3  = 0.37D0
      k4  = 0.034D0
      k5  = 0.72D0
      k6  = 0.53D0
      k7  = 0.19D0
      k7r = 0.42D0
      k8  = 0.027D0

    ! Polynomials
      C_TMP(1) = k1*1 -k2*A -k5*A*BB -k7*b*A + k7r*(1-b)
      C_TMP(2) = -k7*b*A + k7r*(1-b)
      C_TMP(3) = k3*b -k4*BB -k5*A*BB + k8*(1-b)
      C_TMP(4) = k5*A*BB -k6*d
   END SUBROUTINE SET_RESIDUALS

END PROGRAM VELDEN1
