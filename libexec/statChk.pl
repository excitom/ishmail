#!/bin/sh
exec perl -x $0 ${1+"$@"}
#!perl
#
# Scan mail folders for status change.
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

$HOME = $ENV{'HOME'};
$DIR = "$HOME/Mail";

#
# Look for list of "interesting" folders. If not found, check them all.
#
if ( -f "$HOME/folders" ) {
	open (LIST, "<$HOME/folders") || die;
	while (<LIST>) {
		chop;
		push(@folders, $_);
	}
	close LIST;
} else {
	opendir (FOLDERS, "$DIR") || die "Can't read $DIR : $!";
	@folders = grep(!/^\./, readdir(FOLDERS));
	closedir(FOLDERS);
}
#
# Get old status
#
if ( -f "$HOME/folder.stats") {
	open (STATS, "<$HOME/folder.stats") || die;
	while (<STATS>) {
		chop;
		($folder, $mtime) = split(/:/);
		$folders{$folder} = $mtime;
	}
}
#
# Check folder status
#
chdir $DIR;
$changed = 0;
foreach $folder (@folders) {

	($dev,$ino,$mode,$nlink,$uid,$gid,$rdev,$size,$atime,$mtime,$ctime,$blksize,$blocks) = stat $folder;

	if (defined($folders{$folder})) {
		if ($mtime != $folders{$folder}) {
			print "$DIR/$folder has changed\n";
			$folders{$folder} = $mtime;
			$changed = 1;
		}
	} else {
		print "New folder stats for $DIR/$folder\n";
		$folders{$folder} = $mtime;
		$changed = 1;
	}
}
#
# Save status
#
open (STATS, ">$HOME/folder.stats") || die;
foreach $folder (sort keys %folders) {
	print STATS "$folder:$folders{$folder}\n";
}
close STATS;
exit $changed;
