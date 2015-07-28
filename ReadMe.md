#The README file.

<h1>README</h1>

<p>Phonetisaurus G2P</p>

<h2>Introduction to WFST-based LMBR decoding for G2P</h2>

<p><a href='http://phonetisaurus.googlecode.com/files/phonetisaurus-lmbr-g2p.pdf'>phonetisaurus-lmbr-g2p.pdf</a></p>

<h2>DEPENDENCIES</h2>

<ul>
<li>g++</li>
<li>Python 2.7+</li>
<li><p>OpenFst</p>

<ul><li><a href='http://www.openfst.org'><a href='http://www.openfst.org'>http://www.openfst.org</a></a></li>
<li><p><pre><code>$./configure --enable-compact-fsts --enable-const-fsts --enable-far --enable-lookahead-fsts --enable-pdt</code></pre></p>

<p><pre><code>$ make install</code></pre></p></li></ul></li>
<li><p>Language Modeling toolkit (you just need one)</p>
<ul>
<li><a href='http://code.google.com/p/mitlm/'>mitlm</a>(recommended)</li>
<li><a href='http://www.speech.sri.com/projects/srilm/'>SRILM</a></li>
<li><a href='http://mi.eng.cam.ac.uk/~prc14/toolkit.html'>CMU-Cambridge SLM toolkit</a></li>
<li><a href='http://www.openfst.org/twiki/bin/view/GRM/NGramLibrary'>Google NGramLibrary</a></li>
<blockquote></ul></li>
</ul></blockquote>

<h2>COMPILATION</h2>

```

$ cd src
$ make
```

<h2>EXAMPLE USAGE</h2>

<h3>Basic training</h3>

<h4>alignment</h4>

```

$ cd script
$ mkdir g014a2
$ ../phonetisaurus-align --input=../data/g014a2.train.bsf --ofile=g014a2/g014a2.corpus
```

<h4>model building</h4>
OLD **only supports mitlm**:
```

$ ./train-model.py --dict ../data/g014a2.train.bsf --prefix g014a2/g014a2 --noalign --palign --order 7
```
NEW **any LM toolkit should work**:
> - Use your favorite LM training toolkit to train an ARPA model directly from the aligned corpus.

> - Convert it with the C++ tool (easier, less python scripting, more flexible - less tested...)
```

$ estimate-ngram -s FixKN -o 7 -t g014a2/g014a2.corpus -wl g014a2/g014a2.arpa
$ ../phonetisaurus-arpa2fst --input=g014a2/g014a2.arpa --prefix="g014a2/g014a2"
```

<h3>Basic evaluation</h3>

```

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
```

<h4>One word example</h4>

```

$ ../phonetisaurus-g2p --model=g014a2/g014a2.fst --input=abbreviate
25.6                   @ b r i v i e t
```

<h4>Nbest example</h4>

```

$ ../phonetisaurus-g2p --model=g014a2/g014a2.fst --input=abbreviate --nbest=5
25.664                 @ b r i v i e t
28.2066                @ b i v i e t
29.0391                x b b r i v i e t
29.1606                @ b b r i v i e t
29.268                 @ b r E v i e t
```

<h4>Output words</h4>

```

$ ../phonetisaurus-g2p --model=g014a2/g014a2.fst --input=abbreviate --nbest=3 --words
abbreviate             25.664               @ b r i v i e t
abbreviate             28.2066              @ b i v i e t
abbreviate             29.0391              x b b r i v i e t
```

<h4>LMBR decoding</h4>

```

$ ../phonetisaurus-g2p --model=g014a2/g014a2.fst --input=abbreviate --beam=1000 --nbest=3 --words \
--mbr --alpha=.6 --order=2
abbreviate       -0.436579                 @ b r i v i e t
abbreviate       -0.330031                 @ b b r i v i e t
abbreviate       -0.0922226                @ b i v i e t
```

<h4>LMBR decoding - higher n-gram order</h4>

```

$ ../phonetisaurus-g2p --model=g014a2/g014a2.fst --input=abbreviate --beam=1000 --nbest=3 --words \
--mbr --alpha=.6 --order=7
abbreviate        0.122205                  @ b r i v i e t
abbreviate        2.12447                   x b r i v i e t
abbreviate        2.12672                   @ b b r i v i e t
```

<h4>LMBR evaluation</h4>

<p>Example evaluation for the NETtalk-15k/5k partition.<br>
This uses the latest version of the LMBR decoder which<br>
incorporates a length-normalized N-gram factor.<br>
This is a slight improvement over the submitted paper.</p>

```

$ ./evaluate.py --modelfile g014a2/g014a2.fst --testfile ../data/g014a2.test.tabbed.bsf --prefix g014a2/g014a2 \
--order 6 --alpha .65 --mbrdecode
Command: ../phonetisaurus-g2p --model=g014a2/g014a2.fst --input=g014a2/g014a2.words \
--alpha=0.6500 --prec=0.8500 --ratio=0.7200 --order=6 --words --isfile --mbr \
&gt; g014a2/g014a2.hyp
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
```

<h3>Error analysis</h3>

<p>Highly flexibly may be used for full evaluations or just comparing arbitrary strings.<br>
Also possible to 'filter' tokens based on rules which can be useful in error analysis.</p>

```

$ ./script/calculateER.py --hyp "z o a m" --ref "z u m"
z * u m
|     |
z o a m
```

<h3>RNNLM RESCORING</h3>

<p>This currently requires a separate download as the configuration<br>
for integration with the RNNLM toolkit has slightly different requirements.</p>

<p>See the repository downloads section:</p>

<p><a href='http://phonetisaurus.googlecode.com/files/g2p2rnn.tgz'>g2prnn.tgz</a></p>