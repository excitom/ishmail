/*
 * $Id: IconC.C,v 1.2 2000/05/07 12:26:10 fnevgeny Exp $
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

#include "IconC.h"
#include "rsrc.h"
#include "HalAppC.h"

#define BLACK	BlackPixel(halApp->display, DefaultScreen(halApp->display))
#define WHITE	WhitePixel(halApp->display, DefaultScreen(halApp->display))

extern int	debug1, debug2;

/*-----------------------------------------------------------------------
 *  Constructor for IconC
 */

IconC::IconC(Widget par, VItemC *i, ArgList argv, Cardinal argc) : data(*i)
{
   focusHere   = False;
   gc          = NULL;
   window      = (Window)NULL;
   selected    = False;
   realized    = False;
   labelPos    = LABEL_BELOW;
   data.pixmap = &pixmap;
   selectionOk = True;
   openOk      = True;
   goodFont    = True;
   focusCalls.AllowDuplicates(TRUE);

//
// Create a drawing area for this view
//
   iconDA = XmCreateDrawingArea(par, "iconDA", argv, argc);
   XtVaSetValues(iconDA, XmNresizePolicy, XmRESIZE_NONE, NULL);

//
// Get resources
//
   char	*cl = "IconC";
   regFgColor   = get_color(cl, iconDA, "foreground", BLACK);
   regBgColor   = get_color(cl, iconDA, "background", WHITE);
   invFgColor   = get_color(cl, iconDA, "selectedForeground", regBgColor);
   invBgColor   = get_color(cl, iconDA, "selectedBackground", regFgColor);
   hlColor      = get_color(cl, iconDA, "highlightColor", regFgColor);
   hlThick      = get_int  (cl, iconDA, XmNhighlightThickness, 1);
   labelOffset  = get_int  (cl, iconDA, "labelOffset", 5);
   labelSpacing = get_int  (cl, iconDA, "labelSpacing");
   itemMarginWd = get_int  (cl, iconDA, "itemMarginWidth", 3);
   itemMarginHt = get_int  (cl, iconDA, "itemMarginHeight", 3);
   daMarginWd   = get_int  (cl, iconDA, "marginWidth", 0);
   daMarginHt   = get_int  (cl, iconDA, "marginHeight", 0);

//
// Load font
//
   StringC	fontName = get_string(cl, iconDA, XmNfont, "fixed");
   font = XLoadQueryFont(halApp->display, fontName);
   if ( !font ) {
      font = halApp->font;
      goodFont = False;
   }

//
// Create graphics context for icon drawing
//
   XtGCMask  fixMask = GCClipMask | GCFillStyle | GCFunction
		     | GCGraphicsExposures | GCLineStyle | GCLineWidth
		     | GCPlaneMask;
   XtGCMask  modMask = GCForeground;
   XtGCMask  naMask  = GCArcMode | GCBackground | GCCapStyle | GCClipXOrigin
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

   if ( goodFont ) {
      fixMask |= GCFont;
      fixVals.font = font->fid;
   }

   gc = XtAllocateGC(iconDA, 0, fixMask, &fixVals, modMask, naMask);

//
// Get keyboard focus policy
//
   Widget	shell = XtParent(iconDA);
   while ( !XtIsShell(shell) ) shell = XtParent(shell);

   unsigned char	focusPolicy;
   XtVaGetValues(shell, XmNkeyboardFocusPolicy, &focusPolicy, NULL);

   if ( focusPolicy == XmEXPLICIT ) {

//
// Add event handler for keyboard focus change
//
      XtAddEventHandler(iconDA, FocusChangeMask, False,
			(XtEventHandler)HandleFocusChange, (XtPointer)this);

   } else { // XmPOINTER

      XtAddEventHandler(iconDA, EnterWindowMask|LeaveWindowMask, False,
			(XtEventHandler)HandleFocusChange, (XtPointer)this);
   }

   XtAddEventHandler(iconDA, StructureNotifyMask, False,
		     (XtEventHandler)HandleMapChange, (XtPointer)this);

} // End IconC Constructor

/*-----------------------------------------------------------------------
 *  Destructor for IconC
 */

