/*
 * $Id: FieldViewC.C,v 1.9 2001/03/07 10:39:18 evgeny Exp $
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

#include <config.h>

#include "HalAppC.h"
#include "FieldViewC.h"
#include "VItemC.h"
#include "VBoxC.h"
#include "rsrc.h"
#include "WArgList.h"
#include "WXmString.h"

#include <Xm/Label.h>

#include <X11/keysym.h>
#include <X11/IntrinsicP.h>
#include <X11/CoreP.h>

#include <values.h>

/*-----------------------------------------------------------------------
 *  Method to check data pointer
 */

inline Boolean
FieldViewC::DataValid(const ItemDataC *data)
{
   return (data && (data->view == this));
}

/*-----------------------------------------------------------------------
 *  Constructor for FieldViewC
 */

FieldViewC::FieldViewC(VBoxC *vb) : ViewC(vb)
{
   WArgList	args;

   titleWin            = (Window)NULL;
   titleGC             = NULL;
   titlePickGC         = NULL;
   resizeCursor        = (Cursor)NULL;
   maxTitleHt          = 0;
   maxPmWd             = 0;
   focusHere           = False;
   hlItem              = NULL;
   flashItem           = NULL;
   flashTimer          = (XtIntervalId)NULL;
   dropItem            = NULL;
   dragIcon            = NULL;
   scrollTimer         = (XtIntervalId)NULL;
   pressItem           = NULL;
   clickCount          = 1;
   lastClickTime       = 0;
   needPlaces          = True;
   needSetVisItemCount = False;
   prevCompareFunction = NULL;

   columnList.AllowDuplicates(TRUE);
   columnDisplayOrder.AllowDuplicates(TRUE);
   columnDisplayOrder.SetSorted(FALSE);
   columnSortOrder.AllowDuplicates(FALSE);
   columnSortOrder.SetSorted(FALSE);

//
// Create a title area in the main view box
//
   Dimension	st, bw, mw;
   XtVaGetValues(vb->ViewFrame(), XmNshadowThickness, &st, XmNborderWidth,
		 &bw, XmNmarginWidth, &mw, NULL);
   Position	off = st + bw + mw;

   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM, off);
   args.RightAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_NONE);
   args.MarginWidth(0);
   args.MarginHeight(0);
   titleDA = XmCreateDrawingArea(vb->ViewForm(), "titleDA", ARGS);

   args.Reset();
   args.MappedWhenManaged(False);
   args.RecomputeSize(False);
   titleSizeLabel = XmCreateLabel(titleDA, "titleSizeLabel", ARGS);
   XtManageChild(titleSizeLabel);

   XtAddCallback(titleDA, XmNexposeCallback, (XtCallbackProc)TitleExpose,
		 (XtPointer)this);
   XtAddCallback(titleDA, XmNinputCallback,  (XtCallbackProc)TitleInput,
		 (XtPointer)this);

//
// Get resources
//
   Widget      da = viewBox->ViewDA();
   char	      *cl = "FieldViewC";
   regFgColor   = get_color(cl, da, "foreground",	  XtDefaultForeground);
   regBgColor   = get_color(cl, da, "background",	  XtDefaultBackground);
   hlColor      = get_color(cl, da, "highlightColor",	  regFgColor);
   invFgColor   = get_color(cl, da, "selectedForeground", regBgColor);
   invBgColor   = get_color(cl, da, "selectedBackground", regFgColor);
   titleFgColor = get_color(cl, da, "titleForeground",	  regFgColor);
   titleBgColor = get_color(cl, da, "titleBackground",	  regBgColor);
   dropColor	= get_color(cl, da, "validDropColor",	  hlColor);

   hlThick            = get_int(cl, da, XmNhighlightThickness);
   sepThick           = get_int(cl, da, "separatorThickness",	1);
   ySpacing           = get_int(cl, da, "vertSpacing");
   itemMarginWd       = get_int(cl, da, "itemMarginWidth");
   itemMarginHt       = get_int(cl, da, "itemMarginHeight");
   visItemCount       = get_int(cl, da, "visibleItemCount",	0);
   autoScrollInterval = get_int(cl, da, "autoScrollInterval",	100);
   pickSortThreshold  = get_int(cl, da, "pickSortThreshold",	7);

   showPixmaps           = get_boolean(cl, da, "showPixmaps",		True);
   dragTitlesEnabled     = get_boolean(cl, da, "dragTitlesEnabled",	True);
   pickSortColumnEnabled = get_boolean(cl, da, "pickSortColumnEnabled", False);
#if 0
   pickSortColumnEnabled = get_boolean(cl, da, "pickSortColumnEnabled", 
						viewBox->compFunc == NULL);
#endif

   if ( visItemCount > 0 ) needSetVisItemCount = True;

//
// Load the XmFontList
//
   fontList = viewBox->FontList();

//
// Load Font
//
   font = viewBox->Font();

//
// Get keyboard focus policy
//
   Widget	shell = XtParent(viewBox->ViewDA());
   while ( !XtIsShell(shell) ) shell = XtParent(shell);

   unsigned char	focusPolicy;
   XtVaGetValues(shell, XmNkeyboardFocusPolicy, &focusPolicy, NULL);

   if ( focusPolicy == XmEXPLICIT ) {

//
// Add event handler for keyboard focus change
//
      XtAddEventHandler(viewBox->ViewDA(), FocusChangeMask, False,
			(XtEventHandler)HandleFocusChange, (XtPointer)this);

   } else { // XmPOINTER

      XtAddEventHandler(viewBox->ViewDA(), EnterWindowMask|LeaveWindowMask,
			False, (XtEventHandler)HandleFocusChange,
			(XtPointer)this);
      XtAddEventHandler(viewBox->ViewDA(),
      			PointerMotionMask|PointerMotionHintMask, False,
			(XtEventHandler)HandlePointerMotion, (XtPointer)this);
   }

   if ( XtIsManaged(viewBox->ViewDA()) && needSetVisItemCount ) {
      SetVisibleItemCount(visItemCount);
   }

} // End Constructor

/*-----------------------------------------------------------------------
 *  Destructor for FieldViewC
 */

FieldViewC::~FieldViewC()
{
//
// Delete data
//
   unsigned	count = dataDict.size();
   int	i;
   for (i=0; i<count; i++) {
      delete dataDict[i]->val;
      delete dataDict[i];
   }

//
// Free pixmaps
//
   count = pixmapFileDict.size();
   for (i=0; i<count; i++) {
      PixmapC	*pm = pixmapFileDict[i]->val;
      delete pm;
   }

   count = pixmapDataDict.size();
   for (i=0; i<count; i++) {
      PixmapC	*pm = pixmapDataDict[i]->val;
      delete pm;
   }

   count = columnList.size();
   for (i=0; i<count; i++) {
      delete columnList[i];
   }

//
// Free graphics structures
//
   if ( halApp->xRunning ) {
      if ( titleGC     ) XtReleaseGC(titleDA, titleGC);
      if ( titlePickGC ) XtReleaseGC(viewBox->ViewDA(), titlePickGC);
      if ( resizeCursor ) XFreeCursor(halApp->display, resizeCursor);
   }

} // End Destructor

/*-----------------------------------------------------------------------
 *  Display this view
 */

void
FieldViewC::Show()
{
   shown = True;

//
// Display the title area
//
   if ( !titleWin && XtIsRealized(titleDA) ) InitializeTitle();
   XtManageChild(titleDA);

//
// Attach the scrolled window to the title area
//
   WArgList	args;
   args.TopAttachment(XmATTACH_WIDGET, titleDA);
   XtSetValues(viewBox->ScrollForm(), ARGS);

//
// Loop through items and add any that are needed
//
   VItemListC&	items = viewBox->Items();
   unsigned	count = items.size();
   for (int i=0; i<count; i++) {
      VItemC		*item = items[i];

//
// Look up the view data in the data dictionary
//
      ItemDataC *data = dataDict.definitionOf(item);
      if ( data ) {
         item->SetViewData(data);
	 //..data->GetFieldSizes(font);
      } else {
	 _AddItem(*item);
      }

   } // End for each item in view

   needPlaces = True;

//
// Set the GC values
//
   XtGCMask	valMask = GCClipMask | GCFillStyle | GCFunction
			| GCGraphicsExposures | GCLineStyle | GCLineWidth
			| GCPlaneMask | GCForeground;
   XGCValues vals;
   vals.clip_mask		= None;
   vals.fill_style		= FillSolid;
   vals.function		= GXcopy;
   vals.graphics_exposures	= TRUE;
   vals.line_style		= LineSolid;
   vals.line_width		= 0;
   vals.plane_mask		= AllPlanes;
   vals.foreground		= regFgColor;

   if ( viewBox->GoodFont() ) {
      valMask |= GCFont;
      vals.font = font->fid;
   }

   XChangeGC(halApp->display, viewBox->ViewGC(), valMask, &vals);

   _EnablePickSortColumn(pickSortColumnEnabled);

   Redraw();

} // End Show

/*-----------------------------------------------------------------------
 *  Remove this view
 */

void
FieldViewC::Hide()
{
//
// Attach the scrolled window to the form
//
   WArgList	args;
   args.TopAttachment(XmATTACH_FORM);
   XtSetValues(viewBox->ScrollForm(), ARGS);

//
// Hide the title areas
//
   XtUnmanageChild(titleDA);

//
// Clear view data
//
   VItemListC&	items = viewBox->Items();
   unsigned     count = items.size();
   for (int i=0; i<count; i++) {
      //cout <<"Clearing data for item " <<hex <<(int)items[i] <<dec NL;
      items[i]->SetViewData(NULL);
   }

   _EnablePickSortColumn(False);

   shown = False;

} // End Hide

/*-----------------------------------------------------------------------
 *  Calculate all sizes
 */

void
FieldViewC::PlaceItems()
{
   fontList = viewBox->FontList();
   font     = viewBox->Font();

//
// Initialize columns with title sizes.  Also get title height.
//
   maxTitleHt = 0;
   unsigned	ccount = columnList.size();
   int	i;
   for (i=0; i<ccount; i++) {

      int id = *columnDisplayOrder[i];
      ColumnC	*column = columnList[id];
      if ( !column->visible ) continue;

      FieldC&   title = column->title;
      if ( title.height == 0 ) GetTitleSize(title);

      column->curWd = title.width;
      if ( title.height > maxTitleHt ) maxTitleHt = title.height;

   } // End for each column

//
// Loop through items
//
   daHt    = 0;
   daWd    = 0;
   maxPmWd = 0;

   int	extra = (hlThick + itemMarginHt) * 2 + ySpacing;

   VItemListC&  visItems = viewBox->VisItems();
   unsigned     icount = visItems.size();
   for (i=0; i<icount; i++) {

      VItemC	*item = visItems[i];
      ItemDataC *data = (ItemDataC *)item->ViewData();
      if ( !DataValid(data) ) continue;

//
// Get the item size
//
      if ( data->itemHt == 0 ) GetFieldSizes(data);

//
// Update the pixmap size width
//
      if ( showPixmaps && data->pixmap && data->pixmap->wd > maxPmWd )
	 maxPmWd = data->pixmap->wd;

//
// Update the column widths
//
      for (int j=0; j<ccount; j++) {

	 int		pos     = *columnDisplayOrder[j];
	 ColumnC	*column = columnList[pos];
	 if ( !column->visible || pos < 0 || pos >= data->fieldList.size() )
	    continue;

	 FieldC  *field = data->fieldList[pos];
	 if ( field->width > column->curWd ) column->curWd = field->width;
	 
      } // End for each column

//
// Update the drawing area height
//
      daHt += data->itemHt + extra;

   } // End for each view item

   daHt -= ySpacing;

//
// Get the overall area width
//
   extra = (itemMarginWd*2) + sepThick;
   daWd = (itemMarginWd + hlThick) * 2;
   daWd += maxPmWd;
   for (i=0; i<ccount; i++) {

      int id = *columnDisplayOrder[i];
      ColumnC	*column = columnList[id];
      if ( !column->visible ) continue;

      daWd += column->Width() + extra;
   }

//
// Loop through the items and set their bounds
//
   extra = (hlThick + itemMarginHt) * 2;

   int	x = 0;
   int	y = 0;
   for (i=0; i<icount; i++) {

      VItemC	*item = visItems[i];
      ItemDataC	*data = (ItemDataC *)item->ViewData();
      if ( !DataValid(data) ) continue;

      int	ht = data->itemHt + extra;
      data->bounds.Set(x, y, daWd, ht);

      y += ht + ySpacing;

   } // End for each visible item

   needPlaces = False;

} // End PlaceItems

