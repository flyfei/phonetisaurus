# Introduction #

The `*`-omega utilities employ a slightly different underlying WFST representation for the ARPA-format joint n-gram models utilized by Phonetisaurus.  In contrast to previous versions of the toolkit, in the new tools adopt a Google style representation in which the sentence-begin and sentence-end tokens are represented implicitly, rather than explicitly.  This leads to a slightly more compact off-line representation, and also simplifies the on-line modification procedures employed to phi-ify the the models for composition with failure transitions.

The alignment and model training stages are unchanged to previous versions, thus this page will focus only on the model transformation and decoder,

```
 $ phonetisaurus-arpa2wfst-omega
 $ phonetisaurus-g2p-omega
```

which must be utilized together.  The legacy transformation and decoding tools are still included in the distribution, but should be considered deprecated at this stage.  They will be eliminated and replaced with the `*`-omega tools as soon as I get some time to do this.

# Model conversion #

The basic underlying model representation now takes the form depicted in the below figure,
| <img src='http://i.imgur.com/vHHlT3e.png' width='600px' /> |
|:-----------------------------------------------------------|
| **Fig. 1.** Basic offline joint n-gram WFST representation for the new Phonetisaurus `*`-omega utilities. |

In the simplest case, which should normally be sufficient, the `phonetisaurus-arpa2wfst` utility can be used as follows to convert an ARPA format model to the new WFST format utilized by `phonetisaurus-g2p-omega`:
```
$ phonetisaurus-arpa2wfst-omega --lm=test/test.arpa --ofile=test/test.fst
```

# Decoding #
Standard epsilon-based approach:
```
$ phonetisaurus-g2p-omega --model=test/test.fst  --words  --input=DIRECTORSHIP
Using 'fsa_eps' approach.  No model modifications required.
DIRECTORSHIP	14.0099	D ER EH K T ER SH IH P
```
Failure-transition mode 1, `--decoder_type=fst_phi`:
```
$ phonetisaurus-g2p-omega --model=test/test.fst  --words  --input=DIRECTORSHIP --decoder_type=fst_phi
Preparing model for 'fst_phi' style decoding.
DIRECTORSHIP	14.1664	D IH R EH K T ER SH IH P
```
Failure-transition mode 2, `--decoder_type=fsa_phi`:
```
$ phonetisaurus-g2p-omega --model=test/test.fst  --words  --input=DIRECTORSHIP --decoder_type=fsa_phi
Preparing model for 'fsa_phi' style decoding.
DIRECTORSHIP	14.1664	D IH R EH K T ER SH IH P
```
`*`Note that in general the 1-best results for the epsilon and failure-based versions will be identical.  This example is used to highlight the fact that this will not _always_ be the case.

The new `*`-omega decoder, `phonetisaurus-g2p-omega` implements two novel algorithms, which support the use of φ-transitions (failure transitions).  The φ-transition approach is conceptually attractive because it provides an _exact_ rather than approximate representation of the ARPA model as a finite-state machine.  These two algorithms are modified versions of the classic approach proposed for word-ngram models and described in,

<blockquote>
<code>[1]</code> C. Allauzen, M. Mohri, and B. Roark,<br>
<i>Generalized Algorithms for Constructing Statistical Language Models</i>,<br>
Proc. ACL, 2003, pp. 40-47<br>
(<a href='http://acl.ldc.upenn.edu/acl2003/main/pdfs/Allauzen.pdf'>pdf</a>).<br>
</blockquote>

The modifications introduced provide support for several quirks of joint n-gram models, which make the standard algorithm unsuitable for use.  Details regarding the modification are described in an upcoming paper, but toy example graphs are provided below for the sake of completeness:

| <img src='http://i.imgur.com/5Syc7Gp.png' width='600px' /> |
|:-----------------------------------------------------------|
| **Fig. 2.** Result of applying of the canonical ARPA-to-WFSA conversion algorithm described in `[1]` and implemented <br />in the Google [NGramLibrary](http://www.openfst.org/twiki/bin/view/GRM/NGramLibrary), to a joint n-gram model. The <font color='red'>red</font>, dashed transitions indicate transitions that <br /> would not be followed for a word starting with `a`, because _at least one_ valid non-backoff transition exists.|

| <img src='http://i.imgur.com/lGQ1Z73.png' width='600px' /> |
|:-----------------------------------------------------------|
| **Fig. 3.** Result of utilizing the `--decoder_type=fst_phi` algorithm and applying it to the toy model. <br />This is identical to the standard conversion algorithm from `[1]`, with the exception that the input-output <br />labels have been encoded, and each new target word is constructed as an FST, adding transitions for <br />all possible G-to-P correspondences learned during the alignment phase.|

| <img src='http://i.imgur.com/u19GgSI.png' width='600px' /> |
|:-----------------------------------------------------------|
| **Fig. 4.** Result of utilizing the `--decoder_type=fsa_phi` algorithm and applying it to the toy model. <br />This approach generates new transitions in the G2P representation, allowing for continued use<br /> of FSA-style input words, but with the drawback of longer conversion time and a larger model.|

## N-best and pruning ##
The new decoder can also be used to produce n-best results where the list is also _pruned_ relative to the 1-best hypothesis.  It is recommended that you use one of the failure-transition based solutions, primarily because this tends to be faster and eliminates many issues of epsilon-transition redundancy.

Standard n-best support is provided as before:
```
$ phonetisaurus-g2p-omega --model=g014b2b/g014b2b8.fst  --words  --input=DIRECTORSHIP \
              --decoder_type=fst_phi --nbest=5 
Preparing model for 'fst_phi' style decoding.
DIRECTORSHIP	14.1664	D IH R EH K T ER SH IH P
DIRECTORSHIP	14.2312	D IY R EH K T ER SH IH P
DIRECTORSHIP	14.3524	D AY R EH K T ER SH IH P
DIRECTORSHIP	14.4503	D ER EH K T ER SH IH P
DIRECTORSHIP	20.9382	D ER EH K T ER SH AH P
```

It is also possible to apply a pruning threshold now, using the `--thresh` parameter (latest repo version):
```
phonetisaurus-g2p-omega --model=g014b2b/g014b2b8.fst  --words  --input=DIRECTORSHIP \
             --decoder_type=fst_phi --nbest=5 --thresh=.2
Preparing model for 'fst_phi' style decoding.
DIRECTORSHIP	14.1664	D IH R EH K T ER SH IH P
DIRECTORSHIP	14.2312	D IY R EH K T ER SH IH P
DIRECTORSHIP	14.3524	D AY R EH K T ER SH IH P
```