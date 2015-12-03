/*
 *  $Id: CharC.h,v 1.2 2000/06/05 17:00:41 evgeny Exp $
 *  
 *  Copyright (c) 1994 HAL Computer Systems International, Ltd.
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
#ifndef _CharC_h_
#define _CharC_h_

#include <stream.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <X11/Intrinsic.h>

class RangeC;
class StringC;
class SubStringC;

/*
 * Character array class.  This class is designed to aid in the use of
 *    character strings that aren't guaranteed to be null-terminated.  It
 *    contains a character pointer and a length.  The character data cannot
 *    be modified.
 */

class CharC {

private:

   const char	*p;
   u_int	len;

public:

//
// Assignment methods
//
   inline void	 Set(const char *cs, u_int l) { p = cs; len = l; }
   inline CharC& operator=(const CharC& c) { Set(c.p, c.len); return *this; }
   inline CharC& operator=(const char *cs)
				     { Set(cs, cs?strlen(cs):0); return *this; }
   CharC& 	 operator=(const StringC&);
   CharC& 	 operator=(const SubStringC&);
   void		 CutBeg(u_int);	// Remove characters from beginning
   void		 CutEnd(u_int); // Remove characters from end
   inline void	 CutBoth(u_int l) { CutBeg(l); CutEnd(l); }

//
// Constructors
//
   inline CharC(const char *cs, u_int l) { Set(cs, l); }
   inline CharC(const char *cs)          { *this = cs; }
   inline CharC(const CharC& c)		 { *this = c; }
   inline CharC(const StringC& s)	 { *this = s; }
   inline CharC(const SubStringC& ss)	 { *this = ss; }
   inline CharC()			 { p = NULL; len = 0; }

//
// Comparison methods
//
   int		compare(const CharC&) const;
   inline int	operator<  (const CharC& s) const {return (compare(s) <  0); }
   inline int	operator>  (const CharC& s) const {return (compare(s) >  0); }
   inline int	operator<= (const CharC& s) const {return (compare(s) <= 0); }
   inline int	operator>= (const CharC& s) const {return (compare(s) >= 0); }
   inline int	operator== (const CharC& s) const {return (compare(s) == 0); }
   inline int	operator!= (const CharC& s) const {return (compare(s) != 0); }

   int		compare(const StringC&) const;
   inline int	operator<  (const StringC& s) const {return (compare(s) <  0); }
   inline int	operator>  (const StringC& s) const {return (compare(s) >  0); }
   inline int	operator<= (const StringC& s) const {return (compare(s) <= 0); }
   inline int	operator>= (const StringC& s) const {return (compare(s) >= 0); }
   inline int	operator== (const StringC& s) const {return (compare(s) == 0); }
   inline int	operator!= (const StringC& s) const {return (compare(s) != 0); }

   int		compare(const char*) const;
   inline int	operator<  (const char *cs) const {return (compare(cs) <  0); }
   inline int	operator>  (const char *cs) const {return (compare(cs) >  0); }
   inline int	operator<= (const char *cs) const {return (compare(cs) <= 0); }
   inline int	operator>= (const char *cs) const {return (compare(cs) >= 0); }
   inline int	operator== (const char *cs) const {return (compare(cs) == 0); }
   inline int	operator!= (const char *cs) const {return (compare(cs) != 0); }

   friend int	compare(const char*, const CharC&);
   inline friend int	operator< (const char* cs, const CharC& c)
      { return (::compare(cs, c) <  0); }
   inline friend int	operator> (const char* cs, const CharC& c)
      { return (::compare(cs, c) >  0); }
   inline friend int	operator<=(const char* cs, const CharC& c)
      { return (::compare(cs, c) <= 0); }
   inline friend int	operator>=(const char* cs, const CharC& c)
      { return (::compare(cs, c) >= 0); }
   inline friend int	operator==(const char* cs, const CharC& c)
      { return (::compare(cs, c) == 0); }
   inline friend int	operator!=(const char* cs, const CharC& c)
      { return (::compare(cs, c) != 0); }

//
// Query
//
   inline u_int		Len()    const { return len; }
   inline u_int		Length() const { return len; }
   inline u_int		Size()   const { return len; }
   inline const char	*Addr()  const { return p; }
   inline char	operator[](u_int i) const { return ((i >= len) ? 0 : p[i]); }
   inline char	LastChar() const { return ((len > 0) ? p[len-1] : 0); }
   Boolean	FollowedByNull() const;	// Is *(p+len) NULL?

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

