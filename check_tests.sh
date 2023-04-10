#!/bin/bash
if [ ! -d "./build-check" ]; then
  mkdir build-check
fi
cd build-check
cmake .. 
make -j4
cd ..

cd bin
ts=`find ./ -maxdepth 1 -executable -type f  |egrep -v "log" |egrep -v ".gdb*" |egrep -v "*.sh"`
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
