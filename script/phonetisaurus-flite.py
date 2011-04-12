#!/usr/bin/python
import re, os, commands, platform

#2011-04-12 Copyright, Josef R. Novak, all rights reserved.
#
#This script is an example of what can be done with a G2P system.
#
#It plugs G2P hypotheses directly into the flite phoneticizer to 
# illustrate the functionality of the system.
#
#The script requires that both phonetisaurus-g2p and the flite 
# synthesizer are installed and available from your $PATH variable.
#
#Flite can be obtained here: 
#     http://www.speech.cs.cmu.edu/flite/
#
#For best results your G2P model should be trained on the accented
# version of the CMUDICT.

def phoneticizeWithFlite( args ):
    """
      Phoneticize a sentence and hand the result to flite for synthesis.
    """
    
    sentence = args.sent
    print "Generating Phonetisaurus pronunciation for sentence: %s" % sentence
    command = """phonetisaurus-g2p -m MODEL -s "SENT" -n 1""".replace("MODEL",args.model).replace("SENT",sentence.upper())
    print command
    output = commands.getoutput( command )
    
    words = re.split(r"\n",output.strip())
    prons = []
    for word in words:
        score,pron = word.split("\t")
        prons.append(pron)
    sentpron = " pau ".join(prons)
    print "Pronunciation: %s" % sentpron
    print "Converting pronunciation to wavfile with Flite..."

    command = """flite -voice VOICE -p "PHONES" -o /tmp/test.wav""".replace("PHONES",sentpron.lower()).replace("VOICE",args.voice)
    os.system(command)
    print "Playing wavfile..."
    if platform.system()=='Darwin':
        os.system("afplay /tmp/test.wav")
    elif platform.system()=='Linux':
        os.system("play /tmp/test.wav")
    else:
        print "Unsupported OS: %s" % platform.system()

    return

def which(program):
    """
     Nice 'which' snippet from:
      http://stackoverflow.com/questions/377017/test-if-executable-exists-in-python
    """

    import os
    def is_exe(fpath):
        return os.path.exists(fpath) and os.access(fpath, os.X_OK)

    fpath, fname = os.path.split(program)
    if fpath:
        if is_exe(program):
            return program
    else:
        for path in os.environ["PATH"].split(os.pathsep):
            exe_file = os.path.join(path, program)
            if is_exe(exe_file):
                return exe_file

    return None

if __name__=="__main__":
    import sys, argparse

    example = """./phonetisaurus-flite.py --model g2pmodel.fst --sent "some sentence to phoneticize" [--voice somevoice]"""
    parser = argparse.ArgumentParser(description=example)
    parser.add_argument('--model',     "-m", help="The input G2P pronunciation model.  The model should be based on the cmudict.", required=True )
    parser.add_argument('--sent',      "-s", help="A space ' ' separated string of words to phoneticize.", required=True )
    parser.add_argument('--voice',     "-o", help="The preferred voice to use: kal awb_time kal16 awb rms slt .  Run $ flite -lv for more info.", default="rms" )
    parser.add_argument('--verbose',   "-v", help='Verbose mode.', default=False, action="store_true")
    args = parser.parse_args()

    if args.verbose:
        for attr, value in args.__dict__.iteritems():
            print attr, value
        
    if not which("flite"):
        print "Cannot find 'flite' synthesizer in $PATH. Aborting."
        sys.exit()
    if not which("phonetisaurus-g2p"):
        print "Cannot find 'phonetisaurus-g2p' phoneticizer in $PATH. Aborting."
        sys.exit()
    phoneticizeWithFlite( args )
