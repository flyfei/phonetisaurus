#!/usr/bin/python
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
# Toy client to connect to the server and run through a test set.
import socket, sys, struct
import json, math

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(('localhost', 8000))

results = []
sents   = []
for line in open(sys.argv[1], "r"):
    line = line.strip()
    sents.append(line)

t = 0
total_prob = 0.0
for sss in sents:
    t += 1
    size = s.send(sss)
    continue_recv = True
    data = s.recv(4)
    length = struct.unpack ("!L", data)[0]
    print sss
    data = s.recv (length)
    response = json.loads (data)
    print "kenlm:", response["kenlm"]
    print "rnnlm:", response["rnnlm"]
    print "KenLM\tRnnLM\tWord"
    print "---------------------"
    for w in response["words"]:
        print "{0:.4f}\t{1:.4f}\t{2}".format (w[1], math.pow(10,w[2]), w[0].encode("utf8"))
    total_prob += response["rnnlm"]
    print ""

print "Total #queries:", t
print "Total prob (rnnlm):", total_prob
s.close()
