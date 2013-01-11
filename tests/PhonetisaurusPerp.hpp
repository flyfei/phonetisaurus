/*
 PhonetisaurusPerp.hpp 

 Copyright (c) [2012-], Josef Robert Novak
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
  modification, are permitted #provided that the following conditions
  are met:

  * Redistributions of source code must retain the above copyright 
    notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above 
    copyright notice, this list of #conditions and the following 
    disclaimer in the documentation and/or other materials provided 
    with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
 COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
 INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
 STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED 
 OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/
#ifndef PHONETISAURUSPERP_H
#define PHONETISAURUSPERP_H
#include <fst/fstlib.h>
using namespace fst;
typedef PhiMatcher<SortedMatcher<Fst<StdArc> > > PM;

class PhonetisaurusPerp {
    /*
     Load a G2P/P2G model and compute the perplexity for
     a given test word or test set.
     In this case the process is complicated by two factors
     not normally present in this situation for a normal 
     language model and set of 'sentences'.
       1. The input word may have multiple valid pronunciations.
       2. There may be multiple combinations of joint n-grams
          for each pronunciation, given the model parameters.
           - For example:  ABBY -> AE B IY
                 A:AE B:B B:_ Y:IY
		 A:AE B|B:B Y:IY

     As a first cut we approximate this by extracting the 
     shortest path through the set of valid reference pronunciations,
     and then use this to compute the perplexity in the normal way.
    */

public:
  string    skip;
  string    tie;
  bool      use_phi;
  bool      logopt;
  bool      lmbr;
  int       beam;
  int       nbest;
  int       verbose;
  set<int>  skipSeqs;
  int       order;
  float     alpha;
  float     prec;
  float     ratio;
  vector<float> thetas;
  map<vector<string>, size_t>   clusters;
  map<size_t, vector<string> >   oclusters;
  EncodeMapper<StdArc>* encoder;
  VectorFst<StdArc>*    g2pmodel;
  VectorFst<StdArc>*    loop;
  VectorFst<StdArc>     filter;
  VectorFst<StdArc>     ofilter;
  VectorFst<StdArc>     word;
  VectorFst<StdArc>     prons;
  SymbolTable           isyms;
  SymbolTable           osyms;
        
  PhonetisaurusPerp( );
        
  PhonetisaurusPerp( const char* _g2pmodel_file, bool _use_phi, 
		      bool _logopt, int _beam, int _nbest, bool _lmbr, 
		      int _order, float _alpha, float _prec, 
		      float _ratio, int _verbose );

  VectorFst<StdArc> phoneticize( vector<string>* tokens, vector<vector<string> >* p_tokens );

  void _entry_to_fsm( vector<string>* tokens );
  VectorFst<StdArc> _pron_to_fsm( vector<string>* tokens );

  void _entry_to_skip_fsa( vector<string>* tokens );
  void _prons_to_fsm( vector<vector<string> >* p_tokens );
private:
  void _make_loop( );
  void _make_filter( );
  void _make_ofilter( );
  void _load_clusters( );
  void _load_oclusters( );

  void _phi_ify( );
  StdArc::Weight _get_final_bow( StdArc::StateId st );
};

#endif // PHONETISAURUSPERP_H //
