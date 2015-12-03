/*
 * $Id: VBoxC.C,v 1.7 2000/08/07 12:36:18 evgeny Exp $
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
#include "VBoxC.h"
#include "LgIconViewC.h"
#include "SmIconViewC.h"
#include "WArgList.h"
#include "WXmString.h"
#include "rsrc.h"
#include "StringC.h"
#include "SortDialogC.h"
#include "FiltDialogC.h"
#include "FindDialogC.h"
#include "JoyStickC.h"

#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/DrawingA.h>
#include <Xm/ScrollBar.h>
#include <Xm/RowColumn.h>
#include <Xm/Separator.h>
#include <Xm/PushB.h>
#include <Xm/Label.h>

/*-----------------------------------------------------------------------
 *  Constructor for VBoxC
 */

VBoxC::VBoxC(Widget parent, const char *name, ArgList argv, Cardinal argc)
{
   WArgList	args;
   Widget	wlist[7];

   viewGC         = NULL;
   viewPm	  = (Pixmap)NULL;
   dropAtoms      = NULL;
   dropAtomCount  =
   dropAtomAlloc  = 0;
   realized       = False;
   filtFunc       = (FilterFn)NULL;
   findFunc       = (FindFn)NULL;
   compFunc       = (CompareFn)NULL;

   visItems.SetSorted(TRUE);
   selectChangeCalls.AllowDuplicates(TRUE);
   dragCalls.AllowDuplicates(TRUE);

#if 1
   viewForm = XmCreateForm(parent, (char *)name, argv, argc);
#else
   args.Reset();
   for (int i=0; i<argc; i++) args[i] = argv[i];
   args.Orientation(XmVERTICAL);
   args.Packing(XmPACK_TIGHT);
   viewForm = XmCreateRowColumn(parent, (char *)name, ARGS);
#endif

//
// Create the viewForm widget hierarchy
//
// viewForm
//    Form	scrollForm
//    Form	statusForm
//
   args.Reset();
#if 1
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_NONE);
   args.TopAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_FORM);
#endif
   statusForm = XmCreateForm(viewForm, "statusForm", ARGS);

   args.Reset();
#if 1
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   args.TopAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_WIDGET, statusForm);
#endif
   scrollForm = XmCreateForm(viewForm, "scrollForm", ARGS);

//
// Create a parent for the joy stick and then the stick itself.
//
   args.Reset();
   args.LeftAttachment(XmATTACH_NONE);
   args.RightAttachment(XmATTACH_FORM);
   args.TopAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_FORM);
   args.ShadowType(XmSHADOW_IN);
   args.MappedWhenManaged(FALSE);
   args.MarginWidth(0);
   args.MarginHeight(0);
   joyStickFrame = XmCreateFrame(scrollForm, "joyStickFrame", ARGS);

   joyStick = new JoyStickC(joyStickFrame, "joyStick", 0,0);
   XtManageChild(*joyStick);
   joyStickOn = FALSE; 

   joyStick->AddMoveCallback((CallbackFn*)DoMoveJoyStick, this);

//
// Create the scrollForm widget hierarchy
//
// scrollForm
//    Frame		viewFrame
//	 DrawingArea	viewDA
//    ScrollBar		hScrollBar
//    ScrollBar		vScrollBar
//
   args.Reset();
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   args.TopAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_FORM);
   viewFrame = XmCreateFrame(scrollForm, "viewFrame", ARGS);

   args.Reset();
   args.LeftAttachment(XmATTACH_OPPOSITE_WIDGET, viewFrame);
   args.RightAttachment(XmATTACH_OPPOSITE_WIDGET, viewFrame);
   args.TopAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_FORM);
   args.Orientation(XmHORIZONTAL);
   args.ProcessingDirection(XmMAX_ON_RIGHT);
   args.ShowArrows(True);
   args.MappedWhenManaged(False);
   hScrollBar = XmCreateScrollBar(scrollForm, "hScrollBar", ARGS);

   args.Reset();
   args.LeftAttachment(XmATTACH_NONE);
   args.RightAttachment(XmATTACH_FORM);
   args.TopAttachment(XmATTACH_OPPOSITE_WIDGET, viewFrame);
   args.BottomAttachment(XmATTACH_OPPOSITE_WIDGET, viewFrame);
   args.Orientation(XmVERTICAL);
   args.ProcessingDirection(XmMAX_ON_BOTTOM);
   args.ShowArrows(True);
   args.MappedWhenManaged(False);
   vScrollBar = XmCreateScrollBar(scrollForm, "vScrollBar", ARGS);

   hScrollOn       = vScrollOn       = False;
   hScrollValue    = vScrollValue    = 0;
   hScrollMaxValue = vScrollMaxValue = 0;

   args.Reset();
   args.ResizePolicy(XmRESIZE_NONE);
   args.UserData(this);
   viewDA = XmCreateDrawingArea(viewFrame, "viewDA", ARGS);
   XtManageChild(viewDA);

   XtAddCallback(hScrollBar, XmNdecrementCallback,
		 (XtCallbackProc)HandleHScroll, (XtPointer)this);
   XtAddCallback(hScrollBar, XmNdragCallback,
		 (XtCallbackProc)HandleHScroll, (XtPointer)this);
   XtAddCallback(hScrollBar, XmNincrementCallback,
		 (XtCallbackProc)HandleHScroll, (XtPointer)this);
   XtAddCallback(hScrollBar, XmNpageDecrementCallback,
		 (XtCallbackProc)HandleHScroll, (XtPointer)this);
   XtAddCallback(hScrollBar, XmNpageIncrementCallback,
		 (XtCallbackProc)HandleHScroll, (XtPointer)this);
   XtAddCallback(hScrollBar, XmNtoBottomCallback,
		 (XtCallbackProc)HandleHScroll, (XtPointer)this);
   XtAddCallback(hScrollBar, XmNtoTopCallback,
		 (XtCallbackProc)HandleHScroll, (XtPointer)this);
   XtAddCallback(hScrollBar, XmNvalueChangedCallback,
		 (XtCallbackProc)HandleHScroll, (XtPointer)this);

   XtAddCallback(vScrollBar, XmNdecrementCallback,
		 (XtCallbackProc)HandleVScroll, (XtPointer)this);
   XtAddCallback(vScrollBar, XmNdragCallback,
		 (XtCallbackProc)HandleVScroll, (XtPointer)this);
   XtAddCallback(vScrollBar, XmNincrementCallback,
		 (XtCallbackProc)HandleVScroll, (XtPointer)this);
   XtAddCallback(vScrollBar, XmNpageDecrementCallback,
		 (XtCallbackProc)HandleVScroll, (XtPointer)this);
   XtAddCallback(vScrollBar, XmNpageIncrementCallback,
		 (XtCallbackProc)HandleVScroll, (XtPointer)this);
   XtAddCallback(vScrollBar, XmNtoBottomCallback,
		 (XtCallbackProc)HandleVScroll, (XtPointer)this);
   XtAddCallback(vScrollBar, XmNtoTopCallback,
		 (XtCallbackProc)HandleVScroll, (XtPointer)this);
   XtAddCallback(vScrollBar, XmNvalueChangedCallback,
		 (XtCallbackProc)HandleVScroll, (XtPointer)this);

   wlist[0] = viewFrame;
   wlist[1] = hScrollBar;
   wlist[2] = vScrollBar;
   XtManageChildren(wlist, 3);	// scrollForm children

//
// Create the status form widget hierarchy
//
//      statusForm
//         Label	totalLabel
//         Label	totalVal
//         Label	dispLabel
//         Label	dispVal
//         Label	selLabel
//         Label	selVal
//
   Widget	totalLabel = XmCreateLabel(statusForm, "totalLabel", 0,0);

   args.Reset();
   args.LeftAttachment(XmATTACH_WIDGET, totalLabel);
   totalVal = XmCreateLabel(statusForm, "totalVal", ARGS);

   args.LeftWidget(totalVal);
   Widget	dispLabel = XmCreateLabel(statusForm, "dispLabel", ARGS);

   args.LeftWidget(dispLabel);
   dispVal = XmCreateLabel(statusForm, "dispVal", ARGS);

   args.LeftWidget(dispVal);
   Widget	selLabel = XmCreateLabel(statusForm, "selLabel", ARGS);

   args.LeftWidget(selLabel);
   selVal = XmCreateLabel(statusForm, "selVal", ARGS);

   wlist[0] = totalLabel;
   wlist[1] = dispLabel;
   wlist[2] = selLabel;
   wlist[3] = totalVal;
   wlist[4] = dispVal;
   wlist[5] = selVal;
   XtManageChildren(wlist, 6);	// statusForm children

   SetLabel(totalVal, 0);
   SetLabel(dispVal, 0);
   SetLabel(selVal, 0);

   wlist[0] = statusForm;
   wlist[1] = scrollForm;
   XtManageChildren(wlist, 2);	// viewForm

   XtManageChild(joyStickFrame);

   XtManageChild(viewForm);

   statusShown = True;

