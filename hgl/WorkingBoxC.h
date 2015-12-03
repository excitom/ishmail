/*
 * $Id: WorkingBoxC.h,v 1.1.1.1 2000/04/25 13:49:02 fnevgeny Exp $
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

#ifndef _WorkingBoxC_h_
#define _WorkingBoxC_h_

#include <X11/Intrinsic.h>

class WorkingBoxC {

private:

   static void	Popup (Widget, WorkingBoxC *wb, XtPointer);
   static void	Cancel(Widget, WorkingBoxC *wb, XtPointer);
   void		UpdateDisplay();

   Widget	shell;
   Widget	realParent;
   Widget	dialog;
   Widget	symbol;
   Widget	scaleForm;
   Widget	scale;
   Boolean	cancel;

//
// Data for animated pixmap
//
   Pixmap	*pixmaps;
   int		pixnum;		// Which one is currently displayed
   GC           gc;             // Graphics context for drawing
   Drawable     drawable;       // Window of drawing area
   Position     pixmap_x;       // Position of pixmap within drawing area
   Position     pixmap_y;
   Dimension    pixmap_w;       // Size of pixmap within drawing area
   Dimension    pixmap_h;

   Boolean	allowKeyPress;

public:

   void			Message(const char *msg);
   Boolean		Cancelled();
   void			Show(Widget);
   void			Show();
   void			Hide();
   void			ShowScale();
   void			HideScale();
   void			SetScaleValue(int);

   inline operator	Widget() const		{ return dialog; }

   inline Widget	Dialog() const	{ return dialog; }
   inline Widget	Scale() const	{ return scale; }

   void                 UnmanageButton();
   void                 ManageButton();

   WorkingBoxC(Widget parent);
   ~WorkingBoxC();
};

#endif // _WorkingBoxC_h_
