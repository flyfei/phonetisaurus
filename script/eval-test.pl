#!/usr/bin/perl -w
use strict;
use warnings;
#Trivial script to compute Word Error Rate (WER)
# for phoneticizer test output.  Assumes that any 
# multiple entries occur just once in the input list
# with multiple pronunciations separated by commas, e.g.,
#    affix    @ f I X,x f I X
# not
#    affix    @ f I X
#    affix    x f I X
#See phonetisaurus/data/g014a2.clean.test for more examples.
#Should achieve a WER of 33.6% on g014a2.clean.test.

my $total = 0;
my $corr  = 0;

while(<STDIN>){
    chomp;
    @_ = split(/\t/);

    chomp($_[1]);
    #remove accent markers, if any
    $_[1] =~ s/[0-9]+//g;
    chomp($_[2]);
    my @prons = split(/,/,$_[2]);
    foreach my $pron(@prons){
	if( $_[1] eq $pron ){
	    $corr++;
	    last;
	}else{
	    if( @ARGV==1 && $ARGV[0] eq "-v" ){
		print $_."\n";
	    }
	}
    }
    $total++;
}

print "Total: ".$total."\n";
print "Corr:  ".$corr."\n";
print "WER:  ".(1-($corr/$total))."\n";
