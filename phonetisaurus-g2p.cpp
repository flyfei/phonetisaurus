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
#include <getopt.h>
#include "utf8.h"


vector<string> tokenize_utf8_string( string utf8_string, SymbolTable* isyms ) {
    /*
     Support for tokenizing a utf-8 string.
     http://stackoverflow.com/questions/2852895/c-iterate-or-split-utf-8-string-into-array-of-symbols#2856241
    */
    char* str = (char*)utf8_string.c_str(); // utf-8 string
    char* str_i = str;                      // string iterator
    char* str_j = str;
    char* end = str+strlen(str)+1;          // end iterator
    vector<string> string_vec;
    
    do
    {
        str_j = str_i;
        uint32_t code = utf8::next(str_i, end); // get 32 bit code of a utf-8 symbol
        if (code == 0)
            continue;
        int start = strlen(str) - strlen(str_j);
        int end   = strlen(str) - strlen(str_i);
        int len   = end - start;
        if( isyms->Find( utf8_string.substr(start,len) )==-1 ){
                cout << "Symbol: '" << utf8_string.substr(start,len) 
                << "' not found in input symbols table." << endl
                << "Aborting phoneticization of word: '" << utf8_string << "'." << endl
                << "Aborting phoneticizer job." << endl;
            exit(-1);
        }
        string_vec.push_back( utf8_string.substr(start,len) );
    }
    while ( str_i < end );
    
    return string_vec;
}

void phoneticizeWord( const char* g2pmodel_file, string testword, int nbest ){
    
    Phonetisaurus phonetisaurus( g2pmodel_file );
    //This approach assumes that the input alphabet 
    // consists exclusively of 1-character tokens
    //This should be OK for English G2P, but is no good 
    // for P2G or languages with multicharacter graphemes.
    vector<string> entry = tokenize_utf8_string( testword, phonetisaurus.isyms );
    
    vector<PathData> paths = phonetisaurus.phoneticize( entry, nbest );
    phonetisaurus.printPaths( paths, nbest );
    
    return;
}

void phoneticizeSentence( const char* g2pmodel_file, string sentence, int nbest ){
    /*
     Produce a sentence level pronunciation hypothesis for sequence of words.
     Ideally we should concatenate the FSA versions of the word entries prior 
     to running the phoneticizer.  This would further facilitate a later focus on 
     sentence level accent/pronunciation modeling.  Future work!
    */
    Phonetisaurus phonetisaurus( g2pmodel_file );
    
    char* tmpstring = (char *)sentence.c_str();
    char *p = strtok(tmpstring, " ");
    string word;
    
    int i=0;
    while (p) {
        word = p;
        vector<string> entry = tokenize_utf8_string( word, phonetisaurus.isyms );
        vector<PathData> paths = phonetisaurus.phoneticize( entry, nbest );
        phonetisaurus.printPaths( paths, nbest );
        
        p = strtok(NULL, " ");
    }
    return;
}
    
void phoneticizeTestSet( const char* g2pmodel_file, const char* testset_file, int nbest ){
    
    Phonetisaurus phonetisaurus( g2pmodel_file );
    
    ifstream test_fp;
    test_fp.open( testset_file );
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
            vector<string> entry = tokenize_utf8_string( word, phonetisaurus.isyms );
            vector<PathData> paths = phonetisaurus.phoneticize( entry, nbest );
            phonetisaurus.printPaths( paths, nbest, pron );
        }
        test_fp.close();
    }else{
        cout <<"Problem opening test file..." << endl;
    }            
    return;
}
    
int main( int argc, char **argv ) {
    
    int c;
    int g2pmodel_file_flag = 0;
    int testset_file_flag  = 0;
    int testword_flag      = 0;
    int sentence_flag      = 0;
    int nbest_flag         = 0;
    
	/* File names */
	const char* g2pmodel_file;
	const char* testset_file;
	string testword;
    string sentence;
    int nbest    = 1;
    
	/* Help Info */
	char help_info[1024];
    
	sprintf(help_info, "Syntax: %s -m g2p-model.fst [-t testfile | -w testword | -s 'a few words'] [-n N]\n\
Required:\n\
   -m --model: WFST representing the G2P/P2G model.\n\
   -t --testset: File containing a list of input words/pronunciations to phoneticize.\n\
   -w --word: A word/pronunciation to phoneticize.\n\
Optional:\n\
   -s --sent: A ' ' separated sentence to phoneticize.\n\
   -n --nbest: Optional max number of hypotheses to produce for each entry.  Defaults to 1.\n\
   -h --help: Print this help message.\n\n", argv[0]);
    
    /* Begin argument parsing */
    while( 1 ) {
        static struct option long_options[] = 
        {
            /* These options set a flag */
            {"model",      required_argument, 0, 'm'},
            {"testset",    required_argument, 0, 't'},
            {"testword",   required_argument, 0, 'w'},
            {"sent",       required_argument, 0, 's'},
            {"nbest",      required_argument, 0, 'n'},
            {"help",       no_argument, 0, 'h'},
            {0,0,0,0}
        };
    
        int option_index = 0;
        c = getopt_long( argc, argv, "hm:t:w:n:s:", long_options, &option_index);
        
        if ( c == -1 )
            break;
        switch ( c ) {
            case 0:
                if ( long_options[option_index].flag != 0)
                    break;
            case 'm':
                g2pmodel_file_flag = 1;
                g2pmodel_file = optarg;
                break;
            case 't':
                testset_file_flag = 1;
                testset_file = optarg;
                break;
            case 'w':
                testword_flag = 1;
                testword = optarg;
                break;
            case 'n':
                nbest_flag = 1;
                nbest = atoi(optarg);
                break;
            case 's':
                sentence_flag = 1;
                sentence = optarg;
                break;
            case 'h':
                printf("%s", help_info);
                exit(0);
                break;
            default:
                abort ();
        }
    }
    
    if( g2pmodel_file_flag==0 ){
        printf("%s", help_info);
        exit(0);
    }
    if( testset_file_flag + testword_flag + sentence_flag == 0 ){
        printf("%s", help_info);
        exit(0);
    }
    if( testset_file_flag + testword_flag + sentence_flag == 3 ){
        printf("Please enter only one of --testset, --word or --sent.");
        printf("%s", help_info);
        exit(0);
    }
    
    if( testword_flag==1 ){
        phoneticizeWord( g2pmodel_file, testword, nbest );
    }else if( testset_file_flag==1 ){
        phoneticizeTestSet( g2pmodel_file, testset_file, nbest );
    }else if( sentence_flag==1 ){
        phoneticizeSentence( g2pmodel_file, sentence, nbest );
    }

    return 0;
}
