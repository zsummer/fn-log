#!/bin/bash
if [ ! -d "./build-check" ]; then
  mkdir build-check
fi
cd build-check
cmake .. 
make -j4
cd ..

cd bin
ts=`find ./ -maxdepth 1  -type f  |egrep -v ".*[a-zA-Z0-9]\..*"`
for t in $ts ;
do
  if [[ "$t" == *test_shm ]]; then
	echo "jump $t"
	continue
  fi
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
