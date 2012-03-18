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
from M2MFstAligner import M2MFstAligner
import sys

def splitter( part, sep ):
    if sep=="":
        return list(part)
    else:
        return part.split(sep)

def train_aligner( aligner, training_file, max_iter, s1in_sep="", s2in_sep=" ", s1s2in_sep="\t" ):
    print "Building WFST-based alignment corpus from training data..."
    for line in open(training_file,"r"):
        part1, part2 = line.strip().split(s1s2in_sep)
        seq1 = splitter( part1, s1in_sep )
        seq2 = splitter( part2, s2in_sep )
        aligner.entry2alignfst( seq1, seq2 )
    print "Finished adding entries..."
    print "Starting EM training..."

    change = aligner.maximization(False)
    print "Finished initialization..."
    for i in xrange(max_iter):
        print "Iter:", i
        aligner.expectation()
        change = aligner.maximization(False)
        print "Change:", change

    print "Last iteration..."
    aligner.expectation()
    aligner.maximization(True)
    return aligner

def write_aligned_training_data( aligner, aligned_file, nbest=1 ):
    print "Writing %d-best alignments to file: %s." % (nbest, aligned_file)
    ofp = open(aligned_file,"w")
    for i in xrange(aligner.num_fsas()):
        results = aligner.write_alignment_wrapper( i, nbest )
        for result in results:
            ofp.write("%s\n"%" ".join(result.path))
    ofp.close()
    return

def align_sequences( aligner, part1, part2, s1in_sep="", s2in_sep=" ", nbest=1, write_lattice=None ):
    """Convenience access to perform a single alignment.  Mainly for testing purposes."""
    seq1 = splitter( part1, s1in_sep )
    seq2 = splitter( part2, s2in_sep )

    if write_lattice:
        print "Writing weighted alignment lattice to file: %s" % write_lattice
        results = aligner.entry2alignfstnoinit( seq1, seq2, nbest, write_lattice )
    else:
        results = aligner.entry2alignfstnoinit( seq1, seq2, nbest )

    for result in results:
        print "%0.4f\t%s" % (result.pathcost, " ".join(result.path))

    return


