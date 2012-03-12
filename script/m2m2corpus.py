#!/usr/bin/python
#    Copyright 2011-2012 Josef Robert Novak
#
#    This file is part of Phonetisaurus.
#
#    Phonetisaurus is free software: you can redistribute it 
#    and/or modify it under the terms of the GNU General Public 
#    License as published by the Free Software Foundation, either 
#    version 3 of the License, or (at your option) any later version.
#
#    Phonetisaurus is distributed in the hope that it will be useful, but 
#    WITHOUT ANY WARRANTY; without even the implied warranty of 
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
#    General Public License for more details.
#
#    You should have received a copy of the GNU General Public 
#    License along with Phonetisaurus. If not, see http://www.gnu.org/licenses/.
import re

def m2m2Corpus( aligned_file, prefix="test", input_sep=" ", output_sep=" ", entry_sep="\t", multi_sep="|", io_sep="}" ):
    """
      Transform an aligned dictionary into a corpus suitable for training a joint LM.
    """

    aligned_file_fp = open( aligned_file, "r" )
    corpus_file_fp  = open( "PREFIX.corpus".replace("PREFIX",prefix), "w" )

    for line in aligned_file_fp:
        line = line.strip()
        input, output = line.split( entry_sep )
        input = input.strip()
        output = output.strip()
        i_toks = input.split( input_sep )
        o_toks = output.split( output_sep )
        io_toks = []
        assert len(i_toks)==len(o_toks)
        
        for i, i_tok in enumerate(i_toks):
            io_tok = "%s%s%s" % (i_tok, io_sep, o_toks[i])
            io_toks.append(io_tok)

        corpus_file_fp.write("%s\n" % (" ".join(io_toks)))
    aligned_file_fp.close()
    corpus_file_fp.close()
    return

if __name__=="__main__":
    import sys

    m2m2Corpus( sys.argv[1], prefix=sys.argv[2] )
