#!/usr/bin/python
import re

class ErrorRater( ):
    """
      This class provides tools suitable for computing the Word Error Rate (WER)
       or Pronunciation Error Rate (PER) of one or more hypothesis-reference
       transcription pairs.
       
      Specifically it provides functions to compute Levenshtein penalty matrix,
       a non-recursive traceback function, WER/PER computation function, and 
       several formatting convenience functions.
    """

    def __init__( self ):
        #The 'totals' array tracks [ TotalChars, Matches, Substitutions, Insertions, Deletions ] 
        # over a test set.  This information is used to compute WER/PER scores for 
        # the entire test set in the standard manner.
        self.totals = [ 0., 0., 0., 0., 0. ]

        #A running count of the number of hyp/ref pairs that were NOT perfectly aligned
        #For ASR this corresponds to SENTENCE ERRORS, for G2P this corresponds to WORD ERRORS
        self.sequence_errors = 0.
        self.total_sequences = 0.
        
    def compute_penalty_matrix( self, hyp, ref ):
        """
          Compute the penalty matrix using the levenshtein algorithm.  
          The edit distance between the two sequences will be stored 
            in the last entry in the matrix.
        """
        matrix = [ [0 for x in xrange(len(ref)+1)] for y in xrange(len(hyp)+1) ]
        for i in xrange(len(hyp)+1):
            matrix[i][0] = i
        for j in xrange(len(ref)+1):
            matrix[0][j] = j

        for i in xrange(1,len(hyp)+1):
            for j in xrange(1,len(ref)+1):
                if hyp[i-1]==ref[j-1]:
                    matrix[i][j] = matrix[i-1][j-1]
                else:
                    matrix[i][j] = min( matrix[i-1][j]+1, matrix[i][j-1]+1, matrix[i-1][j-1]+1 )

        return [matrix,ref]


    def compute_traceback( self, matrix, hyp, ref ):
        """
          Compute the traceback and align the two input sequences.
          Also compute the Insertions, Deletions, and Substitutions.
          Note that there is often more than one valid alignment for 
           a given alignment score.  In these cases the order of operations
           below determines whether an 'S', 'I' or 'D' will be chosen.
        """
        alignment = []
        # [ TotalChars, Match, Substitution, Insertion, Deletion ]
        scores    = [0.,0.,0.,0.,0.]
    
        i = len(hyp); j = len(ref)
        while i>0 and j>0:
            if hyp[i-1]==ref[j-1]:
                alignment.append( self._normalize([ hyp[i-1], "|", ref[j-1] ]) )
                i-= 1; j-=1; scores[1]+=1
            elif matrix[i][j]==matrix[i-1][j-1]+1:
                alignment.append( self._normalize([ hyp[i-1], " ", ref[j-1] ]) )
                i-=1; j-=1; scores[2]+=1
            elif matrix[i][j]==matrix[i-1][j]+1:
                alignment.append( self._normalize([ hyp[i-1], " ", "*" ]) )
                i-=1; scores[3]+=1
            else:
                alignment.append( self._normalize([ "-", " ", ref[j-1] ]) )
                j-=1; scores[4]+=1
        while i>0:
            alignment.append( self._normalize([ hyp[i-1], " ", "*" ]) )
            i-=1; scores[3]+=1
        while j>0:
            alignment.append( self._normalize([ "-", " ", ref[j-1] ]) )
            j-=1; scores[4]+=1

        alignment.reverse()
        scores[0] = len(ref)
        
        return alignment, scores

    def _normalize( self, units ):
        """
          Normalize the length of the individual units/words being aligned.
          We want each unit of the REF, CONN, and HYP arrays to be the same 
           length, mainly because it makes the alignment output easier on the 
           eyes.
        """

        max_unit = max( units, key=lambda unit:len(unit) )
        for i,unit in enumerate(units):
            diff = len(max_unit) - len(unit)
            units[i] = unit.center(len(unit)+diff, " ")
        
        return units

    def print_alignment( self, alignment, scores ):
        """
          Print out an alignment.  The alignment information is stored in a 
           2D array where each element stores a triple consisting of: 
             [ HypothesisToken, Connector, ReferenceToken ]
          These sequences will be printed out in reverse order so that the 
           reference sequence is on top.
        """
        
        print " ".join([ x[2] for x in alignment ])
        print " ".join([ x[1] for x in alignment ])
        print " ".join([ x[0] for x in alignment ])
        self.print_ER( scores )
        print ""

        return

    def align_sequences( self, hyp, refs, verbose=False ):
        """
          Align a hypothesis sequence with one or more references.
          If more than one reference is supplied, only the one with 
           the smallest edit-distance will be used for alignment and ER 
           calculations.
        """

        if not type(hyp).__name__=="list" or not type(refs).__name__=="list":
            raise TypeError, "Hypothesis and reference(s) must be lists!"

        if type(refs[0]).__name__=="str":
            matrix, ref = self.compute_penalty_matrix( hyp, refs )
        else:
            matrix, ref = min( 
                  [ self.compute_penalty_matrix( hyp, ref ) for ref in refs ], 
                  key=lambda mat:mat[0][-1][-1] 
                )
        alignment, scores = self.compute_traceback( matrix, hyp, ref )
        self.total_sequences += 1
        if matrix[-1][-1]>0: 
            self.sequence_errors += 1
        if verbose: 
            self.print_alignment( alignment, scores )

        self.totals = [ scores[i]+self.totals[i] for i in xrange(len(scores)) ]
    
        return alignment, scores
    
    def split_sequence( self, sequence, usep=" ", fsep="\t" ):
        """
          Split an input string into one or more sequences, and return a 
           1D or 2D array containing the results.
        """

        if not type(sequence).__name__=="str":
            raise TypeError, "Input sequence must be of type string ('str')!"
        
        if usep=="":
            sequences = [
                  [ unit for unit in list(seq) ] 
                    for seq in re.split( fsep, sequence )
                ]
        else:
            sequences = [ 
                  [ unit for unit in re.split( usep, seq ) ] 
                    for seq in re.split( fsep, sequence ) 
                ]
        if len(sequences)==1: 
            return sequences[0]
        else: 
            return sequences

    def compute_WER_Sphinx( self, hypfile, reffile, usep="", fsep="\t", verbose=False ):
        """
          Compute WER for CMU Sphinx style input files.  The basic algorithm is EXACTLY the 
          same as for compute_PER_phonetisaurus.  The only difference being that this 
           function handles formatting issues specific to the CMU Sphinx standard.
        """
        #Not yet implemented
        pass

    def compute_WER_HTK( self, hypfile, reffile, usep="", fsep="\t", verbose=False ):
        """
          Compute WER for HTK style input files.  The basic algorithm is EXACTLY the 
          same as for compute_PER_phonetisaurus.  The only difference being that this 
           function handles formatting issues specific to the HTK standard.
        """
        #Not yet implemented
        pass
    
    def compute_PER_phonetisaurus( self, hypfile, reffile, usep=" ", fsep="\t", verbose=False ):
        """
          Compute the total PER for Phonetisaurus input test files.
          Standard Phonetisaurus output format is:
            hypfile: 
             WORD SCORE PRON
            reffile:
             WORD PRON1, ..., PRONn
          Fields are separated by 'tab', and phoneme tokens in each PRON are 
           separated by a single space, ' '.
          
          PER is computed in the standard manner:
              (S+I+D)/(T)
           where T=TotalTokens, S=Substitutions, I=Insertions, D=Deletions
        """

        words = []; hyps = []; refs = []
        for line in open(hypfile,"r"):
            #There should be three fields
            word, score, pron = line.strip().split(fsep)
            hyps.append( re.split(usep, pron) )
            words.append(word)
        for line in open(reffile,"r"):
            #There should be at least 2 fields.  
            # Word, Hyp1, ..., HypN
            fields = line.strip().split(fsep)
            refs.append( [ re.split(usep, field) for field in fields[1:] ] )
            
        #Make sure we have the same number of entries
        assert len(words)==len(hyps) and len(hyps)==len(refs)
        
        for i, word in enumerate(words):
            if verbose: print word
            self.align_sequences( hyps[i], refs[i], verbose=verbose )
            
        self.print_ER(self.totals)

        return

    def print_ER( self, totals ):
        """
          PrettyPrint out the Word/Pronunciation Error Rate and information about
           the relative number of Matches, Substitutions, Insertions and Deletions.
        """
        width = 70
        if totals==self.totals: print "".join(["#" for i in xrange(width)])
        print "EVALUATION RESULTS".center(width," ")
        if totals==self.totals: print "".join(["-" for i in xrange(width)])
        print "(T)otal tokens in reference: %d" % (int(totals[0]))
        print "(M)atches: %d  (S)ubstitutions: %d  (I)nsertions: %d  (D)eletions: %d" %\
            (int(totals[1]), int(totals[2]), int(totals[3]), int(totals[4]))
        print "%% Correct (M/T)           -- %%%0.2f" % ( 100*totals[1]/totals[0] )
        print "%% Token ER ((S+I+D)/T)    -- %%%0.2f" % ( 100*        (sum(totals[2:])/totals[0])   )
        print "%% Accuracy 1.0-ER         -- %%%0.2f" % ( 100*(1.0 - (sum(totals[2:])/totals[0]) ) )
        if totals==self.totals: print "".join(["-" for i in xrange(int((float(width)/5)*4))]).center(width," ")
        print "(S)equences: %d  (C)orrect sequences: %d  (E)rror sequences: %d" %\
            (int(self.total_sequences), int(self.total_sequences - self.sequence_errors), int(self.sequence_errors))
        print "%% Sequence ER (E/S)       -- %%%0.2f" % ( 100*(self.sequence_errors/self.total_sequences) )
        print "%% Sequence Acc (1.0-E/S)  -- %%%0.2f" % ( 100*(1.0-(self.sequence_errors/self.total_sequences)) )
        if totals==self.totals: print "".join(["#" for i in xrange(width)])
        return


