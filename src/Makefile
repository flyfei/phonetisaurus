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
CFLAGS ?= -O2
THIRD_PARTIES_INCLUDE ?= -I3rdparty/sparsehash -I3rdparty/utfcpp
EXTRA=$(CFLAGS) $(THIRD_PARTIES_INCLUDE) $(CXXFLAGS) $(CPPFLAGS) 

POBJS=Phonetisaurus.o FstPathFinder.o MBRDecoder.o LatticePruner.o util.o
MOBJS=M2MFstAligner.o FstPathFinder.o MBRDecoder.o LatticePruner.o util.o

all: $(OUT)

%.o: %.cpp
	$(CC) $(EXTRA) -c $(<) -o $(@)

../phonetisaurus-align: $(MOBJS) phonetisaurus-align.cpp
	$(CC) $(EXTRA) $(LDFLAGS) $(MOBJS) phonetisaurus-align.cpp -o ../phonetisaurus-align $(LIBS)

../phonetisaurus-g2p: $(POBJS) phonetisaurus-g2p.cpp
	$(CC) $(EXTRA) $(LDFLAGS) $(POBJS) phonetisaurus-g2p.cpp -o ../phonetisaurus-g2p $(LIBS)

../phonetisaurus-arpa2fst: Arpa2Fst.o util.o phonetisaurus-arpa2fst.cpp
	$(CC) $(EXTRA) $(LDFLAGS) Arpa2Fst.o util.o phonetisaurus-arpa2fst.cpp -o ../phonetisaurus-arpa2fst $(LIBS)

clean:
	rm $(OUT) $(TMP)
