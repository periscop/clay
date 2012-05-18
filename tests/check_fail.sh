#!/bin/sh

list_files=`find | grep -e 'must_fail\/.*\.c$' | sort`

for code in $list_files; do
  echo "check $code \c"
  clan "$code" | clay 2>/dev/null 1>/dev/null
  ret=$?

  if [ $ret -eq 0 ]; then
    echo "\033[33m[INFO] : This program seems to work\033[0m"
  elif [ $ret -eq 139 ]; then
    echo "\033[31m[FAIL] : Segmentation fault !\033[0m"
  else
    echo "\033[32m[OK]\033[0m"
  fi
done

