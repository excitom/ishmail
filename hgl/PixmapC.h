/*
 * $Id: PixmapC.h,v 1.1.1.1 2000/04/25 13:49:01 fnevgeny Exp $
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

#ifndef _PixmapC_h_
#define _PixmapC_h_

#include <X11/Intrinsic.h>
#include <stream.h>

//
// This is the bitmap data type
//
typedef struct {
   unsigned		width;
   unsigned		height;
   unsigned char	*bits;
} XbmT;

typedef char	**XpmT;

/*-----------------------------------------------------------------------
 *  This dictionary is used to relate pixmap file names with pixmaps
 */

class PixmapC {

public:

   Pixmap	reg;
   Pixmap	inv;
   Pixmap	mask;
   unsigned	wd;
   unsigned	ht;
   unsigned	depth;
   Screen	*scr;
   Boolean	freeUsingXm;

   inline int	operator==(const PixmapC& p) const {
      return (reg == p.reg && inv == p.inv);
   }
   inline int	operator!=(const PixmapC& p) const { return !(*this==p); }

   void	operator=(const PixmapC&);
   inline int	compare(const PixmapC&) const { return 0; }
   inline int	operator<(const PixmapC& p) const { return (compare(p) < 0); }
   inline int	operator>(const PixmapC& p) const { return (compare(p) > 0); }

   void		Init();
   void		Reset();

//
// Load new data
//
   void		Set(const char*, Pixel, Pixel, Pixel, Pixel, Screen*, Window,
   		    Colormap cmap=(Colormap)NULL);
   void		Set(const XbmT&, Pixel, Pixel, Pixel, Pixel, Screen*, Window);
   void		Set(const XpmT,  Window, Colormap cmap=(Colormap)NULL);

//
// Constructors
//
   PixmapC();
   PixmapC(const PixmapC& p) { *this = p; }
   PixmapC(const char*, Pixel, Pixel, Pixel, Pixel, Screen*, Window,
	   Colormap cmap=(Colormap)NULL);
   PixmapC(const XbmT&, Pixel, Pixel, Pixel, Pixel, Screen*, Window);
   PixmapC(const XpmT,  Window, Colormap cmap=(Colormap)NULL);
   ~PixmapC();

//
// Cast to Pixmap
//
   inline operator      Pixmap() const          { return reg; }

   inline ostream& PrintOn(ostream& strm) const {
      strm <<hex <<"reg: " <<reg <<" inv: " <<inv <<" mask: " <<mask
           <<dec <<" wd: " <<wd <<" ht: " <<ht <<" depth: " <<depth;
      return strm;
   }

};

//
// Method to allow printing of PixmapC
//

inline ostream&
operator<<(ostream& strm, const PixmapC& p)
{
   return p.PrintOn(strm);
}

typedef PixmapC	*PixmapPtrT;

#endif // _PixmapC_h_
