#!/usr/bin/python
from phonetisaurus import Phonetisaurus
import sys, os


def LoadTestSet (testfile) :
    words = []
    for entry in open (testfile, "r") :
        words.append(entry.strip())
    return words

def PhoneticizeWord (model, word, nbest, band, prune, write) :
    prons = model.Phoneticize (word, nbest, band, prune, write)
    for pron in prons :
        phones = " ".join([ model.FindOsym (p) for p in pron.Uniques ])
        print "{0}\t{1:.4f}\t{2}".format (word, pron.PathWeight, phones)
    return 

def PhoneticizeTestSet (model, words, nbest, band, prune, write) :

    for word in words :
        PhoneticizeWord (model, word, nbest, band, prune)

    return

if __name__ == "__main__" :
    import sys, argparse

    example = "USAGE: {0} --model g2p.fst --testset test.txt".format (sys.argv[0])
    parser  = argparse.ArgumentParser (description = example)

    group   = parser.add_mutually_exclusive_group (required=True)
    group.add_argument ("--testset", "-t", help="Input test set to evaluate.")
    group.add_argument ("--word", "-w", help="Input word to evaluate.")

    parser.add_argument ("--model",   "-m", help="Input G2P model.", required=True)
    parser.add_argument ("--nbest",   "-n", help="N-best results.", default=1, type=int)
    parser.add_argument ("--band",    "-b", help="Band for n-best search", default=10000, type=int)
    parser.add_argument ("--prune",   "-p", help="Pruning threshold for n-best.", default=99, type=float)
    parser.add_argument ("--write",   "-r", help="Write out the FSTs.", default=False, action="store_true")
    parser.add_argument ("--verbose", "-v", help="Verbose mode", default=False, action="store_true")
    args = parser.parse_args ()
    
    if args.verbose :
        for k,v in args.__dict__.iteritems ():
            print k, "=", v

    #Load the G2P model
    g2p    = Phonetisaurus (args.model)

    words  = []
    if args.testset :
        words  = LoadTestSet (args.testset)
        PhoneticizeTestSet (g2p, words, args.nbest, args.band, args.prune, args.write)
    else :
        PhoneticizeWord (g2p, args.word, args.nbest, args.band, args.prune, args.write)
