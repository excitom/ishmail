/*
 * $Id: StringC.C,v 1.6 2000/07/24 13:24:06 evgeny Exp $
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

/* StringC.c -- implementation of character strings
 *
 *  Function:
 *  	
 *  Class StringC implements character string objects.  Operations provided
 *  include & (concatenation) and () (substring extraction).  Type
 *  conversions between StringC and char* are provided, permitting the two
 *  to be used interchangeably in many contexts.
 *
 *
 */

#include "StringC.h"
#include "CharC.h"
#include "StrCase.h"

#include <unistd.h>

extern int	debug1, debug2;

/*-----------------------------------------------------------------------
 * Comparison method
 */

static int
compare(const char *cs1, u_int len1, const char *cs2, u_int len2)
{
   if ( (!cs1 && !cs2) || (len1 == 0 && len2 == 0) ) return  0;	// two nulls
   if ( !cs1 || len1 == 0 )			     return -1;	// string1 null
   if ( !cs2 || len2 == 0 )			     return  1;	// string2 null

//
// Check only up to the end of the shortest string
//
   u_int	minLen = MIN(len1, len2);
   int		result = memcmp(cs1, cs2, minLen);

//
// If these first characters don't match return the result
// If they do match and the lengths are equal, then return the result (0)
//
   if ( result != 0 || len1 == len2 ) return result;

//
// If they do match and the lengths are not equal, compare the lengths
//
   return (len1>len2 ? 1 : -1);

} // End compare

//---------------------------------------------------------------------------
//  Null string

StringC StringC::Null;

/*---------------------------------------------------------------------------
 * Substring constructors
 */

SubStringC::SubStringC(const StringC& s, unsigned pos, unsigned len)
{
   init(s, pos, len);
}

SubStringC::SubStringC(const StringC& s, const RangeC& r)
{
   init(s, r.firstIndex(), r.length());
}

SubStringC::SubStringC(SubStringC& ss)
{
   _parent = ss._parent;
   _subp   = ss._subp;
   _sublen = ss._sublen;
}

/*---------------------------------------------------------------------------
 * Method to initialize substring
 */

void
SubStringC::init(const StringC& s, unsigned pos, unsigned len)
{
   if ( pos >= s._len ) pos = len = 0;
   if ( pos+len > s._len ) len = s._len - pos;

   if ( s._p ) {
      _subp   = &s._p[pos];
      _sublen = len;
   } else {
      _subp   = 0;
      _sublen = 0;
   }

   _parent = (StringC *)&s;

} // End SubStringC init

/*---------------------------------------------------------------------------
 * Method to update memory allocation
 */

void
StringC::reallocate(unsigned count, char shrinkOk)
{
//
// Allocate enough blocks to hold count chars plus the null terminator
//
   count++;

   char growing   = (_alloc < count);
   char shrinking = (shrinkOk && _alloc > 32 && count <= (_alloc>>1));

//
// If we're shrinking, reduce the block size until it becomes smaller than
//    the amount to be allocated
//
   if ( shrinking ) {
      while ( BLOCK_SIZE > count && BLOCK_SIZE > 32 )
	 BLOCK_SIZE /= 2;
   }

//
// See if we're reallocating
//
   if ( growing || shrinking ) {

//
// Count the number of memory blocks needed
//
      unsigned	blockSize = BLOCK_SIZE>0 ? BLOCK_SIZE : 32;
      unsigned	blocks    = count / blockSize;
      if ( count % blockSize ) blocks++;
      if ( blocks < 1 ) blocks = 1;

//
// Increase the block size if necessary for next time
//
      if ( adjustBlock && BLOCK_SIZE < MAX_BLOCK_SIZE ) {
         BLOCK_SIZE *= 2;
	 if ( BLOCK_SIZE > MAX_BLOCK_SIZE ) BLOCK_SIZE = MAX_BLOCK_SIZE;
      }

      if ( debug2 ) {
	 cout <<"StringC::reallocate:" 
	    <<" _len=" << _len
	    <<" _alloc=" << _alloc
	    <<" count=" << count
	    <<" new_alloc=" << blocks * blockSize
	    <<endl;
      }

//
// Allocate the memory
//
      _alloc = blocks * blockSize;
      char *op = _p;

      _p = new char[_alloc];

//
// Copy the old data if present
//
      if ( op ) {

//
// Copy only as much as will now fit
//
	 if ( _len >= count ) _len = count-1;

	 memcpy(_p, op, _len);
	 _p[_len] = 0;
	 delete [] op;
      }

      else
	 *_p = 0;

   } // End if we need to reallocate

} // End StringC::reallocate

