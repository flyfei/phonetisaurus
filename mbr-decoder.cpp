#include <fst/fstlib.h>
#include "MBRDecoder.hpp"
using namespace fst;


int main( int argc, char* argv[] ){
  //VectorFst<StdArc>* std_lattice = VectorFst<StdArc>::Read(argv[1]);
  //VectorFst<LogArc>* log_lattice = new VectorFst<LogArc>();
  //Map(*std_lattice, log_lattice, StdToLogMapper());
  VectorFst<LogArc>* log_lattice = VectorFst<LogArc>::Read(argv[1]);

  MBRDecoder         mbrdecoder( atoi(argv[2]), log_lattice );
  mbrdecoder.build_decoder( );

  return 1;
}