/*-----------------------------------------------------------------------
 *  Return size
 */

void
FieldViewC::GetSize(int *wd, int *ht)
{
   *wd = daWd;
   *ht = daHt;
}

/*-----------------------------------------------------------------------
 *  Public method to add a new item to this view box
 */

void
FieldViewC::AddItem(VItemC& item)
{
   if ( !viewBox->Realized() ) return;

//
// Add the item
//
   _AddItem(item);

//
// Redraw if necessary
//
   viewBox->Changed(True);

} // End AddItem

/*-----------------------------------------------------------------------
 *  Add new items to this view
 */

void
FieldViewC::AddItems(const VItemListC& list)
{
   if ( !viewBox->Realized() ) return;

//
// Add items
//
   unsigned	count = list.size();
   for (int i=0; i<count; i++) _AddItem(*list[i]);

   viewBox->Changed(True);

} // End AddItems

/*-----------------------------------------------------------------------
 *  Private method to add a new item to this view box
 */

void
FieldViewC::_AddItem(VItemC& item)
{
//
// Create a new icon data structure and add it to the dictionary
//
   ItemDataC	*data = new ItemDataC(item, this);
   //cout <<"Created data " <<hex <<(int)data <<" for item "
   // 	  <<(int)&item SP (int)data->item <<dec NL;
   dataDict.add(&item, data);
   item.SetViewData(data);

//
// Create the pixmap if necessary
//
   if ( showPixmaps /*..&& viewBox->Realized()..*/ ) {
      GetPixmap(*data);
      //..if ( data->pixmap->wd > maxPmWd ) maxPmWd = data->pixmap->wd;
   }

   needPlaces = True;

} // End _AddItem

/*-----------------------------------------------------------------------
 *  Handle a title exposure event
 */

void
FieldViewC::TitleExpose(Widget, FieldViewC *This,
		        XmDrawingAreaCallbackStruct *da)
{
   XExposeEvent	*ev = (XExposeEvent*)da->event;

//
// Initialize if necessary
//
   if ( !This->titleWin ) This->InitializeTitle();

//
// Draw if this is the last expose event
//
   if ( ev->count == 0 ) This->DrawTitles();

} // End TitleExpose

/*-----------------------------------------------------------------------
 *  Create graphics context
 */

void
FieldViewC::InitializeTitle()
{
   titleWin = XtWindow(titleDA);

//
// Create graphics contexts for drawing
//
   XtGCMask	fixMask = GCClipMask | GCFillStyle | GCFunction
			| GCGraphicsExposures | GCLineStyle | GCLineWidth
			| GCPlaneMask;
   XtGCMask	modMask = GCForeground;
   XtGCMask	naMask	= GCArcMode | GCBackground | GCCapStyle | GCClipXOrigin
			| GCClipYOrigin | GCDashList | GCDashOffset
			| GCFillRule | GCJoinStyle | GCStipple | GCSubwindowMode
			| GCTile | GCTileStipXOrigin | GCTileStipYOrigin;
   XGCValues fixVals;
   fixVals.clip_mask = None;
   fixVals.fill_style = FillSolid;
   fixVals.function = GXcopy;
   fixVals.graphics_exposures = TRUE;
   fixVals.line_style = LineSolid;
   fixVals.line_width = 0;
   fixVals.plane_mask = AllPlanes;
   fixVals.foreground = regFgColor;

   if ( viewBox->GoodFont() ) {
      fixMask |= GCFont;
      fixVals.font = font->fid;
   }

   titleGC = XtAllocateGC(titleDA, 0, fixMask, &fixVals, modMask, naMask);

} // End InitializeTitle

/*-----------------------------------------------------------------------
 *  Handle button and keyboard events in title bar
 */

void
FieldViewC::TitleInput(Widget, FieldViewC *This,
		       XmDrawingAreaCallbackStruct *da)
{
   int	type = da->event->type;
   if ( type != ButtonPress && type != ButtonRelease ) return;

   XButtonEvent	*be = (XButtonEvent *)da->event;
   if ( be->button != Button1 ) return;

   if      ( type == ButtonPress   ) This->TitleButton1Press(be);
   else if ( type == ButtonRelease ) This->TitleButton1Release(be);

} // End TitleInput

/*-----------------------------------------------------------------------
 *  Handle press of mouse button 1 in title bar
 */

void
FieldViewC::TitleButton1Press(XButtonEvent *be)
{
//
// Translate the position by the scroll factors
//
   titlePickX = titlePickOrigX = be->x + viewBox->HScrollValue();
   pickSortDistance = 0;

//
// Find the closest thing, a column separator or a title..
//
   int	x = itemMarginWd + hlThick;
   if ( showPixmaps ) x += maxPmWd + itemMarginWd + sepThick + itemMarginWd;

   int	minDist = MAXINT;
   titlePickCol = NULL;
   int		finalX = x;
   unsigned	count = columnList.size();
   for (int i=0; i<count; i++) {

      int id = *columnDisplayOrder[i];
      ColumnC	*column = columnList[id];
      if ( !column->visible ) continue;

      int dist;
      if ( dragTitlesEnabled ) {
//
//       See if the pick position is closer to the title string or
//       to the separator.  First Find the start and end positions
//       of the title string.
//
         FieldC&  title = column->title;
         int      colWd = column->Width();
         int      totalWd = itemMarginWd*2 + colWd;
         int      xoff = (totalWd - title.width) / 2;
         if ( xoff < 0 ) xoff = 0;
         int sx = x+xoff;
         int ex = sx+title.width;
         if ( title.width+xoff > colWd ) // Consider clipping...
            ex = x+colWd;

         dist = (sx - titlePickX);
         if ( dist < 0 ) dist = -dist;
         if ( dist < minDist ) {
            minDist = dist;
            titlePickCol = column;
            finalX = x;
            moveIndex = id;
            moveField = &title;
         }

         dist = (ex - titlePickX);
         if ( dist < 0 ) dist = -dist;
         if ( dist < minDist ) {
            minDist = dist;
            titlePickCol = column;
            finalX = x;
            moveIndex = id;
            moveField = &title;
         }
      }

//
//    Find position of separator
//
      x += column->Width() + itemMarginWd;
      dist = (x - titlePickX);
      if ( dist < 0 ) dist = -dist;
      if ( dist < minDist ) {
	 minDist = dist;
	 titlePickCol = column;
	 finalX = x;
	 moveField = NULL;
      }

//
//    Move to next column
//
      x += sepThick + itemMarginWd;

   } // End for each column

   if ( !titlePickCol ) return;

//
// Create a GC for the rubberband line if necessary
//
   Widget gcw = viewBox->ViewForm();
   if ( !titlePickGC ) {

      Pixel	fg, bg;
      XtVaGetValues( viewBox->ViewDA(), XmNforeground, &fg,
			  XmNbackground, &bg, NULL);

      XGCValues	pickVals;
      pickVals.foreground     = fg ^ bg;
      pickVals.function       = GXxor;
      pickVals.subwindow_mode = IncludeInferiors;
      titlePickGC = XtGetGC(gcw,
			    GCSubwindowMode|GCForeground|GCFunction,
      			    &pickVals);
      titlePickWin = XtWindow(gcw);
   }

   Window  child;
   XTranslateCoordinates(XtDisplay(gcw), be->window,
			 XtWindow(gcw), 0, 0,
                         &titlePickXoffset, &titlePickYoffset,
			 &child);

   if ( moveField ) {
//
//    Disable the cursor and start moving the title.
//
      // XUndefineCursor(halApp->display, XtWindow(viewBox->ViewForm()));
      titlePickX = be->x;
      titlePickY = be->y;
      XDrawString(halApp->display, titlePickWin, titlePickGC,
		  titlePickX+titlePickXoffset,
		  titlePickY+titlePickYoffset,
		  moveField->string, moveField->string.size());
   }
   else {
//
//    Move the cursor to that position and change its image
//
      if ( !resizeCursor )
         resizeCursor = XCreateFontCursor(halApp->display, XC_sb_h_double_arrow);
      XDefineCursor(halApp->display, XtWindow(viewBox->ViewForm()), resizeCursor);

      XWarpPointer(halApp->display, None, titleWin, 0, 0, 0, 0,
                      finalX - viewBox->HScrollValue(), maxTitleHt/(int)2);

//
//    Find the range of motion
//
      Dimension ht;
      XtVaGetValues(titleDA, XmNheight, &ht, NULL);
      titlePickXmin = titlePickX - titlePickCol->Width();
      titlePickYmin = 0;
      titlePickYmax = maxTitleHt + daHt + (int)ht;

//
//    Initialize the rubberband line
//
      titleX = titleY = 0;
      XDrawLine(halApp->display, titlePickWin, titlePickGC,
                titleX+titlePickX-viewBox->HScrollValue()+titlePickXoffset,
                titleY+titlePickYmin+titlePickYoffset,
                titleX+titlePickX-viewBox->HScrollValue()+titlePickXoffset,
                titleY+titlePickYmax+titlePickYoffset);
   }

//
// Look for movement
//
   XtAddEventHandler(titleDA, Button1MotionMask, False,
		     (XtEventHandler)TitleButton1Motion, (XtPointer)this);

} // End TitleButton1Press

/*-----------------------------------------------------------------------
 *  Handle motion of cursor with mouse button 1 pressed in title bar
 */

void
FieldViewC::TitleButton1Motion(Widget, FieldViewC *This, XMotionEvent *ev,
			       Boolean*)
{
   //cout <<"TitleButton1Motion to " <<ev->x <<endl;

   if ( This->moveField ) {
//
//    Clear the current string.
//
      XDrawString(halApp->display, This->titlePickWin, This->titlePickGC,
		  This->titlePickX+This->titlePickXoffset,
		  This->titlePickY+This->titlePickYoffset,
		  This->moveField->string, This->moveField->string.size());
      This->titlePickX = ev->x;
      This->titlePickY = ev->y;

//
//    Keep track of the max distance we've moved
//
      int dx = (ev->x + This->viewBox->HScrollValue())-This->titlePickOrigX;
      if ( dx < 0 ) dx = -dx;
      if ( dx > This->pickSortDistance ) This->pickSortDistance = dx;

//
//    Draw the string in the new position.
//
      XDrawString(halApp->display, This->titlePickWin, This->titlePickGC,
		  This->titlePickX+This->titlePickXoffset,
		  This->titlePickY+This->titlePickYoffset,
		  This->moveField->string, This->moveField->string.size());
   }
   else {
      int ex = ev->x + This->viewBox->HScrollValue();

//
//    Don't allow movement past the limits
//
      if ( ex < This->titlePickXmin ) ex = This->titlePickXmin;

//
//    Clear the current line
//
      XDrawLine(halApp->display, This->titlePickWin, This->titlePickGC   ,
	        This->titleX+This->titlePickX-This->viewBox->HScrollValue()+This->titlePickXoffset,
   	        This->titleY+This->titlePickYmin+This->titlePickYoffset,
	        This->titleX+This->titlePickX-This->viewBox->HScrollValue()+This->titlePickXoffset,
	        This->titleY+This->titlePickYmax+This->titlePickYoffset);

//
//    Redraw the line
//
      This->titlePickX = ex;
      XDrawLine(halApp->display, This->titlePickWin, This->titlePickGC,
	        This->titleX+This->titlePickX-This->viewBox->HScrollValue()+This->titlePickXoffset,
   	        This->titleY+This->titlePickYmin+This->titlePickYoffset,
	        This->titleX+This->titlePickX-This->viewBox->HScrollValue()+This->titlePickXoffset,
	        This->titleY+This->titlePickYmax+This->titlePickYoffset);
   }

} // End TitleButton1Motion

