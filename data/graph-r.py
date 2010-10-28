#!/usr/bin/python
from rpy import *
import Numeric
import os,sys,re

r.postscript(file=sys.argv[1],paper="letter", horizontal=False)
r.par(mar=(4, 4, 2, 1),cex=1.5)
r.plot([2.0,7.0],[0.30,0.75], xlab="n-gram order", ylab="pronunciation WER", type='n', xaxs="i", yaxs="i", cex_axis=1.1,cex_lab=1.1, cex=1.5 )
r.title(main="Phonetisaurus performance on NETTALK", lwd=3.0)
col=["green","blue","red","orange","purple","black","pink","brown","yellow"]

r.lines([2,3,4,5,6,7], [0.700,0.422,0.348, 0.338, 0.329, 0.328], type="l",col=col[0], lty=1, lwd=3.5)
r.lines([2,3,4,5,6,7], [0.721,0.447,0.373, 0.370, 0.363, 0.367], type="l",col=col[1], lty=2, lwd=3.5)

r.legend("topright", ["manual", "needleman-wunsch"], col=["green","blue"], lty=[1,2], cex=0.9, inset=(0.05,0.05), lwd=3.0, title="Alignment algorithm")
r.dev_off()
os.system('epstopdf '+sys.argv[1])