/*---------------------------------------------------------------------------
 * Method to resize string
 */
unsigned
StringC::reSize(unsigned new_capacity)
{
   reallocate(new_capacity, autoShrink);
   return (_alloc-1);
}

/*---------------------------------------------------------------------------
 * Replace this substring with the given string
 * The parent of this substring can be divided into 3 parts:
 *
 *           ------head-------- substring --tail--
 *  String:  ccccccccccccccccccSSSSSSSSSSScccccccc
 *
 * The substring is replaced using this process:
 *    1) Copy head to another string
 *    2) Copy replacement to other string
 *    3) Copy tail to other string
 *    4) Copy other string to parent
 *    5) Reset substring length
 */

void
SubStringC::ReplaceWith(const char *cs, u_int clen)
{
   int	spos = position();	// start position of substring

//
// Save the part to the right of this substring (the tail)
//
   int		tpos = spos + _sublen;
   int		tlen = _parent->length() - tpos;
   StringC	tail = (*_parent)(tpos,tlen);

//
// If the source points into the destination, we've got to make copies
//
   if ( cs >= _parent->_p && cs <= _parent->_p + _parent->_len ) {
      StringC	head = (*_parent)(0, spos);
      StringC	body(clen); body.Set(cs, clen);
      *_parent = head;
      *_parent += body;
      *_parent += tail;
   }

   else {

//
// Clear the parent from the start of this substring to the end
//
      _parent->Clear(spos);

//
// Add the new text and the tail
//
      _parent->Append(cs, clen);
      *_parent += tail;
   }

} // End SubStringC ReplaceWith

void
SubStringC::operator=(const StringC& s)
{
   ReplaceWith(s._p, s._len);
}

void
SubStringC::operator=(const SubStringC& ss)
{
//
// It's not clear what this method should do, but the latter is more useful
//
#if 1
   _parent = ss._parent;
   _subp   = ss._subp;
   _sublen = ss._sublen;
#else
   ReplaceWith(ss._subp, ss._sublen);
#endif
}

void
SubStringC::operator=(const CharC& c)
{
   ReplaceWith(c.Addr(), c.Length());
}

void
SubStringC::operator=(const char* cs)
{
   ReplaceWith(cs, cs ? strlen(cs) : 0);
}

/*---------------------------------------------------------------------------
 * Substring concatentation
 */

StringC
SubStringC::operator+(const StringC& s)
{
   StringC      t(_sublen + s._len);
   if ( _subp ) memcpy(t._p, _subp, _sublen);
   if ( s._p  ) memcpy(&t._p[_sublen], s._p, s._len);
   t._len = _sublen + s._len;
   t._p[t._len] = 0;
   return (t);
}

StringC
SubStringC::operator+(const SubStringC& ss)
{
   StringC      s(ss);
   return (operator+(s));
}

StringC
SubStringC::operator+(const char* cs)
{
   if (cs) {
      StringC   s(cs);
      return (operator+(s));
   } else {
      return ((StringC)*this);
   }
}

/*---------------------------------------------------------------------------
 * Methods to compare against substring
 */

int
SubStringC::compare(const StringC& s) const
{
   return ::compare(_subp, _sublen, s._p, s._len);
}

int
SubStringC::compare(const SubStringC& ss) const
{
   return ::compare(_subp, _sublen, ss._subp, ss._sublen);
}

int
SubStringC::compare(const char *cs) const
{
   return ::compare(_subp, _sublen, cs, cs?strlen(cs):0);
}

/*---------------------------------------------------------------------------
 * Return position
 */

unsigned
SubStringC::position()
{
   return (_subp - _parent->_p);
}

/*---------------------------------------------------------------------------
 * String initialization methods
 */

void
StringC::init()
{
   _len       = 0;
   _alloc     = 0;
   _p         = NULL;
}

void
StringC::init(unsigned count)
{
   init();
   reallocate(count);
   *_p = 0;
}

void
StringC::init(const char *cs)
{
   if ( cs ) {
      init((unsigned)strlen(cs));
      strcpy(_p, cs);
      _len = strlen(cs);
   } else {
      init();
   }
}

