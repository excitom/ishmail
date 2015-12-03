/*
 *  $Id: MimeRichTextDraw.C,v 1.2 2000/05/07 12:26:10 fnevgeny Exp $
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
#include "MimeRichTextC.h"
#include "MimeRichTextP.h"
#include "HalAppC.h"
#include "WArgList.h"
#include "gray.xbm"

#include <Xm/ScrollBar.h>

extern int	debuglev;

/*----------------------------------------------------------------------
 * Expose Callback
 */

void
MimeRichTextP::HandleExpose(Widget, MimeRichTextP *This,
			    XmDrawingAreaCallbackStruct *da)
{
   if ( !This->realized ) {

      This->textWin = XtWindow(This->textDA);

//
// Create stipple pixmap
//
      This->stipplePm = XCreateBitmapFromData(halApp->display, This->textWin,
					      gray_bits, gray_width,
					      gray_height);
      Window    root;
      int       x, y;
      unsigned	w, h;
      unsigned  bw;
      unsigned	dep;
      XGetGeometry(halApp->display, This->stipplePm, &root, &x, &y, &w, &h,
		   &bw, &dep);

//
// Create graphics contexts for drawing
//
      XtGCMask	fixMask = GCGraphicsExposures | GCBackground | GCLineStyle
      			| GCLineWidth | GCPlaneMask | GCStipple;
      XtGCMask	modMask = GCClipMask | GCFont | GCFunction | GCForeground
      			| GCFillStyle;
      XtGCMask	naMask	= GCArcMode | GCCapStyle | GCClipXOrigin
			| GCClipYOrigin | GCDashList | GCDashOffset
			| GCFillRule | GCJoinStyle | GCSubwindowMode | GCTile
			| GCTileStipXOrigin | GCTileStipYOrigin;
      XGCValues fixVals;
      fixVals.clip_mask		 = None;
      fixVals.fill_style	 = FillSolid;
      fixVals.stipple            = This->stipplePm;
      fixVals.font               = This->plainFont.fid;
      fixVals.foreground	 = This->fgColor;
      fixVals.background	 = This->bgColor;
      fixVals.function	 	 = GXcopy;
      fixVals.graphics_exposures = TRUE;
      fixVals.line_style	 = LineSolid;
      fixVals.line_width	 = 0;
      fixVals.plane_mask	 = AllPlanes;

      This->textGC = XtAllocateGC(This->textDA, 0, fixMask, &fixVals, modMask,
				  naMask);
      This->cursorPos.Set(This->topTextLine, 0, 0);

      ScreenPosC	spos = This->cursorPos;
      This->desiredCursorX = spos.x;
      This->cursorOn = False;

//
// Get scrollbars if necessary
//
      if ( This->textSW ) {

	 XtVaGetValues(This->textSW, XmNverticalScrollBar,   &This->textVSB,
	 			     XmNhorizontalScrollBar, &This->textHSB,
				     XmNclipWindow,          &This->textClip,
				     NULL);
	 XtVaGetValues(This->textClip, XmNwidth,  &This->clipWd,
	 			       XmNheight, &This->clipHt, NULL);

	 if ( This->drawWd == 0 )
	    XtVaGetValues(This->textDA, XmNwidth, &This->drawWd, NULL);
	 if ( This->drawHt == 0 )
	    XtVaGetValues(This->textDA, XmNwidth, &This->drawHt, NULL);

	 This->maxWd = This->drawWd;
	 This->maxHt = This->drawHt;

//
// If resize is not allowed, keep the text the same width as the scrolled
//    window
//
	 if ( !This->resizeWidth )
	    XtAddEventHandler(This->textSW, StructureNotifyMask, False,
			      (XtEventHandler)HandleSWResize, (XtPointer)This);

      } // End if we're in a scrolled window

      else {

	 if ( (int)This->drawWd <= (int)(This->marginWd*2) )
	    XtVaGetValues(This->textDA, XmNwidth, &This->drawWd, NULL);
	 if ( (int)This->drawHt <= (int)(This->marginHt*2) )
	    XtVaGetValues(This->textDA, XmNheight, &This->drawHt, NULL);

	 This->maxWd = This->drawWd;
	 This->maxHt = This->drawHt;

//
// Set scrollbar values
//
	 WArgList	args;
	 args.Minimum(0);
	 args.Value(0);

	 int	lh = This->pub->LineHeight();
	 args.Increment(lh);

	 if ( (int)This->drawHt > lh ) {
	    args.Maximum(This->drawHt);
	    args.SliderSize(This->drawHt);
	    args.PageIncrement(This->drawHt - lh);
	 }
	 XtSetValues(This->textVSB, ARGS);

	 int	cw = This->pub->CharWidth();
	 args.Increment(cw);

	 if ( (int)This->drawWd > cw ) {
	    args.Maximum(This->drawWd);
	    args.SliderSize(This->drawWd);
	    args.PageIncrement(This->drawWd - cw);
	 }
	 XtSetValues(This->textHSB, ARGS);

	 This->hsbVal = This->vsbVal = 0;
	 This->hsbMax = This->drawWd;
	 This->vsbMax = This->drawHt;

//
// Create a pixmap for off-screen drawing
//
	 unsigned depth = DefaultDepth(halApp->display,
				       DefaultScreen(halApp->display));
	 This->textPmWd = This->drawWd;
	 This->textPmHt = This->drawHt;
	 This->textPm = XCreatePixmap(halApp->display, This->textWin,
	 			      This->textPmWd, This->textPmHt, depth);

	 XtAddEventHandler(This->scrollForm, StructureNotifyMask, False,
			   (XtEventHandler)HandleFormResize, (XtPointer)This);

      } // End if we're doing our own scrolling

      This->realized = True;
      This->FormatScreen();

//
// Initialize the input method
//
      if ( This->editable ) {

	 XmImRegister(This->textDA, 0);

	 XPoint		spot;
	 spot.x = 0;
	 spot.y = 0;

	 Pixmap		bgPixmap;
	 XtVaGetValues(This->textDA, XmNbackgroundPixmap, &bgPixmap, NULL);

	 XmFontList	fontList = XmFontListCreate(This->plainFont.xfont,
						       XmSTRING_DEFAULT_CHARSET);

	 WArgList		args;
	 args.Background(This->bgColor);
	 args.Foreground(This->fgColor);
	 args.BackgroundPixmap(bgPixmap);
	 args.FontList(fontList);
	 args.Add("XmNlineSpacing",   This->lineSpacing);
	 args.Add("XmNspotLocation", &spot);

	 XmImSetValues(This->textDA, ARGS);

      } // End if input method needed

//
// Start the cursor blinking if the focus is here
//
      if ( This->hasFocus )
	 CursorBlink(This, NULL);

   } // End if not realized

//
// Redraw
//
   XExposeEvent *ev = (XExposeEvent*)da->event;
   RectC        area(ev->x, ev->y, ev->width, ev->height);
   This->DrawScreen(area);

} // End HandleExpose

/*----------------------------------------------------------------------
 * Resize callback for scrollForm
 */

void
MimeRichTextP::HandleFormResize(Widget, MimeRichTextP *This, XEvent*, Boolean*)
{
   if ( debuglev > 1 )
      cout <<"MimeRichTextC(" <<XtName(This->pub->MainWidget())
	   <<")::HandleFormResize" <<endl;

   Dimension	wd, ht;
   XtVaGetValues(This->textDA, XmNwidth, &wd, XmNheight, &ht, NULL);
   if ( wd == This->drawWd && ht == This->drawHt ) return;

   halApp->BusyCursor(True);

//
// See how much the size changed.  Change the max size by these same amounts.
//
   Dimension	dwd = wd - This->drawWd;
   Dimension	dht = ht - This->drawHt;

   This->drawWd = wd;
   This->drawHt = ht;
   This->maxWd += dwd;
   This->maxHt += dht;

   This->CheckPixmapSize();

//
// Update drawing area
//
   This->changed = True;
   This->FormatScreen();

   if ( This->realized ) {
      RectC	area(0, 0, wd, ht);
      This->DrawScreen(area);
      ScreenPosC	spos = This->cursorPos;
      This->desiredCursorX = spos.x;
   }

   halApp->BusyCursor(False);

} // End HandleResize

