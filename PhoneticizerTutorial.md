DEPRECATED

Please see the ReadMe which is up-to-date.

Phonetisaurus g2p tutorial. The quickest way to get started with phonetisaurus.

# Introduction #

This tutorial introduces the phonetisaurus WFST-driven grapheme-to-phoneme system.  It is intended to serve as both a quick-start guide to using the toolkit as well as a brief theoretical overview of the training and decoding processes and other concepts frequently encountered in the context of g2p/p2g conversion.  The tutorial is divided into two parts.  Part I. gives a general overview of the approach, and Part II. provides a hands-on introduction to phonetisaurus.

# Part I. Background #


## Model Training ##
The g2p model training process comprises two main steps.  The first step is to align the grapheme and phoneme sequences in the pronunciation dictionary.  The second step consists of generating a joint g2p model based on the results of the alignment process.  In the case of the phonetisaurus project the joint sequence model takes the form of a standard `N`-gram language model.  The two steps are described in more detail in the following sections.

### Alignment ###
In most cases pronunciation dictionaries are not aligned, and the phoneme-grapheme correspondences exhibit significant inconsistencies.  In the case of English, there are 26 normal graphemes, not including punctuation marks, while most phoneme sets comprise 40 or more units.  As a consequence it is essential to obtain a reliable alignment of the grapheme-phoneme correspondences in the dictionary prior to proceeding to the model building phase.

There are many viable approaches to producing dictionary alignments.  Of course the simplest approach is to simply align the sequences manually, however this is clearly undesirable in most cases as it is labor intensive and subject to human error.

<table><tr><td>
<img src='http://www.gavo.t.u-tokyo.ac.jp/~novakj/graphs/thorax.png' width='200' />
</td></tr>
<tr align='center'><td align='center'><code>Possible g2p alignment for the word "thorax".</code></td></tr></table>

In terms of automatic alignment approaches there exists a fairly rich body of work on the subject.  Perhaps the simplest approach involves gap-alignment often used to align DNA sequences in bio-informatics.  This approach treats the alignment problem as a dynamic programming task, and often includes heuristic penalties for gaps or unlikely correspondences.  This approach, while simple tends to perform poorly however, in the context of real-world g2p data.

Lately more sophisticated machine-learning techniques have been gaining ground.  One previously popular approach focuses on 1-to-1 viterbi-based sequence alignment to solve the alignment problem.  This approach has found much effective application in the g2p and p2g literature, and is commonly found in the best literature on the subject.  Nevertheless the 1-to-1 constraint that the approach embraces ignores the fundamentally multiple-to-multiple relationships embodied by complex romanized languages like English or French.  In these languages it is common to see multiple token sequences such as `/ght/` found in `might` or `naughty`, or `/ph/` found in `philosophical` or `phonebook`, or grapheme-to-phoneme relationships like that found in `sphinx  /s f ih n k s/`.  In response to this issue, the m2m-aligner researchers revived and improved a much more sophisticated EM-based alignment algorithm capable of teasing out multiple-token clusters and perform multiple-to-multiple alignment.