void
StringC::init(const SubStringC& ss)
{
   if ( ss._subp ) {
      init(ss._sublen);
      _len = ss._sublen;
      memcpy(_p, ss._subp, _len);
      _p[_len] = 0;
   } else {
      init();
   }
}

void
StringC::init(const CharC& c)
{
   if ( c.Addr() ) {
      init(c.Length());
      _len = c.Length();
      memcpy(_p, c.Addr(), _len);
      _p[_len] = 0;
   } else {
      init();
   }
}

void
StringC::init(const StringC& s)
{
   if ( s._p ) {
      init(s._len);
      _len = s._len;
      memcpy(_p, s._p, _len);
      _p[_len] = 0;
   } else {
      init();
   }
}

/*---------------------------------------------------------------------------
 * Method to give us ownership of an allocated string pointer
 */

void
StringC::Own(const char *cs)
{
   if ( _p ) delete [] _p;
   _p     = (char*)cs;
   _len   = cs ? strlen(cs) : 0;
   _alloc = cs ? _len + 1   : 0;
}

/*---------------------------------------------------------------------------
 * String assignment methods
 */

void
StringC::Set(const char *cs, u_int clen)
{
   if ( cs ) {

//
// If cs points into this string, make a copy
//
      if ( cs >= _p && cs <= (_p + _len) ) {
	 CharC	data(cs, clen);
	 StringC   t(data);
	 *this = t;
      }

      else {

//
// If the new string is larger or if we're allowed to shrink and the new
//   string requires less than half the allocated space, reallocate memory
//
	 if ( clen >= _alloc ||
	      (autoShrink && _alloc > 32 && clen <= (_alloc>>1)) ) {
	    if ( _p ) {
	       delete [] _p;
	       _p = NULL;
	       _alloc = 0;
	    }
	    reallocate(clen, autoShrink);
	 }

	 _len = clen;
	 memcpy(_p, cs, _len);
	 _p[_len] = 0;
      }

   } // End if source address is valid

   else {

      if ( _p ) delete [] _p;
      init();
   }

} // End Assign char* and length

StringC&
StringC::operator=(const StringC& s)
{
   Set(s._p, s._len);
   return *this;
}

StringC&
StringC::operator=(const SubStringC& ss)
{
   Set(ss._subp, ss._sublen);
   return *this;
}

StringC&
StringC::operator=(const CharC& c)
{
   Set(c.Addr(), c.Length());
   return *this;
}

StringC&
StringC::operator=(const char *cs)
{
   Set(cs, cs?strlen(cs):0);
   return *this;
}

/*---------------------------------------------------------------------------
 * String concatentation methods
 */

void
StringC::Add(const char *p1, u_int len1, const char *p2, u_int len2, StringC& t)
const
{
   if ( p1 ) memcpy(t._p, p1, len1);
   if ( p2 ) memcpy(&t._p[len1], p2, len2);
   t._len = len1 + len2;
   t._p[t._len] = 0;
}

StringC
StringC::operator+(const StringC& s) const
{
   StringC	t(_len+s._len);
   Add(_p, _len, s._p, s._len, t);
   return t;
}

StringC
StringC::operator+(const SubStringC& ss) const
{
   StringC	t(_len+ss._sublen);
   Add(_p, _len, ss._subp, ss._sublen, t);
   return t;
}

StringC
StringC::operator+(const CharC& c) const
{
   StringC	t(_len+c.Length());
   Add(_p, _len, c.Addr(), c.Length(), t);
   return t;
}

StringC
StringC::operator+(const char* cs) const
{
   u_int	clen = cs ? strlen(cs) : 0;
   StringC	t(_len+clen);
   Add(_p, _len, cs, clen, t);
   return t;
}

StringC
StringC::operator+(char c) const
{
   StringC	t(_len+1);
   if ( _p ) memcpy(t._p, _p, _len);	// Copy first string
   t._len = _len;
   t._p[t._len++] = c;		// Add character to end
   t._p[t._len] = 0;
   return (t);
}

StringC
operator+(const char* cs, const StringC& s)
{
   StringC	t(cs);
   return (t + s);
}

StringC
operator+(char c, const StringC& s)
{
   char	cs[2];
   cs[0] = c;
   cs[1] = 0;
   StringC	t(cs);
   return (t + s);
}

