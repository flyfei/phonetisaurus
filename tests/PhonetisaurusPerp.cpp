/*
 *  Phonetisaurus.cpp 
 *  
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
#include "PhonetisaurusPerp.hpp"

PhonetisaurusPerp::PhonetisaurusPerp( ) {
    //Default constructor
}

PhonetisaurusPerp::PhonetisaurusPerp( const char* _g2pmodel_file, bool _use_phi,
					bool _logopt, int _beam, int _nbest, bool _lmbr, 
					int _order, float _alpha, float _prec, 
					float _ratio, int _verbose
					){

  g2pmodel = VectorFst<StdArc>::Read( _g2pmodel_file );
  if(g2pmodel == NULL) return;
  //Initialize
  use_phi = _use_phi;
  logopt  = _logopt;
  lmbr    = _lmbr;
  beam    = _beam;
  nbest   = _nbest;
  order   = _order;
  alpha   = _alpha;
  prec    = _prec;
  ratio   = _ratio;
  verbose = _verbose;
  //Have to make a copy in case we encode the model
  isyms = *g2pmodel->InputSymbols();
  osyms = *g2pmodel->OutputSymbols();

  tie  = isyms.Find(1); //The separator symbol is reserved for index 1
  skip = isyms.Find(2); //The del/ins character is reserved for index 2
  encoder = new EncodeMapper<StdArc>(kEncodeLabels, ENCODE);

  _load_clusters( );
  _load_oclusters( );
  _make_filter( );
  _make_ofilter( );
  if( use_phi==true )
    _phi_ify( );

  ArcSort( g2pmodel, ILabelCompare<StdArc>() );
}

VectorFst<StdArc> PhonetisaurusPerp::phoneticize( vector<string>* tokens, vector<vector<string> >* p_tokens ){
  /*                                                                       
    Compose with phi (failure) transitions.                                
  */

  //Convert the token sequence into an appropriate FSM
  cerr << "Convert entry" << endl;
  _entry_to_fsm( tokens );
  word.SetOutputSymbols(&osyms);
  //We need to filter the lattice to contain ONLY valid
  // reference pronunciations.  We do this by composition
  // with an FSA representation of the set of valid pronunciations.
  //Note that there may be more than one valid pronunciation!!
  cerr << "Convert prons... " << endl;
  _prons_to_fsm( p_tokens );
  prons.SetInputSymbols(&osyms);
  prons.SetOutputSymbols(&osyms);
  ArcSort(&prons,ILabelCompare<StdArc>());
  word = VectorFst<StdArc>(ComposeFst<StdArc>(word,prons));
  RmEpsilon(&word);

  //If using failure transitions we have to encode the result
  Encode(&word, encoder);
  ComposeFstOptions<StdArc, PM> opts;
  opts.gc_limit = 0;
  opts.matcher1 = new PM(word, MATCH_NONE, kNoLabel);
  //The encoder will map the backoff <eps>:<eps> pair to ID=1
  opts.matcher2 = new PM(*g2pmodel,   MATCH_INPUT, 1);

  word = VectorFst<StdArc>(ComposeFst<StdArc>(word, *g2pmodel, opts));
  //Now decode back to input-output symbols
  Decode(&word, *encoder);
  word.SetOutputSymbols(&osyms);

  //Now find the shortest path
  VectorFst<StdArc>* shortest = new VectorFst<StdArc>();
  ShortestPath( word, shortest, 1 );
  word = *shortest;
  
  word.SetInputSymbols(&isyms);
  word.SetOutputSymbols(&osyms);
  return word;
}

void PhonetisaurusPerp::_entry_to_fsm( vector<string>* tokens ){
  /*
    Convert an input grapheme sequence to an equivalent
    Finite-State Machine.
  */

  //Build a linear FSA representing the input
  _entry_to_skip_fsa( tokens );

  //Add any multi-grapheme arcs
  word = VectorFst<StdArc>(ComposeFst<StdArc>(word,filter));
  Project(&word, PROJECT_OUTPUT);

  //If using phi transitions we need to build and encode
  // the word as a special transducer.  
  cerr << "Trying to compose with loop" << endl;
  if( use_phi==true )
    word = VectorFst<StdArc>(ComposeFst<StdArc>(word,*loop));
  cerr << "Successfully composed with loop" << endl;
  //Now optimize the result
  RmEpsilon(&word);

  return;
}

