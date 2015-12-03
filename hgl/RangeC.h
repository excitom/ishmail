/*
 * $Id: RangeC.h,v 1.1.1.1 2000/04/25 13:49:01 fnevgeny Exp $
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

#ifndef	_RangeC_h_
#define	_RangeC_h_

/*
 * RangeC.h -- header file for class RangeC
 */

#include "Base.h"

class RangeC {

   unsigned	_first;
   unsigned	_len;

public:

   inline RangeC()			  : _first(0), _len(0) {}
   inline RangeC(unsigned f, unsigned l) : _first(f), _len(l) {}

   inline unsigned	length() const 		{ return (_len); }
   inline void		length(unsigned l)	{ _len = l; }
   inline unsigned	firstIndex() const 	{ return (_first); }
   inline void		firstIndex(unsigned f)	{ _first = f; }
   inline unsigned	lastIndex() const	{ return (_first + _len - 1); }
   inline void		lastIndex(unsigned i)	{
      if ( _first > i ) _first = i;
      _len = i - _first + 1;
   }
   inline void		range(unsigned f, unsigned l) {
      _first = f;
      _len   = l;
   }

// Assignment

   inline void	operator=(const RangeC& r) {
      _first = r._first;
      _len = r._len;
   }

// Comparison

   inline int	operator==(const RangeC& r) const {
      return ((_first == r._first) && (_len == r._len));
   }
   inline int	operator!=(const RangeC& r) const { return !(*this==r); }
   inline int   operator< (const RangeC& r) const { return (_first<r._first); }
   inline int   operator> (const RangeC& r) const { return (_first>r._first); }

// Printing

   inline ostream&	printOn(ostream& strm=cout) const {
       strm << _first << ':' << _len;
       return strm;
   }
};

// PRINTING

inline ostream&
operator<<(ostream& strm, const RangeC& r) { return r.printOn(strm); }

#endif // _RangeC_h_
