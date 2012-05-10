#ifndef MBRDECODER_H
#define MBRDECODER_H
#include <fst/fstlib.h>
#include "google/dense_hash_map"
using namespace fst;

typedef SigmaMatcher<SortedMatcher<Fst<StdArc> > > SSM;
typedef SigmaMatcher<SortedMatcher<Fst<LogArc> > > SM;
typedef RhoMatcher<SM> RM;
typedef google::dense_hash_map<int, LogArc::Weight> UF;

template <class T> inline string to_string (const T& t){
  stringstream ss;
  ss << t;
  return ss.str();
}

class MBRDecoder{
  /*
    The MBRDecoder class takes a Ngram WFSA and uses this 
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
  VectorFst<StdArc>* std_lattice;
  vector<set<vector<int> > >  ngrams;
  vector<VectorFst<LogArc>* > mappers;
  vector<VectorFst<LogArc>* > pathcounters;
  vector<VectorFst<LogArc>* > latticeNs;
  vector<VectorFst<LogArc>* > omegas;

  MBRDecoder( );

  MBRDecoder( int _order,  VectorFst<LogArc>* _lattice, float _alpha, vector<float> _thetas );

  //Build all required MBR decoder components
  void build_decoder( );

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
  void add_ngram( VectorFst<LogArc>* mapper, int state, vector<int> ngram, const vector<int>* lab, bool olab );

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

  VectorFst<StdArc> count_ngrams( VectorFst<StdArc>* std_lattice, int order );

private:
  string _vec_to_string( const vector<int>* vec );

  const LogArc& _get_arc( VectorFst<LogArc>* mapper, int i, vector<int> ngram );
}; 

#endif //MBRDECODER_H
