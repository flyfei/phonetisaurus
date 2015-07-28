# Introduction #

Maximum-Entropy based joint n-gram models have been shown to perform quite well in the G2P domain in past,
<blockquote>
S.F. Chen, "<i>Conditional and Joint Models for Grapheme-to-Phoneme Conversion</i>,'' in Proc. of INTERSPEECH, 2003.<br>
</blockquote>
Recently an excellent [`MaxEnt` extension for SRILM](http://www.phon.ioc.ee/dokuwiki/doku.php?id=people:tanel:srilm-me.en) has been developed which provides advanced support for general word-based `MaxEnt` n-gram models, and output using the standard ARPA format.

A paper describing both the method and toolkit extension may be found here,
<blockquote>
T. Alumae, M. Kurimo, "<i>Efficient Estimation of Maximum Entropy Language Models with N-gram features: an SRILM extension</i>", in Proc. INTERSPEECH, 2010.<br>
</blockquote>

This tutorial page provides some simple examples of how to use the SRILM extension to train a Maximum-Entropy style joint n-gram model for use in Phonetisaurus.

# Requirements #
This tutorial requires `OpenFst`, Phonetisaurus, SRILM (plus the `MaxEnt` extension above).  If you would like to also follow the interpolation examples you will need to further install MITLM, although in principle this can also be done with SRILM.  For the fractional counts example you will also need the Google `NGramLibrary`.

# Basic `MaxEnt` Model training #

This tutorial focuses on model training and combination, as such the alignment step is ignored.  It is assumed that you have used the aligner to align your dictionary, and that the alignment result is stored in the `test.corpus` file.  See the basic tutorial or README file for details on how to run the aligner if you have never done so before.

Train a basic join n-gram `MaxEnt` model,
```
 $ ngram-count -order 7 -text test.corpus -maxent-lm test.me.arpa -maxent-convert-to-arpa
```

This will generate a `MaxEnt` n-gram model and dump it in standard ARPA format.   The model may then be used in the normal fashion with Phonetisaurus:
```
 $ phonetisaurus-arpa2wfst --lm=test.me.arpa --ofile=test.me.fst
```

# Model mixing #
Since the `MaxEnt` model is stored in ARPA format it is also possible to utilize it in a straightforward manner in combination with other models via linear interpolation.  Assuming we have another Kneser-Ney model, `test.mkn.arpa`, we can perform vanilla linear interpolation via MITLM's `interpolate-ngram` program,
```
 $ interpolate-ngram -o 7 -l "test.arpa,test.me.arpa" -i LI -wl test-me-mkn.arpa
```

and again, the result may be converted and used in the standard way.

# Fractional counts #
Like Witten-Bell discounting, Maximum-Entropy n-gram models of the form utilized in the preceding sections generalize naturally to fractional counts.  The Phonetisaurus aligner is also capable of dumping this fractional counts in the `OpenFst` `.far` archive format.  The Google `NGramLibrary` may then be used to dump the counts to a form compatible with SRILM, which makes it possible to train a `MaxEnt` n-gram model from fractional alignment counts.  This process is slightly more complex, and requires additional consideration during the alignment process, so all steps are provided below:

Compute the 3-best alignments for each dictionary entry (it is also possible to use a cut-off threshold), and dump them to a `.far` archive:
```
 $ phonetisaurus-align --input=test.train --ofile=test/test.n3.far --nbest=3 --lattice --seq1_del=false
```

Extract fractional count information from the `.far` archive, and dump it to a counts file:
```
 $ ngramcount --order=7 test/test.n3.far > test/test.n3.cnts
```
Now convert the `.cnts` to text format compatible with SRILM.  Note that in some rare cases 'empty' counts may be produced.  This appears to be a bug somewhere, most likely in the aligner:
```
 $ ngramprint test/test.n3.cnts | grep -v "^  " > test/test.n3.cnts.txt
```

Use the fractional counts to train a `MaxEnt` n-gram model via SRILM:
```
 $ ngram-count -order 7 -read test/test.n3.cnts.txt \
        -maxent-lm test/test.n3.me.arpa \
        -maxent-convert-to-arpa -float-counts
```
CHANGED.  The plugin has been changed so the above command is no longer valid.  Should be:
```
 $ ngram-count -text test.corpus -maxent -lm test.arpa -order 7
```
the model will now be dumped in ARPA format by default.

The final result of the above command will be an ARPA format `MaxEnt` n-gram model incorporating fractional alignment counts from the un-restricted 3-best alignments for each entry in the dictionary.  It may now be converted to WFST format, or combined with other models.

Of course the above only scratches the surface.  The idea here is mainly to note that the approach is quite flexible and many other combination techniques should be both viable and fast to train.