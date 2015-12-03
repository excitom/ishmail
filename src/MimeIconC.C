/*
 *  $Id: MimeIconC.C,v 1.3 2000/05/08 11:28:18 fnevgeny Exp $
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

#include <config.h>
#include "MimeIconC.h"

#include <hgl/HalAppC.h>
#include <hgl/PixmapC.h>
#include <hgl/rsrc.h>
#include <hgl/Shadow.h>
#include <hgl/CharC.h>

XtIntervalId	MimeIconC::animTimer    = (XtIntervalId)NULL;
int		MimeIconC::animInterval = -1;
PtrListC	*MimeIconC::activeAnims = NULL;

/*---------------------------------------------------------------
 *  Constructor
 */

MimeIconC::MimeIconC(char *name, MimeRichTextC *rich)
{
   rt          = rich;
   pm          = NULL;
   font        = halApp->font;
   labelWd     = 0;
   labelHt     = 0;
   lastEvent   = NULL;
   highlighted = False;
   animIndex   = -1;
   animList    = NULL;
   animPm      = NULL;

   singleClickCalls.AllowDuplicates(TRUE);
   doubleClickCalls.AllowDuplicates(TRUE);
   menuCalls.AllowDuplicates(TRUE);

//
// Read resources
//
   char		*cl = "MimeIconC";
   Widget	par = rt->TextArea();
   regBgColor   = get_color(cl, name, "background",	    par, "White");
   regFgColor   = get_color(cl, name, "foreground",	    par, "Black");
   invBgColor   = get_color(cl, name, "selectedBackground", par, regFgColor);
   invFgColor   = get_color(cl, name, "selectedForeground", par, regBgColor);
   topShadColor = get_color(cl, name, "topShadowColor",	    par, "White");
   botShadColor = get_color(cl, name, "bottomShadowColor",  par, "Black");
   labelSpacing = get_int  (cl, name, "labelSpacing",	    par, 1);
   labelOffset  = get_int  (cl, name, "labelOffset",	    par, 5);
   marginWd     = get_int  (cl, name, "marginWidth",	    par, 3);
   marginHt     = get_int  (cl, name, "marginHeight",	    par, 3);
   shadowType   = get_shadow_type(cl, name, "shadowType",   par, XmSHADOW_OUT);
   shadowThick  = get_int  (cl, name, "shadowThickness",    par, 2);
   pixmapFile   = get_string(cl, name, "pixmap",	par);

   if ( !activeAnims ) {
      activeAnims = new PtrListC;
      animInterval = get_int(cl, name, "animationInterval", par, 250);
   }

   StringC	tmp;
   tmp = get_string(cl, name, XmNfont, par);
   if ( tmp.size() > 0 )
      font = XLoadQueryFont(halApp->display, tmp);

   labelStr = get_string(cl, name, "labelString", par);
   GetLabelSize();

//
// Create graphics context for drawing
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
   fixVals.clip_mask  = None;
   fixVals.fill_style = FillSolid;
   fixVals.function   = GXcopy;
   fixVals.graphics_exposures = TRUE;
   fixVals.line_style = LineSolid;
   fixVals.line_width = 0;
   fixVals.plane_mask = AllPlanes;
   fixVals.foreground = regFgColor;
   fixVals.background = regFgColor;

   if ( font != halApp->font ) {
      fixMask |= GCFont;
      fixVals.font = font->fid;
   }

   gc = XtAllocateGC(par, 0, fixMask, &fixVals, modMask, naMask);

} // End constructor

/*---------------------------------------------------------------
 *  Destructor
 */

MimeIconC::~MimeIconC()
{
   if ( !halApp->xRunning ) return;

   void	*tmp = (void*)this;
   if ( activeAnims->includes(tmp) ) {
      activeAnims->remove(tmp);
      if ( activeAnims->size() == 0 && animTimer ) {
	 XtRemoveTimeOut(animTimer);
	 animTimer = (XtIntervalId)NULL;
      }
   }

   XtReleaseGC(rt->TextArea(), gc);
   if ( font != halApp->font ) XFreeFont(halApp->display, font);
   delete pm;

   DeleteCallbacks(singleClickCalls);
   DeleteCallbacks(doubleClickCalls);
   DeleteCallbacks(menuCalls);
}

