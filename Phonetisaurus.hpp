#ifndef PHONETISAURUS_H
#define PHONETISAURUS_H
/*
 *  Phonetisaurus.hpp 
 *  
 *  Created by Josef Novak on 2011-04-07.
 *  Copyright 2011 Josef Novak. All rights reserved.
 *
 */
#include <fst/fstlib.h>
#include "FstPathFinder.hpp"

using namespace fst;

class Phonetisaurus {
    /*
     Load a G2P/P2G model and generate pronunciation/spelling
     hypotheses for input items.
    */
public:
    //Basics
    string        eps;
    string        se;
    string        sb;
    string        skip;
    string        tie;
    set<string>   skipSeqs;
    map<vector<string>, int>   clusters;
    //FST stuff
    StdVectorFst  *g2pmodel;
    StdVectorFst  epsMapper;
    SymbolTable   *isyms;
    SymbolTable   *osyms;
        
    Phonetisaurus( );
        
    Phonetisaurus( const char* g2pmodel_file );

    StdVectorFst entryToFSA( vector<string> entry );

    StdVectorFst makeEpsMapper( );
    
    vector<PathData> phoneticize( vector<string> entry, int nbest, int beam=500 );

    void printPaths( vector<PathData> paths, int nbest, string correct="", string word="" );
    
private:
    void loadClusters( );

};

#endif // PHONETISAURUS_H //
