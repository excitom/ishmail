/*
 * $Id: IconViewC.C,v 1.6 2000/08/07 10:58:16 evgeny Exp $
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

#include <config.h>

#include "HalAppC.h"
#include "IconViewC.h"
#include "VItemC.h"
#include "VBoxC.h"
#include "rsrc.h"
#include "WArgList.h"

#include <X11/keysym.h>
#include <values.h>
#include <math.h>

#include <Xm/ScrollBar.h>
#include <Xm/AtomMgr.h>
#include <Xm/DragDrop.h>

#define BLACK	BlackPixel(halApp->display, DefaultScreen(halApp->display))
#define WHITE	WhitePixel(halApp->display, DefaultScreen(halApp->display))

/*-----------------------------------------------------------------------
 *  Method to check data pointer
 */

inline Boolean
IconViewC::DataValid(const IconDataC *data)
{
   return (data && (data->view == this));
}

/*-----------------------------------------------------------------------
 *  Constructor for IconViewC
 */

IconViewC::IconViewC(VBoxC *vb) : ViewC(vb)
{
   maxPmWd	 =
   maxPmHt	 =
   maxLabelWd	 =
   maxLabelHt	 =
   maxItemWd	 =
   maxItemHt	 = 0;
   daRows        =
   daCols        = 0;
   daWd          =
   daHt          = 0;
   focusHere	 = False;
   hlItem	 = NULL;
   flashItem	 = NULL;
   flashTimer	 = (XtIntervalId)NULL;
   dropItem	 = NULL;
   dragIcon      = NULL;
   pickGC        = NULL;
   scrollTimer   = (XtIntervalId)NULL;
   pressItem     = NULL;
   clickCount    = 1;
   lastClickTime = 0;
   needPlaces    = True;

//
// Get resources
//
   Widget	da = viewBox->ViewDA();
   char		*cl = "IconViewC";
   regFgColor   = get_color(cl, da, "foreground", BLACK);
   regBgColor   = get_color(cl, da, "background", WHITE);
   invFgColor   = get_color(cl, da, "selectedForeground", regBgColor);
   invBgColor   = get_color(cl, da, "selectedBackground", regFgColor);
   hlColor      = get_color(cl, da, "highlightColor", regFgColor);
   dropColor    = get_color(cl, da, "validDropColor", hlColor);
   hlThick      = get_int  (cl, da, XmNhighlightThickness);
   xSpacing     = get_int  (cl, da, "horizSpacing");
   ySpacing     = get_int  (cl, da, "vertSpacing");
   labelOffset  = get_int  (cl, da, "labelOffset");
   labelSpacing = get_int  (cl, da, "labelSpacing");
   itemMarginWd = get_int  (cl, da, "itemMarginWidth");
   itemMarginHt = get_int  (cl, da, "itemMarginHeight");

   fontList = viewBox->FontList();
   font     = viewBox->Font();

//
// Get keyboard focus policy
//
   unsigned char	focusPolicy;
   Widget		shell = XtParent(vb->ViewDA());
   while ( !XtIsShell(shell) ) shell = XtParent(shell);
   XtVaGetValues(shell, XmNkeyboardFocusPolicy, &focusPolicy, NULL);

   if ( focusPolicy == XmEXPLICIT ) {

//
// Add event handler for keyboard focus change
//
      XtAddEventHandler(vb->ViewDA(), FocusChangeMask, False,
			(XtEventHandler)HandleFocusChange, (XtPointer)this);

   } else { // XmPOINTER

      XtAddEventHandler(vb->ViewDA(), EnterWindowMask|LeaveWindowMask,
			False, (XtEventHandler)HandleFocusChange,
			(XtPointer)this);
      XtAddEventHandler(vb->ViewDA(),
			PointerMotionMask|PointerMotionHintMask, False,
			(XtEventHandler)HandlePointerMotion,
			(XtPointer)this);
   }

} // End Constructor

/*-----------------------------------------------------------------------
 *  Destructor for IconViewC
 */

