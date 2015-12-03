/*
 * $Id: StringC.h,v 1.3 2000/08/11 04:40:33 kherron Exp $
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

#ifndef	_StringC_h_
#define	_StringC_h_

/* StringC.h -- Character string class
 */

#include "RangeC.h"

#include <string.h>
#include <memory.h>
#include <stdio.h>
#include <sys/types.h>

#include <X11/Intrinsic.h>	// For Boolean

class StringC;
class CharC;

/*
 *  A SubStringC always points into the memory for some StringC no memory is
 *  ever allocated for a SubString
 */

class SubStringC {

   char*	_subp;		// pointer to start of character string
   u_int	_sublen;	// substring length
   StringC	*_parent;	// parent StringC

   void	init(const StringC&, u_int, u_int);

   friend	StringC;

public:

   int	compare(const StringC&) const;
   int	compare(const SubStringC&) const;
   int	compare(const char*) const;

//
// Constructors
//
   SubStringC(const StringC&, u_int, u_int);
   SubStringC(const StringC&, const RangeC&);
   SubStringC(SubStringC&);

// Return size

   inline u_int		length() const	{ return (_sublen); }
   inline u_int		size() const	{ return (_sublen); }

// CAST TO (char *)

   inline	operator const char*() const { return (_subp); }
   inline	operator       char*() const { return (_subp); }

// Check STATUS

   inline int   IsNull()        { return (_subp == NULL); }
   inline int   IsEmpty()       { return (_sublen == 0); }

// Assignment

   void		ReplaceWith(const char*, u_int);
   void		operator=(const StringC&);
   void		operator=(const SubStringC&);
   void		operator=(const CharC&);
   void		operator=(const char*);

// Comparison

   inline int operator< (const StringC& s) const { return (compare(s) <  0); }
   inline int operator> (const StringC& s) const { return (compare(s) >  0); }
   inline int operator<=(const StringC& s) const { return (compare(s) <= 0); }
   inline int operator>=(const StringC& s) const { return (compare(s) >= 0); }
   inline int operator==(const StringC& s) const { return (compare(s) == 0); }
   inline int operator!=(const StringC& s) const { return (compare(s) != 0); }

   int	operator< (const SubStringC& ss) const { return (compare(ss) <  0); }
   int	operator> (const SubStringC& ss) const { return (compare(ss) >  0); }
   int	operator<=(const SubStringC& ss) const { return (compare(ss) <= 0); }
   int	operator>=(const SubStringC& ss) const { return (compare(ss) >= 0); }
   int	operator==(const SubStringC& ss) const { return (compare(ss) == 0); }
   int	operator!=(const SubStringC& ss) const { return (compare(ss) != 0); }

   int	operator< (const char* cs) const { return (compare(cs) <  0); }
   int	operator> (const char* cs) const { return (compare(cs) >  0); }
   int	operator<=(const char* cs) const { return (compare(cs) <= 0); }
   int	operator>=(const char* cs) const { return (compare(cs) >= 0); }
   int	operator==(const char* cs) const { return (compare(cs) == 0); }
   int	operator!=(const char* cs) const { return (compare(cs) != 0); }

   friend int	compare  (const char* cs, const SubStringC& ss);
   inline friend int	operator<(const char* cs, const SubStringC& ss) {
      return (::compare(cs, ss) >  0);
   }
   inline friend int	operator>(const char* cs, const SubStringC& ss) {	
      return (::compare(cs, ss) <  0);
   }
   inline friend int	operator<=(const char* cs, const SubStringC& ss) {
      return (::compare(cs, ss) >= 0);
   }
   inline friend int	operator>=(const char* cs, const SubStringC& ss) {
      return (::compare(cs, ss) <= 0);
   }
   inline friend int	operator==(const char* cs, const SubStringC& ss) {
      return (::compare(cs, ss) == 0);
   }
   inline friend int	operator!=(const char* cs, const SubStringC& ss) {
      return (::compare(cs, ss) != 0);
   }

// Concatenation

   StringC		operator+(const StringC& s);
   StringC		operator+(const SubStringC& ss);
   StringC		operator+(const char* cs);
   friend StringC	operator+(const char* cs, const SubStringC& s);

   u_int		position();
   inline u_int		length() { return _sublen; }

   inline ostream&	printOn(ostream& strm=cout) const {
      if ( _subp ) strm.write(_subp, _sublen);
      return strm;
   }
};

// PRINTING

