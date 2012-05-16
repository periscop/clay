#pragma scop
/* Clay
   iss([0], [1,0,-10]);
*/
for(i = 0 ; i <= N ; i++) {
  a[i] = 0;
  if(i >= 10) {
    b[i] = 0;
  }
}
#pragma endscop
