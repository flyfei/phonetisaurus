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
                cerr << "Symbol: '" << utf8_string.substr(start,len) 
                << "' not found in input symbols table." << endl
		//<< "Aborting phoneticization of word: '" << utf8_string << "'." << endl
         	<< "Mapping to null..." << endl;
		
                //<< "Aborting phoneticizer job." << endl;
		//exit(-1);
        }else{
          string_vec.push_back( utf8_string.substr(start,len) );
	}
    }
    while ( str_i < end );
    
    return string_vec;
}

vector<string> tokenize_string( string input_string, SymbolTable* isyms, string sep ){
    /*
     Tokenize the input using a user-specified character.
    */
    string strcopy = input_string.c_str();
    char* tmpstring = (char *)input_string.c_str();
    char *p = strtok(tmpstring, sep.c_str());
    vector<string> entry;
    
    int i=0;
    while (p) {
        if( isyms->Find(p)==-1 ){
            cerr << "Symbol: '" << p 
            << "' not found in input symbols table." << endl
            << "Mapping to null..." << endl;
	    //<< "Aborting phoneticization of word: '" << strcopy << "'." << endl
	    //<< "Aborting phoneticizer job." << endl;
            //exit(-1);
        }else{
	  entry.push_back(p);
	}
	p = strtok(NULL, sep.c_str());
    }
    
    return entry;
}

void phoneticizeWord( const char* g2pmodel_file, string testword, int nbest, string sep, int beam=500, int output_words=0 ){
    
    Phonetisaurus phonetisaurus( g2pmodel_file );

    vector<string> entry;
    if( sep.compare("")==0 )
        entry = tokenize_utf8_string( testword, phonetisaurus.isyms );
    else
        entry = tokenize_string( testword, phonetisaurus.isyms, " " );

    vector<PathData> paths = phonetisaurus.phoneticize( entry, nbest, beam );
    if( output_words==0){
      while( phonetisaurus.printPaths( paths, nbest )==true ){
	nbest++;
	paths = phonetisaurus.phoneticize( entry, nbest, beam );
      }
    }else{
      while( phonetisaurus.printPaths( paths, nbest, "", testword )==true){
	nbest++;
	paths = phonetisaurus.phoneticize( entry, nbest, beam );
      }
    }
    return;
}

void phoneticizeSentence( const char* g2pmodel_file, string sentence, int nbest, int beam=500 ){
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
        vector<PathData> paths = phonetisaurus.phoneticize( entry, nbest, beam );
        phonetisaurus.printPaths( paths, nbest );
        
        p = strtok(NULL, " ");
    }
    return;
}
    
void phoneticizeTestSet( const char* g2pmodel_file, const char* testset_file, int nbest, string sep, int beam=500, int output_words=0 ){
    
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
            
            vector<string> entry;
            if( sep.compare("")==0 )
                entry = tokenize_utf8_string( word, phonetisaurus.isyms );
            else
                entry = tokenize_string( word, phonetisaurus.isyms, sep );
            
            vector<PathData> paths = phonetisaurus.phoneticize( entry, nbest, beam=beam );
	    int nbest_new = nbest;
	    if( output_words==0){
	      while( phonetisaurus.printPaths( paths, nbest_new, pron )==true ){
		nbest_new++;
		paths = phonetisaurus.phoneticize( entry, nbest_new, beam );
	      }
	    }else{
	      while( phonetisaurus.printPaths( paths, nbest_new, pron, word )==true){
		nbest_new++;
		paths = phonetisaurus.phoneticize( entry, nbest_new, beam );
	      }
	    }
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
    int beam_flag          = 0;
    int sep_flag           = 0;
    int output_words_flag  = 0;
    
	/* File names */
	const char* g2pmodel_file;
	const char* testset_file;
	string testword;
    string sentence;
    string sep = "";
    int nbest  = 1;
    int beam   = 500;
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
   -o --output_words: Output the orthography along with the pronunciation hypotheses and scores.\n\
   -e --sep: Optional separator character for tokenization.  Defaults to ''.\n\
   -b --beam: Optional beam for N-best search.  Defaults to '500'. Larger beams will yield more N-best, but will not affect accuracy.\n\
   -h --help: Print this help message.\n\n", argv[0]);
    
    /* Begin argument parsing */
    while( 1 ) {
        static struct option long_options[] = 
        {
            /* These options set a flag */
            {"model",      required_argument, 0, 'm'},
            {"testset",    required_argument, 0, 't'},
            {"word",       required_argument, 0, 'w'},
            {"sent",       required_argument, 0, 's'},
            {"nbest",      required_argument, 0, 'n'},
	    {"output_words", no_argument, 0, 'o'},
            {"beam",       required_argument, 0, 'b'},
            {"sep",        required_argument, 0, 'e'},
            {"help",       no_argument, 0, 'h'},
            {0,0,0,0}
        };
    
        int option_index = 0;
        c = getopt_long( argc, argv, "hom:t:w:b:n:s:e:", long_options, &option_index);
        
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
	    case 'o':
	        output_words_flag = 1;
		break;
            case 'b':
                beam_flag = 1;
		beam = atoi(optarg);
		break;
            case 's':
                sentence_flag = 1;
                sentence = optarg;
                break;
            case 'e':
                sep_flag = 1;
                sep = optarg;
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
      phoneticizeWord( g2pmodel_file, testword, nbest, sep, beam, output_words_flag );
    }else if( testset_file_flag==1 ){
      phoneticizeTestSet( g2pmodel_file, testset_file, nbest, sep, beam, output_words_flag );
    }else if( sentence_flag==1 ){
      phoneticizeSentence( g2pmodel_file, sentence, nbest, beam );
    }

    return 0;
}
