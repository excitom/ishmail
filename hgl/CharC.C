/*
 *  $Id: CharC.C,v 1.7 2000/07/05 16:57:01 kherron Exp $
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

#include <config.h>
#include "CharC.h"
#include "Base.h"
#include "StrCase.h"
#include "RangeC.h"
#include "StringC.h"

#include <unistd.h>

/*-----------------------------------------------------------------------
 * Comparison methods
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

int
CharC::compare(const CharC& c) const
{
   return ::compare(p, len, c.p, c.len);
}

int
CharC::compare(const StringC& s) const
{
   return ::compare(p, len, (const char*)s, s.size());
}

int
CharC::compare(const char *cs) const
{
   return ::compare(p, len, cs, cs?strlen(cs):0);
}

int
compare(const char *cs, const CharC& c)
{
   return ::compare(cs, cs?strlen(cs):0, c.p, c.len);
}

/*---------------------------------------------------------------------------
 *  Do a string comparison at the beginning
 */

int
CharC::StartsWith(const char *cs, u_int clen, u_int off, Boolean checkCase)const
{
//
// Do quick checks
//
   if ( p == NULL || cs == NULL ) return 0;

   if ( clen > (len-off) ) return 0;

//
// Compare at beginning
//
   if ( checkCase )
      return (strncmp(p+off, cs, clen) == 0);
   else
      return (strncasecmp((char*)(p+off), (char*)cs, clen) == 0);

}

int
CharC::StartsWith(const StringC& s, u_int offset, Boolean checkCase) const
{
   return StartsWith(s, s.size(), offset, checkCase);
}

int
CharC::StartsWith(char c, u_int offset, Boolean checkCase) const
{
//
// Do quick checks
//
   if ( p == NULL || offset >= len ) return 0;

//
// Compare at beginning
//
   if ( checkCase )
      return (*(p+offset) == c);
   else {
      char	pc = *(p+offset);
      return (to_lower(pc) == to_lower(c));
   }

} // End StartsWith

/*---------------------------------------------------------------------------
 *  Do a string comparison at the end
 */

int
CharC::EndsWith(const char *cs, u_int clen, u_int off, Boolean checkCase) const
{
//
// Do quick checks
//
   if ( p == NULL || cs == NULL || clen > off+1 ) return 0;

//
// Compare backwards, starting at offset
//
   int	pos = off+1 - clen;
   if ( checkCase )
      return (strncmp(p+pos, cs, clen) == 0);
   else
      return (strncasecmp((char*)(p+pos), (char*)cs, clen) == 0);

} // End EndsWith

int
CharC::EndsWith(const StringC& s, u_int offset, Boolean checkCase) const
{
   return EndsWith(s, s.size(), offset, checkCase);
}

/*---------------------------------------------------------------------------
 *  See if string matches exactly
 */

int
CharC::Equals(const char *cs, u_int clen, u_int offset, Boolean checkCase) const
{
//
// Do quick checks
//
   if ( p == NULL || cs == NULL || clen != (len-offset) ) return 0;

//
// Compare
//
   if ( checkCase )
      return (strncmp(p+offset, cs, clen) == 0);
   else
      return (strncasecmp((char*)(p+offset), (char*)cs, clen) == 0);

} // End Equals

int
CharC::Equals(const StringC& s, u_int offset, Boolean checkCase) const
{
   return Equals(s, s.size(), offset, checkCase);
}

int
CharC::Equals(char c, u_int offset, Boolean checkCase) const
{
//
// Do quick checks
//
   if ( p == NULL || offset != (len-1) ) return 0;

//
// Compare at beginning
//
   if ( checkCase )
      return (*(p+offset) == c);
   else {
      char	pc = *(p+offset);
      return (to_lower(pc) == to_lower(c));
   }

} // End Equals

/*---------------------------------------------------------------
 *  Search for a string
 */