//
// Create the drawing area widget hierarchy
//
//	viewDA
//         PopupMenu		viewPopup
//
   popupEnabled = get_boolean("VBoxC", viewForm, "popupEnabled", True);
   vScrollInc   = get_int("VBoxC", viewForm, "vJoyStickInc", 5);
   hScrollInc   = get_int("VBoxC", viewForm, "hJoyStickInc", 5);
   viewPopup      = (Widget)NULL;
   selectButton   = (Widget)NULL;
   deselectButton = (Widget)NULL;
   sortButton     = (Widget)NULL;
   filterButton   = (Widget)NULL;
   findButton     = (Widget)NULL;

   if ( popupEnabled ) {

      args.Reset();
      args.RadioBehavior(True);
      viewPopup = XmCreatePopupMenu(viewDA, "viewPopup", ARGS);

//
// Create the popup menu widget hierarchy
//
//	viewPopup
//         PushButton	selectButton
//         PushButton	deselectButton
//	   Separator	sep;
//         PushButton	filterButton
//         PushButton	sortButton
//         PushButton	findButton
//	   Separator	sep;
//
      selectButton   = XmCreatePushButton(viewPopup, "selectButton",   0,0);
      deselectButton = XmCreatePushButton(viewPopup, "deselectButton", 0,0);

      Widget	sep1 = XmCreateSeparator(viewPopup, "sep1", 0,0);

	 sortButton   = XmCreatePushButton(viewPopup, "sortButton", 0,0);
	 filterButton = XmCreatePushButton(viewPopup, "filterButton", 0,0);
	 findButton   = XmCreatePushButton(viewPopup, "findButton", 0,0);


      wlist[0] = selectButton;
      wlist[1] = deselectButton;
      wlist[2] = sep1;
      wlist[3] = sortButton;
      wlist[4] = filterButton;
      wlist[5] = findButton;
      XtManageChildren(wlist, 6);	// viewPopup

      XtAddCallback(selectButton, XmNactivateCallback,
      		    (XtCallbackProc)DoSelectAll, (XtPointer)this);
      XtAddCallback(deselectButton, XmNactivateCallback,
		    (XtCallbackProc)DoDeselectAll, (XtPointer)this);
      XtAddCallback(sortButton, XmNactivateCallback, (XtCallbackProc)DoSort,
		    (XtPointer)this);
      XtAddCallback(filterButton, XmNactivateCallback, (XtCallbackProc)DoFilt,
		    (XtPointer)this);
      XtAddCallback(findButton, XmNactivateCallback, (XtCallbackProc)DoFind,
		    (XtPointer)this);

      XtSetSensitive(selectButton,   False);
      XtSetSensitive(deselectButton, False);

   } // End if the popup menu is enabled

//
// Build default sort, filter and find dialogs
//
   Boolean builtIns = get_boolean("VBoxC", viewForm, "builtInDialogs", True);
   if ( builtIns ) {
      sortDialog = defSortDialog = (HalDialogC *)new SortDialogC(this);
      filtDialog = defFiltDialog = (HalDialogC *)new FiltDialogC(this);
      findDialog = defFindDialog = (HalDialogC *)new FindDialogC(this);
   }
   else {
      sortDialog = defSortDialog = NULL;
      filtDialog = defFiltDialog = NULL;
      findDialog = defFindDialog = NULL;
   }

//
// Initialize variables
//
   changed = False;
   partialSelect = 0;
   filterOutput = DISPLAY_FILTER_OUTPUT;
   findPos = 0;
   findBlinkCount    = get_int("VBoxC", viewForm, "findBlinkCount",    0);
   findBlinkInterval = get_int("VBoxC", viewForm, "findBlinkInterval", 0);
   dragEnabled       = get_boolean("VBoxC", viewForm, "enableDragOut", True);
   dropEnabled       = get_boolean("VBoxC", viewForm, "enableDropIn",  False);

//
// NEW FOR I18N - Check for a fontList resource first.  If it is found, use it
//		  instead of the font resource
//
   
   StringC	fontListName = get_string("VBoxC", viewDA, "fontList");
   Boolean goodFontList = False;
   freeFontList = False;

   if ( fontListName.size() > 0 && fontListName != "none" ) {
      XrmValue fromVal, toVal;
      fromVal.addr = (XPointer)((char*)fontListName);
      fromVal.size = fontListName.size() + 1;
      toVal.addr = (XPointer)&fontList;
      toVal.size = sizeof(XmFontList);
      goodFontList = XtConvertAndStore(viewDA, XmRString, &fromVal, 
				       XmRFontList, &toVal);
      if (goodFontList) {
        freeFontList = True;
      } else {
	fontList = NULL;
      }
   } else {
     fontList = NULL;
   }

   StringC	fontName = get_string("VBoxC", viewDA, "font", "fixed");
   freeFont = True;
   goodFont = True;
  
   font = XLoadQueryFont(halApp->display, fontName);
   if ( !font ) {
      font = halApp->font;
      freeFont = False;
      goodFont = False;
   }

   view = NULL;

//
// Add default view types
//
   VTypeT	ltype, stype;

   Boolean	doit = get_boolean("VBoxC", viewForm, "hasLargeIconView", True);
   if ( doit ) {
      lgIconView = new LgIconViewC(this);
      ltype = AddView(*lgIconView);
   } else {
      lgIconView = NULL;
   }

   doit = get_boolean("VBoxC", viewForm, "hasSmallIconView", True);
   if ( doit ) {
      smIconView = new SmIconViewC(this);
      stype = AddView(*smIconView);
   } else {
      smIconView = NULL;
   }

   if ( lgIconView ) viewType = ltype;
   else if ( smIconView ) viewType = stype;

   XtAddEventHandler(viewForm, StructureNotifyMask, False,
		     (XtEventHandler)HandleMapChange, (XtPointer)this);

} // End Constructor

/*-----------------------------------------------------------------------
 *  Destructor for VBoxC
 */

VBoxC::~VBoxC()
{
//
// Release GC
//
   if ( halApp->xRunning ) {
      if ( viewGC ) XtReleaseGC(halApp->appShell, viewGC);
      if ( freeFont ) XFreeFont(halApp->display, font);
#if 0
      // FIXME: problems with Linux Motif 2.0
      if ( freeFontList ) XmFontListFree(fontList);
#endif
      if (viewPm) XFreePixmap(halApp->display, viewPm);
   }
//
// Delete dialogs
//
   delete defSortDialog;
   delete defFiltDialog;
   delete defFindDialog;

//
// Delete views
//
   delete lgIconView;
   delete smIconView;

   delete dropAtoms;

//
// Delete callback structures
//
   DeleteCallbacks(selectChangeCalls);
   DeleteCallbacks(dragCalls);
   DeleteCallbacks(dropCalls);

} // End Destructor

/*-----------------------------------------------------------------------
 *  Called on initial display
 */

