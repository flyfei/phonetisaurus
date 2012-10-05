/*
 M2MFstAligner.cpp 

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
#include "M2MFstAligner.hpp"



M2MFstAligner::M2MFstAligner( ){
  //Default constructor
}

M2MFstAligner::M2MFstAligner( bool _seq1_del, bool _seq2_del, int _seq1_max, int _seq2_max, 
			      string _seq1_sep, string _seq2_sep, string _s1s2_sep, 
			      string _eps, string _skip, bool _penalize ){
  //Base constructor.  Determine whether or not to allow deletions in seq1 and seq2
  // as well as the maximum allowable subsequence size.
  seq1_del = _seq1_del;
  seq2_del = _seq2_del;
  seq1_max = _seq1_max;
  seq2_max = _seq2_max;
  seq1_sep = _seq1_sep;
  seq2_sep = _seq2_sep;
  s1s2_sep = _s1s2_sep;
  penalize = _penalize;
  eps      = _eps;
  skip     = _skip;
  skipSeqs.insert(eps);
  isyms = new SymbolTable("syms");
  //Add all the important symbols to the table.  We can store these 
  // in the model that we train and then attach them to the fst model
  // if we want to use it later on. 
  //Thus, in addition to eps->0, we reserve symbol ids 1-4 as well.
  isyms->AddSymbol(eps);
  isyms->AddSymbol(skip);
  //The '_' as a separator here is dangerous
  isyms->AddSymbol(seq1_sep+"_"+seq2_sep);
  isyms->AddSymbol(s1s2_sep);
  string s1_del_str = seq1_del ? "true" : "false";
  string s2_del_str = seq2_del ? "true" : "false";
  string s1_max_str = itoas(seq1_max);
  string s2_max_str = itoas(seq2_max);
  string model_params = s1_del_str + "_" + s2_del_str + "_" + s1_max_str + "_" + s2_max_str;
  isyms->AddSymbol( model_params );
  total     = LogWeight::Zero();
  prevTotal = LogWeight::Zero();
  penalties.set_empty_key(NULL);
}

M2MFstAligner::M2MFstAligner( string _model_file ){
  /*
    Initialize the aligner with a previously trained model.
    The model requires that the first several symbols in the 
    symbols table contain the separator and other bookkeeping info.
  */
  VectorFst<LogArc>* model = VectorFst<LogArc>::Read( _model_file );
  for( StateIterator<VectorFst<LogArc> > siter(*model); !siter.Done(); siter.Next() ){
    LogArc::StateId q = siter.Value();
    for( ArcIterator<VectorFst<LogArc> > aiter(*model, q); !aiter.Done(); aiter.Next() ){
      const LogArc& arc = aiter.Value();
      alignment_model.insert( pair<LogArc::Label,LogWeight>(arc.ilabel,arc.weight) );
    }
  }      
  isyms = (SymbolTable*)model->InputSymbols();
  int i = 0;
  eps      = isyms->Find(i);//Can't write '0' here for some reason...
  skip     = isyms->Find(1);
  string tie = "_"; //tie to pack parameters

  string sseps = isyms->Find(2);
  vector<string> seps = tokenize_utf8_string( &sseps, &tie );
  seq1_sep = seps[0];
  seq2_sep = seps[1];
  s1s2_sep = isyms->Find(3);

  string sparams = isyms->Find(4);
  vector<string> params = tokenize_utf8_string( &sparams, &tie );
  seq1_del = params[0].compare("true") ? false : true;
  seq2_del = params[1].compare("true") ? false : true;
  seq1_max = atoi(params[2].c_str());
  seq2_max = atoi(params[3].c_str());

}

void M2MFstAligner::write_model( string _model_file ){
  /*
    Write the alignment model to disk using a single-state WFSA.
  */
  VectorFst<LogArc> model;
  model.AddState();
  model.SetStart(0);
  model.SetFinal(0,LogWeight::One());
  map<LogArc::Label,LogWeight>::iterator it;
  for( it=alignment_model.begin(); it != alignment_model.end(); it++ )
    model.AddArc( 0, LogArc( (*it).first, (*it).first, (*it).second, 0 ) );
  model.SetInputSymbols(isyms);
  model.Write(_model_file);
  return;
}

