/*
 * $Id: RegexC.h,v 1.2 2000/11/19 09:21:06 evgeny Exp $
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

#ifndef	_RegexC_h_
#define	_RegexC_h_

/* RegexC.h -- header file for class RegexC
 */

#include "StringC.h"
#include "RangeC.h"
#include "regex.h"

class CharC;

const unsigned	DEFAULT_BUFSIZE	= 64;

class RegexC: public StringC {

   struct re_pattern_buffer	pattern;
   unsigned			ngroups;// 1 + number of \( \) groups in pattern
   struct re_registers		regs;

   void		compile_pattern();
   int		_match(const StringC&, int pos=0);
   int		_match(const CharC&, int pos=0);
   int		_match(const char*, int pos=0);
   void		init(int bufsize);
   void		fixCopy();
   void		setGroups(int);

public:

// Constructors

   inline RegexC(unsigned bufsize=DEFAULT_BUFSIZE) { init(bufsize); }
   inline RegexC(const StringC& rexp, unsigned bufsize=DEFAULT_BUFSIZE)
      : StringC(rexp) {
      init(bufsize);
      compile_pattern();
   }
   inline RegexC(const CharC& rexp, unsigned bufsize=DEFAULT_BUFSIZE)
      : StringC(rexp) {
      init(bufsize);
      compile_pattern();
   }
   inline RegexC(const char *rexp, unsigned bufsize=DEFAULT_BUFSIZE)
      : StringC(rexp) {
      init(bufsize);
      compile_pattern();
   }

// A copy is more efficient than compile_pattern() in this case
   inline RegexC(const RegexC& rexp) : StringC(rexp) {
      pattern = rexp.pattern;
      regs    = rexp.regs;
      fixCopy();
   }
   virtual inline ~RegexC() {
      delete [] pattern.buffer;
      if ( pattern.fastmap ) delete [] pattern.fastmap;
   }

// Cast to (char *)

   inline operator char*() const {
      return (*(StringC*)this);
   }

   inline unsigned	groups() const {
      return ngroups;
   }

// Copy and assignment

   inline void	operator=(const char* cs) {
      *(StringC*)this = cs ? cs : "";
      compile_pattern();
   }
   inline void	operator=(const CharC& c) {
      *(StringC*)this = c;
      compile_pattern();
   }
   inline void	operator=(const StringC& s) {
      *(StringC*)this = s;
      compile_pattern();
   }
   inline void	operator=(const RegexC& r) {
      if (this != &r) { // watch out for x = x
	 *(StringC*)this = r;           // copy the StringC part
	 delete [] pattern.buffer;
	 pattern = r.pattern;
	 regs    = r.regs;
	 fixCopy();
      }
   }

// Compare

   inline int operator==(const RegexC& that) const {
      return StringC::operator==(that);
   }

// Check for match of RegexC at index pos of StringC s.

   inline int	match(const StringC& s, int pos=0) {
      return (_match(s, pos) != -1);
   }
   inline int	match(const CharC& c, int pos=0) {
      return (_match(c, pos) != -1);
   }
   inline int	match(const char *cs, int pos=0) {
      return (_match(cs, pos) != -1);
   }

// Search for next match of RegexC in str beginning at startpos.
// Return position of match or -1 if no match.

   int	search(const StringC&, int, int);
   int	search(const CharC&, int, int);
   int	search(const char*, int, int);

   inline int	search(const StringC& s, int startpos=0)
      { return (search(s, startpos, s.length()-startpos)); }
   inline int	search(const char *cs, int startpos=0)
      { return (cs ? search(cs, startpos, strlen(cs)-startpos):-1); }
   int	search(const CharC&, int startpos=0);

// Conversion

   inline void	toAscii() {
      StringC::toAscii();
      compile_pattern();
   }
   inline void	toUpper() {
      StringC::toUpper();
      compile_pattern();
   }
   inline void	toLower() {
      StringC::toLower();
      compile_pattern();
   }

// Conversion

   inline void	Trim() {
      StringC::Trim();
      compile_pattern();
   }

// Ranges

   inline RangeC	operator[](unsigned i) const {
      if (i >= ngroups) {
	 cerr << "RegexC[" << i << "]: reference out of bounds."
	      << " Valid range is 0:" << ngroups-1 NL;
	 return (RangeC(0,0));
      } else {
	 return (RangeC(regs.start[i], regs.end[i]-regs.start[i]));
      }
   }
};

#endif	// _RegexC_h_
