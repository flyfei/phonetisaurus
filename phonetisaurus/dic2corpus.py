#!/usr/bin/python
import sys

fp = open(sys.argv[1],"r")
for line in fp:
    line = line.strip()
    word,pron = line.split("\t")
    seq1 = list(word)
    seq2 = pron.split()
    entry = []
    for i,ph in enumerate(seq2):
        entry.append("%s:%s"%(ph,seq1[i]))
    print " ".join(entry)
fp.close()