/*-----------------------------------------------------------------------
 *  Handle release of mouse button 1 in title bar
 */
#define NULL_INDEX -1

void
FieldViewC::TitleButton1Release(XButtonEvent* ev)
{
   // if ( scrollTimer ) XtRemoveTimeOut(scrollTimer);

//
// Stop looking for movement
//
   XtRemoveEventHandler(titleDA, Button1MotionMask, False,
		     (XtEventHandler)TitleButton1Motion, (XtPointer)this);

   if ( moveField ) {
//
//    Clear the string.
//
      XDrawString(halApp->display, titlePickWin, titlePickGC,
		  titlePickX+titlePickXoffset,
		  titlePickY+titlePickYoffset,
		  moveField->string, moveField->string.size());

//
//    Find the column (if any) that the current event position is in
//    and move the moveField column to that column if its different.
//
      int drop_x = ev->x + viewBox->HScrollValue();

      int sx;
      int ex = 0;
      int mwd = itemMarginWd*2;
      unsigned	count = columnList.size();
      for (int i=0; i<count; i++) {

         int id = *columnDisplayOrder[i];
         ColumnC  *column = columnList[id];
         if ( !column->visible ) continue;

         sx = ex;
         ex += column->Width() + mwd;
         if ( drop_x >= sx && drop_x <= ex ) {
            if ( pickSortDistance < pickSortThreshold ) {

//             the sort indexes or 1 based, if the index is negative
//             that means to sort in reverse order.
               int fnd_id = id+1;
	       int pos_id = columnSortOrder.indexOf(fnd_id);
	       int neg_id = columnSortOrder.indexOf(-fnd_id);

	       if ( ev->state & (ShiftMask|ControlMask) ) {
		  if ( pos_id != NULL_INDEX ) {       // change to descending
                     int *nums = columnSortOrder.start();
		     nums[pos_id] = -fnd_id;
                  }
		  else if ( neg_id != NULL_INDEX )    // no sorting
		     columnSortOrder.remove(-fnd_id);
                  else
		     columnSortOrder.append(fnd_id);  // add ascending
	       }
	       else {
		  columnSortOrder.removeAll();        // no sorting
		  if ( pos_id != NULL_INDEX )         // single descending
		     columnSortOrder.append(-fnd_id);
		  else if ( neg_id == NULL_INDEX )
		     columnSortOrder.append(fnd_id);  // single ascending
	       }
	       viewBox->Sort();
	    }
	    else
	       SetColumnDisplayIndex(moveIndex, i);
	    break;
         }
      }
      moveField = NULL;
   }
   else {
//
//    Clear the current rubberband line
//
      XDrawLine(halApp->display, titlePickWin, titlePickGC,
	        titleX+titlePickX-viewBox->HScrollValue()+titlePickXoffset,
   	        titleY+titlePickYmin+titlePickYoffset,
	        titleX+titlePickX-viewBox->HScrollValue()+titlePickXoffset,
	        titleY+titlePickYmax+titlePickYoffset);

//
//    Reset the cursor
//
      XUndefineCursor(halApp->display, XtWindow(viewBox->ViewForm()));

//
//    Set the new column size
//
      int	delta = titlePickX - titlePickOrigX;
      int	wd = titlePickCol->Width() + delta;
      if ( wd < 1 ) wd = 1;
      int	index = columnList.indexOf(titlePickCol);

      if ( delta < 0 ) { // Moved left

         if ( titlePickCol->minWd > 0 && wd < titlePickCol->minWd )
	    SetColumnMinPixels(index, wd);
         SetColumnMaxPixels(index, wd);

      } else if ( delta > 0 ) { // Moved right

         if ( titlePickCol->maxWd > 0 && wd > titlePickCol->maxWd )
	    SetColumnMaxPixels(index, wd);
         SetColumnMinPixels(index, wd);
      }
   }
   
   Redraw();

} // End TitleButton1Release

/*-----------------------------------------------------------------------
 *  Redraw the items in this view
 */

void
FieldViewC::Redraw()
{
   Draw(viewBox->VisRect());
}

/*-----------------------------------------------------------------------
 *  Redraw the items in this view
 */

void
FieldViewC::Draw(RectC& area)
{
   if ( !viewBox->Realized() ) return;

   if ( needPlaces ) {
      PlaceItems();
      viewBox->UpdateScrollBars();
   }

//
// See how many items are visible
//
   VItemListC&	visItems = viewBox->VisItems();
   unsigned	count = visItems.size();

   if ( needSetVisItemCount )
      SetVisibleItemCount(visItemCount);

//
// Highlight the first one if there is none highlighted
//
   if ( !hlItem || !visItems.includes(hlItem) )
      hlItem = count ? visItems[0] : (VItemC*)NULL;

//
// Scroll the area
//
   RectC	sarea = area;
   sarea.xmin += viewBox->HScrollValue();
   sarea.xmax += viewBox->HScrollValue();
   sarea.ymin += viewBox->VScrollValue();
   sarea.ymax += viewBox->VScrollValue();

   GC		gc     = viewBox->ViewGC();
   Window	window = viewBox->ViewPm();

//
// Clear the drawing area
//
   XSetForeground(halApp->display, gc, regBgColor);
   XFillRectangle(halApp->display, window, gc,
		  area.xmin, area.ymin, area.wd, area.ht);

//
// Position of first object
//
   int	x = 0;
   int	y = 0;
   int	maxItemWd = daWd;

//
// Loop through item list
//

   Boolean	inArea   = False;
   Boolean	exitArea = False;
   //..int		extraHt = (hlThick + itemMarginHt)*2;
   int		extraHt = (hlThick + itemMarginHt) * 2 + ySpacing;
   RectC	drawBounds;
   for (int i=0; i<count; i++) {

      VItemC	*item = visItems[i];
      ItemDataC	*data = (ItemDataC *)item->ViewData();
      if ( !DataValid(data) ) continue;

//
// Only draw this item if it is at least partially visible
//
      if ( !exitArea ) {

	 drawBounds.Set(x, y, maxItemWd, data->itemHt + extraHt);

	 if ( sarea.Overlaps(drawBounds) ) {
	    inArea = True;
	    DrawItem(*data, AS_IS, window);
	 }
	 else if ( inArea ) {
	    exitArea = True;
	 }
      }

//
// Move to next line
//
      y += data->bounds.ht + ySpacing;

   } // End for each visible item

   XCopyArea(halApp->display, window, viewBox->ViewWin(), gc, 0, 0,
	     viewBox->DAWidth(), viewBox->DAHeight(), 0, 0);

//
// Draw titles
//
   DrawTitles();

} // End Draw

/*-----------------------------------------------------------------------
 *  Draw the column headings
 */

void
FieldViewC::DrawTitles()
{
   if ( !titleGC || maxTitleHt == 0 ) return;

//
// Set the drawing area size
//
   XtVaSetValues(viewBox->ViewForm(), XmNresizePolicy, XmRESIZE_GROW, NULL);
   XtVaSetValues(titleSizeLabel, XmNheight, maxTitleHt, NULL);
   XtVaSetValues(viewBox->ViewForm(), XmNresizePolicy, XmRESIZE_ANY, NULL);

   Dimension	wd;
   XtVaGetValues(titleDA, XmNwidth, &wd, NULL);

//
// Clear the title area
//
   XSetForeground(halApp->display, titleGC, titleBgColor);
   XFillRectangle(halApp->display, titleWin, titleGC, 0, 0, wd, maxTitleHt);
   XSetForeground(halApp->display, titleGC, titleFgColor);

//
// Draw the titles
//
   int	x = itemMarginWd + hlThick - viewBox->HScrollValue();
   if ( showPixmaps ) x += maxPmWd + itemMarginWd + sepThick + itemMarginWd;
   int		y = itemMarginHt;
   unsigned	tcount = columnList.size();
   for (int i=0; i<tcount; i++) {

      int id = *columnDisplayOrder[i];
      ColumnC	*column = columnList[id];
      if ( !column->visible ) continue;

//
// Draw string with font or fontList
//
      FieldC&	title = column->title;
      int	colWd = column->Width();
      int	totalWd = itemMarginWd*2 + colWd;
      int	xoff = (totalWd - title.width) / 2;
      if ( xoff < 0 ) xoff = 0;

      if ( !fontList ) {

	 int	yoff = (maxTitleHt - title.height) / 2 + font->ascent;

//
// See if we need clipping
//
	 if ( title.width > colWd ) {
	    XRectangle	rect;
	    rect.x      = x+xoff;
	    rect.y      = y;
	    rect.width  = colWd;
	    rect.height = maxTitleHt;
	    XSetClipRectangles(halApp->display, titleGC, 0, 0, &rect, 1,
			       YXBanded);
	 }

//
// Draw string
//
	 XDrawString(halApp->display, titleWin, titleGC, x+xoff, y+yoff,
		     title.string, title.string.size());

//
// Restore clipping
//
	 if ( title.width > colWd )
	    XSetClipMask(halApp->display, titleGC, None);

      } // End if no font list specified

      else {

	 WXmString tempStr((char*)title.string, column->title.tag);
	 XRectangle rect;
	 rect.x      = x+xoff;
	 rect.y      = y;
	 rect.width  = colWd;
	 rect.height = maxTitleHt;
	 XmStringDraw(halApp->display, titleWin, fontList, (XmString)tempStr,
		      titleGC, x, y, colWd, XmALIGNMENT_CENTER,  
		      XmSTRING_DIRECTION_L_TO_R, &rect);
      }

//
// Draw separator
//
      if ( sepThick > 0 /*&& i < tcount-1*/ ) {
	 xoff = colWd + itemMarginWd;
	 if ( sepThick == 1 ) {
	    XDrawLine(halApp->display, titleWin, titleGC, x+xoff, 0, x+xoff,
		      maxTitleHt-1);
	 } else {
	    XFillRectangle(halApp->display, titleWin, titleGC, x+xoff, 0,
			   sepThick, maxTitleHt);
	 }
      }

//
// Move to next column
//
      x += totalWd + sepThick;

   } // End for each column

} // End DrawTitles

/*-----------------------------------------------------------------------
 *  Private method to redraw the specified item
 */

