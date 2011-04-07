/*
 *  Phonetisaurus.cpp 
 *  
 *  Created by Josef Novak on 2011-04-07.
 *  Copyright 2011 Josef Novak. All rights reserved.
 *
 */
#include <stdio.h>
#include <string>
#include <fst/fstlib.h>
#include <iostream>
#include <set>
#include <algorithm>
#include "FstPathFinder.hpp"
#include "Phonetisaurus.hpp"

using namespace fst;

Phonetisaurus::Phonetisaurus( ) {
    //Default constructor
}

Phonetisaurus::Phonetisaurus( const char* g2pmodel_file, const char* clusters_file, 
                             const char* isyms_file, const char* osyms_file ) {
    //Base constructor.  Load the clusters file, the models and setup shop.
    eps  = "<eps>";
    sb   = "<s>";
    se   = "</s>";
    skip = "_";
    tie  = "&";
    loadClusters( clusters_file );
    
    isyms = SymbolTable::ReadText( isyms_file );

    osyms = SymbolTable::ReadText( osyms_file );

    g2pmodel = StdVectorFst::Read( g2pmodel_file );

    //We need make sure the g2pmodel is arcsorted
    ILabelCompare<StdArc> icomp;
    ArcSort( g2pmodel, icomp );
}


void Phonetisaurus::loadClusters( const char* clusters_file ){
    /*
     Load the clusters file containing the list of 
     subsequences generated during multiple-to-multiple alignment
    */
    
    ifstream clusters_fp;
    clusters_fp.open( clusters_file );
    string line;
    if( clusters_fp.is_open() ){
        while( clusters_fp.good() ){
            getline( clusters_fp, line );
            if( line.compare("")==0 )
                continue;
            clusters.insert(line);
        }
        clusters_fp.close();
    }else{
        cout << "Couldn't open the clusters file!" << endl;
    }
    return;
}


StdVectorFst Phonetisaurus::entryToFSA( string entry ){
    /*
     Transform an input spelling/pronunciation into an equivalent
     FSA, adding extra arcs as needed to accomodate clusters.
    */
    
    StdVectorFst efst;
    efst.AddState();
    efst.SetStart(0);

    set<string>::iterator it;
    efst.AddState();
    efst.AddArc( 0, StdArc( isyms->Find( sb ), isyms->Find( sb ), 0, 1 ));
    size_t i=0;
    
    for( i=0; i<entry.size(); i++){
        efst.AddState();
        string ch = entry.substr(i,1);
        efst.AddArc( i+1, StdArc( isyms->Find(ch), isyms->Find(ch), 0, i+2 ));
        if( i==0 ) 
            continue;
        
        string sub = entry.substr(i-1,2);
        it = clusters.find(sub);
        if( it!=clusters.end() ){
            sub.insert( 1, tie );
            efst.AddArc( i, StdArc( isyms->Find(sub), isyms->Find(sub), 0, i+2));
        }
    }
    
    efst.AddState();
    efst.AddArc( i+1, StdArc( isyms->Find( se ), isyms->Find( se ), 0, i+2));
    efst.SetFinal(i+2,0);

    return efst;
}


vector<PathData> Phonetisaurus::phoneticize( string entry, int nbest ){
    /*
     Generate pronunciation/spelling hypotheses for an 
     input entry.
    */
    
    StdVectorFst result;
    StdVectorFst shortest;

    StdVectorFst efst = entryToFSA( entry );

    Compose( efst, *g2pmodel, &result );

    Project(&result, PROJECT_OUTPUT);

    if( nbest > 1 ){
        //This is a cheesy hack. We should really be determinizing the result
        // after removing all the unwanted symbols, otherwise we cannot guarantee 
        // that the paths returned will actually be unique.  
        //We are not doing this because the vanilla LM WFST is not determinization
        // friendly and the operation may not terminate.
        ShortestPath( result, &shortest, 5000 );
    }else{
        ShortestPath( result, &shortest, 1 );
    }

    FstPathFinder pathfinder;
    pathfinder.findAllStrings( shortest, *osyms, eps );
    
    return pathfinder.paths;
}


void Phonetisaurus::printPaths( vector<PathData> paths, int nbest, string correct ){
    /*
     Convenience function to print out a path vector.
     Will print only the first N unique entries.
    */

    set<string> seen;
    set<string>::iterator sit;
    
    int numseen = 0;
    string onepath;
    size_t k;
    for( k=0; k < paths.size(); k++ ){
        
        size_t j;
        for( j=0; j < paths[k].path.size(); j++ ){
            
            if( paths[k].path[j]==eps || paths[k].path[j]=="-" || 
               paths[k].path[j]==skip || paths[k].path[j]==sb || 
               paths[k].path[j]==se )
                continue;
            
            if( paths[k].path[j] != tie )
                replace( paths[k].path[j].begin(), paths[k].path[j].end(), '&', ' ');
            
            onepath += paths[k].path[j];
            
            if( j != paths[k].path.size()-1 )
                onepath += " ";
        }
        
        sit = seen.find(onepath);
        
        if( sit == seen.end() ){
            cout << paths[k].pathcost << "\t" << onepath;
            if( correct != "" )
                cout << "\t" << correct;
            cout << endl;
            seen.insert( onepath );
            numseen++;
        }
        
        onepath = "";
        if( numseen >= nbest )
            break;
    }
}










