#!/bin/bash

rm MBRDecoder.o mbr-decoder

g++ -c MBRDecoder.cpp

g++ -O2 -lfst -ldl MBRDecoder.o mbr-decoder.cpp -o mbr-decoder