/*---------------------------------------------------------------
 *  Method to return the preferred width
 */

int
MimeIconC::PrefWidth()
{
   GetPixmap();

//
// Decorations
//
   int	wd = marginWd*2;	// Outside shadow
   wd += shadowThick*2;
   wd += marginWd*2;		// Inside shadow

//
// Animation pixmap
//
   if ( animPm ) wd += animPm->wd + labelOffset;

//
// Regular pixmap
//
   if ( pm ) wd += pm->wd + labelOffset;

//
// Label string
//
   wd += labelWd;

   return wd;

} // End PrefWidth

/*---------------------------------------------------------------
 *  Method to return the preferred height
 */

int
MimeIconC::PrefHeight()
{
   GetPixmap();

//
// Decorations
//
   int	ht = marginHt*2;	// Outside shadow
   ht += shadowThick*2;
   ht += marginHt*2;		// Inside shadow

//
// Animation pixmap
//
   int	maxHt = 0;
   if ( animPm && animPm->ht > maxHt ) maxHt = animPm->ht;

//
// Regular pixmap
//
   if ( pm && pm->ht > maxHt ) maxHt = pm->ht;

//
// Label string
//
   if ( labelHt > maxHt ) maxHt = labelHt;

   ht += maxHt;

   return ht;

} // End PrefHeight

/*---------------------------------------------------------------
 *  Method to cause a redraw
 */

void
MimeIconC::Refresh()
{
   GetPixmap();

   Drawable	drawto = rt->DrawTo();
   if ( !drawto ) return;

   RectC	rect(bounds.xmin + marginWd,
   		     bounds.ymin + marginHt,
		     bounds.wd   - (marginWd*2),
		     bounds.ht   - (marginHt*2));

//
// Clear background
//
   XSetForeground(halApp->display, gc,/*highlighted ? invBgColor :*/regBgColor);
   XFillRectangle(halApp->display, drawto, gc, rect.xmin, rect.ymin,
					       rect.wd,   rect.ht);

//
// Draw shadow
//
   if ( shadowThick > 0 )
      DrawShadow(halApp->display, drawto, gc,
      		 highlighted ? botShadColor : topShadColor,
      		 highlighted ? topShadColor : botShadColor,
      		 rect.xmin, rect.ymin, rect.wd, rect.ht, shadowThick,
		 shadowType);

   int	x = rect.xmin + shadowThick + marginWd;
   int	y;

//
// Draw animation pixmap
//
   if ( animPm && /*(( highlighted && animPm->inv) ||
      		   (!highlighted &&*/ animPm->reg/*))*/ ) {

      y = rect.ymin + (rect.ht - animPm->ht) / 2;

      if ( animPm->mask ) {
	 XSetClipMask(halApp->display, gc, animPm->mask);
	 XSetClipOrigin(halApp->display, gc, x, y);
      }

      XCopyArea(halApp->display, /*highlighted ? animPm->inv :*/ animPm->reg,
      		drawto, gc, /*srcx*/0, /*srcy*/0, animPm->wd, animPm->ht,
		/*dstx*/x, /*dsty*/y);

      if ( animPm->mask ) {
	 XSetClipMask(halApp->display, gc, None);
	 XSetClipOrigin(halApp->display, gc, 0, 0);
      }

      x += animPm->wd + labelOffset;

   } // End if there is an animation pixmap

//
// Draw regular pixmap
//
   if ( pm && /*((highlighted && pm->inv) || (!highlighted &&*/pm->reg/*))*/ ) {

      y = rect.ymin + (rect.ht - pm->ht) / 2;

      if ( pm->mask ) {
	 XSetClipMask(halApp->display, gc, pm->mask);
	 XSetClipOrigin(halApp->display, gc, x, y);
      }

      XCopyArea(halApp->display, /*highlighted ? pm->inv :*/pm->reg, drawto, gc,
      		/*srcx*/0, /*srcy*/0, pm->wd, pm->ht, /*dstx*/x, /*dsty*/y);

      if ( pm->mask ) {
	 XSetClipMask(halApp->display, gc, None);
	 XSetClipOrigin(halApp->display, gc, 0, 0);
      }

      x += pm->wd + labelOffset;

   } // End if there is a pixmap

//
// Draw label
//
   XSetForeground(halApp->display, gc,/*highlighted ? invFgColor :*/regFgColor);
   y = rect.ymin + (rect.ht - labelHt) / 2;

//
// Loop through lines
//
   u_int	offset = 0;
   int		pos = labelStr.PosOf('\n', offset);
   char		*cs = labelStr;
   CharC	line;
   while ( pos >= 0 ) {

      y += font->ascent;
      XDrawString(halApp->display, drawto, gc, x, y, cs + offset, pos - offset);
      y += font->descent + labelSpacing;

      offset = pos + 1;
      pos = labelStr.PosOf('\n', offset);
   }

//
// Draw remaining text
//
   y += font->ascent;
   XDrawString(halApp->display, drawto, gc, x, y, cs + offset,
	       labelStr.size() - offset);

} // End Refresh

