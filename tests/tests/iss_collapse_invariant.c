#pragma scop
/* Clay
   iss([0,2], {1||-3});
   collapse([0,2]);
   iss([1], {1||-3});
   split([1,0],1);
   iss([2], {1||-5});
   collapse([2]);
   fuse([1]);
   collapse([1]);
 */
for (i = 0; i < N; i++) {
  S(i);
  S2(i);
  for (j = 0; j < P; j++) {
    S4(i, j);
    S5(j, i);
  }
  S6(i);
}
for (j = 0; j < K; k++) {
  S3(j);
}
#pragma endscop