void
VBoxC::HandleMapChange(Widget, VBoxC *This, XEvent *ev, Boolean*)
{
   if ( ev->type != MapNotify ) return;

   XtRemoveEventHandler(This->viewForm, StructureNotifyMask, False,
		        (XtEventHandler)HandleMapChange, (XtPointer)This);

//
// Create graphics contexts for drawing
//
   XtGCMask	modMask = GCClipMask | GCFillStyle | GCFont | GCFunction
			| GCGraphicsExposures | GCLineStyle | GCLineWidth
			| GCPlaneMask | GCForeground | GCArcMode | GCBackground
			| GCCapStyle | GCClipXOrigin | GCClipYOrigin
			| GCDashList | GCDashOffset | GCFillRule | GCJoinStyle
			| GCStipple | GCSubwindowMode | GCTile
			| GCTileStipXOrigin | GCTileStipYOrigin;
   XGCValues fixVals;
   This->viewGC  = XtAllocateGC(This->viewDA, 0, 0, &fixVals, modMask, 0);
   This->viewWin = XtWindow(This->viewDA);

#if 0
// If you use backing store, the window manager "Refresh" function doesn't
//    work

//
// Turn on backing store
//
   unsigned long	winMask = CWBackingStore;
   XSetWindowAttributes	winVals;
   winVals.backing_store = WhenMapped;
   XChangeWindowAttributes(halApp->display, This->viewWin, winMask, &winVals);
#endif

//
// Create a drop site
//
   WArgList	args;
   args.DragProc((XtCallbackProc)HandleDragOver);
   args.DropProc((XtCallbackProc)HandleDropIn);
   args.AnimationStyle(XmDRAG_UNDER_NONE);
   args.DropSiteType(XmDROP_SITE_SIMPLE);
   if ( This->dropEnabled ) {
      args.DropSiteActivity(XmDROP_SITE_ACTIVE);
      args.ImportTargets(This->dropAtoms);
      args.NumImportTargets(This->dropAtomCount);
   } else {
      args.DropSiteActivity(XmDROP_SITE_INACTIVE);
   }
   XmDropSiteRegister(This->viewDA, ARGS);

//
// Finalize layout
//
   Dimension	dawd, daht;
   XtVaGetValues(This->viewDA, XmNwidth, &dawd, XmNheight, &daht, NULL);
   This->visRect.Set(0, 0, dawd, daht);

//
// Get offsets to use if scrollbars are not displayed
//
   XtVaGetValues(This->viewFrame, XmNrightOffset,  &This->noScrollRoff,
				  XmNbottomOffset, &This->noScrollBoff, NULL);

//
// Get size and offset for scrollbars
//
   int		roff, boff;
   Dimension	sbwd, sbht;
   XtVaGetValues(This->vScrollBar, XmNrightOffset,  &roff, XmNwidth,  &sbwd, 0);
   XtVaGetValues(This->hScrollBar, XmNbottomOffset, &boff, XmNheight, &sbht, 0);

//
// Get offsets to use if scrollbars are displayed
//
   This->scrollRoff = This->noScrollRoff + sbwd + roff;
   This->scrollBoff = This->noScrollBoff + sbht + boff;

//
// Set scrollbar values
//
   args.Reset();
   args.Minimum(0);
   args.Maximum(This->DAWidth());
   args.SliderSize(This->DAWidth());
   args.Increment(1);
   args.PageIncrement(This->DAWidth());
   args.Value(0);
   XtSetValues(This->hScrollBar, ARGS);

   args.Increment(1);
   args.Maximum(This->DAHeight());
   args.SliderSize(This->DAHeight());
   args.PageIncrement(This->DAHeight());
   XtSetValues(This->vScrollBar, ARGS);

//
// Create a pixmap for off-screen drawing
//
   unsigned depth = DefaultDepth(halApp->display,
				 DefaultScreen(halApp->display));
   This->viewPmWd = dawd;
   This->viewPmHt = daht;
   This->viewPm = XCreatePixmap(halApp->display, This->viewWin, This->viewPmWd,
				This->viewPmHt, depth);

//
// Fix the values of the joy stick.
//
   XtVaSetValues(This->joyStickFrame, XmNwidth, sbwd, XmNheight, sbht, NULL);
   Pixel clr;
   XtVaGetValues(This->vScrollBar, XmNtroughColor, &clr, NULL);
   XtVaSetValues(*This->joyStick, XmNbackground, clr, NULL);

//
// Add callbacks
//
   XtAddCallback(This->viewDA, XmNresizeCallback, (XtCallbackProc)HandleResize,
		 (XtPointer)This);
   XtAddCallback(This->viewDA, XmNexposeCallback, (XtCallbackProc)HandleExpose,
		 (XtPointer)This);

   XtAddEventHandler(This->viewDA, Button1MotionMask, False,
		     (XtEventHandler)HandleButton1Motion, (XtPointer)This);
   XtAddEventHandler(This->viewDA, ButtonPressMask|ButtonReleaseMask, False,
		     (XtEventHandler)HandleInput, (XtPointer)This);

   This->realized = True;

   if ( This->view ) This->view->Show();

} // End HandleMapChange

/*-----------------------------------------------------------------------
 *  Callback for expose
 */

void
VBoxC::HandleExpose(Widget, VBoxC *This, XmDrawingAreaCallbackStruct *da)
{
   if ( !This->view ) return;
//
// Draw the exposed area
//
   XExposeEvent *ev = (XExposeEvent*)da->event;
   RectC	area(ev->x, ev->y, ev->width, ev->height);
   This->view->Draw(area);

} // End HandleExpose

/*-----------------------------------------------------------------------
 *  Callback for resize
 */

void
VBoxC::HandleResize(Widget, VBoxC *This, XtPointer)
{
   Dimension	wd, ht;
   XtVaGetValues(This->viewDA, XmNwidth, &wd, XmNheight, &ht, NULL);
   This->visRect.Set(0, 0, wd, ht);

//
// Set scrollbar values
//
   int		max;
   WArgList	args;

   XtVaGetValues(This->hScrollBar, XmNmaximum, &max, NULL);
   args.Reset();
   args.SliderSize(wd);
   args.PageIncrement(wd);
   if ( (int)wd > max ) {
      args.Maximum(wd);
      max = wd;
   }
   This->hScrollMaxValue = max - wd;
   if ( This->hScrollValue > This->hScrollMaxValue ) {
      This->hScrollValue = This->hScrollMaxValue;
      args.Value(This->hScrollValue);
   }
   XtSetValues(This->hScrollBar, ARGS);

   XtVaGetValues(This->vScrollBar, XmNmaximum, &max, NULL);
   args.Reset();
   args.SliderSize(ht);
   args.PageIncrement(ht);
   if ( (int)ht > max ) {
      args.Maximum(ht);
      max = ht;
   }
   This->vScrollMaxValue = max - ht;
   if ( This->vScrollValue > This->vScrollMaxValue ) {
      This->vScrollValue = This->vScrollMaxValue;
      args.Value(This->vScrollValue);
   }
   XtSetValues(This->vScrollBar, ARGS);

//
// Re-create pixmap if size has increased
//
   if ( wd > This->viewPmWd || ht > This->viewPmHt ) {

      if (This->viewPm) XFreePixmap(halApp->display, This->viewPm);
      unsigned depth = DefaultDepth(halApp->display,
				    DefaultScreen(halApp->display));
      This->viewPmWd = MAX(wd, This->viewPmWd);
      This->viewPmHt = MAX(ht, This->viewPmHt);
      This->viewPm = XCreatePixmap(halApp->display, This->viewWin,
				   This->viewPmWd, This->viewPmHt, depth);
   }

//
// Resize current view
//
   if ( This->view ) {
      This->view->PlaceItems();
      This->UpdateScrollBars();
      This->view->Redraw();
   }

} // End HandleResize

/*-----------------------------------------------------------------------
 *  Handle button and keyboard events
 */

void
VBoxC::HandleInput(Widget, VBoxC *This, XEvent *ev, Boolean*)
{
   if ( !This->view ) return;

   switch ( ev->type ) {

      case (ButtonPress):
      {
	 XButtonEvent	*be = (XButtonEvent *)ev;
	 switch ( be->button ) {
	    case (Button1): This->view->HandleButton1Press(be); break;
	    case (Button2): This->HandleButton2Press(be); break;
	    case (Button3): This->HandleButton3Press(be); break;
	 }
      } break;

      case (ButtonRelease):
      {
	 XButtonEvent	*be = (XButtonEvent *)ev;
	 if ( be->button == Button1 )
	    This->view->HandleButton1Release(be);
      } break;

      case (KeyPress):
      {
	 XKeyEvent	*ke = (XKeyEvent *)ev;
	 This->view->HandleKeyPress(ke);
      }

   } // End switch event type

} // End HandleInput

/*-----------------------------------------------------------------------
 *  Handle press of mouse button 2
 */

void
VBoxC::HandleButton2Press(XButtonEvent *be)
{
   if ( !dragEnabled ) return;

//
// Translate the position by the scroll factors
//
   be->x += hScrollValue;
   be->y += vScrollValue;

//
// See which icon is under pointer
//
   VItemC	*dragItem = view->PickItem(be->x, be->y);
   if ( !dragItem ) return;

   //cout <<"Starting drag for item " << dragItem NL;
   ViewDragDataT	*dragData = new ViewDragDataT;
   dragData->widget  = viewDA;
   dragData->viewBox = this;
   dragData->event   = (XEvent *)be;

//
// If this icon is selected, drag the entire selection.  If not, select and
//    drag only this icon.
//
   if ( selItems.includes(dragItem) ) dragData->itemList = selItems;
   else	{
      SelectItemOnly(*dragItem);
      dragData->itemList.add(dragItem);
   }

//
// Create widget for drag icon
//
   dragData->icon = view->CreateDragIcon(dragData->itemList);

//
// Call drag callbacks
//
   CallDragCallbacks(dragData);

   //cout <<"Done with drag callbacks" NL;

} // End HandleButton2Press