void M2MFstAligner::expectation( ){
  /*
    Here we compute the arc posteriors.  This routine is almost 
    fun to implement in the FST paradigm.
  */
  for( int i=0; i<fsas.size(); i++ ){
    //Comput Forward and Backward probabilities
    ShortestDistance( fsas.at(i), &alpha );
    ShortestDistance( fsas.at(i), &beta, true );

    //Compute the normalized Gamma probabilities and 
    // update our running tally
    for( StateIterator<VectorFst<LogArc> > siter(fsas.at(i)); !siter.Done(); siter.Next() ){
      LogArc::StateId q = siter.Value();
      for( ArcIterator<VectorFst<LogArc> > aiter(fsas.at(i),q); !aiter.Done(); aiter.Next() ){
	const LogArc&      arc = aiter.Value();
	const LogWeight& gamma = Divide(Times(Times(alpha[q], arc.weight), beta[arc.nextstate]), beta[0]); 
	//Check for any BadValue results, otherwise add to the tally.
        //We call this 'prev_alignment_model' which may seem misleading, but
        // this conventions leads to 'alignment_model' being the final version.
	if( gamma.Value()==gamma.Value() ){
	  prev_alignment_model[arc.ilabel] = Plus(prev_alignment_model[arc.ilabel], gamma);
	  total = Plus(total, gamma);
	}
      }
    }
    alpha.clear();
    beta.clear();
  }
}


float M2MFstAligner::maximization( bool lastiter ){
  //Maximization. Standard approach is simple count normalization.  
  //The 'penalize' option penalizes links by total length.  Results seem to be inconclusive.
  //  Probably get an improvement by distinguishing between gaps and insertions, etc.
  map<LogArc::Label,LogWeight>::iterator it;
  float change = abs(total.Value()-prevTotal.Value());
  //cout << "Total: " << total << " Change: " << abs(total.Value()-prevTotal.Value()) << endl;
  prevTotal = total;

  //Normalize and iterate to the next model.  We apply it dynamically 
  // during the expectation step.
  for( it=prev_alignment_model.begin(); it != prev_alignment_model.end(); it++ ){
    alignment_model[(*it).first] = Divide((*it).second,total);
    (*it).second = LogWeight::Zero();
  }

  for( int i=0; i<fsas.size(); i++ ){
    for( StateIterator<VectorFst<LogArc> > siter(fsas[i]); !siter.Done(); siter.Next() ){
      LogArc::StateId q = siter.Value();
      for( MutableArcIterator<VectorFst<LogArc> > aiter(&fsas[i], q); !aiter.Done(); aiter.Next() ){
	LogArc arc = aiter.Value();
	if( penalize==true )
	  arc.weight = alignment_model[arc.ilabel].Value() * penalties[arc.ilabel].tot;
	else
	  arc.weight = alignment_model[arc.ilabel];
	aiter.SetValue(arc);
      }
    }
  }

  total = LogWeight::Zero();
  return change;
}