IconViewC::~IconViewC()
{
//
// Free pixmaps
//
   unsigned	count = pixmapFileDict.size();
   int	i;
   for (i=0; i<count; i++) {
      PixmapC	*pm = pixmapFileDict[i]->val;
      delete pm;
   }

   count = pixmapDataDict.size();
   for (i=0; i<count; i++) {
      PixmapC	*pm = pixmapDataDict[i]->val;
      delete pm;
   }

//
// Delete data
//
   count = dataDict.size();
   for (i=0; i<count; i++) {
      delete dataDict[i]->val;
      delete dataDict[i];
   }

   if ( halApp->xRunning ) {
      if ( pickGC ) XtReleaseGC(halApp->appShell, pickGC);
   }

} // End Destructor

/*-----------------------------------------------------------------------
 *  Display this view
 */

void
IconViewC::Show()
{
   shown = True;

//
// Add any items that aren't yet added
//
   VItemListC&	items = viewBox->Items();
   unsigned	count = items.size();
   for (int i=0; i<count; i++) {
      VItemC	*item = items[i];

//
// Look first at the view data, then in the dictionary
//
      IconDataC	*data = dataDict.definitionOf(item);
      if ( data ) {
	 item->SetViewData(data);
	 //..data->GetLabelSize(font, labelSpacing);
	 //..UpdateMaxSize(*data);
      } else {
	 _AddItem(*item);
      }

   } // End for each item in view

   needPlaces = True;

//
// Set the GC values
//
   XtGCMask     valMask = GCClipMask | GCFillStyle | GCFunction
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
      vals.font	= font->fid;
   }

   XChangeGC(halApp->display, viewBox->ViewGC(), valMask, &vals);

//
// Create a GC for the rubberband rectangle if necessary
//
   if ( !pickGC ) {
      Pixel	fg, bg;
      XtVaGetValues(viewBox->ViewDA(), XmNforeground, &fg,
      				       XmNbackground, &bg, NULL);

      XGCValues	pickVals;
      pickVals.foreground = fg ^ bg;
      pickVals.function   = GXxor;
      pickGC = XtGetGC(viewBox->ViewDA(), GCForeground|GCFunction, &pickVals);
   }

   Redraw();

} // End Show

/*-----------------------------------------------------------------------
 *  Remove this view
 */

void
IconViewC::Hide()
{
//
// Clear view data
//
   VItemListC&	items = viewBox->Items();
   unsigned	count = items.size();
   for (int i=0; i<count; i++) {
      //cout <<"Clearing data for item " <<hex <<(int)items[i] <<dec NL;
      items[i]->SetViewData(NULL);
   }
   
   shown = False;

} // End Hide

/*-----------------------------------------------------------------------
 *  Calculate positions
 */

void
IconViewC::PlaceItems()
{
   fontList = viewBox->FontList();
   font     = viewBox->Font();

   maxPmWd	 =
   maxPmHt	 =
   maxLabelWd	 =
   maxLabelHt	 =
   maxItemWd	 =
   maxItemHt	 = 0;

//
// Loop through items, and get the max pixmap and label sizes.
//
   VItemListC&	visItems = viewBox->VisItems();
   unsigned	count = visItems.size();
   int	i;
   for (i=0; i<count; i++) {

      VItemC	*item = visItems[i];
      IconDataC	*data = (IconDataC *)item->ViewData();
      if ( !DataValid(data) ) continue;

      if ( fontList ) data->GetLabelSize(fontList, labelSpacing);
      else	      data->GetLabelSize(font,     labelSpacing);

      if ( data->pixmap ) {
	 if ( data->pixmap->wd > maxPmWd ) maxPmWd = data->pixmap->wd;
	 if ( data->pixmap->ht > maxPmHt ) maxPmHt = data->pixmap->ht;
      }
      if ( data->labelWd > maxLabelWd ) maxLabelWd = data->labelWd;
      if ( data->labelHt > maxLabelHt ) maxLabelHt = data->labelHt;

   } // End for each item

//
// Now get max overall size
//
   UpdateMaxItemWd();
   UpdateMaxItemHt();

//
// Get width
//
   daWd = viewBox->DAWidth();

//
// Figure out how many icons will fit across
//
   if ( maxItemWd > 0 || xSpacing > 0 ) {
      daCols = daWd / (int)(maxItemWd + xSpacing);
      if ( daCols < 1 ) daCols = 1;
   } else {
      daCols = 1;
   }

//
// Figure out how many rows are needed
//
   daRows = (count / daCols);
   if ( count % daCols ) daRows++;

//
// Figure out how much space is needed for the rows
//
   daHt = (daRows * maxItemHt) + ((daRows-1) * ySpacing);

//
// Loop through the items and set their bounds
//
   int	row = 0;
   int	col = 0;
   int	x   = 0;
   int	y   = 0;

   for (i=0; i<count; i++) {

      VItemC	*item = visItems[i];
      IconDataC	*data = (IconDataC *)item->ViewData();
      if ( !DataValid(data) ) continue;

      data->bounds.Set(x, y, maxItemWd, maxItemHt);
      data->row = row;
      data->col = col;

//
// Update next position
//
      x += maxItemWd + xSpacing;

//
// Wrap at end of row
//
      if ( ++col >= daCols ) {
	 y += maxItemHt + ySpacing;
	 x = 0;
	 col = 0;
	 row++;
      }

   } // End for each visible item

   needPlaces = False;

} // End PlaceItems

