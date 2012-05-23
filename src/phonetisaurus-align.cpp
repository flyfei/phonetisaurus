/*
 phonetisaurus-align.cpp

 Copyright (c) [2012-], Josef Robert Novak
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
  modification, are permitted #provided that the following conditions
  are met:

  * Redistributions of source code must retain the above copyright 
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above 
    copyright notice, this list of #conditions and the following 
    disclaimer in the documentation and/or other materials provided 
    with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
 COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
 INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
 STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED 
 OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/
#include "M2MFstAligner.hpp"
#include "util.hpp"
using namespace fst;


void load_input_file( M2MFstAligner* aligner, string input_file, string delim, string s1_char_delim, string s2_char_delim ){
  ifstream infile( input_file.c_str() );
  string line;
  cerr << "Loading input file: " << input_file << endl;

  if( infile.is_open() ){
    while( infile.good() ){
      getline(infile, line);
      if( line.empty() )
        continue;
      vector<string> tokens;
      split_string( &line, &tokens, &delim );
      vector<string> seq1 = tokenize_utf8_string( &tokens.at(0), &s1_char_delim );
      vector<string> seq2 = tokenize_utf8_string( &tokens.at(1), &s2_char_delim );
      aligner->entry2alignfst( seq1, seq2 );
    }
    infile.close();
  }

  return;
}

void write_alignments( M2MFstAligner* aligner, string ofile_name, int nbest ){
  /* Write the alignments to a file. */

  ofstream ofile(ofile_name.c_str());
  for( int i=0; i<aligner->fsas.size(); i++ ){
    vector<PathData> paths = aligner->write_alignment( aligner->fsas[i], nbest );
    for( int j=0; j<paths.size(); j++ ){
	for( int k=0; k<paths[j].path.size(); k++ ){
	  if( ofile )
	    ofile << paths[j].path[k];
	  else
	    cout << paths[j].path[k];
	  if( j<paths[j].path.size() ){
	    if( ofile )
	      ofile << " ";
	    else
	      cout << " ";
	  }
	}
	if( ofile )
	  ofile << endl;
	else
	  cout << endl;
    }
  }

  return;
}


DEFINE_string( input,          "",  "Two-column input file to align." );
DEFINE_bool(   seq1_del,     true,  "Allow deletions in sequence one." );
DEFINE_bool(   seq2_del,     true,  "Allow deletions in sequence two." );
DEFINE_int32(  seq1_max,        2,  "Maximum subsequence length for sequence one." );
DEFINE_int32(  seq2_max,        2,  "Maximum subsequence length for sequence two." );
DEFINE_string( seq1_sep,      "|",  "Multi-token separator for input tokens." );
DEFINE_string( seq2_sep,      "|",  "Multi-token separator for output tokens." );
DEFINE_string( s1s2_sep,      "}",  "Token used to separate input-output subsequences in the g2p model." );
DEFINE_string( delim,        "\t",  "Delimiter separating entry one and entry two in the input file." );
DEFINE_string( eps,       "<eps>",  "Epsilon symbol." );
DEFINE_string( skip,          "_",  "Skip token used to represent null transitions.  Distinct from epsilon." );
DEFINE_bool(   penalize,     true,  "Penalize scores." );
DEFINE_bool(   model,          "",  "Load a pre-trained model for use." );
DEFINE_string( ofile,          "",  "Output file to write the aligned dictionary to." );
DEFINE_bool(   mbr,         false,  "Use the LMBR decoder (not yet implemented)." );
DEFINE_int32(  iter,           11,  "Maximum number of EM iterations to perform." );
DEFINE_double( thresh,      1e-10,  "Delta threshold for EM training termination." ); 
DEFINE_int32(  nbest,           1,  "Output the N-best alignments given the model." );
DEFINE_string( s1_char_delim,  "",  "Sequence one input delimeter." );
DEFINE_string( s2_char_delim, " ",  "Sequence two input delimeter." );


int main( int argc, char* argv[] ){
  string usage = "phonetisaurus-align dictionary aligner.\n\n Usage: ";
  set_new_handler(FailedNewHandler);
  SetFlags(usage.c_str(), &argc, &argv, false );

  M2MFstAligner aligner( 
			FLAGS_seq1_del, FLAGS_seq2_del, FLAGS_seq1_max, FLAGS_seq2_max, 
			FLAGS_seq1_sep, FLAGS_seq2_sep, FLAGS_s1s2_sep, 
			FLAGS_eps, FLAGS_skip, FLAGS_penalize 
			 );

  load_input_file( &aligner, FLAGS_input, FLAGS_delim, FLAGS_s1_char_delim, FLAGS_s2_char_delim );

  cerr << "Starting EM..." << endl;
  aligner.maximization(false);
  cerr << "Finished first iter..." << endl;
  for( int i=1; i<=FLAGS_iter; i++ ){
    cerr << "Iteration: " << i << " Change: ";
    aligner.expectation();
    cerr << aligner.maximization(false) << endl;
  }
  cerr << "Last iteration: " << endl;
  aligner.expectation();
  aligner.maximization(true);

  write_alignments( &aligner, FLAGS_ofile, FLAGS_nbest );

  return 1;
}