StringC
operator+(int i, const StringC& s)
{
   char	is[MAX_DIGITS_IN_INT];
   sprintf (is, "%d", i);
   return (is + s);
}

StringC
StringC::operator+(int i) const {
   char      is[MAX_DIGITS_IN_INT];	// Can't inline because of array
   sprintf (is, "%d", i);
   return (*this + is);
}

/*---------------------------------------------------------------------------
 * String appending methods
 */

void
StringC::Append(const char *cs, u_int clen)
{
   if ( cs == NULL || clen == 0 ) return;

//
// If cs points into this string, make a copy
//
   if ( cs >= _p && cs <= (_p + _len) ) {
      CharC	data(cs, clen);
      StringC   t(data);
      *this += t;
   }

   else {

      reallocate(_len + clen);

      if ( _p ) {
	 memcpy(&_p[_len], cs, clen);
	 _p[_len+clen] = 0;
      }

      _len += clen;
   }

} // End Append

StringC&
StringC::operator+=(const StringC& s)
{
   Append(s._p, s._len);
   return *this;
}

StringC&
StringC::operator+=(const SubStringC& ss)
{
   Append(ss._subp, ss._sublen);
   return *this;
}

StringC&
StringC::operator+=(const CharC& c)
{
   Append(c.Addr(), c.Length());
   return *this;
}

StringC&
StringC::operator+=(const char* cs)
{
   Append(cs, cs ? strlen(cs) : 0);
   return *this;
}

StringC&
StringC::operator+=(char c)
{
   reallocate(_len+1);			// Add more space if necessary
   _p[_len++] = c;			// Add character in last position
   _p[_len] = 0;
   return *this;
}

StringC&
StringC::operator+=(int i) {
   char      is[MAX_DIGITS_IN_INT];	// Can't inline because of array
   sprintf (is, "%d", i);
   Append(is, strlen(is));
   return *this;
}

/*---------------------------------------------------------------------------
 * Methods to return substrings
 */

SubStringC
StringC::operator()(unsigned pos, unsigned len)
{
   SubStringC   sub(*this, pos, len);
   return (sub);
}

SubStringC
StringC::operator()(const RangeC& r)
{
   SubStringC   sub(*this, r);
   return (sub);
}

const SubStringC
StringC::operator()(unsigned pos, unsigned len) const
{
   SubStringC   sub(*this, pos, len);
   return (sub);
}

const SubStringC
StringC::operator()(const RangeC& r) const
{
   SubStringC   sub(*this, r);
   return (sub);
}

const
CharC
StringC::Range(u_int first, u_int length) const
{
   if ( _len == 0 ) return *this;

   const char	*newp;
   u_int	newlen;

   if ( first >= _len ) {
      newp   = _p + _len - 1;
      newlen = 0;
   }

   else if ( first + length > _len ) {
      newp   = _p + first;
      newlen = _len - first;
   }

   else {
      newp   = _p + first;
      newlen = length;
   }

   return CharC(newp, newlen);
}

const
CharC
StringC::Range(const RangeC& r) const
{
   return Range(r.firstIndex(), r.length());
}

CharC
StringC::Range(u_int first, u_int length)
{
   if ( _len == 0 ) return *this;

   const char	*newp;
   u_int	newlen;

   if ( first >= _len ) {
      newp   = _p + _len - 1;
      newlen = 0;
   }

   else if ( first + length > _len ) {
      newp   = _p + first;
      newlen = _len - first;
   }

   else {
      newp   = _p + first;
      newlen = length;
   }

   return CharC(newp, newlen);
}

CharC
StringC::Range(const RangeC& r)
{
   return Range(r.firstIndex(), r.length());
}

/*---------------------------------------------------------------------------
 *  The following compare functions were implemented because strncmp is
 *  not adequate for comparing character strings of unequal length.  For
 *  example, strncmp("abc","abcd",3) will return 0.
 */

int
StringC::compare(const StringC& s) const
{
   return ::compare(_p, _len, s._p, s._len);
}

int
StringC::compare(const CharC& c) const
{
   return ::compare(_p, _len, c.Addr(), c.Length());
}

int
StringC::compare(const char *cs) const
{
   return ::compare(_p, _len, cs, cs?strlen(cs):0);
}

int
compare(const char *cs, const StringC& s)
{
   return ::compare(cs, cs?strlen(cs):0, s._p, s._len);
}

