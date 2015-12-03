/*
 *  $Id: RowOrColC.h,v 1.1.1.1 2000/04/25 13:49:01 fnevgeny Exp $
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
#ifndef _RowOrColC_h_
#define _RowOrColC_h_

#include "RowCol.h"
#include "RowColChildListC.h"

class RowColC;

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// This is the base class for RowC and ColC.  It contains variables and
//    methods that are shared by both
//

class RowOrColC {

public:

   RowColC		*par;		// Parent RowColC
   RcAlignT		alignment;
   RcAdjustT		adjust;
   Boolean		resizeOk;	// Can this row or column be resized?
   Boolean		visible;	// Is this row or column visible?
   RowColChildListC	childList;	// List of widgets in this row or col
   int			prefSize;	// Preferred height for rows and width
   					// for columns
   int			realSize;	// Established height for rows and
   					// width for columns
   int			pos;		// Offset to row or column

//
// Methods
//
   RowOrColC(RowColC*);

   void		AddChild(RowColChildC*);
   void		Reset();
   void		SetAdjust(RcAdjustT);
   void		SetAlignment(RcAlignT);
   void		SetPosition(int);
   void		SetResize(Boolean);
   void		SetSize(int);
   void		SetVisible(Boolean);
   void		Refresh();

//
// These are provided by RowC or ColC
//
   virtual void	CalcPrefSize() {};
   virtual void	PlaceChildren() {};
   virtual void	SizeChildren() {};

//
// Operators
//
   int	compare(const RowOrColC&) const;
   inline int operator==(const RowOrColC& p) const { return (compare(p) == 0); }
   inline int operator!=(const RowOrColC& p) const { return (compare(p) != 0); }
   inline int operator< (const RowOrColC& p) const { return (compare(p) <  0); }
   inline int operator> (const RowOrColC& p) const { return (compare(p) >  0); }
   inline int operator<=(const RowOrColC& p) const { return (compare(p) <= 0); }
   inline int operator>=(const RowOrColC& p) const { return (compare(p) >= 0); }
};

inline ostream& operator<<(ostream& strm, const RowOrColC&) { return strm; }

#endif // _RowOrColC_h_