/*-----------------------------------------------------------------------
 *  Event handler for scrolled window resize
 */

void
MimeRichTextP::HandleSWResize(Widget, MimeRichTextP *This, XEvent*, Boolean*)
{
//
// Get the size of the clip window
//
   XtVaGetValues(This->textClip, XmNwidth,  &This->clipWd,
   				 XmNheight, &This->clipHt, NULL);

//
// If we're not allowed to resize ourself, follow the size of the scrolled
//    window.
//
   if ( !This->resizeWidth ) {

      halApp->BusyCursor(True);

      Dimension	wd = This->clipWd - 1;
      XtVaSetValues(This->textDA, XmNwidth, wd, NULL);

      This->drawWd  = wd;
      This->changed = True;

      This->FormatScreen();

      if ( This->realized ) {
	 RectC	area(0, 0, This->drawWd, This->drawHt);
	 This->DrawScreen(area);
	 ScreenPosC	spos = This->cursorPos;
	 This->desiredCursorX = spos.x;
      }

      halApp->BusyCursor(False);

   } // End if we're not allowed to set our own size

} // End HandleSWResize

/*----------------------------------------------------------------------
 * Adjust the reference widget so the specified amount of the drawing
 *    area size is displayed.
 */

void
MimeRichTextP::SetVisibleSize(Dimension newWd, Dimension newHt, Widget ref)
{
//
// If we're in a scrolled window, resize that
//
   if ( textSW ) {

      if ( !ref ) ref = textSW;

//
// Get the current size
//
      Dimension	swd, sht;
      XtVaGetValues(ref, XmNwidth, &swd, XmNheight, &sht, NULL);

//
// Determine the size difference
//
      Dimension	dwd = swd - clipWd;
      Dimension	dht = sht - clipHt;

//
// Calculate the new size needed for the scrolled window
//
      clipWd = newWd + marginWd2 + 1;
      clipHt = newHt + marginHt2 + 1;

      swd = clipWd + dwd;
      sht = clipHt + dht;
      XtVaSetValues(ref, XmNwidth, swd, XmNheight, sht, NULL);
      
//
// Calculate the new size needed for the text form
//
      Dimension	fwd = newWd + marginWd2;
      Dimension	fht = newHt + marginHt2;

      if ( resizeWidth || (int)fwd < (int)(clipWd - 1) ) fwd = clipWd - 1;
      if ( (int)fht < (int)(clipHt - 1) ) fht = clipHt - 1;

      XtVaSetValues(textDA, XmNwidth, fwd, XmNheight, fht, NULL);

//
// Remove any resize events from the queue.  We don't need them.
//
      XSync(halApp->display, FALSE);
      XEvent	event;
      while ( XCheckTypedWindowEvent(halApp->display, XtWindow(textSW),
				     ConfigureNotify, &event) )
	 ;

   } // End if we're in a scrolled window

//
// If we're doing our own scrolling, resize the scrollForm
//
   else {

//
// Get the current size
//
      Dimension	swd = 0, sht = 0;
      if ( ref )
	 XtVaGetValues(ref, XmNwidth, &swd, XmNheight, &sht, NULL);

      if ( swd > 0 && sht > 0 ) {

//
// Determine the size difference
//
	 Dimension	dwd = swd - drawWd;
	 Dimension	dht = sht - drawHt;

//
// Set the new size
//
	 swd = newWd + dwd;
	 sht = newHt + dht;
	 XtVaSetValues(ref, XmNwidth, swd, XmNheight, sht, NULL);

      } // End if reference widget is specified

//
// Compute the size of the scrollForm required to give us the requested
//    drawing area size
//
      else {

	 Dimension	shad;
	 XtVaGetValues(textFrame, XmNshadowThickness, &shad, NULL);

	 swd = newWd
	     + shad		// textFrame
	     + (hlThick*2)	// hlForm
	     + 2;		// for grins
	 if ( vsbOn ) swd += scrollRoff;
	 else	      swd += noScrollRoff;

	 sht = newHt
	     + shad		// textFrame
	     + (hlThick*2) 	// hlForm
	     + 2;		// for grins
	 if ( hsbOn ) sht += scrollBoff;
	 else	      sht += noScrollBoff;

	 XtVaSetValues(scrollForm, XmNwidth, swd, XmNheight, sht, NULL);

      } // End if no reference widget is specified

#if 0
//
// Remove any resize events from the queue.  We don't need them.
//
      XSync(halApp->display, FALSE);
      XEvent	event;
      while ( XCheckTypedWindowEvent(halApp->display, XtWindow(scrollForm),
				     ConfigureNotify, &event) )
	 ;
#endif

   } // End if we're doing our own scrolling

   maxWd = drawWd = newWd;
   maxHt = drawHt = newHt;

//
// Reallocate pixmap
//
   CheckPixmapSize();

//
// Update drawing area
//
   changed = True;
   if ( !defer ) {

      FormatScreen();

      if ( realized ) {
	 RectC	area(0, 0, maxWd, maxHt);
	 DrawScreen(area);
	 ScreenPosC	spos = cursorPos;
	 desiredCursorX = spos.x;
      }
   }

} // End SetVisibleSize

/*-----------------------------------------------------------------------
 *  Resize the off-screen pixmap if the visible area has increased
 */

void
MimeRichTextP::CheckPixmapSize()
{
//
// Re-create pixmap if size has increased
//
   if ( !textPm || (drawWd <= textPmWd && drawHt <= textPmHt) ) return;

   XFreePixmap(halApp->display, textPm);
   u_int depth = DefaultDepth(halApp->display, DefaultScreen(halApp->display));
   textPmWd    = MAX(drawWd, textPmWd);
   textPmHt    = MAX(drawHt, textPmHt);
   textPm      = XCreatePixmap(halApp->display, textWin, textPmWd, textPmHt,
			       depth);

} // End CheckPixmapSize

/*-----------------------------------------------------------------------
 *  Format the specified line and redraw all affected lines
 */

void
MimeRichTextP::LineChanged(TextLineC *tline, Boolean forcePlace)
{
   LinesChanged(tline, tline, forcePlace);
}

/*-----------------------------------------------------------------------
 *  Format the specified lines and redraw all affected lines
 */