/*-----------------------------------------------------------------------
 *  Display popup menu
 */

void
VBoxC::HandleButton3Press(XButtonEvent *be)
{
//
// Translate the position by the scroll factors
//
   be->x += hScrollValue;
   be->y += vScrollValue;

//
// See if there is a view item under the cursor
//
   VItemC	*item = view->PickItem(be->x, be->y);
   if ( item && item->HasMenu() ) {
      item->PostMenu(be);
   }

//
// Display the view box menu if no item menu was found
//
   else if ( popupEnabled ) {
      XmMenuPosition(viewPopup, be);
      XtManageChild(viewPopup);
   } else {
      XtUngrabPointer(viewDA, CurrentTime);
   }

} // End HandleButton3Press

/*-----------------------------------------------------------------------
 *  Callback to handle cursor movement while mouse button 1 is pressed
 */

void
VBoxC::HandleButton1Motion(Widget, VBoxC *This, XMotionEvent *ev, Boolean*)
{
   if ( This->view )
      This->view->HandleButton1Motion(ev);
}

/*-----------------------------------------------------------------------
 *  Callback to handle press of "select all"
 */

void
VBoxC::DoSelectAll(Widget, VBoxC *This, XtPointer)
{
   This->SelectAllItems();
}

/*-----------------------------------------------------------------------
 *  Callback to handle press of "deselect all"
 */

void
VBoxC::DoDeselectAll(Widget, VBoxC *This, XtPointer)
{
   This->DeselectAllItems();
}

/*-----------------------------------------------------------------------
 *  Store a number in the given label
 */

#define  VAL_LENGTH	4

void
VBoxC::SetLabel(Widget label, unsigned count)
{
   StringC	val;
   val += (int)count;                // Store count in a string
// Pad string with spaces in front
   while ( val.length() < VAL_LENGTH ) val = " " + val;
   WXmString	wval((char *)val);
   XtVaSetValues(label, XmNlabelString, (XmString)wval, NULL);
}

/*-----------------------------------------------------------------------
 *  Register a new view type
 */

VTypeT
VBoxC::AddView(ViewC& _view)
{
//
// Look for an unused type
//
   VTypeT	type = 0;
   unsigned	count = views.size();
   while ( type<count && views[type] ) type++;

//
// Add a new entry to view dictionary
//
   views.insert(&_view, type);

//
// If this is the first view entry, create the separator.
//
   if ( popupEnabled ) {

      static Widget sep2         = NULL;
      static Widget first_viewTB = NULL;

      if ( views.size() == 1 )
	 sep2 = XmCreateSeparator(viewPopup, "sep2", 0,0);

//
// Add an entry to the popup menu for this type
//
      WArgList	args;
      args.UserData((void *)&_view);
      Widget viewTB = XmCreateToggleButton(viewPopup, _view.ButtonName(), ARGS);
      void	*tmp = viewTB;
      viewTBList.insert(tmp, type);

//
// Set this button if it is the first
//
      if ( views.size() == 1 ) {
	 XmToggleButtonSetState(viewTB, True, False);
	 first_viewTB = viewTB;
      }
      else {
	 XtManageChild(sep2);
	 XtManageChild(first_viewTB);
	 XtManageChild(viewTB);
      }

//
// Add callback for selection
//
      XtAddCallback(viewTB, XmNvalueChangedCallback, (XtCallbackProc)ChangeView,
		    (XtPointer)this);

   } // End if we have a popup menu

//
// Set this is the first view
//
   if ( views.size() == 1 ) {
      view     = &_view;
      viewType = type;
      if ( realized ) view->Show();
   }

   return type;

} // End AddView

/*-----------------------------------------------------------------------
 *  Register a new view type
 */

void
VBoxC::ViewType(VTypeT type)
{
//
// Return if already this type or type out of range
//
   if ( type == viewType || type < 0 || type >= views.size() ) return;

//
// Force change using button in popup menu
//
   if ( viewPopup )
      XmToggleButtonSetState((Widget)*viewTBList[type], True, realized);

   else if ( realized ) {

//
// Hide current view
//
      if ( view ) view->Hide();

//
// Reset scrollbar values
//
      WArgList	args;
      args.Minimum(0);
      args.Maximum(DAWidth());
      args.SliderSize(DAWidth());
      args.Increment(1);
      args.PageIncrement(DAWidth());
      args.Value(0);
      XtSetValues(hScrollBar, ARGS);

      args.Maximum(DAHeight());
      args.SliderSize(DAHeight());
      args.Increment(1);
      args.PageIncrement(DAHeight());
      XtSetValues(vScrollBar, ARGS);
      hScrollValue = vScrollValue = 0;

//
// Get new View
//
      viewType = type;
      view     = views[viewType];
      view->Show();

   } // End if realized

} // End ViewType

/*-----------------------------------------------------------------------
 *  Display a different view
 */

void
VBoxC::ChangeView(Widget w, VBoxC *This, XmToggleButtonCallbackStruct *tb)
{
   if ( tb->set ) {

//
// Hide current view
//
      if ( This->view ) This->view->Hide();

//
// Reset scrollbar values
//
      WArgList	args;
      args.Minimum(0);
      args.Maximum(This->DAWidth());
      args.SliderSize(This->DAWidth());
      args.Increment(1);
      args.PageIncrement(This->DAWidth());
      args.Value(0);
      XtSetValues(This->hScrollBar, ARGS);

      args.Maximum(This->DAHeight());
      args.SliderSize(This->DAHeight());
      args.Increment(1);
      args.PageIncrement(This->DAHeight());
      XtSetValues(This->vScrollBar, ARGS);
      This->hScrollValue =
      This->vScrollValue = 0;

//
// Get new View from user data and display it
//
      XtVaGetValues(w, XmNuserData, &This->view, NULL);
      This->view->Show();

//
// Save the new view type
//
      This->viewType = This->views.indexOf(This->view);

   } // End if button is set

} // End ChangeView

/*-----------------------------------------------------------------------
 *  Public method to add a new item to this view box
 */

void
VBoxC::AddItem(VItemC& item)
{
   if ( items.includes(&item) ) return;

//
// Add it to the lists
//
   _AddItem(item);

//
// Add item to current view
//
   if ( realized && view ) view->AddItem(item);

   if ( selItems.includes(&item) ) {
      item.CallSelectCallbacks();
      FinishSelect();
   }

   changed = True;

} // End AddItem

/*-----------------------------------------------------------------------
 *  Private method to add a new item to this view box
 */

void
VBoxC::_AddItem(VItemC& item)
{
//
// Set compare function
//
   item.SetCompareFunction(compFunc);

//
// Add callbacks
//
   item.AddFieldChangeCallback ((CallbackFn *)ItemFieldChanged,  this);
   item.AddLabelChangeCallback ((CallbackFn *)ItemLabelChanged,  this);
   item.AddPixmapChangeCallback((CallbackFn *)ItemPixmapChanged, this);

//
// Add it to the list
//
   items.append(&item);

//
// If it passes the filter, add it to the visible list and/or selected list
//
   if ( !filtFunc ) {
      visItems.add(&item);
   } else {
      if ( filterOutput == DISPLAY_FILTER_OUTPUT ) {
	 if ( (*filtFunc)(item) ) {
	    visItems.add(&item);
	 }
      } else /* if ( filterOutput == SELECT_FILTER_OUTPUT ) */ {
	 visItems.add(&item);
	 if ( (*filtFunc)(item) ) {
	    selItems.append(&item);
	 }
      }
   }

} // End _AddItem

/*-----------------------------------------------------------------------
 *  Add new items to this view box
 */

void
VBoxC::AddItems(VItemListC& list)
{
//
// Remove items we already know about
//
   VItemListC	newItems;
   unsigned	count = list.size();
   int	i;
   for (i=0; i<count; i++) {
      VItemC	*item = list[i];
      if ( !items.includes(item) ) newItems.add(item);
   }

   if ( newItems.size() == 0 ) return;

   PartialSelect(True);

//
// Add items
//
   count = newItems.size();
   for (i=0; i<count; i++) {
      VItemC	*item = newItems[i];
      _AddItem(*item);
      if ( selItems.includes(item) ) {
	 item->CallSelectCallbacks();
      }
   }

//
// Add items to current view
//
   if ( realized && view ) view->AddItems(list);
   PartialSelect(False);

   changed = True;

} // End AddItems

/*-----------------------------------------------------------------------
 *  Public method to add a new selected item to this view box
 */

