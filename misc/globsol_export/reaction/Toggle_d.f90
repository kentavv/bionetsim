!  This is a simple example of mixed equality and inequality
!  constraints, one of which is binding at the solution.

PROGRAM TOGGLE

  USE CODELIST_CREATION
  IMPLICIT NONE

      TYPE(CDLVAR), DIMENSION(5):: X
      TYPE(CDLLHS):: PHI
      TYPE(CDLEQ), DIMENSION(5) :: C

      TYPE(CDLVAR), DIMENSION(5) :: C_TMP
      TYPE(CDLVAR) :: SUM
      INTEGER :: I


      CALL INITIALIZE_CODELIST(X)

      CALL SET_RESIDUALS( X(1), X(2), X(3), X(4), X(5) )

      SUM = 0
      DO I = 1,5
         C(I) = C_TMP(I)
         SUM = SUM + C_TMP(I)**2
      END DO
      PHI = SUM

      CALL FINISH_CODELIST

CONTAINS

   SUBROUTINE SET_RESIDUALS( R1, R2, Z, a, b )
      IMPLICIT NONE
      TYPE(CDLVAR) :: R1, R2, Z, a, b
      DOUBLE PRECISION :: k1, k2, k3, k4, k4r, k5, k6, k6r, k7, k8

    ! Constants
      k1 = 0.1D0
      k2 = 0.1D0
      k3 = 0.1D0
      k4 = 0.01D0
      k4r = 1D0
      k5 = 1D0
      k6 = 10D0
      k6r = 1D0
      k7 = 1D0
      k8 = 1D0

    ! Polynomials
      C_TMP(1) = -4 * k6 * R1**4 * b + 4 * k6r * (1-b) + k8 * a - k3 * R1
      C_TMP(2) = -4 * k4 * a * R2**4 + 4 * k4r * (1-a) + k7 * b - k1 * R2
      C_TMP(3) = k5 * a - k2 * Z
      C_TMP(4) = -k4 * a * R2**4 + k4r * (1-a) - k5 * a + k5 * a - k8 * a + k8 * a
      C_TMP(5) = -k6 * R1**4 * b + k6r * (1-b) - k7 * b + k7 * b

   END SUBROUTINE SET_RESIDUALS

END PROGRAM TOGGLE
