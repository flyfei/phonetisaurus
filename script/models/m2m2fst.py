#!/usr/bin/python
import sys, re

def m2m2fst( word, cf ):

    fp = open(cf,"r")
    clusters = set([])
    for line in fp:
        line = line.strip()
        clusters.add(line)
    fp.close()


    tokens = word[:]
    arcs = []
    for i,t in enumerate(tokens):
        if i==0: continue
        sub = tokens[i-1]+tokens[i]
        if sub in clusters:
            arcs.append( "%d %d %s&%s" % (i, i+2, tokens[i-1], tokens[i]) )


    graphs = list(word)
    state = 1
    word_fst = ""
    word_fst += "0 1 <s>\n"
    state = 1
    for gr in graphs:
        word_fst += "%d %d %s\n"%(state, state+1, gr) 
        state += 1
    word_fst += "%d %d </s>\n" % (state, state+1) 
    word_fst += "%d\n"%(state+1)
    word_fst += "\n".join(arcs)
    return word_fst

fst = m2m2fst( sys.argv[1], sys.argv[2] )
print fst
