DEPRECATED

Please see the ReadMe which is up-to-date.

# Introduction #

This guide assumes that you have already compiled and installed Phonetisaurus and its dependencies.  If you haven't, please take a look at the PhoneticizerTutorial before proceeding.

# Quick start examples #
Phonetisaurus supports a variety of different model types and build techniques.  Although the main focus in on Grapheme-to-Phoneme conversion, the approach that Phonetisaurus employs is symmetric with regard to the input and output symbols, thus it is relatively trivial to also construct Phoneme-to-Grapheme models using the same approach with albeit swapped inputs and outputs.  Below are several commands which utilize the included version of the NETtalk15k database and associated 5k non-overlapping test set.

## Forward G2P ##
This is the standard approach.  The command below will generate a multiple-to-multiple alignment, allowing deletions on the input side (graphemes in this case), maximum input clusters of length two symbols, and maximum output clusters of length two symbols.  The joint maximization function will be used.  Following alignment a 9-gram LM will be trained from the aligned sequence pairs utilizing modified Kneser-Ney smoothing, and all associated models and intermediate files will be stored in the 'test' directory and prepended with the prefix 'test'.

```
./train-model.py --dict ../data/g014a2.train.bsf \
         --delX --maxX 2 --maxY 2 \
         --maxFn joint \
         --prefix "test/test" \
         --order 9
```

The resulting model can be tested using the evaluation script,
```
$ ./evaluate.py 
         --testfile   ../data/g014a2.test.tabbed.bsf 
         --prefix     "test/test"
         --modelfile  test/test.fst
 
######################################################################
                          EVALUATION RESULTS                          
----------------------------------------------------------------------
(T)otal tokens in reference: 30800
(M)atches: 28450  (S)ubstitutions: 2182  (I)nsertions: 200  (D)eletions: 168
% Correct (M/T)           -- %92.37
% Token ER ((S+I+D)/T)    -- %8.28
% Accuracy 1.0-ER         -- %91.72
       --------------------------------------------------------       
(S)equences: 4951  (C)orrect sequences: 3286  (E)rror sequences: 1665
% Sequence ER (E/S)       -- %33.63
% Sequence Acc (1.0-E/S)  -- %66.37
######################################################################

```
The test should produce something like the above.  The entire training and testing process using the above parameters shouldn't take more than 5 minutes on a commodity machine.

## Reverse G2P ##
In some case it may be preferable to train a reversed model.  This simply involves reversing the individual input and output sequences (grapheme and phoneme sequences).  This can be achieved by simply switching on the '--reverse' flag.
```
$ ./train-model.py \
         --dict ../data/g014a2.train.bsf \
         --order 9 \
         --prefix test-reverse/test-reverse \
         --delX --maxX 2 --maxY 2 \
         --maxFn joint \
         --smoothing FixModKN
         --reverse
```

## Forward P2G ##
A Phoneme-to-Grapheme model can be generated from the same input dictionary by simply flipping the '--swap' flag, which will swap the positions of the pronunciations and words in the training data.
```
$ ./train-model.py \
         --dict ../data/g014a2.train.bsf \
         --order 9 \
         --prefix test/test \
         --delX --maxX 2 --maxY 2 \
         --maxFn joint \
         --smoothing FixModKN
         --swap
```

## Reverse P2G ##
Finally, we can both reverse and swap the input, generating a reversed P2G model.
```
$ ./train-model.py \
         --dict ../data/g014a2.train.bsf \
         --order 9 \
         --prefix test/test \
         --delX --maxX 2 --maxY 2 \
         --maxFn joint \
         --smoothing FixModKN
         --swap
         --reverse
```

It is fairly straight-forward to try out different model and training combinations, and most of the training processes, particularly for the NETtalk database only take a couple of minutes to train and a couple of seconds to test.  It is worth trying out a variety of combinations to get a feel for the effects of different parameter choices.

Have fun!