inline ostream&
operator<<(ostream& strm, const SubStringC& ss)
{
   return ss.printOn(strm);
}

class StringC {

   char		*_p;		// character string
   u_int	_len;		// length of string, excluding null character
   u_int	_alloc;		// amount of storage allocated
   char		autoShrink;	// Controls whether memory is freed if string
   				//   gets smaller.
   char		adjustBlock;	// Controls whether block size increases
   				//   with successive allocations.

   friend	SubStringC;

// CONSTRUCTOR SUPPORT METHODS

   void	reallocate(u_int, char shrinkOk=0);
   				 // Update memory allocation
   void	init();			 // Create a blank string
   void	init(u_int);		 // Create a blank string with initial size
   void	init(const char*);	 // Create a string from a character string
   void	init(const CharC&);	 // Create a string from a character array
   void	init(const SubStringC&); // Create a string from a substring
   void	init(const StringC&);	 // Create a string from a string
   void	Add(const char*, u_int, const char*, u_int, StringC&) const;

public:

   u_int	BLOCK_SIZE;
   const u_int	MAX_BLOCK_SIZE;

//
// Take over control of the given character pointer.  It must point to
//    allocated memory because we will free it when we're done
//
   void	Own(const char*);

// CONSTRUCTORS AND DESTRUCTOR

   inline StringC()
		  : autoShrink(TRUE), adjustBlock(TRUE),
		    BLOCK_SIZE(32), MAX_BLOCK_SIZE(32768)
      { init(1); }
   inline StringC(u_int size)
		  : autoShrink(FALSE), adjustBlock(FALSE),
		    BLOCK_SIZE(size), MAX_BLOCK_SIZE(size)
      { init(size); }

   inline StringC(const StringC& s)
		  : autoShrink(TRUE), adjustBlock(TRUE),
		    BLOCK_SIZE(32), MAX_BLOCK_SIZE(32768)
      { init(s); }
   inline StringC(const StringC& s,     u_int blockSize)
		  : autoShrink(FALSE), adjustBlock(FALSE),
		    BLOCK_SIZE(blockSize), MAX_BLOCK_SIZE(blockSize)
      { init(s); }

   inline StringC(const SubStringC& ss)
		  : autoShrink(TRUE), adjustBlock(TRUE),
		    BLOCK_SIZE(32), MAX_BLOCK_SIZE(32768)
      { init(ss); }
   inline StringC(const SubStringC& ss, u_int blockSize)
		  : autoShrink(FALSE), adjustBlock(FALSE),
		    BLOCK_SIZE(blockSize), MAX_BLOCK_SIZE(blockSize)
      { init(ss); }

   inline StringC(const CharC& c)
		  : autoShrink(TRUE), adjustBlock(TRUE),
		    BLOCK_SIZE(32), MAX_BLOCK_SIZE(32768)
      { init(c); }
   inline StringC(const CharC& c,       u_int blockSize)
		  : autoShrink(FALSE), adjustBlock(FALSE),
		    BLOCK_SIZE(blockSize), MAX_BLOCK_SIZE(blockSize)
      { init(c); }

   inline StringC(const char *cs)
		  : autoShrink(TRUE), adjustBlock(TRUE),
		    BLOCK_SIZE(32), MAX_BLOCK_SIZE(32768)
      { init(cs); }
   inline StringC(const char *cs,	u_int blockSize)
		  : autoShrink(FALSE), adjustBlock(FALSE),
		    BLOCK_SIZE(blockSize), MAX_BLOCK_SIZE(blockSize)
      { init(cs); }

   inline StringC(const char *cs,	Boolean)
		  : autoShrink(TRUE), adjustBlock(TRUE),
		    BLOCK_SIZE(32), MAX_BLOCK_SIZE(32768)
      { init(); Own(cs); }

   virtual inline ~StringC()		{ if (_p) delete [] _p; }

// NULL STRING

   static StringC	Null;

// Check STATUS

   inline int	IsNull() const	{ return (_p == NULL); }
   inline int	IsEmpty() const	{ return (_len == 0); }

// CAST TO (char *)

   inline	operator const char*() const { return _p; }
   inline	operator       char*() const { return _p; }

// INDEXING

   inline char	operator[](u_int i) const {return ((i >= _len) ? 0 : _p[i]);}
   inline char	LastChar() const { return ((_len > 0) ? _p[_len-1] : 0); }

// Set autoShrink

