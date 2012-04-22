#include "MBRDecoder.hpp"

NgramExtractor::NgramExtractor( ){
  //Default constructor
}

NgramExtractor::NgramExtractor( int _order, SymbolTable* _syms ){
  /*
    Preferred class constructor
  */
  order   = _order;
  syms    = _syms;
  sigmaID = 1;
  rhoID   = 2;
  alpha   = 1.0;

  for( int i=0; i<order+1; i++ )
    thetas.push_back(1.0); 
  ngrams.resize(order);
  lattice = new VectorFst<LogArc>();
  
  for( int i=0; i<order; i++ ){
    mappers.push_back(new VectorFst<LogArc>() );
    pathcounters.push_back(new VectorFst<LogArc>() );
    latticeNs.push_back(new VectorFst<LogArc>() );
    omegas.push_back(new VectorFst<LogArc>());
  }
}

void NgramExtractor::build_decoder( VectorFst<LogArc>* _lattice ){
  /*
    Helper function that will build all required MBR decoder components.
    This includes the following:
       * Unique lattice Ngram sets
       * Ngram Context-Dependency transducers
       * Ngram path-counting automata
       * Integrated MBR decoder cores
  */

  _alpha_normalize(_lattice);

  //Step 1. Extract all unique Ngrams up to 'order'
  extract_ngrams( lattice->Start(), vector<int>() );

  //Step 2. Construct N-gram CD FSTs from the result of 1.
  build_ngram_cd_fsts( );

  //Step 3. Construct Psi pathcounter FSTs from the result of 1.
  build_right_pathcounters( );

  //Step 4. Construct the decoders from the results of Steps 2. and 3.
  build_decoders( );

  //Step 5. Finally find the shortest path through the decoder cascade
  decode( );

  return;
}

void NgramExtractor::decode( ){
  //First map the lattice weights to \Theta_{0}
  for( StateIterator<VectorFst<LogArc> > siter(*lattice); !siter.Done(); siter.Next() ){
    size_t i = siter.Value();
    for( MutableArcIterator<VectorFst<LogArc> > aiter(lattice,i); !aiter.Done(); aiter.Next() ){
      LogArc arc = aiter.Value();
      arc.weight = thetas[0];
      aiter.SetValue(arc);
    }
  }

  Compose(*lattice, *mappers[0], omegas[0]);
  for( int i=1; i<order; i++ )
    Compose(*omegas[i-1], *mappers[i], omegas[i]);
  omegas[order-1]->Write("omega.fst");

  return;
}

void NgramExtractor::_alpha_normalize( VectorFst<LogArc>* _lattice ){
  for( StateIterator<VectorFst<LogArc> > siter(*_lattice); !siter.Done(); siter.Next() ){
    size_t i = siter.Value();
    for( MutableArcIterator<VectorFst<LogArc> > aiter(_lattice, i); !aiter.Done(); aiter.Next() ){
      LogArc arc = aiter.Value();
      arc.weight = arc.weight.Value() * alpha;
      aiter.SetValue(arc);
    }
  }

  Push<LogArc, REWEIGHT_TO_FINAL>(*_lattice, lattice, kPushWeights);
  for( StateIterator<VectorFst<LogArc> > siter(*_lattice); !siter.Done(); siter.Next() ){
    size_t i = siter.Value();
    if( _lattice->Final(i)!=LogArc::Weight::Zero() ){
      _lattice->SetFinal(i,LogArc::Weight::One());
    }
  }
  return;
}

void NgramExtractor::build_decoders( ){
  /*
    Construct the order-N lattices from the order-N
    context-dependency WFSTs and the raw input lattice.
  */
  ArcSort(lattice, OLabelCompare<LogArc>());
  lattice->SetInputSymbols(syms);
  lattice->SetOutputSymbols(syms);

  for( int i=0; i<order; i++ ){
    ArcSort(mappers[i], ILabelCompare<LogArc>());
    mappers[i]->SetInputSymbols(syms);
    mappers[i]->SetOutputSymbols(syms);

    //
    //---Generate the order-N lattice--\\
    //
    Compose(*lattice, *mappers[i], latticeNs[i]);
    Project(latticeNs[i], PROJECT_OUTPUT);
    RmEpsilon(latticeNs[i]);
    string fname = "lattice-" + to_string(i) + ".fst";
    latticeNs[i]->SetInputSymbols(syms);
    latticeNs[i]->SetOutputSymbols(syms);
    latticeNs[i]->Write(fname);

    //
    //---Compute the path-posterior N-gram probabilities---\\
    //
    pathcounters[i]->SetInputSymbols(syms);
    pathcounters[i]->SetOutputSymbols(syms);
    countPaths( pathcounters[i], latticeNs[i], i );
  }
  syms->WriteText("finalsyms.syms");
  return;
}


