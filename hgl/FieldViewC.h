/*
 * $Id: FieldViewC.h,v 1.1.1.1 2000/04/25 13:49:01 fnevgeny Exp $
 *
 * Copyright (c) 1992 HAL Computer Systems International, Ltd.
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

#ifndef _FieldViewC_h_
#define _FieldViewC_h_

#include "ViewC.h"
#include "VItemListC.h"
#include "IntListC.h"
#include "RectC.h"
#include "ColumnListC.h"
#include "PixmapNameDictC.h"
#include "PixmapDataDictC.h"
#include "ItemDataDictC.h"

#include <Xm/DrawingA.h>
#include <Xm/ScrollBar.h>
#include <Xm/DragDrop.h>

class VBoxC;
class StringC;
class StringListC;

/*-----------------------------------------------------------------------
 *  This class is used to display a set of icons with a label
 */

class FieldViewC : public ViewC {

public:

   FieldViewC(VBoxC *vb);
   ~FieldViewC();

//
// Methods required by ViewC
//
   void		AddItem(VItemC&);	// Add a view item to the display
   void		AddItems(const VItemListC&);	// Add view items to the display
   char		*ButtonName() const;	// Return widget name for button in
					//  popup menu used to display this view
   void		ChangeItemFields(const VItemC&);
   void		ChangeItemLabel(const VItemC&) {}
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
   void         HandleButton1Motion(XMotionEvent*);
   void		HandleButton1Press(XButtonEvent*);
   void		HandleButton1Release(XButtonEvent*);
   void         HandleDoubleClick(XButtonEvent*);
   void		HandleDragOver(XmDragProcCallbackStruct*);
   void		HandleKeyPress(XKeyEvent*);
   void		HandleSingleClick();
   void		Hide();			// Remove this view
   void		HighlightItem(const VItemC*);
   VItemC	*PickItem(int, int);	// Return the item at the given point
   void		PickItems(RectC&, VItemListC&);	// Return the items in the pick
						// rect
   void		PlaceItems();
   void		Redraw();		// Redraw the items in this view
   void		RedrawItem(const VItemC&);	// Redraw the specified item
   void		RemoveItem(VItemC&);
   void		RemoveItems(const VItemListC&);
   void		Resize();		// Update the size of the visible area
   void		ScrollToItem(const VItemC&);
   void		Show();			// Display this view
   void		TitleButton1Press(XButtonEvent*);
   void		TitleButton1Release(XButtonEvent*);
   void		printOn(ostream&) const {} // Print information about this view

//
// Compare two views
//
   int  	operator==(const ViewC&) const { return 1; }
   int		compare(const ViewC&) const { return 0; }

//
// Methods unique to FieldViewC
//
   int	 ColumnMaxWidth(int);	 // Chars
   int	 ColumnMinWidth(int);	 // Chars
   int	 ColumnMaxPixels(int);	 // Pixels
   int	 ColumnMinPixels(int);	 // Pixels
   int	 ColumnDisplayIndex(int); // Visual order
   int	 ColumnIndex(int display_id); // Visual to logical order
   void	 HideColumn(int);
   void	 HidePixmaps();
   void	 JustifyColumn(int, ColumnJustifyT);
   void	 SetTitles(const StringListC&);
   void	 SetItemFields(VItemC&, const StringListC&);
   void	 SetItemField(VItemC&, int, const StringC&);
   void	 SetColumnDisplayIndex(int, int);
   void	 SetColumnMaxWidth(int, int);	// Chars
   void	 SetColumnMinWidth(int, int);	// Chars
   void	 SetColumnMaxPixels(int, int);	// Pixels
   void	 SetColumnMinPixels(int, int);	// Pixels
   void	 SetColumnTitle(int, const char*);
   void	 SetColumnSortOrder(IntListC&);
   void	 SetVisibleItemCount(unsigned);   // uses first item as base.
   void	 ShowColumn(int);
   void	 ShowPixmaps();
   int	 VisibleItemCount();
   void  EnableDragTitles(Boolean v = True)  {dragTitlesEnabled = v;}
   void  DisableDragTitles()       {dragTitlesEnabled = False;}
   void  EnablePickSortColumn(Boolean v = True);
   void  DisablePickSortColumn()   { EnablePickSortColumn(False);}

      PTR_QUERY(IntListC&,	ColumnDisplayOrder,	columnDisplayOrder);
      PTR_QUERY(IntListC&,	ColumnSortOrder,	columnSortOrder);
   MEMBER_QUERY(Boolean,       	DragTitlesEnabled, 	dragTitlesEnabled);
   MEMBER_QUERY(Boolean,       	PickSortColumnEnabled, 	pickSortColumnEnabled);

protected:

//
// Called when drawing area recieves an expose event
//
   static void	FlashProc(FieldViewC*, XtIntervalId*);
   static void	HandleAutoScroll(FieldViewC*, XtIntervalId*);
   static void	HandleFocusChange(Widget, FieldViewC*, XEvent*, Boolean*);
   static void	HandlePointerMotion(Widget, FieldViewC*,XMotionEvent*,Boolean*);
   static void	TitleButton1Motion(Widget, FieldViewC*, XMotionEvent*,Boolean*);
   static void	TitleExpose(Widget, FieldViewC*, XmDrawingAreaCallbackStruct*);
   static void	TitleInput(Widget, FieldViewC*, XmDrawingAreaCallbackStruct*);

