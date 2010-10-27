#!/usr/bin/python
import simplejson, re, math

class probGapAligner( ):
    """
       Implementation of the Needleman/Wunsch Algorithm
       The algorithm has been modified to employ
       a penalty matrix based on phoneme->grapheme
       transition probabilities in the training data.

       A very accessible description of the basic algorithm, which 
       uses static match/mismatch/gap penalties rather than a 
       transition matrix, can be found here:

       http://www.avatar.se/molbioinfo2001/dynprog/adv_dynamic.html
    """
    
    def __init__( self, s2o_file, match_score=1.0, gap_penalty=-.1 ):
        self.seq1  = []
        self.seq2  = []
        self.s2o   = self._load_json_from_file( s2o_file )
        self.trace = []
        self.score = 0.0
        self.aligned = []
        self.match_score = match_score
        self.gap_penalty = gap_penalty

    def _init_trace( self, i, j ):
        """
           Initialize the traceback matrix.
           By doing this manually we avoid a nasty and useless
            NumPy dependency.
        """
        trace = [ [ [0.0,0.0,0.0] for k in xrange(j) ] for l in xrange(i) ]
        return trace

    def _load_json_from_file( self, infile ):
        """
           Load a json object from a file.
        """
        if type(infile).__name__=="str":
            json_obj = simplejson.loads(open(infile,"r").read( ))
            return json_obj
        return infile

    def dic_print( self ):
        """
           Print the aligned entries out in 
           standard CMU dictionary format.
        """
        s1 = [ x[0] for x in self.aligned ]
        s2 = [ x[1] for x in self.aligned ]
        print "%s\t%s" % ("".join(s2), " ".join(s1) )
        return

    def pprint_aligned( self ):
        """
           "Pretty-Print" the aligned entries in a 
           format that emphasizes the alignment results.
        """
        s1 = [ x[0] for x in self.aligned ]
        s2 = [ x[1] for x in self.aligned ]
        s3 = [ x[2] for x in self.aligned ]
        print " ".join(s1)
        print " ".join(s3)
        print " ".join(s2)
        return

    def traceback( self, i, j ):
        """ 
           Perform the traceback through the traceback matrix,
           determining the optimal alignment path associated with 
           best alignment score.
        """
        pi = self.trace[i][j][1]
        pj = self.trace[i][j][2]
        if i>0 and j>0:
            if pi == i-1 and pj == j-1:
                self.aligned.append( (self.seq1[i], self.seq2[j], " ") )
            elif pi == i-1 and pj == j:
                self.aligned.append( (self.seq1[i], "_", " ") )
            else:
                self.aligned.append( ("_", self.seq2[j], " ") )
            self.traceback(self.trace[i][j][1], self.trace[i][j][2])
        else:
            self.aligned.reverse()
            return
        return

    def gap_align( self ):
        """
           Perform the gap-based Needleman/Wunsch alignment routine,
            using the supplied penalty matrix.
        """
        for i,s1 in enumerate(self.seq1):
            if i==0 : continue
            for j,s2 in enumerate(self.seq2):
                if j==0 : continue
                #Do we have a match? Instead of static penalties
                # we look up the score in our transition matrix.
                match = self.s2o[self.seq1[i]][self.seq2[j]]

                #What is the best decision?
                m1 = self.trace[i-1][j-1][0] + match
                m2 = self.trace[i][j-1][0] + self.gap_penalty
                m3 = self.trace[i-1][j][0] + self.gap_penalty
                if m1 >= m2 and m1 >= m3:
                    self.trace[i][j] = [ m1, i-1, j-1 ]
                elif m2 >= m1 and m2 >= m3:
                    self.trace[i][j] = [ m2, i, j-1 ]
                else:
                    self.trace[i][j] = [ m3, i-1, j ]
        return

