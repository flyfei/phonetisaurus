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
import re, operator, os
from collections import defaultdict
from calculateER import ErrorRater

def process_testset( testfile, wordlist_out, reference_out, verbose=False ):
    """
      Process the testfile, a normal dictionary, output a wordlist for testing,
      and a reference file for results evaluation.  Handles cases where a single
      word has multiple pronunciations.
    """
    
    if verbose: print "Preprocessing the testset dictionary file..."
    test_dict = defaultdict(list)
    for entry in open(testfile,"r"):
        word, pron = entry.strip().split("\t")
        test_dict[word].append(pron)

    wordlist_ofp  = open(wordlist_out,"w")
    reference_ofp = open(reference_out,"w")
    test_list = sorted(test_dict.iteritems(), key=operator.itemgetter(0))
    for entry in test_list:
        wordlist_ofp.write("%s\n"%entry[0])
        reference_ofp.write("%s\t%s\n"%(entry[0],"\t".join(entry[1])))
    wordlist_ofp.close()
    reference_ofp.close()
    return

def evaluate_testset( modelfile, wordlistfile, referencefile, hypothesisfile, verbose=False, ignore="", ignore_both=False, regex_ignore="", mbrdecode="", alpha=1.0, theta=1.0, order=2 ):
    """
      Evaluate the Word Error Rate (WER) for the test set.
      Each word should only be evaluated once.  The word is counted as 
      'correct' if the pronunciation hypothesis exactly matches at least
      one of the pronunciations in the reference file.
      WER is then computed as:
         (1.0 - (WORDS_CORRECT / TOTAL_WORDS))
    """

    if verbose: print "Executing evaluation with command:"
    command = "../phonetisaurus-g2p -b 1500 -m %s -o -t %s %s -a %0.4f -x %0.4f -d %d > %s" % (modelfile, wordlistfile,  mbrdecode, alpha, theta, order, hypothesisfile)
    if verbose: print command
    os.system(command)
    references = {}
    for entry in open(referencefile,"r"):
        parts = entry.strip().split("\t")
        word  = parts.pop(0)
        references[word] = parts
    for entry in open(hypothesisfile,"r"):
        word, score, hypothesis = entry.strip().split("\t")

    PERcalculator = ErrorRater( ignore=ignore, ignore_both=ignore_both, regex_ignore=regex_ignore )
    PERcalculator.compute_PER_phonetisaurus( hypothesisfile, referencefile, verbose=verbose )

    return
    
    
if __name__=="__main__":
    import sys, argparse


    example = """%s --modelfile someg2pmodel.fst --testfile test.dic --wordlist_out test.words --reference_out test.ref --hypothesisfile test.hyp""" % sys.argv[0]
    parser = argparse.ArgumentParser(description=example)
    parser.add_argument('--testfile',  "-t", help="The test file in dictionary format. 1 word, 1 pronunciation per line, separated by '\\t'.", required=True )
    parser.add_argument('--prefix',    "-p", help="Prefix used to generate the wordlist, hypothesis and reference files.  Defaults to 'test'.", required=False )
    parser.add_argument('--modelfile', "-m", help="Path to the phoneticizer model.", required=True )
    parser.add_argument('--mbrdecode', "-e", help="Use the mbr decoderl.", default="" )
    parser.add_argument('--alpha', "-a", help="Alpha for the mbr decoder.", default=.65, type=float )
    parser.add_argument('--order', "-o", help="N-gram order for the mbr decoder.", default=5, type=int )
    parser.add_argument('--theta', "-x", help="Uniform theta for the MBR decoder (should be order specific though...).", default=1.0, type=float )
    parser.add_argument('--ignore', "-i", help="Ignore chars in list.  Chars should be ' ' separated.  Applied only to Hypotheses by default.", required=False, default="" )
    parser.add_argument('--ignore_both', "-b", help="Ignore chars in --ignore both for Hypotheses AND References.", default=False, action="store_true" )
    parser.add_argument('--regex_ignore', "-r", help="Ignore chars in the regex.  Applied only to Hypotheses by default.", required=False, default="" )
    parser.add_argument('--verbose',   "-v", help='Verbose mode.', default=False, action="store_true")
    args = parser.parse_args()

    if args.verbose:
        for attr, value in args.__dict__.iteritems():
            print attr, value
    wordlist = "%s.words"%( args.prefix ); hyp_file = "%s.hyp"%(args.prefix); ref_file = "%s.ref"%(args.prefix)
    
    process_testset( args.testfile, wordlist, ref_file )
    evaluate_testset( 
        args.modelfile, wordlist, ref_file, 
        hyp_file, verbose=args.verbose, ignore=args.ignore, 
        ignore_both=args.ignore_both, regex_ignore=args.regex_ignore,
        mbrdecode=args.mbrdecode, alpha=args.alpha, theta=args.theta, order=args.order 
        ) 
