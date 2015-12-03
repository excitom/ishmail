/*
 *  $Id: SortElemC.h,v 1.1.1.1 2000/04/25 13:49:01 fnevgeny Exp $
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
#ifndef _SortElemC_h_
#define _SortElemC_h_

#include "Base.h"
#include <X11/Intrinsic.h>

class SortKeyC;

//----------------------------------------------------------------------

class SortElemC {

   SortKeyC	*key;
   Widget	widget;
   Dimension	ht;

public:

   SortElemC&	operator=(const SortElemC&);

   SortElemC() {}
   SortElemC(SortKeyC*, Widget);
   ~SortElemC();

   inline SortElemC(const SortElemC& e) { *this = e; }

   int		operator==(const SortElemC&) const;
   inline int	operator!=(const SortElemC& e) const { return !(*this==e); }
   inline int	compare(const SortElemC&) const { return 0; }
   inline int	operator<(const SortElemC& e) const { return (compare(e) < 0); }
   inline int	operator>(const SortElemC& e) const { return (compare(e) > 0); }
//
// Casts
//
   inline operator	Widget() const		{ return widget; }
   inline operator	SortKeyC*() const	{ return key; }

   MEMBER_QUERY(SortKeyC&,	SortKey,	*key);
   MEMBER_QUERY(Dimension,	Height,		ht);
};

//
// Method to allow printing of SortElemC
//
inline ostream& operator<<(ostream& strm, const SortElemC&) { return(strm); }

#endif // _SortElemC_h_
