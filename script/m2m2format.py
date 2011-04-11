#!/usr/bin/python
import re, sys, os

def m2m2Format( dict_file, prefix="test", graph_sep="", phon_sep=" ", entry_sep="\t", g2p=True ):
    """
      Format the raw dictionary file for the m2m alignment tool.
    """

    if graph_sep==entry_sep:
        raise ValueError("graph_sep and entry_sep share the same value!")
    elif phon_sep==entry_sep:
        raise ValueError("phon_sep and entry_sep share the same value!")

    dict_file_fp = open( dict_file, "r" )
    train_file_fp = open( "PREFIX.train".replace("PREFIX",prefix), "w" )

    for line in dict_file_fp:
        line = line.strip()
        line = line.decode("utf8")
        word = None; pron = None
        graphs = []; phons = [];
        
        if g2p==True:
            try:
                word, pron = line.split(entry_sep)
                phons = pron.split(phon_sep)
            except:
                phons = re.split(r"\s+", line)
                word = phons.pop(0)
        else:
            try:
                pron, word = line.split(entry_sep)
                phons = pron.split(phon_sep)
            except:
                phons = re.split(r"\s+", line)
                word = phons.pop(-1)
        
        if graph_sep=="":
            graphs = list(word)
        else:
            graphs = word.split(graph_sep)

        outline = "%s\t%s\n" % (" ".join(graphs), " ".join(phons))
        outline = outline.encode("utf8")
        train_file_fp.write(outline)
    dict_file_fp.close()
    train_file_fp.close()

    return


if __name__=="__main__":
    import sys
    m2m2Format( sys.argv[1], prefix=sys.argv[2] )