/*---------------------------------------------------------------
 *  Method to return a textual description of this icon
 */

Boolean
MimeIconC::GetText(StringC& text)
{
   text += labelStr;
   return True;
}

/*---------------------------------------------------------------
 *  Method to change the label
 */

void
MimeIconC::SetLabel(CharC str)
{
   if ( labelStr == str ) return;

   labelStr = str;

   int	oldWd = labelWd;
   int	oldHt = labelHt;
   GetLabelSize();

   Boolean	sizeChanged = (labelWd != oldWd || labelHt != oldHt);
   rt->GraphicChanged(this, sizeChanged);
}

/*---------------------------------------------------------------
 *  Method to compute the size of the label
 */

void
MimeIconC::GetLabelSize()
{
   int		dir, asc, dsc;
   XCharStruct	size;

   labelWd =
   labelHt = 0;

//
// Loop through lines
//
   u_int	offset = 0;
   int		pos = labelStr.PosOf('\n', offset);
   char		*cs = labelStr;
   while ( pos >= 0 ) {

      if ( labelHt > 0 ) labelHt += labelSpacing;

      XTextExtents(font, cs + offset, pos - offset, &dir, &asc, &dsc, &size);
      labelHt += font->ascent + font->descent;
      if ( size.width > labelWd ) labelWd = size.width;

      offset = pos + 1;
      pos = labelStr.PosOf('\n', offset);
   }

//
// Add remaining text
//
   if ( labelHt > 0 ) labelHt += labelSpacing;

   XTextExtents(font, cs + offset, labelStr.size() - offset,
   		&dir, &asc, &dsc, &size);
   labelHt += font->ascent + font->descent;
   if ( size.width > labelWd ) labelWd = size.width;

} // End GetLabelSize

/*---------------------------------------------------------------
 *  Method to allocate the pixmap and compute its size
 */

void
MimeIconC::GetPixmap()
{
   if ( pm ) return;

   Window       win = RootWindowOfScreen(halApp->screen);
   if ( !win ) return;

//
// Get the name of a good pixmap file.  A derived class may pick a name other
//    than the default
//
   StringC	file;
   GetPixmapFile(file);
   if ( file.size() == 0 ) return;

//
// Try to create the pixmap
//
   pm = new PixmapC(file, regFgColor, regBgColor, invFgColor, invBgColor,
   		    halApp->screen, win);
   if ( !pm->reg ) {

      delete pm;
      pm = NULL;

//
// Try the default if it is different
//
      if ( pixmapFile.size() > 0 && pixmapFile != file ) {

	 pm = new PixmapC(pixmapFile, regFgColor, regBgColor,
				      invFgColor, invBgColor,
				      halApp->screen, win);
	 if ( !pm->reg ) {
	    delete pm;
	    pm = NULL;
	 }

      } // End if default pixmap file should be tried

   } // End if pixmap could not be created

} // End GetPixmap

