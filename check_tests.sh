#!/bin/bash
cd bin
ts=`find ./ -maxdepth 1 -executable -type f`
for t in $ts ;
do
  echo begin test $t 
  $t
  if [ $? -eq 0 ]; then
    echo test $t success.
  else
    echo test $t failed
    exit $?
  fi

done
cd ..
