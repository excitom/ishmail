/*
 * $Id: RectC.h,v 1.1.1.1 2000/04/25 13:49:01 fnevgeny Exp $
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

#ifndef _RectC_h_
#define _RectC_h_

#include "Base.h"

/*-----------------------------------------------------------------------
 *  Simple rectangle class
 */

class RectC {

public:

   int		xmin, ymin;
   int		xmax, ymax;
   unsigned	wd, ht;

   inline void	Set(int x, int y, unsigned w, unsigned h) {
      xmin = x;
      ymin = y;
      xmax = x + w - 1;
      ymax = y + h - 1;
      wd   = w;
      ht   = h;
   }

   inline void	SetPos(int x, int y) {
      xmin = x;
      ymin = y;
      xmax = x + wd - 1;
      ymax = y + ht - 1;
   }

   inline void	SetSize(unsigned w, unsigned h) {
      xmax = xmin + w - 1;
      ymax = ymin + h - 1;
      wd   = w;
      ht   = h;
   }

   inline RectC() {
      Set(0, 0, 0, 0);
   }

   inline RectC(int x, int y, unsigned w, unsigned h) {
      Set(x, y, w, h);
   }

//
// Does one rectangle overlap the other
//
   inline int	Overlaps(const RectC& r) const {
      int	value = TRUE;
      if ( (xmin > r.xmax) || (r.xmin > xmax) ||
	   (ymin > r.ymax) || (r.ymin > ymax) ) value = FALSE;
      return value;
   }

//
// Does one rectangle overlap the other in the x dimension
//
   inline int	OverlapsX(const RectC& r) const {
      int	value = TRUE;
      if ( (xmin > r.xmax) || (r.xmin > xmax) ) value = FALSE;
      return value;
   }

//
// Does one rectangle overlap the other in the y dimension
//
   inline int	OverlapsY(const RectC& r) const {
      int	value = TRUE;
      if ( (ymin > r.ymax) || (r.ymin > ymax) ) value = FALSE;
      return value;
   }

//
// Does this rectangle contain the specified point
//
   inline int	Inside(int x, int y) const {
      int	value = TRUE;
      if ( (x > xmax) || (x < xmin) ||
           (y > ymax) || (y < ymin) ) value = FALSE;
      return value;
   }

//
// Return the intersection of this rectangle with another
//
   inline RectC	operator|(const RectC& r) const {
      RectC	ir;
      if ( Overlaps(r) ) {
	 ir.xmin = MAX(xmin, r.xmin);
	 ir.ymin = MAX(ymin, r.ymin);
	 ir.xmax = MIN(xmax, r.xmax);
	 ir.ymax = MIN(ymax, r.ymax);
	 ir.wd   = ir.xmax - ir.xmin + 1;
	 ir.ht   = ir.ymax - ir.ymin + 1;
      }
      return ir;
   }

//
// Return the union of this rectangle with another
//
   inline RectC	operator&(const RectC& r) const {
      RectC	ir;
      ir.xmin = MIN(xmin, r.xmin);
      ir.ymin = MIN(ymin, r.ymin);
      ir.xmax = MAX(xmax, r.xmax);
      ir.ymax = MAX(ymax, r.ymax);
      ir.wd   = ir.xmax - ir.xmin + 1;
      ir.ht   = ir.ymax - ir.ymin + 1;
      return ir;
   }

};

#endif // _RectC_h_
