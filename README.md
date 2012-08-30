# README #
Josef Robert Novak - 2012-07-20
Phonetisaurus G2P

This distribution has been specially compiled for the FSMNLP 2012 tutorial.

Please see the related wiki page for details:

http://code.google.com/p/phonetisaurus/wiki/FSMNLPTutorial








## Introduction to WFST-based LMBR decoding for G2P ##
[phonetisaurus-lmbr-g2p.pdf](http://phonetisaurus.googlecode.com/files/phonetisaurus-lmbr-g2p.pdf)
 

## DEPENDENCIES ##

* g++
* Python 2.7+
* OpenFst
    * [http://www.openfst.org](http://www.openfst.org)
    * `$./configure --enable-compact-fsts --enable-const-fsts --enable-far --enable-lookahead-fsts --enable-pdt`

        `$ make install`

* mitlm
    * [http://code.google.com/p/mitlm/](http://code.google.com/p/mitlm/)
    * `$./configure`

        `$ make install`

## COMPILATION ##

    $ cd src
    $ make

## EXAMPLE USAGE ##

### Basic training ###

#### alignment ####
    $ cd script
    $ mkdir g014a2
    $ ../phonetisaurus-align --input=../data/g014a2.train.bsf --ofile=g014a2/g014a2.corpus

#### model building ####
    OLD:
    $ ./train-model.py --dict ../data/g014a2.train.bsf --prefix g014a2/g014a2 --noalign --palign --order 7

    NEW: 
      - Call your LM trainer directly
      - Then just convert it with the C++ tool (easier, less python scripting, more flexible - less tested...)
    $ estimate-ngram -s FixKN -o 7 -t g014a2/g014a2.corpus -wl g014a2/g014a2.arpa
    $ ../phonetisaurus-arpa2fst --input=g014a2/g014a2.arpa --prefix="g014a2/g014a2"

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
    $ ../phonetisaurus-g2p --model=g014a2/g014a2.fst --input=abbreviate
    25.6                   @ b r i v i e t

#### Nbest example ####
    $ ../phonetisaurus-g2p --model=g014a2/g014a2.fst --input=abbreviate --nbest=5
    25.664                 @ b r i v i e t
    28.2066                @ b i v i e t
    29.0391                x b b r i v i e t
    29.1606                @ b b r i v i e t
    29.268                 @ b r E v i e t

#### Output words ####
    $ ../phonetisaurus-g2p --model=g014a2/g014a2.fst --input=abbreviate --nbest=3 --words
    abbreviate             25.664               @ b r i v i e t
    abbreviate             28.2066              @ b i v i e t
    abbreviate             29.0391              x b b r i v i e t

#### LMBR decoding ####
    $ ../phonetisaurus-g2p --model=g014a2/g014a2.fst --input=abbreviate --beam=1000 --nbest=3 --words --mbr --alpha=.6 --order=2
    abbreviate	     -0.436579		       @ b r i v i e t
    abbreviate	     -0.330031		       @ b b r i v i e t
    abbreviate	     -0.0922226		       @ b i v i e t

#### LMBR decoding - higher n-gram order ####
    $ ../phonetisaurus-g2p --model=g014a2/g014a2.fst --input=abbreviate --beam=1000 --nbest=3 --words --mbr --alpha=.6 --order=7
    abbreviate	      0.122205			@ b r i v i e t
    abbreviate	      2.12447			x b r i v i e t
    abbreviate	      2.12672			@ b b r i v i e t

#### LMBR evaluation ####
Example evaluation for the NETtalk-15k/5k partition.
This uses the latest version of the LMBR decoder which 
incorporates a length-normalized N-gram factor.
This is a slight improvement over the submitted paper.

    $ ./evaluate.py --modelfile g014a2/g014a2.fst --testfile ../data/g014a2.test.tabbed.bsf --prefix g014a2/g014a2 --order 6 --alpha .65 --mbrdecode
      Command: ../phonetisaurus-g2p --model=g014a2/g014a2.fst --input=g014a2/g014a2.words \
                --alpha=0.6500 --prec=0.8500 --ratio=0.7200 --order=6 --words --isfile --mbr \
		 > g014a2/g014a2.hyp
    Words: 4951  Hyps: 4951 Refs: 4951
    ######################################################################
                          EVALUATION RESULTS                          
    ----------------------------------------------------------------------
    (T)otal tokens in reference: 30799
    (M)atches: 28484  (S)ubstitutions: 2148  (I)nsertions: 202  (D)eletions: 167
    % Correct (M/T)           -- %92.48
    % Token ER ((S+I+D)/T)    -- %8.17
    % Accuracy 1.0-ER         -- %91.83
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

[g2prnn.tgz](http://phonetisaurus.googlecode.com/files/g2p2rnn.tgz)


--------------------------------
FstPathFinder license info: 
The original FstPathFinder class was written by Chris Taylor as part of another project.
I (JRN) modified this nice project to return a list of hypothesis-cost pairs and included it in Phonetisaurus.
His README is included below for the sake of completeness.

README

OpenFst Tools

	This is a collection of useful tools put together for use with the OpenFst (http://www.openfst.org) toolkit.
	The collection contains some code found on the OpenFst forum. Currently considering offering additional tools found in lextools.
	Where the forum provided information, a link to said forum article is provided.

License

	This code was licensed under the Apache license that OpenFst is released under.

Dependencies

	Assumptions:
		I assume you're building with gcc/g++.
	Deps:
		libdl and libfst

	libdl should be installed already, if it's not, install your OS "developer" packages
	libfst is the OpenFst library, you can get it at http://www.openfst.org

Build Instuctions

	Edit the Makefile variable EXTRA to match your system configuration. This means you'll have to add any custom 
	-I and -L flags so gcc/g++ know where to find the proper include and library files.

Contact

	If you have any recommendations, ideas, or comments, please feel free to contact the google code page related to this collection.

	Chris Taylor
	http://code.google.com/p/openfst-tools/

Thanks

	The OpenFst team and user community

