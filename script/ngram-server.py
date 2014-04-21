#!/usr/bin/env python
# Copyright (c) Twisted Matrix Laboratories.
# Transformed into a cheesy ngram-server by JRN
# See LICENSE for details.
#
# Copyright (c) [2014-], Yandex, LLC
# Author: jorono@yandex-team.ru (Josef Robert Novak)
# All rights reserved.
#
#   Redistribution and use in source and binary forms, with or without
#   modification, are permitted #provided that the following conditions
#   are met:
#
#   * Redistributions of source code must retain the above copyright
#   notice, this list of conditions and the following disclaimer.
#   * Redistributions in binary form must reproduce the above
#   copyright notice, this list of #conditions and the following
#   disclaimer in the documentation and/or other materials provided
#   with the distribution.
#
#   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
#   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
#   FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
#   COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
#   INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
#   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
#   SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
#   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
#   STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
#   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
#   OF THE POSSIBILITY OF SUCH DAMAGE.
#
# \file
# A simplistic example of an ngram-server for the rnnlm bindings.
# This is based on the twisted-python echo-server example.
# It fields requests in the form of a string, and returns a JSON
# object consisting of the sentence-prob given the RNNLM, as well 
# as the probabilities of the individual tokens. There are also some
# stubs for KenLM, but we don't use these currently for the G2P.
from twisted.internet.protocol import Protocol, Factory
from twisted.internet import reactor
#import kenlm, 
import struct, re
from RnnLM import RnnLMPy
import json, sys
from json import encoder
encoder.FLOAT_REPR = lambda o: format(o, '.4f')

### Protocol Implementation
def FormatKenLMResult (response) :
    return response

def FormatRnnLMResult (response) :
    return response

def FormatG2PResult (response):
    return response

# This is just about the simplest possible ngram server
class NgramServer (Protocol):
    def dataReceived(self, data):
        """
         Compute the probability of the requested sentence
         for both a standard n-gram model, and an rnnlm
        """
        #Setup
        #response = {"kenlm": 0., "rnnlm": 0., "words" : []}
        words    = re.split(r"\s+", data) + ["</s>"]
        #response["words"] = [[w, 0., 0.] for w in words]

        #KenLM
        #response["kenlm"] = model.score (data)
        #for i, (prob, length) in enumerate (model.full_scores (data)):
        #    response["words"][i][1] = prob

        #RnnLM
        #rnnlm_result = rnnlm.EvaluateSentence (words)
        #response["rnnlm"] = rnnlm_result.sent_prob
        #for i, prob in enumerate (rnnlm_result.word_probs) :
        #    response["words"][i][2] = prob

        #Package everything
        response_string = "RESPONSE" #json.dumps (response)

        #First send the size of the full response so the client 
        # will be able to know how much to read
        self.transport.write(struct.pack("!L", len(response_string)))
        #Finally send the response
        self.transport.write(response_string)


def main (rnnlm, port=8000) :
    f = Factory()
    f.protocol = NgramServer
    #f.protocol.model = model
    f.protocol.rnnlm = rnnlm
    reactor.listenTCP(port, f)
    reactor.run()

if __name__ == '__main__':
    import sys, argparse
    
    example = "USAGE: {0} <rnnlm>".format (sys.argv[0])
    parser  = argparse.ArgumentParser (description = example)
    parser.add_argument ("--rnnlm",   "-r", help="RnnLM to use.", required=True)
    parser.add_argument ("--port",    "-p", help="Port to run the server on",
                         type=int, default=8000)
    parser.add_argument ("--verbose", "-v", help="Verbose mode", default=False, 
                         action="store_true")
    args = parser.parse_args ()

    if args.verbose :
        for k,v in args.__dict__.iteritems () :
            print k, "=", v

    if args.verbose :
        print >> sys.stderr, "Loading RnnLM..."
    rnnlm = RnnLMPy (args.rnnlm)
    if args.verbose :
        print >> sys.stderr, "Ready!"

    main (rnnlm)