   void		AutoShrink(char);
   inline void	AdjustBlockSize(char val) { adjustBlock = val; }

// Query autoShrink

   char		AutoShrink()   { return autoShrink; }
   char		AdjustBlockSize() { return adjustBlock; }

// ASSIGNMENT FROM StringC and char *

   StringC&	operator=(const StringC&);
   StringC&	operator=(const SubStringC&);
   StringC&	operator=(const CharC&);
   StringC&	operator=(const char*);
   void		Set(const char*, u_int);
   void		CutBeg(u_int); // Remove characters from beginning
   void		CutEnd(u_int); // Remove characters from end
   inline void	CutBoth(u_int l) { CutBeg(l); CutEnd(l); }

// COMPARISON TO StringC

   int	compare(const StringC& s) const;
   inline int	operator<  (const StringC& s) const {return (compare(s) <  0); }
   inline int	operator>  (const StringC& s) const {return (compare(s) >  0); }
   inline int	operator<= (const StringC& s) const {return (compare(s) <= 0); }
   inline int	operator>= (const StringC& s) const {return (compare(s) >= 0); }
   inline int	operator== (const StringC& s) const {return (compare(s) == 0); }
   inline int	operator!= (const StringC& s) const {return (compare(s) != 0); }

// Comparison to SubStringC

   inline int compare(const SubStringC& ss) const
					  { return (ss.compare(*this)); }
   inline int operator< (const SubStringC& ss) const
					  { return (ss.compare(*this) >  0); }
   inline int operator> (const SubStringC& ss) const
					  { return (ss.compare(*this) <  0); }
   inline int operator<=(const SubStringC& ss) const
					  { return (ss.compare(*this) >= 0); }
   inline int operator>=(const SubStringC& ss) const
					  { return (ss.compare(*this) <= 0); }
   inline int operator==(const SubStringC& ss) const
					  { return (ss.compare(*this) == 0); }
   inline int operator!=(const SubStringC& ss) const
					  { return ss.compare(*this) != 0; }

// COMPARISON TO CharC

   int	compare(const CharC& s) const;
   inline int	operator<  (const CharC& c) const {return (compare(c) <  0); }
   inline int	operator>  (const CharC& c) const {return (compare(c) >  0); }
   inline int	operator<= (const CharC& c) const {return (compare(c) <= 0); }
   inline int	operator>= (const CharC& c) const {return (compare(c) >= 0); }
   inline int	operator== (const CharC& c) const {return (compare(c) == 0); }
   inline int	operator!= (const CharC& c) const {return (compare(c) != 0); }

// COMPARISON TO char *

   int	compare(const char* cs) const;
   inline int	operator<  (const char* cs) const { return (compare(cs) <  0); }
   inline int	operator>  (const char* cs) const { return (compare(cs) >  0); }
   inline int	operator<= (const char* cs) const { return (compare(cs) <= 0); }
   inline int	operator>= (const char* cs) const { return (compare(cs) >= 0); }
   inline int	operator== (const char* cs) const { return (compare(cs) == 0); }
   inline int	operator!= (const char* cs) const { return (compare(cs) != 0); }

// COMPARISON FROM char *

   friend int	compare(const char* cs, const StringC& s);
   inline friend int	operator< (const char* cs, const StringC& s) {
      return (::compare(cs, s) <  0);
   }
   inline friend int	operator> (const char* cs, const StringC& s) {
      return (::compare(cs, s) >  0);
   }
   inline friend int	operator<=(const char* cs, const StringC& s) {
      return (::compare(cs, s) <= 0);
   }
   inline friend int	operator>=(const char* cs, const StringC& s) {
      return (::compare(cs, s) >= 0);
   }
   inline friend int	operator==(const char* cs, const StringC& s) {
      return (::compare(cs, s) == 0);
   }
   inline friend int	operator!=(const char* cs, const StringC& s) {
      return (::compare(cs, s) != 0);
   }

// CONCATENATION

   StringC		operator+(const StringC&) const;
   StringC		operator+(const SubStringC&) const;
   StringC		operator+(const CharC&) const;
   StringC		operator+(const char*) const;
   StringC		operator+(char) const;
   StringC		operator+(int) const;
   friend StringC	operator+(const char*, const StringC&);
   friend StringC	operator+(const char*, const SubStringC&);
   friend StringC	operator+(char, const StringC&);
   friend StringC	operator+(int, const StringC&);

// APPENDING

