#!/usr/bin/python
#! -*- mode: python; coding: utf-8 -*-
import getopt, logging
import math, sys, re
from collections import defaultdict
from operator import itemgetter
from datetime import datetime

class simpleLM( ):
    """
       A simple LM generation class.  Performs trivial backoff computation for 3-gram language models.
       This script can output LMs in ARPA format.  
       It is only suitable for small corpora, and is intended as a learning tool.

       This is basically just a python port of the quick_lm.pl perl script.
             http://www.speech.cs.cmu.edu/tools/download/quick_lm.pl
    """
    def __init__( self, infile, outfile=None, discount=0.5, log10=math.log(10.0) ):
        self.n = 3 #This is currently the only order supported
        self.infile        = infile
        self.infile_fp     = open(infile,"r")
        self.outfile_fp    = self._get_outfile_fp( outfile )
        self.log10         = log10
        self.discount_mass = self._fix_discount( discount )
        self.deflator      = 1.0 - self.discount_mass
        self.sent_count    = 0
        #Hard-wired to tri-gram models
        self.trigrams = defaultdict(int)
        self.bigrams  = defaultdict(int)
        self.unigrams = defaultdict(int)
        #Counters
        self.unicounts = 0.
        self.unisum   = 0.
        self.bicounts  = 0.
        self.tricounts = 0.
        #Probs
        self.uniprobs  = {}
        self.alphas    = {}
        self.biprobs   = {}
        self.bialphas  = {}
        
    def generate_lm( self ):
        """Generate the Language Model."""
        self._count_ngrams()
        self._compute_alphas()
        self._compute_bialphas()
        return
    
    def print_arpa_lm( self ):
        """Print the LM out in ARPA format."""
        self._print_header()
        self._print_alphas()
        self._print_bialphas()
        self._print_trigrams()
        self._print_footer()
        return
    
    def _print_header( self ):
        self.outfile_fp.write("Language model created by QuickLM.py on %s\n" % (datetime.now().isoformat()))
        self.outfile_fp.write("Copyright (c) 2009-2010 - Josef Robert Novak\n\n")
        self.outfile_fp.write("This model is based on a corpus of %d sentences and %d words.\n" % (self.sent_count, self.unisum))
        self.outfile_fp.write("The (fixed) discount mass is %f.\n\n" % self.discount_mass)
        self.outfile_fp.write("\\data\\\ngram 1=%d\nngram 2=%d\nngram 3=%d\n\n" % (len(self.unigrams),len(self.bigrams),len(self.trigrams)))
        return

    def _print_footer( self ):
        self.outfile_fp.write("\\end\\\n")

    def _get_outfile_fp( self, outfile ):
        if outfile:
            return open(outfile,"w")
        else:
            return sys.stdout

    def _fix_discount( self, discount ):
        """Enforce discount requirements."""
        if discount <= 0.0 or discount >= 1.0:
            return 0.5
        else:
            return discount

    def _count_ngrams( self ):
        """Counts n-grams."""
        for line in self.infile_fp:
            line = line.strip()
            line = "<s> "+line+" </s>"
            line = re.sub(r"\s+"," ",line)
            if line=="": 
                continue
            self.sent_count += 1
            line = unicode(line,"utf8")
            words = re.split(r"\s+",line)
            #Get the n-grams
            for j in xrange(0,len(words)-2):
                tri = u" ".join(words[j:j+3]).strip()
                self.trigrams[tri] += 1
                bi  = u" ".join(words[j:j+2]).strip()
                self.bigrams[bi] += 1
                uni = words[j].strip()
                self.unigrams[uni] += 1
            #Get the leftovers
            bi  = u" ".join(words[len(words)-2:]).strip()
            self.bigrams[bi] += 1
            uni = words[len(words)-2].strip()
            self.unigrams[uni] += 1
            uni = words[len(words)-1].strip()
            self.unigrams[uni] += 1
            #Total number of words
            self.unisum += len(words)
        
        #Total the counts
        self.tricounts  = len(self.trigrams.keys())
        self.bicounts   = len(self.bigrams.keys())
        self.unicounts = len(self.unigrams.keys())
        self.infile_fp.close()
        
        return

    def _compute_uni_prob( self, uni ):
        """Compute unigram probabilities."""
        if not self.uniprobs.has_key(uni):
            self.uniprobs[uni] = (self.unigrams[uni] / self.unisum) * self.deflator
        return

    def _compute_alphas( self ):
        """Compute the alphas."""
        for uni in self.unigrams.keys():
            w1 = uni
            self._compute_uni_prob(w1)
            sum_denom = 0.
            for bi in self.bigrams:
                if bi.find(w1)==0:
                    w2 = bi[len(w1)+1:].strip()
                    self._compute_uni_prob(w2)
                    sum_denom += self.uniprobs[w2]
            self.alphas[w1] = self.discount_mass / (1.0 - sum_denom)

    def _print_alphas( self ):
        """Print the alphas."""
        self.outfile_fp.write("\\1-grams:\n")
        unikeys = self.unigrams.keys()
        unikeys.sort()
        for uni in unikeys:
            self.outfile_fp.write("%6.4f %s %6.4f\n" % (math.log(self.uniprobs[uni])/self.log10, uni.encode("utf8"), math.log(self.alphas[uni])/self.log10))
        self.outfile_fp.write("\n")

    def _compute_bi_prob( self, bi ):
        """Compute bi probs."""
        if not self.biprobs.has_key(bi):
            w1 = bi[:bi.find(" ")]
            self.biprobs[bi] = (float(self.bigrams[bi])*self.deflator)/float(self.unigrams[w1])
        return

    def _compute_bialphas( self ):
        """Compute the bigram backoff weights."""
        for bi in self.bigrams:
            w1w2 = bi
            self._compute_bi_prob(w1w2)
            sum_denom = 0.
            for tri in self.trigrams:
                if tri.find(w1w2)==0:
                    w2w3 = tri[tri.find(" ")+1:]
                    self._compute_bi_prob(w2w3)
                    sum_denom += self.biprobs[w2w3]
            self.bialphas[w1w2] = self.discount_mass / (1.0 - sum_denom)

    def _print_bialphas( self ):
        """Print the bigrams."""
        if self.bicounts > 0:
            self.outfile_fp.write("\\2-grams:\n")
            bikeys = self.bigrams.keys()
            bikeys.sort()
            for bi in bikeys:
                self.outfile_fp.write("%6.4f %s %6.4f\n" % (math.log(self.biprobs[bi])/self.log10, bi.encode("utf8"), math.log(self.bialphas[bi])/self.log10))
            self.outfile_fp.write("\n")
        return

    def _print_trigrams( self ):
        """Print the trigrams."""
        if self.tricounts > 0:
            self.outfile_fp.write("\\3-grams:\n")
            trikeys = self.trigrams.keys()
            trikeys.sort()
            for tri in trikeys:
                w1w2 = tri[:tri.rfind(" ")]
                self.outfile_fp.write("%6.4f %s\n" % (math.log((self.trigrams[tri]*self.deflator)/self.bigrams[w1w2])/self.log10, tri.encode("utf8")))
            self.outfile_fp.write("\n")
            
if __name__=="__main__":
    import sys
    lmObj = simpleLM( infile=sys.argv[1] )
    lmObj.generate_lm( )
    lmObj.print_arpa_lm( )
