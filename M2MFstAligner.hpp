#ifndef M2MFSTALIGNER_H
#define M2MFSTALIGNER_H
/*
 *  M2MFstAligner.hpp 
 *  
 */
#include <fst/fstlib.h>
#include <vector>
#include "M2MFstPathFinder.hpp"
using namespace fst;

class M2MFstAligner {
  /*
    Read in pairs of sequences of the form SEQ1 and SEQ2 and 
     transform them into an FST that encodes all possible 
     alignments between the symbols in the two sequences.
    Note that this may include a combination of multi-symbol 
     subsequences depending on user specifications.

    This is achieved using the following steps:
     1. Generate 'FSA1' encoding all possible sequences,
         through SEQ1, including any multi-symbol subsequences.
     2. Generate 'FSA2' encoding all possible sequences,
         through SEQ2, including any multi-symbol subsequences.
     3. Generate a single-state loop FST 'M' encoding all 
         possible symbol transductions between the component
         symbols of SEQ1 and SEQ2, including any subsequences.
     4. Compute the alignment FST, 
           aFST = compose(compose(FSA1,M),FSA2)

    The user may optionally specify whether to allow deletions 
     for SEQ1 or SEQ2, as well as a maximum subsequence length 
     for each sequence.  
  */
public: 
  //Basics declarations
  bool   seq1_del;
  bool   seq2_del;
  int    seq1_max;
  int    seq2_max;
  string seq1_sep;
  string seq2_sep;
  string s1s2_sep;
  string eps;
  string skip;
  vector<LogWeight> alpha, beta;
  //This will be used during decoding to clean the paths
  set<string>   skipSeqs;
  //OpenFst stuff
  //These will be overwritten after each FST construction
  vector<VectorFst<LogArc> > fsas; 
  //This will be maintained for the life of object
  //These symbol tables will be maintained entire life of 
  // the object.  This will ensure that any resulting 'corpus' 
  // shares the same symbol tables.
  SymbolTable *isyms;
  map<LogArc::Label, LogWeight> alignment_model;
  map<LogArc::Label, LogWeight> prev_alignment_model;
  LogWeight total;
  LogWeight prevTotal;

  //Constructors
  M2MFstAligner( );
  M2MFstAligner( bool _seq1_del, bool _seq2_del, int _seq1_max, int _seq2_max, 
		 string _seq1_sep, string _seq2_sep, string _s1s2_sep,
		 string _eps, string _skip );

  //Transform a sequence pair into an equivalent multiple-to-multiple FST,
  // encoding all possible alignments between the two sequences
  void Sequences2FST( VectorFst<LogArc>* fst, vector<string>* seq1, vector<string>* seq2 );
  //Initialize all of the training data
  void entry2alignfst( vector<string> seq1, vector<string> seq2 );
  //The expectation routine
  void expectation( );
  //The maximization routine
  void maximization( bool lastiter );
  //Print out the EM-optimized alignment for the training data
  void write_alignment( VectorFst<StdArc>& fst, int nbest );
  //max routine
  int get_max_length( string joint_label );
};
#endif // M2MFSTALIGNER_H //