StringC
operator+(const char* cs, const SubStringC& ss)
{
   if (cs) {
      StringC	s(cs);
      return (s + ss);
   } else {
      return ((StringC)ss);
   }
}

int
compare(const char *cs, const SubStringC& ss)
{
   StringC   s(cs);
   return s.compare(ss);
}

/*---------------------------------------------------------------------------
 *  Remove whitespace from ends
 */

void
StringC::Trim()
{
   if ( !_p || _len == 0 ) return;

//
// Remove from end
//
   char	*cp = _p + _len - 1;
   while ( (cp >= _p) && (*cp == ' ' || *cp == '\t' || *cp == '\n') )
      cp--, _len--;
   *(cp+1) = 0;

//
// Remove from beginning
//
   cp = _p;
   while ( (*cp != 0) && (*cp == ' ' || *cp == '\t' || *cp == '\n') )
      cp++, _len--;

   memcpy(_p, cp, _len);
   *(_p+_len) = 0;

//
// If we're allowed to shrink, see if memory needs to be freed
//
   if ( autoShrink ) reallocate(_len, True);

} // End Trim

/*---------------------------------------------------------------------------
 *  Clear the string starting with the specified position
 */

void
StringC::Clear(int start)
{
   if ( start < _len ) {

//
// Mark the end of the string and update the length
//
      if ( _p ) *(_p+start) = 0;
      _len = start;

//
// If we're allowed to shrink, see if memory needs to be freed
//
      if ( autoShrink ) reallocate(_len, True);
   }
}

/*---------------------------------------------------------------------------
 *  Conversion methods
 */

void
StringC::toAscii()
{
   register unsigned	i = _len;
   register char	*q = _p;
   while (i--) {
      *q = toascii(*q);
      q++;
   }
}

void
StringC::toLower()
{
   register unsigned	i = _len;
   register char	*q = _p;
   while (i--) {
      *q = to_lower(*q);
      q++;
   }
}

void
StringC::toUpper()
{
   register unsigned	i = _len;
   register char	*q = _p;
   while (i--) {
      *q = to_upper(*q);
      q++;
   }
}

/*---------------------------------------------------------------------------
 *  Method to read a word separated by white space from a file
 */

int
StringC::GetWord(FILE *fp)
{
//
// Clear it...
//
   *this = "";

//
// Skip over the initial white space...
//
   register int	c = getc(fp);
   while ( c == ' ' || c == '\n' || c == '\t' ) c=getc(fp);

//
// Loop until we hit EOF, newline, or white space...
//
   while ( c != EOF && c != ' ' && c != '\t' && c != '\n' ) {
      *this += (char)c;
      c=getc(fp);
   }

   return c;
}  // end GetWord()


/*---------------------------------------------------------------------------
 *  Method to read a line from a file
 */

int
StringC::GetLine(FILE *fp, int trim)
{
#define MAXLINE 1024
   char	line[MAXLINE];

   register int	  c = getc(fp);
   register char  *cp = line;
   register char  *ep = &line[MAXLINE-1];

//
// If we're trimming then skip the white space.
//
   if ( trim ) {
      while ( c != EOF && (c == ' ' || c == '\t') ) c=getc(fp);
   }

//
// Loop until we hit EOF or newline
//
   while ( c != EOF && c != '\n' ) {

      *cp++ = c;

//
// See if we've filled our buffer
//
      if ( cp == ep ) {
	 *cp = 0;
	 *this += line;
	 cp = line;
      }

//
// Get another character
//
      c = getc(fp);

   } // End for each character

//
// Add the last bit
//
   if ( cp != line ) {
      *cp = 0;
      *this += line;
   }

   if ( c != EOF && _p == 0 )
      *this = "";
   else if ( trim && _len ) {    // trim the right edge.
      cp = _p + _len - 1;
      while ( _len > 0 && (*cp == ' ' || *cp == '\t') ) {
	 _len--;
	 cp--;
      }
      cp++;
      *cp = 0;
   }

   return c;
}

/*---------------------------------------------------------------------------
 *  Method to read the specified number of bytes from an open file and add
 *     them to the end of this string.
 */