if __name__=="__main__":
    import sys, argparse
    from argparse import RawTextHelpFormatter

    example2 = """Train a model:\n\t%s --align train.txt -s2 --write_model train.model.fst --write_align train.aligned""" % sys.argv[0]
    example1 = """Align a sequence:\n\t%s --model model.fst --string1 "aback" --string2 "x b @ k" --nbest 2""" % sys.argv[0]
    examples = example1+"\n\n"+example2
    parser = argparse.ArgumentParser(description=examples, formatter_class=RawTextHelpFormatter)
    parser.add_argument('--seq1_del',      "-s1", help="Allow deletions in sequence 1.", default=False, action="store_true" )
    parser.add_argument('--seq2_del',      "-s2", help="Allow deletions in sequence 2.", default=False, action="store_true" )
    parser.add_argument('--seq1_max',      "-m1", help="Maximum subsequence length for sequence 1. Defaults to 2.", default=2, type=int, required=False )
    parser.add_argument('--seq2_max',      "-m2", help="Maximum subsequence length for sequence 2. Defaults to 2.", default=2, type=int, required=False )
    parser.add_argument('--seq1_sep',      "-p1", help="Separator token for sequence 1. Defaults to '|'.", default="|", required=False )
    parser.add_argument('--seq2_sep',      "-p2", help="Separator token for sequence 2. Defaults to '|'.", default="|", required=False )
    parser.add_argument('--s1s2_sep',      "-ss", help="Separator token for seq1 and seq2 alignments. Defaults to '}'.", default="}", required=False )
    parser.add_argument('--eps',           "-e",  help="Epsilon symbol.  Defaults to '<eps>'.", default="<eps>", required=False )
    parser.add_argument('--skip',          "-s",  help="Skip/null symbol.  Defaults to '_'.", default="_", required=False )
    parser.add_argument('--write_model',   "-wm", help="Write the model to 'file'.", default=None, required=False )
    parser.add_argument('--write_align',   "-wa", help="Write the alignments to 'file'.", default=None, required=False )
    parser.add_argument('--s1in_sep',      "-i1", help="Separator for seq1 in the input training file. Defaults to ''.", default="", required=False )
    parser.add_argument('--s2in_sep',      "-i2", help="Separator for seq2 in the input training file. Defaults to ' '.", default=" ", required=False )
    parser.add_argument('--s1s2_delim',     "-d", help="Separator for seq1/seq2 in the input training file. Defaults to '\\\t'.", default="\t", required=False )
    parser.add_argument('--model',          "-m", help="Input FST-based alignment model.", default=None, required=False )
    parser.add_argument('--align',          "-a", help="File containing sequences to be aligned.", default=None, required=False )
    parser.add_argument('--nbest',          "-n", help="n-best. Defaults to 1.", default=1, type=int, required=False )
    parser.add_argument('--iter',           "-i", help="Maximum number of iterations for EM. Defaults to 11.", default=11, type=int, required=False )
    parser.add_argument('--threshold',      "-t", help="Threshold for EM training. Defaults to '.001'.", default=.001, type=float, required=False )
    parser.add_argument('--string1',       "-r1", help="Input string1 to align.", required=False )
    parser.add_argument('--string2',       "-r2", help="Input string2 to align.", required=False )
    parser.add_argument('--no_penalty',    "-np", help="By default multi-subsequences are penalized during decoding.  Not needed for large training corpora.  Defaults to 'False'.", required=False, default=False, action="store_true" )
    parser.add_argument('--write_lattice', "-l",  help="Write out the union of the weighted alignment lattices from the training corpus.", default=None, required=False )
    parser.add_argument('--prefix',    "-p", help="Prefix used to generate the model and alignment files.  Defaults to 'test'.", required=False, default="test" )
    parser.add_argument('--verbose',   "-v", help='Verbose mode.', default=False, action="store_true")
    args = parser.parse_args()

    if args.verbose:
        for attr, value in args.__dict__.iteritems():
            print attr, value

    #aligner = M2MFstAligner( False, True, 2, 2, "|", "|", "}", "<eps>","_")
    if args.align:
        #Initialize a new aligner object for training
        aligner = M2MFstAligner( 
            args.seq1_del, 
            args.seq2_del, 
            args.seq1_max, 
            args.seq2_max, 
            args.seq1_sep, 
            args.seq2_sep, 
            args.s1s2_sep, 
            args.eps, 
            args.skip,
            args.no_penalty
        )
        #Run the EM training
        train_aligner( 
            aligner, 
            args.align, 
            args.iter, 
            s1in_sep=args.s1in_sep, 
            s2in_sep=args.s2in_sep, 
            s1s2in_sep=args.s1s2_delim
            )
        #Optionally write the model to disk
        if args.write_model:
            aligner.write_model( args.write_model )
        #Optionally write the n-best alignments for the training corpus
        if args.write_align:
            write_aligned_training_data( aligner, args.write_align, nbest=args.nbest )
        #Optionally write out the union of the weighted alignment lattices
        # useful for building better joint n-gram models and MBR decoding.
        if args.write_lattice:
            aligner.write_lattice( args.write_lattice )
    elif args.model:
        #Iinitialize a new aligner object for testing
        aligner = M2MFstAligner( args.model )
        #Align a pair of sequences supplied via the command line
        align_sequences( 
            aligner, 
            args.string1, 
            args.string2, 
            s1in_sep=args.s1in_sep, 
            s2in_sep=args.s2in_sep, 
            nbest=args.nbest,
            write_lattice=args.write_lattice
            )
    else:
        print "You need to specify a valid command sequence..."