void
FieldViewC::DrawItem(const ItemDataC& data, VItemDrawModeT mode,
		     Drawable drawto)
{
   if ( !viewBox->Realized() ) return;

   Boolean	invert;

   if ( mode == AS_IS )
      invert = viewBox->SelItems().includes(data.item);
   else
      invert = (mode == INVERT);

#if 0
   cout <<"Drawing item " <<data.item <<" for data " <<(int*)&data;
   if ( invert ) cout <<" inverted";
   cout <<endl;
#endif

//
// If drawto is NULL, draw to both the offscreen pixmap and the visible window
//
   Drawable	dst1 = drawto;
   Drawable	dst2 = (Drawable)NULL;
   if ( !dst1 ) {
      dst1 = viewBox->ViewWin();
      dst2 = viewBox->ViewPm();
   }
   GC		gc = viewBox->ViewGC();

//
// Draw background
//
   XSetForeground(halApp->display, gc, invert ? invBgColor : regBgColor);
   int	x  = data.bounds.xmin + hlThick - viewBox->HScrollValue();
   int	y  = data.bounds.ymin + hlThick - viewBox->VScrollValue();
   int	wd = data.bounds.wd - hlThick*2;
   int	ht = data.bounds.ht - hlThick*2;

   XFillRectangle(halApp->display, dst1, gc, x, y, wd, ht);
   if ( dst2 )
      XFillRectangle(halApp->display, dst2, gc, x, y, wd, ht);
   x += itemMarginWd;
   y += itemMarginHt;

//
// Draw pixmap
//
   if ( showPixmaps && data.pixmap ) {

      PixmapC	*pm = data.pixmap;
      Pixmap	src = invert ? pm->inv : pm->reg;

      if ( src ) {

	 int	px = x + (maxPmWd - pm->wd) / 2;
	 int	py = data.bounds.ymin + ((data.bounds.ht - pm->ht) / 2)
		   - viewBox->VScrollValue();

	 if ( pm->mask ) {
            XSetClipMask(halApp->display, gc, pm->mask);
            XSetClipOrigin(halApp->display, gc, px, py);
	 }

	 XCopyArea(halApp->display, src, dst1, gc, /*srcx*/0, /*srcy*/0,
		   pm->wd, pm->ht, /*dstx*/px, /*dsty*/py);
	 if ( dst2 )
	    XCopyArea(halApp->display, src, dst2, gc, /*srcx*/0, /*srcy*/0,
		      pm->wd, pm->ht, /*dstx*/px, /*dsty*/py);

	 if ( pm->mask ) {
            XSetClipMask(halApp->display, gc, None);
            XSetClipOrigin(halApp->display, gc, 0, 0);
	 }

      } // End if source pixmap available

   } // End if pixmap data available and needed

   if ( showPixmaps )
      x += maxPmWd + itemMarginWd + sepThick + itemMarginWd;

   XSetForeground(halApp->display, gc,
		  invert ? invFgColor : regFgColor);
   if ( viewBox->GoodFont() ) 
      XSetFont(halApp->display, gc, font->fid);

//
// Draw field strings
//
   unsigned  count = columnList.size();
   for (int i=0; i<count; i++) {

      int id = *columnDisplayOrder[i];
      ColumnC  *column = columnList[id];
      if ( !column->visible ) continue;

      int  colWd   = column->Width();
      int  totalWd = itemMarginWd*2 + colWd;

      if ( id >= 0 && id < data.fieldList.size() ) {
//
//       Draw the string.
//
         FieldC *field = data.fieldList[id];
         if ( fontList ) {

            unsigned char  alignment;
            WXmString      tempStr((char*)field->string, field->tag);
            XRectangle     rect;
            rect.x         = x;
            rect.y         = y;
            rect.width     = colWd;
            rect.height    = field->height;

            switch (column->justify) {
               case (JUSTIFY_LEFT):
                  alignment = XmALIGNMENT_BEGINNING;
                  break;
               case (JUSTIFY_RIGHT):
                  alignment = XmALIGNMENT_END;
                  break;
               case (JUSTIFY_CENTER):
                  alignment = XmALIGNMENT_CENTER;
                  break;
            } 

            XmStringDraw(halApp->display, dst1, fontList, (XmString)tempStr, gc,
                         x, y, colWd, alignment, XmSTRING_DIRECTION_L_TO_R,
                         &rect);
            if (dst2)
               XmStringDraw(halApp->display, dst2, fontList, (XmString)tempStr, gc,
                         x, y, colWd, alignment, XmSTRING_DIRECTION_L_TO_R,
                         &rect);

         } // End if using font list

         else { //    Draw string with font

            int  xoff = 0;
            switch (column->justify) {

               case (JUSTIFY_LEFT):
                  break;
               case (JUSTIFY_RIGHT):
                  xoff = colWd - field->width;
                  break;
               case (JUSTIFY_CENTER):
                  xoff = (colWd - field->width) / 2;
                  if ( xoff < 0 ) xoff = 0;
                  break;
            } 
            int  yoff = (data.itemHt - field->height) / 2 + font->ascent;

//
//          See if we need clipping
//
            if ( field->width > colWd ) {
               XRectangle  rect;
               rect.x      = x+xoff;
               rect.y      = y;
               rect.width  = colWd;
               rect.height = field->height;
               XSetClipRectangles(halApp->display, gc, 0, 0, &rect, 1, YXBanded);
            }

//
//         Draw the string
//
           XDrawString(halApp->display, dst1, gc, x+xoff, y+yoff,
		       field->string, field->string.size());
           if ( dst2 )
               XDrawString(halApp->display, dst2, gc, x+xoff, y+yoff, 
                           field->string, field->string.size());

//
//          Restore clipping
//
            if ( field->width > colWd )
               XSetClipMask(halApp->display, gc, None);
         }
      }

//
//    Move to next column
//
      x += totalWd + sepThick;

   } // End for each column

//
// Draw separators
//
   DrawItemSeparators(data, mode, drawto);

//
// Draw highlight if necessary
//
   if ( focusHere && data.item == hlItem )
      DrawHighlight(&data, hlColor, drawto);

} // End DrawItem

/*-----------------------------------------------------------------------
 *  Draw a highlight border around an item using the given color
 */

void
FieldViewC::DrawHighlight(const VItemC *item, Pixel color)
{
   if ( !item ) return;

   ItemDataC	*data = (ItemDataC *)item->ViewData();
   if ( DataValid(data) )
      DrawHighlight(data, color, (Drawable)NULL);
}

void
FieldViewC::DrawHighlight(const ItemDataC *data, Pixel color, Drawable drawto)
{
   if ( !data ) return;

   XRectangle	rects[4];
   XRectangle	*rect = rects;

   int		xoff = viewBox->HScrollValue();
   int		yoff = viewBox->VScrollValue();

   rect->x      = data->bounds.xmin - xoff;
   rect->y      = data->bounds.ymin - yoff;
   rect->width  = data->bounds.wd;
   rect->height = hlThick;

   rect++;
   rect->x      = data->bounds.xmin - xoff;
   rect->y      = data->bounds.ymin + hlThick - yoff;
   rect->width  = hlThick;
   rect->height = data->bounds.ht - hlThick*2;

   rect++;
   rect->x      = data->bounds.xmax - hlThick + 1 - xoff;
   rect->y      = data->bounds.ymin + hlThick - yoff;
   rect->width  = hlThick;
   rect->height = data->bounds.ht - hlThick*2;

   rect++;
   rect->x      = data->bounds.xmin - xoff;
   rect->y      = data->bounds.ymax - hlThick + 1 - yoff;
   rect->width  = data->bounds.wd;
   rect->height = hlThick;

   GC	gc = viewBox->ViewGC();
   XSetForeground(halApp->display, gc, color);

//
// If drawto is NULL, draw to both the offscreen pixmap and the visible window
//
   Drawable	dst1 = drawto;
   Drawable	dst2 = (Drawable)NULL;
   if ( !dst1 ) {
      dst1 = viewBox->ViewWin();
      dst2 = viewBox->ViewPm();
   }

#if 0
   cout <<"Highlighting item " <<(int*)data->item
   	<<" using color " <<(int*)color <<endl;
   cout <<"   drawing to " <<(int*)dst1;
   if ( dst2 ) cout <<" and " <<(int*)dst2;
   cout <<endl;
#endif

   XFillRectangles(halApp->display, dst1, gc, rects, 4);
   if ( dst2 )
      XFillRectangles(halApp->display, dst2, gc, rects, 4);

//
// Redraw the separators if we erased the highlight
//
   if ( color != hlColor )
      DrawItemSeparators(*data, AS_IS, drawto);

} // End DrawHighlight

/*-----------------------------------------------------------------------
 *  Method to draw the separators for the specified item
 */

void
FieldViewC::DrawItemSeparators(const ItemDataC& data, VItemDrawModeT mode,
			       Drawable drawto)
{
   if ( sepThick <= 0 ) return;

   GC		gc = viewBox->ViewGC();

//
// If drawto is NULL, draw to both the offscreen pixmap and the visible window
//
   Drawable	dst1 = drawto;
   Drawable	dst2 = (Drawable)NULL;
   if ( !dst1 ) {
      dst1 = viewBox->ViewWin();
      dst2 = viewBox->ViewPm();
   }

   Boolean	invert;
   if ( mode == AS_IS )
      invert = viewBox->SelItems().includes(data.item);
   else
      invert = (mode == INVERT);

   int	count = columnList.size();
   int	x = data.bounds.xmin + hlThick - viewBox->HScrollValue();
   if ( showPixmaps ) x += itemMarginWd + maxPmWd + itemMarginWd + sepThick;
   for (int i=0; i<count-1; i++) {

      int id = *columnDisplayOrder[i];
      ColumnC	*column = columnList[id];
      if ( !column->visible ) continue;

      x += itemMarginWd + column->Width() + itemMarginWd;
      int	y = data.bounds.ymin - viewBox->VScrollValue();

//
//  Add spacing to height if this is not the last item
//
      int	ht = data.bounds.ht;
      if ( viewBox->VisItems().indexOf(data.item) !=
	   viewBox->VisItems().size()-1 ) ht += ySpacing;

      if ( sepThick == 1 ) {

//
// Draw separator
//
	 XSetForeground(halApp->display, gc, regFgColor);

	 XDrawLine(halApp->display, dst1, gc, x, y, x, y+ht);
	 if ( dst2 )
	    XDrawLine(halApp->display, dst2, gc, x, y, x, y+ht);

//
// Invert part of separator if item is selected
//
	 if ( invert ) {
	    XSetForeground(halApp->display, gc, invFgColor);
	    XDrawLine(halApp->display, dst1, gc, x, y+hlThick, x,
		      data.bounds.ymax - viewBox->VScrollValue() - hlThick);
	    if ( dst2 )
	       XDrawLine(halApp->display, dst2, gc, x, y+hlThick, x,
			 data.bounds.ymax - viewBox->VScrollValue() - hlThick);
         }

      } else {

//
// Draw separator
//
	 XSetForeground(halApp->display, gc, regFgColor);
	 XFillRectangle(halApp->display, dst1, gc, x, y, sepThick+1, ht);
	 if ( dst2 )
	    XFillRectangle(halApp->display, dst2, gc, x, y, sepThick+1, ht);

//
// Invert part of separator if item is selected
//
	 if ( invert ) {

	    XSetForeground(halApp->display, gc, invFgColor);
	    XDrawLine(halApp->display, dst1, gc, x, y+hlThick, x,
		      data.bounds.ymax - viewBox->VScrollValue() - hlThick);
	    XFillRectangle(halApp->display, dst1, gc, x, y+hlThick,
			   sepThick+1, data.bounds.ht - hlThick*2);

	    if ( dst2 ) {
	       XDrawLine(halApp->display, dst2, gc, x, y+hlThick, x,
			 data.bounds.ymax - viewBox->VScrollValue() - hlThick);
	       XFillRectangle(halApp->display, dst2, gc, x, y+hlThick,
			      sepThick+1, data.bounds.ht - hlThick*2);
	    }
         }

      } // End sepThick > 1

//
// Move to next column
//
      x += sepThick;

   } // End for each column

} // End DrawItemSeparators

/*-----------------------------------------------------------------------
 *  Public method to redraw the specified item
 */

void
FieldViewC::RedrawItem(const VItemC& item)
{
   if ( !viewBox->Realized() ) return;

//
// Get icon data
//
   ItemDataC	*data = (ItemDataC *)item.ViewData();
   if ( DataValid(data) ) RedrawItem(*data);
}

void
FieldViewC::RedrawItem(ItemDataC& data)
{
   if ( !viewBox->Realized() ) return;

//
// Redraw if item not clipped out
//
   RectC	drawBounds(data.bounds.xmin, data.bounds.ymin,
			   data.bounds.wd, data.bounds.ht+ySpacing);
   RectC	sarea = viewBox->VisRect();
   sarea.xmin += viewBox->HScrollValue();
   sarea.xmax += viewBox->HScrollValue();
   sarea.ymin += viewBox->VScrollValue();
   sarea.ymax += viewBox->VScrollValue();

   if ( sarea.Overlaps(drawBounds) )
      DrawItem(data, AS_IS);

} // End RedrawItem

void
FieldViewC::DrawItem(const ItemDataC& data)
{
   DrawItem(data, AS_IS);
}

void
FieldViewC::DrawItem(const VItemC& item, VItemDrawModeT mode)
{
   ItemDataC	*data = (ItemDataC *)item.ViewData();
   if ( DataValid(data) ) DrawItem(*data, mode);
}

/*-----------------------------------------------------------------------
 *  Find the item that contains the given coordinate
 */