/*-----------------------------------------------------------------------
 *  Return size
 */

void
IconViewC::GetSize(int *wd, int *ht)
{
   *wd = daWd;
   *ht = daHt;
}

/*-----------------------------------------------------------------------
 *  Public method to add a new item to this view box
 */

void
IconViewC::AddItem(VItemC& item)
{
   if ( !viewBox->Realized() ) return;

//
// Add the item
//
   _AddItem(item);

   viewBox->Changed(True);

} // End AddItem

/*-----------------------------------------------------------------------
 *  Add new items to this view
 */

void
IconViewC::AddItems(const VItemListC& list)
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
IconViewC::_AddItem(VItemC& item)
{
//
// Create a new icon data structure and add it to the dictionary
//
   IconDataC	*data = new IconDataC(item, this);
   //cout <<"Created data " <<hex <<(int)data <<" for item "
   //	  <<(int)&item SP (int)data->item <<dec NL;
   dataDict.add(&item, data);
   item.SetViewData(data);

//
// Load the information about the pixmap and name for this item
//
   /*..if ( viewBox->Realized() )..*/ GetPixmap(*data);

   //..data->GetLabelSize(font, labelSpacing);
   //..if ( viewBox->VisItems().includes(&item) ) UpdateMaxSize(*data);

   needPlaces = True;

} // End _AddItem

/*-----------------------------------------------------------------------
 *  Redraw the items in this view
 */

void
IconViewC::Redraw()
{
   Draw(viewBox->VisRect());
}

/*-----------------------------------------------------------------------
 *  Redraw the items in this view
 */

void
IconViewC::Draw(RectC& area)
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
// Loop through item list
//
   for (int i=0; i<count; i++) {

      VItemC	*item = visItems[i];
      IconDataC	*data = (IconDataC *)item->ViewData();
      if ( !DataValid(data) ) continue;

//
// Only draw this icon if it is at least partially visible
//
      if ( sarea.Overlaps(data->bounds) )
	 DrawItem(*item, *data, AS_IS, window);

   } // End for each visible icon

   XCopyArea(halApp->display, window, viewBox->ViewWin(), gc, 0, 0,
	     viewBox->DAWidth(), viewBox->DAHeight(), 0, 0);

} // End Draw

/*-----------------------------------------------------------------------
 *  Public method to redraw the specified item
 */

void
IconViewC::RedrawItem(const VItemC& item)
{
   if ( !viewBox->Realized() ) return;

//
// Get icon data
//
   IconDataC	*data = (IconDataC *)item.ViewData();
   if ( !DataValid(data) ) return;

//
// Redraw if item not clipped out
//
   RectC	sarea = viewBox->VisRect();
   sarea.xmin += viewBox->HScrollValue();
   sarea.xmax += viewBox->HScrollValue();
   sarea.ymin += viewBox->VScrollValue();
   sarea.ymax += viewBox->VScrollValue();

   if ( data && sarea.Overlaps(data->bounds) )
      DrawItem(item, *data, AS_IS);

} // End RedrawItem