void
MimeRichTextP::LinesChanged(TextLineC *begLine, TextLineC *endLine,
			    Boolean forcePlace)
{
//
// Re-format the text lines
//
   Boolean	placeNeeded  = False;
   Boolean	widthNeeded  = False;
   Boolean	widthChanged = False;
   TextLineC	*tline = begLine;
   SoftLineC	*sprev = begLine->prev ? begLine->prev->LastSoft()
				       : (SoftLineC*)NULL;
   while ( tline != endLine->next ) {

      int	oldHt = tline->bounds.ht;
      int	oldWd = tline->bounds.wd;
      FormatLine(tline);

      if ( !placeNeeded ) placeNeeded = (oldHt != tline->bounds.ht);

//
// If the line shrunk, we'll have to recalculate the max line width for all
//    lines.
//
      if ( tline->bounds.wd < oldWd ) widthNeeded = True;

//
// If the line grew, see if the text width is affected.  If it is, we'll have
//    to re-justify all lines.
//
      else if ( tline->bounds.wd > textWd ) {
	 textWd = tline->bounds.wd;
	 widthChanged = True;
      }

//
// Update the soft line links
//
      SoftLineC	*first = tline->FirstSoft();
      if ( sprev ) {
	 sprev->next = first;
	 first->prev = sprev;
      }
      else
	 topSoftLine = first;

      sprev = tline->LastSoft();
      tline = tline->next;

   } // End for each changed line

//
// Update last soft link
//
   SoftLineC	*snext = endLine->next ? endLine->next->FirstSoft()
				       : (SoftLineC*)NULL;
   SoftLineC	*last = endLine->LastSoft();
   if ( snext ) {
      last->next  = snext;
      snext->prev = last;
   }
   else
      botSoftLine = last;

//
// See if we need to recalculate the maximum line width.
//
   if ( widthNeeded ) {
      int	newWd = MaxLineWidth();
      if ( newWd != textWd ) {
	 textWd = newWd;
	 widthChanged = True;
      }
   }

//
// If the max width has changed, we'll need to re-justify all lines.
// If not, we'll justify only the lines that changed
//
   if ( widthChanged )
      JustifyLines();

   else /*if ( resizeWidth )*/ {

      tline = begLine;
      while ( tline != endLine->next ) {

	 SoftLineC	*sline = tline->softLine;
	 while ( sline && sline->textLine == tline ) {
	    Justify(sline);
	    sline = sline->next;
	 }

	 tline = tline->next;
      }
   }

//
// See if we need to re-position the lines vertically.  This will update
//    both scroll bars.
//
   int	oldVsbVal = vsbVal;
   if ( placeNeeded || forcePlace )
      PlaceLines(begLine);

//
// Update horizontal scroll bar if necessary.  There must be room for at least
//   one complete line before we turn the scroll bars on.
//
   else if ( !textSW ) {

      WArgList	args;
      int	lineHt = pub->LineHeight();
      int	sbSize = scrollBoff - noScrollBoff;
      int	drawHtWithSB = drawHt;
      if ( !hsbOn ) drawHtWithSB -= sbSize;

      if ( (textWd > (int)drawWd) && (drawHtWithSB >= lineHt) ) {

	 if ( !hsbOn ) {
	    XtVaSetValues(hlForm, XmNbottomOffset, scrollBoff, NULL);
	    drawHt -= sbSize;
	    hsbOn = True;
	 }

      } // End if horizontal scroll bar needed

      else {

	 if ( hsbOn && !hsbAlwaysOn ) {
	    XtVaSetValues(hlForm, XmNbottomOffset, noScrollBoff, NULL);
	    drawHt += sbSize;
	    hsbOn = False;
	 }

      } // End if horizontal scroll bar not needed

      hsbMax = MAX((int)drawWd, textWd);
      int	ss = MIN((int)drawWd, hsbMax);
      if ( hsbVal + ss > hsbMax ) hsbVal = hsbMax - ss;
      int	pi = drawWd - pub->CharWidth();
      if ( pi <= 0 ) pi = drawWd;

      args.Value(hsbVal);
      args.Maximum(hsbMax);
      args.MappedWhenManaged(hsbOn);
      args.SliderSize(ss);
      args.PageIncrement(pi);
      XtSetValues(textHSB, ARGS);

   } // End if horizontal scroll bar needs to be checked

//
// Redraw the affected lines.  Draw all lines if the scroll bar moved.
//
   int	ymin, ht;
   if ( vsbVal != oldVsbVal ) {
      ymin = 0;
      ht   = drawHt;
   }
   else {
      ymin = begLine->bounds.ymin - vsbVal;
      if ( ymin < 0 ) ymin = 0;

      int	ymax;
      if ( placeNeeded || forcePlace ) ymax = drawHt - 1;
      else			       ymax = endLine->bounds.ymax - vsbVal;

      ht = ymax - ymin + 1;
      if ( ht > (int)drawHt ) ht = drawHt;
   }

   RectC area(0, ymin, drawWd, ht);
   DrawScreen(area);

//
// Call text change callbacks
//
   CallCallbacks(changeCalls, pub);

} // End LinesChanged

/*----------------------------------------------------------------------
 * Set the position of all lines
 */

void
MimeRichTextP::PlaceLines(TextLineC *startLine)
{
   if ( startLine && startLine->prev )
      textY = startLine->prev->bounds.ymax + 1 + lineSpacing;

   else {
      startLine = topTextLine;
      textY = marginHt + 2;	// This will allow the accent characters to be
      				// displayed on the top line
   }

   TextLineC	*line = startLine;
   while ( line ) {
      SetLinePosition(line, textY);
      textY += line->bounds.ht + lineSpacing;
      line = line->next;
   }
   if ( startLine ) textY -= lineSpacing;
   textY += marginHt;

//
// Clear out the bottom part
//
   if ( realized && textY < (int)textHt ) {
      int	y = textY - vsbVal;
      if ( y >= 0 && y <= (int)drawHt ) {
	 XSetForeground(halApp->display, textGC, bgColor);
	 Drawable	drawto = pub->DrawTo();
	 XFillRectangle(halApp->display, drawto, textGC, 0, y,
			drawWd, drawHt - y + 1);
	 if ( drawto != textWin )
	    XFillRectangle(halApp->display, textWin, textGC, 0, y,
			   drawWd, drawHt - y + 1);
	 XSetForeground(halApp->display, textGC, fgColor);
      }
   }

//
// If we're in a scrolled window, see if we need to resize ourself.
//
   if ( textSW ) {

      Dimension	newHt = MAX(textY, (int)(clipHt-marginHt2-1));
      if ( newHt != textHt ) {

	 drawHt = textHt = newHt;

	 if ( debuglev > 1 ) cout <<"setting height to " <<textHt NL;
	 XtVaSetValues(textDA, XmNheight, textHt+marginHt2, 0);

      } // End if height changed

   } // End if height can be adjusted

//
// If we're not in a scrolled window, update the scroll bars
//
   else {

      textHt = textY;

//
// Update the scroll bars
//
      static Boolean	formatAgain = False;
      if ( !formatAgain ) {

	 WArgList	args;

//
// See if we need a vertical scroll bar
//
	 args.Reset();
	 if ( textHt > (int)maxHt && (int)drawWd >= 2*pub->CharWidth() ) {

	    if ( !vsbOn ) {
	       XtVaSetValues(hlForm, XmNrightOffset, scrollRoff, NULL);
	       drawWd -= (scrollRoff - noScrollRoff);
	       formatAgain = True;
	       vsbOn = True;
	    }

	 } // End if vertical scroll bar needed

	 else {

	    if ( vsbOn && !vsbAlwaysOn ) {
	       XtVaSetValues(hlForm, XmNrightOffset, noScrollRoff, NULL);
	       drawWd += (scrollRoff - noScrollRoff);
	       formatAgain = True;
	       vsbOn = False;
	    }

	 } // End if vertical scroll bar not needed

	 if ( formatAgain ) FormatScreen();
	 formatAgain = False;

	 vsbMax = MAX((int)drawHt, textHt);
	 int	ss = MIN((int)drawHt, vsbMax);
	 if ( vsbVal + ss > vsbMax ) vsbVal = vsbMax - ss;
	 int	pi = drawHt - pub->LineHeight();
	 if ( pi <= 0 ) pi = drawHt;

	 args.Value(vsbVal);
	 args.Maximum(vsbMax);
	 args.MappedWhenManaged(vsbOn);
	 args.SliderSize(ss);
	 args.PageIncrement(pi);
	 XtSetValues(textVSB, ARGS);

//
// See if we need a horizontal scroll bar.  There must be room for at least
//   one complete line before we turn the scroll bar on.
//
	 args.Reset();

	 int	lineHt = pub->LineHeight();
	 int	sbSize = scrollBoff - noScrollBoff;
	 int	drawHtWithSB = drawHt;
	 if ( !hsbOn ) drawHtWithSB -= sbSize;

	 if ( (textWd > (int)drawWd) && (drawHtWithSB >= lineHt) ) {

	    if ( !hsbOn ) {
	       XtVaSetValues(hlForm, XmNbottomOffset, scrollBoff, NULL);
	       drawHt -= sbSize;
	       hsbOn = True;
	    }

	 } // End if horizontal scroll bar needed

	 else {

	    if ( hsbOn && !hsbAlwaysOn ) {
	       XtVaSetValues(hlForm, XmNbottomOffset, noScrollBoff, NULL);
	       drawHt += sbSize;
	       hsbOn = False;
	    }

	 } // End if horizontal scroll bar not needed

	 hsbMax = MAX((int)drawWd, textWd);
	 ss = MIN((int)drawWd, hsbMax);
	 if ( hsbVal + ss > hsbMax ) hsbVal = hsbMax - ss;
	 pi = drawWd - pub->CharWidth();
	 if ( pi <= 0 ) pi = drawWd;

	 args.Value(hsbVal);
	 args.Maximum(hsbMax);
	 args.MappedWhenManaged(hsbOn);
	 args.SliderSize(ss);
	 args.PageIncrement(pi);
	 XtSetValues(textHSB, ARGS);

	 CheckPixmapSize();

      } // End if we didn't already do this

   } // End if we're doing our own scroll bars

} // End PlaceLines

/*----------------------------------------------------------------------
 * Format the text lines
 */

