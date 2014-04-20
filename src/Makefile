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
VERSION=2.7
PYTHON_H=/usr/include/python$(VERSION)
BINDIR=../bin
LIBDIR=../lib
PYLIB=../phonetisaurus
RNNPY=../rnnlm

CC=g++
UNAME := $(shell uname)
ifeq ($(UNAME), Darwin)
LIBS=-lfst -ldl -lm
RNNSONAME=$(DEST)/librnnlm.dylib
G2PSONAME=$(DEST)/libphonetisaurus.dylib
endif
ifeq ($(UNAME), Linux)
LIBS=-lfst -ldl -lm -lrt
RNNSONAME=$(DEST)/librnnlm.so
G2PSONAME=$(DEST)/libphonetisaurus.so
endif

%.o: %.cc 
	$(CC) $(EXTRA) -c $(<) -o $(@)

OUT=$(OBJS) $(BINDIR)/phonetisaurus-align $(BINDIR)/phonetisaurus-arpa2wfst-omega 

CFLAGS ?= -O2     
THIRD_PARTIES_INCLUDE ?= -I3rdparty/sparsehash -I3rdparty/utfcpp -I3rdparty/rnnlm
EXTRA=$(CFLAGS) $(THIRD_PARTIES_INCLUDE) $(CXXFLAGS) $(CPPFLAGS) 

OBJS=util.o ARPA2WFST.o rnnlmlib.o FstPathFinder.o LatticePruner.o \
	M2MFstAligner.o phonetisaurus-align.o


.PHONY: phonetisaurus-align phonetisaurus-arpa2wfst-omega \
	rnnlm-binding phonetisaurus-binding

all: phonetisaurus-align phonetisaurus-arpa2wfst-omega \
	rnnlm-binding phonetisaurus-binding

phonetisaurus-align: $(BINDIR)/phonetisaurus-align
phonetisaurus-arpa2wfst-omega: $(BINDIR)/phonetisaurus-arpa2wfst-omega

$(BINDIR)/phonetisaurus-align: util.o LatticePruner.o M2MFstAligner.o \
	FstPathFinder.o phonetisaurus-align.o
	$(CC) $(EXTRA) $(LDFLAGS) $(LIBS) util.o LatticePruner.o M2MFstAligner.o \
	FstPathFinder.o phonetisaurus-align.o \
	-o $(BINDIR)/phonetisaurus-align $(LIBS)

$(BINDIR)/phonetisaurus-arpa2wfst-omega: ARPA2WFST.o util.o phonetisaurus-arpa2wfst-omega.o
	$(CC) $(EXTRA) $(LDFLAGS) $(LIBS) ARPA2WFST.o util.o phonetisaurus-arpa2wfst-omega.o \
	-o $(BINDIR)/phonetisaurus-arpa2wfst-omega $(LIBS)

#RnnLM bindings
rnnlm-binding: $(DEST)/RnnLM.so

$(DEST)/RnnLM.so: $(RNNSONAME) RnnLMPy-binding.o
	$(CC) -shared $(RT) -fPIC RnnLMPy-binding.o -o $(RNNPY)/RnnLM.so -L$(LIBDIR) \
	-I3rdparty/rnnlm rnnlmlib.o -lpython$(VERSION) -lrnnlm -lfst -O3 \
	-funroll-loops -ffast-math -lm

$(DEST)/librnnlm.so: RnnLMPy-binding.o
	$(CC) -Wl,-soname,librnnlm.so.0 -shared -lfst RnnLMPy-binding.o \
	rnnlmlib.o -I. -o $(LIBDIR)/librnnlm.so -O3 -fPIC -funroll-loops -ffast-math -lm
	cd $(LIBDIR); ln -s librnnlm.so librnnlm.so.0; cd -;

$(DEST)/librnnlm.dylib: RnnLMPy-binding.o
	$(CC) -Wl,-install_name,librnnlm.so -shared -lc -lfst RnnLMPy-binding.o \
	rnnlmlib.o -I. -o $(LIBDIR)/librnnlm.dylib -O3 -fPIC -funroll-loops \
	-ffast-math -lm -lpython$(VERSION)

RnnLMPy-binding.o: RnnLMPy-binding.cc
	$(CC) -O3 -fPIC -funroll-loops -ffast-math -lm -I. -I3rdparty/rnnlm -I$(PYTHON_H) -c $(<) -o $(@)

RnnLMPy-binding.cc: rnnlmlib.o
	python rnnlm-module.py > RnnLMPy-binding.cc

rnnlmlib.o : 3rdparty/rnnlm/rnnlmlib.cpp
	$(CC) -O3 -fPIC -funroll-loops -ffast-math -lm -fPIC -w -c 3rdparty/rnnlm/rnnlmlib.cpp

#G2P bindings
phonetisaurus-binding: $(PYLIB)/Phonetisaurus.so

$(PYLIB)/Phonetisaurus.so: $(G2PSONAME) Phonetisaurus-binding.o util.o
	$(CC) -shared $(RT) -fPIC Phonetisaurus-binding.o util.o \
	-o $(PYLIB)/Phonetisaurus.so -I3rdparty/sparsehash -I3rdparty/utfcpp \
	-lpython$(VERSION) -L. -L$(LIBDIR) -lphonetisaurus -lfst -O3 -lm

$(LIB)/libphonetisaurus.so: Phonetisaurus-binding.o util.o
	$(CC) -Wl,-soname,libphonetisaurus.so.0 -shared -lfst Phonetisaurus-binding.o \
	util.o -I. -o $(LIB)/libphonetisaurus.so -O3 -fPIC -funroll-loops -ffast-math -lm

$(LIB)/libphonetisaurus.dylib: Phonetisaurus-binding.o util.o
	$(CC) -Wl,-install_name,libphonetisaurus.dylib -shared -lc -lfst Phonetisaurus-binding.o \
	-I. util.o -o $(LIBDIR)/libphonetisaurus.dylib -O3 -fPIC -funroll-loops \
	-ffast-math -lm -lpython$(VERSION)

Phonetisaurus-binding.o: Phonetisaurus-binding.cc
	$(CC) -O3 -fPIC -funroll-loops -ffast-math -lm \
	-I. -I3rdparty/sparsehash -I3rdparty/utfcpp -I$(PYTHON_H) -c $(<) -o $(@)

Phonetisaurus-binding.cc: phonetisaurus-module.py
	python phonetisaurus-module.py > Phonetisaurus-binding.cc

clean: clean-bindings
	$(RM) $(OUT) $(TMP)

clean-bindings:
	$(RM) $(DEST)/*.so $(LIBDIR)/*.so.0 rnnlmlib.o RnnLMPy-binding.o \
	RnnLMPy-binding.cc $(LIBDIR)/*.dylib Phonetisaurus-binding.o \
	Phonetisaurus-binding.cc $(PYLIB)/*.so $(RNNPY)/*.so \
	$(PYLIB)/*.pyc $(RNNPY)/*.pyc