void NgramExtractor::countPaths( VectorFst<LogArc>* Psi, VectorFst<LogArc>* latticeN, int i ){
  /*
    Begin cascaded matcher stuff
    NOTE: Order and precedence matter!!!
  */

  //Sigma (any) matcher
  SM* sm = new SM( *Psi, MATCH_INPUT, sigmaID );
  //Rho (remaining) matcher
  RM* rm = new RM( *Psi, MATCH_INPUT, rhoID, MATCHER_REWRITE_AUTO, sm );

  ComposeFstOptions<LogArc, RM> opts;
  opts.gc_limit = 0;
  opts.matcher1 = new RM(*latticeN, MATCH_NONE,  kNoLabel);
  opts.matcher2 = rm;
  //End cascaded matcher stuff

  //Cast to normal VectorFst
  VectorFst<LogArc> result(ComposeFst<LogArc>(*latticeN, *Psi, opts));

  //Delete unconnected states and arcs
  Connect(&result);

  //Project output labels
  Project(&result, PROJECT_OUTPUT);

  //Remove epsilon arcs
  RmEpsilon(&result);

  //Determinize and minimize the result
  VectorFst<LogArc> detResult;
  Determinize( result, &detResult );
  Minimize( &detResult );
  string fname = "post-"+to_string(i)+".fst";
  detResult.SetInputSymbols(syms);
  detResult.SetOutputSymbols(syms);
  detResult.Write(fname);
  //Now set the arc weights for the decoding transducer, which is just the 
  //order-N mapping transducer where the Psi weights have been applied 
  //to all the arcs.

  for( StateIterator<VectorFst<LogArc> > siter(*mappers[i]); !siter.Done(); siter.Next() ){
    size_t id = siter.Value();
    for( MutableArcIterator<VectorFst<LogArc> > aiter(mappers[i],id); !aiter.Done(); aiter.Next() ){
      LogArc arc = aiter.Value();
      for( ArcIterator<VectorFst<LogArc> > viter(detResult,0); !viter.Done(); viter.Next() ){
	const LogArc& varc = viter.Value();
	if( arc.olabel == varc.ilabel ){
	  arc.weight = varc.weight.Value() * thetas[i+1];
	  aiter.SetValue(arc);
	}
      }
    }
  }
  Project(mappers[i], PROJECT_INPUT);

  fname = "wmapper-"+to_string(i)+".fst";
  mappers[i]->Write(fname);

  return;
}


void NgramExtractor::build_right_pathcounters( ){
  for( int i=0; i<ngrams.size(); i++ ){
    set<vector<int> >::iterator ait;
    pathcounters[i]->AddState();
    pathcounters[i]->SetStart(0);
    pathcounters[i]->AddArc(0, LogArc( 1, 0, LogArc::Weight::One(), 0 ) );
    for( ait=ngrams[i].begin(); ait!=ngrams[i].end(); ait++ ){
      int rhoid   = pathcounters[i]->AddState();
      int final   = pathcounters[i]->AddState();
      int ngramid = syms->Find(_vec_to_string(&(*ait)));
      pathcounters[i]->AddArc( 0, LogArc( ngramid, ngramid, LogArc::Weight::One(), rhoid ) );
      pathcounters[i]->SetFinal(rhoid, LogArc::Weight::One());
      pathcounters[i]->AddArc( rhoid, LogArc( 2, 0, LogArc::Weight::One(), rhoid ) );
      pathcounters[i]->AddArc( rhoid, LogArc( ngramid, 0, LogArc::Weight::One(), final ) );
    }
    string fstname = "rpathcounter-" + to_string(i) + ".fst";
    pathcounters[i]->SetInputSymbols(syms);
    pathcounters[i]->SetOutputSymbols(syms);
    pathcounters[i]->Write(fstname);
  }
  return;
}