void
MimeRichTextP::FormatScreen()
{
   if ( defer || !changed || !realized ) return;

   if ( debuglev > 1 )
      cout <<"MimeRichTextC(" <<XtName(pub->MainWidget())
      	   <<")::FormatScreen" <<endl;

   textWd = marginWd2;
   textHt = marginHt2;

//
// Remove all soft lines
//
   TextLineC	*tline = topTextLine;
   while ( tline ) {

      SoftLineC	*sline = tline->softLine;
      while ( sline && sline->textLine == tline ) {

	 SoftLineC	*next = sline->next;
	 delete sline;
	 sline = next;
      }
      tline->softLine = NULL;

      tline = tline->next;
   }
   topSoftLine = botSoftLine = NULL;

//
// Process the lines and link soft lines as we go
//
   tline = topTextLine;
   while ( tline ) {

      FormatLine(tline);

      if ( !topSoftLine ) {
	 topSoftLine = tline->FirstSoft();
	 botSoftLine = tline->LastSoft();
      }
      else {
	 SoftLineC	*first = tline->FirstSoft();
	 botSoftLine->next = first;
	 first->prev = botSoftLine;
	 botSoftLine = tline->LastSoft();
      }

      tline = tline->next;
   }

//
// Set the text width to the maximum line width and justify all lines.
//
   textWd = MaxLineWidth();
   JustifyLines();	// Set horizontal positions

//
// Set the lines' vertical positions
//
   PlaceLines();

   pub->ResetPartIndex();
   changed = False;

   if ( debuglev > 1 ) cout <<"Leaving FormatScreen" NL;

} // End FormatScreen

/*----------------------------------------------------------------------
 * Lay out the specified text line
 */

void
MimeRichTextP::FormatLine(TextLineC *textLine)
{
   if ( debuglev > 1 ) cout <<"Formatting line " <<textLine->index <<endl;

//
// Delete the soft lines for this text
//
   SoftLineC	*sline = textLine->softLine;
   while ( sline && sline->textLine == textLine ) {
      SoftLineC	*next = sline->next;
      delete sline;
      sline = next;
   }
   textLine->softLine = NULL;

// If we do this, we can't change state at the end of some text
#if 0
//
// Make the line as efficient as possible
//
   CompactLine(textLine);
#endif

//
// Create a new soft line
//
   SoftLineC	*softLine = new SoftLineC;
   softLine->textLine = textLine;
   textLine->softLine = softLine;

//
// Get the initial state and the initial indent conditions
//
   TextStateC	*state = textLine->State(0);
   int		lindent = state->LIndent();
   int		rindent = state->RIndent();

//
// Determine the available width
//
   int	availWd;
   if ( resizeWidth )
      availWd = 32768;
   else
      availWd = drawWd - (indentWd * (lindent + rindent))
		       - (excerptWd * state->Excerpt())
		       - marginWd2;

   int leftX = marginWd + (state->Excerpt() * excerptWd) + (lindent * indentWd);

//
// Loop through the commands.
//
   unsigned	ccount = textLine->cmdList.size();
   RichCmdC	*cmd;
   int i=0; for (i=0; i<ccount; i++) {

      cmd = textLine->Cmd(i);
      if ( cmd->IsText() )
	 softLine = FormatText(textLine, softLine, i, cmd->state, &leftX,
			       &availWd, &lindent, &rindent);
      else
	 softLine = FormatGraphic(textLine, softLine, i, cmd->state, &leftX,
				  &availWd, &lindent, &rindent);

   } // End for each command

//
// If there is a blank soft line at the end of the list, remove it.  If it
//    is the only one, leave it.
//
   SoftLineC	*first = textLine->FirstSoft();
   SoftLineC	*last  = textLine->LastSoft();
   if ( first != last && !last->drawData ) {
      last->prev->next = NULL;
      delete softLine;
   }

//
// Loop through the soft lines and calculate their sizes
//
   leftX = marginWd + (state->Excerpt() * excerptWd) + (lindent * indentWd);
   int			textLineHt = 0;
   int			textLineWd = 0;
   RichDrawDataC	*prevData = NULL;

   sline = textLine->softLine;
   while ( sline && sline->textLine == textLine ) {

//
// Create an empty draw data for any soft line that has none
//
      if ( !sline->drawData ) {

	 RichDrawDataC	*data = new RichDrawDataC;
	 data->softLine = sline;
	 if ( prevData ) {
	    data->cmdPos    = prevData->cmdPos;
	    data->textOff   = prevData->textOff + prevData->string.size();
	    data->font      = prevData->font;
	    data->underline = prevData->underline;
	 }
	 else {
	    data->cmdPos    = 0;
	    data->textOff   = 0;
	    data->font      = CurFont(*state);
	    data->underline = (state->Underline()>0);
	 }
	 data->x     = leftX;
	 data->width = 0;

	 RichDrawDataC	*lastData = softLine->LastData();
	 if ( lastData ) {
	    lastData->next = data;
	    data->prev = lastData;
	 }
	 else {
	    softLine->drawData = data;
	 }

      } // End if this is an empty line

      else {
	 prevData = sline->LastData();
      }

//
// Calculate width and height
//
      GetLineSize(sline);

//
// Update the bounds of this text line
//
      textLineHt += sline->bounds.ht;
      textLineHt += lineSpacing;
      if ( /*resizeWidth &&*/ sline->totalWd > textLineWd )
	 textLineWd = sline->totalWd;

      sline = sline->next;

   } // End for each soft line

   textLineHt -= lineSpacing;

   if ( textLineHt == 0 ) textLineHt = 1;
   textLine->bounds.SetSize(textLineWd, textLineHt);

   if ( debuglev > 1 )
      cout <<"Text Line bounds are: " <<textLine->bounds.xmin <<" "
				      <<textLine->bounds.ymin <<" "
				      <<textLine->bounds.wd   <<" "
				      <<textLine->bounds.ht   <<endl;

//
// Position the soft lines vertically
//
   SetLinePosition(textLine, textLine->bounds.ymin);

} // End FormatLine

/*----------------------------------------------------------------------
 * Try to place the text in the available area.  Wrap if necessary
 */

SoftLineC*
MimeRichTextP::FormatText(TextLineC *textLine, SoftLineC *softLine,
			  int cmdPos, TextStateC& state, int *leftX,
			  int *availWd, int *lindent, int *rindent)
{
//
// Grab the text
//
   StringC	*textStr = textLine->Text(cmdPos);
   char		*cs = *textStr;
   int		len = textStr->size();
   int		remaining = len;

   if ( debuglev > 1 ) cout <<"Formatting text ^" <<*textStr <<"^" <<endl;

//
// If the length is 0, create one draw data and be done with it
//
   if ( remaining == 0 ) {
      RichDrawDataC	*data = new RichDrawDataC;
      data->softLine  = softLine;
      data->cmdPos    = cmdPos;
      data->textOff   = 0;
      data->x         = *leftX;
      data->width     = 0;
      data->font      = CurFont(state);
      data->underline = (state.Underline()>0);
      data->string    = "";
      softLine->AddDrawData(data);
   }

//
// Loop until we're fininshed with this text
//
   while ( remaining > 0 ) {

//
// If there are any tabs, split them out
//
      char	*tp = strchr(cs, '\t');
      if ( tp ) {
	 if ( tp == cs ) len = 1;
	 else		 len = tp - cs;
      }

//
// Get size of text
//
      int		dir, asc, dsc;
      XCharStruct	size;
      FontDataC		*curFont = CurFont(state);
      if ( *cs == '\t' ) {
	 int	charWd  = curFont->CharWidth();
	 int	tabSize = tabStop * charWd;
	 int	tabX    = (state.Excerpt() * excerptWd) +
	 		  (state.LIndent() * indentWd)  + tabSize;
	 while ( tabX <= *leftX ) tabX += tabSize;
	 size.width = tabX - *leftX;
      }

      else {
	 XTextExtents(curFont->xfont, cs, len, &dir, &asc, &dsc, &size);
      }

//
// If it doesn't fit, find out how much will
//
      Boolean	addIt = True;
      Boolean	lineFull = (size.width > *availWd);
      if ( lineFull ) {

	 if ( *cs == '\t' ) {
	    addIt = (softLine->drawData == NULL);
	 }
	 else {
	    int	fitLen = FitText(cs, curFont->xfont, *availWd);
	    if ( fitLen > 0 ) {
	       len = fitLen;
	       XTextExtents(curFont->xfont, cs, len, &dir, &asc, &dsc, &size);
	    }
	    else
	       addIt = (softLine->drawData == NULL);
	 }
      }

//
// Add this section of text if necessary
//
      if ( addIt ) {

//
// Create a new draw data
//
	 RichDrawDataC	*data = new RichDrawDataC;
	 data->softLine  = softLine;
	 data->cmdPos    = cmdPos;
	 data->textOff   = cs - (char*)*textStr;
	 data->x         = *leftX;
	 data->width     = size.width;
	 data->font      = curFont;
	 data->underline = (state.Underline()>0);
	 data->string.Set(cs, len);
	 softLine->AddDrawData(data);

	 *availWd -= size.width;
	 *leftX   += size.width;

//
// Advance past this text
//
	 remaining -= len;
	 cs += len;
	 len = remaining;

      } // End if block added to current line

//
// Start a new line if it didn't all fit
//
      if ( lineFull ) {

//
// Create a new soft line
//
	 softLine = new SoftLineC;
	 softLine->textLine = textLine;
	 textLine->AddSoftLine(softLine);

//
// Update the indention
//
	 *lindent = state.LIndent();
	 *rindent = state.RIndent();

//
// Compute the new available width
//
	 if ( resizeWidth )
	    *availWd = 32768;
	 else
	    *availWd = drawWd - (indentWd * (*lindent + *rindent))
			      - (excerptWd * state.Excerpt())
			      - marginWd2;

	 *leftX = marginWd + (state.Excerpt() * excerptWd)
	 		   + (*lindent * indentWd);

//
// Skip whitespace except tabs
//
	 if ( len == 0 ) len = remaining;
	 while ( remaining > 0 && isspace(*cs) && *cs != '\t' ) {
	    cs++;
	    remaining--;
	    len--;
	 }

      } // End if more characters remain

   } // End while more characters remain

   return softLine;

} // End FormatText

