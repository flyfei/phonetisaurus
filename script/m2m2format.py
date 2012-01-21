#!/usr/bin/python
import re, sys, os

def m2m2Format( dict_file, prefix="test", graph_sep="", phon_sep=" ", entry_sep="\t", reverse=False, swap=False, unify_case=False ):
    """
      Format the raw dictionary file for the m2m alignment tool.
    """

    if graph_sep==entry_sep:
        raise ValueError("graph_sep and entry_sep share the same value!")
    elif phon_sep==entry_sep:
        raise ValueError("phon_sep and entry_sep share the same value!")

    dict_file_fp = open( dict_file, "r" )
    train_file_fp = open( "PREFIX.train".replace("PREFIX",prefix), "w" )
    chars = set([])
    for line in dict_file_fp:
        line = line.strip()
        line = line.decode("utf8")
        word = None; pron = None
        graphs = []; phons = [];
        
        word, pron = line.split(entry_sep)
        if unify_case==True:
            word = word.lower()

        #We will map spaces, dashes, underscores, etc. categorically
        # to "" during training.  These will be auto-mapped to <eps>
        # during decoding.
        word = word.replace(" ","").replace("_","").replace("-","")
        if graph_sep=="":
            graphs = list(word)
        else:
            graphs = word.split(graph_sep)
        for g in graphs:
            chars.update( [ x for x in list(g) ])

        if phon_sep=="":
            phons = list(pron)
        else:
            phons = pron.split(phon_sep)
        for p in phons:
            chars.update( [ x for x in list(p) ])

        if reverse==True:
            graphs.reverse()
            phons.reverse()

        if swap==True:
            outline = "%s\t%s\n" % (" ".join(phons), " ".join(graphs))
        else:
            outline = "%s\t%s\n" % (" ".join(graphs), " ".join(phons))
        outline = outline.encode("utf8")
        train_file_fp.write(outline)
    dict_file_fp.close()
    train_file_fp.close()

    return chars


if __name__=="__main__":
    import sys, argparse

    example = """./m2m2format.py --dict training.dic --prefix test"""
    parser = argparse.ArgumentParser(description=example)
    parser.add_argument('--dict',     "-d", help="The input pronunciation dictionary.  This will be used to build the G2P/P2G model.", required=True )
    parser.add_argument('--graphsep', "-g", help="The grapheme separator for the raw dictionary file. Defaults to ''.", default="" )
    parser.add_argument('--phonsep',  "-e", help="The phoneme separator for the raw dictionary file. Defaults to ' '.", default=" " )
    parser.add_argument('--reverse',  "-r", help="Reverse the training data. 'word'->'drow'.", default=False, action="store_true")
    parser.add_argument('--entrysep', "-y", help="The word/pronunciation separator for the raw dictionary file. Defaults to '\t'.", default="\t" )
    parser.add_argument('--swap',     "-w", help="Swap the grapheme/phomeme input.  G2P vs. P2G model.", default=False, action="store_true" )
    parser.add_argument('--prefix',   "-p", help="A file prefix.  Will be prepended to all model files created during cascade generation.", default="test" )
    parser.add_argument('--verbose',  "-v", help='Verbose mode.', default=False, action="store_true")
    args = parser.parse_args()

    if args.verbose:
        for attr, value in args.__dict__.iteritems():
            print attr, value
        
    m2m2Format( 
        args.dict, 
        prefix=args.prefix, 
        graph_sep=args.graphsep, 
        phon_sep=args.phonsep, 
        entry_sep=args.entrysep, 
        reverse=args.reverse, 
        swap=args.swap 
        )

