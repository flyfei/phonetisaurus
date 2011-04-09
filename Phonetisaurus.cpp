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

Phonetisaurus::Phonetisaurus( const char* g2pmodel_file ) {
    //Base constructor.  Load the clusters file, the models and setup shop.
    eps  = "<eps>";
    sb   = "<s>";
    se   = "</s>";
    skip = "_";
    tie  = "&";
    
    g2pmodel = StdVectorFst::Read( g2pmodel_file );

    isyms = (SymbolTable*)g2pmodel->InputSymbols(); 

    osyms = (SymbolTable*)g2pmodel->OutputSymbols(); 
    
    loadClusters( );
    
    //We need make sure the g2pmodel is arcsorted
    ILabelCompare<StdArc> icomp;
    ArcSort( g2pmodel, icomp );
}


void Phonetisaurus::loadClusters( ){
    /*
     Load the clusters file containing the list of 
     subsequences generated during multiple-to-multiple alignment
    */
    
    for( size_t i = 0; i < isyms->NumSymbols(); i++ ){
        string sym = isyms->Find( i );
        
        if( sym.find(tie) != string::npos ){
            char* tmpstring = (char *)sym.c_str();
            char* p = strtok(tmpstring, tie.c_str());
            vector<string> cluster;
            
            while (p) {
                cluster.push_back(p);
                p = strtok(NULL, tie.c_str());
            }
            
            clusters[cluster] = i;
        }
    }
    return;
}


StdVectorFst Phonetisaurus::entryToFSA( vector<string> entry ){
    /*
     Transform an input spelling/pronunciation into an equivalent
     FSA, adding extra arcs as needed to accomodate clusters.
    */
    
    StdVectorFst efst;
    efst.AddState();
    efst.SetStart(0);

    efst.AddState();
    efst.AddArc( 0, StdArc( isyms->Find( sb ), isyms->Find( sb ), 0, 1 ));
    size_t i=0;
    
    //Build the basic FSA
    for( i=0; i<entry.size(); i++){
        efst.AddState();
        string ch = entry[i];
        efst.AddArc( i+1, StdArc( isyms->Find(ch), isyms->Find(ch), 0, i+2 ));
        if( i==0 ) 
            continue;
        
    }
    
    //Add any cluster arcs
    map<vector<string>,int>::iterator it_i;
    for( it_i=clusters.begin(); it_i!=clusters.end(); it_i++ ){
        vector<string>::const_iterator it_j;
        vector<string> cluster = (*it_i).first;
        it_j = search( entry.begin(), entry.end(), cluster.begin(), cluster.end() );
        if( it_j != entry.end() )
            efst.AddArc( it_j-entry.begin()+1, StdArc( 
                        (*it_i).second,                     //input symbol
                        (*it_i).second,                     //output symbol
                        0,                                  //weight
                        it_j-entry.begin()+cluster.size()+1 //destination state
                    ) );
    }
    
    efst.AddState();
    efst.AddArc( i+1, StdArc( isyms->Find( se ), isyms->Find( se ), 0, i+2));
    efst.SetFinal(i+2,0);
    efst.SetInputSymbols(isyms);
    efst.SetOutputSymbols(isyms);
    
    return efst;
}


vector<PathData> Phonetisaurus::phoneticize( vector<string> entry, int nbest ){
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










