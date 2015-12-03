/*
 *  $Id: RowColChildC.h,v 1.1.1.1 2000/04/25 13:49:01 fnevgeny Exp $
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
#ifndef _RowColChildC_h_
#define _RowColChildC_h_

#include <X11/Intrinsic.h>
#include <stream.h>

class RowC;
class ColC;

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// This class store useful information about each child widget of the RowColC
//

class RowColChildC {

public:

   Widget	w;
   Dimension	prefWd;		// Preferred width
   Dimension	prefHt;		// Preferred height
   Position	curX;		// Current x position
   Position	curY;		// Current y position
   Dimension	curWd;		// Current width
   Dimension	curHt;		// Current height
   RowC		*row;		// Row this one belongs to
   ColC		*col;		// Column this one belongs to

//
// Methods
//
   RowColChildC(Widget);

   void		GetPrefSize();
   int		PrefWidth();
   int		PrefHeight();

//
// Operators
//
   int	compare(const RowColChildC&) const;
   inline int operator==(const RowColChildC& c) const {return (compare(c)==0); }
   inline int operator!=(const RowColChildC& c) const {return (compare(c)!=0); }
   inline int operator< (const RowColChildC& c) const {return (compare(c)< 0); }
   inline int operator> (const RowColChildC& c) const {return (compare(c)> 0); }
   inline int operator<=(const RowColChildC& c) const {return (compare(c)<=0); }
   inline int operator>=(const RowColChildC& c) const {return (compare(c)>=0); }
};

inline ostream& operator<<(ostream& strm, const RowColChildC&) { return strm; }

#endif // _RowColChildC_h_