int
StringC::AppendFile(FILE *fp, unsigned count)
{
//
// If length not specified, see how far to end
//
   if ( count == (unsigned)-1 ) {
      long	cur  = ftell(fp);
      fseek(fp, 0, SEEK_END);
      long	last = ftell(fp);
      count = (unsigned)(last - cur);
      fseek(fp, cur, SEEK_SET);
   }

//
// Allocate space and read data
//
   reallocate(_len + count);
   unsigned	readCount = fread(_p+_len, 1, count, fp);
   int	error = (readCount != count);

   _len += readCount;
   _p[_len] = 0;

   return !error;

} // End AppendFile

/*---------------------------------------------------------------------------
 *  Method to read a file and add it to the end of this string
 */

int
StringC::AppendFile(char *fname)
{
//
// Open file
//
   FILE	*fp = fopen(fname, "r");
   if ( !fp ) return 0;

   int	success = AppendFile(fp);
   fclose(fp);

   return success;

} // End AppendFile

/*---------------------------------------------------------------------------
 *  Method to read the specified number of bytes from an open file and
 *     replace the contents of this string
 */

int
StringC::ReadFile(FILE *fp, unsigned count)
{
//
// If length not specified, see how far to end
//
   if ( count == (unsigned)-1 ) {
      long	cur  = ftell(fp);
      fseek(fp, 0, SEEK_END);
      long	last = ftell(fp);
      count = (unsigned)(last - cur);
      fseek(fp, cur, SEEK_SET);
   }

//
// Allocate space and read data
//
   reallocate(count, autoShrink);
   unsigned	readCount = fread(_p, 1, count, fp);
   int	error = (readCount != count);

   _len = readCount;
   _p[_len] = 0;

   return !error;

} // End ReadFile

/*---------------------------------------------------------------------------
 *  Method to read a file into the string, replacing the current contents
 */

int
StringC::ReadFile(char *fname)
{
//
// Open file
//
   FILE	*fp = fopen(fname, "r");
   if ( !fp ) return 0;

   int	success = ReadFile(fp);
   fclose(fp);

   return success;

} // End ReadFile

/*---------------------------------------------------------------------------
 *  Method to append the string to an open file
 */

int
StringC::WriteFile(FILE *fp)
{
   int	error = (fwrite(_p, 1, _len, fp) != _len);
   if ( !error ) error = (fflush(fp) != 0);
   return !error;
}

/*---------------------------------------------------------------------------
 *  Method to write the string to a file
 */

int
StringC::WriteFile(char *fname)
{
//
// Open file
//
   FILE	*fp = fopen(fname, "w+");
   if ( !fp ) return 0;

   int	success = WriteFile(fp);
   fclose(fp);

   return success;

} // End WriteFile

/*---------------------------------------------------------------------------
 *  Method to change auto shrinking
 */

void
StringC::AutoShrink(char val)
{
   if ( (autoShrink && val) || (!autoShrink && !val) ) return;

   autoShrink = val;
   if ( autoShrink ) reallocate(_len, True);
}

/*---------------------------------------------------------------------------
 *  Remove characters from the beginning
 */

void
StringC::CutBeg(u_int l)
{
   (*this)(0,l) = "";
}

/*---------------------------------------------------------------------------
 *  Remove characters from the end
 */

void
StringC::CutEnd(u_int l)
{
   if ( !_p ) return;

   if ( l > _len ) l = _len;

   Clear(_len - l);
}

/*---------------------------------------------------------------------------
 *  Do a string comparison at the beginning
 */

int
StringC::StartsWith(const char *cs, u_int clen, u_int off, Boolean checkCase)
const
{
//
// Do quick checks
//
   if ( _p == NULL || cs == NULL ) return 0;

   if ( clen > (_len-off) ) return 0;

//
// Compare at beginning
//
   if ( checkCase )
      return (strncmp(_p+off, cs, clen) == 0);
   else
      return (strncasecmp((char*)(_p+off), (char*)cs, clen) == 0);
}

int
StringC::StartsWith(const CharC& c, u_int offset, Boolean checkCase) const
{
   return StartsWith(c.Addr(), c.Length(), offset, checkCase);
}

int
StringC::StartsWith(char c, u_int offset, Boolean checkCase) const
{
//
// Do quick checks
//
   if ( _p == NULL || offset >= _len ) return 0;

//
// Compare at beginning
//
   if ( checkCase )
      return (*(_p+offset) == c);
   else {
      char	pc = *(_p+offset);
      return (to_lower(pc) == to_lower(c));
   }

} // End StartsWith