IconC::~IconC()
{
   DeleteCallbacks(focusCalls);

   if ( !halApp->xRunning ) return;

   pixmap.Reset();

   XtReleaseGC(iconDA, gc);
   if ( goodFont ) XFreeFont(halApp->display, font);

   XtDestroyWidget(iconDA);

} // End IconC Destructor

/*-----------------------------------------------------------------------
 *  Called on initial display
 */

void
IconC::HandleMapChange(Widget, IconC *This, XEvent *ev, Boolean*)
{
   if ( ev->type != MapNotify ) return;

   This->window = XtWindow(This->iconDA);

   XtRemoveEventHandler(This->iconDA, StructureNotifyMask, False,
		        (XtEventHandler)HandleMapChange, (XtPointer)This);

#if 0
// If you use backing store, the window manager refresh function doesn't work

//
// Turn on backing store
//
   unsigned long	winMask = CWBackingStore;
   XSetWindowAttributes	winVals;
   winVals.backing_store = WhenMapped;
   XChangeWindowAttributes(halApp->display, This->window, winMask, &winVals);
#endif

//
// Build the icon
//
   This->GetPixmap();
   This->data.GetLabelSize(This->font, This->labelSpacing);
   This->GetIconSize();

//
// Add callbacks
//
   XtAddCallback(This->iconDA, XmNresizeCallback, (XtCallbackProc)HandleResize,
		 (XtPointer)This);
   XtAddCallback(This->iconDA, XmNexposeCallback, (XtCallbackProc)HandleExpose,
		 (XtPointer)This);
   XtAddCallback(This->iconDA, XmNinputCallback,  (XtCallbackProc)HandleInput,
		 (XtPointer)This);

   This->realized = True;
   This->Draw();

} // End HandleMapChange

/*-----------------------------------------------------------------------
 *  Create a pixmap for this icon
 */

void
IconC::GetPixmap()
{
   //if ( !window ) return;
   Window	win = RootWindowOfScreen(XtScreen(iconDA));

//
// Load new data
//
   switch ( data.item->ImageSource() ) {

      case (VItemC::IMAGE_FILE):
	 pixmap.Set(data.item->LgImageFile(), regFgColor, regBgColor,
		    invFgColor, invBgColor, XtScreen(iconDA), win);
	 break;

      case (VItemC::XBM_DATA):
	 pixmap.Set(data.item->LgXbmData(), regFgColor, regBgColor,
		    invFgColor, invBgColor, XtScreen(iconDA), win);
	 break;

      case (VItemC::XPM_DATA):
	 pixmap.Set(data.item->LgXpmData(), win);
	 break;

   } // End switch image data source

} // End GetPixmap

/*-----------------------------------------------------------------------
 * Calculate size of icon
 */

void
IconC::GetIconSize()
{
//
// Calculate the preferred size of the drawing area
//
   int	wd, ht;
   switch (labelPos) {

      case (LABEL_ABOVE):
      case (LABEL_BELOW):
	 wd = MAX(pixmap.wd, data.labelWd) + itemMarginWd*2;
	 ht = pixmap.ht + labelOffset + data.labelHt + itemMarginHt*2;
	 daWd = wd + (hlThick + daMarginWd) * 2;
	 daHt = ht + (hlThick + daMarginHt) * 2;
         break;

      case (LABEL_LEFT):
      case (LABEL_RIGHT):
	 wd = pixmap.wd + labelOffset + data.labelWd + itemMarginWd*2;
	 ht = MAX(pixmap.ht, data.labelHt) + itemMarginHt*2;
	 daWd = wd + (hlThick + daMarginWd) * 2;
	 daHt = ht + (hlThick + daMarginHt) * 2;
         break;
   }

   int	x = (int)(daWd - wd) / (int)2;
   int	y = (int)(daHt - ht) / (int)2;
   data.bounds.Set(x, y, wd, ht);

   wd += hlThick * 2;
   ht += hlThick * 2;
   x -= hlThick;
   y -= hlThick;
   hlRect.Set(x, y, wd, ht);

//
// Set the size.  This might cause a resize event
//
   XtVaSetValues(iconDA, XmNwidth, daWd, XmNheight, daHt, NULL);

} // End IconC GetIconSize

/*-----------------------------------------------------------------------
 *  Handle an exposure event
 */

void
IconC::HandleExpose(Widget, IconC *This, XmDrawingAreaCallbackStruct*)
{
   This->Draw();
}

