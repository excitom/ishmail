#!/bin/sh
exec perl -x $0 ${1+"$@"}
#!perl
#
# This is a filter which customizes an e-mail message, based on the 
# contents of an X-Identity: header, prior to sending the message to
# sendmail.
#
# Copyright (c) 1997 H&L Software
# All Rights Reserved
#
# Tom Lang 7/97
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#
# The value to the X-Identity: header is assumed to be the name of a 
# subdirectory of $HOME, containing these files:
# .headers - zero or more header lines added to the message in place of
#	the X-Identity header.
# .signature - a signature file to add to the end of the message
# .from - a file containing the name you want to use for the From: line
#
# Note: some sendmail's may not like you using -f.
#
# Input is copied to a temp file rather than piped directly to sendmail
# since we have to find the X-Identity header first to deterimine the -f
# parameter for sendmail. Only the headers are copied to the temp file,
# to avoid unnecessary copying of the (potentially large) body.
#

$n = "customize.pl";
$tmp = "/tmp/ishmail.$$";
$identity = "";
$from = "";
$HOME = $ENV{'HOME'};
$mimeBdy = "";		# MIME message part boundary
$NL = "";

open (TMP, ">$tmp") || die "$n: Couldn't create temp file $tmp : $!";

#
# scan the incoming headers
#
while (<STDIN>) {
	if (/^X-Identity:/i) {
		chop;
		($junk, $identity) = split(/:/);
		$identity =~ s/^\s+//;
		if ( -f "$HOME/$identity/.headers" ) {
			open (HDRS, "<$HOME/$identity/.headers") || die "$n: couldn't read $HOME/$identity/.headers : $!";
			while (<HDRS>) {
				print TMP;
			}
			close HDRS;
		}
		if ( -f "$HOME/$identity/.from" ) {
			open (FROM, "<$HOME/$identity/.from") || die "$n: couldn't read $HOME/$identity/.from : $!";
			$from = <FROM>;
			chop $from;
			close FROM;
		}
		
	} elsif (/^\s+boundary="(.*)"/i) {
		$mimeBdy = "--$1";	# leading dashes, not pre-decrement
		print TMP;
	} elsif (/^Content-Type: text\/enriched/i) {
		$NL = "\n";
		print TMP;
	} else {
		print TMP;
	}

	last if (/^\n$/);	# end of the headers
}
close TMP;

#
# add -f to the sendmail parameters
#
$dash = 1;
foreach $_ (@ARGV) {
	if (!/^\-/ && $dash) {
		$dash = 0;
		push(@args, "-f \"$from\"") if ($from ne "");
	}
	push(@args, $_);
}

open (MAIL, "| /usr/lib/sendmail @args") || die "$n: Can't pipe to sendmail : $! ";
open (TMP, "<$tmp") || die "$n: Can't read $tmp : $!";
while (<TMP>) {
	print MAIL;
}
close TMP;
unlink $tmp;

#
# copy the body
#
while (<STDIN>) {
	last if ($mimeBdy ne "" && /^$mimeBdy--/);  # that's trailing dashes,
						    # not post decrement.
	print MAIL;
}

#
# copy signature file, if any
#
if (-f "$HOME/$identity/.signature") {
	print MAIL "$mimeBdy\nContent-Type: text/plain\n\n" if ($mimeBdy ne "");
	open (SIG, "<$HOME/$identity/.signature") || die "$n: Can't read $HOME/$identity/.signature : $!";
	print MAIL "\n--\n$NL";
	while (<SIG>) {
		print MAIL "$_$NL";
	}
}
close SIG;

print MAIL "$mimeBdy--\n" if ($mimeBdy ne "");
close MAIL;
exit 0;