void NgramExtractor::build_left_pathcounters( ){
  for( int i=0; i<ngrams.size(); i++ ){
    set<vector<int> >::iterator ait;
    pathcounters[i]->AddState();
    pathcounters[i]->SetStart(0);
    int final = pathcounters[i]->AddState();
    for( ait=ngrams[i].begin(); ait!=ngrams[i].end(); ait++ ){
      int rhoid   = pathcounters[i]->AddState();
      int ngramid = syms->Find(_vec_to_string(&(*ait)));
      pathcounters[i]->AddArc( 0, LogArc( 0, 0, LogArc::Weight::One(), rhoid ) );
      pathcounters[i]->AddArc( rhoid, LogArc( 2, 0, LogArc::Weight::One(), rhoid ) );
      pathcounters[i]->AddArc( rhoid, LogArc( ngramid, ngramid, LogArc::Weight::One(), final ) );
    }
    pathcounters[i]->AddArc( final, LogArc( 1, 0, LogArc::Weight::One(), final ) );

    string fstname = "lpathcounter-" + to_string(i) + ".fst";
    pathcounters[i]->SetInputSymbols(syms);
    pathcounters[i]->SetOutputSymbols(syms);
    pathcounters[i]->Write(fstname);
  }
  return;
}

void NgramExtractor::build_ngram_cd_fsts( ){
  /*
    Parent function to build the individual order-N Ngram
    context-dependency transducers.
  */
  for( int i=0; i<ngrams.size(); i++ ){
    //Build the Ngram trees from the Ngram sets
    mappers[i]->AddState();
    mappers[i]->SetStart(0);
    set<vector<int> >::iterator it;
    for( it=ngrams[i].begin(); it!=ngrams[i].end(); it++ ){
      vector<int> ngram = (*it);
      add_ngram( mappers[i], mappers[i]->Start(), ngram, &(*it) );
    }
    
    //Next, connect the final states based on the allowable 
    // transitions found in the original input word lattice.
    connect_ngram_cd_fst( mappers[i], i );

    string fstname = "mapper-cd-"+to_string(i)+".fst";
    mappers[i]->SetInputSymbols(syms);
    mappers[i]->SetOutputSymbols(syms);
    mappers[i]->Write(fstname);
  }
  return;
}

void NgramExtractor::connect_ngram_cd_fst( VectorFst<LogArc>* mapper, int i ){
  for( set<vector<int> >::iterator ait=ngrams[i].begin(); ait!=ngrams[i].end(); ait++ ){
    vector<int> ngram = (*ait);
    const LogArc& iarc = _get_arc( mapper, mapper->Start(), ngram  );
    ngram.erase(ngram.begin());
    for( set<vector<int> >::iterator bit=ngrams[0].begin(); bit!=ngrams[0].end(); bit++ ){
      string label = _vec_to_string(&ngram) + syms->Find(bit->at(0));
      if( syms->Find(label)!=SymbolTable::kNoSymbol ){
	ngram.push_back(bit->at(0));
	const LogArc& oarc = _get_arc( mapper, mapper->Start(), ngram );
	mapper->AddArc( iarc.nextstate, LogArc( oarc.ilabel, oarc.olabel, LogArc::Weight::One(), oarc.nextstate ) );
	ngram.pop_back();
      }
    }
  }

  return;
}

const LogArc& NgramExtractor::_get_arc( VectorFst<LogArc>* mapper, int i, vector<int> ngram ){
  //We are ASS-U-ME-ing there is a match.  
  //This will break if I fucked up somewhere else (highly likely).
  if( ngram.size()==1 ){
    for( ArcIterator<VectorFst<LogArc> > aiter(*mapper,i); !aiter.Done(); aiter.Next() ){
      const LogArc& arc = aiter.Value();
      if( arc.ilabel==ngram[0] ){
	return arc;
      }
    }
  }
	
  if( ngram.size()>1 ){
    for( ArcIterator<VectorFst<LogArc> > aiter(*mapper,i); !aiter.Done(); aiter.Next() ){
      LogArc arc = aiter.Value();
      if( arc.ilabel==ngram[0] ){
	ngram.erase(ngram.begin());
	return _get_arc( mapper, arc.nextstate, ngram );
      }
    }
  }

}