VItemC*
FieldViewC::PickItem(int x, int y)
{
//
// Set up a rectangle around the point in case we are in the space between
//    two items.
//
   RectC	rect(x, y-ySpacing, 1, ySpacing*2);
   VItemListC	list;
   PickItems(rect, list);

   VItemC	*item = NULL;

//
// If there are two items, pick the closest
//
   if ( list.size() == 1 )

      item = list[0];

   else if ( list.size() > 1 ) {

      ItemDataC	*data0 = (ItemDataC *)list[0]->ViewData();
      ItemDataC	*data1 = (ItemDataC *)list[1]->ViewData();

      if ( y <= data0->bounds.ymax )
	 item = list[0];
      else if ( y >= data1->bounds.ymin )
	 item = list[1];
      else {
	 int	dist0 = y - data0->bounds.ymax;
	 int	dist1 = data0->bounds.ymin - y;
	 if ( dist0 < dist1 ) item = list[0];
	 else		      item = list[1];
      }
   }

   return item;

} // End PickItem

/*-----------------------------------------------------------------------
 *  Return the items in the pick rect
 */

void
FieldViewC::PickItems(RectC& pickRect, VItemListC& list)
{
   VItemListC&	visItems = viewBox->VisItems();
   unsigned	count = visItems.size();
   for (int i=0; i<count; i++) {

      VItemC	*item = visItems[i];
      ItemDataC	*data = (ItemDataC *)item->ViewData();
      if ( !DataValid(data) ) continue;

//
// See if the item area overlaps the pick rectangle
//
      if ( data->bounds.Overlaps(pickRect) ) list.append(item);

   } // End for each visible item

} // End PickItems

/*-----------------------------------------------------------------------
 *  Change the visibility of an item
 */

void
FieldViewC::ChangeItemVis(const VItemC&)
{
   needPlaces = True;

   viewBox->Changed(True);

} // End ChangeItemVis

/*-----------------------------------------------------------------------
 *  Change the visibility of unknown items
 */

void
FieldViewC::ChangeItemVis()
{
   needPlaces = True;

   viewBox->Changed(True);

} // End ChangeItemVis

/*-----------------------------------------------------------------------
 *  Flash an item
 */

void
FieldViewC::FlashItem(const VItemC *item)
{
//
// If an item is already flashing, stop it
//
   if ( flashTimer ) {
      XtRemoveTimeOut(flashTimer);
      RedrawItem(*flashItem);
   }

//
// Make sure the item is visible
//
   ScrollToItem(*item);

//
// Start a time out proc to flash the item a few times
//
   flashItem  = (VItemC *)item;
   flashCount = viewBox->FindBlinkCount();
   flashOn    = viewBox->SelItems().includes(item);
   flashTimer = XtAppAddTimeOut(halApp->context, viewBox->FindBlinkInterval(),
				(XtTimerCallbackProc)FlashProc,
			        (XtPointer)this);

} // End FlashItem

/*-----------------------------------------------------------------------
 * Scroll the view window so the specified item is visible
 */

void
FieldViewC::ScrollToItem(const VItemC& item)
{
   ItemDataC	*data = (ItemDataC *)item.ViewData();
   if ( DataValid(data) ) ScrollToItem(data);
}

void
FieldViewC::ScrollToItem(const ItemDataC *data)
{
   if ( !data || !viewBox->Realized() ) return;

   int	ymax = viewBox->VisRect().ymax + viewBox->VScrollValue();
   int	ymin = viewBox->VisRect().ymin + viewBox->VScrollValue();

   if ( data->bounds.ymax > ymax || viewBox->VisRect().ht < data->itemHt ) {

//
// It is off the bottom, so position the bottom of the scroll bar.
//
      int	val, size, inc, pinc, max;
      XmScrollBarGetValues(viewBox->VScrollBar(), &val, &size, &inc, &pinc);
      XtVaGetValues(viewBox->VScrollBar(), XmNmaximum, &max, NULL);

      val = data->bounds.ymax + ySpacing;
      if ( val > max ) val = max;
      val -= size;

      XmScrollBarSetValues(viewBox->VScrollBar(), val, size, inc, pinc,
			   /*notify*/True);

   } else if ( data->bounds.ymin < ymin ) {

//
// It is off the top, so position the top of the scroll bar.
//
      int	val, size, inc, pinc, min;
      XmScrollBarGetValues(viewBox->VScrollBar(), &val, &size, &inc, &pinc);
      XtVaGetValues(viewBox->VScrollBar(), XmNminimum, &min, NULL);

      val = data->bounds.ymin - ySpacing;
      if ( val < min ) val = min;

      XmScrollBarSetValues(viewBox->VScrollBar(), val, size, inc, pinc,
			   /*notify*/True);
   }

} // End ScrollToItem

/*-----------------------------------------------------------------------
 *  Timer proc used to flash an item
 */

void
FieldViewC::FlashProc(FieldViewC *This, XtIntervalId*)
{
   if ( This->flashCount == 0 ) {
      This->RedrawItem(*This->flashItem);
      This->flashItem = NULL;
      This->flashTimer = (XtIntervalId)NULL;
      return;
   }

//
// Flip the state of the blinking item
//
   if ( This->flashOn ) This->DrawItem(*This->flashItem, NORMAL);
   else			This->DrawItem(*This->flashItem, INVERT);
   This->flashOn = !This->flashOn;
   This->flashCount--;

//
// Add this timeout again
//
   This->flashTimer =
      XtAppAddTimeOut(halApp->context, This->viewBox->FindBlinkInterval(),
		      (XtTimerCallbackProc)FlashProc, (XtPointer)This);

} // End FlashProc

/*-----------------------------------------------------------------------
 *  Handle a keyboard focus change event in the area
 */

void
FieldViewC::HandleFocusChange(Widget, FieldViewC *This, XEvent *ev, Boolean *)
{
   if ( !This->shown ) return;

   switch (ev->type) {

      case (FocusIn):
      case (EnterNotify):
	 This->focusHere = True;
	 This->DrawHighlight(This->hlItem, This->hlColor);
	 break;

      case (FocusOut):
      case (LeaveNotify):
	 This->focusHere = False;
	 This->DrawHighlight(This->hlItem, This->regBgColor);
	 break;
   }

} // End HandleFocusChange

/*-----------------------------------------------------------------------
 *  Handle an input event
 */

void
FieldViewC::HandleKeyPress(XKeyEvent *ev)
{
   if ( !focusHere || !hlItem ) return;

   KeySym	keysym;
   Modifiers	state;
   XtTranslateKey(halApp->display, ev->keycode, ev->state, &state, &keysym);

   VItemListC&	visItems = viewBox->VisItems();
   int		index = visItems.indexOf(hlItem);
   unsigned	size = visItems.size();

   switch (keysym) {

      case (XK_Home):
	 HighlightItem(visItems[0]);
	 break;

      case (XK_End):
	 HighlightItem(visItems[size-1]);
	 break;

      case (XK_Up):
	 if ( ev->state & ControlMask )
	    HighlightItem(visItems[0]);
	 else if ( index > 0 )
	    HighlightItem(visItems[index-1]);
	 break;

      case (XK_Down):
	 if ( ev->state & ControlMask )
	    HighlightItem(visItems[size-1]);
	 else if ( index < size-1 )
	    HighlightItem(visItems[index+1]);
	 break;

#if 0
      case (XK_Page_Up): {
	 char	*parms = "0";
	 XtCallActionProc(viewBox->VScrollBar(), "PageUpOrLeft", (XEvent*)ev,
	 		  &parms, 1);
      } break;

      case (XK_Page_Down): {
	 char	*parms = "0";
	 XtCallActionProc(viewBox->VScrollBar(), "PageDownOrRight", (XEvent*)ev,
	 		  &parms, 1);
      } break;
#endif

      case (XK_space):
	 if ( ev->state & (ShiftMask|ControlMask) )
            viewBox->ToggleItem(*hlItem);
	 else
            viewBox->SelectItemOnly(*hlItem);
         break;


   } // End switch keysym

} // End HandleKeyPress

/*-----------------------------------------------------------------------
 *  Handle press of mouse button 1
 */

void
FieldViewC::HandleButton1Press(XButtonEvent *be)
{
//
// There is a chance that several different selections/deselections will take
//    place.  Wait for the button release to really do them.
//
   viewBox->PartialSelect(True);

//
// See if this is part of a multiple click
//
   unsigned long	elapsed = be->time - lastClickTime;
   if ( elapsed <= XtGetMultiClickTime(halApp->display) )
      clickCount++;
   else
      clickCount = 1;
   lastClickTime = be->time;

//
// Translate the position by the scroll factors
//
   pickX = be->x + viewBox->HScrollValue();
   pickY = be->y + viewBox->VScrollValue();

//
// Initialize the pick rectangle.  If this is a shift selection, extend the
//    rectangle to include the nearest existing selection.
//
   Boolean	useRectToPick = False;
   pickDrawMode = INVERT;
   pickState = be->state;
   if ( (pickState & ShiftMask) == ShiftMask ) {

      VItemListC&	selItems = viewBox->SelItems();
      unsigned		selCount = selItems.size();
      if ( selCount == 0 ) {

	 pickRect.Set(0, pickY, viewBox->DAWidth(), 1);

      } else {

//
// Find the closest item
//
	 int	minDist = MAXINT;
	 VItemC	*minItem = NULL;
	 for (int i=0; i<selCount; i++) {

	    VItemC	*item = selItems[i];
	    ItemDataC	*data = (ItemDataC *)item->ViewData();

	    int	dist1 = data->bounds.ymin - pickY;
	    int	dist2 = data->bounds.ymax - pickY;
	    if ( dist1 < 0 ) dist1 = -dist1;
	    if ( dist2 < 0 ) dist2 = -dist2;

	    int	dist = MIN(dist1, dist2);
	    if ( dist < minDist ) {
	       minDist = dist;
	       minItem = item;
	    }

	 } // End for each selected item

//
// Extend the pick rect to include the nearest item
//
         if ( !minItem ) {
	    pickRect.Set(0, pickY, viewBox->DAWidth(), 1);
	 } else {
	    ItemDataC	*data = (ItemDataC *)minItem->ViewData();
	    if ( pickY < data->bounds.ymin ) {
	       pickRect.Set(0, pickY, viewBox->DAWidth(),
	       		    data->bounds.ymax - pickY);
	       pickY = pickRect.ymax;
	       useRectToPick = True;
	    } else if ( pickY > data->bounds.ymax ) {
	       pickRect.Set(0, data->bounds.ymin, viewBox->DAWidth(),
	       		    pickY - data->bounds.ymin);
	       pickY = pickRect.ymin;
	       useRectToPick = True;
	    } else
	       pickRect.Set(0, pickY, viewBox->DAWidth(), 1);
	 }

      } // End if there are any selected items

   } // End if this is a shift selection

   else if ( (pickState & ControlMask) == ControlMask ) {

//
// If this press is over a selected item, we will be turning selections off
//
      VItemC	*item = PickItem(pickX, pickY);
      if ( item && viewBox->SelItems().includes(item) )
	 pickDrawMode = NORMAL;
      pickRect.Set(0, pickY, viewBox->DAWidth(), 1);
   }

   else {
      if ( clickCount == 1 ) viewBox->DeselectAllItems();
      pickRect.Set(0, pickY, viewBox->DAWidth(), 1);
   }

//
// Pick and highlight the new items
//
   pickList.removeAll();

   if ( useRectToPick ) {
      PickItems(pickRect, pickList);
      pressItem = NULL;
   } else {
      pressItem = PickItem(pickX, pickY);
      if ( pressItem ) {
	 pickList.add(pressItem);
	 HighlightItem(pressItem);
      }
   }

   if ( clickCount == 1 ) {
      unsigned	count = pickList.size();
      for (int i=0; i<count; i++)
	 DrawItem(*pickList[i], pickDrawMode);
   }

} // End HandleButton1Press

/*-----------------------------------------------------------------------
 *  Handle release of mouse button 1
 */