/*-----------------------------------------------------------------------
 *  Handle a resize event
 */

void
IconC::HandleResize(Widget, IconC *This, XmDrawingAreaCallbackStruct*)
{
//
// Read the new size
//
   XtVaGetValues(This->iconDA, XmNwidth, &This->daWd, XmNheight, &This->daHt,
		 NULL);

//
// Calculate the various icon rectangles
//
   int	wd, ht;
   switch (This->labelPos) {

      case (LABEL_ABOVE):
      case (LABEL_BELOW):
	 wd = MAX(This->pixmap.wd, This->data.labelWd)
	    + This->itemMarginWd*2;
	 ht = This->pixmap.ht + This->labelOffset + This->data.labelHt
	    + This->itemMarginHt*2;
         break;

      case (LABEL_LEFT):
      case (LABEL_RIGHT):
	 wd = This->pixmap.wd + This->labelOffset + This->data.labelWd
	    + This->itemMarginWd*2;
	 ht = MAX(This->pixmap.ht, This->data.labelHt)
	    + This->itemMarginHt*2;
         break;
   }

   int	x = (int)(This->daWd - wd) / (int)2;
   int	y = (int)(This->daHt - ht) / (int)2;
   This->data.bounds.Set(x, y, wd, ht);

   wd += This->hlThick * 2;
   ht += This->hlThick * 2;
   x -= This->hlThick;
   y -= This->hlThick;
   This->hlRect.Set(x, y, wd, ht);

//
// Redraw the icon
//
   This->Draw();

} // End IconC DoResize

/*-----------------------------------------------------------------------
 *  Public method to redraw the item
 */

void
IconC::Draw()
{
   if ( !realized ) return;

//
// Make sure the size is what we think it is.
//
   Dimension	chkWd, chkHt;
   XtVaGetValues(iconDA, XmNwidth, &chkWd, XmNheight, &chkHt, NULL);

   if ( chkWd != daWd || chkHt != daHt ) {
      XtVaSetValues(iconDA, XmNwidth, daWd, XmNheight, daHt, NULL);
      return;
   }

//
// Clear drawing area
//
   XSetForeground(halApp->display, gc, regBgColor);
   XFillRectangle(halApp->display, window, gc, 0, 0, daWd, daHt);

//
// Draw icon background
//
   Pixel	bg = selected ? invBgColor : regBgColor;
   XSetForeground(halApp->display, gc, bg);
   XFillRectangle(halApp->display, window, gc, data.bounds.xmin,
		  data.bounds.ymin, data.bounds.wd, data.bounds.ht);

//
// Draw pixmap
//
   int	x, y;
   switch (labelPos) {

      case (LABEL_BELOW):
	 x = (daWd - pixmap.wd) / 2;
	 y = data.bounds.ymin + itemMarginHt;
         break;

      case (LABEL_ABOVE):
	 x = (daWd - pixmap.wd) / 2;
	 y = data.bounds.ymin + itemMarginHt + data.labelHt + labelOffset;
         break;

      case (LABEL_LEFT):
	 x = data.bounds.xmin + itemMarginWd + data.labelWd + labelOffset;
	 y = (daHt - pixmap.ht) / 2;
         break;

      case (LABEL_RIGHT):
	 x = data.bounds.xmin + itemMarginWd;
	 y = (daHt - pixmap.ht) / 2;
         break;

   } // End switch label position

   if ( pixmap.mask ) {
      XSetClipMask(halApp->display, gc, pixmap.mask);
      XSetClipOrigin(halApp->display, gc, x, y);
   }

   Pixmap	src = selected ? pixmap.inv : pixmap.reg;
   if ( src )
      XCopyArea(halApp->display, src, window, gc, /*srcx*/0, /*srcy*/0,
		pixmap.wd, pixmap.ht, /*dstx*/x, /*dsty*/y);

   if ( pixmap.mask ) {
      XSetClipMask(halApp->display, gc, None);
      XSetClipOrigin(halApp->display, gc, 0, 0);
   }

//
// Draw name string
//
   Pixel	fg = selected ? invFgColor : regFgColor;
   XSetForeground(halApp->display, gc, fg);
   DrawLabel();


//
// Draw highlight if necessary
//
   if ( focusHere ) DrawHighlight(hlColor);

} // End IconC Draw

