!  This is a simple example of mixed equality and inequality
!  constraints, one of which is binding at the solution.

PROGRAM DUAL

  USE CODELIST_CREATION

      TYPE(CDLVAR), DIMENSION(1):: X
      TYPE(CDLLHS):: PHI
      TYPE(CDLEQ), DIMENSION(1) :: C

      TYPE(CDLVAR), DIMENSION(1) :: C_TMP
      TYPE(CDLVAR) :: SUM
      INTEGER :: I


      CALL INITIALIZE_CODELIST(X)

      CALL SET_RESIDUALS( X(1) )

      SUM = 0
      DO I = 1,1
         C(I) = C_TMP(I)
         SUM = SUM + C_TMP(I)**2
      END DO
      PHI = SUM

      CALL FINISH_CODELIST

CONTAINS

   SUBROUTINE SET_RESIDUALS( a )
      TYPE(CDLVAR) :: a

    ! Constants

    ! Polynomials
      C_TMP(1) = (a-1)*(a-1) - 4;
   END SUBROUTINE SET_RESIDUALS

END PROGRAM DUAL
