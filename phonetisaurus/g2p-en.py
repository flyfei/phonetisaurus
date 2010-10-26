#!/usr/bin/python
#phoneticizer
import commands, sys


class nbestPhoneticizer():
    """
       The N-best Phoneticizer
       
       Reads in the OpenFST-format WFST pronunciation model
       and generate novel pronunciations based on user input.
    """
    def __init__( self, fstmodel, isyms, osyms ):
        self.fstmodel = fstmodel
        self.isyms    = isyms
        self.osyms    = osyms
        self.results = {}
        self.wfst = {}
        self.word_fst = ""
        self.command_output = ""

    def _reinit( self ):
        self.results = {}
        self.wfst = {}
        self.word_fst = ""
        self.command_output = ""
        return 

    def gen_fst( self, word, n=5 ):
        """
           Generate an FSA for the requested word.
        """
        graphs = list(word)
        state = 1
        self.word_fst += "0 1 <s>\$"
        for gr in graphs:
            self.word_fst += "%d %d %s\$"%(state, state+1, gr) 
            state += 1
        self.word_fst += "%d %d </s>\$" % (state, state+1) 
        self.word_fst += "%d\$"%(state+1)
        command = """echo "%s" | 
                     fstcompile --acceptor=true --isymbols=%s | 
                     fstcompose - %s | 
                     fstproject --project_output=true | 
                     fstshortestpath --nshortest=%s - | 
                     fstsynchronize - | 
                     fstrmepsilon - | 
                     fstprint --isymbols=%s --acceptor=true""" % ( self.word_fst, self.isyms, self.fstmodel, n, self.osyms )
        command = command.replace("\n","").replace("\$","\n")
        self.command_output = commands.getoutput( command )
        return

    def read_nbest( self ):
        """
           Read the n-best WFST output and parse it 
           into a structure suitable for further processing.
        """
        
        onebest = []
        score = 0.0
        for line in self.command_output.split("\n"):
            parts = line.split("\t")
            if len(parts)>2:
                weight=0.0
                if len(parts)==4:
                    weight = float(parts[3])
                if self.wfst.has_key(int(parts[0])):
                    self.wfst[int(parts[0])].append( (int(parts[1]), parts[2], weight) )
                else:
                    self.wfst[int(parts[0])] = [ (int(parts[1]), parts[2], weight) ]
            elif len(parts)==1:
                self.wfst[int(parts[0])] = [ (-99, 0.0) ]
            else:
                self.wfst[int(parts[0])] = [ (-99, float(parts[1])) ]
        
        self.traverse( self.wfst, 0 )

    def traverse(self, wfst, s, results=[], score=0.0):
        """
           Recursively traverse the n-best lattice and return unique
           pronunciations with their associated scores.
        """
        
        for i,arc in enumerate(wfst[s]):
            if s==0:
                results = []; score = 0.0
            if not arc[0]==-99:
                results.append( arc[1] )
                score += arc[2]
                self.traverse(wfst, arc[0], results=results, score=score)
            else:
                if not self.results.has_key(" ".join(results)):
                    self.results[" ".join(results)] = score
                return
        return

    def phoneticize_word( self, word, n=10 ):
        """Phoneticize a single word."""
        self.gen_fst( word, n=n )
        self.read_nbest( )
        hypotheses = []
        for item in sorted(self.results.iteritems(), key=operator.itemgetter(1)):
            pron = item[0].replace("_","").replace("<s>","").replace("</s>","")
            pron = pron.strip()
            pron = re.sub(r"\s+"," ",pron)
            hypotheses.append("%.5f\t%s" % (item[1],pron))
        return hypotheses

    def phoneticize_list( self, wordlist, n=10 ):
        """Phoneticize a list of words."""
        wl_fp = open(wordlist,"r")
        compare = []
        for line in wl_fp:
            line = line.strip()
            if line.find("\t")>-1:
                word, pron = line.split("\t")
                hypos = self.phoneticize_word( word, n=n )
                pron = pron.replace("_","")
                pron = re.sub(r"\s+"," ",pron)
                pron = pron.strip()
                compare.append( (hypos[0],pron) )
                print "%s\t%s" % (hypos[0],pron)
                self._reinit()
        wl_fp.close()
        return compare
            

if __name__=="__main__":
    import sys, operator, re, argparse

    parser = argparse.ArgumentParser()
    parser.add_argument('--word',    "-w", help='Word to transcribe.', required=False)
    parser.add_argument('--file',    "-f", help='List of words to transcribe.', required=False)
    parser.add_argument('--nbest',   "-n", type=int, default=10, help='Maximum number of alternative hypotheses to produce.', required=False)
    parser.add_argument('--model',   "-m", help='OpenFST-based WFST pronunciation model.', required=True)
    parser.add_argument('--isyms',   "-i", help='WFST Input symbols.', required=True)
    parser.add_argument('--osyms',   "-o", help='WFST Output symbols.', required=True)
    args = parser.parse_args()

    phon = nbestPhoneticizer( args.model, args.isyms, args.osyms )
    if args.word and not args.file:
        results = phon.phoneticize_word( args.word, n=args.nbest )
        for result in results:
            print result
    elif args.file and not args.word:
        results = phon.phoneticize_list( args.file, n=args.nbest )
        #for result in results:
        #    print "%s\t\t%s"%(result[0], result[1])
    else:
        print "Please supply exactly one of --file or --word"