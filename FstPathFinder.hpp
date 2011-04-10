/*

fstprintpaths.hpp

traverses an fst and prints out all of the paths in that fst along with the costs.

output format is: WORD cost X.XX

where WORD is a sequence of characters accepted or generated by the fst AND X.XX 
is a floating point value representing the cost of traversing that structure.

author: chris taylor

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

typedef struct PathData{
    vector<string> path;
    TropicalWeight pathcost;
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

