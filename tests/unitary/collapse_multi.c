#pragma scop
/* Clay
   iss([0], {1||-3});
   split([0,0],1);
   iss([1], {1||-5});
   collapse([1]);
 */
for (j = 0; j < K; k++) {
  S3(j);
}
#pragma endscop
