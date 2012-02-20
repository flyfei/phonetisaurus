%module M2MFstAligner
%{
	#define SWIG_FILE_WITH_INIT
	#include "M2MFstAligner.hpp"
        #include "M2MFstPathFinder.hpp"
%}
%include "std_vector.i"
%include "std_string.i"
namespace std {
  %template(VecString) vector<string>;
  %template(VecPData)  vector<PathData>;
}
%include "M2MFstAligner.hpp"
%include "M2MFstPathFinder.hpp"
