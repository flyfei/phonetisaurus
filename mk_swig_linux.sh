#!/bin/bash
#Linux version, shouldn't need any mucking about.
echo "Running massive SWIG code generation cluster-fuck..."
swig -c++ -python M2MFstAligner.i
echo "Compiling..."
g++  -Ofast -c -fPIC -I/usr/include/python2.7 M2MFstAligner_wrap.cxx M2MFstAligner.cpp M2MFstPathFinder.cpp -lfst -ldl 
echo "Building the shared object..."
g++  -Ofast -shared -Wl,-soname,_M2MFstAligner.so -o _M2MFstAligner.so  M2MFstAligner.o M2MFstPathFinder.o M2MFstAligner_wrap.o -lfst -ldl