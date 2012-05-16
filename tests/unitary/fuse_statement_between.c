#pragma scop
/* Clay
   fuse([1]);
*/
a = 0;
for(i = 0 ; i <= N ; i++) {
  b[i] = 0;
}
c = 0;
for(i = 0 ; i <= N ; i++) {
  t[i] = 0;
  s[i] = 0;
}
f = 0;
#pragma endscop