/*----------------------------------------------------------------------
 * See how many characters will fit in the available width.
 */

int
MimeRichTextP::FitText(char *cp, XFontStruct *font, int availWd)
{
   int	wd  = 0;
   char	*wp = cp;
   int	len = 0;

//
// Loop while we have more room and more characters
//
   while ( *wp && availWd > wd ) {

//
// Skip over current whitespace
//
      while ( *wp && isspace(*wp) ) wp++;

//
// Look for next whitespace
//
      while ( *wp && !isspace(*wp) ) wp++;

//
// See how long this section is
//
      int		wlen = wp - cp;
      int		dir, asc, dsc;
      XCharStruct	size;
      XTextExtents(font, cp, wlen, &dir, &asc, &dsc, &size);
      wd = size.width;
      if ( wd <= availWd ) len = wlen;

   } // End while more room and more chars

   return len;

} // End FitText

/*----------------------------------------------------------------------
 * Try to place the graphic in the available area.  Wrap if necessary
 */

SoftLineC*
MimeRichTextP::FormatGraphic(TextLineC *textLine, SoftLineC *softLine,
			     int cmdPos, TextStateC& state, int *leftX,
			     int *availWd, int *lindent, int *rindent)
{
   RichGraphicC	*rg = textLine->Graphic(cmdPos);

   if ( debuglev > 1 ) cout <<"Formatting graphic ^" <<rg <<"^" <<endl;

   int	prefWd = rg->PrefWidth();
   int	prefHt = rg->PrefHeight();

//
// Start a new line if the current line is full.
//
   if ( prefWd > *availWd && softLine->drawData != NULL ) {

//
// Create a new soft line
//
      softLine = new SoftLineC;
      softLine->textLine = textLine;
      textLine->AddSoftLine(softLine);

//
// Update the indention
//
      *lindent = state.LIndent();
      *rindent = state.RIndent();

//
// Compute the new available width
//
      if ( resizeWidth )
	 *availWd = 32768;
      else
	 *availWd = drawWd - (indentWd * (*lindent + *rindent))
			   - (excerptWd * state.Excerpt())
			   - marginWd2;

      *leftX = marginWd + (state.Excerpt() * excerptWd) + (*lindent * indentWd);

   } // End if graphic won't fit

//
// Now add the graphic.  First, create a new draw data
//
   RichDrawDataC	*data = new RichDrawDataC;
   data->softLine  = softLine;
   data->cmdPos    = cmdPos;
   data->x         = *leftX;
   data->width     = prefWd;
   data->graphic   = rg;

   rg->bounds.Set(*leftX, textLine->bounds.ymin, prefWd, prefHt);
   if ( debuglev > 1 )
      cout <<"   bounds are " <<rg->bounds.xmin SP rg->bounds.ymin SP
      				rg->bounds.wd   SP rg->bounds.ht <<endl;
   softLine->AddDrawData(data);

//
// Update the available space.
//
   *availWd -= prefWd;
   *leftX   += prefWd;

   return softLine;

} // End FormatGraphic

/*----------------------------------------------------------------------
 * Get the width and height of a soft line
 */

void
MimeRichTextP::GetLineSize(SoftLineC *line)
{
   int	lineWd   = 0;
   int	lineHt   = 0;
   line->descent = 0;

   TextStateC		*state = NULL;
   RichDrawDataC	*dd = line->drawData;
   while ( dd ) {

//
// Update width
//
      lineWd += dd->width;

//
// Update height
//
      if ( dd->graphic ) {
	 if ( dd->graphic->bounds.ht > lineHt )
	    lineHt = dd->graphic->bounds.ht;
      }
      else {
	 int	fontHt = dd->font->xfont->ascent + dd->font->xfont->descent;
	 if ( fontHt > lineHt ) lineHt = fontHt;

	 if ( dd->font->xfont->descent > line->descent )
	    line->descent = dd->font->xfont->descent;
      }

//
// Save initial state for use below
//
      if ( dd == line->drawData && line->textLine )
	 state = line->textLine->State(dd->cmdPos);

      dd = dd->next;

   } // End for each draw data

//
// Add size of excerpt string if necessary
//
   if ( state && state->Excerpt() ) {

      int	fontHt = plainFont.xfont->ascent + plainFont.xfont->descent;
      if ( fontHt > lineHt ) lineHt = fontHt;

      if ( plainFont.xfont->descent > line->descent )
	 line->descent = plainFont.xfont->descent;
   }

//
// Fall back to default font height if line is blank
//
   if ( lineHt == 0 ) {
      lineHt        = plainFont.xfont->ascent + plainFont.xfont->descent;
      line->descent = plainFont.xfont->descent;
   }

   line->bounds.SetSize(lineWd, lineHt);

//
// See what indentation and excerpting does to the width
//
   line->totalWd = lineWd;
   if ( state ) {
      line->totalWd += indentWd  * (state->LIndent() + state->RIndent());
      line->totalWd += excerptWd * state->Excerpt();
   }
   line->totalWd += marginWd2;

} // End GetLineSize

/*----------------------------------------------------------------------
 * Justify all soft lines
 */

void
MimeRichTextP::JustifyLines()
{
   SoftLineC	*line = topSoftLine;
   while ( line ) {
      Justify(line);
      line = line->next;
   }
}

/*----------------------------------------------------------------------
 * Justify the specified line
 */

void
MimeRichTextP::Justify(SoftLineC *line)
{
//
// Get the state for the first draw data
//
   TextStateC	*state = NULL;
   if ( line->drawData && line->textLine ) {
      RichDrawDataC	*dd = line->drawData;
      state = line->textLine->State(dd->cmdPos);
   }

//
// Calculate left and right edges
//
   int lx = marginWd + (state->Excerpt() * excerptWd)
   		     + (state->LIndent() * indentWd);
   int rx;
   if ( resizeWidth ) rx = MAX(textWd, (int)drawWd);
   else		      rx = drawWd;
   rx -= marginWd + (state->RIndent() * indentWd);

//
// Center if necessary
//
   int	lineX = lx;
   if ( state->CurJustCmd() == JC_CENTER ) {
      int	avail = rx - lx + 1;
      int	pad   = (avail - line->bounds.wd) / (int)2;
      lineX = lx + pad;
   }

   else if ( state->CurJustCmd() == JC_RIGHT ) {
      lineX = rx - line->bounds.wd;
   }
   if ( lineX < lx ) lineX = lx;

   line->bounds.SetPos(lineX, line->bounds.ymin);

//
// Adjust data segments
//

   RichDrawDataC	*dd = line->drawData;
   while ( dd ) {

      dd->x = lineX;

      if ( dd->graphic )
	 dd->graphic->bounds.SetPos(lineX, dd->graphic->bounds.ymin);

      lineX += dd->width;
      dd = dd->next;
   }

} // End Justify

