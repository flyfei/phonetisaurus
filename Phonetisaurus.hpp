#ifndef PHONETISAURUS_H
#define PHONETISAURUS_H
/*
    Phonetisaurus.hpp 
   
    Created by Josef Novak on 2011-04-07.
    Copyright 2011-2012 Josef Novak. All rights reserved.

    This file is part of Phonetisaurus.

    Phonetisaurus is free software: you can redistribute it 
    and/or modify it under the terms of the GNU General Public 
    License as published by the Free Software Foundation, either 
    version 3 of the License, or (at your option) any later version.

    Phonetisaurus is distributed in the hope that it will be useful, but 
    WITHOUT ANY WARRANTY; without even the implied warranty of 
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
    General Public License for more details.

    You should have received a copy of the GNU General Public 
    License along with Phonetisaurus. If not, see http://www.gnu.org/licenses/.
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

    bool printPaths( vector<PathData> paths, int nbest, string correct="", string word="" );
    
private:
    void loadClusters( );

};

#endif // PHONETISAURUS_H //