void
IconViewC::DrawItem(const VItemC& item, const IconDataC& data)
{
   DrawItem(item, data, AS_IS);
}

void
IconViewC::DrawItem(const VItemC& item, VItemDrawModeT mode)
{
   IconDataC	*data = (IconDataC *)item.ViewData();
   if ( DataValid(data) ) DrawItem(item, *data, mode);
}

/*-----------------------------------------------------------------------
 *  Find the item that contains the given coordinate
 */

VItemC*
IconViewC::PickItem(int x, int y)
{
//
// Set up a rectangle around the point in case we are in the space between
//    two items.
//
   RectC	rect(x-xSpacing, y-ySpacing, xSpacing*2, ySpacing*2);
   VItemListC	list;
   PickItems(rect, list);

   VItemC	*item = NULL;

//
// If there is more than 1 item, pick the closest
//
   if ( list.size() == 1 )

      item = list[0];

   else if ( list.size() > 1 ) {

      int	minDist = MAXINT;
      unsigned	count = list.size();
      for (int i=0; i<count; i++) {

	 VItemC		*li = list[i];
	 IconDataC	*data = (IconDataC *)li->ViewData();

	 if ( data->bounds.Inside(x, y) ) {
	    item = li;
	    i = count;
	 } else {

	    int	midX = data->bounds.xmin + (data->bounds.wd/2);
	    int	midY = data->bounds.ymin + (data->bounds.ht/2);
	    int	dx = x - midX;
	    int	dy = y - midY;
	    dx *= dx;
	    dy *= dy;

	    int	dist = (int)sqrt((double)(dx + dy));
	    if ( dist <= minDist ) {
	       minDist = dist;
	       item = li;
	    }
	 }

      } // End for each picked item

   } // End if more than one item picked

   return item;

} // End PickItem

/*-----------------------------------------------------------------------
 *  Return the items in the pick rect
 */

