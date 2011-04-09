/*
 *  phonetisaurus-g2p.hpp 
 *  
 *  Created by Josef Novak on 2011-04-07.
 *  Copyright 2011 Josef Novak. All rights reserved.
 *
 */
#include "Phonetisaurus.hpp"
#include <stdio.h>
#include <string>


int main( int argc, const char* argv[] ) {
    
    Phonetisaurus phonetisaurus( argv[1]);
    
    ifstream test_fp;
    test_fp.open( argv[2] );
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
            
            //This approach assumes that the input alphabet 
            // consists exclusively of 1-character tokens
            //This should be OK for English G2P, but is no good 
            // for P2G or languages with multicharacter graphemes.
            vector<string> entry;
            int j;
            for( j=0; j < word.size(); j++ )
                entry.push_back( word.substr(j,1) );
            vector<PathData> paths = phonetisaurus.phoneticize( entry, 1 );
            phonetisaurus.printPaths( paths, 1, pron );
        }
        test_fp.close();
    }else{
        cout <<"Problem..." << endl;
    }            
    
    
    return 0;
}
