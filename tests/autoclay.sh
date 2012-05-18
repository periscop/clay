#!/bin/sh

alias clay="../clay";
alias clan="~/usr/bin/clan";

find -name *.c | sort | while read code 
do
  if [ ! -f "$code.scop" ] || [ "$1" = "-a" ]
  then
    rm -f "$code.scop"
    echo "add $code"
    data=`clan "$code" | sed 1d | clay`
    if [ "$data" != "" ]
    then
      echo "$data" >"$code.scop"
    fi
  fi
done
