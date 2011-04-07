//2011-04-07 Copyright Josef R. Novak 
// Some ugly but functional C++ code.
// Desperately needs refactoring but is nonetheless
//  about a jillion times better than the python
//  'decoder' that preceded it!
#include <stdio.h>
#include <string>
#include <fst/fstlib.h>
#include <iostream>
#include <set>
#include <algorithm>
#include "FstPathFinder.hpp"

using namespace fst;

set<string> loadClusters( char* clusterFile ){
  ifstream clusters_fp;
  clusters_fp.open(clusterFile);
  set<string> clusters;
  string line;
  if( clusters_fp.is_open() ){
    while( clusters_fp.good() ){
      //Read one line
      getline( clusters_fp, line );
      if( line.compare("")==0 )
	continue;
      clusters.insert(line);
    }
    clusters_fp.close();
  }else{
    cout << "Couldn't open the file, something went terribly wrong!" << endl;
  }
  return clusters;
}

StdVectorFst processEntry( string entry, set<string> clusters, SymbolTable* syms ){
  StdVectorFst efst;
  efst.AddState();
  efst.SetStart(0);

  set<string>::iterator it;
  efst.AddState();
  efst.AddArc( 0, StdArc(syms->Find("<s>"), syms->Find("<s>"), 0, 1 ));
  unsigned int i=0;
  for( i=0; i<entry.size(); i++){
    efst.AddState();
    string ch = entry.substr(i,1);
    efst.AddArc( i+1, StdArc(syms->Find(ch), syms->Find(ch), 0, i+2 ));
    if( i==0 ) continue;
    string sub = entry.substr(i-1,2);
    it = clusters.find(sub);
    if( it!=clusters.end() ){
      sub.insert(1,"&");
      efst.AddArc( i, StdArc(syms->Find(sub), syms->Find(sub), 0, i+2));
    }
  }
  efst.AddState();
  efst.AddArc( i+1, StdArc( syms->Find("</s>"), syms->Find("</s>"), 0, i+2));
  efst.SetFinal(i+2,0);
  return efst;
}

void readTestSet( char* testFile, set<string> clusters, SymbolTable* syms, StdVectorFst* model, SymbolTable* osyms, int n ){
  ifstream test_fp;
  StdVectorFst efst;
  test_fp.open(testFile);
  string line;
  if( test_fp.is_open() ){
    while( test_fp.good() ){
      getline( test_fp, line );
      if( line.compare("")==0 )
	continue;

      char* tmpstring = (char *)line.c_str();
      char *p = strtok(tmpstring, "\t");
      string word;
      string pron;
      int i=0;
      while (p) {
	if( i==0 ) 
	  word = p;
	else
	  pron = p;
	i++;
	p = strtok(NULL, "\t");
      }

      StdVectorFst result;
      efst = processEntry( word, clusters, syms );
      Compose( efst, *model, &result );
      StdVectorFst shortest;
      string seqToSkip = "<eps>";
      Project(&result, PROJECT_OUTPUT);
      if( n > 1 ){
	//We should really be determinizing the result
	// after removing all the unwanted symbols, otherwise
	// we cannot guarantee that the paths returned will
	// actually be unique.  
	//     R EH D vs. R EH <eps> D vs. R EH _ D
	// These are all equivalent so we really only 
	//  want the cheapest version.
	// What I've done below is just a full-retard hack
	//  we over-generate the nbest hypotheses and 
	//  then check for uniqueness as we print.
	//  this is slow but not as slow as trying to 
	//  determinize the projected model.
	ShortestPath( result, &shortest, 5000 );
      }else{
	ShortestPath( result, &shortest, n );
      }
      
      FstPathFinder pf;
      pf.findAllStrings( shortest, *osyms, seqToSkip );
      set<string> seen;
      set<string>::iterator sit;
      int numseen = 0;
      string thepath;
      size_t k;
      for( k=0; k < pf.paths.size(); k++ ){
	size_t j;
	for( j=0; j < pf.paths[k].path.size(); j++ ){
	  if( pf.paths[k].path[j]=="<eps>" || pf.paths[k].path[j]=="-" || pf.paths[k].path[j]=="_" || pf.paths[k].path[j]=="<s>" || pf.paths[k].path[j]=="</s>" )
	    continue;
	  if( pf.paths[k].path[j] != "&" )
	    replace(pf.paths[k].path[j].begin(), pf.paths[k].path[j].end(), '&', ' ');
	      
	  thepath += pf.paths[k].path[j];
	  if( j != pf.paths[k].path.size()-1 )
	    thepath += " ";
	}
	sit = seen.find(thepath);
	if( sit == seen.end() ){
	  cout << pf.paths[k].pathcost << "\t" << thepath << "\t" << pron << endl;
	  seen.insert(thepath);
	  numseen++;
	}
	thepath = "";
	if( numseen >= n )
	  break;
      }
    }
    test_fp.close();
  }else{
    cout <<"Problem..." << endl;
  }
  return;
}

int main( int argc, const char* argv[] ) {

  if( argc<6 ){
    cout << "./phonetisaurus <clusters> <testlist> <isyms> <model.fst> <osyms> <N-best>" << endl;
    exit(0);
  }
  
  set<string> clusters = loadClusters( (char *)argv[1] );
  set<string>::iterator it;
  SymbolTable *syms   = SymbolTable::ReadText(argv[3]);
  SymbolTable *osyms  = SymbolTable::ReadText(argv[5]);
  StdVectorFst *model = StdVectorFst::Read(argv[4]);
  int n = atoi(argv[6]);
  ILabelCompare<StdArc> icomp;
  ArcSort( model, icomp );
  readTestSet( (char *)argv[2], clusters, syms, model, osyms, n );
  return 0;
}
