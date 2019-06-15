/*
 * trans.c - Matrix transpose B = A^T
 *
 * Author: Qizheng "Alex" Zhang (qizhengz)
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

char transpose_sit1_desc[] = "32X32 and 32X64 transpose";
void transpose_sit1(int M, int N, int A[N][M], int B[M][N]){
  for(int i = 0; i < N; i += 8){
    for(int j = 0; j < M; j += 8){
      for(int k = i; k < i + 8; k++){
	int tmp1 = A[k][j];
	int tmp2 = A[k][j+1];
	int tmp3 = A[k][j+2];
	int tmp4 = A[k][j+3];
	int tmp5 = A[k][j+4];
	int tmp6 = A[k][j+5];
	int tmp7 = A[k][j+6];
	int tmp8 = A[k][j+7];
	B[j][k] = tmp1;
	B[j+1][k] = tmp2;
	B[j+2][k] = tmp3;
	B[j+3][k] = tmp4;
	B[j+4][k] = tmp5;
	B[j+5][k] = tmp6;
	B[j+6][k] = tmp7;
	B[j+7][k] = tmp8;
      }
    }
  }
}

char transpose_sit2_desc[] = "64X64 transpose";
void transpose_sit2(int M, int N, int A[N][M], int B[M][N]){
  int rr;
  int tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
  for(int r = 0; r < N; r+=8){
    for(int c = 0; c < M; c+=8){

      for(rr = r; rr < r + 4; rr++){
        tmp0 = A[rr][c];
        tmp1 = A[rr][c+1];
        tmp2 = A[rr][c+2];
        tmp3 = A[rr][c+3];
        tmp4 = A[rr][c+4];
        tmp5 = A[rr][c+5];
        tmp6 = A[rr][c+6];
        tmp7 = A[rr][c+7];
        B[c][rr] = tmp0;
        B[c+1][rr] = tmp1;
        B[c+2][rr] = tmp2;
        B[c+3][rr] = tmp3;
        B[c][rr+4] = tmp4;
        B[c+1][rr+4] = tmp5;
        B[c+2][rr+4] = tmp6;
        B[c+3][rr+4] = tmp7;
      } // deal with the upper-left sub-block

      for(rr = c; rr < c + 4; rr++){
        tmp0 = A[r+4][rr];
        tmp1 = A[r+5][rr];
        tmp2 = A[r+6][rr];
        tmp3 = A[r+7][rr];
        tmp4 = B[rr][r+4];
        tmp5 = B[rr][r+5];
        tmp6 = B[rr][r+6];
        tmp7 = B[rr][r+7];
        // store lower-left first 4 var and upper-right 4 var
        B[rr][r+4] = tmp0;
        B[rr][r+5] = tmp1;
        B[rr][r+6] = tmp2;
        B[rr][r+7] = tmp3;
        B[rr+4][r] = tmp4;
        B[rr+4][r+1] = tmp5;
        B[rr+4][r+2] = tmp6;
        B[rr+4][r+3] = tmp7;
        // exchange value stored in lower-left and upper-right sub-block
      }

      for(rr = c + 4; rr < c + 8; rr++){
        tmp0 = A[r+4][rr];
        tmp1 = A[r+5][rr];
        tmp2 = A[r+6][rr];
        tmp3 = A[r+7][rr];
        B[rr][r+4] = tmp0;
        B[rr][r+5] = tmp1;
        B[rr][r+6] = tmp2;
        B[rr][r+7] = tmp3;
      }

    }
  }
}

/*
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded.
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
  if(M == 32 && N == 32){
    transpose_sit1(M, N, A, B);
  }
  else if(M == 32 && N == 64){
    transpose_sit1(N, M, B, A);
  }
  else if(M == 64 && N == 64){
    transpose_sit2(M, N, A, B);
  }
}

/*
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started.
 */

/*
 * trans - A simple baseline transpose function, not optimized for the cache.
 */

char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc);

    /* Register any additional transpose functions */
    registerTransFunction(transpose_sit1, transpose_sit1_desc);
    registerTransFunction(transpose_sit2, transpose_sit2_desc);
    //registerTransFunction(trans, trans_desc);
}

/*
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}