void
VBoxC::AddItemSelected(VItemC& item, Boolean notify)
{
   if ( items.includes(&item) ) return;

//
// Add it to the lists
//
   _AddItem(item);

//
// If it is not in the selected list, add it
//
   if ( !selItems.includes(&item) ) {
      selItems.append(&item);
   }

//
// Add it to the current view
//
   if ( realized && view ) view->AddItem(item);

   if ( notify ) item.CallSelectCallbacks();
   FinishSelect();

   changed = True;

} // End AddItemSelected

/*-----------------------------------------------------------------------
 *  Add new selected items to this view box
 */

void
VBoxC::AddItemsSelected(VItemListC& list, Boolean notify)
{
//
// Remove items we already know about
//
   VItemListC	newItems;
   unsigned	count = list.size();
   int	i;
   for (i=0; i<count; i++) {
      VItemC	*item = list[i];
      if ( !items.includes(item) ) newItems.add(item);
   }

   if ( newItems.size() == 0 ) return;

   PartialSelect(True);

   count = newItems.size();
   for (i=0; i<count; i++) AddItemSelected(*list[i], notify);

//
// Add items to the current view
//
   if ( realized && view ) view->AddItems(list);
   PartialSelect(False);

   changed = True;

} // End AddItemsSelected

/*-----------------------------------------------------------------------
 *  Public method to select the given item
 */

void
VBoxC::SelectItem(VItemC& item, Boolean notify)
{
//
// Return if this item is already selected or is not visible
//
   if ( selItems.includes(&item) ||
       !visItems.includes(&item) ) return;

//
// Add this item to the selected list
//
   selItems.append(&item);

//
// Redraw this item in the current view
//
   changed = True;
   if ( realized && view ) view->RedrawItem(item);

//
// Call the selection callbacks for the item
//
   if ( notify ) item.CallSelectCallbacks();

//
// Perform final updates after selection
//
   FinishSelect();

} // End SelectItem

/*-----------------------------------------------------------------------
 *  Select the given items
 */

void
VBoxC::SelectItems(VItemListC& list, Boolean notify)
{
   unsigned	count = list.size();
   if ( count == 0 ) return;

   if ( count == 1 )
      SelectItem(*list[0], notify);

   else {

      PartialSelect(True);

      for (int i=0; i<count; i++)
	 SelectItem(*list[i], notify);

      PartialSelect(False);
   }

} // End SelectItems

/*-----------------------------------------------------------------------
 *  Select the given item and deselect all others
 */

void
VBoxC::SelectItemOnly(VItemC& item, Boolean notify)
{
//
// Loop through selected list and deselect all items except this one
//
   if ( selItems.includes(&item) ) {

      VItemListC	deselectList;
      unsigned		count = selItems.size();
      for (int i=0; i<count; i++) {
	 VItemC	*sitem = selItems[i];
	 if ( sitem != &item )
	    deselectList.append(sitem);
      }

      count = deselectList.size();
      if ( count == 1 )
	 DeselectItem(*deselectList[0], notify);
      else
	 DeselectItems(deselectList, notify);
   }

//
// Item is not selected, so deselect them all and select this one
//
   else {

      if ( selItems.size() <= 1 ) {
	 if ( selItems.size() == 1 )
	    DeselectItem(*selItems[0], notify);
	 SelectItem(item, notify);
      }

      else {

	 PartialSelect(True);

	 DeselectAllItems(notify);
	 SelectItem(item, notify);

	 PartialSelect(False);
      }

   } // End if item not already selected

} // End SelectItemOnly

/*-----------------------------------------------------------------------
 *  Select the given items and deselect all others
 */

void
VBoxC::SelectItemsOnly(VItemListC& list, Boolean notify)
{
   if ( list.size() == 1 )
      SelectItemOnly(*list[0], notify);

   else {
      PartialSelect(True);

//
// Loop through selected list and deselect all items except the requested ones
//
      VItemListC	deselectList;
      u_int		count = selItems.size();
      for (int i=0; i<count; i++) {
	 VItemC	*item = selItems[i];
	 if ( !list.includes(item) ) {
	    deselectList.append(item);
	 }
      }

      count = deselectList.size();
      if ( count == 1 )
	 DeselectItem(*deselectList[0], notify);
      else
	 DeselectItems(deselectList, notify);

//
// Now select the requested items
//
      SelectItems(list, notify);

      PartialSelect(False);
   }

} // End SelectItemsOnly

/*-----------------------------------------------------------------------
 *  Public method to deselect the given item
 */

void
VBoxC::DeselectItem(VItemC& item, Boolean notify)
{
//
// Return if this item is not selected
//
   if ( !selItems.includes(&item) ) return;

//
// Remove this item from the selected list
//
   selItems.remove(&item);

//
// Redraw this item in the current view
//
   changed = True;
   if ( realized && view ) view->RedrawItem(item);

//
// Call the deselection callbacks for the item
//
   if ( notify ) item.CallDeselectCallbacks();

//
// Perform final updates after deselection
//
   FinishSelect();

} // End DeselectItem

/*-----------------------------------------------------------------------
 *  Deselect the given items
 */

void
VBoxC::DeselectItems(VItemListC& list, Boolean notify)
{
   u_int	count = list.size();
   if ( count == 0 ) return;

   if ( count == 1 )
      DeselectItem(*list[0], notify);

   else {

      PartialSelect(True);

      for (int i=0; i<count; i++)
	 DeselectItem(*list[i], notify);

      PartialSelect(False);
   }

} // End DeselectItems

/*-----------------------------------------------------------------------
 *  Select all unselected items
 */

void
VBoxC::SelectAllItems(Boolean notify)
{
   SelectItems(visItems, notify);
}

/*-----------------------------------------------------------------------
 *  Deselect all selected items
 */

void
VBoxC::DeselectAllItems(Boolean notify)
{
   u_int	count = selItems.size();
   if ( count == 0 ) return;

   if ( count == 1 )
      DeselectItem(*selItems[0], notify);

   else {

//
// Copy the selected list since it will get modified in the process
// Copy it in reverse order so that items will be removed from the end
//
      VItemListC	deselItems;
      for (int i=count-1; i>=0; i--) deselItems.append(selItems[i]);
      DeselectItems(deselItems, notify);
   }

} // End DeselectAllItems

/*-----------------------------------------------------------------------
 *  Toggle the selection of the given item
 */

void
VBoxC::ToggleItem(VItemC& item, Boolean notify)
{
//
// Deselect if selected
//
   if ( selItems.includes(&item) ) {
      DeselectItem(item, notify);
   } else {
      SelectItem(item, notify);
   }

} // End ToggleItem

/*-----------------------------------------------------------------------
 *  Toggle the selection of the given items
 */

void
VBoxC::ToggleItems(VItemListC& list, Boolean notify)
{
   PartialSelect(True);

   unsigned	count = list.size();
   for (int i=0; i<count; i++) {
      VItemC	*item = list[i];
      ToggleItem(*item, notify);
   }

   PartialSelect(False);

} // End ToggleItems

/*-----------------------------------------------------------------------
 *  Make an item visible
 */

void
VBoxC::ShowItem(VItemC& item)
{
//
// Return if already visible
//
   if ( visItems.includes(&item) ) return;

//
// Add item to list
//
   visItems.add(&item);

//
// Tell current view about change
//
   if ( realized && view ) view->ChangeItemVis(item);

   changed = True;

} // End ShowItem

/*-----------------------------------------------------------------------
 *  Make several items visible
 */

void
VBoxC::ShowItems(VItemListC& list)
{
//
// Set visibility for each item
//
   unsigned	count = list.size();
   for (int i=0; i<count; i++) {
      ShowItem(*list[i]);
   }

} // End ShowItems

/*-----------------------------------------------------------------------
 *  Make an item invisible
 */

void
VBoxC::HideItem(VItemC& item)
{
//
// Return if not currently visible
//
   if ( !visItems.includes(&item) ) return;

//
// Remove item from list
//
   visItems.remove(&item);

//
// Tell current view about change
//
   if ( realized && view ) view->ChangeItemVis(item);

   changed = True;

} // End HideItem

/*-----------------------------------------------------------------------
 *  Make several items invisible
 */

void
VBoxC::HideItems(VItemListC& list)
{
//
// Remove items from visible list
//
   unsigned	count = list.size();
   int	i;
   for (i=0; i<count; i++) {
      visItems.remove(list[i]);
   }

//
// Tell current view about change
//
   if ( realized && view ) {
      //view->ChangeItemVis();
      for (i=0; i<count; i++) view->ChangeItemVis(*list[i]);
   }

   changed = True;

} // End HideItems

/*-----------------------------------------------------------------------
 *  Open the given item
 */