   int StartsWith(const CharC& c, u_int offset, Boolean checkCase=True) const
      { return StartsWith(c.p, c.len, offset, checkCase); }
   int StartsWith(const CharC& c, Boolean checkCase=True) const
      { return StartsWith(c, 0, checkCase); }

   int StartsWith(const StringC&, u_int offset, Boolean checkCase=True) const;
   int StartsWith(const StringC& s, Boolean checkCase=True) const
      { return StartsWith(s, 0, checkCase); }

//
// Searching at end
//
   int EndsWith(const char *cs, u_int clen, u_int offset,
   		Boolean checkCase=True) const;

   int EndsWith(char c, u_int offset, Boolean checkCase=True) const
      { return StartsWith(c, offset, checkCase); }
   int EndsWith(char c, Boolean checkCase=True) const
      { return StartsWith(c, len-1, checkCase); }

   int EndsWith(const char *cs, u_int offset, Boolean checkCase=True) const
      { return EndsWith(cs, cs?strlen(cs):0, offset, checkCase); }
   int EndsWith(const char *cs, Boolean checkCase=True) const
      { return EndsWith(cs, len-1, checkCase); }

   int EndsWith(const CharC& c, u_int offset, Boolean checkCase=True) const
      { return EndsWith(c.p, c.len, offset, checkCase); }
   int EndsWith(const CharC& c, Boolean checkCase=True) const
      { return EndsWith(c, len-1, checkCase); }

   int EndsWith(const StringC&, u_int offset, Boolean checkCase=True) const;
   int EndsWith(const StringC& s, Boolean checkCase=True) const
      { return EndsWith(s, len-1, checkCase); }

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

   int Equals(const CharC& c, u_int offset, Boolean checkCase=True) const
      { return Equals(c.p, c.len, offset, checkCase); }
   int Equals(const CharC& c, Boolean checkCase=True) const
      { return Equals(c, 0, checkCase); }

   int Equals(const StringC&, u_int offset, Boolean checkCase=True) const;
   int Equals(const StringC& s, Boolean checkCase=True) const
      { return Equals(s, 0, checkCase); }

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

   const char *Search(const CharC& c, u_int offset, Boolean checkCase=True)const
      { return Search(c.p, c.len, offset, checkCase); }
   const char *Search(const CharC& c, Boolean checkCase=True) const
      { return Search(c, 0, checkCase); }

   const char *Search(const StringC&, u_int offset, Boolean chkCase=True) const;
   const char *Search(const StringC& s, Boolean checkCase=True) const
      { return Search(s, 0, checkCase); }

//
// Searching backwards
//
   const char *RevSearch(const char *cs, u_int clen, u_int offset,
	      Boolean checkCase=True) const;

   const char *RevSearch(char, u_int offset, Boolean checkCase=True) const;
   const char *RevSearch(char c, Boolean checkCase=True) const
      { return RevSearch(c,  len-1, checkCase); }

   const char *RevSearch(const char *cs, u_int off, Boolean checkCase=True)const
      { return RevSearch(cs, cs?strlen(cs):0, off, checkCase); }
   const char *RevSearch(const char *cs, Boolean checkCase=True) const
      { return RevSearch(cs, len-1, checkCase); }

   const char *RevSearch(const CharC& c, u_int off, Boolean checkCase=True)const
      { return RevSearch(c.p, c.len, off, checkCase); }
   const char *RevSearch(const CharC& c, Boolean checkCase=True) const
      { return RevSearch(c, len-1, checkCase); }

   const char *RevSearch(const StringC&,u_int off,Boolean checkCase=True) const;
   const char *RevSearch(const StringC& s, Boolean checkCase=True) const
      { return RevSearch(s, len-1, checkCase); }

//
// Searching forwards
//
   inline int	PosOf(      char   c, Boolean checkCase=True) const {
      const char *cs = Search(c, checkCase);
      return (cs ? cs - p : -1);
   }
   inline int	PosOf(const char  *c, Boolean checkCase=True) const {
      const char *cs = Search(c, checkCase);
      return (cs ? cs - p : -1);
   }
   inline int	PosOf(const CharC& c, Boolean checkCase=True) const {
      const char *cs = Search(c, checkCase);
      return (cs ? cs - p : -1);
   }
   inline int	PosOf(const StringC& s, Boolean checkCase=True) const {
      const char *cs = Search(s, checkCase);
      return (cs ? cs - p : -1);
   }

