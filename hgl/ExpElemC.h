/*
 *  $Id: ExpElemC.h,v 1.1.1.1 2000/04/25 13:49:01 fnevgeny Exp $
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
#ifndef _ExpElemC_h_
#define _ExpElemC_h_

#include "Base.h"

#include <X11/Intrinsic.h>

class TermExpC;

//----------------------------------------------------------------------

class ExpElemC {

public:

   enum ExpElemT {
      LPAR,
      RPAR,
      AND,
      OR,
      NOT,
      TERMINAL
   };

private:

   ExpElemT	type;
   TermExpC	*term;
   Widget	widget;
   Dimension	wd;
   Dimension	ht;

public:

   ExpElemC&	operator=(const ExpElemC&);

   ExpElemC() {}
   ExpElemC(ExpElemT, Widget);
   ExpElemC(TermExpC*, Widget);
   ~ExpElemC();

   inline ExpElemC(const ExpElemC& e) { *this = e; }

   int		operator==(const ExpElemC&) const;
   inline int	operator!=(const ExpElemC& e) const { return !(*this==e); }
   inline int	compare(const ExpElemC& e) const {
      int	val;
      if      ( type < e.type ) val = -1;
      else if ( type > e.type ) val =  1;
      else			val =  0;
      return val;
   }
   inline int	operator<(const ExpElemC& e) const { return (compare(e) < 0); }
   inline int	operator>(const ExpElemC& e) const { return (compare(e) > 0); }

   void		GetSize();

//
// Casts
//
   inline operator	Widget() const		{ return widget; }
   inline operator	ExpElemT() const	{ return type; }
   inline operator	TermExpC*() const	{ return term; }

   MEMBER_QUERY(ExpElemT,	Type,		type);
   MEMBER_QUERY(TermExpC&,	TermExp,	*term);
   MEMBER_QUERY(Dimension,	Width,		wd);
   MEMBER_QUERY(Dimension,	Height,		ht);
};

//
// Method to allow printing of ExpElemC
//
inline ostream& operator<<(ostream& strm, const ExpElemC&) { return(strm); }

#endif // _ExpElemC_h_
