/*
 * $Id: ViewC.h,v 1.2 2000/04/27 13:04:05 fnevgeny Exp $
 *
 * Copyright (c) 1992 HaL Computer Systems, Inc.  All rights reserved.
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

#ifndef _ViewC_h_
#define _ViewC_h_

#include "HalAppC.h"

#include <Xm/DragDrop.h>

class VBoxC;
class VItemC;
class VItemListC;
class RectC;

/*-----------------------------------------------------------------------
 *  This class is used as the abstract base class for a view
 */

class ViewC {

public:

   ViewC(VBoxC*);
   virtual	~ViewC() {}

   virtual void	AddItem(VItemC&) = 0;// Add a view item to the display
   virtual void	AddItems(const VItemListC&) = 0;// Add view items to the display
   virtual char	*ButtonName() const = 0;	// Return widget name for button
						// in popup menu used to display
						// this view
   virtual void	ChangeItemFields(const VItemC&) = 0;
   virtual void	ChangeItemLabel(const VItemC&) = 0;
   virtual void	ChangeItemPixmaps(const VItemC&) = 0;
   virtual void	ChangeItemVis() = 0;
   virtual void	ChangeItemVis(const VItemC&) = 0;
   virtual Widget CreateDragIcon(VItemListC&) = 0;
   virtual void	DropFinished() = 0;	// Signal completion of drop
   virtual VItemC *DropItem() = 0;	// Return current drop site
   virtual void	Draw(RectC&) = 0;	// Draw the items within the rect

   virtual void FlashItem(const VItemC*) = 0;
   virtual void	GetSize(int*, int*) = 0;
   virtual void	HandleButton1Motion(XMotionEvent*) = 0;
   virtual void	HandleButton1Press(XButtonEvent*) = 0;
   virtual void	HandleButton1Release(XButtonEvent*) = 0;
   virtual void	HandleDoubleClick(XButtonEvent*) = 0;
   virtual void	HandleDragOver(XmDragProcCallbackStruct *) = 0;
   virtual void	HandleKeyPress(XKeyEvent*) = 0;
   virtual void	HandleSingleClick() = 0;
   virtual void	Hide() = 0;		// Remove this view
   virtual void	HighlightItem(const VItemC*) = 0;
   virtual VItemC *PickItem(int, int) = 0;// Return the item at the given point
   virtual void	PickItems(RectC&, VItemListC&) = 0; // Return the items in the
						    // pick rect
   virtual void	PlaceItems() = 0;	// Calculate positions
   virtual void	Redraw() = 0;		// Redraw the items in this view
   virtual void	RedrawItem(const VItemC&) = 0;	// Redraw the specified item
   virtual void	RemoveItem(VItemC&) = 0;
   virtual void	RemoveItems(const VItemListC&) = 0;
   virtual void	ScrollToItem(const VItemC&) = 0;
   virtual void	Show() = 0;		// Display this view
   virtual void	printOn(ostream& strm=cout) const = 0;
					// Print information about this view

//
// Compare two views
//
   virtual int	operator==(const ViewC&) const = 0;
   inline int	operator!=(const ViewC& i) const { return !(*this==i); }

   virtual int	compare(const ViewC&) const = 0;
   inline int	operator<(const ViewC& i) const { return (compare(i) < 0); }
   inline int	operator>(const ViewC& i) const { return (compare(i) > 0); }

protected:

   VBoxC	*viewBox;
   Boolean	shown;

   friend	VBoxC;
};

//
// Method to allow printing of ViewC
//

inline ostream&
operator<<(ostream& strm, const ViewC& v)
{
   v.printOn(strm);
   return(strm);
}

#endif // _ViewC_h_
