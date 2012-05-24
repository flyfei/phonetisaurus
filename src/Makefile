# Makefile
#
# Copyright (c) [2012-], Josef Robert Novak
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
#  modification, are permitted #provided that the following conditions
#  are met:
#
#  * Redistributions of source code must retain the above copyright 
#    notice, this list of conditions and the following disclaimer.
#    * Redistributions in binary form must reproduce the above 
#    copyright notice, this list of #conditions and the following 
#    disclaimer in the documentation and/or other materials provided 
#    with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
# COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
# INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
# STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED 
# OF THE POSSIBILITY OF SUCH DAMAGE.
CC=g++
LIBS=-lfst -ldl -lm
OUT=../phonetisaurus-g2p ../phonetisaurus-align ../phonetisaurus-arpa2fst
TMP=*.o
EXTRA=-O2

POBJS=Phonetisaurus.o FstPathFinder.o MBRDecoder.o 
MOBJS=M2MFstAligner.o FstPathFinder.o MBRDecoder.o 

all: phonetisaurus-g2p phonetisaurus-align phonetisaurus-arpa2fst

Phonetisaurus.o: Phonetisaurus.cpp
	$(CC) $(EXTRA) Phonetisaurus.cpp -c -o Phonetisaurus.o

MBRDecoder.o: MBRDecoder.cpp
	$(CC) $(EXTRA) MBRDecoder.cpp -c -o MBRDecoder.o

FstPathFinder.o: FstPathFinder.cpp
	$(CC) $(EXTRA) FstPathFinder.cpp -c -o FstPathFinder.o

Arpa2Fst.o: Arpa2Fst.cpp
	$(CC) $(EXTRA) Arpa2Fst.cpp -c -o Arpa2Fst.o

M2MFstAligner.o: M2MFstAligner.cpp
	$(CC) $(EXTRA) M2MFstAligner.cpp -c -o M2MFstAligner.o

phonetisaurus-align: FstPathFinder.o M2MFstAligner.o phonetisaurus-align.cpp
	$(CC) $(EXTRA) $(MOBJS) phonetisaurus-align.cpp -o ../phonetisaurus-align $(LIBS)

phonetisaurus-g2p: FstPathFinder.o Phonetisaurus.o MBRDecoder.o phonetisaurus-g2p.cpp
	$(CC) $(EXTRA) $(POBJS) phonetisaurus-g2p.cpp -o ../phonetisaurus-g2p $(LIBS)

phonetisaurus-arpa2fst: Arpa2Fst.o phonetisaurus-arpa2fst.cpp
	$(CC) $(EXTRA) Arpa2Fst.o phonetisaurus-arpa2fst.cpp -o ../phonetisaurus-arpa2fst $(LIBS)

clean:
	rm $(OUT) $(TMP)
