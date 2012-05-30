#pragma scop
/* Clay
   split([0,0], 1);
*/
for(i = 0 ; i <= N ; i++) {
  a[i] = 0;
}
#pragma endscop
