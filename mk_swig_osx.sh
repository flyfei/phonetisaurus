#!/bin/bash
#Change the pyH and pyL paths to reflect your environment
#This version is for OSX
pyH=/opt/local/Library/Frameworks/Python.framework/Versions/2.7/include/python2.7
pyL=/opt/local/Library/Frameworks/Python.framework/Versions/2.7/lib

echo "Generating SWIG code..."
swig -c++ -python M2MFstAligner.i
echo "Compiling..."
g++  -O3 -c -fPIC -I${pyH} M2MFstAligner_wrap.cxx M2MFstAligner.cpp FstPathFinder.cpp -lfst -ldl 
echo "Building the shared object..."
g++  -O3 -L${pyL} -shared -lc -Wl,-install_name,_M2MFstAligner.so -o _M2MFstAligner.so  M2MFstAligner.o FstPathFinder.o M2MFstAligner_wrap.o -lfst -ldl -lpython2.7