This multiple-to-multiple approach is the one adopted by phonetisaurus, via the venerable [m2m-aligner](http://code.google.com/p/m2m-aligner/) project. Further mathematical details of the m2m alignment algorithm as well as an open-source implementation can be found in the same place.

### Joint Sequence Model Training ###
Once an alignment has been obtained from a pronunciation dictionary, this can be used to produce a joint-sequence language model by simply concatenating the aligned grapheme-phoneme sequences, resulting in a sequence of aligned grapheme-phoneme pairs.

`sphinx   →   /s f ih n k s/`

`s:s ph:f i:ih n:n x:ks`

The set of joint sequences can then be utilized directly to train a statistical `N`-gram language model.  For technical details on this joint sequence LM training approach see Galescu2002 or Upon completion the resulting model is then split, and transformed into a Weighted Finite-State Transducer (WFST), where the input alphabet corresponds to the set of graphemes and any multiple-grapheme clusters generated during the alignment process, and the output alphabet corresponds to the set of phonemes again along with any multiple-phoneme clusters generated during the alignment process.  In the case of phonetisaurus this model is encoded in binary `OpenFst` format and utilized directly, without further modification as the input to the phonetisaurus-g2p decoder.

## WFST-based Decoding ##
The phonetisaurus project relies on the `OpenFst` project for all low-level WFST-based operations.  Once the pronunciation model has been generated and transformed into a WFST, the decoding process is straightforward.

First the WFST model is read in by the decoder, at which point it detects and registers any multiple-token clusters. The model is then input arc-sorted to facilitate the downstream composition process.  Next the input word that the user wishes to obtain a pronunciation hypothesis for is transformed into a corresponding FSA.  The registered grapheme clusters are used to generate alternative arcs in the input FSA.  An example of such an FSA is depicted below,

<table><tr><td>
<img src='http://www.gavo.t.u-tokyo.ac.jp/~novakj/graphs/radical.png' width='600' />
</td></tr>
<tr align='center'><td align='center'><code>WFST representation of the awesome input word "radical", when using the multiple-to-multiple alignment algorithm.</code></td></tr></table>


The input FSA is then composed with the WFST pronunciation model.  This results in a reduced model which encodes the set of grapheme-to-phoneme correspondences relevant to the input word.  The result is then output-projected, resulting in just the sequences of weighted pronunciation hypotheses corresponding to the original input word.  A standard shortest-path algorithm is then applied to obtain the best, or `N`-best pronunciation hypotheses and these are returned to the user for downstream processing.

The exact sequence of operations is as follows,

<font size='4'><code> nBest(π(oProj(W⚬M)) </code></font>

where the '**⚬**' operator refers to composition, **W** refers to the input word FSA, **M** refers to the g2p model, **oProj** refers to projecting the output symbols, **π** denotes the removal of unwanted symbols such as the skip, '**`_`**', epsilons, `<s>`, and `</s>` as well as the process of re-separating any remaining multi-phone clusters.  Finally, **nBest** refers to the shortest-path algorithm which obtains the best hypotheses given the combination of the original input and the pronunciation model.

# Part II. Introduction to phonetisaurus #

## Installation ##
In order to get started, there are three essential components that need to be installed.  First of course is the phonetisaurus distribution.  Check out the mercurial repository and build the distribution,

### Bare necessities ###
```
$ hg clone https://phonetisaurus.googlecode.com/hg/ phonetisaurus
$ cd phonetisaurus
$ make
g++ -O2  FstPathFinder.cpp -c -o FstPathFinder.o
g++ -O2  Phonetisaurus.cpp -c -o Phonetisaurus.o
g++ -O2 -lfst  Phonetisaurus.o FstPathFinder.o phonetisaurus-g2p.cpp -o phonetisaurus-g2p
...
```

Next, download, compile and install,

[OpenFst](http://www.openfst.org/twiki/bin/view/FST/FstDownload),

[m2m-aligner](http://code.google.com/p/m2m-aligner/)

and

[mitlm](https://code.google.com/p/mitlm/)

Finally, make sure that all three of the associated binaries,
```
phonetisaurus-g2p
m2m-aligner
estimate-ngram
```

are accessible from your `${PATH`} variable.  If you have root access you can just install them in `/usr/local/bin` or your favorite place.  Otherwise, add the local directories to your `${PATH`}, e.g.,
```
$ export PATH=${PATH}:/some/directory/phonetisaurus
```

That's it, you're ready to start the training process.

### Just for fun ###
The phonetisaurus project also includes a plugin python script in the form of [phonetisaurus-flite.py](http://code.google.com/p/phonetisaurus/source/browse/script/phonetisaurus-flite.py), that shunts phonetisaurus hypotheses to the CMU `flite` synthesizer.  If you would like to try this out you need to also install the `flite` synthesizer, which can be obtained from,

http://www.speech.cs.cmu.edu/flite/

That's it, now you're ready for anything!

## Model training ##
Training up a new pronunciation model is now fairly trivial and can be done with a single command.  For starters we will try training a simple 6-gram LM using the NETtalk training partition included in the `data/` subdirectory.  Assuming the installation steps were successful, it should be sufficient to perform the following steps,
```
$ cd script/
$ ./train-model.py --dict ../data/g014a2.train.bsf --verbose --delX --prefix "g014a2/g014a2"
```

This will invoke several default training variables and perform the multi-to-multiple alignment process as well as train up the language model, and final transform the resulting LM into WFST format suitable for use with the `phonetisaurus-g2p` decoder.  For the NETtalk15k training set listed above, it took about 3 minutes to complete the entire training process on my `MacBook Pro`.  The `phonetisaurus-g2p` tool has a fairly sophisticated set of options, that may be of interest once you get going with the toolkit,
```
$ ./train-model.py --help
usage: train-model.py [-h] --dict DICT [--delX] [--delY] [--noalign]
                      [--maxX MAXX] [--maxY MAXY] [--maxFn MAXFN]
                      [--prefix PREFIX] [--order ORDER]
                      [--smoothing SMOOTHING] [--verbose]

./train-model.py --dict training.dic --delX --maxX 2 --maxY 2 --conFn joint
--prefix test --order 6

optional arguments:
  -h, --help            show this help message and exit
  --dict DICT, -d DICT  The input pronunciation dictionary. This will be used
                        to build the G2P/P2G model.
  --delX, -a            m2m-aligner option: Allow deletions of left-hand
                        (input/grapheme) tokens.
  --delY, -b            m2m-aligner option: Allow deletions of right-hand
                        (output/phoneme) tokens.
  --noalign, -n         Skip the alignment step. Useful if you just want to
                        try out different N-gram models.
  --maxX MAXX, -x MAXX  m2m-aligner option: Maximum substring length for left-
                        hand (input/grapheme) tokens.
  --maxY MAXY, -y MAXY  m2m-aligner option: Maximum substring length for
                        right-hand (outut/phoneme) tokens.
  --maxFn MAXFN, -c MAXFN
                        m2m-aligner option: Maximization function. May be one
                        of 'conYX', 'conXY', or 'joint'.
  --prefix PREFIX, -p PREFIX
                        A file prefix. Will be prepended to all model files
                        created during cascade generation.
  --order ORDER, -o ORDER
                        estimate-ngram option: Maximum order of the joint
                        N-gram LM
  --smoothing SMOOTHING, -s SMOOTHING
                        estimate-ngram option: Specify smoothing algorithm.
                        (ML, FixKN, FixModKN, FixKN#, KN, ModKN, KN#)
  --verbose, -v         Verbose mode.
```

## Decoding ##
It is then trivial to perform decoding, and you can obtain pronunciation hypotheses with something like the following command,
```
$ phonetisaurus-g2p -m g014a2/g014a2.fst -n 7 -w airmail
24.7751	@ r m e l
25.4308	x R m e l
25.4499	@ R m e l
25.8758	e R m e l
26.419	a R m e l
26.6219	R m e l
26.9742	E r m e l

```
which should generate a maximum of 7 pronunciation hypotheses in order of decreasing likelihood.

The `phonetisaurus-g2p` also supports several options,
```
$ phonetisaurus-g2p --help
Syntax: ../phonetisaurus-g2p -m g2p-model.fst [-t testfile | -w testword | -s 'a few words'] [-n N]
Required:
   -m --model: WFST representing the G2P/P2G model.
   -t --testset: File containing a list of input words/pronunciations to phoneticize.
   -w --word: A word/pronunciation to phoneticize.
Optional:
   -s --sent: A ' ' separated sentence to phoneticize.
   -n --nbest: Optional max number of hypotheses to produce for each entry.  Defaults to 1.
   -h --help: Print this help message.
```

You can also try decoding the 5002 item test partition, which should be very fast,
```
$ time phonetisaurus-g2p -m test/test.fst -n 1 -t ../data/g014a2.clean.test > results
real	0m3.187s
user	0m2.632s
sys	0m0.546s
```

That's all there is to it.

## Synthesis with CMU `flite` and phonetisaurus ##
Assuming you decided to install the CMU `flite` synthesizer, you can also try synthesizing your phonetisaurus pronunciation hypotheses with [phonetisaurus-flite.py](http://code.google.com/p/phonetisaurus/source/browse/script/phonetisaurus-flite.py).  In order to do this however, you will also need a model derived from the CMUdict.  You can either train one up yourself, using the [cmudict.0.7a](https://cmusphinx.svn.sourceforge.net/svnroot/cmusphinx/trunk/cmudict/), or download a ready-made one that I've prepared in advance, [g014b2.tgz (38MB)](http://www.gavo.t.u-tokyo.ac.jp/~novakj/g014b2.tgz).  Once you download the file to your `phonetisaurus/script` directory, you can setup shop with the following commands,
```
$ tar -xvzf g014b2.tgz
x g014b2/
x g014b2/g014b2.align
x g014b2/g014b2.corpus
x g014b2/g014b2.fst.txt
x g014b2/g014b2.isyms
x g014b2/g014b2.osyms
x g014b2/g014b2.ssyms
x g014b2/g014b2.train

$ cd g014b2
$ fstcompile --ssymbols=g014b2.ssyms \
> --isymbols=g014b2.isyms --keep_isymbols \
> --osymbols=g014b2.osyms --keep_osymbols \
> g014b2.fst.txt  > g014b2.fst
```
This will compile the text version of the WFST model and store the symbols.

Note: If you decide to train your own model, the multiple-to-multiple alignment process may take about 20~30 minutes depending on your machine.

Once that's all done, you can try out the synthesis with something like the following command:
```
$ phonetisaurus-flite.py -m g014b2.fst --sent "what an amazing tool"
```

Note however, that the `phonetisaurus-g2p` will just produce the best hypothesis for each word, without regard to context or sentence structure.  If you want something more sophisticated it will require construction and integration of a sentence-level or other model.

Which might be quite an interesting next step...

Happy phoneticizing!