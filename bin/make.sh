#!/bin/bash
if [ ! -d build_linux ]; then
    mkdir build_linux
fi
cd build_linux
if [ $# -gt 0 ] && [ $1 = "max" ]; then
    echo "has search max version" 
    if [ -f /usr/bin/g++-8 ]; then 
        cmake -DCMAKE_C_COMPILER=/usr/bin/gcc-8 -DCMAKE_CXX_COMPILER=/usr/bin/g++-8 $* ../
    elif [ -f /usr/bin/g++-6 ]; then 
        cmake -DCMAKE_C_COMPILER=/usr/bin/gcc-6 -DCMAKE_CXX_COMPILER=/usr/bin/g++-6 $* ../
    else
        cmake $* ../
    fi
else 
    cmake $* ../
fi
make -j2 
