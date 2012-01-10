#!/usr/bin/python
import re, math, sys

class Arpa2WFST( ):
    """
       Class to convert an ARPA-format LM to WFST format.
       
       NOTE: This class will convert arbitrary length n-gram models, but
             it does not perform special handling of missing back-off
             weights except in the case of the </s> (sentence-end) 
             marker.
       NOTE: If your model contains '-' as a regular symbol, make sure you
             change the eps symbol - or you will be in for a world of hurt!
    """

    def __init__( self, arpaifile, prefix="test", eps="<eps>", max_order=4, multi_sep="|", io_sep="}" ):
        self.arpaifile = arpaifile
        self.arpaofile = "PREFIX.fst.txt".replace("PREFIX",prefix)
        self.ssyms    = set([])
        self.isyms    = set([])
        self.osyms    = set([])
        self.eps      = eps
        self.order    = 0
        self.multi_sep = multi_sep
        self.io_sep   = io_sep
        self.max_order = max_order

    def print_syms( self, syms, ofile, reserved=[] ):
        """
           Print out a symbols table.
        """

        ofp = open(ofile,"w")
        offset = 0
        for sym in reserved:
            ofp.write("%s %d\n"%(sym, offset))
            offset += 1
        for i,sym in enumerate(syms):
            ofp.write("%s %d\n"%(sym,i+offset))
        ofp.close()
        return

    def to_tropical( self, val ):
        """
           Convert values to the tropical semiring.
        """
        logval = math.log(10.0) * float(val) * -1.0
        return logval

    def make_arc( self, istate, ostate, isym, osym, weight=0.0 ):
        """
           Build a single arc.  Add symbols to the symbol tables
           as necessary, but ignore epsilons.
        """
        if not istate==self.eps: self.ssyms.add(istate)
        if not ostate==self.eps: self.ssyms.add(ostate)
        itoks = isym.split(self.multi_sep)
        gtoks = []
        for t in itoks:
            if not t=="_":
                gtoks.append(t)
        if not isym=="_":
            isym = self.multi_sep.join(gtoks)
        else:
            isym = self.eps

        if not isym==self.eps: self.isyms.add(isym)
        if not osym==self.eps: self.osyms.add(osym)
        arc = "%s\t%s\t%s\t%s\t%f\n" % (istate, ostate, isym, osym, self.to_tropical(weight))
        return arc

    def arpa2fst( self ):
        """
           Convert an arbitrary length ARPA-format n-gram LM to WFST format.
        """

        arpa_ifp = open(self.arpaifile, "r")
        arpa_ofp = open(self.arpaofile, "w")
        arpa_ofp.write(self.make_arc( "<start>", "<s>", "<s>", "<s>", 0.0 ))
        for line in arpa_ifp:
            line = line.strip()
            #Process based on n-gram order
            if self.order>0 and not line=="" and not line.startswith("\\"):
                parts = re.split(r"\s+",line)
                if self.order==1:
                    if parts[1]=="</s>":
                        arpa_ofp.write( self.make_arc( self.eps, "</s>", "</s>", "</s>", parts[0] ) )
                    elif parts[1]=="<s>":
                        arpa_ofp.write( self.make_arc( "<s>", self.eps, self.eps, self.eps, parts[2] ) )
                    elif len(parts)==3:
                        arpa_ofp.write( self.make_arc( parts[1], self.eps, self.eps, self.eps, parts[2] ) )
                        g,p = parts[self.order].split(self.io_sep)
                        arpa_ofp.write( self.make_arc( self.eps, parts[1], g, p, parts[0] ) )
                    else:
                        sys.stderr.write("%s\n"%(line))
                elif self.order<self.max_order:
                    if parts[self.order]=="</s>":
                        arpa_ofp.write( self.make_arc( ",".join(parts[1:self.order]), parts[self.order], parts[self.order], parts[self.order], parts[0] ) )
                    elif len(parts)==self.order+2:
                        arpa_ofp.write( self.make_arc( ",".join(parts[1:self.order+1]), ",".join(parts[2:self.order+1]), self.eps, self.eps, parts[-1] ) )
                        g,p = parts[self.order].split(self.io_sep)
                        arpa_ofp.write( self.make_arc( ",".join(parts[1:self.order]), ",".join(parts[1:self.order+1]), g, p, parts[0] ) )
                    else:
                        arpa_ofp.write( self.make_arc( ",".join(parts[1:self.order+1]), ",".join(parts[2:self.order+1]), self.eps, self.eps, 0.0 ) )
                elif self.order==self.max_order:
                    if parts[self.order]=="</s>":
                        arpa_ofp.write( self.make_arc( ",".join(parts[1:self.order]), parts[self.order], parts[self.order], parts[self.order], parts[0] ) )
                    else:
                        g,p = parts[self.order].split(self.io_sep)
                        arpa_ofp.write( self.make_arc( ",".join(parts[1:self.order]), ",".join(parts[2:self.order+2]), g, p, parts[0] ) )
                else:
                    pass
            #Check the current n-gram order
            if line.startswith("ngram"):
                pass
            elif line.startswith("\\data"):
                pass
            elif line.startswith("\\end"):
                pass
            elif line.startswith("\\"):
                self.order = int(line.replace("\\","").replace("-grams:",""))
                
        arpa_ifp.close()
        arpa_ofp.write("</s>\n")
        arpa_ofp.close()
        return

if __name__=="__main__":
    import sys, os
    arpa = Arpa2WFST( sys.argv[1], prefix=sys.argv[2], max_order=int(sys.argv[3]) )
    arpa.arpa2fst( )
    arpa.print_syms( arpa.ssyms, "%s.ssyms"%(sys.argv[2]), reserved=[arpa.eps] )
    arpa.print_syms( arpa.isyms, "%s.isyms"%(sys.argv[2]), reserved=[arpa.eps,arpa.multi_sep] )
    arpa.print_syms( arpa.osyms, "%s.osyms"%(sys.argv[2]), reserved=[arpa.eps] )
    command = "fstcompile --ssymbols=PREFIX.ssyms --isymbols=PREFIX.isyms --keep_isymbols --osymbols=PREFIX.osyms --keep_osymbols PREFIX.fst.txt > PREFIX.fst".\
        replace("PREFIX",sys.argv[2])
    print command
    os.system(command)
