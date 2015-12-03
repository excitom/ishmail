/*
 * $Id: PixmapC.C,v 1.4 2000/05/31 15:19:46 evgeny Exp $
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

#include "PixmapC.h"
#include "HalAppC.h"

#ifdef HAVE_X11_XPM_H
# include <X11/xpm.h>
#else
# include <xpm.h>
#endif

/*------------------------------------------------------------------------
 * Constructors
 */

PixmapC::PixmapC()
{
   Init();
}

PixmapC::PixmapC(const char *file, Pixel fg, Pixel bg, Pixel ifg, Pixel ibg,
		 Screen *scrn, Window win, Colormap cmap)
{
   Init();
   Set(file, fg, bg, ifg, ibg, scrn, win, cmap);
}

PixmapC::PixmapC(const XbmT& xbm, Pixel fg, Pixel bg, Pixel ifg, Pixel ibg,
		 Screen *scrn, Window win)
{
   Init();
   Set(xbm, fg, bg, ifg, ibg, scrn, win);
}

PixmapC::PixmapC(const XpmT xpm, Window win, Colormap cmap)
{
   Init();
   Set(xpm, win, cmap);
}

/*------------------------------------------------------------------------
 * Destructor
 */

PixmapC::~PixmapC()
{
   Reset();
}

/*------------------------------------------------------------------------
 * Initialize members
 */

void
PixmapC::Init()
{
   reg         = (Pixmap)NULL;
   inv         = (Pixmap)NULL;
   mask        = (Pixmap)NULL;
   wd          = 0;
   ht          = 0;
   depth       = 0;
   scr         = NULL;
   freeUsingXm = False;
}

/*------------------------------------------------------------------------
 * Reset members
 */

void
PixmapC::Reset()
{
   if ( halApp->xRunning ) {

      if ( freeUsingXm ) {
	 if ( reg ) XmDestroyPixmap(scr, reg);
	 if ( inv && inv != reg ) XmDestroyPixmap(scr, inv);
      }
      else {
	 if ( reg ) XFreePixmap(halApp->display, reg);
	 if ( inv && inv != reg ) XFreePixmap(halApp->display, inv);
      }

      if ( mask ) XFreePixmap(halApp->display, mask);
   }

   Init();

} // End Reset

/*------------------------------------------------------------------------
 * Assignment operator
 */

void
PixmapC::operator=(const PixmapC& p)
{
   reg         = p.reg;
   inv         = p.inv;
   mask        = p.mask;
   wd          = p.wd;
   ht          = p.ht;
   depth       = p.depth;
   scr         = p.scr;
   freeUsingXm = p.freeUsingXm;
}

/*------------------------------------------------------------------------
 * Load a file
 */

void
PixmapC::Set(const char *file, Pixel fg, Pixel bg, Pixel ifg, Pixel ibg,
	     Screen *scrn, Window win, Colormap cmap)
{
   Reset();
   scr = scrn;

//
// Try XPM format first.
//

//
// Use XBMLANGPATH TO locate the file
//
   const char	*xbmPath = getenv("XBMLANGPATH");

   SubstitutionRec	subs[1];
   subs[0].match = 'B';
   subs[0].substitution = (char*)file;
   char	*path = XtResolvePathname(halApp->display, "bitmaps", NULL,
      				       NULL, xbmPath, subs, 1, NULL);
   if ( !path ) path = (char*)file;

//
// See if we can read it
//
   XpmAttributes	attr;
   attr.valuemask = XpmColormap;
   if ( cmap == (Colormap)NULL )
      XtVaGetValues(*halApp, XmNcolormap, &attr.colormap, NULL);
   else
      attr.colormap = cmap;
   int status = XpmReadFileToPixmap(halApp->display, win, path, &reg, &mask,
				    &attr);
   if ( status != XpmSuccess ) reg = mask = (Pixmap)NULL;
   inv = reg;

   if ( path != (char*)file ) XtFree(path);

//
// Try XBM format next.
//
   if ( reg == (Pixmap)NULL ) {
      reg = XmGetPixmap(scr, (char*)file, fg, bg);
      if ( reg != XmUNSPECIFIED_PIXMAP) {
         inv = XmGetPixmap(scr, (char*)file, ifg, ibg);
         freeUsingXm = True;
      } else {
         reg = inv = mask = (Pixmap)NULL;
         freeUsingXm = False;
      }
   }

//
// Get pixmap size
//
   if ( reg ) {
      Window	root;
      int	x, y;
      unsigned	bw;
      XGetGeometry(halApp->display, reg, &root, &x, &y, &wd, &ht, &bw, &depth);
   }

} // End Set with file

/*------------------------------------------------------------------------
 * Load a bitmap
 */

void
PixmapC::Set(const XbmT& xbm, Pixel fg, Pixel bg, Pixel ifg, Pixel ibg,
	     Screen *scrn, Window win)
{
   Reset();
   scr = scrn;
   if ( !xbm.bits ) return;

   unsigned	depth = DefaultDepthOfScreen(scr);
   reg = XCreatePixmapFromBitmapData(halApp->display, win, (char *)xbm.bits,
   				     xbm.width, xbm.height, fg, bg, depth);
   inv = XCreatePixmapFromBitmapData(halApp->display, win, (char *)xbm.bits,
   				     xbm.width, xbm.height, ifg, ibg, depth);

//
// Get pixmap size
//
   if ( reg ) {
      Window	root;
      int	x, y;
      unsigned	bw;
      XGetGeometry(halApp->display, reg, &root, &x, &y, &wd, &ht, &bw, &depth);
   }

} // End Set with xbm data

/*------------------------------------------------------------------------
 * Load a pixmap
 */

void
PixmapC::Set(const XpmT xpm, Window win, Colormap cmap)
{
   Reset();
   scr = NULL;
   if ( !xpm ) return;

   XpmAttributes	attr;
   attr.valuemask = XpmColormap;
   if ( cmap == (Colormap)NULL )
      XtVaGetValues(*halApp, XmNcolormap, &attr.colormap, NULL);
   else
      attr.colormap = cmap;
   int	status = XpmCreatePixmapFromData(halApp->display, win, xpm, &reg, &mask,
					 &attr);
   if ( status != XpmSuccess ) reg = mask = (Pixmap)NULL;
   inv = reg;

//
// Get pixmap size
//
   if ( reg ) {
      Window	root;
      int	x, y;
      unsigned	bw;
      XGetGeometry(halApp->display, reg, &root, &x, &y, &wd, &ht, &bw, &depth);
   }

} // End Set with xpm data