   StringC&	operator+=(const StringC&);
   StringC&	operator+=(const SubStringC&);
   StringC&	operator+=(const CharC&);
   StringC&	operator+=(const char*);
   StringC&	operator+=(char c);
   StringC&	operator+=(int);
   void		Append(const char*, u_int);

// GETTING SUBSTRINGS

   SubStringC		operator()(u_int, u_int);
   const SubStringC	operator()(u_int, u_int) const;
   SubStringC		operator()(const RangeC&);
   const SubStringC	operator()(const RangeC&) const;
   CharC		Range(u_int, u_int);
   const CharC		Range(u_int, u_int) const;
   CharC		Range(const RangeC&);
   const CharC		Range(const RangeC&) const;

// LENGTH AND SIZE

   inline u_int		length() const	{ return (_len); }
   inline u_int		size() const	{ return (_len); }
   inline u_int		capacity() const{ return (_alloc-1); }
   u_int		reSize(u_int new_capacity);

// CLEARING

   void		Clear(int start=0);

// CONVERSION

   void		toAscii();
   void		toLower();
   void		toUpper();


// FILE I/O

   int	AppendFile(char*);
   int	AppendFile(FILE*, u_int count=(u_int)-1);
   int	GetWord(FILE*);
   int	GetLine(FILE*, int trim = 0);
   int	ReadFile(char*);
   int	ReadFile(FILE*, u_int count=(u_int)-1);
   int	WriteFile(char*);
   int	WriteFile(FILE*);

// PRINTING

   inline ostream&	printOn(ostream& strm=cout) const {
      if ( _p ) strm << _p;
      return strm;
   }

//
// Searching at beginning
//
   int StartsWith(const char *cs, u_int clen, u_int offset,
   		  Boolean checkCase=True) const;

   int StartsWith(char, u_int offset, Boolean checkCase=True) const;
   int StartsWith(char c, Boolean checkCase=True) const
      { return StartsWith(c,  0, checkCase); }

   int StartsWith(const char *cs, u_int offset, Boolean checkCase=True) const
      { return StartsWith(cs, cs?strlen(cs):0, offset, checkCase); }
   int StartsWith(const char *cs, Boolean checkCase=True) const
      { return StartsWith(cs, 0, checkCase); }

   int StartsWith(const StringC& s, u_int offset, Boolean checkCase=True) const
      { return StartsWith(s._p, s._len, offset, checkCase); }
   int StartsWith(const StringC& s, Boolean checkCase=True) const
      { return StartsWith(s, 0, checkCase); }

   int StartsWith(const CharC&, u_int offset, Boolean checkCase=True) const;
   int StartsWith(const CharC& c, Boolean checkCase=True) const
      { return StartsWith(c, 0, checkCase); }

//
// Searching at end
//
   int EndsWith(const char *cs, u_int clen, u_int offset,
   		Boolean checkCase=True) const;

   int EndsWith(char c, u_int offset, Boolean checkCase=True) const
      { return StartsWith(c, offset, checkCase); }
   int EndsWith(char c, Boolean checkCase=True) const
      { return StartsWith(c, _len-1, checkCase); }

   int EndsWith(const char *cs, u_int offset, Boolean checkCase=True) const
      { return EndsWith(cs, cs?strlen(cs):0, offset, checkCase); }
   int EndsWith(const char *cs, Boolean checkCase=True) const
      { return EndsWith(cs, _len-1, checkCase); }

   int EndsWith(const StringC& s, u_int offset, Boolean checkCase=True) const
      { return EndsWith(s._p, s._len, offset, checkCase); }
   int EndsWith(const StringC& s, Boolean checkCase=True) const
      { return EndsWith(s, _len-1, checkCase); }

   int EndsWith(const CharC&, u_int offset, Boolean checkCase=True) const;
   int EndsWith(const CharC& c, Boolean checkCase=True) const
      { return EndsWith(c, _len-1, checkCase); }

//
// Comparison
//
   int Equals(const char *cs, u_int clen, u_int offset,
	      Boolean checkCase=True) const;

   int Equals(char, u_int offset, Boolean checkCase=True) const;
   int Equals(char c, Boolean checkCase=True) const
      { return Equals(c,  0, checkCase); }

   int Equals(const char *cs, u_int offset, Boolean checkCase=True) const
      { return Equals(cs, cs?strlen(cs):0, offset, checkCase); }
   int Equals(const char *cs, Boolean checkCase=True) const
      { return Equals(cs, 0, checkCase); }

