# Makefile
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
	rm $(OUT) $(TMP) 