void
FieldViewC::HandleButton1Release(XButtonEvent *be)
{
   if ( scrollTimer ) XtRemoveTimeOut(scrollTimer);

//
// Perform single or double click actions.  If the single click fails,
//    reset the lastClickTime so we don't get a double click
//
   if ( clickCount == 1 )
      HandleSingleClick();
   else if ( clickCount == 2 )
      HandleDoubleClick(be);

//
// Now finish processing any selections that were queued up.
//
   viewBox->PartialSelect(False);

} // End HandleButton1Release

/*-----------------------------------------------------------------------
 *  Handle a single click event in the area.
 */

void
FieldViewC::HandleSingleClick()
{
//
// If there isn't exactly one item selected, the the lastClickTime to 0 so
//    we don't get a double click
//
   if ( pickList.size() != 1 ) lastClickTime = 0;

//
// If no items are selected, return
//
   if ( pickList.size() < 1 ) return;

//
// Update the selection
//
   if ( (pickState & ShiftMask) == ShiftMask )
      viewBox->SelectItems(pickList);
   else if ( (pickState & ControlMask) == ControlMask ) {
      if ( pickDrawMode == INVERT ) // Selecting
	 viewBox->SelectItems(pickList);
      else
	 viewBox->DeselectItems(pickList);
   }
   else
      viewBox->SelectItemsOnly(pickList);

} // End HandleSingleClick

/*-----------------------------------------------------------------------
 *  Handle a double click event in the area
 */

void
FieldViewC::HandleDoubleClick(XButtonEvent *be)
{
   if ( !pressItem ) return;

//
// Translate the position by the scroll factors
//
   int	x = be->x + viewBox->HScrollValue();
   int	y = be->y + viewBox->VScrollValue();

//
// See which icon is under pointer.  If this is not the same item as the
//    press event, start over
//
   VItemC	*releaseItem = PickItem(x, y);
   if ( releaseItem != pressItem ) return;

//
// Highlight the item
//
   HighlightItem(pressItem);

//
// Open the item
//
   pressItem->CallOpenCallbacks();

} // End HandleDoubleClick

/*-----------------------------------------------------------------------
 *  Handle pointer motion with button 1 pressed
 */

void
FieldViewC::HandleButton1Motion(XMotionEvent *ev)
{
//
// If the pointer is outside the bounds, start a timer to auto-scroll
//
   Boolean	off_top = (ev->y <= 0);
   Boolean	off_bot = (ev->y >= (int)viewBox->DAHeight());
   if ( off_top || off_bot ) {

//
// Return if no scrolling is possible
//
      if ( !viewBox->VScrollOn() ||
	   (off_top && viewBox->VScrollValue() == 0) ||
	   (off_bot && viewBox->VScrollValue() == viewBox->VScrollMaxValue()) )
	 return;

//
// Keep the event position inside the window.
//
      if ( off_top ) ev->y = 1;
      else           ev->y = viewBox->DAHeight() - 1;
      scrollEvent  = *ev;

//
// Start scrolling if we're not already
//
      if ( !scrollTimer ) {

//
// Call the action proc to scroll
//
	 char	*parms = "0";
	 scrollAction =
            (char *) (off_top ? "IncrementUpOrLeft" : "IncrementDownOrRight");
	 XtCallActionProc(viewBox->VScrollBar(), scrollAction,
			  (XEvent*)&scrollEvent, &parms, 1);

//
// Start automatic scrolling
//
	 scrollTimer  = XtAppAddTimeOut(halApp->context, autoScrollInterval,
					(XtTimerCallbackProc)HandleAutoScroll,
					(XtPointer)this);

      } // End if not already scrolling

   } // End if pointer hit top or bottom of window

   else {

//
// Stop any auto scrolling
//
      if ( scrollTimer ) XtRemoveTimeOut(scrollTimer);
      scrollTimer = (XtIntervalId)NULL;
   }

//
// Translate the position by the scroll factors
//
   int	x = ev->x + viewBox->HScrollValue();
   int	y = ev->y + viewBox->VScrollValue();
   UpdatePickList(x, y);

} // End HandleButton1Motion

/*-----------------------------------------------------------------------
 *  Handle automatic scrolling
 */

void
FieldViewC::HandleAutoScroll(FieldViewC *This, XtIntervalId*)
{
//
// Call the action proc to scroll
//
   char	*parms = "0";
   XtCallActionProc(This->viewBox->VScrollBar(), This->scrollAction,
		    (XEvent*)&This->scrollEvent, &parms, 1);

//
// Redraw the items in the pick list since the scroll cleared them
//
   RectC	sarea = This->viewBox->VisRect();
   sarea.xmin += This->viewBox->HScrollValue();
   sarea.xmax += This->viewBox->HScrollValue();
   sarea.ymin += This->viewBox->VScrollValue();
   sarea.ymax += This->viewBox->VScrollValue();

   RectC	drawBounds;
   unsigned	count = This->pickList.size();
   for (int i=0; i<count; i++) {

      VItemC	*item = This->pickList[i];
      ItemDataC	*data = (ItemDataC *)item->ViewData();

      drawBounds = data->bounds;
      drawBounds.ht   += This->ySpacing;
      drawBounds.ymax += This->ySpacing;

//
// Redraw if item not clipped out
//
      if ( sarea.Overlaps(drawBounds) )
	 This->DrawItem(*data, This->pickDrawMode);
   }

//
// Translate the position by the scroll factors and pick again
//
   int	x = This->scrollEvent.x + This->viewBox->HScrollValue();
   int	y = This->scrollEvent.y + This->viewBox->VScrollValue();
   This->UpdatePickList(x, y);

//
// Repeat the cycle
//
   This->scrollTimer = XtAppAddTimeOut(halApp->context,
				       This->autoScrollInterval,
				       (XtTimerCallbackProc)HandleAutoScroll,
				       (XtPointer)This);

} // End HandleAutoScroll

/*-----------------------------------------------------------------------
 *  Method to calculate the range of selected items and invert them
 */

void
FieldViewC::UpdatePickList(int, int y)
{
//
// Update the pick rectangle
//
   if ( pickY > y ) {
      pickRect.ymin = y;
      pickRect.ymax = pickY;
   } else {
      pickRect.ymin = pickY;
      pickRect.ymax = y;
   }
   pickRect.ht = pickRect.ymax - pickRect.ymin + 1;

//
// Get a new list of items in the pick rectangle
//
   VItemListC	list;
   PickItems(pickRect, list);

//
// Remove items in old list, but not in new list
//
   unsigned	count = pickList.size();
   int	i;
   for (i=count-1; i>=0; i--) {
      VItemC	*item = pickList[i];
      if ( !list.includes(item) ) {
	 DrawItem(*item, AS_IS);
	 pickList.remove(i);
      }
   }

//
// Add items in new list, but not in old list
//
   count = list.size();
   for (i=0; i<count; i++) {
      VItemC	*item = list[i];
      if ( !pickList.includes(item) ) {
	 DrawItem(*item, pickDrawMode);
	 pickList.add(item);
      }
   }

} // End UpdatePickList

/*-----------------------------------------------------------------------
 *  Highlight an item
 */

void
FieldViewC::HighlightItem(const VItemC *item)
{
   if ( item == hlItem ) return;

//
// Remove current highlight
//
   if ( hlItem ) DrawHighlight(hlItem, regBgColor);

   hlItem = (VItemC *)item;
   if ( !hlItem ) return;

//
// Make sure the new item is visible
//
   ScrollToItem(*hlItem);

//
//  Highlight the new item
//
   DrawHighlight(hlItem, hlColor);

} // End HighlightItem

/*-----------------------------------------------------------------------
 *  Handle a pointer motion event in the area
 */

void
FieldViewC::HandlePointerMotion(Widget, FieldViewC *This, XMotionEvent*,
				Boolean*)
{
   if ( !This->shown ) return;

//
// Get all pending motion events
//
   XMotionEvent	mev;
   while (XCheckMaskEvent(halApp->display, PointerMotionMask, (XEvent *)&mev));

   Window	root, child;
   int		rx, ry;         // Position in root window
   int		vx, vy;         // Position in view window
   unsigned	mods;           // Keyboard modifiers

//
// Get current pointer location
//
   XQueryPointer(halApp->display, This->viewBox->ViewWin(), &root, &child,
		 &rx, &ry, &vx, &vy, &mods);

//
// Find the item under the pointer
//
   VItemC	*item = This->PickItem(vx + This->viewBox->HScrollValue(),
   				       vy + This->viewBox->VScrollValue());

   if ( item == This->hlItem ) return;

//
// Remove current highlight
//
   if ( This->hlItem )
      This->DrawHighlight(This->hlItem, This->regBgColor);

   This->hlItem = item;

//
//  Highlight the new item
//
   if ( This->hlItem )
      This->DrawHighlight(This->hlItem, This->hlColor);

} // End HandlePointerMotion

/*-----------------------------------------------------------------------
 *  Change the fields for an item
 */

void
FieldViewC::SetItemFields(VItemC& item, const StringListC& list)
{
//
// Look up the icon data
//
   ItemDataC	*data = (ItemDataC *)item.ViewData();
   if ( !DataValid(data) ) return;

//
// Get list sizes
//
   StringListC&	fieldList = item.FieldList();
   unsigned	oldCount = fieldList.size();
   unsigned	newCount = list.size();

//
// Loop through new fields
//
   if ( &list != &fieldList ) {
      for (int i=0; i<newCount; i++) {

	 if ( i < oldCount ) *fieldList[i] = *list[i];
	 else		     fieldList.append(*list[i]);
      }
   }

   ChangeItemFields(data);
}

void
FieldViewC::ChangeItemFields(const VItemC& item)
{
//
// Look up the icon data
//
   ItemDataC	*data = (ItemDataC *)item.ViewData();
   if ( DataValid(data) ) ChangeItemFields(data);
}

void
FieldViewC::ChangeItemFields(ItemDataC *data)
{
   if ( viewBox->Realized() ) GetFieldSizes(data);

   needPlaces = True;

   viewBox->Changed(True);

} // End SetItemFields

/*-----------------------------------------------------------------------
 *  Change one field for an item
 */

void
FieldViewC::SetItemField(VItemC& item, int i, const StringC& value)
{
//
// Look up the icon data
//
   ItemDataC	*data = (ItemDataC *)item.ViewData();
   if ( !DataValid(data) ) return;

//
// Check validity of index
//
   StringListC&	fieldList = item.FieldList();
   unsigned	count = fieldList.size();
   if ( i<0 || i>=count ) return;

//
// Update value
//
   StringC	*oldString = fieldList[i];
   *oldString = value;

   if ( viewBox->Realized() ) GetFieldSizes(data);

   needPlaces = True;
   viewBox->Changed(True);

} // End SetItemField

/*-----------------------------------------------------------------------
 *  Public method to remove an item from this view
 */

void
FieldViewC::RemoveItem(VItemC& item)
{
   //cout <<"FieldViewC::RemoveItem" NL;
//
// Get icon data structure
//
   ItemDataC	*data = (ItemDataC *)item.ViewData();
   if ( !DataValid(data) ) return;

   //cout <<"Removing entry from data dictionary: " <<&item SP data NL;
   //cout <<"Before removal: \n" <<dataDict NL;
   dataDict.remove(&item);
   //cout <<"After removal: \n" <<dataDict NL;
   item.SetViewData(NULL);

   //cout <<"Deleting data " <<hex <<(int)data <<" for item "
   //     <<(int)&item SP (int)data->item <<dec NL;

   needPlaces = True;

//
// Delete icon data structure
//
   delete data;
   if ( hlItem == &item ) hlItem = NULL;

   viewBox->Changed(True);

} // End RemoveItem

/*-----------------------------------------------------------------------
 *  Remove items from this view
 */