   static int   ColumnSortCompare(const void*, const void*);

   void		GetPixmap(ItemDataC&);	// Create pixmap for view item if
					//  necessary
   void		GetTitleSize(FieldC&);
   void		GetFieldSizes(ItemDataC*);
   void		ChangeItemFields(ItemDataC*);
   void		DrawTitles();		// Draw the column titles
   void		DrawItem(const VItemC&, VItemDrawModeT);
   void		DrawItem(const ItemDataC&, VItemDrawModeT,
   			 Drawable drawto=(Drawable)NULL);
   void		DrawItem(const ItemDataC&);
   void		DrawItemSeparators(const ItemDataC&, VItemDrawModeT, Drawable);
   void		DrawHighlight(const ItemDataC*, Pixel, Drawable);
   void		DrawHighlight(const VItemC*, Pixel);
   void		RedrawItem(ItemDataC&);		// Redraw the specified item
   void		ScrollToItem(const ItemDataC*);
   void		InitializeTitle();
   void		UpdatePickList(int, int);
   void         _EnablePickSortColumn(Boolean);

   void		DrawLabel(const ItemDataC&);
   Boolean	DataValid(const ItemDataC*);
   int		GetCharWidth();

//
// Private versions of public methods.  These methods don't update the screen
//    after doing their work.
//
   void		_AddItem(VItemC&);

//
// Drawing area specifics
//
   Widget	titleDA;
   Window	titleWin;
   GC		titleGC;			// Drawing context for titles
   Position	titleX;
   Position	titleY;

//
// Drawing area attributes
//
   Pixel	regFgColor, regBgColor;
   Pixel	invFgColor, invBgColor;
   Pixel	titleFgColor, titleBgColor;
   XFontStruct	*font;
   XmFontList	fontList;
   int		ySpacing;		// Spacing between icons
   int		sepThick;		// Thickness of separator lines
   Dimension	itemMarginWd;		// Margins on each item
   Dimension	itemMarginHt;
   int		daWd;
   int		daHt;
   Boolean	focusHere;		// True if drawing area has focus
   Boolean	needPlaces;		// True if we need to calculate size
   unsigned     visItemCount;           // Number of items visible in viewDA
   Boolean	needSetVisItemCount;    // True if we need to reset size

//
// Used in item selection
//
   VItemC	*hlItem;		// Item currently highlighted
   Pixel	hlColor;		// Highlight color
   int		hlThick;		// Highlight thickness
   VItemListC	pickList;		// Current selections
   RectC	pickRect;		// Range of highlight
   int		pickX, pickY;		// Location of first press
   int		pickState;		// State of Shift and Control buttons
   VItemDrawModeT	pickDrawMode;	// Turn them on or off during pick
   VItemC	*pressItem;		// Item under button 1 press
   int		clickCount;		// Number of click in multi-click
   Time		lastClickTime;		// Time of last click

//
// Used in resizing columns
//
   int		titlePickOrigX;
   int		titlePickX;
   int		titlePickY;
   int		titlePickXoffset;
   int		titlePickYoffset;
   int		titlePickXmin;
   int		titlePickYmin;
   int		titlePickYmax;
   ColumnC	*titlePickCol;
   GC		titlePickGC;
   Window	titlePickWin;
   Cursor	resizeCursor;

//
// Used for drag changing column orders.
//
   int          moveIndex;
   FieldC*	moveField;
   Boolean      dragTitlesEnabled;
   Boolean      pickSortColumnEnabled;
   CompareFn    prevCompareFunction;

//
// Used for auto scrolling
//
   XMotionEvent	scrollEvent;            // Motion event that started scrolling
   XtIntervalId	scrollTimer;
   char		*scrollAction;          // Auto scroll action
   int		autoScrollInterval;

//
// Used for drag and drop
//
   VItemC	*dropItem;
   Pixel	dropColor;
   Widget	dragIcon;

//
// Used for flashing
//
   VItemC	*flashItem;		// Which item is flashing
   int		flashCount;		// How many times to flash
   Boolean	flashOn;		// Whether currently highlighted or not
   XtIntervalId	flashTimer;		// Process doing the flashing

//
// Information about the columns
//
   int      		pickSortDistance;
   int      		pickSortThreshold;
   IntListC		columnSortOrder;
   IntListC		columnDisplayOrder;
   ColumnListC		columnList;
   int			maxTitleHt;
   int			maxPmWd;	// Widest pixmap
   Widget		titleSizeLabel;

//
// This dictionary is used to relate pixmap file names with pixmaps
//
   Boolean		showPixmaps;
   PixmapNameDictC	pixmapFileDict;
   PixmapDataDictC	pixmapDataDict;

//
// This dictionary is used to relate view item data with the item
//
   ItemDataDictC	dataDict;
};

#endif // _FieldViewC_h_
