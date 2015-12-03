/*
 * $Id: IconViewC.h,v 1.1.1.1 2000/04/25 13:49:01 fnevgeny Exp $
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

#ifndef _IconViewC_h_
#define _IconViewC_h_

#include "ViewC.h"
#include "VItemListC.h"
#include "PixmapNameDictC.h"
#include "PixmapDataDictC.h"
#include "IconDataDictC.h"
#include "RectC.h"

#include <Xm/DrawingA.h>
#include <Xm/DragDrop.h>

class VBoxC;
class VItemListC;

/*-----------------------------------------------------------------------
 *  This class is used to display a set of icons with a label
 */

class IconViewC : public ViewC {

public:

   IconViewC(VBoxC *vb);
   ~IconViewC();

   void		AddItem(VItemC&);	// Add a view item to the display
   void		AddItems(const VItemListC&);	// Add view items to the display
   virtual char	*ButtonName() const = 0;// Return widget name for button in
					//  popup menu used to display this view
   void		ChangeItemFields(const VItemC&) {}
   void		ChangeItemLabel(const VItemC&);
   void		ChangeItemPixmaps(const VItemC&);
   void		ChangeItemVis();	// Change item visibility
   void		ChangeItemVis(const VItemC&);	// Change item visibility
   Widget	CreateDragIcon(VItemListC&);
   void		Draw(RectC&);		// Draw the items in this view, but
					//  only update the specified area
   void		DropFinished();
   VItemC	*DropItem();
   void		FlashItem(const VItemC*);
   void		GetSize(int*, int*);
   void		HandleButton1Motion(XMotionEvent*);
   void		HandleButton1Press(XButtonEvent*);
   void		HandleButton1Release(XButtonEvent*);
   void		HandleDoubleClick(XButtonEvent*);
   void		HandleDragOver(XmDragProcCallbackStruct*);
   void		HandleKeyPress(XKeyEvent*);
   void		HandleSingleClick();
   void		Hide();			// Remove this view
   void		HighlightItem(const VItemC*);
   VItemC	*PickItem(int, int);	// Return the item at the given point
   void		PickItems(RectC&, VItemListC&);	// Return the items in the pick
						// rect
   void		PlaceItems();		// Calculate positions
   void		Redraw();		// Redraw the items in this view
   void		RedrawItem(const VItemC&);	// Redraw the specified item
   void		RemoveItem(VItemC&);
   void		RemoveItems(const VItemListC&);
   void		ScrollToItem(const VItemC&);
   void		Show();			// Display this view
   void		printOn(ostream&) const {} // Print information about this view

//
// Compare two views
//
   int  	operator==(const ViewC&) const { return 1; }
   int		compare(const ViewC&) const { return 0; }

protected:

//
// Event handlers
//
   static void  HandleAutoScroll(IconViewC*, XtIntervalId*);
   static void	HandleFocusChange(Widget, IconViewC*, XEvent*, Boolean*);
   static void	FlashProc(IconViewC*, XtIntervalId*);
   static void	HandlePointerMotion(Widget, IconViewC*, XMotionEvent*,Boolean*);


   //..void	UpdateMaxSize();	// Find max item size
   //..void	UpdateMaxSize(const VItemC&);
   //..void	UpdateMaxSize(const IconDataC&);
   void		DrawItem(const VItemC&, VItemDrawModeT mode);
   void		DrawItem(const VItemC&, const IconDataC&);
   void		DrawHighlight(const IconDataC*, Pixel,
			      Drawable drawto=(Drawable)NULL);
   void		DrawHighlight(const VItemC*, Pixel);
   void		ScrollToItem(const IconDataC*);
   Boolean      DataValid(const IconDataC*);
   void		UpdatePickRect(int, int);

//
// These methods must be provided by the subclass
//
   virtual void	GetPixmap(IconDataC&) = 0;// Create pixmap for view item if
					//  necessary
   virtual void	DrawItem(const VItemC&, const IconDataC&,
			 VItemDrawModeT mode,
			 Drawable drawto=(Drawable)NULL) = 0;
   virtual void	UpdateMaxItemWd() = 0;	// Calculate max width and height
   virtual void	UpdateMaxItemHt() = 0;

//
// Private versions of public methods.  These methods don't update the screen
//    after doing thier work.
//
   void		_AddItem(VItemC&);

//
// Max size of pixmap and name
//
   int		maxPmWd;
   int		maxPmHt;
   int		maxLabelWd;
   int		maxLabelHt;
   int		maxItemWd;
   int		maxItemHt;

//
// Drawing area attributes
//
   Pixel	regFgColor, regBgColor;
   Pixel	invFgColor, invBgColor;
   XFontStruct	*font;
   XmFontList	fontList;
   int		xSpacing;		// Spacing between icons
   int		ySpacing;
   int		labelSpacing;		// Spacing between lines in label
   int		labelOffset;		// Spacing between pixmap and name
   Dimension	itemMarginWd;		// Margins on each item
   Dimension	itemMarginHt;
   int		daRows;
   int		daCols;
   int		daWd, daHt;
   Boolean	needPlaces;		// True if positions to be calculated

//
// Used in item selection
//
   VItemC	*hlItem;		// Item currently highlighted
   Pixel	hlColor;		// Highlight color
   int		hlThick;		// Highlight thickness
   GC		pickGC;
   RectC	pickRect;
   VItemListC	pickList;
   int		pickX, pickY;
   int		pickState;
   VItemDrawModeT	pickDrawMode;	// Turn them on or off during pick
   VItemC	*pressItem;		// Item under button 1 press
   int		clickCount;		// Number of click in multi-click
   Time		lastClickTime;		// Time of last click

//
// Used for auto scrolling
//
   XMotionEvent	scrollEvent;            // Motion event that started scrolling
   XtIntervalId	scrollTimer;
   char		*scrollAction;          // Auto scroll action

//
// Used for focus
//
   Boolean		focusHere;	// True if drawing area has focus

//
// Used for drag and drop
//
   VItemC		*dropItem;
   Pixel		dropColor;
   Widget		dragIcon;

//
// Used for flashing
//
   VItemC	*flashItem;		// Which item is flashing
   int		flashCount;		// How many times to flash
   Boolean	flashOn;		// Whether currently highlighted or not
   XtIntervalId	flashTimer;		// Process doing the flashing

//
// This dictionary is used to relate pixmap file names with pixmaps
//
   PixmapNameDictC	pixmapFileDict;
   PixmapDataDictC	pixmapDataDict;

//
// This dictionary is used to relate view item data with the item
//
   IconDataDictC	dataDict;
};

#endif // _IconViewC_h_
