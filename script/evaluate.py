#!/usr/bin/python
#
# Copyright (c) [2012-], Josef Robert Novak
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
#  modification, are permitted #provided that the following conditions
#  are met:
#
#  * Redistributions of source code must retain the above copyright 
#    notice, this list of conditions and the following disclaimer.
#    * Redistributions in binary form must reproduce the above 
#    copyright notice, this list of #conditions and the following 
#    disclaimer in the documentation and/or other materials provided 
#    with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
# COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
# INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
# STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED 
# OF THE POSSIBILITY OF SUCH DAMAGE.
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

def evaluate_testset( 
    modelfile, wordlistfile, referencefile, hypothesisfile, verbose=False, ignore="", ignore_both=False, regex_ignore="", 
    mbrdecode="", alpha=.65, precision=.85, ratio=.72, order=6
    ):
    """
      Evaluate the Word Error Rate (WER) for the test set.
      Each word should only be evaluated once.  The word is counted as 
      'correct' if the pronunciation hypothesis exactly matches at least
      one of the pronunciations in the reference file.
      WER is then computed as:
         (1.0 - (WORDS_CORRECT / TOTAL_WORDS))
    """

    if verbose: print "Executing evaluation with command:"
    command = "../phonetisaurus-g2p --model=%s --input=%s --alpha=%0.4f --prec=%0.4f --ratio=%0.4f --order=%d --words --isfile %s > %s" \
        % (modelfile, wordlistfile, alpha, precision, ratio, order, mbrdecode, hypothesisfile)
    print command
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
    parser.add_argument('--testfile',     "-t", help="The test file in dictionary format. 1 word, 1 pronunciation per line, separated by '\\t'.", required=True )
    parser.add_argument('--prefix',       "-p", help="Prefix used to generate the wordlist, hypothesis and reference files.  Defaults to 'test'.", required=False )
    parser.add_argument('--modelfile',    "-m", help="Path to the phoneticizer model.", required=True )
    parser.add_argument('--mbrdecode',    "-e", help="Use the LMBR decoder.", default=False, action="store_true" )
    parser.add_argument('--alpha',        "-a", help="Alpha for the mbr decoder.", default=.65, type=float )
    parser.add_argument('--order',        "-o", help="N-gram order for the mbr decoder.", default=6, type=int )
    parser.add_argument('--precision',    "-x", help="Avg. N-gram precision factor for LMBR decoder. (.85)", default=.85, type=float )
    parser.add_argument('--ratio',        "-y", help="N-gram ratio factor for LMBR decoder. (.72)", default=.72, type=float )
    parser.add_argument('--ignore',       "-i", help="Ignore ' ' separated chars in list. Applied only to Hypothesis by default. (false)", required=False, default="" )
    parser.add_argument('--ignore_both',  "-b", help="Ignore chars in --ignore both for Hypotheses AND References. (false)", default=False, action="store_true" )
    parser.add_argument('--regex_ignore', "-r", help="Ignore chars in the regex.  Applied only to Hypotheses by default.", required=False, default="" )
    parser.add_argument('--verbose',      "-v", help='Verbose mode.', default=False, action="store_true")
    args = parser.parse_args()

    if args.mbrdecode==False:
        args.mbrdecode=""
    else:
        args.mbrdecode="--mbr"

    if args.verbose:
        for attr, value in args.__dict__.iteritems():
            print attr, value
    wordlist = "%s.words"%( args.prefix ); hyp_file = "%s.hyp"%(args.prefix); ref_file = "%s.ref"%(args.prefix)
    
    process_testset( args.testfile, wordlist, ref_file )
    evaluate_testset( 
        args.modelfile, wordlist, ref_file, 
        hyp_file, verbose=args.verbose, ignore=args.ignore, 
        ignore_both=args.ignore_both, regex_ignore=args.regex_ignore,
        mbrdecode=args.mbrdecode, alpha=args.alpha, precision=args.precision, ratio=args.ratio, order=args.order 
        ) 