void
VBoxC::OpenItem(VItemC& item)
{
//
// Call the open callbacks for the item
//
   item.CallOpenCallbacks();

} // End OpenItem


/*-----------------------------------------------------------------------
 *  Refresh the box
 */

void
VBoxC::Refresh()
{
   if ( realized && view ) {
      view->PlaceItems();
      UpdateScrollBars();
      view->Draw(visRect);
   }

   SetLabel(totalVal, items.size());
   SetLabel(dispVal, visItems.size());
   SetLabel(selVal, selItems.size());
   if ( selectButton )
      XtSetSensitive(selectButton, visItems.size() &&
      				   (selItems.size() != visItems.size()));

} // End Refresh

/*----------------------------------------------------------------------
 *  Set a flag so that routines know that more items are going to be
 *     selected.  This keeps work from being repeated and the select change
 *     callbacks from being called too many times.
 */

void
VBoxC::PartialSelect(Boolean on)
{
   if ( on ) {

      partialSelect++;		// Up the count

   } else if ( partialSelect > 0 ) {

      partialSelect--;		// Lower the count
      FinishSelect();		// Only does work if partialSelect==0
   }

} // End PartialSelect

/*-----------------------------------------------------------------------
 *  Perform final updates after selection
 */

void
VBoxC::FinishSelect()
{
//
// If there is more to be selected, just return
//
   if ( partialSelect>0 ) return;

//
// Update the status label
//
   SetLabel(selVal, selItems.size());

//
// Update the button sensitivities
//
   if ( selectButton ) {
      XtSetSensitive(selectButton,
		     visItems.size() && (selItems.size() != visItems.size()));
      XtSetSensitive(deselectButton, selItems.size() > 0 );
   }

//
// Redraw the box box
//
   Refresh();

//
// Call the selection-change callbacks for view box
//
   CallSelectChangeCallbacks();

} // End FinishSelect

/*-----------------------------------------------------------------------
 *  Specify a new dialog for the sort function
 */

void
VBoxC::SetSortDialog(HalDialogC *ad)
{
   Boolean	shown = False;
   if ( sortDialog && sortDialog->IsShown() ) {
      sortDialog->Hide();
      shown = True;
   }

   sortDialog = ad ? ad : defSortDialog;
   if ( shown ) ad->Show();

} // End SetSortDialog

/*-----------------------------------------------------------------------
 *  Specify a new dialog for the filter function
 */

void
VBoxC::SetFilterDialog(HalDialogC *ad)
{
   Boolean	shown = False;
   if ( filtDialog && filtDialog->IsShown() ) {
      filtDialog->Hide();
      shown = True;
   }

   filtDialog = ad ? ad : defFiltDialog;
   if ( shown ) ad->Show();

} // End SetFilterDialog

/*-----------------------------------------------------------------------
 *  Specify a new dialog for the find function
 */

void
VBoxC::SetFindDialog(HalDialogC *ad)
{
   Boolean	shown = False;
   if ( findDialog && findDialog->IsShown() ) {
      findDialog->Hide();
      shown = True;
   }

   findDialog = ad ? ad : defFindDialog;
   if ( shown ) ad->Show();

} // End SetFindDialog

/*-----------------------------------------------------------------------
 *  Specify a new comparison function for view items
 */

void
VBoxC::SetCompareFunction(CompareFn comp)
{
   //cout <<"Setting compFunc to " <<(void*)comp <<endl;
   compFunc = comp;

//
// Set compare function for view items
//
   unsigned	count = items.size();
   for (int i=0; i<count; i++) {
      items[i]->SetCompareFunction(comp);
   }

} // End SetCompareFunction

/*-----------------------------------------------------------------------
 *  This method resorts the visible items
 */

void
VBoxC::Sort()
{
   if ( !compFunc ) return;

   visItems.sort(compFunc);

   changed = True;

} // End Sort

/*-----------------------------------------------------------------------
 *  Change the filter output option
 */

void
VBoxC::FilterOutput(FilterOutputT type)
{
   if ( filterOutput == type ) return;

   filterOutput = type;

} // End FilterOutput

/*-----------------------------------------------------------------------
 *  Filter the list
 */

void
VBoxC::Filter()
{
   if ( !filtFunc ) return;

//
// See if we're displaying or selecting
//
   if ( filterOutput == DISPLAY_FILTER_OUTPUT ) {

//
// Pass all items through filter.  Create lists of items that need to be
//   shown and those that need to be hidden
//
      VItemListC	showList, hideList;

      int	count = items.size();
      for (int i=0; i<count; i++) {

	 VItemC	*item = items[i];
//
// If item passes, make it visible
//
	 if ( (*filtFunc)(*item) ) {
	    showList.append(item);
	 } else {
	    hideList.append(item);
	 }

      } // End for each item

//
// Update view
//
      DeselectItems(hideList);		// Hidden items cannot be selected
      HideItems(hideList);
      ShowItems(showList);

   } // End if display output

   else /* if ( filterOutput == SELECT_FILTER_OUTPUT ) */ {

//
// Make all items visible
//
      ShowItems(items);

//
// Pass all items through filter.  Keep a list of items that need to be
//    selected
//
      VItemListC	selList;

      unsigned	count = items.size();
      for (int i=0; i<count; i++) {

	 VItemC	*item = items[i];

//
// Apply filter.  If item passes, select it.
//
	 if ( (*filtFunc)(*item) ) {
	    selList.append(item);
	 }

      } // End for each view item

//
// Make selection changes
//
      SelectItemsOnly(selList);

   } // End if select output

} // End Filter

/*-----------------------------------------------------------------------
 *  Find the first item that passes the find proc
 */

void
VBoxC::FindFirst()
{
//
// Reset the search position and call FindNext
//
   findPos = 0;
   FindNext();

} // End FindFirst

/*-----------------------------------------------------------------------
 *  Find the next item that passes the find proc
 */

void
VBoxC::FindNext()
{
   if ( !findFunc ) return;

   unsigned	count = visItems.size();
   unsigned	remaining = count;
   Boolean	found = False;
   VItemC	*item;

//
// Loop through visible items, starting at the current search pos.  Wrap at
//   the end of the list.  Stop when the current pos is reached again.
//
   while ( !found && remaining>0 ) {

     if(findPos > visItems.size())findPos = 0;
      item = visItems[findPos];

      if ( (*findFunc)(*item) ) {
	 if ( view ) view->FlashItem(item);
         found = True;
      }

//
// Advance to next item
//
      findPos++;
      if ( findPos >= count ) findPos = 0;
      remaining--;

   } // End for each view item

} // End FindNext

/*-----------------------------------------------------------------------
 *  Change the label string for a view item
 */

void
VBoxC::SetItemLabel(VItemC& item, const char *lab)
{
   item.Label(lab);
}

/*-----------------------------------------------------------------------
 *  Change the pixmaps for a view item
 */

void
VBoxC::SetItemPixmaps(VItemC& item, const char *lg, const char *sm)
{
   item.SetPixmaps(lg, sm);
}

void
VBoxC::SetItemPixmaps(VItemC& item, const XbmT *lg, const XbmT *sm)
{
   item.SetPixmaps(lg, sm);
}

void
VBoxC::SetItemPixmaps(VItemC& item, const XpmT lg, const XpmT sm)
{
   item.SetPixmaps(lg, sm);
}

/*-----------------------------------------------------------------------
 *  Public method to remove an item from this view box
 */

void
VBoxC::RemoveItem(const char *name)
{
   StringC	tname(name);
   RemoveItem(tname);

} // End RemoveItem

void
VBoxC::RemoveItem(const StringC& name)
{
//
// Find all items with the given name
//
   VItemListC	list;
   unsigned	count = items.size();
   for (int i=0; i<count; i++) {
      VItemC	*item = items[i];
      if ( item->Name() == name ) {
	 list.append(item);
      }
   }

//
// Remove the matching items
//
   if ( list.size() > 0 ) {
      RemoveItems(list);
   }

} // End RemoveItem

void
VBoxC::RemoveItem(VItemC& item)
{
   Boolean	wasSelected = selItems.includes(&item);

//
// Remove it from the lists
//
   _RemoveItem(item);

//
// Remove item from current view
//
   if ( realized && view ) view->RemoveItem(item);
   if ( wasSelected ) FinishSelect();

   changed = True;

} // End RemoveItem

/*-----------------------------------------------------------------------
 *  Private method to remove an item from this view box
 */