void
IconViewC::PickItems(RectC &pickRect, VItemListC& list)
{
   VItemListC&	visItems = viewBox->VisItems();
   unsigned	count = visItems.size();
   for (int i=0; i<count; i++) {

      VItemC	*item = visItems[i];
      IconDataC *data = (IconDataC *)item->ViewData();
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
IconViewC::ChangeItemVis(const VItemC&)
{
   needPlaces = True;
   viewBox->Changed(True);

} // End ChangeItemVis

/*-----------------------------------------------------------------------
 *  Change the visibility of unknown items
 */

void
IconViewC::ChangeItemVis()
{
   needPlaces = True;

   viewBox->Changed(True);

} // End ChangeItemVis

/*-----------------------------------------------------------------------
 *  Flash an item
 */

void
IconViewC::FlashItem(const VItemC *item)
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
IconViewC::ScrollToItem(const VItemC& item)
{
   IconDataC	*data = (IconDataC *)item.ViewData();
   if ( DataValid(data) ) ScrollToItem(data);
}

void
IconViewC::ScrollToItem(const IconDataC *data)
{
   if ( !data ) return;

   int	ymax = viewBox->VisRect().ymax + viewBox->VScrollValue();
   int	ymin = viewBox->VisRect().ymin + viewBox->VScrollValue();

   if ( data->bounds.ymax > ymax || viewBox->VisRect().ht < maxItemHt ) {

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
IconViewC::FlashProc(IconViewC *This, XtIntervalId*)
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
   else                 This->DrawItem(*This->flashItem, INVERT);
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
 *  Draw a highlight border around an item using the given color
 */

void
IconViewC::DrawHighlight(const VItemC *item, Pixel color)
{
   if ( !item ) return;

   IconDataC	*data = (IconDataC *)item->ViewData();
   if ( DataValid(data) ) DrawHighlight(data, color);
}

void
IconViewC::DrawHighlight(const IconDataC *data, Pixel color, Drawable drawto)
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
   rect->y      = data->bounds.ymin - yoff;
   rect->width  = hlThick;
   rect->height = data->bounds.ht;

   rect++;
   rect->x      = data->bounds.xmax - hlThick + 1 - xoff;
   rect->y      = data->bounds.ymin - yoff;
   rect->width  = hlThick;
   rect->height = data->bounds.ht;

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

   XFillRectangles(halApp->display, dst1, gc, rects, 4);
   if ( dst2 )
      XFillRectangles(halApp->display, dst2, gc, rects, 4);

} // End DrawHighlight

/*-----------------------------------------------------------------------
 *  Handle a keyboard focus change event in the area
 */

void
IconViewC::HandleFocusChange(Widget, IconViewC *This, XEvent *ev, Boolean *)
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

      default:
	 break;
   }

} // End HandleFocusChange

/*-----------------------------------------------------------------------
 *  Handle an input event
 */

void
IconViewC::HandleKeyPress(XKeyEvent *ev)
{
   if ( !focusHere || !hlItem ) return;

   IconDataC	*data = (IconDataC *)hlItem->ViewData();
   if ( !DataValid(data) ) return;

   KeySym	keysym;
   Modifiers	state;
   XtTranslateKey(halApp->display, ev->keycode, ev->state, &state, &keysym);

   VItemListC&	visItems = viewBox->VisItems();
   unsigned	size = visItems.size();
   VItemC	*item;

   switch (keysym) {

      case (XK_Home):
	 HighlightItem(visItems[0]);
	 break;

      case (XK_End):
	 HighlightItem(visItems[size-1]);
	 break;

      case (XK_Left):
	 if ( ev->state & ControlMask ) {
	    item = visItems[data->row * daCols];
	    HighlightItem(item);
	 } else if ( data->col > 0 ) {
	    item = visItems[data->row * daCols + data->col - 1];
	    HighlightItem(item);
	 }
	 break;

      case (XK_Right):
	 if ( ev->state & ControlMask ) {
	    int	index = (data->row+1) * daCols - 1;
	    if ( index >= size ) index = size-1;
	    HighlightItem(visItems[index]);
	 } else if ( data->col < (daCols-1) ) {
	    int	index = data->row * daCols + data->col + 1;
	    if ( index < size ) {
	       HighlightItem(visItems[index]);
	    }
	 }
	 break;

      case (XK_Up):
	 if ( ev->state & ControlMask ) {
	    item = visItems[data->col];
	    HighlightItem(item);
	 } else if ( data->row > 0 ) {
	    item = visItems[(data->row-1) * daCols + data->col];
	    HighlightItem(item);
	 }
	 break;

      case (XK_Down):
	 if ( ev->state & ControlMask ) {
	    int	index = (daRows-1) * daCols + data->col;
	    if ( index >= size ) index -= daCols;
	    HighlightItem(visItems[index]);
	 } else if ( data->row < (daRows-1) ) {
	    int	index = (data->row+1) * daCols + data->col;
	    if ( index < size ) {
	       HighlightItem(visItems[index]);
	    }
	 }
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
IconViewC::HandleButton1Press(XButtonEvent *be)
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
// Initialize the rubberband rectangle
//
   pickRect.Set(pickX, pickY, 1, 1);

//
// Pick and highlight press item
//
   pickList.removeAll();
   pressItem = PickItem(pickX, pickY);

   pickDrawMode = INVERT;
   pickState = be->state;
   if ( (pickState & ControlMask) == ControlMask ) {
      if ( pressItem && viewBox->SelItems().includes(pressItem) )
	 pickDrawMode = NORMAL;
   }

   else if ( (pickState & ShiftMask) == ShiftMask ) {
      ;
   }

   else if ( clickCount == 1 ) {
      viewBox->DeselectAllItems();
      if ( pressItem ) DrawItem(*pressItem, pickDrawMode);
   }

   if ( pressItem ) {
//      DrawItem(*pressItem, pickDrawMode);
      pickList.add(pressItem);
      HighlightItem(pressItem);
   }

   XDrawRectangle(halApp->display, viewBox->ViewWin(), pickGC,
   		  pickRect.xmin - viewBox->HScrollValue(),
		  pickRect.ymin - viewBox->VScrollValue(),
		  pickRect.wd, pickRect.ht);

} // End HandleButton1Press

/*-----------------------------------------------------------------------
 *  Handle release of mouse button 1
 */

void
IconViewC::HandleButton1Release(XButtonEvent *be)
{
   if ( scrollTimer ) XtRemoveTimeOut(scrollTimer);

//
// Clear the current rectangle
//
   XDrawRectangle(halApp->display, viewBox->ViewWin(), pickGC,
   		  pickRect.xmin - viewBox->HScrollValue(),
		  pickRect.ymin - viewBox->VScrollValue(),
		  pickRect.wd, pickRect.ht);

//
// Get the list of items in the rectangle
//
   PickItems(pickRect, pickList);

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
IconViewC::HandleSingleClick()
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
IconViewC::HandleDoubleClick(XButtonEvent *be)
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
IconViewC::HandleButton1Motion(XMotionEvent *ev)
{
//
// Clear the current rectangle
//
   XDrawRectangle(halApp->display, viewBox->ViewWin(), pickGC,
		  pickRect.xmin - viewBox->HScrollValue(),
		  pickRect.ymin - viewBox->VScrollValue(),
		  pickRect.wd, pickRect.ht);

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

      scrollEvent = *ev;

//
// Start scrolling if we're not already
//
      if ( !scrollTimer ) {

//                               
// Call the action proc to scroll
//                                                         
         char   *parms = "0";                              
	 scrollAction =
            (char *) (off_top ? "IncrementUpOrLeft" : "IncrementDownOrRight");
	 XtCallActionProc(viewBox->VScrollBar(), scrollAction,
			  (XEvent*)&scrollEvent, &parms, 1);

//
// Start automatic scrolling
//
	 scrollTimer  = XtAppAddTimeOut(halApp->context, 250,
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
   UpdatePickRect(x, y);

} // End HandleButton1Motion

/*-----------------------------------------------------------------------
 *  Handle automatic scrolling
 */

void
IconViewC::HandleAutoScroll(IconViewC *This, XtIntervalId*)
{
//
// Clear the current rectangle
//
   XDrawRectangle(halApp->display, This->viewBox->ViewWin(), This->pickGC,
		  This->pickRect.xmin - This->viewBox->HScrollValue(),
		  This->pickRect.ymin - This->viewBox->VScrollValue(),
		  This->pickRect.wd, This->pickRect.ht);

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
      IconDataC	*data = (IconDataC *)item->ViewData();

      drawBounds = data->bounds;
      drawBounds.ht   += This->ySpacing;
      drawBounds.ymax += This->ySpacing;

//
// Redraw if item not clipped out
//
      if ( sarea.Overlaps(drawBounds) )
	 This->DrawItem(*item, *data, This->pickDrawMode);
   }

//
// Update the rectangle
//
   int	x = This->scrollEvent.x + This->viewBox->HScrollValue();
   int	y = This->scrollEvent.y + This->viewBox->VScrollValue();
   This->UpdatePickRect(x, y);

//
// Repeat the cycle
//
   This->scrollTimer = XtAppAddTimeOut(halApp->context, 250,
				       (XtTimerCallbackProc)HandleAutoScroll,
				       (XtPointer)This);

} // End HandleAutoScroll

/*-----------------------------------------------------------------------
 *  Redraw the pick rectangle
 */

void
IconViewC::UpdatePickRect(int x, int y)
{
//
// Update the pick rectangle
//
   if ( pickX > x ) {
      pickRect.xmin = x;
      pickRect.xmax = pickX;
   } else {
      pickRect.xmin = pickX;
      pickRect.xmax = x;
   }

   if ( pickY > y ) {
      pickRect.ymin = y;
      pickRect.ymax = pickY;
   } else {
      pickRect.ymin = pickY;
      pickRect.ymax = y;
   }

   pickRect.wd = pickRect.xmax - pickRect.xmin + 1;
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

//
// Draw the new rectangle
//
   XDrawRectangle(halApp->display, viewBox->ViewWin(), pickGC,
		  pickRect.xmin - viewBox->HScrollValue(),
		  pickRect.ymin - viewBox->VScrollValue(),
		  pickRect.wd, pickRect.ht);

} // End UpdatePickRect

/*-----------------------------------------------------------------------
 *  Highlight an item
 */

void
IconViewC::HighlightItem(const VItemC *item)
{
   if ( item == hlItem ) return;

//
// Remove current highlight
//
   if ( hlItem ) {
      DrawHighlight(hlItem, regBgColor);
   }
   
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
IconViewC::HandlePointerMotion(Widget, IconViewC *This, XMotionEvent*, Boolean*)
{
   if ( !This->shown || !This->focusHere ) return;

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
   if ( This->hlItem ) {
      This->DrawHighlight(This->hlItem, This->regBgColor);
   }
   
   This->hlItem = item;

//
//  Highlight the new item
//
   if ( This->hlItem ) {
      This->DrawHighlight(This->hlItem, This->hlColor);
   }

} // End HandlePointerMotion

/*-----------------------------------------------------------------------
 *  Change the label for an item
 */

void
IconViewC::ChangeItemLabel(const VItemC& item)
{
//
// Look up the icon data
//
   IconDataC	*data = (IconDataC *)item.ViewData();
   if ( !DataValid(data) ) return;

//
// Save current sizes
//
   int	oldLabelWd = data->labelWd;
   int	oldLabelHt = data->labelHt;

//
// Calculate new sizes
//
   if ( fontList ) data->GetLabelSize(fontList, labelSpacing);
   else		   data->GetLabelSize(font,     labelSpacing);

   if ( data->labelWd != oldLabelWd ||
        data->labelHt != oldLabelHt ) {

      needPlaces = True;

   }
   
   viewBox->Changed(True);

} // End ChangeItemLabel

/*-----------------------------------------------------------------------
 *  Change the pixmaps for an item
 */

void
IconViewC::ChangeItemPixmaps(const VItemC& item)
{
   //..if ( !viewBox->Realized() ) return;

//
// Look up the icon data
//
   IconDataC	*data = (IconDataC *)item.ViewData();
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
 *  Public method to remove an item from this view
 */

void
IconViewC::RemoveItem(VItemC& item)
{
//
// Get icon data structure
//
   IconDataC	*data = (IconDataC *)item.ViewData();
   if ( !DataValid(data) ) return;

   dataDict.remove(&item);
   item.SetViewData(NULL);

   //cout <<"Deleting data " <<hex <<(int)data <<" for item "
   //	<<(int)&item SP (int)data->item <<dec NL;

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
IconViewC::RemoveItems(const VItemListC& list)
{
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
      IconDataC	*data = (IconDataC *)item->ViewData();
      if ( !DataValid(data) ) continue;

      dataDict.remove(item);
      item->SetViewData(NULL);

      //cout <<"Deleting data " <<hex <<(int)data <<" for item "
      //     <<(int)item SP (int)data->item <<dec NL;

      if ( viewBox->VisItems().includes(item) ) needPlaces = True;

//
// Delete icon data structure
//
      delete data;
      if ( hlItem == item ) hlItem = NULL;

   } // End for each item to be deleted

   viewBox->Changed(True);

} // End RemoveItems

/*-----------------------------------------------------------------------
 *  Handle drag over event
 */

void
IconViewC::HandleDragOver(XmDragProcCallbackStruct *dp)
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
IconViewC::DropFinished()
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
IconViewC::CreateDragIcon(VItemListC &list)
{
// FIXME: doesn't work correctly
#if 0
   IconDataC	*data = (IconDataC *)list[0]->ViewData();
   if ( !DataValid(data) ) return NULL;

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
IconViewC::DropItem()
{
   return dropItem;
}
