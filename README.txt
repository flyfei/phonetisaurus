Phonetisaurus
2010-10-27
Josef Robert Novak

UPDATED: 2012-03-13 Josef R. Novak
m2m-aligner is no longer required.  The native WFST-based aligner should
be preferred as it is faster and produces more accurate alignments.

UPDATED: 2011-04-07 Josef R. Novak
The python 'decoder' is now obsolete.  Use the C++ tool.

UPDATED: 2011-04-11 Josef R. Novak
Many updates to the C++ 'decoder'.  It is now fairly nice.
Also wrote new model-training scripts.  

It should be much easier to train and test models now.

REQUIREMENTS
estimate-ngram must be accessibile from your ${PATH}
variable.  If you have not installed it, you can obtain the source
code from the following locations.  

mitlm:
 https://code.google.com/p/mitlm/

The m2m-aligner is also a nice tool and is supported as an alternative
but support is now deprecated in favor of the native implementation.
m2m-aligner:
 http://code.google.com/p/m2m-aligner/
  
RECOMMENDED INSTALL:
$ make -j
$ ./mk_swig.sh

ALIGN A DATABASE
$ ./m2m-aligner.py --align data/g014a2.train.bsf -s2 -s1 --write_align script/test/test.corpus -m1 2 -m2 2

TRAIN A MODEL
$ cd script
$ ./train-model.py --dict ../data/g014a2.train.bsf --prefix test/test --order 7 --noalign --palign

TEST A MODEL
$ ./evaluate.py --modelfile test/test.fst --testfile ../data/g014a2.test.tabbed.bsf --prefix test/test


-----

OLD INSTALL
$ make

TRAIN A MODEL
$ cd script
$ ./train-model.py --dict ../data/g014a2.train.bsf --verbose --delX --prefix "test/test"

TEST A MODEL
$ ../phonetisaurus-g2p -m test/test.fst -n 1 -t ../data/g014a2.clean.test

TEST A WORD
$ ../phonetisaurus-g2p -m test/test.fst -n 1 -w airmail

GET NBEST RESULTS
$ ../phonetisaurus-g2p -m test/test.fst -n 7 -w airmail


I've only tested this latest build properly with the NETtalk database.
It might die with something else...








------------------------------------
OLD STUFF
------------------------------------

Compile:
$ make

Running the tool:
$ ./phonetisaurus <clusters> <testlist> <isyms> <model.fst> <osyms> <n-best>

for example,
$ ./phonetisaurus mymodel.clusters testlist mymodel.isyms mymodel.fst mymodel.osyms 5

Will produce pronunciation (or spelling) hypotheses for each item in 'testlist'.  
It will produce a maximum of 5 hypotheses for each item.  Depending on the item it
may produce fewer hypotheses.









INSTALL
First, install the following packages, 

Python:
sudo easy_install simplejson
sudo easy_install argparse (if using python version < 2.7)

Other:
OpenFST  http://openfst.org/
mitlm    https://code.google.com/p/mitlm/

The project is setup to use the open nettalk db by default,
cd into the phonetisaurus sub-directory and run,

---------------------------------------
phonetisaurus$ ./train-phoneticizer.sh
  STARTING TRAINING
  Using partitioned dataset: 95% training, 5% testing
  Aligning the dictionary...
  Iteration 1. Only equal length pairs...
  Iteration 2. Only equal length pairs...
  Iteration 3...
  Iteration 4...
  Iteration 5...
  Generating LM training corpus...
  Training the n-gram model with mitlm, n=6...
  Loading corpus models/dev-train-0.95-un.corpus...
  Smoothing[1] = ModKN
  Smoothing[2] = ModKN
  Smoothing[3] = ModKN
  Smoothing[4] = ModKN
  Smoothing[5] = ModKN
  Smoothing[6] = ModKN
  Set smoothing algorithms...
  Estimating full n-gram model...
  Saving LM to models/aligned_corpus-6g.arpa...
  Building the phoneticizer FST...
  TRAINING FINISHED
  Running the test...
  Evaluating WER...
  WER: 0.361
  TESTING FINISHED
  In order to run the new models yourself, try something like,
  ./g2p-en.py -m g-mod.fst -i g-mod.isyms -o g-mod.osyms -w testing
-------------------------------------

That's it!

In order to use your own dictionary, it needs to follow the simple
format where each line contains a single word (no spaces) followed
by a tab, followed by a sequence of phonemes separated by spaces, 
for example,
--------------------------
testing    T EH S T IH NG
--------------------------
