#include <stdio.h>
#define N 100

int main()
{ int i1=0, i2=0, j=0, n=100 ;
  float a[N+1][N+1], b[N+1], c[N+1], result ;

  /* ax-do kernel */
#pragma scop
/* Clay
fission(S0, 1);
tile([1,0], 2, 1, 32, 0);
tile([1,0], 2, 1, 32, 0);
*/
  for (i1=1;i1<=n;i1++) {                 
    c[i1] = 0 ;                       
    for (j=1;j<=n;j++)              
      c[i1] = c[i1] + a[i1][j] * b[j] ;
  }
#pragma endscop
  
  result = c[N-1];
  printf("fib[%d] = %d\n", N-1, result);

  return 0;
}
