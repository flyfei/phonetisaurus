#!/usr/bin/python
import re, math

class Arpa2WFST( ):
    """
       Class to convert an ARPA-format LM to WFST format.
       
       NOTE: This class is hard-wired to the 3-gram case, and 
             does not perform special handling of missing back-off
             weights except in the case of the </s> (sentence-end) 
             marker.
    """

    def __init__( self, arpaifile, arpaofile, eps="-", max_order=4 ):
        self.arpaifile = arpaifile
        self.arpaofile = arpaofile
        self.ssyms    = set([])
        self.isyms    = set([])
        self.osyms    = set([])
        self.eps      = eps
        self.order    = 0
        self.max_order = max_order

    def print_syms( self, syms, ofile ):
        """
           Print out a symbols table.
        """

        ofp = open(ofile,"w")
        ofp.write("- 0\n")
        for i,sym in enumerate(syms):
            ofp.write("%s %d\n"%(sym,i+1))
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
        
        if not isym==self.eps: self.isyms.add(isym)
        if not osym==self.eps: self.osyms.add(osym)
        arc = "%s\t%s\t%s\t%s\t%f\n" % (istate, ostate, isym, osym, self.to_tropical(weight))
        return arc

    def arpa2fst_orig( self ):
        """
           Convert a 3-gram ARPA-format LM to WFST format.
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
                        p,g = parts[1].split(":")
                        arpa_ofp.write( self.make_arc( self.eps, parts[1], g, p, parts[0] ) )
                    else:
                        pass
                elif self.order==2:
                    if parts[2]=="</s>":
                        arpa_ofp.write( self.make_arc( parts[1], parts[2], parts[2], parts[2], parts[0] ) )
                    elif len(parts)==4:
                        arpa_ofp.write( self.make_arc( "%s,%s"%(parts[1],parts[2]), parts[2], self.eps, self.eps, parts[3] ) )
                        p,g = parts[2].split(":")
                        arpa_ofp.write( self.make_arc( parts[1], "%s,%s"%(parts[1],parts[2]), g, p, parts[0] ) )
                    else:
                        pass
                elif self.order==3:
                    #if parts[3]=="</s>":
                    #    arpa_ofp.write( self.make_arc( "%s,%s"%(parts[1],parts[2]), parts[3], parts[3], parts[3], parts[0] ) )
                    #else:
                    #    p,g = parts[3].split(":")
                    #    arpa_ofp.write( self.make_arc( "%s,%s"%(parts[1],parts[2]), "%s,%s"%(parts[2],parts[3]), g, p, parts[0] ) )
                    if parts[3]=="</s>":
                        arpa_ofp.write( self.make_arc( "%s,%s"%(parts[1],parts[2]), parts[3], parts[3], parts[3], parts[0] ) )
                    elif len(parts)==5:
                        arpa_ofp.write( self.make_arc( "%s,%s,%s"%(parts[1],parts[2],parts[3]), "%s,%s"%(parts[2],parts[3]), self.eps, self.eps, parts[4] ) )
                        p,g = parts[3].split(":")
                        arpa_ofp.write( self.make_arc( "%s,%s"%(parts[1],parts[2]), "%s,%s,%s"%(parts[1],parts[2],parts[3]), g, p, parts[0] ) )
                elif self.order==4:
                    if parts[4]=="</s>":
                        arpa_ofp.write( self.make_arc( "%s,%s,%s"%(parts[1],parts[2],parts[3]), parts[4], parts[4], parts[4], parts[0] ) )
                    else:
                        p,g = parts[4].split(":")
                        arpa_ofp.write( self.make_arc( "%s,%s,%s"%(parts[1],parts[2],parts[3]), "%s,%s,%s"%(parts[2],parts[3],parts[4]), g, p, parts[0] ) )
                else:
                    pass
            #Check the current n-gram order
            if line.startswith("ngram"):
                pass
            elif line.startswith("\\1-grams:"):
                self.order = 1
            elif line.startswith("\\2-grams:"):
                self.order = 2
            elif line.startswith("\\3-grams:"):
                self.order = 3
            elif line.startswith("\\4-grams:"):
                self.order = 4
        arpa_ifp.close()
        arpa_ofp.write("</s>\n")
        arpa_ofp.close()
        return

    def arpa2fst( self ):
        """
           Convert a 3-gram ARPA-format LM to WFST format.
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
                        p,g = parts[1].split(":")
                        arpa_ofp.write( self.make_arc( self.eps, parts[1], g, p, parts[0] ) )
                    else:
                        pass
                elif self.order<self.max_order:
                    if parts[self.order]=="</s>":
                        arpa_ofp.write( self.make_arc( ",".join(parts[1:self.order]), parts[self.order], parts[self.order], parts[self.order], parts[0] ) )
                    elif len(parts)==self.order+2:
                        arpa_ofp.write( self.make_arc( ",".join(parts[1:self.order+1]), ",".join(parts[2:self.order+1]), self.eps, self.eps, parts[-1] ) )
                        p,g = parts[self.order].split(":")
                        arpa_ofp.write( self.make_arc( ",".join(parts[1:self.order]), ",".join(parts[1:self.order+1]), g, p, parts[0] ) )
                    else:
                        pass
                elif self.order==self.max_order:
                    if parts[self.order]=="</s>":
                        arpa_ofp.write( self.make_arc( ",".join(parts[1:self.order]), parts[self.order], parts[self.order], parts[self.order], parts[0] ) )
                    else:
                        p,g = parts[self.order].split(":")
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
    arpa = Arpa2WFST( sys.argv[1], sys.argv[5], max_order=int(sys.argv[7]) )
    arpa.arpa2fst( )
    arpa.print_syms( arpa.ssyms, sys.argv[2] )
    arpa.print_syms( arpa.isyms, sys.argv[3] )
    arpa.print_syms( arpa.osyms, sys.argv[4] )
    command = "fstcompile --ssymbols=%s --isymbols=%s --osymbols=%s %s > %s" % ( sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5], sys.argv[6] )
    os.system(command)