const char *
CharC::Search(const char *pat, u_int plen, u_int offset, Boolean checkCase)const
{
   if ( p == NULL || pat == NULL || len == 0 )
      return NULL;

   const char	*end = ((p + len) - plen) + 1;
   const char	*src = p + offset;

   if (end < src) return NULL;
   if ( checkCase ) {
      while ( (src = (const char *)memchr(src, pat[0], end - src)) ) {
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
CharC::Search(const StringC& s, u_int offset, Boolean checkCase) const
{
   return Search(s, s.size(), offset, checkCase);
}

/*---------------------------------------------------------------
 *  Search for a particular character
 */

const char *
CharC::Search(char c, u_int offset, Boolean checkCase) const
{
   if ( p == NULL || len == 0 || offset > len )
      return NULL;

   u_int	rem = len - offset;
   const char	*cs = p   + offset;
   if ( checkCase ) {
      return (const char *)memchr(cs, c, rem);
   }
   else {
      c = to_lower(c);
      while ( rem > 0 ) {
	 if ( to_lower(*cs) == c )
	    return cs;
	 else {
	    cs++; rem--;
	 }
      }
      return NULL;
   }
}

/*---------------------------------------------------------------
 *  Search in reverse for a string.  Start at offset and look backwards.
 */

const char *
CharC::RevSearch(const char *pat, u_int plen, u_int off, Boolean checkCase)const
{
   if ( p == NULL || pat == NULL || len == 0 )
      return NULL;

   const char	*src = p + off - plen + 1;

   if ( checkCase ) {
      while ( src >= p ) {
	 if ( strncmp(src, pat, plen) == 0 ) return src;
	 src--;
      }
   }
   else {
      while ( src >= p ) {
	 if ( strncasecmp((char*)src, (char*)pat, plen) == 0 ) return src;
	 src--;
      }
   }

   return NULL;
}

const char *
CharC::RevSearch(const StringC& s, u_int offset, Boolean checkCase) const
{
   return RevSearch(s, s.size(), offset, checkCase);
}

/*---------------------------------------------------------------
 *  Search in reverse for a particular character.  Start at offset and
 *     look backwards.
 */

const char *
CharC::RevSearch(char c, u_int offset, Boolean checkCase) const
{
   if ( p == NULL || len == 0 )
      return NULL;

   const char	*cs = p + offset;
   if ( checkCase ) {
      while ( cs >= p && *cs != c ) cs--;
   }
   else {
      c = to_lower(c);
      while ( cs >= p && to_lower(*cs) != c ) cs--;
   }

   if ( cs < p ) return NULL;
   return cs;
}

/*---------------------------------------------------------------
 *  Count the number of occurrences of a string
 */

u_int
CharC::NumberOf(const char *pat, u_int plen, u_int off, Boolean checkCase) const
{
   if ( p == NULL || pat == NULL || len == 0 )
      return 0;

   u_int	count = 0;
   const char	*end = p + len - plen;
   const char	*src = p + off;

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
CharC::NumberOf(const StringC& s, u_int offset, Boolean checkCase) const
{
   return NumberOf(s, s.size(), offset, checkCase);
}

/*---------------------------------------------------------------
 *  Count the number of occurrences of a particular character
 */

u_int
CharC::NumberOf(char c, u_int offset, Boolean checkCase) const
{
   if ( p == NULL || len == 0 )
      return 0;

   u_int	count = 0;
   u_int	rem = len - offset;
   const char	*cs = p   + offset;
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
 *  Method to append the string to an open file
 */

int
CharC::WriteFile(int fd) const
{
   if ( p == NULL || len == 0 ) return 1;
   return (write(fd, p, len) == len);
}

/*---------------------------------------------------------------------------
 *  Method to append the string to an open file
 */

int
CharC::WriteFile(FILE *fp) const
{
   if ( p == NULL || len == 0 ) return 1;

   int	error = (fwrite(p, 1, len, fp) != len);
   if ( !error ) error = (fflush(fp) != 0);
   return !error;
}

/*---------------------------------------------------------------------------
 *  Method to write the string to a file
 */

int
CharC::WriteFile(char *fname) const
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
 *  Remove whitespace from ends
 */

void
CharC::Trim()
{
   if ( !p || len == 0 ) return;

//
// Skip over whitespace at the end
//
   const char	*cp = p + len - 1;
   while ( (cp >= p) && (*cp == ' ' || *cp == '\t' || *cp == '\n') )
      cp--, len--;

//
// Skip over whitespace at the beginning
//
   const char	*ep = p + len;
   while ( (p < ep) && (*p == ' ' || *p == '\t' || *p == '\n') )
      p++, len--;

} // End Trim

/*---------------------------------------------------------------------------
 *  Return substring
 */

const
CharC
CharC::operator()(u_int first, u_int length) const
{
   if ( len == 0 ) return *this;

   const char	*newp;
   u_int	newlen;

   if ( first >= len ) {
      newp   = p + len - 1;
      newlen = 0;
   }

   else if ( first + length > len ) {
      newp   = p + first;
      newlen = len - first;
   }

   else {
      newp   = p + first;
      newlen = length;
   }

   return CharC(newp, newlen);
}

const
CharC
CharC::operator()(const RangeC& r) const
{
   return operator()(r.firstIndex(), r.length());
}

CharC
CharC::operator()(u_int first, u_int length)
{
   if ( len == 0 ) return *this;

   const char	*newp;
   u_int	newlen;

   if ( first >= len ) {
      newp   = p + len - 1;
      newlen = 0;
   }

   else if ( first + length > len ) {
      newp   = p + first;
      newlen = len - first;
   }

   else {
      newp   = p + first;
      newlen = length;
   }

   return CharC(newp, newlen);
}

CharC
CharC::operator()(const RangeC& r)
{
   return operator()(r.firstIndex(), r.length());
}

/*---------------------------------------------------------------------------
 *  Remove characters from the beginning
 */

void
CharC::CutBeg(u_int l)
{
   if ( !p ) return;

   if ( l > len ) l = len;

   p   += l;
   len -= l;
}

/*---------------------------------------------------------------------------
 *  Remove characters from the end
 */

void
CharC::CutEnd(u_int l)
{
   if ( !p ) return;

   if ( l > len ) l = len;

   len -= l;
}

/*---------------------------------------------------------------------------
 *  See if the array is null-terminated
 */

Boolean
CharC::FollowedByNull() const
{
//
// If the array ends on a page boundary, we can't check the next character.
// If the range is memory mapped, there is a chance that accessing the next
//    page will cause a bus error.
//
   static u_long	pagesize = 0;

   if ( pagesize == 0 ) {
#if defined HAVE_SYSCONF && defined _SC_PAGE_SIZE
      pagesize = (u_long)sysconf(_SC_PAGE_SIZE);
#elif defined HAVE_GETPAGESIZE
      pagesize = (u_long)getpagesize();
#elif defined PAGESIZE
      pagesize = (u_long)PAGESIZE;
#else
#error No way to get pagesize on your architecture
#endif
   }

//
// If the next character is at the beginning of a page, we can't take the
//    chance and look at it.
//
   const char	*next = p + len;
   if ( ((u_long)next % pagesize) == 0 ) return False;

//
// Now that we've determined it's ok to look at the next character, do so.
//
   return (*next == 0);

} // End FollowedByNull

/*---------------------------------------------------------------------------
 *  Assignment from StringC
 */

CharC&
CharC::operator=(const StringC& s)
{
   Set(s, s.size());
   return *this;
}

/*---------------------------------------------------------------------------
 *  Assignment from SubStringC
 */

CharC&
CharC::operator=(const SubStringC& ss)
{
   Set(ss, ss.size());
   return *this;
}

/*---------------------------------------------------------------------------
 *  Return the range of the next word delimited by the given characters
 */

CharC
CharC::NextWord(u_int offset, const char *delim)
{
//
// Skip any non-escaped characters in delim
//
   char	prev = 0;
   char	ch;
   while ( offset < len && strchr(delim, ch=p[offset]) != NULL &&
	   prev != '\\' ) {
      prev = ch;
      offset++;
   }

   u_int	newPos = offset;

//
// Skip any characters not in delim
//
   while ( offset < len &&
           (strchr(delim, ch=p[offset]) == NULL || prev == '\\') ) {
      prev = ch;
      offset++;
   }

   return (*this)(newPos, offset-newPos);
}

/*---------------------------------------------------------------------------
 *  Return the range of the next word delimited by the given character
 */

CharC
CharC::NextWord(u_int offset, char delim)
{
//
// Skip non-escaped delim characters
//
   char	prev = 0;
   char	ch;
   while ( offset < len && ((ch=p[offset]) == delim) && prev != '\\' ) {
      prev = ch;
      offset++;
   }

   u_int	newPos = offset;

//
// Skip any non-delim characters
//
   while ( offset < len && ((ch=p[offset]) != delim || prev == '\\') ) {
      prev = ch;
      offset++;
   }

   return (*this)(newPos, offset-newPos);
}
