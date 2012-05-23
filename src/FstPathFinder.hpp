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
    Original author: Chris Taylor

    OpenFst forum post title: "Natural code for printing all strings accepted by an FST?"
    OpenFst forum post link: http://openfst.cs.nyu.edu/twiki/bin/view/Forum/FstForum#Natural_code_for_printing_all_st

  ----------------

    2011-04-07: Modified by Josef Novak 

    Modified to build a 'paths' object to store the individual paths
    and associated weights, rather than just print them out from 
    inside the class.  Useful if you want to return the paths for further
    processing.
*/
#ifndef __FSTPATHFINDER__
#define __FSTPATHFINDER__

#include <fst/fstlib.h>

using namespace fst;

struct PathData{
    vector<string> path;
    float pathcost;
};

class FstPathFinder{
    
public:
    
    vector<PathData> paths;
    
    set<string> skipSeqs;
    
    set< vector<string> > uniqueStrings;
    
    SymbolTable *isyms; 
    
    FstPathFinder( );
    
    FstPathFinder( set<string> skipset );

    void findAllStrings( StdVectorFst&  fst );
    
private:
    
    void addOrDiscardPath( PathData pdata );

    void findAllStringsHelper( 
                StdVectorFst&   fst, 
			    int             state, 
			    vector<string>& str, 
			    TropicalWeight  cost 
              ); 

}; // end class

#endif