void PhonetisaurusPerp::_prons_to_fsm( vector<vector<string> >* p_tokens ){
  /*
    Create a single FSA representing the union of all valid pronunciations
    for the given input word.
  */
  prons = VectorFst<StdArc>();
  prons.AddState();
  prons.SetStart(0);
  prons.SetInputSymbols(&osyms);
  prons.SetOutputSymbols(&osyms);
  for( int i=0; i<p_tokens->size(); i++ ){
    Union( &prons, _pron_to_fsm( &p_tokens->at(i) ) );
  }

  RmEpsilon(&prons);
  return;
}

VectorFst<StdArc> PhonetisaurusPerp::_pron_to_fsm( vector<string>* tokens ){
  /*
    Convert an input grapheme sequence to an equivalent
    Finite-State Machine.
  */

  //Build a linear FSA representing the input
  VectorFst<StdArc> pron;
  pron.AddState();
  pron.SetStart(0);
  int skip_id = 2;
  size_t i=0;
  for( i=0; i<tokens->size(); i++){
    pron.AddState();
    string ch = tokens->at(i);
    pron.AddArc( i,
                 StdArc(
                        osyms.Find(ch),
                        osyms.Find(ch),
                        StdArc::Weight::One(), i+1
			)
                 );
    pron.AddArc( i, StdArc( skip_id, skip_id, StdArc::Weight::One(), i ) );                                                                                          
  }

  pron.AddArc( i, StdArc( skip_id, skip_id, StdArc::Weight::One(), i));                                                                                              
  pron.SetFinal( i, StdArc::Weight::One() );
  pron.SetInputSymbols(&osyms);
  pron.SetOutputSymbols(&osyms);
  //Add any multi-phoneme arcs
  ArcSort(&pron, ILabelCompare<StdArc>());
  pron = VectorFst<StdArc>(ComposeFst<StdArc>(ofilter,pron));
  Project(&pron, PROJECT_INPUT);

  //Now epsilon-remove the result and return it
  RmEpsilon(&pron);

  return pron;
}
             

void PhonetisaurusPerp::_make_loop( ){
  const EncodeTable<StdArc>& table = encoder->table();
  loop = new VectorFst<StdArc>();
  loop->AddState();
  loop->SetStart(0);
  for( size_t i=2; i<table.Size(); i++ ){
    const EncodeTable<StdArc>::Tuple *t = table.Decode(i);
    loop->AddArc( 0, StdArc( t->ilabel, t->olabel, StdArc::Weight::One(), 0 ) );
  }
  loop->SetFinal(0, StdArc::Weight::One());
  ArcSort(loop, ILabelCompare<StdArc>());
  return;
}

void PhonetisaurusPerp::_load_clusters( ){
  /*
     Load the clusters file containing the list of 
     subsequences generated during multiple-to-multiple alignment
  */
  string tie = "|";

  for( size_t i = 2; i < isyms.NumSymbols(); i++ ){
    string sym = isyms.Find( i );
    if( sym.find(tie) != string::npos ){
      char* tmpstring = (char *)sym.c_str();
      char* p = strtok(tmpstring, tie.c_str());
      vector<string> cluster;
      while (p) {
        cluster.push_back(p);
        p = strtok(NULL, tie.c_str());
      }
            
      clusters.insert(pair<vector<string>, size_t>(cluster, i));
    }
  }
  return;
}

void PhonetisaurusPerp::_load_oclusters( ){
  /*
     Load the clusters file containing the list of 
     subsequences generated during multiple-to-multiple alignment
  */
  string tie = "|";

  for( size_t i = 2; i < osyms.NumSymbols(); i++ ){
    string sym = osyms.Find( i );
    if( sym.find(tie) != string::npos ){
      char* tmpstring = (char *)sym.c_str();
      char* p = strtok(tmpstring, tie.c_str());
      vector<string> cluster;
      while (p) {
        cluster.push_back(p);
        p = strtok(NULL, tie.c_str());
      }
            
      oclusters.insert(pair<size_t, vector<string> >(i, cluster));
    }
  }
  return;
}

