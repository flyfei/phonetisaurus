# Introduction #
This wiki page was designed as a follow-along companion to the FSMNLP 2012 Phonetisaurus G2P tutorial.

It includes a special distribution of the Phonetisaurus G2P toolkit as well as the latest versions of [OpenFst](http://www.openfst.org), [Google NGramLibrary](http://www.openfst.org/twiki/bin/view/GRM/NGramLibrary) and the Python [argparse](http://pypi.python.org/pypi/argparse/) module in addition to a modified Phonetisaurus bundle.  This should cover all non-standard dependencies required to install and build the distribution and follow along with the tutorial examples on OSX and most Linux distros.

The remainder of this wiki is divided into three parts: Part I. Installation and compilation, Part II. Model training, and Part III. Decoding examples.

In most cases you should be able to simply copy and paste the example code line-by-line to quickly and painlessly compile, install and get started with training models and producing new pronunciations.

# Part I. Installation and Compilation #
This section covers basic download, dependency installation and compilation issues for the Phonetisaurus G2P toolkit and tutorial.

## Obtaining the tutorial distribution ##
The tutorial distribution, which includes all required dependencies, can be downloaded from the following location,

[fsmnlp-tutorial.tgz](http://code.google.com/p/phonetisaurus/downloads/detail?name=fsmnlp-tutorial.tgz&can=2&q=)

and you can download and unpack the tutorial distribution with the following series of commands,
```
$ wget http://phonetisaurus.googlecode.com/files/fsmnlp-tutorial.tgz
$ tar -xvzf fsmnlp-tutorial.tgz
$ cd fsmnlp-tutorial
```

## Installing OpenFst ##
If you have already installed OpenFst you may be able to skip this step.  Note however, that for the NGramLibrary and Phonetisaurus to work properly it is necessary to compile several OpenFst extensions that are **not** flagged by `configure` by default.

To install the following series of commands will compile and install the version of OpenFst included in the tutorial distribution,
```
$ cd 3rdparty
$ tar -xvzf openfst-1.3.2.tar.gz
$ cd openfst-1.3.2
$ ./configure --enable-compact-fsts --enable-const-fsts --enable-far --enable-lookahead-fsts --enable-pdt
$ make
$ sudo make install
$ cd ..
```
Note that OpenFst may take 10 minutes or more to compile, depending on your machine specs.

## Installing an SLM training toolkit ##
In practice any Statistical Language Modeling (SLM) toolkit capable of generating a standard ARPA-format N-gram model may be used with Phonetisaurus.  There are several well-known SLM toolkits including,

  * [SRILM](http://www.speech.sri.com/projects/srilm/)
  * [MITLM](http://code.google.com/p/mitlm)
  * [CMU-Cambridge SLM](http://mi.eng.cam.ac.uk/~prc14/toolkit.html)
  * [Google NGramLibrary](http://www.openfst.org/twiki/bin/view/GRM/NGramLibrary)

any of which is suitable for use with Phonetisaurus.  The tutorial distribution includes the latest version of the Google NGramLibrary, mainly because it depends only on OpenFst.  Tutorial examples in the decoding section will focus on models trained with the NGramLibrary, but Part II. of this wiki provides basic model training examples for each of the above toolkits.

### Compiling Google NGramLibrary ###
Assuming you have successfully installed OpenFst, compiling and installing the NGramLibrary should be simple,
```
$ tar -xvzf opengrm-ngram-1.0.3.tar.gz
$ cd opengrm-ngram-1.0.3
$ ./configure
$ make
$ sudo make install
$ cd ../..
```

## Compiling Phonetisaurus ##
Assuming you have successfully installed OpenFst, compiling Phonetisaurus should be straightforward,
```
$ cd src
$ make
$ cd ..
```
and that's it!

## Manual installation of Python `argparse` ##
**You may skip this step if you are using Python v.2.7+ or already have `argparse` installed.**
```
$ python
Python 2.7.1 (r271:86832, Jan 28 2011, 20:55:04) 
[GCC 4.2.1 (Apple Inc. build 5664)] on darwin
Type "help", "copyright", "credits" or "license" for more information.
>>> import argparse
>>> 
```
OK.
```
$ python2.5
Python 2.5.5 (r255:77872, Sep 18 2011, 13:58:27) 
[GCC 4.2.1 (Apple Inc. build 5666) (dot 3)] on darwin
Type "help", "copyright", "credits" or "license" for more information.
>>> import argparse
Traceback (most recent call last):
  File "<stdin>", line 1, in <module>
ImportError: No module named argparse
>>> 
```
Need to install `argparse`.

The `argparse` module became a part of the Python core with v2.7, but if you are using an older version of Python you may need to install `argparse` manually.

The best, most reliable way to install this is via [pypi](http://pypi.python.org/pypi/argparse/) using the installation instructions there, however the package has also been included in the tutorial distribution for the sake of completeness.

```
$ cd 3rdparty
$ tar -xvzf argparse-1.2.1.tar.gz
$ cd argparse-1.2.1
$ sudo python ./setup.py install 
$ cd ../..
```

# Part II. Model Training #
There are two distinct steps to the G2P model training approach implemented by Phonetisaurus.  The first step involves **aligning** the grapheme and phoneme sequences in the training dictionary.  The second step involves training a joint-sequence N-gram model from the alignment result.

First we will setup a new directory for our toy data,
```
$ cd script
$ mkdir toy
```

## Dictionary Alignment ##
The default parameter values should be suitable for a first pass with most Indo-European languages.  This should require only a few seconds to complete, and will write the aligned corpus to `toy/toy.corpus`.

### Text corpus output ###
The simplest way to get started is to generated a corpus of 1-best alignments in text format:
```
$ ../phonetisaurus-align --input=toy.train --ofile=toy/toy.corpus
```
### Weighted N-best alignment lattices and .far archives ###
Phonetisaurus is also capable of generating weighted N-best alignment lattices and compiling them into .far archives.  These can subsequently be used with Google NGramLibrary to train an LM from the resulting fractional counts.
```
$ ../phonetisaurus-align --input=toy.train --ofile=toy/toy.far --lattice --nbest=2
```
The '--lattice' flag tells the aligner to generate a .far archive.   There are multiple additional parameters that may be used to heuristically prune the alignment lattices.  See the '--help' message for details.

## Joint-Sequence N-gram model ##
The result of the previous step is simply a corpus of aligned joint-sequences.  This can be used directly to train a standard N-gram model using any SLM toolkit capable of outputting a standard ARPA-format model.

Examples training commands are given below for the MITLM, SRILM, and Google NGramLibrary.

### MITLM ###
Train a joint-sequence N-gram model with MITLM.
```
$ estimate-ngram -s FixKN -o 7 -t toy/toy.corpus -wl toy/toy.arpa
```

### SRILM ###
Train a joint-sequence N-gram model with SRILM.
```
$ ngram-count -order 7 –kn-modify-counts-at-end   -gt1min 0 -gt2min 0 \
-gt3min 0 -gt4min 0 -gt5min 0 -gt6min 0 -gt7min 0 -ukndiscount \
-ukndiscount1 -ukndiscount2 -ukndiscount3 -ukndiscount4 \
-ukndiscount5 -ukndiscount6 -ukndiscount7 -text toy/toy.corpus -lm toy/toy.arpa

```

### Google NGramLibrary ###
Train a joint-sequence N-gram model with NGramLibrary.
```
$ ngramsymbols < toy/toy.corpus > toy/toy.syms
$ farcompilestrings --symbols=toy/toy.syms --keep_symbols=1    toy/toy.corpus > toy/toy.far
$ ngramcount --order=7 toy/toy.far > toy/toy.cnts
$ ngrammake  --method=kneser_ney toy/toy.cnts > toy/toy.mod
$ ngramprint --ARPA toy/toy.mod > toy/toy.arpa
```
#### Using lattices and .far archives ####
As mentioned above, the NGramLibrary tools can utilize the alignment lattices output by `````phonetisaurus-align````` as well as the text corpus.  The series of commands is essentially the same; the `````ngramsymbols````` and `````farcompilestrings````` commands are simply skipped as these are performed automatically by `````phonetisaurus-align`````.
```
$ ngramcount --order=7 toy/toy.far > toy/toy.cnts
$ ngrammake  --method=witten_bell toy/toy.cnts > toy/toy.mod
$ ngramprint --ARPA toy/toy.mod > toy/toy.arpa
```
Note however that when using fractional counts NGramLibrary only supports **witten\_bell** smoothing.  Utilizing N-best alignments will typically improve the within-smoothing performance of a resulting model.  For example `````--nbest=3````` with Witten-Bell smoothing will typically out-perform `````--nbest=2````` which will outperform `````--nbest=1`````, etc., but Kneser-Ney smoothing will typically outperform all three even with `````--nbest=1`````.   Fractional Kneser-Ney will most likely give an additional boost with `````--nbest=2````` etc., but NGramLibrary uses bins, so I guess I'll have to implement this myself...

### Converting the model ###
Once the ARPA-format N-gram model has been trained, the final step is to convert this to Phonetisaurus WFST format.  This can be achieved with the following command,
```
$ ../phonetisaurus-arpa2fst --input=toy/toy.arpa --prefix="toy/toy"
```
which will save the resulting model to `toy/toy.fst`.

## Generating pronunciations ##
Once a model has been trained and converted to WFST format, it can be used to generate pronunciations for previously unseen words.  Below are a variety of examples using the toy model trained in the previous section.
#### Basic decoding - simplest possible example ####
```
$ ../phonetisaurus-g2p --model=toy/toy.fst --input=phoneme  
14.1211	f o n i m
```
#### Output words - useful for long lists ####
```
$ ../phonetisaurus-g2p --model=toy/toy.fst --input=phoneme --words
phoneme	14.1211	f o n i m
```
#### Caution! Everything is case sensitive ####
Input tokens that are not found in the input symbols table for the model are automatically mapped to the epsilon symbol.
```
$ ../phonetisaurus-g2p --model=toy/toy.fst --input=pHoneme --words
      Symbol: 'H' not found in input symbols table.
      Mapping to null...
      pHoneme	15.9053	p o n i m
```
#### Generating pronunciations for a list ####
The `--input` parameter takes either an individual word, or a wordlist.  A wordlist is expected to consist of a single target word per line,
```
$ cat toy.wordlist
abbreviate
abeam
abolitionist
absorb
accede
```
the `--isfile` boolean flag tells `phonetisaurus-g2p` to treat the value of the `--input` parameter as a file rather than an individual word,
```
$ ../phonetisaurus-g2p --model=toy/toy.fst --input=toy.wordlist --isfile --words
abbreviate    27.3796	@ b r i v i e t
abeam         17.2314	@ b i m
abolitionist  28.7543	@ b x l I S x n I s t
absorb        20.8632	@ b s c r b
accede        17.6658	@ k s i d
```
Of course the N-best flags and all other decoder options are also available,
```
$ ../phonetisaurus-g2p --model=toy/toy.fst --input=toy.wordlist --isfile --words --nbest=3
abbreviate	27.3796	@ b r i v i e t
abbreviate	28.336	@ b r I v i e t
abbreviate	28.6073	@ b r E v i e t
abeam	17.2314	@ b i m
abeam	17.7505	x b i m
abeam	20.8689	e b i m
abolitionist	28.7543	@ b x l I S x n I s t
abolitionist	29.8953	@ b x l I S x n x s t
abolitionist	31.0089	x b x l I S x n I s t
absorb	20.8632	@ b s c r b
absorb	21.5107	x b s c r b
absorb	23.7251	@ b s x r b
accede	17.6658	@ k s i d
accede	17.9598	x k i d
accede	18.5324	I k s i d
```

#### N-best decoding and the search beam ####
Searching the entire lattice may lead to a significant slowdown.  By default the N-best search scheme only considers a small portion of the pronunciation lattice.  This may lead to a reduced number of hypotheses in some instances,
```
$ ../phonetisaurus-g2p --model=toy/toy.fst --input=pendragon --nbest=5 
18.235	p E n d r @ g x n
```

This effect can often be mitigated by increasing the search beam, at a cost to decoding speed,
```
$ ../phonetisaurus-g2p --model=toy/toy.fst --input=pendragon --nbest=5 --beam=5000
18.235	p E n d r @ g x n
22.328	p x n d r @ g x n
23.2001	p i n d r @ g x n
25.4441	p I n d r @ g x n
25.8139	p E n d r @ g a n
```

#### Using the LMBR decoder ####
Phonetisaurus also implements an experimental WFST-based Lattice Minimum Bayes-Risk decoder which can be used for reranking.  Examples of how to use the LMBR decoder are provided below.

_Without LMBR_
```
$ ../phonetisaurus-g2p --model=toy/toy.fst --input=zoom  --words --nbest=2
zoom	16.6811	z u m
zoom	19.1512	z o x m
```

_With LMBR_
```
$ ../phonetisaurus-g2p --model=toy/toy.fst --input=zoom  --words --nbest=2 --alpha=0.3 --order=4 --mbr
zoom	0.48193	z u m
zoom	1.0265	z U m
```
Note that while the rank-1 hypothesis is correct in the first case, the LMBR decoder in this case produces an arguably more believable list of alternatives.

_Impact of the LMBR_ `--order` _parameter_
```
--order=1
$ ../phonetisaurus-g2p --model=toy/toy.fst --input=zoom  --words --nbest=5 --alpha=0.3 --order=1 --mbr
zoom	-0.0889866	z u m
zoom	-0.0761653	z o o m
zoom	-0.0655657	z u o m
zoom	-0.0655657	z o u m
zoom	-0.0549661	z u u m
```

```
--order=2
$ ../phonetisaurus-g2p --model=toy/toy.fst --input=zoom  --words --nbest=5 --alpha=0.3 --order=2 --mbr
zoom	0.0097291	z u m
zoom	0.154152	z ^ m
zoom	0.193557	z o x m
zoom	0.209025	z U m
zoom	0.245537	z o ^ m
```

```
--order=3
$ ../phonetisaurus-g2p --model=toy/toy.fst --input=zoom  --words --nbest=5 --alpha=0.3 --order=3 --mbr
zoom	0.283839	z u m
zoom	0.610813	z o x m
zoom	0.642236	z ^ m
zoom	0.677682	z U m
zoom	0.698353	z u M
```

```
--order=4
$ ../phonetisaurus-g2p --model=toy/toy.fst --input=zoom  --words --nbest=5 --alpha=0.3 --order=4 --mbr
zoom	0.48193	z u m
zoom	1.0265	z U m
zoom	1.02737	z o x m
zoom	1.05646	z u M
zoom	1.07134	z ^ m
```

```
--order=5
$ ../phonetisaurus-g2p --model=toy/toy.fst --input=zoom  --words --nbest=5 --alpha=0.3 --order=5 --mbr
zoom	0.50446 z u m
zoom	1.07856	z U m
zoom	1.11055	z u M
zoom	1.15411	z ^ m
zoom	1.21364	! u m
```

```
--order=6
$ ../phonetisaurus-g2p --model=toy/toy.fst --input=zoom  --words --nbest=5 --alpha=0.3 --order=6 --mbr
zoom	0.50446	z u m
zoom	1.07856	z U m
zoom	1.11055	z u M
zoom	1.15411	z ^ m
zoom	1.21364	! u m
```

### Evaluation ###
In addition to model training and decoding tools, Phonetisaurus provides several scripts that can used to efficiently conduct evaluations and error analysis.

#### Evaluating a test set ####
The `evaluate.py` provides a means to quickly evaluate the Word Error Rate (WER) and Phoneme Error Rate (PER) of a test set using the standard Levenshtein metric.
```
$ ./evaluate.py --modelfile toy/toy.fst --testfile toy.test --prefix "toy/toy"
../phonetisaurus-g2p --model=toy/toy.fst --input=toy/toy.words --beam=1500 \
--alpha=0.6500 --prec=0.8500 --ratio=0.7200 --order=6 --words --isfile  > toy/toy.hyp
Words: 500  Hyps: 500 Refs: 500
######################################################################
                          EVALUATION RESULTS                          
----------------------------------------------------------------------
(T)otal tokens in reference: 3131
(M)atches: 2767  (S)ubstitutions: 336  (I)nsertions: 31  (D)eletions: 28
% Correct (M/T)           -- %88.37
% Token ER ((S+I+D)/T)    -- %12.62
% Accuracy 1.0-ER         -- %87.38
       --------------------------------------------------------       
(S)equences: 500  (C)orrect sequences: 250  (E)rror sequences: 250
% Sequence ER (E/S)       -- %50.00
% Sequence Acc (1.0-E/S)  -- %50.00
######################################################################
```
The results in this case are pretty poor, but keep in mind that the training set only contained 5000 entries.  Run the command with the `--verbose` option to display all individual alignments and scores.

#### Error analysis ####
The `calculateER.py` script may also be utilized alone to analyze errors on individual alignments.
```
./calculateER.py --ref "t e s t i N G" --hyp "t e S s t i n G"
t e * s t i N G
| |   | | |   |
t e S s t i n G
                          EVALUATION RESULTS                          
(T)otal tokens in reference: 7
(M)atches: 6  (S)ubstitutions: 1  (I)nsertions: 1  (D)eletions: 0
% Correct (M/T)           -- %85.71
% Token ER ((S+I+D)/T)    -- %28.57
% Accuracy 1.0-ER         -- %71.43
(S)equences: 1  (C)orrect sequences: 0  (E)rror sequences: 1
% Sequence ER (E/S)       -- %100.00
% Sequence Acc (1.0-E/S)  -- %0.00
```
The script also accepts lists or regular expressions of items to ignore, which may be helpful in located particular error patterns in a test set.
```
./calculateER.py --ref "t e s t i N G" --hyp "t e S s t i n G" --ignore S
t e s t i N G
| | | | |   |
t e s t i n G
                          EVALUATION RESULTS                          
(T)otal tokens in reference: 7
(M)atches: 6  (S)ubstitutions: 1  (I)nsertions: 0  (D)eletions: 0
% Correct (M/T)           -- %85.71
% Token ER ((S+I+D)/T)    -- %14.29
% Accuracy 1.0-ER         -- %85.71
(S)equences: 1  (C)orrect sequences: 0  (E)rror sequences: 1
% Sequence ER (E/S)       -- %100.00
% Sequence Acc (1.0-E/S)  -- %0.00
```
In the above example the capital **`S`** has been ignored during the alignment and error computation.  Compiling similar lists or regular expression rules can serve as a means to filter error patterns and highlight particularly weak areas of the model or training data.