class basicGapAligner( ):
    """
       Implementation of the Needleman/Wunsch Algorithm.
       This is the 'basic' version which follows the description to the letter 
       and employs static match/mismatch/gap penalties for scoring.

       It will produce identical results to those described in the tutorial below,

       http://www.avatar.se/molbioinfo2001/dynprog/adv_dynamic.html
    """
    def __init__( self, seq1, seq2, match_score=2.0, mismatch_penalty=-1.0, gap_penalty=-2.0 ):
        self.seq1  = seq1
        self.seq2  = seq2
        self.trace = self._init_trace( len(self.seq1), len(self.seq2) )
        self.score = 0.0
        self.aligned = []
        self.match_score      = match_score
        self.mismatch_penalty = mismatch_penalty
        self.gap_penalty      = gap_penalty

    def _init_trace( self, i, j ):
        trace = [ [ [0.0,0.0,0.0] for k in xrange(j) ] for l in xrange(i) ]
        return trace

    def pprint_aligned( self ):
        s1 = [ x[0] for x in self.aligned ]
        s2 = [ x[1] for x in self.aligned ]
        s3 = [ x[2] for x in self.aligned ]
        print " ".join(s1)
        print " ".join(s3)
        print " ".join(s2)

    def traceback( self, i, j ):
        pi = self.trace[i][j][1]
        pj = self.trace[i][j][2]
        if i>0 and j>0:
            if pi == i-1 and pj == j-1:
                if self.seq1[i]==self.seq2[j]:
                    self.aligned.append( (self.seq1[i], self.seq2[j], "|") )
                else:
                    self.aligned.append( (self.seq1[i], self.seq2[j], " ") )
            elif pi == i-1 and pj == j:
                self.aligned.append( (self.seq1[i], "_", " ") )
            elif pi == i and pj == j-1:
                self.aligned.append( ("_", self.seq2[j], " ") )
                
            self.traceback(self.trace[i][j][1], self.trace[i][j][2])
        else:
            self.aligned.reverse()
            self.score = self.trace[-1][-1][0]
            return
        return

    def gap_align( self ):
        """Perform the gap-based alignment routine"""
        for i,s1 in enumerate(self.seq1):
            if i==0 : continue
            for j,s2 in enumerate(self.seq2):
                if j==0 : continue
                #Do we have a match?
                match = 0.0
                if self.seq1[i]==self.seq2[j]:
                    match = self.match_score
                else:
                    match = self.mismatch_penalty

                #What is the best decision?
                m1 = self.trace[i-1][j-1][0] + match
                m2 = self.trace[i][j-1][0] + self.gap_penalty
                m3 = self.trace[i-1][j][0] + self.gap_penalty
                if m1 >= m2 and m1 >= m3:
                    self.trace[i][j] = [ m1, i-1, j-1 ]
                elif m2 >= m1 and m2 >= m3:
                    self.trace[i][j] = [ m2, i, j-1 ]
                else:
                    self.trace[i][j] = [ m3, i-1, j ]
        return

def generate_s2o( dicfile, eq=True, penalty_type="logodds" ):
    """
       Initialize a new state-to-observation matrix. 
       This relates phonemes to graphemes based on counts. 
    """

    dp = open(dicfile,"r")
    phons = set([])
    grphs = set([])
    total = 0.0
    pairs = []
    skipped = []
    for line in dp:
        line = line.strip()
        word, pron = line.split("\t")
        seq1 = list(word)
        seq2 = pron.split()
        if eq==True and not len(seq1)==len(seq2):
            skipped.append( (seq2,seq1) )
            continue

        grphs = grphs.union(seq1)
        phons = phons.union(seq2)
        pairs.append( (seq2,seq1) )
    dp.close()

    s2o = {}
    #Simple add-one smoothing
    for ph in phons:
        s2o[ph] = {"total":0.0}
        for gr in grphs:
            s2o[ph][gr] = 1.0
            s2o[ph]["total"] += 1.0
            total += 1.0

    for pair in pairs:
        if len(pair[0])==len(pair[1]):
            for i,ph in enumerate(pair[0]):
                s2o[ph][pair[1][i]] += 1.0
                s2o[ph]["total"] += 1.0
                total += 1.0

    for ph in s2o:
        for gr in s2o[ph]:
            if gr=="total": continue
            if penalty_type=="logodds":
                #compute the log-odds scores for the transitions
                m_ij = s2o[ph][gr] / s2o[ph]["total"]
                p_j  = s2o[ph]["total"] / total
                s2o[ph][gr] = math.log(m_ij/p_j)
            else:
                #compute probability scores
                m_ij = s2o[ph][gr] / s2o[ph]["total"]
                s2o[ph][gr] = m_ij
            
    return s2o, pairs, skipped