void M2MFstAligner::Sequences2FST( VectorFst<LogArc>* fst, vector<string>* seq1, vector<string>* seq2 ){
  /*
    Build an FST that represents all possible alignments between seq1 and seq2, given the 
     parameter values input by the user.  Here we encode the input and output labels, in fact
     creating a WFSA.  This simplifies the training process, but means that we can only 
     easily compute a joint maximization.  In practice joint maximization seems to give the 
     best results anyway, so it probably doesn't matter.

    Note: this also performs the initizization routine.  It performs a UNIFORM initialization
     meaning that every non-null alignment sequence is eventually initialized to 1/Num(unique_alignments).
     It might be more appropriate to consider subsequence length here, but for now we stick 
     to the m2m-aligner approach.

    TODO: Add an FST version and support for conditional maximization.  May be useful for languages
     like Japanese where there is a distinct imbalance in the seq1->seq2 length correspondences.
  */
  int istate=0; int ostate=0;
  for( int i=0; i<=seq1->size(); i++ ){
    for( int j=0; j<=seq2->size(); j++ ){
      fst->AddState();
      istate = i*(seq2->size()+1)+j;

      //Epsilon arcs for seq1
      if( seq1_del==true )
	for( int l=1; l<=seq2_max; l++ ){
	  if( j+l<=seq2->size() ){
	    vector<string> subseq2( seq2->begin()+j, seq2->begin()+j+l );
	    int is = isyms->AddSymbol(skip+s1s2_sep+vec2str(subseq2, seq2_sep));
	    ostate = i*(seq2->size()+1) + (j+l);
	    //LogArc arc( is, is, LogWeight::One().Value()*(l+1)*2, ostate );
	    LogArc arc( is, is, 99, ostate );
	    //LogArc arc( is, is, LogWeight::Zero(), ostate );
	    fst->AddArc( istate, arc );
	    if( prev_alignment_model.find(arc.ilabel)==prev_alignment_model.end() )
	      prev_alignment_model.insert( pair<LogArc::Label,LogWeight>(arc.ilabel,arc.weight) );
	    else
	      prev_alignment_model[arc.ilabel] = Plus(prev_alignment_model[arc.ilabel],arc.weight);
	    total = Plus( total, arc.weight );
	  }
	}

      //Epsilon arcs for seq2
      if( seq2_del==true )
	for( int k=1; k<=seq1_max; k++ ){
	  if( i+k<=seq1->size() ){
	    vector<string> subseq1( seq1->begin()+i, seq1->begin()+i+k );
	    int is = isyms->AddSymbol(vec2str(subseq1, seq1_sep)+s1s2_sep+skip);
	    ostate = (i+k)*(seq2->size()+1) + j;
	    //LogArc arc( is, is, LogWeight::One().Value()*(k+1)*2, ostate );
	    LogArc arc( is, is, 99, ostate );
	    //LogArc arc( is, is, LogWeight::Zero(), ostate );
	    fst->AddArc( istate, arc );
	    if( prev_alignment_model.find(arc.ilabel)==prev_alignment_model.end() )
	      prev_alignment_model.insert( pair<LogArc::Label,LogWeight>(arc.ilabel,arc.weight) );
	    else
	      prev_alignment_model[arc.ilabel] = Plus(prev_alignment_model[arc.ilabel],arc.weight);
	    total = Plus(total, arc.weight);
	  }
	}

      //All the other arcs
      for( int k=1; k<=seq1_max; k++ ){
	for( int l=1; l<=seq2_max; l++ ){
	  if( i+k<=seq1->size() && j+l<=seq2->size() ){
	    vector<string> subseq1( seq1->begin()+i, seq1->begin()+i+k );
	    string s1 = vec2str(subseq1, seq1_sep);
	    vector<string> subseq2( seq2->begin()+j, seq2->begin()+j+l );
	    string s2 = vec2str(subseq2, seq2_sep);
	    //This says only 1-M and N-1 allowed, no M-N links!
	    //if( l>1 && k>1)
	    //  continue;
	    int is = isyms->AddSymbol(s1+s1s2_sep+s2);
	    ostate = (i+k)*(seq2->size()+1) + (j+l);
	    LogArc arc( is, is, LogWeight::One().Value()*(k+l), ostate );
	    //LogArc arc( is, is, LogWeight::One().Value(), ostate );
	    fst->AddArc( istate, arc );
	    //During the initialization phase, just count non-eps transitions
	    //We currently initialize to uniform probability so there is also 
            // no need to tally anything here.
	    if( prev_alignment_model.find(arc.ilabel)==prev_alignment_model.end() )
	      prev_alignment_model.insert( pair<LogArc::Label,LogWeight>(arc.ilabel, arc.weight) );
	    else
	      prev_alignment_model[arc.ilabel] = Plus(prev_alignment_model[arc.ilabel],arc.weight);
	    total = Plus( total, arc.weight );
	  }
	}
      }

    }
  }

  fst->SetStart(0);
  fst->SetFinal( ((seq1->size()+1)*(seq2->size()+1))-1, LogWeight::One() );
  //Unless seq1_del==true && seq2_del==true we will have unconnected states
  // thus we need to run connect to clean out these states
  if( seq1_del==false or seq2_del==false )
    Connect(fst);
  return;
}