if __name__=="__main__":
    import sys, argparse, os

    example = """%s --hyp "hypseq or file" --ref "refseq or file" --usep "" """ % sys.argv[0]
    parser = argparse.ArgumentParser(description=example)
    parser.add_argument('--hyp', "-y", help="The file/string containing G2P/ASR hypotheses.", required=True )
    parser.add_argument('--ref', "-r", help="The file/string containing G2P/ASR reference transcriptions.", required=True )
    parser.add_argument('--usep',     "-u", help="Character or regex separating units in a sequence. Defaults to ' '.", required=False, default=" " )
    parser.add_argument('--fsep',     "-s", help="Character or regex separating fields in a sequence. Defaults to '\\t'.", required=False, default="\t" )
    parser.add_argument('--format',   "-f", help="Input format.  One of 'cmu', 'htk', 'g2p'. Defaults to 'g2p'.", required=False, default="g2p" )
    parser.add_argument('--verbose',  "-v", help='Verbose mode.', default=False, action="store_true")
    args = parser.parse_args()

    if args.verbose:
        for attr, value in args.__dict__.iteritems():
            print attr, "=", value

    error_rater = ErrorRater()
    if os.path.exists( args.hyp ) and os.path.exists( args.ref ):
        if args.format=="g2p":
            error_rater.compute_PER_phonetisaurus( args.hyp, args.ref, usep=args.usep, fsep=args.fsep, verbose=args.verbose )
        elif args.format=="cmu":
            print "CMU format not yet implemented..."
        elif args.format=="htk":
            print "HTK format not yet implemented..."
        else:
            print "--format must be one of 'g2p', 'cmu' or 'htk'."
    else:
        error_rater.align_sequences( 
            error_rater.split_sequence( args.hyp, usep=args.usep, fsep=args.fsep ), 
            error_rater.split_sequence( args.ref, usep=args.usep, fsep=args.fsep ),
            verbose=True
            )