void
FieldViewC::RemoveItems(const VItemListC& list)
{
   //cout <<"FieldViewC::RemoveItems" NL;

   //..Boolean	sizeChange = False;

//
// Remove items
//
   unsigned	count = list.size();
   for (int i=0; i<count; i++) {

//
// Get icon data structure
//
      VItemC	*item = list[i];
      ItemDataC	*data = (ItemDataC *)item->ViewData();
      if ( !DataValid(data) ) continue;

      //cout <<"Removing entry from data dictionary: " <<item SP data NL;
      //cout <<"Before removal: \n" <<dataDict NL;
      dataDict.remove(item);
      //cout <<"After removal: \n" <<dataDict NL;

      item->SetViewData(NULL);

      //cout <<"Deleting data " <<hex <<(int)data <<" for item "
      //     <<(int)item SP (int)data->item <<dec NL;

//
// If this item is visible, update the column sizes
//
      //..if ( viewBox->VisItems().includes(item) ) sizeChange = True;
      if ( viewBox->VisItems().includes(item) ) needPlaces = True;

//
// Delete icon data structure
//
      delete data;
      if ( hlItem == item ) hlItem = NULL;

   } // End for each item to be deleted

   viewBox->Changed(True);

} // End VBoxC RemoveItems

/*-----------------------------------------------------------------------
 *  Return widget name for button in popup menu used to display this view
 */

char *
FieldViewC::ButtonName() const
{
   return "fieldViewPB";
}

/*-----------------------------------------------------------------------
 *  Method to set the titles for the fields
 */

void
FieldViewC::SetTitles(const StringListC& titleList)
{
   unsigned	columnCount = columnList.size();
   unsigned	titleCount  = titleList.size();

//
// Loop through new titles
//
   int	i;
   for (i=0; i<titleCount; i++) {

      StringC	*title = titleList[i];

//
// If this column does not exist, create a new one
//
      ColumnC	*column;
      if ( i < columnCount ) {
	 column = columnList[i];
      } else {
	 column = new ColumnC;
	 columnList.append(column);
	 columnCount++;
      }

//
// Update the title and widths
//
      if ( column->title.string != *title ) {
	 column->title.string = *title;
	 if ( viewBox->Realized() ) GetTitleSize(column->title);
      }

   } // End for each title

//
// Remove unnecessary fields
//
   for (i=columnCount-1; i>=titleCount; i--) {
      ColumnC	*column = columnList[i];
      columnList.remove(column);
      delete column;
   }

//
// Add new ones if the list grew.
//
   for ( i=columnDisplayOrder.size(); i<columnList.size(); i++ )
      columnDisplayOrder.append(i);

//
// Check the reused old index values if the list shrunk.
//
   if ( columnDisplayOrder.size() > columnList.size() ) { 
      for ( i=0; i<columnDisplayOrder.size(); i++ ) {
	 if ( *columnDisplayOrder[i] >= columnList.size() ) {
	    columnDisplayOrder.removeAll();
	    for ( i=0; i<columnList.size(); i++ ) {
	       columnDisplayOrder.append(i);
	    }
	    break;
	 }
      }
   }

   needPlaces = True;
   viewBox->Changed(True);

} // End SetTitles

/*-----------------------------------------------------------------------
 *  Method to find the size of a title field
 */

void
FieldViewC::GetTitleSize(FieldC& title)
{
   if ( fontList ) {
     Dimension width, height;
     WXmString tempStr(title.string, title.tag);
     XmStringExtent(fontList, (XmString)tempStr, &width, &height); 
     title.height = height + itemMarginHt*2;
     title.width  = width  + itemMarginWd*2;
   }

   else {
      int	dir, asc, dsc;
      XCharStruct	size;
      XTextExtents(font, title.string, title.string.size(), &dir, &asc, &dsc,
		   &size);
      title.height = font->ascent + font->descent + itemMarginHt*2;
      title.width  = size.width + itemMarginWd*2;
   }

} // End GetTitleSize

/*-----------------------------------------------------------------------
 *  Method to find the size of all fields for a particular item
 */

void
FieldViewC::GetFieldSizes(ItemDataC *data)
{
   if ( showPixmaps && data->pixmap ) data->itemHt = data->pixmap->ht;
   else				      data->itemHt = 0;

//
// Create fields and get sizes
//
   StringListC&	strings = data->item->FieldList();
   u_int	scount  = strings.size();
   u_int	fcount  = data->fieldList.size();
   int	i;
   for (i=0; i<scount; i++) {

      FieldC	*field;
      if ( i < fcount ) {
	 field = data->fieldList[i];
      } else {
	 field = new FieldC;
	 data->fieldList.append(field);
      }

      StringC	*string = strings[i];

//
// Get text size
//
      if ( fontList ) {

	 char	*tag = data->item->FieldTag(i);
	 field->SetString(*string, tag ? tag : XmFONTLIST_DEFAULT_TAG);

	 Dimension	width;
	 Dimension	height;
	 WXmString	tempStr(field->string, field->tag);
	 XmStringExtent(fontList, (XmString)tempStr, &width, &height); 
	 field->height = height;
	 field->width  = width;
      }

      else {

	 field->string = *string;

	 int		dir, asc, dsc;
	 XCharStruct	size;
	 XTextExtents(font, *string, string->size(), &dir, &asc, &dsc, &size);

	 field->height = font->ascent + font->descent;
	 field->width  = size.width;
      }

   } // End for each field

//
// Loop through fields in column display order and get item height
//
   u_int	ccount = columnList.size();
   for (i=0; i<ccount; i++) {

      int	pos     = *columnDisplayOrder[i];
      ColumnC	*column = columnList[pos];
      if ( !column->visible || pos < 0 || pos >= data->fieldList.size() )
	 continue;

      FieldC	*field  = data->fieldList[pos];
      if ( field->height > data->itemHt ) data->itemHt = field->height;
	 
   } // End for each column

} // End GetFieldSizes

/*-----------------------------------------------------------------------
 *  Method to hide a column
 */

void
FieldViewC::HideColumn(int i)
{
   if ( i < 0 || i >= columnList.size() ) return;

   ColumnC	*column = columnList[i];
   if ( !column->visible ) return;

   column->visible = False;

   needPlaces = True;

   viewBox->Changed(True);

} // End HideColumn

/*-----------------------------------------------------------------------
 *  Method to show a column
 */

void
FieldViewC::ShowColumn(int i)
{
   if ( i < 0 || i >= columnList.size() ) return;

   ColumnC	*column = columnList[i];
   if ( column->visible ) return;

   column->visible = True;

   needPlaces = True;

   viewBox->Changed(True);

} // End ShowColumn


/*-----------------------------------------------------------------------
 *  Method to change the order of the columns.
 */

void
FieldViewC::SetColumnDisplayIndex(int id, int new_index)
{
//
// Make sure the values are valid and changing.
//
   int max_index = columnDisplayOrder.size()-1;
   if ( id < 0 || id > max_index ) return;
   if ( new_index < 0 )         new_index = 0;
   if ( new_index > max_index ) new_index = max_index;

   ColumnC	*column = columnList[new_index];
   int old_index = ColumnDisplayIndex(id);
   if ( old_index == new_index ) return;

//
// Make room for the change and then make the change.
//
   int *nums = columnDisplayOrder.start();
   if ( old_index > new_index ) {
      for ( int i=old_index-1; i>=new_index; i--)
         nums[i+1] = nums[i];
   }
   else {
      for ( int i=old_index+1; i<=new_index; i++)
         nums[i-1] = nums[i];
   }
   nums[new_index] = id;

//
// Redraw if we need to...
//
   if ( column->visible ) {
      needPlaces = True;
      viewBox->Changed(True);
   }

} // End SetColumnDisplayIndex


/*-----------------------------------------------------------------------
 *  Method to set the sort order of the columns
 */

void
FieldViewC::SetColumnSortOrder(IntListC& list)
{
   columnSortOrder = list;

   if ( viewBox->compFunc == (CompareFn)ColumnSortCompare )
      viewBox->Sort();

} // End SetColumnSortOrder

/*-----------------------------------------------------------------------
 *  Method to return the logical order of a particular column.
 */

int
FieldViewC::ColumnIndex(int display_id)
{
   if ( display_id >= 0 && display_id < columnDisplayOrder.size() ) {
      return(*columnDisplayOrder[display_id]);
   }
   return(-1);
}


/*-----------------------------------------------------------------------
 *  Method to return the visual order of a particular column.
 */

int
FieldViewC::ColumnDisplayIndex(int id)
{
   int count = columnDisplayOrder.size();
   if ( id < 0 || id >= count ) return -1;
   for ( int i=0; i<count; i++ ) {
      if ( id == *columnDisplayOrder[i] ) {
	 return(i);
      }
   }
   return(-1);
}

/*-----------------------------------------------------------------------
 *  Method to turn pixmaps off
 */

void
FieldViewC::HidePixmaps()
{
   if ( !showPixmaps ) return;

   showPixmaps = False;

//
// Loop through existing items and invalidate height
//
   unsigned	count = dataDict.size();
   for (int i=0; i<count; i++) {
      ItemDataC	*data = dataDict[i]->val;
      data->itemHt = 0;
   }

   needPlaces = True;

   viewBox->Changed(True);

} // End HidePixmaps

/*-----------------------------------------------------------------------
 *  Method to turn pixmaps on
 */

void
FieldViewC::ShowPixmaps()
{
   if ( showPixmaps ) return;

   showPixmaps = True;

//
// Loop through existing items and create pixmaps
//
   unsigned	count = dataDict.size();
   for (int i=0; i<count; i++) {
      ItemDataC	*data = dataDict[i]->val;
      if ( !data->pixmap ) GetPixmap(*data);
      data->itemHt = 0;
   }

   needPlaces = True;

   viewBox->Changed(True);

} // End ShowPixmaps

/*-----------------------------------------------------------------------
 *  Method to set the title of a column
 */

void
FieldViewC::SetColumnTitle(int i, const char *cs)
{
   if ( i < 0 || i >= columnList.size() ) return;

   ColumnC	*column = columnList[i];
   if ( column->title.string == cs ) return;

   column->title.string = cs;
   if ( viewBox->Realized() ) GetTitleSize(column->title);

   if ( !column->visible ) return;

   needPlaces = True;

   viewBox->Changed(True);

} // End SetColumnTitle

/*-----------------------------------------------------------------------
 *  Method to justify a column
 */

void
FieldViewC::JustifyColumn(int i, ColumnJustifyT jus)
{
   if ( i < 0 || i >= columnList.size() ) return;

   ColumnC	*column = columnList[i];
   if ( column->justify == jus ) return;

   column->justify = jus;
   if ( !column->visible ) return;

   viewBox->Changed(True);

} // End JustifyColumn

/*-----------------------------------------------------------------------
 *  Method to find the character width for a fontList or font
 */

int
FieldViewC::GetCharWidth()
{
  int charWd;

   if ( fontList ) {
     XmFontContext	fontCtxt;
     XmFontListInitFontContext(&fontCtxt, fontList);
     XmFontListEntry	fontEntry = XmFontListNextEntry(fontCtxt);
     XmFontType	font_type;
     XtPointer	font_struct = XmFontListEntryGetFont(fontEntry, &font_type);
     if ( font_type == XmFONT_IS_FONT ) {
       XFontStruct *newf = (XFontStruct*)font_struct;
       charWd = (newf->min_bounds.width + newf->max_bounds.width) / (int)2;
     } else {
       XFontSet newf = (XFontSet)font_struct;
       XFontSetExtents *fontExt = XExtentsOfFontSet(newf);
       charWd = fontExt->max_logical_extent.width;
     }
     XmFontListFreeFontContext(fontCtxt);
   }

   else {
//     charWd = (font->min_bounds.width + font->max_bounds.width) / (int)2;
      charWd = MAX(font->min_bounds.width, font->max_bounds.width);
   }
   return (charWd);
} 
  
/*-----------------------------------------------------------------------
 *  Method to set the maximum width for a column
 */

void
FieldViewC::SetColumnMaxWidth(int i, int wd)
{
   if ( i < 0 || i >= columnList.size() ) return;

   wd *= GetCharWidth();

   SetColumnMaxPixels(i, wd);
}

