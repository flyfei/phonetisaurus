/*
 *  M2MFstAligner.cpp 
 *
 */
#include <fst/fstlib.h>
#include <iostream>
#include <set>
#include "M2MFstAligner.hpp"

//Begin Utility functions (these really need to go somewhere else
vector<string> &split(const string &s, string delim, vector<string> &elems) {
  stringstream ss(s);
  string item;
  //delim.c_str()[0] is a VERY bad thing to do
  // this will produce behavior that makes not sense
  // to the user if they try to use a multi-char delimiter
  //Actually, this is inexcusable but first things first let's
  // get everything else working properly.
  while(getline(ss, item, delim.c_str()[0])) {
    elems.push_back(item);
  }
  return elems;
}


vector<string> split(const string &s, string delim) {
  vector<string> elems;
  return split(s, delim, elems);
}


string vec2str( vector<string> vec, string sep ){
  string ss;
  for(size_t i = 0; i < vec.size(); ++i){
    if(i != 0)
      ss += sep;
    ss += vec[i];
  }
  return ss;
}

string itoas( int i ){
  std::stringstream ostring;
  ostring << i;
  return ostring.str();
}

int M2MFstAligner::get_max_length( string joint_label ){
  //We can probably make this a LOT faster...
  vector<string> parts = split( joint_label, s1s2_sep );
  vector<string> s1 = split( parts[0], seq1_sep );
  vector<string> s2 = split( parts[1], seq2_sep );
  int m = max(s1.size(),s2.size());
  //Probably want to rethink this placement...
  if( s1.size()==s2.size() && s1.size()>1 )
    m = -1;
  return m;
}
//End utility functions


M2MFstAligner::M2MFstAligner( ){
  //Default constructor
}

M2MFstAligner::M2MFstAligner( bool _seq1_del, bool _seq2_del, int _seq1_max, int _seq2_max, 
			      string _seq1_sep, string _seq2_sep, string _s1s2_sep, 
			      string _eps, string _skip ){
  //Base constructor.  Determine whether or not to allow deletions in seq1 and seq2
  // as well as the maximum allowable subsequence size.
  seq1_del = _seq1_del;
  seq2_del = _seq2_del;
  seq1_max = _seq1_max;
  seq2_max = _seq2_max;
  seq1_sep = _seq1_sep;
  seq2_sep = _seq2_sep;
  s1s2_sep = _s1s2_sep;
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
  isyms->AddSymbol(seq1_sep);
  isyms->AddSymbol(seq2_sep);
  isyms->AddSymbol(s1s2_sep);
  total     = LogWeight::Zero();
  prevTotal = LogWeight::Zero();
}

void M2MFstAligner::expectation( ){
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
	    LogArc arc( is, is, LogWeight::Zero(), ostate );
	    fst->AddArc( istate, arc );
	    if( prev_alignment_model.find(arc.ilabel)==prev_alignment_model.end() ){
	      prev_alignment_model.insert( pair<LogArc::Label,LogWeight>(arc.ilabel,arc.weight) );
	    }
	  }
	}

      //Epsilon arcs for seq2
      if( seq2_del==true )
	for( int k=1; k<=seq1_max; k++ ){
	  if( i+k<=seq1->size() ){
	    vector<string> subseq1( seq1->begin()+i, seq1->begin()+i+k );
	    int is = isyms->AddSymbol(vec2str(subseq1, seq1_sep)+s1s2_sep+skip);
	    ostate = (i+k)*(seq2->size()+1) + j;
	    LogArc arc( is, is, LogWeight::Zero(), ostate );
	    fst->AddArc( istate, arc );
	    if( prev_alignment_model.find(arc.ilabel)==prev_alignment_model.end() ){
	      prev_alignment_model.insert( pair<LogArc::Label,LogWeight>(arc.ilabel,arc.weight) );
	    }
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
	    int is = isyms->AddSymbol(s1+s1s2_sep+s2);
	    ostate = (i+k)*(seq2->size()+1) + (j+l);
	    LogArc arc( is, is, LogWeight::One(), ostate );
	    fst->AddArc( istate, arc );
	    //During the initialization phase, just count non-eps transitions
	    //We currently initialize to uniform probability so there is also 
            // no need to tally anything here.
	    if( prev_alignment_model.find(arc.ilabel)==prev_alignment_model.end() ){
	      prev_alignment_model.insert( pair<LogArc::Label,LogWeight>(arc.ilabel, arc.weight) );
	      total = Plus( total, arc.weight );
	    }
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

void M2MFstAligner::maximization( bool lastiter ){
  //Maximization. Simple count normalization.  Probably get an improvement 
  // by using a more sophisticated regularization approach. 
  map<LogArc::Label,LogWeight>::iterator it;
  cout << "Total: " << total << " Change: " << abs(total.Value()-prevTotal.Value()) << endl;
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
	arc.weight = alignment_model[arc.ilabel];
	aiter.SetValue(arc);
      }
    }
  }

  total = LogWeight::Zero();
  return;
}


void M2MFstAligner::write_alignment( VectorFst<StdArc>& fst, int nbest ){
  //Generic alignment writer

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
      StdArc arc = aiter.Value();
      int maxl = get_max_length( isyms->Find(arc.ilabel) );
      if( maxl==-1 )
	arc.weight = 999; 
      else
	arc.weight = alignment_model[arc.ilabel].Value() * maxl;
      if( arc.weight == LogWeight::Zero() )
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
  if( shortest.NumStates()==0 )
    return;
  FstPathFinder pathfinder( skipSeqs );
  pathfinder.isyms = isyms;
  pathfinder.findAllStrings( shortest );
  
  for( int k=0; k < pathfinder.paths.size(); k++ ){
    for( int j=0; j < pathfinder.paths[k].path.size(); j++ ){
      cout << pathfinder.paths[k].path[j];
      if( j<pathfinder.paths[k].path.size() )
	cout << " ";
    }
    cout << endl;
  }
  
  return;
}