string NgramExtractor::_vec_to_string( const vector<int>* vec ){
  /*
    Convenience function for converting a pointer to a vector
    of ints to a string based on the input symbol table.
    Mainly used for constructing the Ngram CD transducers.
  */
  string label = "";
  for( int i=0; i<vec->size(); i++ )
    label += syms->Find(vec->at(i));
  return label;
}

void NgramExtractor::add_ngram( VectorFst<LogArc>* mapper, int state, vector<int> ngram, const vector<int>* lab ){
  /*
    Recursively add Ngrams to the order-N ngram tree.  This forms the basis for
    the order-N lattice-Ngram context-dependency transducer.
  
    The recursion is basically the same as the 'extract_ngrams' function, the 
    salient difference being that in this case we are *adding* Ngrams to a 
    partially developed, deterministic tree, while in 'extract_ngrams' we are
    ...extracting Ngrams from a complete lattice.

    This could also be achieved by building the order-N counting transducer, 
    then removing lattice weights, composing with the input lattice and optimizing
    using the Tropical semiring throughout:
            Det( RmEps( Proj_o( L * Counter ) ) )

    In practice I think this will tend to be a lot slower, especially for larger 
    lattices, but I need to analyze it properly to prove that this is the case.

    This could *also* be achieved by building linear string FSTs for each Ngram,
    then computing,
      Det( Union( [s_1, ..., s_R] ) )
    but the determinization and union operations are a little bit annoying to use.
  */
  bool found_ngram = false;

  //Try to find the next label in the current Ngram.  If we don't find it, we will 
  // add it to the Ngram tree.
  for( ArcIterator<VectorFst<LogArc> > aiter(*mapper,state); !aiter.Done(); aiter.Next() ){
    const LogArc arc = aiter.Value();
    if( arc.ilabel==ngram[0] ){
      found_ngram = true;
      ngram.erase(ngram.begin());
      add_ngram( mapper, arc.nextstate, ngram, lab );
    }
  }

  //If no matching arc was found for the current Ngram word, add a new arc to the WFST.
  if( found_ngram==false ){
    int sid = mapper->AddState();
    //If we only have one word/token left in the Ngram vector, then this is a final state.
    //At this point we will also index the arc so that we know which final nodes need 
    // extra arcs based on our available Ngram histories.
    if( ngram.size()==1 ){
      mapper->AddArc( state, LogArc( ngram[0], syms->AddSymbol(_vec_to_string(lab)), LogArc::Weight::One(), sid ) );
      mapper->SetFinal( sid, LogArc::Weight::One() );
    //If we have more than one remaining token in the Ngram vector, add a new arc and 
    // continue recursively calling 'add_ngram'.  This is not a final state.
    }else if( ngram.size()>1 ){
      mapper->AddArc( state, LogArc( ngram[0], 0, LogArc::Weight::One(), sid ) );
      ngram.erase(ngram.begin());
      add_ngram( mapper, sid, ngram, lab );
    }
  }
  
  return;
}