//STEP 1: Create a linear FSA with skip loops
void PhonetisaurusPerp::_entry_to_skip_fsa( vector<string>* tokens ){
  word = VectorFst<StdArc>();
  word.AddState();
  word.SetStart(0);

  size_t i=0;
  for( i=0; i<tokens->size(); i++){
    word.AddState();
    string ch = tokens->at(i);
    word.AddArc( i, 
		 StdArc( 
			isyms.Find(ch), 
			isyms.Find(ch), 
			StdArc::Weight::One(), i+1 
			 )
		 );
    //word.AddArc( i, StdArc( skip_id, skip_id, StdArc::Weight::One(), i ) );
  }

  //word.AddArc( i, StdArc( skip_id, skip_id, StdArc::Weight::One(), i));
  word.SetFinal( i, StdArc::Weight::One() );

  return;
}

//STEP 2: Create a filter, which adds multi-token links and skip support
void PhonetisaurusPerp::_make_filter( ){
  /*
    Create a filter FST.  This will map arcs in the linear 
    input FSA to longer clusters wherever appropriate. A more
    advanced version can be used to also place restrictions on
    how many phoneme insertions to allow, or how to penalize them.
  */
  filter.AddState();
  filter.SetStart(0);
  for( size_t j=2; j<isyms.NumSymbols(); j++ ){
    filter.AddArc( 0, StdArc( j, j, StdArc::Weight::One(), 0 ) );
  }

  typedef map<vector<string>, size_t>::iterator cl_iter;
  size_t k = 1;
  for( cl_iter it=clusters.begin(); it != clusters.end(); it++){
    filter.AddState();
    filter.AddArc( 0, StdArc( isyms.Find(it->first.at(0)), it->second, StdArc::Weight::One(), k ) );
    filter.AddArc( k, StdArc( isyms.Find(it->first.at(1)), 0, StdArc::Weight::One(), 0 ) );
    k++;
  }
  filter.SetFinal( 0, StdArc::Weight::One() );

  return;
}

void PhonetisaurusPerp::_make_ofilter( ){
  /*
    Create an output filter FST.  This maps phoneme clusters
    back to phonemes as a final step.  Faster to do this manually 
    when we just want the output.  Convenient if we want the FST version.
  */
  ofilter.AddState();
  ofilter.SetStart(0);
  typedef map<size_t, vector<string> >::iterator cl_iter;
  cl_iter jt;
  for( size_t j=1; j<osyms.NumSymbols(); j++ ){
    if( oclusters.find(j)==oclusters.end() )
      ofilter.AddArc( 0, StdArc( j, j, StdArc::Weight::One(), 0 ) );
  }

  size_t k = 1;
  for( cl_iter it=oclusters.begin(); it != oclusters.end(); it++){
    ofilter.AddState();
    ofilter.AddArc( 0, StdArc( it->first, osyms.Find(it->second.at(0)), StdArc::Weight::One(), k ) );
    ofilter.AddArc( k, StdArc( 0, osyms.Find(it->second.at(1)), StdArc::Weight::One(), 0 ) );
    k++;
  }
  ofilter.SetFinal( 0, StdArc::Weight::One() );
  ofilter.SetInputSymbols(&osyms);
  ofilter.SetOutputSymbols(&osyms);
  ArcSort(&ofilter,ILabelCompare<StdArc>());
  return;
}


/*  PHI-IFY ALTERNATIVE */
void PhonetisaurusPerp::_phi_ify( ){
  for( StateIterator<VectorFst<StdArc> > siter(*g2pmodel); !siter.Done(); 
       siter.Next() ){
    StdArc::StateId st = siter.Value();
    if( g2pmodel->Final(st)==StdArc::Weight::Zero() ){
      _get_final_bow( st );
    }
  }
}
    
StdArc::Weight PhonetisaurusPerp::_get_final_bow( StdArc::StateId st ){
  if( g2pmodel->Final(st) != StdArc::Weight::Zero() )
    return g2pmodel->Final(st);

  for( ArcIterator<VectorFst<StdArc> > aiter(*g2pmodel,st); 
       !aiter.Done(); 
       aiter.Next() ){
    StdArc arc = aiter.Value();
    //Assume there is never more than 1 <eps> transition
    if( arc.ilabel == 0 ){
      StdArc::Weight w = Times(arc.weight, _get_final_bow(arc.nextstate));
      g2pmodel->SetFinal(st, w);
      return w;
    }
  }

  //Should never reach this place
  return StdArc::Weight::Zero();
}
