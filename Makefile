# Makefile
#
CC=g++ -O2
LIBS=-lfst
OUT=phonetisaurus-g2p
TMP=*.o
EXTRA= #add any -I -L modifications here...

OBJS=Phonetisaurus.o FstPathFinder.o

all: phonetisaurus-g2p

Phonetisaurus.o: Phonetisaurus.cpp
	$(CC) $(EXTRA) Phonetisaurus.cpp -c -o Phonetisaurus.o

FstPathFinder.o: FstPathFinder.cpp
	$(CC) $(EXTRA) FstPathFinder.cpp -c -o FstPathFinder.o

phonetisaurus-g2p: FstPathFinder.o Phonetisaurus.o phonetisaurus-g2p.cpp
	$(CC) $(LIBS) $(EXTRA) $(OBJS) phonetisaurus-g2p.cpp -o phonetisaurus-g2p

clean:
	rm $(OUT) $(TMP) 
