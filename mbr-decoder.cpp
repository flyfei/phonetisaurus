#include <fst/fstlib.h>
#include "MBRDecoder.hpp"
using namespace fst;


int main( int argc, char* argv[] ){
  SymbolTable*       syms = SymbolTable::ReadText(argv[1]);
  NgramExtractor     extractor( atoi(argv[2]), syms );
  VectorFst<StdArc>* std_lattice = VectorFst<StdArc>::Read(argv[3]);
  VectorFst<LogArc>* log_lattice = new VectorFst<LogArc>();
  Map(*std_lattice, log_lattice, StdToLogMapper());
  extractor.build_decoder( log_lattice );

  return 1;
}
