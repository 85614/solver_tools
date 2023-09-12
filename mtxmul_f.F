#include "defs.h"

        SUBROUTINE MUL_F()
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
        CALL backend_push("f-mul"// CHAR(0))

        DO I = 1, N_DIM
            DO J = 1, N_DIM
                C(J, I) = 0
                DO K = 1, N_DIM
                    C(J, I) = C(J, I) + A(K, I) * B(J, K)
                ENDDO
            ENDDO
        ENDDO
        CALL backend_pop()
        ENDDO

        WRITE(*,*) C(4,4)
        
        END