/*
 * $Id: VBoxC.h,v 1.5 2000/08/07 12:36:18 evgeny Exp $
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

#ifndef _VBoxC_h_
#define _VBoxC_h_

#include <X11/cursorfont.h>
#include <Xm/ToggleB.h>
#include <Xm/DragDrop.h>

#include "ViewListC.h"
#include "VItemListC.h"
#include "CallbackListC.h"
#include "PtrListC.h"
#include "RectC.h"

class HalDialogC;
class ViewC;
class FieldViewC;
class IconViewC;
class LgIconViewC;
class SmIconViewC;
class VBoxC;
class JoyStickC;

//
//  This type is used to reference a specific view
//
typedef int	VTypeT;

//
//  This type is used to specify what to do with results of filter
//
typedef enum {

   SELECT_FILTER_OUTPUT,
   DISPLAY_FILTER_OUTPUT

} FilterOutputT;

//
// This is the filter function type
//
typedef Boolean	(*FilterFn)(const VItemC&);
typedef Boolean	(*FindFn)(const VItemC&);

//
// This type is used to send data for a drag event
//
typedef struct {

   VItemListC	itemList;
   Widget       icon;
   Widget       widget;
   VBoxC	*viewBox;
   XEvent       *event;

} ViewDragDataT;

//
// This type is used to send data for a drop event
//
typedef struct {

   VItemC			*item;
   VBoxC			*viewBox;
   XmDropProcCallbackStruct	*procData;

} ViewDropDataT;

/*-----------------------------------------------------------------------
 *  This class is used as the abstract base class for a view
 */

class VBoxC {

public:

   VBoxC(Widget parent, const char *name, ArgList argv=NULL, Cardinal argc=0);
   ~VBoxC();

   VTypeT	AddView(ViewC& view);	// Register a new view with this view
					//  box

//
// Add one or more items to the view box
//
   void		AddItem(VItemC&);
   void		AddItems(VItemListC&);

//
// Remove one or more items from the view box
//
   void		RemoveItem(const StringC&);
   void		RemoveItem(const char*);
   void		RemoveItem(VItemC&);
   void		RemoveItems(VItemListC&);
   void		RemoveSelectedItems();
   void		RemoveAllItems();

//
// Add one or more items to the view box and automatically select them
//
   void		AddItemSelected(VItemC&, Boolean notify=True);
   void		AddItemsSelected(VItemListC&, Boolean notify=True);

//
// Select one or more items
//
   void		SelectItem(VItemC&, Boolean notify=True);
   void		SelectItems(VItemListC&, Boolean notify=True);

//
// Select one or more items and deselect all others
//
   void		SelectItemOnly(VItemC&, Boolean notify=True);
   void		SelectItemsOnly(VItemListC&, Boolean notify=True);

//
// Deselect one or more items
//
   void		DeselectItem(VItemC&, Boolean notify=True);
   void		DeselectItems(VItemListC&, Boolean notify=True);

//
// Select or deselect all items
//
   void		SelectAllItems(Boolean notify=True);
   void		DeselectAllItems(Boolean notify=True);

//
// Toggle selection of one or more items
//
   void		ToggleItem(VItemC&, Boolean notify=True);
   void		ToggleItems(VItemListC&, Boolean notify=True);

//
// Set the visibility of one or more items
//
   void		ShowItem(VItemC&);
   void		ShowItems(VItemListC&);
   void		HideItem(VItemC&);
   void		HideItems(VItemListC&);

//
// Change the label for an item
//
   void		SetItemLabel(VItemC&, const char*);

//
// Change the pixmaps for an item
//
   void		SetItemPixmaps(VItemC&, const char*, const char*);
   void		SetItemPixmaps(VItemC&, const XbmT*, const XbmT*);
   void		SetItemPixmaps(VItemC&, const XpmT,  const XpmT);

//
// Call the open callbacks for an item
//
   void		OpenItem(VItemC&);