   int Equals(const StringC& s, u_int offset, Boolean checkCase=True) const
      { return Equals(s._p, s._len, offset, checkCase); }
   int Equals(const StringC& s, Boolean checkCase=True) const
      { return Equals(s, 0, checkCase); }

   int Equals(const CharC&, u_int offset, Boolean checkCase=True) const;
   int Equals(const CharC& c, Boolean checkCase=True) const
      { return Equals(c, 0, checkCase); }

//
// Searching forwards
//
   const char *Search(const char *cs, u_int clen, u_int offset,
	      Boolean checkCase=True) const;

   const char *Search(char, u_int offset, Boolean checkCase=True) const;
   const char *Search(char c, Boolean checkCase=True) const
      { return Search(c,  0, checkCase); }

   const char *Search(const char *cs, u_int offset, Boolean checkCase=True)const
      { return Search(cs, cs?strlen(cs):0, offset, checkCase); }
   const char *Search(const char *cs, Boolean checkCase=True) const
      { return Search(cs, 0, checkCase); }

   const char *Search(const StringC& s, u_int offset, Boolean checkCase=True)
									   const
      { return Search(s._p, s._len, offset, checkCase); }
   const char *Search(const StringC& s, Boolean checkCase=True) const
      { return Search(s, 0, checkCase); }

   const char *Search(const CharC&, u_int offset, Boolean chkCase=True) const;
   const char *Search(const CharC& c, Boolean checkCase=True) const
      { return Search(c, 0, checkCase); }

//
// Searching backwards
//
   const char *RevSearch(const char *cs, u_int clen, u_int offset,
	      Boolean checkCase=True) const;

   const char *RevSearch(char, u_int offset, Boolean checkCase=True) const;
   const char *RevSearch(char c, Boolean checkCase=True) const
      { return RevSearch(c,  _len-1, checkCase); }

   const char *RevSearch(const char *cs, u_int off, Boolean checkCase=True)const
      { return RevSearch(cs, cs?strlen(cs):0, off, checkCase); }
   const char *RevSearch(const char *cs, Boolean checkCase=True) const
      { return RevSearch(cs, _len-1, checkCase); }

   const char *RevSearch(const StringC& s, u_int off, Boolean checkCase=True)
   									   const
      { return RevSearch(s._p, s._len, off, checkCase); }
   const char *RevSearch(const StringC& s, Boolean checkCase=True) const
      { return RevSearch(s, _len-1, checkCase); }

   const char *RevSearch(const CharC&, u_int off, Boolean checkCase=True) const;
   const char *RevSearch(const CharC& c, Boolean checkCase=True) const
      { return RevSearch(c, _len-1, checkCase); }

//
// Searching forwards
//
   inline int	PosOf(      char   c, Boolean checkCase=True) const {
      const char *cs = Search(c, checkCase);
      return (cs ? cs - _p : -1);
   }
   inline int	PosOf(const char  *c, Boolean checkCase=True) const {
      const char *cs = Search(c, checkCase);
      return (cs ? cs - _p : -1);
   }
   inline int	PosOf(const CharC& c, Boolean checkCase=True) const {
      const char *cs = Search(c, checkCase);
      return (cs ? cs - _p : -1);
   }
   inline int	PosOf(const StringC& s, Boolean checkCase=True) const {
      const char *cs = Search(s, checkCase);
      return (cs ? cs - _p : -1);
   }

   inline int	PosOf(      char   c, u_int i, Boolean checkCase=True) const {
      const char *cs = Search(c, i, checkCase);
      return (cs ? cs - _p : -1);
   }
   inline int	PosOf(const char  *c, u_int i, Boolean checkCase=True) const {
      const char *cs = Search(c, i, checkCase);
      return (cs ? cs - _p : -1);
   }
   inline int	PosOf(const CharC& c, u_int i, Boolean checkCase=True) const {
      const char *cs = Search(c, i, checkCase);
      return (cs ? cs - _p : -1);
   }
   inline int	PosOf(const StringC& s, u_int i, Boolean checkCase=True) const {
      const char *cs = Search(s, i, checkCase);
      return (cs ? cs - _p : -1);
   }

//
// Searching backwards
//
   inline int RevPosOf(      char   c, Boolean checkCase=True) const {
      const char *cs = RevSearch(c, checkCase);
      return (cs ? cs - _p : -1);
   }
   inline int RevPosOf(const char  *c, Boolean checkCase=True) const {
      const char *cs = RevSearch(c, checkCase);
      return (cs ? cs - _p : -1);
   }
   inline int RevPosOf(const CharC& c, Boolean checkCase=True) const {
      const char *cs = RevSearch(c, checkCase);
      return (cs ? cs - _p : -1);
   }
   inline int RevPosOf(const StringC& s, Boolean checkCase=True) const {
      const char *cs = RevSearch(s, checkCase);
      return (cs ? cs - _p : -1);
   }

