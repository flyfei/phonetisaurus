/*
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

    FstPathFinder.cpp
  ----------------
    Original author: chris taylor

    OpenFst forum post title: "Natural code for printing all strings accepted by an FST?"
    OpenFst forum post link: http://openfst.cs.nyu.edu/twiki/bin/view/Forum/FstForum#Natural_code_for_printing_all_st

  ----------------

    2011-04-07: Modified by Josef Novak 

    Modified to build a 'paths' object to store the individual paths
    and associated weights, rather than just print them out from 
    inside the class.  Useful if you want to return the paths for further
    processing.
*/

#include "FstPathFinder.hpp"

FstPathFinder::FstPathFinder( ) {
    //Default constructor
}

FstPathFinder::FstPathFinder( set<string> skipset ) {
    //Constructor for a non-empty skipset
    skipSeqs = skipset;
}

void FstPathFinder::findAllStrings( VectorFst<StdArc>& fst ) {
    /*
     Main search function.  Initiates the WFSA traversal.
     We are making three potentially dangerous assumptions 
     here regarding the input FST:
     
       1. It has *ALREADY* been run through the shortestpath algorithm
           *This guarantees the the FST is acyclic and that the paths are 
             sorted according to path cost.
       2. It has *ALREADY* been projected
           *This just saves us some hassle.
       3. The symbol tables have been stored in the input FST
           *This just saves us some hassle.
     
     If the input FST does not meet these conditions this will 
     cause problems.
     */
    
    vector<string> path;
    if( fst.InputSymbols()!=NULL )
      isyms = (SymbolTable*)fst.InputSymbols();
    findAllStringsHelper( fst, fst.Start(), path, TropicalWeight::One() );
    
    return;
}

void FstPathFinder::addOrDiscardPath( PathData pdata ) {
    /*
     Determine whether or not the input path has been added
     to the paths vector or not.  If it hasn't, add it, otherwise
     discard it.
    */
    
    set< vector<string> >::iterator sit;
    sit = uniqueStrings.find( pdata.path );

    if ( sit == uniqueStrings.end() ){
        paths.push_back( pdata );
        uniqueStrings.insert( pdata.path );
    }        
    return;
}

void FstPathFinder::findAllStringsHelper(VectorFst<StdArc>& fst, int state, vector<string>& path, TropicalWeight cost ) {
    /*
     Recursively traverse the WFSA and build up a vector of 
      unique paths and associated costs.
    */
    
    if( fst.Final(state) != TropicalWeight::Zero() ) {
        
        PathData pdata;
        pdata.path     = path;
        pdata.pathcost = Times(cost,fst.Final(state)).Value();
        
        addOrDiscardPath( pdata );
        
        path.clear();
        
        return;
    }

    for( ArcIterator<VectorFst<StdArc> > iter(fst,state); !iter.Done(); iter.Next() ) {
        StdArc arc = iter.Value();

        string symbol = isyms->Find( arc.ilabel );
        
        bool skip = false;
        for( set<string>::iterator sit=skipSeqs.begin(); sit!=skipSeqs.end(); sit++ )
            if( symbol.compare( *sit ) == 0 )
                skip = true;
        if( skip==false )
            path.push_back(symbol);
        
        findAllStringsHelper( fst, arc.nextstate, path, Times(cost, arc.weight.Value()) );
    }
}