void M2MFstAligner::Sequences2FSTNoInit( VectorFst<LogArc>* fst, vector<string>* seq1, vector<string>* seq2 ){
  /*
    Build an FST that represents all possible alignments between seq1 and seq2, given the 
     parameter values input by the user.  Here we encode the input and output labels, in fact
     creating a WFSA.  This simplifies the training process, but means that we can only 
     easily compute a joint maximization.  In practice joint maximization seems to give the 
     best results anyway, so it probably doesn't matter.
  */
  int istate=0; int ostate=0;
  for( int i=0; i<=seq1->size(); i++ ){
    for( int j=0; j<=seq2->size(); j++ ){
      fst->AddState();
      istate = i*(seq2->size()+1)+j;

      //Epsilon arcs for seq1
      if( seq1_del==true )
	for( int l=1; l<=seq2_max; l++ ){
	  if( j+l<=seq2->size() ){
	    vector<string> subseq2( seq2->begin()+j, seq2->begin()+j+l );
	    int is = isyms->Find(skip+s1s2_sep+vec2str(subseq2, seq2_sep));
	    ostate = i*(seq2->size()+1) + (j+l);
	    //LogArc arc( is, is, LogWeight::One().Value()*(l+1)*2, ostate );
	    LogArc arc( is, is, 99, ostate );
	    fst->AddArc( istate, arc );
	  }
	}

      //Epsilon arcs for seq2
      if( seq2_del==true )
	for( int k=1; k<=seq1_max; k++ ){
	  if( i+k<=seq1->size() ){
	    vector<string> subseq1( seq1->begin()+i, seq1->begin()+i+k );
	    int is = isyms->Find(vec2str(subseq1, seq1_sep)+s1s2_sep+skip);
	    ostate = (i+k)*(seq2->size()+1) + j;
	    //LogArc arc( is, is, LogWeight::One().Value()*(k+1)*2, ostate );
	    LogArc arc( is, is, 99, ostate );
	    fst->AddArc( istate, arc );
	  }
	}

      //All the other arcs
      for( int k=1; k<=seq1_max; k++ ){
	for( int l=1; l<=seq2_max; l++ ){
	  if( i+k<=seq1->size() && j+l<=seq2->size() ){
	    vector<string> subseq1( seq1->begin()+i, seq1->begin()+i+k );
	    string s1 = vec2str(subseq1, seq1_sep);
	    vector<string> subseq2( seq2->begin()+j, seq2->begin()+j+l );
	    string s2 = vec2str(subseq2, seq2_sep);
	    if( l>1 && k>1)
	      continue;
	    int is = isyms->Find(s1+s1s2_sep+s2);
	    ostate = (i+k)*(seq2->size()+1) + (j+l);
	    LogArc arc( is, is, LogWeight::One().Value()*(k+l), ostate );
	    fst->AddArc( istate, arc );
	  }
	}
      }

    }
  }

  fst->SetStart(0);
  fst->SetFinal( ((seq1->size()+1)*(seq2->size()+1))-1, LogWeight::One() );
  //Unless seq1_del==true && seq2_del==true we will have unconnected states
  // thus we need to run connect to clean out these states
  if( seq1_del==false or seq2_del==false )
    Connect(fst);
  return;
}

//Build the composed alignment FST and add it to the list of training data
void M2MFstAligner::entry2alignfst( vector<string> seq1, vector<string> seq2 ){
  VectorFst<LogArc> fst;
  Sequences2FST( &fst, &seq1, &seq2 );
  fsas.push_back(fst);
  return;
}

vector<PathData> M2MFstAligner::entry2alignfstnoinit( vector<string> seq1, vector<string> seq2, int nbest, string lattice ){
  VectorFst<LogArc> fst;
  Sequences2FSTNoInit( &fst, &seq1, &seq2 );
  if( lattice.compare("") != 0 )
    fst.Write(lattice);
  return write_alignment( fst, nbest );
}


