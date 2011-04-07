# Makefile
#
CC=g++ -O2
LIBS=-lfst
OUT=phonetisaurus
TMP=*.o
EXTRA= #add any -I -L modifications here...

OBJS=FstPathFinder.o

all: phonetisaurus

FstPathFinder.o: FstPathFinder.cpp
	$(CC) $(EXTRA) FstPathFinder.cpp -c -o FstPathFinder.o

phonetisaurus: FstPathFinder.o Phonetisaurus.cpp
	$(CC) $(LIBS) $(EXTRA) $(OBJS) Phonetisaurus.cpp -o phonetisaurus

libopenfsttools: FstPathFinder.o
	$(CC) -fPIC $(EXTRA) -c FstPathFinder.cpp

clean:
	rm $(OUT) $(TMP) 
