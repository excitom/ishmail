/*
 * $Id: Shadow.h,v 1.1.1.1 2000/04/25 13:49:01 fnevgeny Exp $
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

#ifndef _Shadow_h_
#define _Shadow_h_

#include <X11/Intrinsic.h>

   void		DrawShadow(Display*, Drawable, GC, Pixel, Pixel, int, int, int,
			   int, int, unsigned char);
   void		DrawShadowIn(Display*, Drawable, GC, Pixel, Pixel, int, int,
			     int, int, int);
   void		DrawShadowEtchedIn(Display*, Drawable, GC, Pixel, Pixel, int,
				   int, int, int, int);

#endif // _Shadow_h_
