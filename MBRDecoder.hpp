#ifndef NGRAMEXTRACTOR_H
#define NGRAMEXTRACTOR_H
#include <fst/fstlib.h>
using namespace fst;

typedef SigmaMatcher<SortedMatcher<Fst<LogArc> > > SM;
typedef RhoMatcher<SM> RM;

template <class T> inline string to_string (const T& t){
  stringstream ss;
  ss << t;
  return ss.str();
}

class NgramExtractor{
  /*
    The NgramExtractor class takes a Ngram WFSA and uses this 
    to build a series of WFST objects that form the basis of a
    Minimum Bayes-Risk decoder.

    Although this is designed for an Ngram WFSA, the class will
    work with an arbitrary input WFSA lattice.
  */
public:
  //The maximum Ngram order to consider when 
  // when constructing the generic MBR decoder
  int order;
  //The following symbol IDs are reserved:
  //   Epsilon: 0  (Null)
  //     Sigma: 1  (Any)
  //       Rho: 2  (Else/Otherwise)
  //       Phi: 3  (Failure)
  int sigmaID;
  int rhoID;
  float alpha;
  vector<float> thetas;
  SymbolTable* syms;
  VectorFst<LogArc>* lattice;
  vector<set<vector<int> > >  ngrams;
  vector<VectorFst<LogArc>* > mappers;
  vector<VectorFst<LogArc>* > pathcounters;
  vector<VectorFst<LogArc>* > latticeNs;
  vector<VectorFst<LogArc>* > omegas;

  NgramExtractor( );

  NgramExtractor( int _order,  SymbolTable* _syms );

  //Build all required MBR decoder components
  void build_decoder( VectorFst<LogArc>* _lattice );

  void build_decoders( );

  void decode();
  //Connect final states in the CD FSTs as needed
  void connect_ngram_cd_fst( VectorFst<LogArc>* mapper, int i );

  //Build a context-dependency Ngram transducer
  void build_ngram_cd_fsts( );

  //Build right path counter FSTs
  void build_right_pathcounters( );
  void build_left_pathcounters( );

  //Add an Ngram to a tree transducer
  void add_ngram( VectorFst<LogArc>* mapper, int state, vector<int> ngram, const vector<int>* lab );

  //Extract all unique Ngrams of order<=order
  void extract_ngrams( int state, vector<int> ngram );

  //Build the mapper FSTs from the set of Ngram mappers
  //One will be built for each Ngram order
  void build_mappers( );

  //Build the path-counting transducers
  //One will be built for each Ngram order
  void build_pathcounters( bool _right=true );

  void countPaths( VectorFst<LogArc>* Psi, VectorFst<LogArc>* latticeN, int i );

  void _alpha_normalize( VectorFst<LogArc>* _lattice );

private:
  string _vec_to_string( const vector<int>* vec );

  const LogArc& _get_arc( VectorFst<LogArc>* mapper, int i, vector<int> ngram );
}; 

#endif //NGRAMEXTRACTOR_H
