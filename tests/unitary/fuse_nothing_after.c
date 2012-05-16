#pragma scop
/* Clay
   fuse([0]);
*/
for(i = 0 ; i <= N ; i++) {
  a[i] = 0;
}
#pragma endscop
