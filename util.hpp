#include <fst/fstlib.h>
#include "utf8.h"
using namespace fst;


vector<string> tokenize_utf8_string( string* utf8_string, string* delimiter ) {
  /*
     Support for tokenizing a utf-8 string. Adapted to also support a delimiter.
     Note that leading, trailing or multiple consecutive delimiters will result in 
     empty vector elements.  Normally should not be a problem but just in case.
     Also note that any tokens that cannot be found in the model symbol table will be
     deleted from the input word prior to grapheme-to-phoneme conversion.

     http://stackoverflow.com/questions/2852895/c-iterate-or-split-utf-8-string-into-array-of-symbols#2856241
  */
  char* str   = (char*)utf8_string->c_str(); // utf-8 string
  char* str_i = str;                        // string iterator
  char* str_j = str;
  char* end   = str+strlen(str)+1;          // end iterator
  vector<string> string_vec;
  if( delimiter->compare("") != 0 )
    string_vec.push_back("");

  do {
    str_j = str_i;
    uint32_t code = utf8::next(str_i, end); // get 32 bit code of a utf-8 symbol
    if (code == 0)
      continue;
    int start = strlen(str) - strlen(str_j);
    int end   = strlen(str) - strlen(str_i);
    int len   = end - start;
      
    if( delimiter->compare("")==0 ){
      string_vec.push_back( utf8_string->substr(start,len) );
    }else{
      if( delimiter->compare(utf8_string->substr(start,len))==0 )
        string_vec.push_back("");
      else
        string_vec[string_vec.size()-1] += utf8_string->substr(start,len);
    }
  } while ( str_i < end );
  
  return string_vec;
}

vector<string> tokenize_entry( string* testword, string* sep, SymbolTable* syms ){
  vector<string> tokens = tokenize_utf8_string( testword, sep );
  vector<string> entry;
  for( int i=0; i<tokens.size(); i++ ){
    if( syms->Find(tokens.at(i)) != -1 ){
      entry.push_back(tokens.at(i));
    }else{
      cerr << "Symbol: '" << tokens.at(i)
           << "' not found in input symbols table." << endl
           << "Mapping to null..." << endl;
    }
  }

  return entry;
}