/*---------------------------------------------------------------------------
 *  Do a string comparison at the end
 */

int
StringC::EndsWith(const char *cs, u_int clen, u_int off, Boolean checkCase)const
{
//
// Do quick checks
//
   if ( _p == NULL || cs == NULL || clen > off+1 ) return 0;

//
// Compare backwards, starting at offset
//
   int	pos = off+1 - clen;
   if ( checkCase )
      return (strncmp(_p+pos, cs, clen) == 0);
   else
      return (strncasecmp((char*)(_p+pos), (char*)cs, clen) == 0);

} // End EndsWith

int
StringC::EndsWith(const CharC& c, u_int offset, Boolean checkCase) const
{
   return EndsWith(c.Addr(), c.Length(), offset, checkCase);
}

/*---------------------------------------------------------------------------
 *  See if string matches exactly
 */

int
StringC::Equals(const char *cs, u_int clen, u_int offset, Boolean checkCase)
									   const
{
//
// Do quick checks
//
   if ( _p == NULL || cs == NULL || clen != (_len-offset) ) return 0;

//
// Compare
//
   if ( checkCase )
      return (strncmp(_p+offset, cs, clen) == 0);
   else
      return (strncasecmp((char*)(_p+offset), (char*)cs, clen) == 0);

} // End Equals

int
StringC::Equals(const CharC& c, u_int offset, Boolean checkCase) const
{
   return Equals(c.Addr(), c.Length(), offset, checkCase);
}

int
StringC::Equals(char c, u_int offset, Boolean checkCase) const
{
//
// Do quick checks
//
   if ( _p == NULL || offset != (_len-1) ) return 0;

//
// Compare at beginning
//
   if ( checkCase )
      return (*(_p+offset) == c);
   else {
      char	pc = *(_p+offset);
      return (to_lower(pc) == to_lower(c));
   }

} // End Equals

/*---------------------------------------------------------------
 *  Search for a string
 */

const char *
StringC::Search(const char *pat, u_int plen, u_int offset, Boolean checkCase)
									   const
{
   if ( _p == NULL || pat == NULL || _len == 0 )
      return NULL;

   const char	*end = ((_p + _len) - plen) + 1;
   const char	*src = _p + offset;

   if (end < src) return NULL;
   if ( checkCase ) {
      while ((src = (const char *)memchr(src, pat[0], end - src))) {
	 if ( (strncmp(src, pat, plen) == 0) ) return src;
	 src++;
      }
   }
   else {
      while ( src < end ) {
	 if ( strncasecmp((char*)src, (char*)pat, plen) == 0 ) return src;
	 src++;
      }
   }

   return NULL;
}

const char *
StringC::Search(const CharC& c, u_int offset, Boolean checkCase) const
{
   return Search(c.Addr(), c.Length(), offset, checkCase);
}

/*---------------------------------------------------------------
 *  Search for a particular character
 */

const char *
StringC::Search(char c, u_int offset, Boolean checkCase) const
{
   if ( _p == NULL || _len == 0 )
      return NULL;

   u_int	rem = _len - offset;
   const char	*cs = _p   + offset;
   if ( checkCase ) {
      while ( rem > 0 && *cs != c ) cs++, rem--;
   }
   else {
      c = to_lower(c);
      while ( rem > 0 && to_lower(*cs) != c ) cs++, rem--;
   }

   if ( rem == 0 ) return NULL;
   return cs;
}

/*---------------------------------------------------------------
 *  Search in reverse for a string.  Start at offset and look backwards.
 */

const char *
StringC::RevSearch(const char *pat, u_int plen, u_int off, Boolean checkCase)
									   const
{
   if ( _p == NULL || pat == NULL || _len == 0 )
      return NULL;

   const char	*src = _p + off - plen + 1;

   if ( checkCase ) {
      while ( src >= _p ) {
	 if ( strncmp(src, pat, plen) == 0 ) return src;
	 src--;
      }
   }
   else {
      while ( src >= _p ) {
	 if ( strncasecmp((char*)src, (char*)pat, plen) == 0 ) return src;
	 src--;
      }
   }

   return NULL;
}

const char *
StringC::RevSearch(const CharC& c, u_int offset, Boolean checkCase) const
{
   return RevSearch(c.Addr(), c.Length(), offset, checkCase);
}

/*---------------------------------------------------------------
 *  Search in reverse for a particular character.  Start at offset and
 *     look backwards.
 */

