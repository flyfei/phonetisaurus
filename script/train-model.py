#!/usr/bin/python
import re, os
from m2m2format import m2m2Format
from m2m2corpus import m2m2Corpus
from arpa2fst import Arpa2WFST


def trainModel( args ):
    """
      Train the G2P/P2G model and transform it into a WFST suitable for use with
        phonetisaurus-g2p
    """
    
    check_prefix( args.prefix )

    #Format the input dictionary file
    m2m2Format( 
        args.dict, 
        prefix=args.prefix, 
        graph_sep=args.graphsep, 
        phon_sep=args.phonsep, 
        entry_sep=args.entrysep, 
        reverse=args.reverse, 
        swap=args.swap 
        )

    #Build up the m2m-aligner command and run it
    command = """m2m-aligner DELX DELY --maxX MAXX --maxY MAXY --maxFn MAXFN --sepInChar "|" --sepChar " " -i PREFIX.train -o PREFIX.align""".\
        replace("MAXX",str(args.maxX)).replace("MAXY",str(args.maxY)).replace("MAXFN",args.maxFn).replace("PREFIX",args.prefix)
    if args.delX==True:
        command = command.replace("DELX","--delX")
    else:
        command = command.replace("DELX","")
    if args.delY==True:
        command = command.replace("DELY","--delY")
    else:
        command = command.replace("DELY","")
    print command
    if args.noalign==False:
        os.system(command)

    #Format the m2m-aligner results for LM training
    m2m2Corpus( "PREFIX.align".replace("PREFIX",args.prefix), prefix=args.prefix )
    
    #Build up the mitlm command and run it
    command = "estimate-ngram -s SMOOTH -o ORDER -t PREFIX.corpus -wl PREFIX.arpa".\
        replace("ORDER",str(args.order)).replace("PREFIX",args.prefix).replace("SMOOTH",args.smoothing)
    print command
    os.system(command)

    #Convert the LM to WFST format and compile it
    arpa = Arpa2WFST( "PREFIX.arpa".replace("PREFIX",args.prefix), prefix=args.prefix, max_order=args.order )
    arpa.arpa2fst( )
    arpa.print_syms( arpa.ssyms, "%s.ssyms"%(args.prefix) )
    arpa.print_syms( arpa.isyms, "%s.isyms"%(args.prefix) )
    arpa.print_syms( arpa.osyms, "%s.osyms"%(args.prefix) )
    command = "fstcompile --ssymbols=PREFIX.ssyms --isymbols=PREFIX.isyms --keep_isymbols --osymbols=PREFIX.osyms --keep_osymbols PREFIX.fst.txt > PREFIX.fst".\
        replace("PREFIX",args.prefix)
    print command
    os.system(command)    

    return

def check_prefix( prefix ):
    path, fname = os.path.split(prefix)
    if not path=="":
        if not os.path.exists(path):
            print "Creating dir: %s" % path
            os.makedirs(path)
    return

if __name__=="__main__":
    import sys, re, argparse

    example = """./train-model.py --dict training.dic --delX --maxX 2 --maxY 2 --conFn joint --prefix test --order 6"""
    parser = argparse.ArgumentParser(description=example)
    parser.add_argument('--dict',     "-d", help="The input pronunciation dictionary.  This will be used to build the G2P/P2G model.", required=True )
    parser.add_argument('--graphsep', "-g", help="The grapheme separator for the raw dictionary file. Defaults to ''.", default="" )
    parser.add_argument('--phonsep',  "-e", help="The phoneme separator for the raw dictionary file. Defaults to ' '.", default=" " )
    parser.add_argument('--entrysep', "-u", help="The entry separator for the raw dictionary file. Defaults to '\t'.", default="\t" )
    parser.add_argument('--reverse',  "-r", help="Reverse the training data. 'word'->'drow'.", default=False, action="store_true" )
    parser.add_argument('--swap',     "-w", help="Swap the grapheme/phoneme input. G2P->P2G.", default=False, action="store_true" )
    parser.add_argument('--delX',     "-a", help="m2m-aligner option: Allow deletions of left-hand (input/grapheme) tokens.", default=False, action="store_true")
    parser.add_argument('--delY',     "-b", help="m2m-aligner option: Allow deletions of right-hand (output/phoneme) tokens.", default=False, action="store_true")
    parser.add_argument('--noalign',  "-n", help="Skip the alignment step.  Useful for testing different N-gram models.", default=False, action="store_true")
    parser.add_argument('--maxX',     "-x", help="m2m-aligner option: Maximum substring length for left-hand (input/grapheme) tokens.", default=2 )
    parser.add_argument('--maxY',     "-y", help="m2m-aligner option: Maximum substring length for right-hand (outut/phoneme) tokens.", default=2 )
    parser.add_argument('--maxFn',    "-c", help="m2m-aligner option: Maximization function.  May be one of 'conYX', 'conXY', or 'joint'.", default="joint" )
    parser.add_argument('--prefix',   "-p", help="A file prefix.  Will be prepended to all model files created during cascade generation.", default="test" )
    parser.add_argument('--order',    "-o", help="mitlm option: Maximum order of the joint N-gram LM", default=6 )
    parser.add_argument('--smoothing',"-s", help="mitlm option: Specify smoothing algorithms.  (ML, FixKN, FixModKN, FixKN#, KN, ModKN, KN#)", default="FixKN" )
    parser.add_argument('--verbose',  "-v", help='Verbose mode.', default=False, action="store_true")
    args = parser.parse_args()

    if args.verbose:
        for attr, value in args.__dict__.iteritems():
            print attr, value
        
    trainModel( args )