void NgramExtractor::extract_ngrams( int state, vector<int> ngram ){
  /*
    Recursively traverse the input lattice and extract all unique Ngrams of length >= order.
    Effectively the 'ngram' vector functions as a sort of stack with a fixed size <= order.  

    The algorithm begins by pushing the first arc label into the 'ngram' vector, then 
    recursively following the destination state of the arc and repeating this procedure.
    If 'ngram' is non-empty then each 'ngram' of length <= order is added to the set of 
    unique lattice Ngrams.  Arc labels are added to the end, and Ngrams are extracted starting
    from the end of the 'ngram' vector, effectively making this a modified FIFO stack. This
    ensures that the string/sentence-final Ngrams are extracted correctly.

    Finally, whenever pushing a new arc label onto the stack results in the stack size increasing
    to length >= order, the *bottom* label is shifted from the stack.  This ensures that 
    no Ngrams of length > order are extracted.

      Example for a linear automaton:
          Sentence = "a b a b b", Order=3
	  Step 1:  
	     - 'a' is pushed onto 'ngram'
	     * ngram = [ 'a' ]
	     - 'a' is inserted into the 1-gram set
	     * ngram = [ 'a' ]
	  Step 2:  
	     - 'b' is pushed onto 'ngram'
	     * ngram = [ 'a', 'b' ]
	     - 'b' is inserted into the 1-gram set
	     - 'ab' is inserted into the 2-gram set
	     * ngram = [ 'a', 'b' ]
	  Step 3: 
	     - 'a' is pushed onto 'ngram'
	     * ngram = [ 'a', 'b', 'a' ]
	     - 'a' is ignored since it has already been added to the 1-gram set
	     - 'ba' is inserted into the 2-gram set
	     - 'aba' is inserted into the 3-gram set
	     - 'a' is shifted from the bottom of the ngram vector/stack
	     * ngram = [ 'b', 'a' ]
	  Step 4:
	     - 'b' is pushed onto 'ngram'
	     * ngram = [ 'b', 'a', 'b' ]
	     - 'b' is ignored since it has already been added to the 1-gram set
	     - 'ab' is ignored since it has already been added to the 2-gram set
	     - 'bab' is inserted into the 3-gram set
	     - 'b' is shifted from the bottom of the ngram vector/stack
	     * ngram = [ 'a', 'b' ]
	  Step 5:
	     - 'b' is pushed onto 'ngram'
	     * ngram = [ 'a', 'b', 'b' ]
	     - 'b' is ignored since it has already been added to the 1-gram set
	     - 'bb' is inserted into the 2-gram set
	     - 'abb' is inserted into the 3-gram set
	     - 'a' is shifted from the bottom of the ngram vector/stack
	     * ngram = [ 'b', 'b' ]

	   +-+ Algorithm terminates, having collected all unique Ngrams of order<=3
	     { 1:['a','b'], 2:['ab','ba','bb'], 3:['aba','bab',abb'] }

    Although it currently does not, this function could be easily augmented to also accumulate
    counts, which should be *MUCH* more efficient than using the pure WFST-based counting 
    methods.

    The vector of Ngram sets that this function computes is used for two purposes:
          1.)  To subsequently construct order-specific context-dependency 
	       transducers which in turn are used to compile the order-specific 
	       hierarchical MBR decoders for hypothesis lattices
	  2.)  To subsequently construct order-specific path-counting automata which 
	       are utilized to estimate the posterior lattice Ngram probabilities,
	          p( u_{n,i} | \mathcal{E}_{n} )
	       where u_{n,i} refers to a particular Ngram of order 'n' with label 
	       identity 'i', and \mathcal{E}_{n} refers to the order-n lattice.
  */

  //If the ngram vector is non-empty, extract all Ngrams and add them to the 
  // set of unique Ngrams.  Any Ngrams that have already been found will be ignored.
  if( ngram.size()>0 )
    for( int i=ngram.size(); i>=0; i-- )
      for( int j=i; j<ngram.size(); j++ )
	ngrams[ngram.size()-j-1].insert( vector<int>(ngram.begin()+j, ngram.end()) );

  //If the size of the ngram vector/stack has grown to >= order, then pop the first/lowest
  // item currently in the vector.
  if( ngram.size()>=order )
    ngram.erase(ngram.begin());

  //Recurse through the entire lattice.  We have to make a copy of 'ngram' before each
  // call or else the ngram contents for states with more than one out-going arc will
  // be incorrect.  I'm pretty sure that despite all this the approach is still loads
  // faster and more memory and space efficient than using standard WFST operations.
  //Nevertheless it would be worth double-checking this at a later date, both analytically
  // and empirically with several of the G2P datasets.  More fodder for the journal...
  for( ArcIterator<VectorFst<LogArc> > aiter(*lattice,state); !aiter.Done(); aiter.Next() ) {
    const LogArc& arc = aiter.Value();
    vector<int> n_ngram = ngram;
    n_ngram.push_back(arc.ilabel);
    extract_ngrams( arc.nextstate, n_ngram );
  }

  return;
}

void NgramExtractor::build_mappers( ){
  
  return;
}

void NgramExtractor::build_pathcounters( bool _right ){
  if( _right==true )
    build_right_pathcounters( );
  else
    build_left_pathcounters( );
  return;
}

