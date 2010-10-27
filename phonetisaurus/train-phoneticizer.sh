#!/bin/bash

echo "STARTING TRAINING"
echo "Using partitioned dataset: 95% training, 5% testing"
echo "Aligning the dictionary..."
./alignment.py ../data/dev-train-0.95-un.dic
echo "Generating LM training corpus..."
./dic2corpus.py models/aligned-v5.dic > models/dev-train-0.95-un.corpus
echo "Training the n-gram model with mitlm, n=6..."
estimate-ngram -o 6 -t models/dev-train-0.95-un.corpus -wl models/aligned_corpus-6g.arpa
echo "Building the phoneticizer FST..."
./arpa2fst.py models/aligned_corpus-6g.arpa models/g.ssyms models/g-mod.isyms models/g-mod.osyms models/g-t3.fst.txt models/g-mod.fst 6
echo "TRAINING FINISHED"
echo "Running the test..."
./g2p-en.py -m models/g-mod.fst -i models/g-mod.isyms -o models/g-mod.osyms -f ../data/dev-test-0.05-un.dic  > models/results-dev-test-0.05.dic
echo "Evaluating WER..."
./eval-wer.py models/results-dev-test-0.05.dic
echo "TESTING FINISHED"

echo "In order to run the new models yourself, try something like,"
echo "./g2p-en.py -m models/g-mod.fst -i models/g-mod.isyms -o models/g-mod.osyms -w testing"