#!/usr/bin/python
from M2MFstAligner import M2MFstAligner
import sys

def train_aligner( aligner, training_file, max_iter, ofile, nbest=0, seq1_sep="", seq2_sep=" ", s1s2_sep="\t" ):
    print "Building WFST-based alignment corpus from training data..."
    for line in open(training_file,"r"):
        part1, part2 = line.strip().split(s1s2_sep)
        seq1 = []; seq2 = [];
        
        if seq1_sep=="":
            seq1 = list(part1)
        else:
            seq1 = part1.split(seq1_sep)
        if seq2_sep=="":
            seq2 = list(part2)
        else:
            seq2 = part2.split(" ")
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

    print "Finished training!"
    if nbest==0:
        print "Will write alignment model to disk."
    elif nbest>0:
        print "Will write %d-best alignments to file: %s." % (nbest,ofile)
        ofp = open(ofile,"w")
        for i in xrange(aligner.num_fsas()):
            results = aligner.write_alignment( i, nbest )
            for result in results:
                ofp.write("%s\n"%" ".join(result.path))
        ofp.close()
    
if __name__=="__main__":
    import sys, argparse


    example = """%s --modelfile s""" % sys.argv[0]
    parser = argparse.ArgumentParser(description=example)
    parser.add_argument('--seq1_del',    "-s1", help="Allow deletions in sequence 1.", default=False, action="store_true" )
    parser.add_argument('--seq2_del',    "-s2", help="Allow deletions in sequence 2.", default=False, action="store_true" )
    parser.add_argument('--seq1_max',    "-m1", help="Maximum subsequence length for sequence 1. Defaults to 2.", default=2, type=int, required=False )
    parser.add_argument('--seq2_max',    "-m2", help="Maximum subsequence length for sequence 2. Defaults to 2.", default=2, type=int, required=False )
    parser.add_argument('--seq1_sep',    "-p1", help="Separator token for sequence 1. Defaults to '|'.", default="|", required=False )
    parser.add_argument('--seq2_sep',    "-p2", help="Separator token for sequence 2. Defaults to '|'.", default="|", required=False )
    parser.add_argument('--s1s2_sep',    "-ss", help="Separator token for seq1 and seq2 alignments. Defaults to '}'.", default="}", required=False )
    parser.add_argument('--eps',         "-e",  help="Epsilon symbol.  Defaults to '<eps>'.", default="<eps>", required=False )
    parser.add_argument('--skip',        "-s",  help="Skip/null symbol.  Defaults to '_'.", default="_", required=False )
    parser.add_argument('--write_model', "-wm", help="Write the model to 'file'.", default="", required=False )
    parser.add_argument('--write_align', "-wa", help="Write the alignments to 'file'.", default="", required=False )
    parser.add_argument('--s1in_sep',    "-i1", help="Separator for seq1 in the input training file. Defaults to ''.", default="", required=False )
    parser.add_argument('--s2in_sep',    "-i2", help="Separator for seq2 in the input training file. Defaults to ' '.", default=" ", required=False )
    parser.add_argument('--s1s2_delim',   "-d", help="Separator for seq1/seq2 in the input training file. Defaults to '\\\t'.", default="\t", required=False )
    parser.add_argument('--model',        "-m", help="Input FST-based alignment model.", default=None, required=False )
    parser.add_argument('--align',        "-a", help="File containing sequences to be aligned.", default=None, required=False )
    parser.add_argument('--nbest',        "-n", help="n-best. Defaults to 1.", default=1, type=int, required=False )
    parser.add_argument('--iter',         "-i", help="Maximum number of iterations for EM. Defaults to 11.", default=11, type=int, required=False )
    parser.add_argument('--threshold',    "-t", help="Threshold for EM training. Defaults to '.001'.", default=.001, type=float, required=False )
    parser.add_argument('--prefix',    "-p", help="Prefix used to generate the model and alignment files.  Defaults to 'test'.", required=False, default="test" )
    parser.add_argument('--verbose',   "-v", help='Verbose mode.', default=False, action="store_true")
    args = parser.parse_args()

    if args.verbose:
        for attr, value in args.__dict__.iteritems():
            print attr, value

    #aligner = M2MFstAligner( False, True, 2, 2, "|", "|", "}", "<eps>","_")
    aligner = M2MFstAligner( 
        args.seq1_del, 
        args.seq2_del, 
        args.seq1_max, 
        args.seq2_max, 
        args.seq1_sep, 
        args.seq2_sep, 
        args.s1s2_sep, 
        args.eps, 
        args.skip
        )

    train_aligner( 
        aligner, 
        args.align, 
        args.iter, 
        args.write_align, 
        nbest=args.nbest, 
        seq1_sep=args.s1in_sep, 
        seq2_sep=args.s2in_sep, 
        s1s2_sep=args.s1s2_delim
        )
