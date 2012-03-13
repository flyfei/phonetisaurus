# Makefile
#    Copyright 2011-2012 Josef Robert Novak
#
#    This file is part of Phonetisaurus.
#
#    Phonetisaurus is free software: you can redistribute it 
#    and/or modify it under the terms of the GNU General Public 
#    License as published by the Free Software Foundation, either 
#    version 3 of the License, or (at your option) any later version.
#
#    Phonetisaurus is distributed in the hope that it will be useful, but 
#    WITHOUT ANY WARRANTY; without even the implied warranty of 
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
#    General Public License for more details.
#
#    You should have received a copy of the GNU General Public 
#    License along with Phonetisaurus. If not, see http://www.gnu.org/licenses/.
#
CC=g++ -O2
LIBS=-lfst -ldl -lm
OUT=phonetisaurus-g2p m2m-fst-aligner
TMP=*.o
EXTRA= #add any -I -L modifications here...

POBJS=Phonetisaurus.o FstPathFinder.o
MOBJS=M2MFstAligner.o FstPathFinder.o

all: phonetisaurus-g2p m2m-fst-aligner

Phonetisaurus.o: Phonetisaurus.cpp
	$(CC) $(EXTRA) Phonetisaurus.cpp -c -o Phonetisaurus.o

FstPathFinder.o: FstPathFinder.cpp
	$(CC) $(EXTRA) FstPathFinder.cpp -c -o FstPathFinder.o

M2MFstAligner.o: M2MFstAligner.cpp
	$(CC) $(EXTRA) M2MFstAligner.cpp -c -o M2MFstAligner.o

m2m-fst-aligner: FstPathFinder.o M2MFstAligner.o m2m-fst-aligner.cpp
	$(CC) $(EXTRA) $(MOBJS) m2m-fst-aligner.cpp -o m2m-fst-aligner $(LIBS)

phonetisaurus-g2p: FstPathFinder.o Phonetisaurus.o phonetisaurus-g2p.cpp
	$(CC) $(EXTRA) $(POBJS) phonetisaurus-g2p.cpp -o phonetisaurus-g2p $(LIBS)

clean:
	rm $(OUT) $(TMP) M2MFstAligner_wrap.cxx M2MFstAligner.py _M2MFstAligner.so