vector<PathData> M2MFstAligner::write_alignment( const VectorFst<LogArc>& ifst, int nbest ){
  //Generic alignment generator
  VectorFst<StdArc> fst;
  Map( ifst, &fst, LogToStdMapper() );

  for( StateIterator<VectorFst<StdArc> > siter(fst); !siter.Done(); siter.Next() ){
    StdArc::StateId q = siter.Value();
    for( MutableArcIterator<VectorFst<StdArc> > aiter(&fst,q); !aiter.Done(); aiter.Next() ){
      //Prior to decoding we make several 'heuristic' modifications to the weights:
      // 1. A multiplier is applied to any multi-token substrings
      // 2. Any LogWeight::Zero() arc weights are reset to '99'.
      //    We are basically resetting 'Infinity' values to a 'smallest non-Infinity'
      //     so that the ShortestPath algorithm actually produces something no matter what.
      // 3. Any arcs that consist of subseq1:subseq2 being the same length and subseq1>1 
      //       are set to '99' this forces shortestpath to choose arcs where one of the
      //       following conditions holds true
      //      * len(subseq1)>1 && len(subseq2)!=len(subseq1)
      //      * len(subseq2)>1 && len(subseq1)!=len(subseq2)
      //      * len(subseq1)==len(subseq2)==1
      //I suspect these heuristics can be eliminated with a better choice of the initialization
      // function and maximization function, but this is the way that m2m-aligner works, so
      // it makes sense for our first cut implementation.
      //In any case, this guarantees that M2MFstAligner produces results identical to those 
      // produced by m2m-aligner - but with a bit more reliability.
      //UPDATE: this now produces a better alignment than m2m-aligner.  
      //  The maxl heuristic is still in place.  The aligner will produce *better* 1-best alignments
      //  *without* the maxl heuristic below, BUT this comes at the cost of producing a less 
      //  flexible corpus.  That is, for a small training corpus like nettalk, if we use the 
      //  best alignment we wind up with more 'chunks' and thus get a worse coverage for unseen 
      //  data.  Using the aignment lattices to train the joint ngram model solves this problem.
      //  Oh baby.  Can't wait to for everyone to see the paper!
      //NOTE: this is going to fail if we encounter any alignments in a new test item that never
      // occurred in the original model.
      StdArc     arc = aiter.Value();
      LabelDatum* ld = &penalties[arc.ilabel];

      if( ld->lhs>1 && ld->rhs>1 ){
        arc.weight = 999; 
      }else{
	//Penalize m-to-1 / 1-to-m links.  This produces 
	// WORSE 1-best alignments, but results in better joint n-gram 
	// models for small training corpora when using only the 1-best
	// alignment.  By further favoring 1-to-1 alignments the 1-best
	// alignment corpus results in a more flexible joint n-gram model
	// with regard to previously unseen data.  
	arc.weight = alignment_model[arc.ilabel].Value() * ld->max;
      }
      if( arc.weight == LogWeight::Zero() )
      	arc.weight = 999;
      if( arc.weight != arc.weight )
	arc.weight = 999;
      aiter.SetValue(arc);
    }
  }

  VectorFst<StdArc> shortest;
  ShortestPath( fst, &shortest, nbest );
  RmEpsilon( &shortest );
  //Skip empty results.  This should only happen
  // in the following situations:
  //  1. seq1_del=false && len(seq1)<len(seq2) 
  //  2. seq2_del=false && len(seq1)>len(seq2)
  //In both 1.and 2. the issue is that we need to 
  // insert a 'skip' in order to guarantee at least
  // one valid alignment path through seq1*seq2, but
  // user params didn't allow us to.  
  //Probably better to insert these where necessary
  // during initialization, regardless of user prefs.
  if( shortest.NumStates()==0 ){
    vector<PathData> dummy;
    return dummy;
  }
  FstPathFinder pathfinder( skipSeqs );
  pathfinder.isyms = isyms;
  pathfinder.findAllStrings( shortest );
  return pathfinder.paths;
}

void M2MFstAligner::computePenalties( ){
  /*
    Precompute exponential penalties for all possible alignment units.
    Later on this will also be where expert rules are precompiled.
  */
  map<LogArc::Label,LogWeight>::iterator it;
  for( it=prev_alignment_model.begin(); it != prev_alignment_model.end(); it++ ){
    string ioLabel = isyms->Find((*it).first);
    vector<string> InOut = tokenize_utf8_string( &ioLabel, &s1s2_sep );
    if( InOut.size()!=2 ){
      cerr << "Input: " << ioLabel << " and separator: " << s1s2_sep << " do not yield an input-output label pair!" << endl;
      cerr << "segfault ahoy!" << endl;
    }
    vector<string> inL   = tokenize_utf8_string( &InOut.at(0), &seq1_sep );    
    vector<string> outL  = tokenize_utf8_string( &InOut.at(1), &seq2_sep );    
    LabelDatum ld;
    ld.lhs  = inL.size(); ld.rhs = outL.size();
    ld.max  = max(ld.lhs,ld.rhs); ld.tot = ld.lhs+ld.rhs;
    ld.lhsE = false; ld.rhsE = false;
    penalties.insert( LabelData::value_type((*it).first, ld) );
  }
    
  return;
}

