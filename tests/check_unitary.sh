#!/bin/sh

list_files=`find | grep -e 'unitary\/.*\.c$' | sort`

for code in $list_files; do
  echo "check $code \c"
  clan "$code" | clay | grep -v "enerated by" >/tmp/clay_scop
  n=`diff $code.scop /tmp/clay_scop | wc -l`
  rm -f /tmp/clay_scop
  if [ $n -ne 0 ]; then
    echo "\033[31m[FAIL]\033[0m"
  else
    echo "\033[32m[OK]\033[0m"
  fi
done
