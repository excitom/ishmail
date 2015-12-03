/*
 * $Id: RegexC.C,v 1.4 2000/11/19 09:21:06 evgeny Exp $
 *
 * Copyright (c) 1991 HaL Computer Systems, Inc.  All rights reserved.
 * 
 *          HAL COMPUTER SYSTEMS INTERNATIONAL, LTD.
 *                  1315 Dell Avenue
 *                  Campbell, CA  95008
 *
 * Author: Greg Hilton
 * Contributors: Tom Lang, Frank Bieser, and others
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * http://www.gnu.org/copyleft/gpl.html
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <config.h>

/* RegexC.c  -- implementation of class RegexC
 *   
 *   Function:
 *   
 *   RegexC is a class derived from StringC and containing a regular
 *   expression and its compiled form.  It implements functions that search
 *   StringCs for and match StringCs with regular expressions using the
 *   regular expression code from the public domain (regex.c)
 *   
 *   Note that when a RegexC is printed, only the StringC portion is printed,
 *   not the compiled form.
 * 
 */

#include "RegexC.h"
#include "CharC.h"

#include <memory.h>

extern "C" {
   char* hre_compile_pattern(
	char*,	// the address of the pattern string
	int size,	// the length of the pattern string
	struct re_pattern_buffer* bufp);
}

/*
hre_compile_pattern takes a regular-expression descriptor string in the
user's format and converts it into a buffer full of byte commands for
matching.

  pattern   is the address of the pattern string
  size      is the length of it.
  bufp	    is a  struct re_pattern_buffer *  which points to the info
	    on where to store the byte commands.
	    This structure contains a  char *  which points to the
	    actual space, which should have been obtained with malloc().
	    compile_pattern may use realloc() to grow the buffer space.

The number of bytes of commands can be found out by looking in the
struct re_pattern_buffer that bufp pointed to, after compile_pattern
returns.
*/

extern "C" {
   int hre_match(struct re_pattern_buffer*, const char*, int, int,
	        struct re_registers*);
}

/*
extern int hre_match_2(
	struct re_pattern_buffer* pbufp,
	char* string1, int size1,
	char* string2, int size2,
	int pos,
	struct re_registers*,
	int mstop);

Match the pattern described by `pbufp' against data which is the
virtual concatenation of `string1' and `string2'.  `size1' and `size2'
are the sizes of the two data strings.  Start the match at position
`pos'.  Do not consider matching past the position `mstop'.

If pbufp->fastmap is nonzero, then it had better be up to date.

The reason that the data to match is specified as two components which
are to be regarded as concatenated is so that this function can be
used directly on the contents of an Emacs buffer.

-1 is returned if there is no match.  Otherwise the value is the
length of the substring which was matched.

hre_match just calls hre_match_2 with size1=0 and mstop=size.
*/

extern "C" {
   int hre_search(struct re_pattern_buffer*, const char*, int, int, int,
		 struct re_registers*);
}

/*
extern int hre_search_2(
	struct re_pattern_buffer*,
	char*, int size1,
	char*, int size2,
	int startpos,
	int range,
	struct re_registers*,
	int mstop);

Like hre_match_2 but tries first a match starting at index `startpos',
then at startpos + 1, and so on.  `range' is the number of places to
try before giving up.  If `range' is negative, the starting positions
tried are startpos, startpos - 1, etc.  It is up to the caller to make
sure that range is not so large as to take the starting position
outside of the input strings.

The value returned is the position at which the match was found, or -1
if no match was found.

hre_search just calls hre_search_2 with size1=0 and mstop=size.
*/

const unsigned BYTEWIDTH = 8;	// width of a byte in bits

void
RegexC::compile_pattern()
{
   char	*error = hre_compile_pattern(*this, length(), &pattern);
   if (error) {
      cerr << error NL;
   }
}

int
RegexC::_match(const StringC& str, int pos)
{
   int	len = hre_match(&pattern, str, str.length(), pos, &regs);
   setGroups(len);
   return (len);
}

int
RegexC::_match(const CharC& c, int pos)
{
   int	len = hre_match(&pattern, (char*)c.Addr(), c.Length(), pos, &regs);
   setGroups(len);
   return (len);
}

int
RegexC::_match(const char *cs, int pos)
{
   int	len = hre_match(&pattern, (char *)cs, strlen(cs), pos, &regs);
   setGroups(len);
   return (len);
}

void
RegexC::init(int bufsize)
{
   pattern.buffer    = new char[bufsize];
   pattern.allocated = bufsize;
   pattern.used      = 0;
   pattern.fastmap   = 0;
   pattern.translate = 0;
   ngroups = 0;
   for (int i=0; i<RE_NREGS; i++) {
      regs.start[i] = regs.end[i] = -1;
   }
}

// copy heap storage in a struct re_pattern_buffer

//*********** IMPORTANT *******************
// Do not delete the old memory.
//    It is still being used by another RegexC
//*****************************************

void
RegexC::fixCopy()
{
   register char*	oldp = pattern.buffer;
   pattern.buffer = new char[pattern.allocated];
   memcpy(pattern.buffer, oldp, pattern.used);

   if (pattern.fastmap) {
      oldp = pattern.fastmap;
      pattern.fastmap = new char[1<<BYTEWIDTH];
      memcpy(pattern.fastmap, oldp, 1<<BYTEWIDTH);
   }
}

// set number of groups matched by last match/search
void
RegexC::setGroups(int result)
{
   ngroups = 0;
   if (result != -1) {
      for (register int i=0; i<RE_NREGS; i++) {
	 if (regs.start[i] == -1) {
	    return;
	 }
	 ngroups++;
      }
   }
}

// Search for next match of RegexC in str beginning at startpos.
// Try no more than abs(range) times.
// Search backwards if range < 0.
// Return position of match or -1 if no match.
int
RegexC::search(const StringC& str, int startpos, int range)
{
   if (!pattern.fastmap)
      pattern.fastmap = new char[1<<BYTEWIDTH];
   int	pos = hre_search(&pattern, str, str.length(), startpos, range, &regs);
   setGroups(pos);
   return (pos);
}

int
RegexC::search(const CharC& c, int startpos, int range)
{
   if (!pattern.fastmap)
      pattern.fastmap = new char[1<<BYTEWIDTH];
   int	pos = hre_search(&pattern, (char*)c.Addr(), c.Len(), startpos, range,
   			 &regs);
   setGroups(pos);
   return (pos);
}

int
RegexC::search(const char *cs, int startpos, int range)
{
   if ( !cs ) {
      return -1;
   }
   if (!pattern.fastmap)
      pattern.fastmap = new char[1<<BYTEWIDTH];
   int	pos = hre_search(&pattern, (char *)cs, strlen(cs), startpos, range,
			&regs);
   setGroups(pos);
   return (pos);
}

int
RegexC::search(const CharC& c, int startpos)
{
   if (!pattern.fastmap)
      pattern.fastmap = new char[1<<BYTEWIDTH];
   int	pos = hre_search(&pattern, (char *)c.Addr(), c.Length(), startpos,
   			 c.Length()-startpos, &regs);
   setGroups(pos);
   return (pos);
}