/*-----------------------------------------------------------------------
 *  Method to draw the label
 */

void
IconC::DrawLabel()
{
   if ( !realized ) return;

   int	lx, y;
   switch (labelPos) {

      case (LABEL_BELOW):
	 lx = 0;
	 y = data.bounds.ymin + itemMarginHt + pixmap.ht + labelOffset;
	 break;

      case (LABEL_ABOVE):
	 lx = 0;
	 y = data.bounds.ymin + itemMarginHt;
	 break;

      case (LABEL_LEFT):
	 lx = data.bounds.xmin + itemMarginWd;
	 y = (int)(daHt - data.labelHt) / (int)2;
	 break;

      case (LABEL_RIGHT):
	 lx = data.bounds.xmin + itemMarginWd + pixmap.wd + labelOffset;
	 y = (int)(daHt - data.labelHt) / (int)2;
	 break;
   }

//
// Loop through label components
//
   unsigned	count = data.labelList.size();
   for (int i=0; i<count; i++) {

      LabelDataC	*ldata = data.labelList[i];

//
// Center component in icon
//
      int	x;
      switch (labelPos) {
	 case (LABEL_BELOW):
	 case (LABEL_ABOVE):
	    x = lx + (int)(daWd - ldata->width) / (int)2;
	    break;
	 case (LABEL_LEFT):
	 case (LABEL_RIGHT):
	    x = lx;
	    break;
      }

      y += font->ascent;

      XDrawString(halApp->display, window, gc, x, y, ldata->string,
		  ldata->string.size());

//
// Move to top of next label
//
      y += font->descent + labelSpacing;

   } // End for each label component

} // End IconC DrawLabel

/*-----------------------------------------------------------------------
 *  Draw a highlight border around the item
 */

void
IconC::DrawHighlight(Pixel color)
{
   if ( !realized ) return;

   XSetForeground(halApp->display, gc, color);

   XRectangle	rects[4];
   XRectangle	*rect = rects;

   rect->x      = hlRect.xmin;
   rect->y      = hlRect.ymin;
   rect->width  = hlRect.wd;
   rect->height = hlThick;

   rect++;
   rect->x      = hlRect.xmin;
   rect->y      = hlRect.ymin;
   rect->width  = hlThick;
   rect->height = hlRect.ht;

   rect++;
   rect->x      = hlRect.xmax - hlThick + 1;
   rect->y      = hlRect.ymin;
   rect->width  = hlThick;
   rect->height = hlRect.ht;

   rect++;
   rect->x      = hlRect.xmin;
   rect->y      = hlRect.ymax - hlThick + 1;
   rect->width  = hlRect.wd;
   rect->height = hlThick;

   XFillRectangles(halApp->display, window, gc, rects, 4);

} // End IconC DrawHighlight


/*-----------------------------------------------------------------------
 *  Handle a keyboard focus change event in the area
 */

void
IconC::HandleFocusChange(Widget, IconC *This, XEvent *ev, Boolean *)
{
   switch (ev->type) {

      case (FocusIn):
      case (EnterNotify):
	 This->focusHere = True;
	 This->DrawHighlight(This->hlColor);
	 break;

      case (FocusOut):
      case (LeaveNotify):
	 This->focusHere = False;
	 This->DrawHighlight(This->regBgColor);
	 break;

      default:
	 break;
   }

   This->CallFocusChangeCallbacks();

} // End IconC HandleFocusChange

/*-----------------------------------------------------------------------
 *  Change the label for an item
 */

void
IconC::SetLabel(const StringC& str)
{
//
// Clear drawing area
//
   if ( realized ) {
      XSetForeground(halApp->display, gc, regBgColor);
      XFillRectangle(halApp->display, window, gc, 0, 0, daWd, daHt);
   }

//
// Calculate new sizes
//
   data.item->Label(str);
   data.GetLabelSize(font, labelSpacing);
   GetIconSize();
   Draw();

} // End IconC SetLabel

/*-----------------------------------------------------------------------
 *  Change the label position for an item
 */

void
IconC::SetLabelPosition(LabelPositionT pos)
{
//
// Clear drawing area
//
   if ( realized ) {
      XSetForeground(halApp->display, gc, regBgColor);
      XFillRectangle(halApp->display, window, gc, 0, 0, daWd, daHt);
   }

   labelPos = pos;
   Draw();
}

