#!/usr/bin/python
import sys

total=0.0
error=0.0
fp = open(sys.argv[1],"r")
for line in fp:
    line = line.strip()
    score,hyp,con = line.split("\t")
    if not hyp==con:
        error += 1.0
    total += 1.0
fp.close()
print "WER: %0.3f" % (error/total)
