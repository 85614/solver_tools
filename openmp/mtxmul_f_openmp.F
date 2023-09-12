#include "defs.h"
        SUBROUTINE MUL_F_OPENMP()

        implicit none
        include "omp_lib.h"

        REAL A(N_DIM,N_DIM) ,B(N_DIM,N_DIM), C(N_DIM,N_DIM)
        REAL TMP
        INTEGER L, I, J, K

        TMP = 0

        DO I = 1, N_DIM
            DO J = 1, N_DIM
                A(J, I) = TMP
                B(J, I) = TMP
                TMP = TMP + 1
            ENDDO
        ENDDO


        DO L = 1, N_LOOP
        CALL backend_push("f-mul-openmp" // CHAR(0))

        
c$omp  PARALLEL DO SCHEDULE(static, 4)
c$omp& shared(A,B,C) private(I, J, K)
        DO I = 1, N_DIM
            DO J = 1, N_DIM
                C(J, I) = 0
                DO K = 1, N_DIM
                    C(J, I) = C(J, I) + A(K, I) * B(J, K)
                ENDDO
            ENDDO
        ENDDO
c$omp  end parallel do
        CALL backend_pop()
        ENDDO

        WRITE(*,*) C(4,4)
        
        END SUBROUTINE


        SUBROUTINE MUL_F_PARA()

        REAL A(N_DIM,N_DIM) ,B(N_DIM,N_DIM), C(N_DIM,N_DIM)
        REAL TMP
        INTEGER L, I, J

        TMP = 0

        DO I = 1, N_DIM
            DO J = 1, N_DIM
                A(J, I) = TMP
                B(J, I) = TMP
                TMP = TMP + 1
            ENDDO
        ENDDO


        DO L = 1, N_LOOP
        CALL backend_push("f-mul-para"// CHAR(0))

        CALL MUL_F_PARA_REQUEST(A,B,C)
        CALL backend_pop()
        ENDDO
        WRITE(*,*) C(4,4)
        
        END SUBROUTINE

        SUBROUTINE MUL_F_PARA_INNER(A,B,C,I_BEGIN, I_END)

        REAL A(N_DIM,N_DIM) ,B(N_DIM,N_DIM), C(N_DIM,N_DIM)
        INTEGER I, J, K, I_BEGIN, I_END
        DO I = I_BEGIN, I_END
            DO J = 1, N_DIM
                C(J, I) = 0
                DO K = 1, N_DIM
                    C(J, I) = C(J, I) + A(K, I) * B(J, K)
                ENDDO
            ENDDO
        ENDDO
        END SUBROUTINE