/*-----------------------------------------------------------------------
 *  Change the pixmap
 */

void
IconC::SetPixmap(const char *cs)
{
//
// Clear drawing area
//
   if ( realized ) {
      XSetForeground(halApp->display, gc, regBgColor);
      XFillRectangle(halApp->display, window, gc, 0, 0, daWd, daHt);
   }

   data.item->SetPixmaps(cs, NULL);
   GetPixmap();
   GetIconSize();
   Draw();
}

void
IconC::SetPixmap(const XbmT *xbm)
{
//
// Clear drawing area
//
   if ( realized ) {
      XSetForeground(halApp->display, gc, regBgColor);
      XFillRectangle(halApp->display, window, gc, 0, 0, daWd, daHt);
   }

   data.item->SetPixmaps(xbm, NULL);
   GetPixmap();
   GetIconSize();
   Draw();
}

void
IconC::SetPixmap(const XpmT xpm)
{
//
// Clear drawing area
//
   if ( realized ) {
      XSetForeground(halApp->display, gc, regBgColor);
      XFillRectangle(halApp->display, window, gc, 0, 0, daWd, daHt);
   }

   data.item->SetPixmaps(xpm, NULL);
   GetPixmap();
   GetIconSize();
   Draw();
}

/*-----------------------------------------------------------------------
 *  Set the drawing colors
 */

void
IconC::SetColors(Pixel bg, Pixel fg)
{
   regBgColor = bg;
   regFgColor = fg;
   XtVaSetValues(iconDA, XmNbackground, bg, NULL);

   GetPixmap();
   GetIconSize();
   if ( !selected ) Draw();
}

/*-----------------------------------------------------------------------
 *  Set the selection colors
 */

void
IconC::SetSelectColors(Pixel bg, Pixel fg)
{
   invBgColor = bg;
   invFgColor = fg;
   GetPixmap();
   GetIconSize();
   if ( selected ) Draw();
}

/*-----------------------------------------------------------------------
 *  Select the icon
 */

void
IconC::Select(Boolean notify)
{
   if ( selected ) return;

   selected = True;
   Draw();
   if ( notify ) data.item->CallSelectCallbacks();
}

/*-----------------------------------------------------------------------
 *  Deselect the icon
 */

void
IconC::Deselect(Boolean notify)
{
   if ( !selected ) return;

   selected = False;
   Draw();
   if ( notify ) data.item->CallDeselectCallbacks();
}

/*-----------------------------------------------------------------------
 *  Toggle the icon
 */

void
IconC::Toggle(Boolean notify)
{
   if ( selected ) Deselect(notify);
   else		   Select(notify);
}

/*-----------------------------------------------------------------------
 *  Handle an input event
 */

void
IconC::HandleInput(Widget, IconC *This, XmDrawingAreaCallbackStruct *da)
{
   XButtonEvent   *be = (XButtonEvent *)da->event;
   switch ( da->event->type ) {

      case (ButtonPress):

         if ( be->button == Button3 && This->data.item->HasMenu() )
	    This->data.item->PostMenu(be);
	 break;

      case (ButtonRelease):

         if ( be->button == Button1 ) {
	    static Time	lastTime = 0;
	    if ( be->time - lastTime > XtGetMultiClickTime(halApp->display) )
	       This->HandleSingleClick(be);
	    else if ( This->openOk )
	       This->data.item->CallOpenCallbacks();

	    lastTime = be->time;
         }
	 break;

   } // End switch event type

} // End IconViewC HandleInput

/*-----------------------------------------------------------------------
 *  Handle a button release event in the icon
 */

void
IconC::HandleSingleClick(XButtonEvent *be)
{
   if ( !selectionOk ) return;

   if ( be->state & (ShiftMask|ControlMask) ) Toggle();
   else					      Select();
}

/*-----------------------------------------------------------------------
 *  Methods to control icon selection
 */

void
IconC::EnableSelection()
{
   selectionOk = True;
}

void
IconC::DisableSelection()
{
   selectionOk = False;
}

/*-----------------------------------------------------------------------
 *  Methods to control icon double-click
 */

void
IconC::EnableOpen()
{
   openOk = True;
}

void
IconC::DisableOpen()
{
   openOk = False;
}