   inline int RevPosOf(      char   c, u_int i, Boolean chkCase=True) const {
      const char *cs = RevSearch(c, i, chkCase);
      return (cs ? cs - _p : -1);
   }
   inline int RevPosOf(const char  *c, u_int i, Boolean chkCase=True) const {
      const char *cs = RevSearch(c, i, chkCase);
      return (cs ? cs - _p : -1);
   }
   inline int RevPosOf(const CharC& c, u_int i, Boolean chkCase=True) const {
      const char *cs = RevSearch(c, i, chkCase);
      return (cs ? cs - _p : -1);
   }
   inline int RevPosOf(const StringC& s, u_int i, Boolean chkCase=True) const {
      const char *cs = RevSearch(s, i, chkCase);
      return (cs ? cs - _p : -1);
   }

//
// Searching
//
   inline int Contains(      char   c, Boolean checkCase=True) const {
      const char *cs = Search(c, checkCase);
      return (cs != NULL);
   }
   inline int Contains(const char  *c, Boolean checkCase=True) const {
      const char *cs = Search(c, checkCase);
      return (cs != NULL);
   }
   inline int Contains(const CharC& c, Boolean checkCase=True) const {
      const char *cs = Search(c, checkCase);
      return (cs != NULL);
   }
   inline int Contains(const StringC& s, Boolean checkCase=True) const {
      const char *cs = Search(s, checkCase);
      return (cs != NULL);
   }

   inline int Contains(      char   c, u_int i, Boolean chkCase=True) const {
      const char *cs = Search(c, i, chkCase);
      return (cs != NULL);
   }
   inline int Contains(const char  *c, u_int i, Boolean chkCase=True) const {
      const char *cs = Search(c, i, chkCase);
      return (cs != NULL);
   }
   inline int Contains(const CharC& c, u_int i, Boolean chkCase=True) const {
      const char *cs = Search(c, i, chkCase);
      return (cs != NULL);
   }
   inline int Contains(const StringC& s, u_int i, Boolean chkCase=True) const {
      const char *cs = Search(s, i, chkCase);
      return (cs != NULL);
   }

//
// Counting
//
   u_int NumberOf(const char *cs, u_int clen, u_int offset,
   		  Boolean checkCase=True) const;

   u_int NumberOf(char, u_int offset, Boolean checkCase=True) const;
   u_int NumberOf(char c, Boolean checkCase=True) const
      { return NumberOf(c,  0, checkCase); }

   u_int NumberOf(const char *cs, u_int offset, Boolean checkCase=True) const
      { return NumberOf(cs, cs?strlen(cs):0, offset, checkCase); }
   u_int NumberOf(const char *cs, Boolean checkCase=True) const
      { return NumberOf(cs, 0, checkCase); }

   u_int NumberOf(const StringC& s, u_int offset, Boolean checkCase=True) const
      { return NumberOf(s._p, s._len, offset, checkCase); }
   u_int NumberOf(const StringC& s, Boolean checkCase=True) const
      { return NumberOf(s, 0, checkCase); }

   u_int NumberOf(const CharC&, u_int offset, Boolean checkCase=True) const;
   u_int NumberOf(const CharC& c, Boolean checkCase=True) const
      { return NumberOf(c, 0, checkCase); }

//
// Misc
//
   void		Replace(CharC, CharC);	// Replace first string with second
   void		Replace(char, char);	// Replace first character with second
   void		Trim();		// Remove whitespace at ends
   CharC	NextWord(u_int offset, const char *white=" \t\n");
			// Return the range of the next word that is delimited
			//    by any chars in "white"
   CharC	NextWord(u_int offset, char c);
			// Return the range of the next word that is delimited
			//    by the character c
};

// PRINTING

inline ostream&
operator<<(ostream& strm, const StringC& s)
{
   return s.printOn(strm);
}

#endif // _StringC_h_
