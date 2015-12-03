/*
 * $Id: IconC.h,v 1.1.1.1 2000/04/25 13:49:01 fnevgeny Exp $
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

#ifndef _IconC_h_
#define _IconC_h_

#include "IconDataC.h"
#include "PixmapC.h"
#include "RectC.h"
#include "VItemC.h"
#include "CallbackListC.h"

#include <Xm/DrawingA.h>

class StringC;

/*-----------------------------------------------------------------------
 *  This class is used to display a single large icon with a name beneath
 */

class IconC {

public:

   enum LabelPositionT {
      LABEL_BELOW,
      LABEL_ABOVE,
      LABEL_LEFT,
      LABEL_RIGHT
   };

   IconC(Widget, VItemC *i, ArgList argv=NULL, Cardinal argc=0);
   ~IconC();

   void			Draw();		// Redraw this icon
   void			DrawHighlight(Pixel);
   void			EnableSelection();
   void			DisableSelection();
   void			EnableOpen();
   void			DisableOpen();
   void			Select(Boolean notify=True);
   void			Deselect(Boolean notify=True);
   void			Toggle(Boolean notify=True);
   void			SetLabelPosition(LabelPositionT);
   void			SetLabel(const StringC&);
   void			SetColors(Pixel, Pixel);
   void			SetSelectColors(Pixel, Pixel);
   void         	SetPixmap(const char*);
   void         	SetPixmap(const XbmT*);
   void         	SetPixmap(const XpmT);
   inline Boolean	IsSelected() { return selected; }

   void		printOn(ostream&) const {} // Print information about this icon

//
// Compare two icons
//
   inline int  	operator==(const IconC& i) const {
      return (*data.item == *i.data.item);
   }
   inline int		compare(const IconC& i) const {
      return (data.item->compare(*i.data.item));
   }

//
// Return components
//
   inline operator      Widget() const      { return iconDA; }

   MEMBER_QUERY(Widget,		DrawingArea,	iconDA)
   MEMBER_QUERY(Boolean,	HasFocus,	focusHere)
      PTR_QUERY(PixmapC&,	PixmapData,	pixmap)
   MEMBER_QUERY(Pixel,		RegBgColor,	regBgColor)
   MEMBER_QUERY(Pixel,		RegFgColor,	regFgColor)
   MEMBER_QUERY(Pixel,		HighlightColor,	hlColor)
   MEMBER_QUERY(Dimension,	Width,		daWd)
   MEMBER_QUERY(Dimension,	Height,		daHt)

   inline void	AddFocusChangeCallback(CallbackFn *fn, void *data) {
      AddCallback(focusCalls, fn, data);
   }
   inline void  CallFocusChangeCallbacks() { CallCallbacks(focusCalls, this); }
   inline void	RemoveFocusChangeCallback(CallbackFn *fn, void *data) {
      RemoveCallback(focusCalls, fn, data);
   }

private:

//
// Callbacks
//
   static void  HandleMapChange(Widget, IconC*, XEvent*, Boolean*);
   static void	HandleExpose(Widget, IconC*, XmDrawingAreaCallbackStruct*);
   static void	HandleResize(Widget, IconC*, XmDrawingAreaCallbackStruct*);
   static void  HandleInput (Widget, IconC*, XmDrawingAreaCallbackStruct*);
   static void	HandleFocusChange(Widget, IconC*, XEvent*, Boolean*);


   void		DrawLabel();
   void		GetPixmap();	// Create pixmap for view item if necessary
   void		GetIconSize();	// Get view item
   void  	HandleSingleClick(XButtonEvent*);
   void		HighlightItem();

//
// Drawing area specifics
//
   Widget	iconDA;
   Window	window;
   Boolean      realized;       // True if initialized

//
// Drawing area attributes
//
   Pixel	regFgColor, regBgColor;
   Pixel	invFgColor, invBgColor;
   GC		gc;			// Drawing context
   XFontStruct	*font;
   Boolean	goodFont;		// True if font can be used
   int		labelSpacing;		// Spacing between lines in label
   int		labelOffset;		// Spacing between pixmap and name
   Dimension	itemMarginWd;
   Dimension	itemMarginHt;
   Dimension	daWd;
   Dimension	daHt;
   Dimension	daMarginWd;
   Dimension	daMarginHt;
   LabelPositionT	labelPos;	// Where label is placed

//
// Used in item selection
//
   Pixel	hlColor;		// Highlight color
   int		hlThick;		// Highlight thickness
   RectC	hlRect;
   Boolean	selectionOk;
   Boolean	openOk;

   XtIntervalId	timeOut;		// Id of timeout callback
   int		buttonState;		// Modifiers of button event
   Boolean	selected;
   Boolean	focusHere;	// True if drawing area has focus
   IconDataC	data;
   PixmapC	pixmap;

//
// User callbacks
//
   CallbackListC	focusCalls;
};

#endif // _IconC_h_