   inline int	PosOf(      char   c, u_int i, Boolean checkCase=True) const {
      const char *cs = Search(c, i, checkCase);
      return (cs ? cs - p : -1);
   }
   inline int	PosOf(const char  *c, u_int i, Boolean checkCase=True) const {
      const char *cs = Search(c, i, checkCase);
      return (cs ? cs - p : -1);
   }
   inline int	PosOf(const CharC& c, u_int i, Boolean checkCase=True) const {
      const char *cs = Search(c, i, checkCase);
      return (cs ? cs - p : -1);
   }
   inline int	PosOf(const StringC& s, u_int i, Boolean checkCase=True) const {
      const char *cs = Search(s, i, checkCase);
      return (cs ? cs - p : -1);
   }

//
// Searching backwards
//
   inline int RevPosOf(      char   c, Boolean checkCase=True) const {
      const char *cs = RevSearch(c, checkCase);
      return (cs ? cs - p : -1);
   }
   inline int RevPosOf(const char  *c, Boolean checkCase=True) const {
      const char *cs = RevSearch(c, checkCase);
      return (cs ? cs - p : -1);
   }
   inline int RevPosOf(const CharC& c, Boolean checkCase=True) const {
      const char *cs = RevSearch(c, checkCase);
      return (cs ? cs - p : -1);
   }
   inline int RevPosOf(const StringC& s, Boolean checkCase=True) const {
      const char *cs = RevSearch(s, checkCase);
      return (cs ? cs - p : -1);
   }

   inline int RevPosOf(      char   c, u_int i, Boolean chkCase=True) const {
      const char *cs = RevSearch(c, i, chkCase);
      return (cs ? cs - p : -1);
   }
   inline int RevPosOf(const char  *c, u_int i, Boolean chkCase=True) const {
      const char *cs = RevSearch(c, i, chkCase);
      return (cs ? cs - p : -1);
   }
   inline int RevPosOf(const CharC& c, u_int i, Boolean chkCase=True) const {
      const char *cs = RevSearch(c, i, chkCase);
      return (cs ? cs - p : -1);
   }
   inline int RevPosOf(const StringC& s, u_int i, Boolean chkCase=True) const {
      const char *cs = RevSearch(s, i, chkCase);
      return (cs ? cs - p : -1);
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

   u_int NumberOf(const CharC& c, u_int offset, Boolean checkCase=True) const
      { return NumberOf(c.p, c.len, offset, checkCase); }
   u_int NumberOf(const CharC& c, Boolean checkCase=True) const
      { return NumberOf(c, 0, checkCase); }

   u_int NumberOf(const StringC&, u_int offset, Boolean checkCase=True) const;
   u_int NumberOf(const StringC& s, Boolean checkCase=True) const
      { return NumberOf(s, 0, checkCase); }

//
// I/O
//
   int		WriteFile(char*) const;
   int		WriteFile(FILE*) const;
   int		WriteFile(int) const;
   inline ostream&	printOn(ostream& strm=cout) const {
      if ( p ) for (u_int i=0; i<len; i++) strm << p[i];
      return strm;
   }

//
// Misc
//
   void		Trim();	// Adjust pointer and length to ignore whitespace
   			//    at either end
   CharC	NextWord(u_int offset, const char *white=" \t\n");
			// Return the range of the next word that is delimited
			//    by any chars in "white"
   CharC	NextWord(u_int offset, char c);
			// Return the range of the next word that is delimited
			//    by the character c

   CharC	operator()(u_int, u_int);
   const CharC	operator()(u_int, u_int) const;

   CharC	operator()(const RangeC&);
   const CharC	operator()(const RangeC&) const;
};

inline ostream&
operator<<(ostream& strm, const CharC& c) { return c.printOn(strm); }

#define CHECK_CASE	(Boolean)True
#define IGNORE_CASE	(Boolean)False

#endif // _CharC_h_