void
VBoxC::_RemoveItem(VItemC& item)
{
   items.remove(&item);
   visItems.remove(&item);
   selItems.remove(&item);

//
// Remove callbacks
//
   item.RemoveFieldChangeCallback ((CallbackFn *)ItemFieldChanged,  this);
   item.RemoveLabelChangeCallback ((CallbackFn *)ItemLabelChanged,  this);
   item.RemovePixmapChangeCallback((CallbackFn *)ItemPixmapChanged, this);

} // End _RemoveItem

/*-----------------------------------------------------------------------
 *  Remove items from this view box
 */

void
VBoxC::RemoveItems(VItemListC& list)
{
//
// Remove items from current view
//
   if ( realized && view ) view->RemoveItems(list);

//
// Remove items from lists
//
   unsigned	selCount = selItems.size();
   unsigned	count = list.size();
   for (int i=count-1; i>=0; i--) {
      VItemC	*item = list[i];
      _RemoveItem(*item);
   }

   if ( selCount != selItems.size() ) FinishSelect();

   changed = True;

} // End RemoveItems

/*-----------------------------------------------------------------------
 *  Remove all selected items from this view box
 */

void
VBoxC::RemoveSelectedItems()
{
//
// Copy the selected list since it will get modified in the process
// Copy it in reverse order so that items will be removed from the end
//
   VItemListC	remItems;
   unsigned	count = selItems.size();
   for (int i=count-1; i>=0; i--) {
      remItems.append(selItems[i]);
   }

   RemoveItems(remItems);

} // End RemoveSelectedItems

/*-----------------------------------------------------------------------
 *  Remove all items from this view box
 */

void
VBoxC::RemoveAllItems()
{
//
// Remove items from current view
//
   if ( realized && view ) {
      view->RemoveItems(items);
   }

//
// Remove callbacks
//
   unsigned	selCount = selItems.size();
   unsigned	count    = items.size();
   for (int i=0; i<count; i++) {
      VItemC	*item = items[i];
      item->RemoveFieldChangeCallback ((CallbackFn *)ItemFieldChanged,  this);
      item->RemoveLabelChangeCallback ((CallbackFn *)ItemLabelChanged,  this);
      item->RemovePixmapChangeCallback((CallbackFn *)ItemPixmapChanged, this);
   }

//
// Remove items from lists
//
   items.removeAll();
   visItems.removeAll();
   selItems.removeAll();

   if ( selCount ) FinishSelect();

   changed = True;

} // End RemoveAllItems

/*-----------------------------------------------------------------------
 * Callbacks for sort, filter and find buttons
 */

void
VBoxC::DoSort(Widget, VBoxC *This, XtPointer)
{
   This->sortDialog->Show();
}

void
VBoxC::DoFilt(Widget, VBoxC *This, XtPointer)
{
   This->filtDialog->Show();
}

void
VBoxC::DoFind(Widget, VBoxC *This, XtPointer)
{
   This->findDialog->Show();
}

void
VBoxC::DoMoveJoyStick(void*,  VBoxC* This)
{
   int px, py;
   This->joyStick->GetMovePercentage(&px, &py);

   int new_x = This->hScrollValue;
   int new_y = This->vScrollValue;

   if ( This->hScrollInc > 1 ) {
      float dx = ((float)px*(float)This->hScrollInc) * 0.01;
      new_x += (int)(dx+0.5);
   }
   else if ( px > 0 ) {
      new_x += 1;
   }
   else if ( px < 0 ) {
      new_x -= 1;
   }

   if ( This->vScrollInc > 1 ) {
      float dy = ((float)py*(float)This->vScrollInc) * 0.01;
      new_y += (int)(dy+0.5);
   }
   else if ( py > 0 ) {
      new_y += 1;
   }
   else if ( py < 0 ) {
      new_y -= 1;
   }

//
// Sanity check.
//
   if ( new_x <= 0 )
      new_x = 0;
   else if ( new_x > This->hScrollMaxValue )
      new_x = This->hScrollMaxValue;

   if ( new_y <= 0 )
      new_y = 0;
   else if ( new_y > This->vScrollMaxValue )
      new_y = This->vScrollMaxValue;

//
// See if the position has changed.
//
   int moved = 0;
   if ( new_x != This->hScrollValue ) {
      moved = 1;
      This->hScrollValue = new_x;
      if ( This->hScrollOn )
         XtVaSetValues(This->hScrollBar, XmNvalue, This->hScrollValue, NULL);
   }
   if ( new_y != This->vScrollValue ) {
      moved = 1;
      This->vScrollValue = new_y;
      if ( This->vScrollOn )
         XtVaSetValues(This->vScrollBar, XmNvalue, This->vScrollValue, NULL);
   }

//
// Redraw if necessary.
//
   if ( moved || This->joyStick->State() == RELEASE_JOYSTICK ) {
      This->view->Redraw();
   }

}  // End DoMoveJoyStick()


/*-----------------------------------------------------------------------
 *  Control access to popup menu
 */

void
VBoxC::DisablePopupMenu()
{
   popupEnabled = False;
}

void
VBoxC::EnablePopupMenu()
{
   if ( !viewPopup ) return;

   popupEnabled = True;
}

/*-----------------------------------------------------------------------
 *  Control visibility of status line
 */

void
VBoxC::HideStatus()
{
   if ( !statusShown ) return;

#if 1
   WArgList	args;
   args.BottomAttachment(XmATTACH_FORM);
   XtSetValues(scrollForm, ARGS);
#endif

   XtUnmanageChild(statusForm);

   statusShown = False;

} // End ShowStatus

void
VBoxC::ShowStatus()
{
   if ( statusShown ) return;

   XtManageChild(statusForm);

#if 1
   WArgList	args;
   args.BottomAttachment(XmATTACH_WIDGET, statusForm);
   XtSetValues(scrollForm, ARGS);
#endif

   statusShown = True;

} // End HideStatus

/*-----------------------------------------------------------------------
 *  React to view item changes
 */

void
VBoxC::ItemFieldChanged(VItemC *item, VBoxC *This)
{
   if ( This->realized && This->view ) This->view->ChangeItemFields(*item);
}

void
VBoxC::ItemLabelChanged(VItemC *item, VBoxC *This)
{
   if ( This->realized && This->view ) This->view->ChangeItemLabel(*item);
}

void
VBoxC::ItemPixmapChanged(VItemC *item, VBoxC *This)
{
   if ( This->realized && This->view ) This->view->ChangeItemPixmaps(*item);
}

/*-----------------------------------------------------------------------
 *  React to view item changes
 */

void
VBoxC::SetSorted(Boolean val)
{
//
// Rebuild the visible items list
//
   visItems.removeAll();

   u_int	count = items.size();
   for (int i=0; i<count; i++) {

      VItemC	*item = items[i];

//
// If it passes the filter, add it to the visible list
//
      if ( !filtFunc ) {
	 visItems.add(item);
      }
      else if ( filterOutput == DISPLAY_FILTER_OUTPUT ) {
	 if ( (*filtFunc)(*item) ) visItems.add(item);
      }
      else /* if ( filterOutput == SELECT_FILTER_OUTPUT ) */ {
	 visItems.add(item);
      }

   } // End for each view item

   visItems.SetSorted(val);
   
   changed = True;

} // End SetSorted

/*-----------------------------------------------------------------------
 *  Change state of drag and drop handling
 */

void
VBoxC::EnableDrag(Boolean val)
{
   if ( dragEnabled == val ) return;

   dragEnabled = val;
}

void
VBoxC::EnableDrop(Boolean val)
{
   if ( dropEnabled == val ) return;

   dropEnabled = val;
   ChangeDrop();
}

/*-----------------------------------------------------------------------
 *  Method to set the types of items that can be dropped in this view box
 */

void
VBoxC::SetDropAtoms(Atom *atoms, int count)
{
   if ( count > dropAtomAlloc ) {
      delete dropAtoms;
      dropAtoms = new Atom[count];
      dropAtomAlloc = count;
   }

   for (int i=0; i<count; i++) dropAtoms[i] = atoms[i];
   dropAtomCount = count;

   ChangeDrop();
}

/*-----------------------------------------------------------------------
 *  Method to update drag and drop status
 */

void
VBoxC::ChangeDrop()
{
   if ( !realized ) return;

   WArgList	args;

   args.Reset();
   args.ImportTargets(dropAtoms);
   args.NumImportTargets(dropAtomCount);
   args.DropSiteActivity(dropEnabled ? XmDROP_SITE_ACTIVE
				     : XmDROP_SITE_INACTIVE);
   XmDropSiteUpdate(viewDA, ARGS);
}

/*-----------------------------------------------------------------------
 *  Handle drag over event
 */

