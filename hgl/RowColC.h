/*
 * $Id: RowColC.h,v 1.1.1.1 2000/04/25 13:49:01 fnevgeny Exp $
 *
 * Copyright (c) 1994 HAL Computer Systems International, Ltd.
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

#ifndef _RowColC_h_
#define _RowColC_h_

#include "WidgetListC.h"
#include "IntListC.h"
#include "RowCol.h"
#include "RowColChildListC.h"
#include "RowOrColListC.h"

#include <X11/Intrinsic.h>

class RowColC;

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

class RowC : public RowOrColC {

public:

//
// Methods
//
   RowC(RowColC*);

   void		CalcPrefSize();
   void		PlaceChildren();
   void		SizeChildren();
};

class ColC : public RowOrColC {

public:

//
// Methods
//
   ColC(RowColC*);

   void		CalcPrefSize();
   void		PlaceChildren();
   void		SizeChildren();
};

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

class RowColC {

private:

   friend	RowOrColC;
   friend	RowC;
   friend	ColC;

   Widget		rcw;		// Container widget
   RowColChildListC	childList;	// List of child widgets
   RowOrColListC	rowList;	// List of rows
   RowOrColListC	colList;	// List of columns

   Widget		titleColWidget;	// Container widget for title column
   Widget		titleRowWidget;	// Container widget for title row
   ColC			*titleCol;	// Title column
   RowC			*titleRow;	// Title row

   Boolean		exposed;
   Boolean		deferred;
   Boolean		changed;
   unsigned char	deferCount;	// Depth if nested
   Boolean		selfResize;
   Boolean		inResize;	// Inside HandleResize
   XtIntervalId		refreshTimer;
   Dimension		newWd;		// For programmatic changes
   Dimension		newHt;

//
// Private methods
//
   void		ChildResize(Widget, Dimension, Dimension);
   void		ExposeIt();
   void		FitToHeight(Dimension);
   void		FitToWidth(Dimension);
   Dimension	PrefHeight();
   Dimension	PrefWidth();
   void		ReloadChildren();
   void		Reset();
   void		UpdateHeight();
   void		UpdateWidth();

   static void	HandleExpose(Widget, RowColC*, XtPointer);
   static void	HandleGeometryRequest(Widget, RowColC*, XtPointer);
   static void	HandleMapChange(Widget, RowColC*, XEvent*, Boolean*);
   static void	HandleResize(Widget, RowColC*, XtPointer);
   static void  HandleTitleResize(Widget, RowColC*, XEvent*, Boolean*);
   static void  HandleChildResize(Widget, RowColC*, XtPointer);
   static void	FinishChildResize(RowColC*, XtIntervalId*);

public:

//
// Public Methods
//
   RowColC(Widget, const char*, ArgList argv=NULL, Cardinal argc=0);
   ~RowColC();

   inline operator	Widget() const	{ return rcw; }

   void		AddChild(Widget);
   void		AddChildren(WidgetListC&);
   void		AddChildren(Widget*, int);
   void		AddTitleColChild(Widget);
   void		AddTitleColChildren(WidgetListC&);
   void		AddTitleColChildren(Widget*, int);
   void		AddTitleRowChild(Widget);
   void		AddTitleRowChildren(WidgetListC&);
   void		AddTitleRowChildren(Widget*, int);
   void		Defer(Boolean val=True);
   void		Refresh();
   void		RefreshChildren(RowColChildListC&);
   void		SetChildren(WidgetListC&);
   void		SetChildren(Widget*, int);
   void		SetTitleColChildren(WidgetListC&);
   void		SetTitleColChildren(Widget*, int);
   void		SetTitleRowChildren(WidgetListC&);
   void		SetTitleRowChildren(Widget*, int);

   void		SetColCount(int);
   void		SetRowCount(int);
   void		SetColSpacing(int);
   void		SetRowSpacing(int);
   void		SetMarginWidth(int);
   void		SetMarginHeight(int);
   void		SetOrientation(RcOrientT);
   void		SetResizeHeight(RcResizeT);
   void		SetResizeWidth(RcResizeT);
   void		SetUniformCols(Boolean);
   void		SetUniformRows(Boolean);
   void		SetWidth(int);
   void		SetHeight(int);

   void		SetColAlignment(RcAlignT);
   void		SetColAlignment(int, RcAlignT);
   void		SetColResize(Boolean);
   void		SetColResize(int, Boolean);
   void		SetColVisible(int, Boolean);
   void		SetColWidthAdjust(RcAdjustT);
   void		SetColWidthAdjust(int, RcAdjustT);
   void		SetRowAlignment(RcAlignT);
   void		SetRowAlignment(int, RcAlignT);
   void		SetRowHeightAdjust(RcAdjustT);
   void		SetRowHeightAdjust(int, RcAdjustT);
   void		SetRowResize(Boolean);
   void		SetRowResize(int, Boolean);
   void		SetRowVisible(int, Boolean);

   RcAlignT	ColAlignment(int) const;
   int		ColCount() const;
   Boolean	ColResize(int) const;
   Boolean	ColVisible(int) const;
   RcAdjustT	ColWidthAdjust(int) const;
   int		ChildListIndexOf(Widget) const;
   Boolean	ChildListIncludes(Widget) const;
   RcOrientT	Orientation() const;
   RcResizeT	ResizeHeight() const;
   RcResizeT	ResizeWidth() const;
   RcAlignT	RowAlignment(int) const;
   int		RowCount() const;
   RcAdjustT	RowHeightAdjust(int) const;
   Boolean	RowResize(int) const;
   Boolean	RowVisible(int) const;
   Boolean	UniformCols() const;
   Boolean	UniformRows() const;
};

#endif // _RowColC_h_