/*---------------------------------------------------------------
 *  Default method to return the name of the pixmap file
 */

void
MimeIconC::GetPixmapFile(StringC& file)
{
   file = pixmapFile;
}

/*---------------------------------------------------------------
 *  Method to handle single-click
 */

void
MimeIconC::SingleClick(XButtonEvent *ev)
{
   lastEvent = (XButtonPressedEvent*)ev;
   CallSingleClickCallbacks();
}

/*---------------------------------------------------------------
 *  Method to handle double-click
 */

void
MimeIconC::DoubleClick(XButtonEvent *ev)
{
   lastEvent = (XButtonPressedEvent*)ev;
   CallDoubleClickCallbacks();
}

/*---------------------------------------------------------------
 *  Method to handle menu post
 */

void
MimeIconC::PostMenu(XButtonEvent *ev)
{
   lastEvent = (XButtonPressedEvent*)ev;
   CallMenuCallbacks();
}

/*---------------------------------------------------------------
 *  Methods to highlight and unhighlight
 */

void
MimeIconC::Highlight()
{
   if ( highlighted ) return;

   highlighted = True;
   rt->GraphicChanged(this, /*sizeChanged=*/False);
}

void
MimeIconC::Unhighlight()
{
   if ( !highlighted ) return;

   highlighted = False;
   rt->GraphicChanged(this, /*sizeChanged=*/False);
}

/*---------------------------------------------------------------
 *  Method to start an animation using the specified pixmaps
 */

void
MimeIconC::Animate(PtrListC *list)
{
   animList = list;
   if ( !list || list->size() <= 1 ) animIndex = -1;
   else				     animIndex =  0;

   if ( !list || list->size() == 0 ) animPm = NULL;
   else				     animPm = (PixmapC*)*((*animList)[0]);

//
// Redraw the icon
//
   rt->GraphicChanged(this, /*sizeChanged=*/True);

//
// Start a timer to change the pixmap
//
   if ( animIndex >= 0 ) {
      void	*tmp = (void*)this;
      activeAnims->add(tmp);
      if ( !animTimer )
	 animTimer = XtAppAddTimeOut(halApp->context, animInterval,
				    (XtTimerCallbackProc)UpdateAnimation, NULL);
   }

} // End Animate

/*---------------------------------------------------------------
 *  Timeout routine to update the animation
 */

void
MimeIconC::UpdateAnimation(XtPointer, XtIntervalId*)
{
//
// Loop through active animations
//
   u_int	count = activeAnims->size();
   for (int i=0; i<count; i++) {

      MimeIconC	*This = (MimeIconC*)*(*activeAnims)[i];

//
// Switch the pixmap
//
      This->animIndex++;
      if ( This->animIndex >= This->animList->size() )
	 This->animIndex = 0;

      This->animPm = (PixmapC*)*((*This->animList)[This->animIndex]);

//
// Redraw the graphic
//
      This->rt->GraphicChanged(This, /*sizeChanged=*/False);
   }

//
// Start another timer
//
   if ( count > 0 )
      animTimer = XtAppAddTimeOut(halApp->context, animInterval,
				  (XtTimerCallbackProc)UpdateAnimation, NULL);
   else
      animTimer = (XtIntervalId)NULL;

} // End UpdateAnimation

/*---------------------------------------------------------------
 *  Method to stop an animation
 */

void
MimeIconC::AnimationOff()
{
   void	*tmp = (void*)this;
   if ( !activeAnims->includes(tmp) ) return;

   activeAnims->remove(tmp);
   animList  = NULL;
   animIndex = -1;
   animPm    = NULL;

//
// Redraw the icon
//
   rt->GraphicChanged(this, /*sizeChanged=*/True);

} // End AnimationOff
