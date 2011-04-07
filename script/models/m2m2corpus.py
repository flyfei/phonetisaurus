#!/usr/bin/python
import sys
fp = open(sys.argv[1],"r")
op = open("clusters.list","w")
clusters = set([])
for line in fp:
    line = line.strip()
    word, pron = line.split("\t")
    word = word.replace(":","&")
    word = word.rstrip("|")
    pron = pron.replace(":","&")
    pron = pron.rstrip("|")
    graphs = word.split("|")
    phons  = pron.split("|")
    for g in graphs:
        if "&" in g:
            clusters.add(g)
    g2p = []
    for i,g in enumerate(graphs):
        g2p.append("%s:%s"%(phons[i],g))
    print " ".join(g2p)
fp.close()

for c in clusters:
    op.write("%s\n"%c.replace("&",""))
op.close()
