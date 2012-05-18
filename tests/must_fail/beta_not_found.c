#pragma scop
/* Clay
   reorder([3], [1,0,0]);
*/
for(i = 0 ; i <= N ; i++) {
  a[i] = 0;
  b[i] = 0;
}
#pragma endscop
