#!/bin/bash
#Generic version via SO
platform='unknown'
unamestr=`uname`

if [[ "${unamestr}" == "Linux" ]];
then
    swig -c++ -python M2MFstAligner.i
    g++  -Ofast -c -fPIC -I/usr/include/python2.7 M2MFstAligner_wrap.cxx M2MFstAligner.cpp FstPathFinder.cpp -lfst -ldl 
    g++  -Ofast -shared -Wl,-soname,_M2MFstAligner.so -o _M2MFstAligner.so  M2MFstAligner.o FstPathFinder.o M2MFstAligner_wrap.o -lfst -ldl
elif [[ "${unamestr}" == "Darwin" ]];
then
    #May need to change these!
    pyH=/opt/local/Library/Frameworks/Python.framework/Versions/2.7/include/python2.7
    pyL=/opt/local/Library/Frameworks/Python.framework/Versions/2.7/lib
    swig -c++ -python M2MFstAligner.i
    g++  -O3 -c -fPIC -I${pyH} M2MFstAligner_wrap.cxx M2MFstAligner.cpp FstPathFinder.cpp -lfst -ldl 
    g++  -O3 -L${pyL} -shared -lc -Wl,-install_name,_M2MFstAligner.so -o _M2MFstAligner.so  M2MFstAligner.o FstPathFinder.o M2MFstAligner_wrap.o -lfst -ldl -lpython2.7
fi