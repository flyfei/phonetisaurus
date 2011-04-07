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
    set<string>   clusters;
    //FST stuff
    StdVectorFst  *g2pmodel;
    SymbolTable   *isyms;
    SymbolTable   *osyms;
        
    Phonetisaurus( );
        
    Phonetisaurus( const char* g2pmodel_file, const char* clusters_file, 
                  const char* isyms_file, const char* osyms_file );

    StdVectorFst entryToFSA( string entry );

    vector<PathData> phoneticize( string entry, int nbest );

    void printPaths( vector<PathData> paths, int nbest, string correct="" );
    
private:
    void loadClusters( const char* clusters_file );

};

#endif // PHONETISAURUS_H //
