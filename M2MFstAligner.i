/*
   
    Copyright 2011-2012 Josef Robert Novak

    This file is part of Phonetisaurus.

    Phonetisaurus is free software: you can redistribute it 
    and/or modify it under the terms of the GNU General Public 
    License as published by the Free Software Foundation, either 
    version 3 of the License, or (at your option) any later version.

    Foobar is distributed in the hope that it will be useful, but 
    WITHOUT ANY WARRANTY; without even the implied warranty of 
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
    General Public License for more details.

    You should have received a copy of the GNU General Public 
    License along with Foobar. If not, see http://www.gnu.org/licenses/.
*/
%module M2MFstAligner
%{
	#define SWIG_FILE_WITH_INIT
	#include "M2MFstAligner.hpp"
        #include "FstPathFinder.hpp"
%}
%include "std_vector.i"
%include "std_string.i"
namespace std {
  %template(VecString) vector<string>;
  %template(VecPData)  vector<PathData>;
}
%include "M2MFstAligner.hpp"
%include "FstPathFinder.hpp"
