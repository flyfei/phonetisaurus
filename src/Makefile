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
UNAME := $(shell uname)
ifeq ($(UNAME), Darwin)
LIBS=-lfst -ldl -lm
endif
ifeq ($(UNAME), Linux)
LIBS=-lfst -ldl -lm -lrt
endif

OUT=../phonetisaurus-g2p ../phonetisaurus-align \
	../phonetisaurus-arpa2fst ../phonetisaurus-arpa2wfst-omega \
	../phonetisaurus-g2p-omega
TMP=*.o
CFLAGS ?= -O2
THIRD_PARTIES_INCLUDE ?= -I3rdparty/sparsehash -I3rdparty/utfcpp -I3rdparty/rnnlm
EXTRA=$(CFLAGS) $(THIRD_PARTIES_INCLUDE) $(CXXFLAGS) $(CPPFLAGS) 

POBJS=Phonetisaurus.o FstPathFinder.o MBRDecoder.o \
	LatticePruner.o util.o
MOBJS=M2MFstAligner.o FstPathFinder.o MBRDecoder.o \
	LatticePruner.o util.o
OOBJS=PhonetisaurusOmega.o PhonetisaurusE2F.o util.o \
	FstPathFinder.o LatticePruner.o MBRDecoder.o


all: $(OUT)

%.o: %.cpp
	$(CC) $(EXTRA) -c $(<) -o $(@)

../phonetisaurus-align: $(MOBJS) phonetisaurus-align.cpp
	$(CC) $(EXTRA) $(LDFLAGS) $(MOBJS) phonetisaurus-align.cpp -o ../phonetisaurus-align $(LIBS)

../phonetisaurus-g2p: $(POBJS) phonetisaurus-g2p.cpp
	$(CC) $(EXTRA) $(LDFLAGS) $(POBJS) phonetisaurus-g2p.cpp -o ../phonetisaurus-g2p $(LIBS)

../phonetisaurus-arpa2fst: Arpa2Fst.o util.o phonetisaurus-arpa2fst.cpp
	$(CC) $(EXTRA) $(LDFLAGS) Arpa2Fst.o util.o phonetisaurus-arpa2fst.cpp -o ../phonetisaurus-arpa2fst $(LIBS)

../phonetisaurus-arpa2wfst-omega: ARPA2WFST.o util.o phonetisaurus-arpa2wfst-omega.cpp
	$(CC) $(EXTRA) $(LDFLAGS) ARPA2WFST.o util.o \
	phonetisaurus-arpa2wfst-omega.cpp -o ../phonetisaurus-arpa2wfst-omega $(LIBS)

../phonetisaurus-g2p-omega: $(OOBJS) phonetisaurus-g2p-omega.cpp
	$(CC) $(EXTRA) $(LDFLAGS) $(OOBJS) phonetisaurus-g2p-omega.cpp -o ../phonetisaurus-g2p-omega $(LIBS)


# Some toy RnnLM bindings
VERSION=2.7
PYTHON_H=/usr/include/python$(VERSION)
DEST=..
ifeq ($(UNAME), Darwin)
SONAME=$(DEST)/librnnlm.dylib
RT=
endif
ifeq ($(UNAME), Linux)
SONAME=$(DEST)/librnnlm.so
RT=-lrt
endif

rnnlm-bindings: $(DEST)/RnnLM.so

$(DEST)/RnnLM.so: $(SONAME) RnnLMPy-binding.o
	$(CC) -shared $(RT) -fPIC RnnLMPy-binding.o -o $(DEST)/RnnLM.so -L.. \
	-I3rdparty/rnnlm rnnlmlib.o -lpython$(VERSION) -lrnnlm -lfst -O3 \
	-funroll-loops -ffast-math -lm

$(DEST)/librnnlm.so: RnnLMPy-binding.o
	$(CC) -Wl,-soname,librnnlm.so.0 -shared -lfst RnnLMPy-binding.o \
	rnnlmlib.o -I. -o $(DEST)/librnnlm.so -O3 -fPIC -funroll-loops -ffast-math -lm
	cd ..; ln -s librnnlm.so librnnlm.so.0; cd -;

$(DEST)/librnnlm.dylib: RnnLMPy-binding.o
	$(CC) -Wl,-install_name,librnnlm.so -shared -lc -lfst RnnLMPy-binding.o \
	rnnlmlib.o -I. -o $(DEST)/librnnlm.dylib -O3 -fPIC -funroll-loops \
	-ffast-math -lm -lpython$(VERSION)

RnnLMPy-binding.o: RnnLMPy-binding.cc
	$(CC) -O3 -fPIC -funroll-loops -ffast-math -lm -I. -I3rdparty/rnnlm -I$(PYTHON_H) -c $(<) -o $(@)

RnnLMPy-binding.cc: rnnlmlib.o
	python rnnlm-module.py > RnnLMPy-binding.cc

rnnlmlib.o : 3rdparty/rnnlm/rnnlmlib.cpp
	$(CC) -O3 -fPIC -funroll-loops -ffast-math -lm -fPIC -w -c 3rdparty/rnnlm/rnnlmlib.cpp

clean:
	$(RM) $(OUT) $(TMP)

clean-bindings:
	$(RM) $(DEST)/*.so $(DEST)/*.so.0 rnnlmlib.o RnnLMPy-binding.o RnnLMPy-binding.cc