/*----------------------------------------------------------------------
 * Return the maximum soft line width
 */

int
MimeRichTextP::MaxLineWidth()
{
   int		maxWd = 0;
   SoftLineC	*line = topSoftLine;
   while ( line ) {
      if ( line->totalWd > maxWd ) maxWd = line->totalWd;
      line = line->next;
   }

   return maxWd;
}

/*--------------------------------------------------------------------
 *  Method to set the Y position for a line
 */

void
MimeRichTextP::SetLinePosition(TextLineC *textLine, int y)
{
   textLine->bounds.SetPos(textLine->bounds.xmin, y);

//
// Update the soft lines
//
   SoftLineC	*sline = textLine->softLine;
   while ( sline && sline->textLine == textLine ) {

      sline->bounds.SetPos(sline->bounds.xmin, y);

      RichDrawDataC	*dd = sline->drawData;
      while ( dd ) {

	 if ( dd->graphic ) {
	    dd->graphic->bounds.ymax = sline->bounds.ymax;
	    dd->graphic->bounds.ymin =
	    	sline->bounds.ymax - dd->graphic->bounds.ht + 1;
	 }

	 dd = dd->next;

      } // End for each draw data

      y += sline->bounds.ht + lineSpacing;

      sline = sline->next;

   } // End for each soft line

} // End SetLinePosition

/*----------------------------------------------------------------------
 * Redraw the lines in the specified range
 */

void
MimeRichTextP::DrawLines(SoftLineC *begLine, SoftLineC *endLine)
{
   if ( !realized || defer ) return;

   int	ht = endLine->bounds.ymax - begLine->bounds.ymin + 1;

   Drawable	drawto = pub->DrawTo();
   RectC	visArea(hsbVal, vsbVal, drawWd, drawHt);

//
// Clear the drawing area
//
   XSetForeground(halApp->display, textGC, bgColor);
   XFillRectangle(halApp->display, drawto, textGC, 0,
		  begLine->bounds.ymin-vsbVal, drawWd, ht);
   XSetForeground(halApp->display, textGC, fgColor);

//
// Draw each line
//
   long		lastFont  = 0;
   Pixel	lastColor = fgColor;
   SoftLineC	*line = begLine;
   Boolean	done = False;
   while ( !done ) {

      if ( visArea.OverlapsY(line->bounds) )
	 DrawLine(line, &lastFont, &lastColor);
      else
	 HideLineGraphics(line);

      done = (line == endLine);
      line = line->next;
   }

   if ( drawto != textWin )
      XCopyArea(halApp->display, drawto, textWin, textGC, 0, 0, drawWd, drawHt,
		0, 0);

} // End DrawLines

/*----------------------------------------------------------------------
 * Draw the current text.  The specified area is in textDA coordinates.
 */

void
MimeRichTextP::DrawScreen(RectC& area)
{
   if ( !realized || defer ) return;

   if ( debuglev > 1 )
      cout <<"MimeRichTextC(" <<XtName(pub->MainWidget())
	   <<")::DrawScreen with area: "
           <<area.xmin <<" " <<area.ymin <<" "
	   <<area.wd   <<" " <<area.ht   <<endl;

   Drawable	drawto = pub->DrawTo();
   RectC	visArea(0, 0, drawWd, drawHt);
   if ( !area.Overlaps(visArea) ) return;

//
// Intersect the area to be drawn with the visible area
//
   RectC	drawArea = area | visArea;

   HideCursor();

//
// Update the size of the drawing area
//
   if ( textSW ) {
      if ( resizeWidth  ) {
	 if ( debuglev > 1 ) cout <<"setting width to " <<drawWd NL;
	 XtVaSetValues(textDA, XmNwidth, drawWd+marginWd2, 0);
      }
      else if ( drawWd != clipWd - marginWd2 - 1 ) {
	 drawWd = clipWd - marginWd2 - 1;
	 if ( debuglev > 1 ) cout <<"setting width to " <<drawWd NL;
	 XtVaSetValues(textDA, XmNwidth, drawWd+marginWd2, 0);
      }

      if ( debuglev > 1 ) cout <<"setting height to " <<drawHt NL;
      XtVaSetValues(textDA, XmNheight, drawHt+marginHt2, 0);
   }

   XRectangle	rect;
   rect.x      = (short)drawArea.xmin;
   rect.y      = (short)drawArea.ymin;
   rect.width  = (short)drawArea.wd;
   rect.height = (short)drawArea.ht;
   XSetClipRectangles(halApp->display, textGC, 0, 0, &rect, 1, YXBanded);

//
// Clear the drawing area
//
   XSetForeground(halApp->display, textGC, bgColor);
   XFillRectangle(halApp->display, drawto, textGC, drawArea.xmin, drawArea.ymin,
   		  drawArea.wd, drawArea.ht);
   XSetForeground(halApp->display, textGC, fgColor);

//
// Gray out the data if we're not sensitive
//
   if ( !XtIsSensitive(textDA) )
      XSetFillStyle(halApp->display, textGC, FillStippled);

//
// Scroll the area if we're doing our own scrolling
//
   RectC	sarea(drawArea);
   sarea.xmin += hsbVal;
   sarea.xmax += hsbVal;
   sarea.ymin += vsbVal;
   sarea.ymax += vsbVal;

   visArea.xmin += hsbVal;
   visArea.xmax += hsbVal;
   visArea.ymin += vsbVal;
   visArea.ymax += vsbVal;

//
// Loop through the lines
//
   long		lastFont = 0;
   Pixel	lastColor = fgColor;
   SoftLineC	*line = topSoftLine;
   while ( line ) {

      if ( debuglev > 1 )
	 cout <<"Checking soft line: " <<line->bounds.xmin <<" "
				       <<line->bounds.ymin <<" "
				       <<line->bounds.wd <<" "
				       <<line->bounds.ht <<endl;

//
// Draw line if it is visible
//
      if ( sarea.OverlapsY(line->bounds) )
	 DrawLine(line, &lastFont, &lastColor);
      else if ( !visArea.OverlapsY(line->bounds) )
	 HideLineGraphics(line);

      line = line->next;

   } // End for each line

//
// Gray out the data if we're not sensitive
//
   if ( !XtIsSensitive(textDA) )
      XSetFillStyle(halApp->display, textGC, FillSolid);

//
// Refresh selection
//
   if ( selectOn )
      DrawSelection();	// This will also call XCopyArea

   else if ( drawto != textWin )
      XCopyArea(halApp->display, drawto, textWin, textGC, 0, 0, drawWd, drawHt,
		0, 0);

   XSetClipMask(halApp->display, textGC, None);

   ShowCursor();

} // End DrawScreen

/*----------------------------------------------------------------------
 * Draw the specified line
 */