const char *
StringC::RevSearch(char c, u_int offset, Boolean checkCase) const
{
   if ( _p == NULL || _len == 0 )
      return NULL;

   const char	*cs = _p + offset;
   if ( checkCase ) {
      while ( cs >= _p && *cs != c ) cs--;
   }
   else {
      c = to_lower(c);
      while ( cs >= _p && to_lower(*cs) != c ) cs--;
   }

   if ( cs < _p ) return NULL;
   return cs;
}

/*---------------------------------------------------------------
 *  Count the number of occurrences of a string
 */

u_int
StringC::NumberOf(const char *pat, u_int plen, u_int off, Boolean checkCase)
									   const
{
   if ( _p == NULL || pat == NULL || _len == 0 )
      return 0;

   u_int	count = 0;
   const char	*end = _p + _len - plen;
   const char	*src = _p + off;

   if ( checkCase ) {
      while ( src <= end ) {
	 if ( strncmp(src, pat, plen) == 0 ) {
	    count++;
	    src += plen;
	 }
	 else
	    src++;
      }
   }
   else {
      while ( src <= end ) {
	 if ( strncasecmp((char*)src, (char*)pat, plen) == 0 ) {
	    count++;
	    src += plen;
	 }
	 else
	    src++;
      }
   }

   return count;

} // End NumberOf

u_int
StringC::NumberOf(const CharC& c, u_int offset, Boolean checkCase) const
{
   return NumberOf(c.Addr(), c.Length(), offset, checkCase);
}

/*---------------------------------------------------------------
 *  Count the number of occurrences of a particular character
 */

u_int
StringC::NumberOf(char c, u_int offset, Boolean checkCase) const
{
   if ( _p == NULL || _len == 0 )
      return 0;

   u_int	count = 0;
   u_int	rem = _len - offset;
   const char	*cs = _p   + offset;
   if ( checkCase ) {
      while ( rem > 0 ) {
	 if ( *cs == c ) count++;
	 cs++;
	 rem--;
      }
   }
   else {
      c = to_lower(c);
      while ( rem > 0 ) {
	 if ( to_lower(*cs) == c ) count++;
	 cs++;
	 rem--;
      }
   }

   return count;

} // End NumberOf

/*---------------------------------------------------------------------------
 *  Return the range of the next word delimited by the given characters
 */

CharC
StringC::NextWord(u_int offset, const char *delim)
{
//
// Skip any non-escaped characters in delim
//
   char	prev = 0;
   char	ch;
   while ( offset < _len && strchr(delim, ch=_p[offset]) != NULL &&
	   prev != '\\' ) {
      prev = ch;
      offset++;
   }

   u_int	newPos = offset;

//
// Skip any characters not in delim
//
   while ( offset < _len &&
           (strchr(delim, ch=_p[offset]) == NULL || prev == '\\') ) {
      prev = ch;
      offset++;
   }

   return Range(newPos, offset-newPos);
}

/*---------------------------------------------------------------------------
 *  Return the range of the next word delimited by the given character
 */

CharC
StringC::NextWord(u_int offset, char delim)
{
//
// Skip non-escaped delim characters
//
   char	prev = 0;
   char	ch;
   while ( offset < _len && ((ch=_p[offset]) == delim) && prev != '\\' ) {
      prev = ch;
      offset++;
   }

   u_int	newPos = offset;

//
// Skip any non-delim characters
//
   while ( offset < _len && ((ch=_p[offset]) != delim || prev == '\\') ) {
      prev = ch;
      offset++;
   }

   return Range(newPos, offset-newPos);
}

/*---------------------------------------------------------------------------
 *  Replace occurences of the first string with the second
 */

void
StringC::Replace(CharC pat, CharC val)
{
   int	pos = PosOf(pat);
   while ( pos >= 0 ) {
      (*this)(pos,pat.Length()) = val;
      pos = PosOf(pat, (u_int)pos + val.Length()); // add val.Length() so we
                                                   // don't search into the
                                                   // string just replaced
   }
}

/*---------------------------------------------------------------------------
 *  Replace occurences of the first character with the second
 */

void
StringC::Replace(char c1, char c2)
{
   char	*cp = _p;
   char	*ep = _p + _len;
   while ( cp < ep ) {
      if ( *cp == c1 ) *cp = c2;
      cp++;
   }
}