   void		Refresh();		// Redraw screen

//
// Add callbacks
//
   inline void	AddSelectChangeCallback(CallbackFn *fn, void *data) {
      AddCallback(selectChangeCalls, fn, data);
   }
   inline void	AddDragCallback(CallbackFn *fn, void *data) {
      AddCallback(dragCalls, fn, data);
   }
   inline void	AddDropCallback(CallbackFn *fn, void *data) {
      AddCallback(dropCalls, fn, data);
   }

//
// Call callbacks
//
   inline void	CallSelectChangeCallbacks() {
      CallCallbacks(selectChangeCalls, this);
   }
   inline void	CallDragCallbacks(ViewDragDataT *data) {
      CallCallbacks(dragCalls, data);
   }
   inline void	CallDropCallbacks(ViewDropDataT *data) {
      CallCallbacks(dropCalls, data);
   }

//
// Remove callbacks
//
   inline void	RemoveSelectChangeCallback(CallbackFn *fn, void *data) {
      RemoveCallback(selectChangeCalls, fn, data);
   }
   inline void	RemoveDragCallback(CallbackFn *fn, void *data) {
      RemoveCallback(dragCalls, fn, data);
   }
   inline void	RemoveDropCallback(CallbackFn *fn, void *data) {
      RemoveCallback(dropCalls, fn, data);
   }

//
// Methods for setting the sort, filter and find dialogs
//
   void		SetSortDialog  (HalDialogC*);
   void		SetFilterDialog(HalDialogC*);
   void		SetFindDialog  (HalDialogC*);

//
// Methods for setting the compare, filter and find functions
//
   void		SetCompareFunction(CompareFn);
   inline void	SetFilterFunction(FilterFn filt)	{ filtFunc = filt; }
   inline void	SetFindFunction(FindFn find)		{ findFunc = find; }

//
// Methods for performing sort, filter and find
//
   void		SetSorted(Boolean);		// Turn sorting on or off
   void		Sort();
   void		Filter();
   void		FilterOutput(FilterOutputT);	// Set output mode
   void		FindFirst();
   void		FindNext();

//
// Modify behavior and appearance
//
   void		DisablePopupMenu();
   void		EnablePopupMenu();
   void		HideStatus();
   void		ShowStatus();

//
// Cast to widget
//
   inline operator	Widget() const		{ return viewForm; }

//
// Return components
//
   MEMBER_QUERY(Widget,		ClipWin,		viewDA)
   MEMBER_QUERY(Dimension,	DAHeight,		visRect.ht)
   MEMBER_QUERY(Dimension,	DAWidth,		visRect.wd)
   MEMBER_QUERY(Boolean,	DragEnabled,		dragEnabled)
      PTR_QUERY(Atom*,		DropAtoms,		dropAtoms)
   MEMBER_QUERY(int,		DropAtomCount,		dropAtomCount)
      PTR_QUERY(CallbackListC&,	DropCallbacks,		dropCalls)
   MEMBER_QUERY(Boolean,	DropEnabled,		dropEnabled)
      PTR_QUERY(HalDialogC&,	FiltDialog,		*filtDialog)
   MEMBER_QUERY(int,		FindBlinkCount,		findBlinkCount)
   MEMBER_QUERY(int,		FindBlinkInterval,	findBlinkInterval)
      PTR_QUERY(HalDialogC&,	FindDialog,		*findDialog)
      PTR_QUERY(XFontStruct*,	Font,			font)
   MEMBER_QUERY(XmFontList,	FontList,		fontList)
   MEMBER_QUERY(Boolean,	GoodFont,		goodFont)
   MEMBER_QUERY(int,		HScrollIncrement,	hScrollInc)
   MEMBER_QUERY(Widget,		HScrollBar,		hScrollBar)
   MEMBER_QUERY(int,		HScrollMaxValue,	hScrollMaxValue)
   MEMBER_QUERY(Boolean,	HScrollOn,		hScrollOn)
   MEMBER_QUERY(int,		HScrollValue,		hScrollValue)
   MEMBER_QUERY(Widget,		ItemBoard,		viewDA)
      PTR_QUERY(VItemListC&,	Items,			items)
      PTR_QUERY(JoyStickC&,	JoyStick,		*joyStick)
   MEMBER_QUERY(Boolean,	Realized,		realized)
   MEMBER_QUERY(Widget,		ScrollForm,		scrollForm)
      PTR_QUERY(VItemListC&,	SelItems,		selItems)
      PTR_QUERY(HalDialogC&,	SortDialog,		*sortDialog)
   MEMBER_QUERY(int,		VScrollIncrement,	vScrollInc)
   MEMBER_QUERY(Widget,		VScrollBar,		vScrollBar)
   MEMBER_QUERY(int,		VScrollMaxValue,	vScrollMaxValue)
   MEMBER_QUERY(Boolean,	VScrollOn,		vScrollOn)
   MEMBER_QUERY(int,		VScrollValue,		vScrollValue)
      PTR_QUERY(ViewC*,		View,			view)
   MEMBER_QUERY(Widget,		ViewDA,			viewDA)
   MEMBER_QUERY(Widget,		ViewForm,		viewForm)
   MEMBER_QUERY(Widget,		ViewFrame,		viewFrame)
   MEMBER_QUERY(GC,		ViewGC,			viewGC)
   MEMBER_QUERY(Widget,		ViewPopup,		viewPopup)
   MEMBER_QUERY(Pixmap,		ViewPm,			viewPm)
   MEMBER_QUERY(VTypeT,		ViewType,		viewType)
   MEMBER_QUERY(Window,		ViewWin,		viewWin)
      PTR_QUERY(VItemListC&,	VisItems,		visItems)
      PTR_QUERY(RectC&,		VisRect,		visRect)
   MEMBER_QUERY(Widget,		FindButton,		findButton)

//
// Set components
//
   inline void	Changed(Boolean val)	{ changed = val; }
   inline void	DisableDrag()			{ EnableDrag(False); }
   inline void	DisableDrop()			{ EnableDrop(False); }
   void		DisableHScroll();
   void		DisableVScroll();
   void		EnableDrag(Boolean val=True);
   void		EnableDrop(Boolean val=True);
   void		EnableHScroll();
   void		EnableVScroll();
   inline void	HScrollValue(int val)		{ hScrollValue = val; }
   inline void	VScrollValue(int val)		{ vScrollValue = val; }
   void		UpdateScrollBars();
   void		SetDropAtoms(Atom*, int);
   void		SetFont(XFontStruct*);
   void		SetFontList(XmFontList);
   void		ViewType(VTypeT);

//
// Perform final updates after selection or deselection
//
   int		partialSelect;
   void		PartialSelect(Boolean);
   void		FinishSelect();

private:

   friend	ViewC;
   friend	FieldViewC;
   friend	IconViewC;
   friend	LgIconViewC;
   friend	SmIconViewC;

   Widget	viewForm;
   Widget	scrollForm;
   Widget	joyStickFrame;
   Widget	viewFrame;
   Widget	viewDA;
   Widget	vScrollBar;
   Widget	hScrollBar;
   Widget	statusForm;
   Widget	totalVal;
   Widget	dispVal;
   Widget	selVal;
   Widget	viewPopup;
   Widget	selectButton;
   Widget	deselectButton;

   Widget       sortButton;
   Widget       filterButton;
   Widget       findButton;

   GC		viewGC;
   Window	viewWin;
   RectC	visRect;
   Pixmap	viewPm;
   unsigned int	viewPmWd;
   unsigned int	viewPmHt;
   XFontStruct	*font;
   XmFontList	fontList;
   Boolean	freeFont;
   Boolean	goodFont;
   Boolean	freeFontList;

   VTypeT	viewType;	// Type of current view
   ViewC	*view;		// Pointer to current view
   ViewListC	views;		// List of registered views
   PtrListC	viewTBList;	// List of view toggle buttons
   VItemListC	items;		// List of all items
   VItemListC	visItems;	// List of visible items
   VItemListC	selItems;	// List of selected items
   JoyStickC	*joyStick;	// The joy stick.

