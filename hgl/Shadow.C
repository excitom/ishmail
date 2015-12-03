/*
 * $Id: Shadow.C,v 1.2 2000/05/07 12:26:11 fnevgeny Exp $
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

#ifdef ICL
#include <stddef.h>
#endif
#include "Shadow.h"
#include <Xm/Xm.h>

/*----------------------------------------------------------------------
 * Method to draw a shadow around the specified region
 */

void
DrawShadow(Display *disp, Drawable draw, GC gc, Pixel tsColor, Pixel bsColor,
	   int x, int y, int wd, int ht, int thick, unsigned char type)
{
   switch (type) {
      case XmSHADOW_IN:
	 DrawShadowIn(disp, draw, gc, tsColor, bsColor, x, y, wd, ht, thick);
	 break;
      case XmSHADOW_OUT:
	 DrawShadowIn(disp, draw, gc, bsColor, tsColor, x, y, wd, ht, thick);
	 break;
      case XmSHADOW_ETCHED_IN:
	 DrawShadowEtchedIn(disp, draw, gc, tsColor, bsColor, x, y, wd, ht,
			    thick);
	 break;
      case XmSHADOW_ETCHED_OUT:
	 DrawShadowEtchedIn(disp, draw, gc, bsColor, tsColor, x, y, wd, ht,
			    thick);
	 break;
   }

} // End DrawShadow

/*----------------------------------------------------------------------
 * Method to draw an inward shadow around the specified region
 */

void
DrawShadowIn(Display *disp, Drawable draw, GC gc, Pixel tsColor, Pixel bsColor,
	     int x, int y, int wd, int ht, int thick)
{
//
// Draw top lines
//
   int	i;
   int	x1 = x;
   int	x2 = x + wd - 1;
   int	y1 = y;
   int	y2;
   XSetForeground(disp, gc, bsColor);
   for (i=0; i<thick; i++) {
      XDrawLine(disp, draw, gc, x1, y1, x2, y1);
      y1++;
      x2--;
   }

//
// Draw left lines
//
   x1 = x;
   y1 = y;
   y2 = y + ht - 1;
   for (i=0; i<thick; i++) {
      XDrawLine(disp, draw, gc, x1, y1, x1, y2);
      x1++;
      y2--;
   }

//
// Draw bottom lines
//
   XSetForeground(disp, gc, tsColor);
   x1 = x + 1;
   x2 = x + wd - 1;
   y1 = y + ht - 1;
   for (i=0; i<thick; i++) {
      XDrawLine(disp, draw, gc, x1, y1, x2, y1);
      x1++;
      y1--;
   }

//
// Draw right lines
//
   x1 = x + wd - 1;
   y1 = y + 1;
   y2 = y + ht - 1;
   for (i=0; i<thick; i++) {
      XDrawLine(disp, draw, gc, x1, y1, x1, y2);
      x1--;
      y1++;
   }

} // End DrawShadowIn

/*----------------------------------------------------------------------
 * Method to draw an etched in shadow around the specified region
 */

void
DrawShadowEtchedIn(Display *disp, Drawable draw, GC gc, Pixel tsColor,
		   Pixel bsColor, int x, int y, int wd, int ht, int thick)
{
   int	thick1 = thick / 2;
   int	thick2 = thick - thick1;
   DrawShadowIn(disp, draw, gc, tsColor, bsColor, x, y, wd, ht, thick1);
   DrawShadowIn(disp, draw, gc, bsColor, tsColor, x+thick1, y+thick1,
		wd-thick1*2, ht-thick1*2, thick2);

} // End DrawShadowEtchedIn

