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
    
    Phonetisaurus phonetisaurus( argv[1], argv[2], argv[3], argv[4] );
    
    ifstream test_fp;
    test_fp.open( argv[5] );
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
            vector<PathData> paths = phonetisaurus.phoneticize( word, 1 );
            phonetisaurus.printPaths( paths, 1, pron );
        }
        test_fp.close();
    }else{
        cout <<"Problem..." << endl;
    }            
    
    
    return 0;
}