   Boolean	changed;
   Boolean	popupEnabled;
   Boolean	realized;	// True if initialized
   Boolean	statusShown;	// True if status line is displayed

//
// Data for picking and selection
//
   int		hScrollInc;		// Horizontal scroll increment
   int		vScrollInc;		// Vertical scroll increment
   int		hScrollValue;		// Position of horizontal scroll bar
   int		vScrollValue;		// Position of vertical scroll bar
   int		hScrollMaxValue;	// Horizontal scroll bar max - slider
   int		vScrollMaxValue;	// Vertical scroll bar max - slider
   Boolean	hScrollOn;		// True if scrolling enabled
   Boolean	vScrollOn;		// True if scrolling enabled
   Boolean	joyStickOn;		// True if the joy stick is on
   int		scrollRoff;		// Offset used if scrolling
   int		scrollBoff;		// Offset used if scrolling
   int		noScrollRoff;		// Offset used if not scrolling
   int		noScrollBoff;		// Offset used if not scrolling

//
// Call these routines when there is a change in the selected list
//
   CallbackListC	selectChangeCalls;

//
// Call these routines for drag out and drop in events
//
   CallbackListC	dragCalls;
   CallbackListC	dropCalls;
   Boolean		dragEnabled;
   Boolean		dropEnabled;
   Atom			*dropAtoms;
   int			dropAtomCount;
   int			dropAtomAlloc;

   static void		ItemFieldChanged(VItemC*, VBoxC*);
   static void		ItemLabelChanged(VItemC*, VBoxC*);
   static void		ItemPixmapChanged(VItemC*, VBoxC*);

//
// These are the dialogs to be used for sort, filter and find
//
   HalDialogC		*sortDialog;
   HalDialogC		*filtDialog;
   HalDialogC		*findDialog;
   HalDialogC		*defSortDialog;
   HalDialogC		*defFiltDialog;
   HalDialogC		*defFindDialog;

//
// Callbacks for sort, filter and find buttons
//
   static void	DoMoveJoyStick(void*, VBoxC*);
   static void	DoSort(Widget, VBoxC*, XtPointer);
   static void	DoFilt(Widget, VBoxC*, XtPointer);
   static void	DoFind(Widget, VBoxC*, XtPointer);

//
// Functions to be called to during sort and filter
//
   CompareFn	compFunc;
   FilterFn	filtFunc;
   FindFn	findFunc;

//
// Variables for find
//
   int		findPos;
   int		findBlinkCount;
   int		findBlinkInterval;

//
// What to do with filter results
//
   FilterOutputT	filterOutput;   // Display or select output

   static void	HandleMapChange(Widget, VBoxC*, XEvent*, Boolean*);
   static void	HandleExpose(Widget, VBoxC*, XmDrawingAreaCallbackStruct*);
   static void	HandleResize(Widget, VBoxC*, XtPointer);
   static void	HandleInput (Widget, VBoxC*, XEvent*, Boolean*);
   static void	HandleButton1Motion(Widget, VBoxC*, XMotionEvent*, Boolean*);
   static void	HandleDragOver(Widget, XtPointer, XmDragProcCallbackStruct*);
   static void	HandleDropIn  (Widget, XtPointer, XmDropProcCallbackStruct*);
   static void	HandleHScroll(Widget, VBoxC*, XmScrollBarCallbackStruct*);
   static void	HandleVScroll(Widget, VBoxC*, XmScrollBarCallbackStruct*);

   static void	DoSelectAll  (Widget, VBoxC*, XtPointer);
   static void	DoDeselectAll(Widget, VBoxC*, XtPointer);

   static void	ChangeView(Widget, VBoxC*, XmToggleButtonCallbackStruct*);

   static void	SetLabel(Widget, unsigned);

   void		HandleButton2Press(XButtonEvent*);
   void		HandleButton3Press(XButtonEvent*);
   void		ChangeDrop();

//
// Built in views
//
   LgIconViewC	*lgIconView;
   SmIconViewC	*smIconView;

//
// Private versions of public methods.  These methods don't update the screen
//    after doing thier work.
//
   void		_AddItem(VItemC&);
   void		_RemoveItem(VItemC&);
};

#endif // _VBoxC_h_
