/*
 * $Id: LgIconViewC.C,v 1.2 2000/05/07 12:26:10 fnevgeny Exp $
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
#include "LgIconViewC.h"
#include "VItemC.h"
#include "VBoxC.h"
#include "rsrc.h"
#include "WXmString.h"

/*-----------------------------------------------------------------------
 *  Return widget name for button in popup menu used to display this view
 */

char *
LgIconViewC::ButtonName() const
{
   return "largeIconViewPB";
}

/*-----------------------------------------------------------------------
 *  Create a pixmap for an item if necessary
 */

void
LgIconViewC::GetPixmap(IconDataC& data)
{
   PixmapC	*pm;
   Screen	*scrn = XtScreen(viewBox->ViewDA());
   Window	win   = viewBox->ViewWin();

   switch ( data.item->ImageSource() ) {

      case (VItemC::IMAGE_FILE): {

	 StringC&	name = data.item->LgImageFile();
	 if ( !pixmapFileDict.includes(name) ) {
	    pm = new PixmapC(name, regFgColor, regBgColor, invFgColor,
			     invBgColor, scrn, win);
	    pixmapFileDict.add(name, pm);
	 }

	 data.pixmap = *pixmapFileDict.definitionOf(name);

      } break;

      case (VItemC::XBM_DATA): {

	 XbmT&	xbm = data.item->LgXbmData();
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

	 XpmT	xpm = data.item->LgXpmData();
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
 *  Private method to redraw the specified item
 */

void
LgIconViewC::DrawItem(const VItemC& item, const IconDataC& data,
		      VItemDrawModeT mode, Drawable drawto)
{
   if ( !viewBox->Realized() ) return;

   Boolean	invert;

   if ( mode == AS_IS )
      invert = viewBox->SelItems().includes(&item);
   else
      invert = (mode == INVERT);

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

//
// Draw pixmap
//
   if ( data.pixmap ) {

      PixmapC	*pm = data.pixmap;
      Pixmap	src = invert ? pm->inv : pm->reg;

      if ( src ) {

	 x = data.bounds.xmin + ((data.bounds.wd - pm->wd) / 2)
	   - viewBox->HScrollValue();
	 y = data.bounds.ymin + hlThick + itemMarginHt
	   - viewBox->VScrollValue();

	 if ( pm->mask ) {
	    XSetClipMask(halApp->display, gc, pm->mask);
	    XSetClipOrigin(halApp->display, gc, x, y);
	 }

	 XCopyArea(halApp->display, src, dst1, gc, /*srcx*/0, /*srcy*/0,
		   pm->wd, pm->ht, /*dstx*/x, /*dsty*/y);
	 if ( dst2 )
	    XCopyArea(halApp->display, src, dst2, gc, /*srcx*/0, /*srcy*/0,
		      pm->wd, pm->ht, /*dstx*/x, /*dsty*/y);

	 if ( pm->mask ) {
	    XSetClipMask(halApp->display, gc, None);
	    XSetClipOrigin(halApp->display, gc, 0, 0);
	 }

      } // End if source pixmap available
   } // End if pixmap data available

//
// Draw name string
//
   XSetForeground(halApp->display, gc, invert ? invFgColor : regFgColor);
   if ( viewBox->GoodFont() ) XSetFont(halApp->display, gc, font->fid);
   DrawLabel(data, drawto);

//
// Draw highlight if necessary
//
   if ( focusHere && (VItemC *)&item == hlItem )
      DrawHighlight(&data, hlColor, drawto);

} // End LgIconViewC DrawItem

/*-----------------------------------------------------------------------
 *  Method to draw the label for the specified item
 */

void
LgIconViewC::DrawLabel(const IconDataC& data, Drawable drawto)
{
//
// If drawto is NULL, draw to both the offscreen pixmap and the visible window
//
   Drawable	dst1 = drawto;
   Drawable	dst2 = (Drawable)NULL;
   if ( !dst1 ) {
      dst1 = viewBox->ViewWin();
      dst2 = viewBox->ViewPm();
   }

   int	y = data.bounds.ymin + hlThick + itemMarginHt + maxPmHt + labelOffset
	  - viewBox->VScrollValue();
   GC	gc = viewBox->ViewGC();

//
// Loop through label components
//
   unsigned	count = data.labelList.size();
   for (int i=0; i<count; i++) {

      LabelDataC	*ldata = data.labelList[i];
      int		x      = data.bounds.xmin - viewBox->HScrollValue() +
				 ((maxItemWd - ldata->width) / 2);

      if ( fontList ) {

	 WXmString	wstr(ldata->string, ldata->tag);
	 XRectangle	rect;
	 rect.x      = x;
	 rect.y      = y;
	 rect.width  = ldata->width;
	 rect.height = ldata->height;
	 XmStringDraw(halApp->display, dst1, fontList, (XmString)wstr,
		      gc, x, y, ldata->width, XmALIGNMENT_CENTER,  
		      XmSTRING_DIRECTION_L_TO_R, &rect);
	 if ( dst2 )
	    XmStringDraw(halApp->display, dst2, fontList, (XmString)wstr,
			 gc, x, y, ldata->width, XmALIGNMENT_CENTER,  
			 XmSTRING_DIRECTION_L_TO_R, &rect);

//
// Move to top of next label
//
	 y += ldata->height + labelSpacing;
      }

      else {

//
// Center component in icon
//
	 y += font->ascent;

	 XDrawString(halApp->display, dst1, gc, x, y, ldata->string,
		     ldata->string.size());
	 if ( dst2 )
	    XDrawString(halApp->display, dst2, gc, x, y, ldata->string,
			ldata->string.size());

//
// Move to top of next label
//
	 y += font->descent + labelSpacing;
      }

   } // End for each label component

} // End LgIconViewC DrawLabel

/*-----------------------------------------------------------------------
 *  Methods to calculate the maximum item size
 */

void
LgIconViewC::UpdateMaxItemWd()
{
   maxItemWd = MAX(maxPmWd, maxLabelWd) + (hlThick+itemMarginWd)*2;
}

void
LgIconViewC::UpdateMaxItemHt()
{
   maxItemHt = maxPmHt + maxLabelHt + labelOffset + (hlThick+itemMarginHt)*2;
}