void
MimeRichTextP::DrawLine(SoftLineC *line, long *lastFont, Pixel *lastColor,
			Boolean clear)
{
   if ( !realized || defer ) return;

   Drawable	drawto = pub->DrawTo();

   if ( debuglev > 1 )
      cout <<"Drawing line with bounds: " <<line->bounds.xmin <<" "
					  <<line->bounds.ymin <<" "
					  <<line->bounds.wd <<" "
					  <<line->bounds.ht <<endl;

   if ( clear ) {
//
// Clear the line area
//
      XSetForeground(halApp->display, textGC, bgColor);
      XFillRectangle(halApp->display, drawto, textGC, 0,
		     line->bounds.ymin-vsbVal, drawWd, line->bounds.ht);
      XSetForeground(halApp->display, textGC, fgColor);
   }

   int	lineX = line->bounds.xmin;
   int	botY  = line->bounds.ymax;

//
// Draw excerpt string if necessary
//
   TextStateC	*state = line->textLine->State(0);
   if ( state->Excerpt() ) {

      int	x = marginWd;
      int	y = botY - line->descent;

      if ( *lastFont != plainFont.fid ) {
	 XSetFont(halApp->display, textGC, plainFont.fid);
	 *lastFont = plainFont.fid;
      }

      int e=0; for (e=0; e<state->Excerpt(); e++) {
	 XDrawString(halApp->display, drawto, textGC, x-hsbVal, y-vsbVal,
		     excerptStr, excerptStr.size());
	 x += excerptWd;
      }

   } // End if excerpting

//
// Loop through data
//
   RichDrawDataC	*dd = line->drawData;
   while ( dd ) {

      lineX = dd->x;

      state = line->textLine->State(dd->cmdPos);

//
// Set color if necessary
//
      Pixel	color = fgColor;
      if ( state->PLinkCount() > 0 )
	 color = linkColor;
      else if ( state->URL() )
	 color = urlColor;
      else {
	 color = state->CurColor();
	 if ( color == (Pixel)NULL ) color = fgColor;
      }

      if ( *lastColor != color ) {
	 XSetForeground(halApp->display, textGC, color);
	 *lastColor = color;
      }

//
// Underline if necessary
//
      if ( dd->underline ) {
	 int	underY = botY + 1 + dd->font->underY;
	 for (int u=0; u<dd->font->underThick; u++) {
	    XDrawLine(halApp->display, drawto, textGC, lineX-hsbVal,
		      underY-vsbVal, lineX-hsbVal+dd->width, underY-vsbVal);
	    underY++;
	 }
      }

//
// Draw text if present
//
      if ( dd->string.size() > 0 && dd->string[0] != '\t' ) {

	 int	y = botY - line->descent;

//
// Set font if necessary
//
	 if ( *lastFont != dd->font->fid ) {
	    XSetFont(halApp->display, textGC, dd->font->fid);
	    *lastFont = dd->font->fid;
	 }

	 if ( debuglev > 1 )
	    cout <<"Drawing text ^" <<dd->string <<"^ at: " <<lineX <<" "
	    	 <<y <<endl;

	 XDrawString(halApp->display, drawto, textGC, lineX-hsbVal, y-vsbVal,
		     dd->string, dd->string.size());

      } // End if not a tab

//
// Draw graphic if present
//
      if ( dd->graphic ) {

//
// Apply the scroll offsets
//
	 dd->graphic->bounds.xmin -= hsbVal;
	 dd->graphic->bounds.xmax -= hsbVal;
	 dd->graphic->bounds.ymin -= vsbVal;
	 dd->graphic->bounds.ymax -= vsbVal;

//
// Draw the graphic
//
	 dd->graphic->Show();
	 dd->graphic->Refresh();

//
// Undo the scroll offsets
//
	 dd->graphic->bounds.xmin += hsbVal;
	 dd->graphic->bounds.xmax += hsbVal;
	 dd->graphic->bounds.ymin += vsbVal;
	 dd->graphic->bounds.ymax += vsbVal;

      } // End if there is a graphic

#if 0
      if ( debuglev > 1 )
	 XDrawRectangle(halApp->display, drawto, textGC, lineX-hsbVal,
			line->bounds.ymin-vsbVal, dd->width, line->bounds.ht);
#endif

      dd = dd->next;

   } // End for each draw data

} // End DrawLine

/*----------------------------------------------------------------------
 * Hide any graphics for the specified line
 */

void
MimeRichTextP::HideLineGraphics(SoftLineC *line)
{
//
// Loop through data
//
   RichDrawDataC	*dd = line->drawData;
   while ( dd ) {
      if ( dd->graphic ) dd->graphic->Hide();
      dd = dd->next;
   }

} // End HideLineGraphics

/*---------------------------------------------------------------
 *  Handle a keyboard focus change event in the area
  */

void
MimeRichTextP::HandleFocusChange(Widget, MimeRichTextP *This, XEvent *ev,
				 Boolean*)
{
   switch (ev->type) {

//
// Start the cursor blinking again
//
      case (FocusIn):
      case (EnterNotify):
      {
#if 0
	 if ( debuglev > 1 ) {
	    if ( ev->type == FocusIn ) cout <<"FocusIn" <<endl;
	    else                       cout <<"EnterNotify" <<endl;
	 }
#endif
	 if ( This->cursorTimer ) {
	    XtRemoveTimeOut(This->cursorTimer);
	    This->cursorTimer = (XtIntervalId)NULL;
	 }

	 if ( This->hlForm )
	    XtVaSetValues(This->hlForm, XmNbackground, This->hlColor, NULL);

	 WArgList	args;
	 XPoint		spot = This->CursorLocation();
	 args.Add("XmNspotLocation", &spot);

	 XmImSetFocusValues(This->textDA, ARGS);

	 This->hasFocus = True;

	 if ( This->realized ) {
	    CursorBlink(This, NULL);
	    XmProcessTraversal(This->textDA, XmTRAVERSE_CURRENT);
	 }

      } break;

//
// Stop the cursor blinking
//
      case (FocusOut):
      case (LeaveNotify):
#if 0 
	 if ( debuglev > 1 ) {
	    if ( ev->type == FocusOut ) cout <<"FocusOut" <<endl;
	    else                        cout <<"LeaveNotify" <<endl;
	 }
#endif
	 if ( This->cursorTimer ) {
	    XtRemoveTimeOut(This->cursorTimer);
	    This->cursorTimer = (XtIntervalId)NULL;
	 }

	 if ( This->hlForm )
	    XtVaSetValues(This->hlForm, XmNbackground, This->hlFormBg, NULL);

         if ( This->cursorOn ) This->DrawCursor();	// Turn it off

	 XmImUnsetFocus(This->textDA);

	 This->hasFocus = False;

	 break;
   }

} // End HandleFocusChange

/*---------------------------------------------------------------
 *  Timer proc to blink insertion cursor
 */

void
MimeRichTextP::CursorBlink(MimeRichTextP *This, XtIntervalId*)
{
#if 0
   if ( debuglev > 1 ) {
      cout <<"CursorBlink";
      if ( This->cursorOn ) cout <<" turning off";
      else                  cout <<" turning on";
      cout <<endl;
   }
#endif

   int	nextTime = This->cursorOn ? This->cursorOffTime : This->cursorOnTime;

//
// Draw the cursor if there is a non-zero interval for the next state or
//    if we have lost focus and the cursor is on.
//
   if ( nextTime > 0 || (!This->hasFocus && This->cursorOn) )

      This->DrawCursor();	// Draw the cursor using XOR mode

//
// Set up the next blink
//
   if ( This->timerOk && nextTime > 0 && This->hasFocus )
      This->cursorTimer = XtAppAddTimeOut(halApp->context, nextTime,
					  (XtTimerCallbackProc)CursorBlink,
					  (XtPointer)This);

} // End CursorBlink

/*---------------------------------------------------------------
 *  Draws the cursor using XOR mode.  May turn it on, may turn it off.
 *  Depends on whether it was on or off previously.
 */

void
MimeRichTextP::DrawCursor()
{
   ScreenPosC	pos = cursorPos;
   int	ht = 0;
   if ( pos.softLine ) ht = pos.softLine->bounds.ht;

   int	x = pos.x - hsbVal;
   int	y = (pos.softLine ? pos.softLine->bounds.ymin : 0) - vsbVal - 1;

   if ( y < 0 ) {
      ht += y;	// Reduce height.  y is negative
      y = 0;
   }
   else if ( y+ht > (int)drawHt ) {
      ht = drawHt - y;
   }

   if ( ht <= 0 )
      ht = plainFont.xfont->ascent + plainFont.xfont->descent;

   XSetFunction(halApp->display, textGC, GXxor);
   XSetForeground(halApp->display, textGC, cursorColor);

#if 0
   if ( debuglev > 1 ) cout <<"DrawCursor at " <<x <<" " <<y <<endl;
#endif
   XDrawLine(halApp->display, textWin, textGC, x, y, x, y+ht-1);

   if ( editable ) {
      XDrawLine(halApp->display, textWin, textGC, x-2, y,      x+2, y);
      XDrawLine(halApp->display, textWin, textGC, x-2, y+ht-1, x+2, y+ht-1);
   }

   XSetFunction(halApp->display, textGC, GXcopy);
   XSetForeground(halApp->display, textGC, fgColor);

   cursorOn = !cursorOn;
}

