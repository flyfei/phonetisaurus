#!/bin/bash
#    Copyright 2011-2012 Josef Robert Novak
#
#    This file is part of Phonetisaurus.
#
#    Phonetisaurus is free software: you can redistribute it 
#    and/or modify it under the terms of the GNU General Public 
#    License as published by the Free Software Foundation, either 
#    version 3 of the License, or (at your option) any later version.
#
#    Phonetisaurus is distributed in the hope that it will be useful, but 
#    WITHOUT ANY WARRANTY; without even the implied warranty of 
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
#    General Public License for more details.
#
#    You should have received a copy of the GNU General Public 
#    License along with Phonetisaurus. If not, see http://www.gnu.org/licenses/.platform='unknown'
unamestr=`uname`

if [[ "${unamestr}" == "Linux" ]];
then
    swig -c++ -python M2MFstAligner.i
    g++  -O2 -c -fPIC -I/usr/include/python2.7 M2MFstAligner_wrap.cxx M2MFstAligner.cpp FstPathFinder.cpp -lfst -ldl 
    g++  -O2 -shared -Wl,-soname,_M2MFstAligner.so -o _M2MFstAligner.so  M2MFstAligner.o FstPathFinder.o M2MFstAligner_wrap.o -lfst -ldl
elif [[ "${unamestr}" == "Darwin" ]];
then
    #May need to change these!
    pyH=/opt/local/Library/Frameworks/Python.framework/Versions/2.7/include/python2.7
    pyL=/opt/local/Library/Frameworks/Python.framework/Versions/2.7/lib
    swig -c++ -python M2MFstAligner.i
    g++  -O2 -c -fPIC -I${pyH} M2MFstAligner_wrap.cxx M2MFstAligner.cpp FstPathFinder.cpp
    g++  -O2 -L${pyL} -shared -lc -Wl,-install_name,_M2MFstAligner.so -o _M2MFstAligner.so  M2MFstAligner.o FstPathFinder.o M2MFstAligner_wrap.o -lfst -ldl -lpython2.7
fi