void
FieldViewC::SetColumnMaxPixels(int i, int wd)
{
   ColumnC	*column = columnList[i];
   if ( column->maxWd == wd ) return;

   column->maxWd = wd;
   if ( column->minWd > column->maxWd ) column->minWd = column->maxWd;

   if ( !column->visible ) return;

   viewBox->Changed(True);

} // End SetColumnMaxPixels

/*-----------------------------------------------------------------------
 *  Method to set the minimum width for a column
 */

void
FieldViewC::SetColumnMinWidth(int i, int wd)
{
   if ( i < 0 || i >= columnList.size() ) return;

   wd *= GetCharWidth();

   SetColumnMinPixels(i, wd);
}

void
FieldViewC::SetColumnMinPixels(int i, int wd)
{
   ColumnC	*column = columnList[i];
   if ( column->minWd == wd ) return;

   column->minWd = wd;
   if ( column->maxWd < column->minWd ) column->maxWd = column->minWd;

   if ( !column->visible ) return;

   viewBox->Changed(True);

} // End SetColumnMinWidth


/*-----------------------------------------------------------------------
 *  Method to return the maximum width for a column
 */

int
FieldViewC::ColumnMaxWidth(int i)
{
   if ( i < 0 || i >= columnList.size() ) return 0;
   ColumnC	*column = columnList[i];

   int	charWd = GetCharWidth();
   int	wd = column->maxWd / charWd;
   if ( charWd * wd != column->maxWd ) wd++;

   return wd;
}

int
FieldViewC::ColumnMaxPixels(int i)
{
   if ( i < 0 || i >= columnList.size() ) return 0;
   ColumnC	*column = columnList[i];

   return column->maxWd;
}

/*-----------------------------------------------------------------------
 *  Method to return the minimum width for a column
 */

int
FieldViewC::ColumnMinWidth(int i)
{
   if ( i < 0 || i >= columnList.size() ) return 0;
   ColumnC	*column = columnList[i];

   int	charWd = GetCharWidth();
   int	wd = column->minWd / charWd;
   if ( charWd * wd != column->minWd ) wd++;

   return wd;
}

int
FieldViewC::ColumnMinPixels(int i)
{
   if ( i < 0 || i >= columnList.size() ) return 0;
   ColumnC	*column = columnList[i];

   return column->minWd;
}

/*-----------------------------------------------------------------------
 *  Create a pixmap for an item if necessary
 */

void
FieldViewC::GetPixmap(ItemDataC& data)
{
   PixmapC	*pm;
   Screen	*scrn = XtScreen(viewBox->ViewDA());
   Window	win   = viewBox->ViewWin();

   switch ( data.item->ImageSource() ) {

      case (VItemC::IMAGE_FILE): {

	 StringC&	name = data.item->SmImageFile();
	 if ( !pixmapFileDict.includes(name) ) {
	    pm = new PixmapC(name, regFgColor, regBgColor, invFgColor,
			     invBgColor, scrn, win);
	    pixmapFileDict.add(name, pm);
	 }

	 data.pixmap = *pixmapFileDict.definitionOf(name);

      } break;

      case (VItemC::XBM_DATA): {

	 XbmT&	xbm = data.item->SmXbmData();
	 void	*bits = xbm.bits;
	 if ( !bits ) {
	    data.pixmap = NULL;
	    break;
	 }

	 if ( !pixmapDataDict.includes(bits) ) {
	    pm = new PixmapC(xbm, regFgColor, regBgColor, invFgColor,
			     invBgColor, scrn, win);
	    pixmapDataDict.add(bits, pm);
	 }

	 data.pixmap = *pixmapDataDict.definitionOf(bits);

      } break;

      case (VItemC::XPM_DATA): {

	 XpmT	xpm = data.item->SmXpmData();
	 if ( !xpm ) {
	    data.pixmap = NULL;
	    break;
	 }

	 if ( !pixmapDataDict.includes(xpm) ) {
	    pm = new PixmapC(xpm, win);
	    pixmapDataDict.add(xpm, pm);
	 }

	 data.pixmap = *pixmapDataDict.definitionOf(xpm);

      } break;

   } // End switch image data source

} // End GetPixmap

/*-----------------------------------------------------------------------
 *  Change the pixmaps for an item
 */

void
FieldViewC::ChangeItemPixmaps(const VItemC& item)
{
   if ( !showPixmaps /*..|| !viewBox->Realized()..*/ ) return;

//
// Look up the item data
//
   ItemDataC	*data = (ItemDataC *)item.ViewData();
   if ( !DataValid(data) ) return;

//
// Remember current size
//
   int	oldWd = 0;
   int	oldHt = 0;
   if ( data->pixmap ) {
      oldWd = data->pixmap->wd;
      oldHt = data->pixmap->ht;
   }

//
// Load the pixmap
//
   GetPixmap(*data);

   if ( data->pixmap ) {
      if ( data->pixmap->wd != oldWd || data->pixmap->ht != oldHt )
	 needPlaces = True;
   } else {
      if ( oldWd > 0 || oldHt > 0 )
	 needPlaces = True;
   }

   viewBox->Changed(True);

} // End ChangeItemPixmaps

/*-----------------------------------------------------------------------
 *  Handle drag over event
 */

void
FieldViewC::HandleDragOver(XmDragProcCallbackStruct *dp)
{
   VItemC	*dropSite = PickItem(dp->x + viewBox->HScrollValue(),
   				     dp->y + viewBox->VScrollValue());
   Boolean	validDrop = (dropSite && dropSite->ValidDropSite());

   if ( dropSite != dropItem ) {

      if ( dropItem ) {
	 if ( dropItem == hlItem ) DrawHighlight(dropItem, hlColor);
	 else			   DrawHighlight(dropItem, regBgColor);
      }

      if ( validDrop ) {
	 DrawHighlight(dropSite, dropColor);
	 dropItem = dropSite;
      } else {
	 dropItem = NULL;
      }

   } // End if this is a new drop site

   if ( validDrop ) dp->dropSiteStatus = XmVALID_DROP_SITE;
   else		    dp->dropSiteStatus = XmINVALID_DROP_SITE;

} // End HandleDragOver

/*-----------------------------------------------------------------------
 *  Handle drop event
 */

void
FieldViewC::DropFinished()
{
//
// Remove drop highlight
//
   if ( dropItem == hlItem ) DrawHighlight(dropItem, hlColor);
   else			     DrawHighlight(dropItem, regBgColor);
   dropItem = NULL;

   if ( dragIcon ) {
      XtDestroyWidget(dragIcon);
      dragIcon = NULL;
   }

} // End DropFinished

/*-----------------------------------------------------------------------
 *  Create an icon for the drag
 */

Widget
FieldViewC::CreateDragIcon(VItemListC &list)
{
// FIXME: doesn't work correctly
#if 0
   ItemDataC	*data = (ItemDataC *)list[0]->ViewData();
   if ( !DataValid(data) ) return NULL;

   if ( !data->pixmap ) GetPixmap(*data);

//
// Create widget for drag icon
//
   WArgList	args;
   args.Reset();
   if ( data->pixmap ) {
      args.Width(data->pixmap->wd);
      args.Height(data->pixmap->ht);
      args.MaxWidth(data->pixmap->wd);
      args.MaxHeight(data->pixmap->ht);
      args.Depth(data->pixmap->depth);
      args.pixmap(data->pixmap->reg);
      if (data->pixmap->mask != XmUNSPECIFIED_PIXMAP &&
	  data->pixmap->mask != None) {
          args.Mask(data->pixmap->mask);
      }
   }
   args.Foreground(regFgColor);
   args.Background(regBgColor);
   dragIcon = XmCreateDragIcon(viewBox->ViewDA(), "dragIcon", ARGS);

   return dragIcon;
#else
   return NULL;
#endif
} // End CreateDragIcon


/*-----------------------------------------------------------------------
 *  Create an icon for the drag
 */

VItemC*
FieldViewC::DropItem()
{
   return dropItem;
}


/*-----------------------------------------------------------------------
 * Set the dimensions of the view form by using the height of a view
 * item and the visItemCount...this does not work to well if the items
 * are variable heights.
 */
void
FieldViewC::SetVisibleItemCount(unsigned cnt)
{
   visItemCount = cnt;
   if ( cnt > 0 ) {
      needSetVisItemCount = True;
      unsigned	num_items = dataDict.size();
      if ( num_items ) {
         ItemDataC *data = dataDict[0]->val;
	 if ( data->itemHt > 0 ) {
            int ht = cnt*(data->bounds.ht+ySpacing) - ySpacing + 1;
	    if ( ht > 0 ) {
               needSetVisItemCount = False;
               XtVaSetValues(viewBox->ViewDA(), XmNheight, (Dimension)ht, NULL);
            }
         }
      }
   }
   else 
      needSetVisItemCount = False;
}

/*-----------------------------------------------------------------------
 * Return the number of currently visible items
 */

int
FieldViewC::VisibleItemCount()
{
   int	viewHt = viewBox->VisRect().ht;
   int	itemHt = 0;

   VItemListC&	visItems = viewBox->VisItems();
   u_int	count    = visItems.size();
   for (int i=0; itemHt==0 && i<count; i++) {
      VItemC	*item = visItems[i];
      ItemDataC	*data = (ItemDataC *)item->ViewData();
      if ( !DataValid(data) ) continue;

      itemHt = data->itemHt;
   }

   if ( itemHt == 0 ) return visItemCount;

   int		extraHt  = (hlThick + itemMarginHt) * 2 + ySpacing;
   itemHt += extraHt;

   int		visCount = viewHt / itemHt;
   if ( visCount * itemHt < viewHt ) visCount++;

   return visCount;

} // End VisibleItemCount

/*-----------------------------------------------------------------------
 * Set the dimensions of the view form by using the height of a view
 * item and the visItemCount...this does not work to well if the items
 * are variable heights.
 */
int
FieldViewC::ColumnSortCompare(const void* a, const void* b)
{
   VItemC* itemA = *(VItemC **)a;
   VItemC* itemB = *(VItemC **)b;
   StringListC& fieldsA = itemA->FieldList();
   StringListC& fieldsB = itemB->FieldList();
   int countA = fieldsA.size();
   int countB = fieldsB.size();

   ItemDataC* itemData = (ItemDataC*)itemA->ViewData();
   if ( itemData ) {
      FieldViewC* This = itemData->view;
      IntListC& sort_order = This->ColumnSortOrder();
      int count = sort_order.size();
      if ( count > 0 ) {
         for ( int i=0; i<count; i++ ) {
            int id = *sort_order[i];
	    Boolean reverse = False;
	    if ( id < 0 ) {
	       id = -id;
               reverse = True;
            }
	    id--;  // 1 based
            if ( id < countA && id < countB ) {
               StringC& str1 = *fieldsA[id];
               StringC& str2 = *fieldsB[id];
               int val = str1.compare(str2);
               if ( val != 0 ) {
		  if ( reverse )
		     return(-val);
                  else
		     return(val);
               }
            }
         }
      }
      if ( This->prevCompareFunction )
         return(This->prevCompareFunction(a,b));
   }

//
// Since we couldn't access anything...use the name.
//
   return(itemA->Name().compare(itemB->Name()));

} // End ColumnSortCompare


/*-----------------------------------------------------------------------
 *
 */
void
FieldViewC::EnablePickSortColumn(Boolean val)
{
   pickSortColumnEnabled = val;
   if ( viewBox->view == this ) {
      _EnablePickSortColumn(val);
   }
}


/*-----------------------------------------------------------------------
 *
 */
void
FieldViewC::_EnablePickSortColumn(Boolean val)
{
   if ( val ) {
      if ( viewBox->compFunc != (CompareFn)FieldViewC::ColumnSortCompare ) {
         prevCompareFunction = viewBox->compFunc;
	 viewBox->SetCompareFunction((CompareFn)FieldViewC::ColumnSortCompare);
      }
   }
   else {
      if ( prevCompareFunction && viewBox->compFunc != prevCompareFunction )
	 viewBox->SetCompareFunction(prevCompareFunction);
   }
}

// EOF FieldViewC.C
