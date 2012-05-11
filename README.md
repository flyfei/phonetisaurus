# README #
anonymized phoneticizer

## Introduction to WFST-based LMBR decoding for G2P ##
[anonymous-lmbr-g2p.pdf](https://bitbucket.org/phoneticizer/phoneticizer/downloads/anonymous-lmbr-g2p.pdf)
 

## DEPENDENCIES ##

* g++
* Python 2.7+
* SWIG 
* OpenFst
    * [http://www.openfst.org](http://www.openfst.org)
    * `$./configure --enable-compact-fsts --enable-const-fsts --enable-far --enable-lookahead-fsts --enable-pdt`

        `$ make install`

* mitlm
    * [http://code.google.com/p/mitlm/](http://code.google.com/p/mitlm/)
    * `$./configure`

        `$ make install`

## COMPILATION ##

    $ make
    $ ./mk_swig.sh

## EXAMPLE USAGE ##

### Basic training ###

#### alignment ####
    $ cd script
    $ mkdir g014a2
    $ ../m2m-aligner.py -s1 -s2 -wa g014a2/g014a2.corpus -a ../data/g014a2.train.bsf

#### model building ####
    $ ./train-model.py --dict ../data/g014a2.train.bsf --prefix g014a2/g014a2 --noalign --palign --order 7

### Basic evaluation ###
    $ ./evaluate.py --modelfile g014a2/g014a2.fst --testfile ../data/g014a2.test.tabbed.bsf --prefix g014a2/g014a2 
    Words: 4951  Hyps: 4951 Refs: 4951
    ######################################################################
                          EVALUATION RESULTS                          
    ----------------------------------------------------------------------
    (T)otal tokens in reference: 30799
    (M)atches: 28470  (S)ubstitutions: 2158  (I)nsertions: 197  (D)eletions: 171
    % Correct (M/T)           -- %92.44
    % Token ER ((S+I+D)/T)    -- %8.20
    % Accuracy 1.0-ER         -- %91.80
           --------------------------------------------------------       
    (S)equences: 4951  (C)orrect sequences: 3293  (E)rror sequences: 1658
    % Sequence ER (E/S)       -- %33.49
    % Sequence Acc (1.0-E/S)  -- %66.51
    ######################################################################

#### One word example ####
    $ ../phonetisaurus-g2p -m g014a2/g014a2.fst -w abbreviate
    25.6                   @ b r i v i e t

#### Nbest example ####
    $ ../phonetisaurus-g2p -m g014a2/g014a2.fst -w abbreviate -n 5
    25.664                 @ b r i v i e t
    28.2066                @ b i v i e t
    29.0391                x b b r i v i e t
    29.1606                @ b b r i v i e t
    29.268                 @ b r E v i e t

#### Output words ####
    $ ../phonetisaurus-g2p -m g014a2/g014a2.fst -w abbreviate -n 3 -o
    abbreviate             25.664               @ b r i v i e t
    abbreviate             28.2066              @ b i v i e t
    abbreviate             29.0391              x b b r i v i e t

#### LMBR decoding ####
    $ ../phonetisaurus-g2p -m g014a2/g014a2.fst -w abbreviate -b 1000 -n 3 -o -r -a .6 -d 2 
    abbreviate             -0.348123            @ b r i v i e t
    abbreviate             -0.214035            @ b b r i v i e t
    abbreviate             0.0421478            @ b i v i e t

#### LMBR decoding - higher n-gram order ####
    $ ../phonetisaurus-g2p -m g014a2/g014a2.fst -w abbreviate -b 1000 -n 3 -o -r -a .6 -d 7
    abbreviate           0.285165             @ b r i v i e t
    abbreviate           2.5544               x b r i v i e t
    abbreviate           2.57029              @ b b r i v i e t

#### LMBR evaluation ####
Example evaluation for the NETtalk-15k/5k partition.
This uses the latest version of the LMBR decoder which 
incorporates a length-normalized N-gram factor.
This is a slight improvement over the submitted paper.

    $ cd script
    $ ./evaluate.py --modelfile g014a2/g014a2.fst --testfile ../data/g014a2.test.tabbed.bsf --prefix g014a2/g014a2 --order 6 --alpha .6 --mbrdecode "\-r"
    Words: 4951  Hyps: 4951 Refs: 4951
    ######################################################################
                              EVALUATION RESULTS                          
    ----------------------------------------------------------------------
    (T)otal tokens in reference: 30799
    (M)atches: 28486  (S)ubstitutions: 2142  (I)nsertions: 201  (D)eletions: 171
    % Correct (M/T)           -- %92.49
    % Token ER ((S+I+D)/T)    -- %8.16
    % Accuracy 1.0-ER         -- %91.84
           --------------------------------------------------------       
    (S)equences: 4951  (C)orrect sequences: 3294  (E)rror sequences: 1657
    % Sequence ER (E/S)       -- %33.47
    % Sequence Acc (1.0-E/S)  -- %66.53
    ######################################################################

### Error analysis ###
Highly flexibly may be used for full evaluations or just comparing arbitrary strings.
Also possible to 'filter' tokens based on rules which can be useful in error analysis.

    $ ./script/calculateER.py --hyp "z o a m" --ref "z u m"
    z * u m
    |     |
    z o a m

### RNNLM RESCORING ###
This currently requires a separate download as the configuration
for integration with the RNNLM toolkit has slightly different requirements.

See the repository downloads section:

[g2p2rnn.tgz](https://bitbucket.org/phoneticizer/phoneticizer/downloads/g2p2rnn.tgz)