def align_pairs( s2o, pairs, gp=-.7 ):
    """Perform alignment on a dictionary."""
    gp = probGapAligner( s2o, gap_penalty=-2.0 )
    aligned_pairs = []
    for pair in pairs:
        seq1 = pair[0]; seq1.insert(0,"");
        seq2 = pair[1]; seq2.insert(0,"");
        gp.seq1 = seq1
        gp.seq2 = seq2
        gp.trace = gp._init_trace( len(seq1), len(seq2) )
        gp.gap_align()
        gp.traceback(len(seq1)-1,len(seq2)-1)
        aseq1 = [ x[0] for x in gp.aligned ]
        aseq2 = [ x[1] for x in gp.aligned ]

        aligned_pairs.append( (aseq1,aseq2) )
        gp.score = gp.trace[-1][-1][0]
        gp.aligned = []
    return aligned_pairs

def print_dic( pairs, filename ):
    """Print an intermediate dictionary."""
    fp = open(filename,"w")
    for pair in pairs:
        fp.write( "%s\t%s\n" % ("".join(pair[1])," ".join(pair[0])) )
    fp.close()
    return

def iterative_alignment( basefile, root="./", basename="aligned" ):
    """
       Perform an iterative alignment of the dictionary.
       
       NOTE: At present this is set heuristically, however
             it should really employ a convergence criterion.
       TODO: Implement convergence criterion.
    """
    print "Iteration 1. Only equal length pairs..."
    s2o, pairs, skipped = generate_s2o( basefile, eq=True)
    aligned_pairs = align_pairs( s2o, pairs, gp=0.02 )
    aligned_pairs.extend( skipped )
    print_dic(aligned_pairs, root+basename+"-v1.dic")
    print "Iteration 2. Only equal length pairs..."
    s2o, pairs, skipped = generate_s2o( root+basename+"-v1.dic", eq=True)
    aligned_pairs = align_pairs( s2o, pairs )
    aligned_pairs.extend( skipped )
    print_dic(aligned_pairs, root+basename+"-v2.dic")
    print "Iteration 3..."
    s2o, pairs, skipped = generate_s2o( root+basename+"-v2.dic", eq=False)
    aligned_pairs = align_pairs( s2o, pairs, gp=-.3 )
    aligned_pairs.extend( skipped )
    print_dic(aligned_pairs, root+basename+"-v3.dic")
    print "Iteration 4..."
    s2o, pairs, skipped = generate_s2o( root+basename+"-v3.dic", eq=False)
    aligned_pairs = align_pairs( s2o, pairs )
    aligned_pairs.extend( skipped )
    print_dic(aligned_pairs, root+basename+"-v4.dic")
    print "Iteration 5..."
    s2o, pairs, skipped = generate_s2o( root+basename+"-v4.dic", eq=False)
    aligned_pairs = align_pairs( s2o, pairs, gp=-1.0 )
    aligned_pairs.extend( skipped )
    print_dic(aligned_pairs, root+basename+"-v5.dic")
    
    return root+basename+"-v5.dic"

def generate_joint_corpus( aligned_dic ):
    """Generate a joint model for LM setup."""
    fp = open(aligned_dic,"r")
    op = open("models/aligned_corpus.txt","w")
    for line in fp:
        line = line.strip()
        word, pron = line.split("\t")
        phons = pron.split()
        corpus_line = " ".join([ "%s:%s" % (phons[i], word[i]) for i in xrange(len(phons)) ])
        op.write(corpus_line+"\n")
    fp.close()
    op.close()
    return


if __name__=="__main__":
    import sys, argparse

    dic_file = iterative_alignment( sys.argv[1], root="models/" )
    generate_joint_corpus( dic_file )