void
VBoxC::HandleDragOver(Widget w, XtPointer, XmDragProcCallbackStruct *dp)
{
   if ( dp->reason == XmCR_DROP_SITE_MOTION_MESSAGE ) {

      VBoxC	*This;
      XtVaGetValues(w, XmNuserData, &This, NULL);

      if ( This->view && This->dropCalls.size() > 0 )
	 This->view->HandleDragOver(dp);
      else
	 dp->dropSiteStatus = XmINVALID_DROP_SITE;

   } // End if this is a motion message

   dp->animate = False;

} // End HandleDragOver

/*-----------------------------------------------------------------------
 *  Handle drop event
 */

void
VBoxC::HandleDropIn(Widget w, XtPointer, XmDropProcCallbackStruct *dp)
{
   VBoxC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);

   if ( !This->view ) return;

//
// Call drop callbacks
//
   if ( This->dropCalls.size()>0 && This->view->DropItem() ) {

      ViewDropDataT	dropData;
      dropData.item     = This->view->DropItem();
      dropData.viewBox  = This;
      dropData.procData = dp;
      This->CallDropCallbacks(&dropData);
   }

   else {
//
// Drop must be acknowledged
//
      WArgList	  args;
      args.TransferStatus(XmTRANSFER_FAILURE);
      XmDropTransferStart(dp->dragContext, ARGS);
   }

   This->view->DropFinished();

} // End HandleDropIn

/*-----------------------------------------------------------------------
 *  Handle horizontal scroll
 */

void
VBoxC::HandleHScroll(Widget, VBoxC *This, XmScrollBarCallbackStruct *sb)
{
//
// If this is a drag scroll, see if there are any more scroll events coming.
//    If there are, ignore this one.
//
   if ( sb->reason == XmCR_DRAG && sb->event->type == MotionNotify ) {

      XMotionEvent	*ev = (XMotionEvent*)sb->event;
      XEvent	next;
      if ( XCheckTypedEvent(ev->display, ev->type, &next) ) {
	 XPutBackEvent(ev->display, &next);
	 return;
      }
   }

   This->hScrollValue = sb->value;
   if ( This->realized && This->view ) This->view->Redraw();

} // End HandleHScroll

/*-----------------------------------------------------------------------
 *  Handle vertical scroll
 */

void
VBoxC::HandleVScroll(Widget, VBoxC *This, XmScrollBarCallbackStruct *sb)
{
//
// If this is a drag scroll, see if there are any more scroll events coming.
//    If there are, ignore this one.
//
   if ( sb->reason == XmCR_DRAG && sb->event->type == MotionNotify ) {

      XMotionEvent	*ev = (XMotionEvent*)sb->event;
      XEvent	next;
      if ( XCheckTypedEvent(ev->display, ev->type, &next) ) {
	 XPutBackEvent(ev->display, &next);
	 return;
      }
   }

   This->vScrollValue = sb->value;
   if ( This->realized && This->view ) This->view->Redraw();

} // End HandleVScroll

/*-----------------------------------------------------------------------
 *  Methods to turn scrollbars on
 */

void
VBoxC::EnableHScroll()
{
   if ( hScrollOn ) return;

   hScrollValue = 0;
   hScrollOn = True;

   int	val, size, inc, pinc;
   XmScrollBarGetValues(hScrollBar, &val, &size, &inc, &pinc);
   XmScrollBarSetValues(hScrollBar, hScrollValue, size, inc, pinc, False);

   Dimension	ht;
   Dimension	delta = scrollBoff - noScrollBoff;
   XtVaGetValues(viewFrame, XmNheight, &ht, NULL);
   XtVaSetValues(viewFrame, XmNheight, ht-delta,
   			    XmNbottomOffset, scrollBoff, NULL);

   XtMapWidget(hScrollBar);

   if ( vScrollOn && !joyStickOn ) {
      XtMapWidget(joyStickFrame);
      joyStickOn = True;
   }
}

void
VBoxC::EnableVScroll()
{
   if ( vScrollOn ) return;

   vScrollValue = 0;
   vScrollOn = True;

   int	val, size, inc, pinc;
   XmScrollBarGetValues(vScrollBar, &val, &size, &inc, &pinc);
   XmScrollBarSetValues(vScrollBar, vScrollValue, size, inc, pinc, False);
//   XtVaSetValues(vScrollBar, XmNvalue, 0, NULL);

   Dimension	wd;
   Dimension	delta = scrollRoff - noScrollRoff;
   XtVaGetValues(viewFrame, XmNwidth, &wd, NULL);
   XtVaSetValues(viewFrame, XmNwidth, wd-delta,
   			    XmNrightOffset, scrollRoff, NULL);
   XtMapWidget(vScrollBar);

   if ( hScrollOn && !joyStickOn ) {
      XtMapWidget(joyStickFrame);
      joyStickOn = True;
   }
}

/*-----------------------------------------------------------------------
 *  Methods to turn scrollbars off
 */

void
VBoxC::DisableHScroll()
{
   if ( !hScrollOn ) return;

   hScrollValue = 0;
   hScrollOn = False;

   XtUnmapWidget(hScrollBar);

   Dimension	ht;
   Dimension	delta = scrollBoff - noScrollBoff;
   XtVaGetValues(viewFrame, XmNheight, &ht, NULL);
   XtVaSetValues(viewFrame, XmNheight, ht+delta,
			    XmNbottomOffset, noScrollBoff, NULL);

   if ( joyStickOn ) {
      XtUnmapWidget(joyStickFrame);
      joyStickOn = False;
   }
}

void
VBoxC::DisableVScroll()
{
   if ( !vScrollOn ) return;

   vScrollValue = 0;
   vScrollOn = False;

   XtUnmapWidget(vScrollBar);

   Dimension	wd;
   Dimension	delta = scrollRoff - noScrollRoff;
   XtVaGetValues(viewFrame, XmNwidth, &wd, NULL);
   XtVaSetValues(viewFrame, XmNwidth, wd+delta,
			    XmNrightOffset, noScrollRoff, NULL);

   if ( joyStickOn ) {
      XtUnmapWidget(joyStickFrame);
      joyStickOn = False;
   }
}

/*-----------------------------------------------------------------------
 *  Method to set size of scrollbars
 */

void
VBoxC::UpdateScrollBars()
{
   int	wd, ht;
   view->GetSize(&wd, &ht);
   if ( wd < 1 ) wd = 1;
   if ( ht < 1 ) ht = 1;

//
// Set the scrollbars
//
   if ( wd <= visRect.wd ) {
      DisableHScroll();
      hScrollMaxValue = 0;
   }
   else {

      EnableHScroll();	// This can change visRect.wd
      if ( visRect.wd > wd ) wd = visRect.wd;
      int	incr = wd/(int)10;
      hScrollMaxValue = wd - visRect.wd;
      if ( hScrollValue > hScrollMaxValue ) hScrollValue = hScrollMaxValue;

      XtVaSetValues(hScrollBar, XmNsliderSize, visRect.wd, XmNmaximum, wd,
      		    XmNincrement, incr, XmNvalue, hScrollValue, NULL);
   }

   if ( ht <= visRect.ht ) {
      DisableVScroll();
      vScrollMaxValue = 0;
   }
   else {

      EnableVScroll();	// This can change visRect.ht
      if ( visRect.ht > ht ) ht = visRect.ht;
      int	count = visItems.size();
      int	incr = (count > 0) ? ht/count : ht;
      vScrollMaxValue = ht - visRect.ht;
      if ( vScrollValue > vScrollMaxValue ) vScrollValue = vScrollMaxValue;

      XtVaSetValues(vScrollBar, XmNsliderSize, visRect.ht, XmNmaximum, ht,
      		    XmNincrement, incr, XmNvalue, vScrollValue, NULL);
   }

} // End UpdateScrollBars

/*-----------------------------------------------------------------------
 *  Public method to set the font
 */

void
VBoxC::SetFont(XFontStruct *newFont)
{
   if ( freeFontList ) XmFontListFree(fontList);
   if ( freeFont ) XFreeFont(halApp->display, font);
   fontList = NULL;
   freeFontList = False;
   font = newFont;
   freeFont = False;

//
// Redraw the current view
//
   if ( view ) {
      view->PlaceItems();
      UpdateScrollBars();
      view->Redraw();
   }

} // End SetFont

/*-----------------------------------------------------------------------
 *  Public method to set the fontList
 */

void
VBoxC::SetFontList(XmFontList newFontList)
{
   if ( freeFontList ) XmFontListFree(fontList);
   fontList = XmFontListCopy(newFontList);

//
// Redraw the current view
//
   if ( view ) {
      view->PlaceItems();
      UpdateScrollBars();
      view->Redraw();
   }

} // End SetFont