/*---------------------------------------------------------------
 *  Turn the cursor off
 */

void
MimeRichTextP::HideCursor()
{
   cursorHideCount++;
   if ( cursorHideCount > 1 ) return;

   if ( cursorTimer ) {
      XtRemoveTimeOut(cursorTimer);
      cursorTimer = (XtIntervalId)NULL;
   }

   lastCursorPos = cursorPos;

   if ( cursorOn ) DrawCursor();	// To turn it off
}

/*---------------------------------------------------------------
 *  Turn the cursor on
 */

void
MimeRichTextP::ShowCursor()
{
   if ( cursorHideCount > 0 ) {
      cursorHideCount--;
      if ( cursorHideCount > 0 ) return;
   }

   if ( !cursorOn && hasFocus ) CursorBlink(this, NULL);

//
// Update the cursor state
//
   RichCmdC	*cmd = cursorPos.Cmd();
   if ( cmd && lastCursorState != cmd->state ) {
      lastCursorState = cmd->state;
      CallCallbacks(stateCalls, pub);
   }

//
// Update the input manager
//
   if ( hasFocus && cursorPos != lastCursorPos ) {

      WArgList	args;
      XPoint		spot = CursorLocation();
      args.Add("XmNspotLocation", &spot);

      XmImSetFocusValues(textDA, ARGS);

      lastCursorPos = cursorPos;
   }

} // End ShowCursor

/*-----------------------------------------------------------------------
 *  Return the x/y position of the cursor
 */

XPoint
MimeRichTextP::CursorLocation()
{
   ScreenPosC	spos(cursorPos);

   XPoint	loc;
   loc.x = spos.x;
   loc.y = spos.softLine ? spos.softLine->bounds.ymax : 0;

   return loc;
}

/*---------------------------------------------------------------
 *  Scroll the window so the cursor stays visible
 */

Boolean
MimeRichTextP::ScrollToCursor()
{
   ScreenPosC	spos = cursorPos;
   return ScrollToPosition(spos);
}

/*---------------------------------------------------------------
 *  Scroll the window so the specified position stays visible
 */

Boolean
MimeRichTextP::ScrollToPosition(ScreenPosC& spos)
{
   int	x = spos.x;
   int	y = spos.softLine ? spos.softLine->bounds.ymin : 0;

//
// Calculate the bounds of the visible area.  Use the scroll bar parameters.
//
   int	xmin, xmax, ymin, ymax, sbwd, sbht;
   if ( textSW ) {
      XtVaGetValues(textHSB, XmNvalue, &xmin, XmNsliderSize, &sbwd, NULL);
      XtVaGetValues(textVSB, XmNvalue, &ymin, XmNsliderSize, &sbht, NULL);
   }
   else {
      xmin = hsbVal;
      ymin = vsbVal;
      sbwd = drawWd;
      sbht = drawHt;
   }
   xmax = xmin + sbwd - 1;
   ymax = ymin + sbht - 1;

   int	charWd = pub->CharWidth();
   int	lineHt = 0;
   if ( spos.softLine ) lineHt = spos.softLine->bounds.ht;
   if ( lineHt == 0 ) lineHt = pub->CharHeight();

//
// Test the x position against the visible region
//
   int	val, size, inc, pinc, sbmax;
   if ( (x < xmin) || (x+charWd > xmax) ) {

      XmScrollBarGetValues(textHSB, &val, &size, &inc, &pinc);
      XtVaGetValues(textHSB, XmNmaximum, &sbmax, NULL);

      if ( x < xmin ) val = x;
      else	      val = xmin + (x+charWd - xmax);
      if ( val + sbwd > sbmax ) val = sbmax - sbwd;

      XmScrollBarSetValues(textHSB, val, sbwd, inc, pinc, True);
   }

//
// Test the y position against the visible region
//
   if ( (y < ymin) || (y+lineHt > ymax) ) {

      XmScrollBarGetValues(textVSB, &val, &size, &inc, &pinc);
      XtVaGetValues(textVSB, XmNmaximum, &sbmax, NULL);

      if ( y < ymin ) val = y;
      else	      val = ymin + (y+lineHt - ymax);
      if ( val + sbht > sbmax ) val = sbmax - sbht;

      XmScrollBarSetValues(textVSB, val, sbht, inc, pinc, True);
   }

   return True;

} // End ScrollToPosition

/*---------------------------------------------------------------
 *  Determine if the specified soft line is completely visible
 */

Boolean
MimeRichTextP::LineFullyVisible(SoftLineC *line)
{
   if ( !textVSB ) return True;

//
// Calculate the bounds of the visible area.  Use the scroll bar parameters.
//
   int	ymin, ymax, sbht;
   XtVaGetValues(textVSB, XmNvalue, &ymin, XmNsliderSize, &sbht, NULL);
   ymax = ymin + sbht - 1;

//
// Test the position against the visible region
//
   if ( line->bounds.ymin < ymin || line->bounds.ymax > ymax )
      return False;

   return True;

} // End LineFullyVisible

/*---------------------------------------------------------------
 *  Determine if any part of the specified soft line is visible
 */

Boolean
MimeRichTextP::LineVisible(SoftLineC *line)
{
//
// Calculate the bounds of the visible area.  Use the scroll bar parameters.
//
   int	ymin, ymax, sbht;
   XtVaGetValues(textVSB, XmNvalue, &ymin, XmNsliderSize, &sbht, NULL);
   ymax = ymin + sbht - 1;

//
// Test the position against the visible region
//
   if ( line->bounds.ymax < ymin || line->bounds.ymin > ymax )
      return False;

   return True;

} // End LineVisible

/*-----------------------------------------------------------------------
 *  Defer changes
 */

void
MimeRichTextC::Defer(Boolean on)
{
   if ( on ) {
      priv->defer++;			// Up the count
   } 
   else if ( priv->defer>0 ) {		// Turn off only if deferred
      priv->defer--;			// Refresh if last turn-off
      if ( (priv->defer==0) && priv->changed ) {
	 Refresh();
	 CallCallbacks(priv->changeCalls, this);
      }
   }

} // End Defer

/*-----------------------------------------------------------------------
 *  Make deferred changes
 */

void
MimeRichTextC::Refresh()
{
//   halApp->BusyCursor(True);
   int	saveDefer = priv->defer;
   priv->defer = 0;

   priv->FormatScreen();

   RectC area(0, 0, priv->drawWd, priv->drawHt);
   priv->DrawScreen(area);

   priv->defer = saveDefer;
//   halApp->BusyCursor(False);

} // End Refresh

/*-----------------------------------------------------------------------
 *  Constructor for soft line data
 */

SoftLineC::SoftLineC()
{
   textLine = NULL;
   totalWd  = 0;
   descent  = 0;
   drawData = NULL;
   prev     = NULL;
   next     = NULL;
}

/*-----------------------------------------------------------------------
 *  Destructor for soft line data
 */

SoftLineC::~SoftLineC()
{
   RichDrawDataC	*data = drawData;
   while ( data ) {
      RichDrawDataC	*next = data->next;
      delete data;
      data = next;
   }
}

/*----------------------------------------------------------------------
 * Add a draw data record to the draw data list
 */

void
SoftLineC::AddDrawData(RichDrawDataC *data)
{
   if ( !drawData ) {
      drawData = data;
      return;
   }

   RichDrawDataC	*last = LastData();
   last->next = data;
   data->prev = last;
}

/*-----------------------------------------------------------------------
 *  Method to see if the specified point is within a graphic
 */

RichGraphicC*
MimeRichTextP::PickGraphic(int x, int y)
{
//
// Scroll the positions
//
   x += hsbVal;
   y += vsbVal;

//
// Loop through the graphics
//
   u_int	count = graphicList.size();
   if ( count == 0 ) return NULL;

   int i=0; for (i=0; i<count; i++) {
      RichGraphicC	*rg = (RichGraphicC*)*graphicList[i];
      if ( rg->bounds.Inside(x, y) ) return rg;
   }

   return NULL;
}

