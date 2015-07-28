### Downloads ###
Google code seems to have ended the ability to create new downloadable releases, so for the final, semi-stable release on googlecode, I'm hosting the tarball via Dropbox:

See also the developing joint-multigram RnnLM decoder (to be merged shortly):

https://github.com/AdolfVonKleist/RnnLMG2P

Update (2014-06-18): This version includes some minor bug fixes:
  * MaxEnt command update for srilm-1.7.1
  * Makefile fix for Linux and g++ v4.6+
  * https://www.dropbox.com/s/154q9yt3xenj2gr/phonetisaurus-0.8a.tgz

Previous version:
  * https://www.dropbox.com/s/k7g94dshfsrknhq/phonetisaurus-0.8.tgz

This is essentially a copy of the Inter Speech 2013 distribution, but included for compatibility, although it is out of sync with the current repository trunk.

The project will soon be migrated to github.
Examples for the current iteration of the toolkit in the trunk can be found here:

  * Basic setup and usage:
    * https://gist.github.com/JosefNovak/e68f1f86a5af184ef00f
  * Advanced usage (ngram-server, rnnlm-server, rescoring):
    * https://gist.github.com/JosefNovak/e68f1f86a5af184ef00f

### Introduction ###
Phonetisaurus is a WFST-driven grapheme-to-phoneme (g2p) framework suitable for rapid development of high quality g2p or p2g systems.  At present it includes a fast, EM-driven, WFST-based multiple-to-multiple alignment program, model conversion tools, a fast WFST-based decoder, and a Lattice Minimum Bayes-Risk decoder implementing a novel length-normalized loss function for computing N-gram factors. A specialized test distribution implementing N-best rescoring with Recurrent Neural Network Language Models via [RNNLM](http://www.fit.vutbr.cz/~imikolov/rnnlm/) is also included.

The project embodies a straight-forward ensemble approach to the g2p problem, and adopts a modular architecture that reflects the alignment,  model-training and decoding steps that are common to most g2p approaches in the related literature.  Fractional smoothing for alignment lattices is now supported via Google NGramTools, but at present their package only supports Witten Bell smoothing for fractional counts.  Fractional counts may also be used in combination with the [SRILM Maximum-Entropy extension](http://www.phon.ioc.ee/dokuwiki/doku.php?id=people:tanel:srilm-me.en) (highly recommended).  The project produces high-quality g2p and p2g results that are competitive with the state-of-the-art in this area.  In addition to a fast C++ decoder which can handle word lists, isolated words, and n-best results, the project also includes training scripts.

See the ReadMe for a bunch of examples.  See the [FAQ page](http://code.google.com/p/phonetisaurus/wiki/FAQ) for solutions to common problems and issues.  There is also a series of slides describing the LMBR decoder: [phonetisaurus-lmbr-g2p.pdf](http://phonetisaurus.googlecode.com/files/phonetisaurus-lmbr-g2p.pdf).  There are also several deprecated tutorials that discuss other aspects of the system.  See the [wiki list](http://code.google.com/p/phonetisaurus/w/list) for details.
<table>
</table>
<table><tr><td>
<img src='http://www.gavo.t.u-tokyo.ac.jp/~novakj/graphs/radical.png' width='800' />
</td></tr>
<tr align='center'><td align='center'>WFST representation of the awesome input word "<code>radical</code>", when using the multiple-to-multiple alignment algorithm.</td></tr></table>

### Coming Soon ###
Full integration and automation for RNNLM-based N-best rescoring.

Language-specific phonotactic template constraints.

### Dependencies ###

Phonetisaurus depends on several other excellent projects, which are listed below.

**[OpenFst](http://openfst.org/)**:  All low-level FST manipulation is handled with the `OpenFst` library.

**Language Model training toolkit**: You may use your favorite toolkit to train an [ARPA-format](http://www.speech.sri.com/projects/srilm/manpages/ngram-format.5.html) LM.  My personal favorite is [mitlm](https://code.google.com/p/mitlm/), but [NGramLibrary](http://www.openfst.org/twiki/bin/view/GRM/NGramLibrary), [SRILM](http://www.speech.sri.com/projects/srilm/), [SRILM MaxEnt extension](http://www.phon.ioc.ee/dokuwiki/doku.php?id=people:tanel:srilm-me.en), [CMU-Cambridge SLM](http://mi.eng.cam.ac.uk/~prc14/toolkit.html) or anything else that is capable of outputting a text-based, ARPA-format LM should also work just fine.  You can also use [SimpleLM](https://github.com/AdolfVonKleist/SimpleLM), a set of toy LM training tools implemented in python.

### Acknowledgments ###
Work on this project was partially funded by the National Institute of Information and Communications Technology ([NICT](http://www.nict.go.jp/about/index-e.html)), Japan.