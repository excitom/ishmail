/*
 * $Id: HalShellC.C,v 1.2 2000/05/07 12:26:10 fnevgeny Exp $
 *
 * Copyright (c) 1993 HAL Computer Systems International, Ltd.
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
#include "HalShellC.h"
#include "ForceDialog.h"
#include "WArgList.h"
#include "WXmString.h"
#include "rsrc.h"
#include "TextMisc.h"
#include "ButtonBox.h"

#include <Xm/MessageB.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/ToggleB.h>
#include <Xm/CascadeB.h>
#include <Xm/DrawnB.h>
#include <Xm/Frame.h>
#include <Xm/RowColumn.h>
#include <Xm/Separator.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/DragDrop.h>
#include <Xm/DrawingA.h>

#include <X11/IntrinsicP.h>
#include <X11/CoreP.h>

extern int	debuglev;

/*----------------------------------------------------------------------
 * Constructor
 */

HalShellC::HalShellC()
{
   shown  = False;
   mapped = False;

   shell      = NULL;
   mainWindow = NULL;
   topForm    = NULL;
   appFrame   = NULL;
   appForm    = NULL;
   msgDialog  = NULL;
   msgWin     = NULL;
   quickHelp  = NULL;
   infoMsg    = NULL;

   hideCalls.AllowDuplicates(TRUE);
   mapCalls.AllowDuplicates(TRUE);
   unmapCalls.AllowDuplicates(TRUE);

//
// The topForm is created by the derived class.  The button box children are
//    created on demand.
//

//
// Traditional setup
//
// topForm
//   Separator          buttonSep       (on demand)
//   Frame              buttonFrame     (on demand)
//      RowColumn       buttonRC        (on demand)
//

//
// Alternate setup
//
// topForm
//   Separator		buttonSep       (on demand)
//   ButtonBox		buttonRC        (on demand)
//
   buttonSep = buttonFrame = buttonRC = NULL;
   buttonGravity = SouthGravity;

} // End HalShellC constructor

HalShellC::~HalShellC()
{
   if ( shown && halApp->xRunning ) Hide();

   shown  = False;
   DeleteCallbacks(hideCalls);
   DeleteCallbacks(mapCalls);
   DeleteCallbacks(unmapCalls);
}

/*----------------------------------------------------------------------
 * Method to add button box
 */

void
HalShellC::AddButtonBox()
{
   if ( buttonRC ) return;

   char	*cl = "HalShellC";
   int	   gravity   = get_gravity(cl, *this, "buttonGravity", "South");
   Boolean useRowCol = get_boolean(cl, *this, "useRowCol",     True);

   WArgList	args;
   if ( useRowCol ) { // Traditional layout

//
// Create the frame with south gravity
//
      args.ShadowThickness(0);
      args.MarginWidth(0);
      args.MarginHeight(0);
      args.TopAttachment(XmATTACH_NONE);
      args.LeftAttachment(XmATTACH_FORM);
      args.RightAttachment(XmATTACH_FORM);
      args.BottomAttachment(XmATTACH_FORM);
      buttonFrame = XmCreateFrame(topForm, "buttonFrame", ARGS);

//
// Create and position the row/column for south gravity
//
      args.Reset();
      args.Orientation(XmHORIZONTAL);
      args.ChildType(XmFRAME_TITLE_CHILD);
      args.ChildHorizontalAlignment(XmALIGNMENT_CENTER);
      args.ChildHorizontalSpacing(0);
      args.ChildVerticalAlignment(XmALIGNMENT_WIDGET_TOP);
      args.EntryAlignment(XmALIGNMENT_CENTER);
      buttonRC = XmCreateRowColumn(buttonFrame, "buttonRC", ARGS);
      XtManageChild(buttonRC);

   } // End if traditional layout

   else { // Alternate layout
      args.TopAttachment(XmATTACH_NONE);
      args.LeftAttachment(XmATTACH_FORM);
      args.RightAttachment(XmATTACH_FORM);
      args.BottomAttachment(XmATTACH_FORM);
      args.Orientation(XmHORIZONTAL);
      args.ResizeWidth(False);
      args.ResizeHeight(True);
      buttonRC = CreateButtonBox(topForm, "buttonRC", ARGS);
      XtManageChild(buttonRC);
   }

//
// Create and position the separator for south gravity
//
   args.Reset();
   args.Orientation(XmHORIZONTAL);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   if ( buttonFrame ) args.BottomAttachment(XmATTACH_WIDGET, buttonFrame);
   else		      args.BottomAttachment(XmATTACH_WIDGET, buttonRC);
   args.TopAttachment(XmATTACH_NONE);
   buttonSep = XmCreateSeparator(topForm, "buttonSep", ARGS);

   Widget	wlist[2];
   wlist[0] = buttonSep;
   wlist[1] = buttonFrame ? buttonFrame : buttonRC;
   XtManageChildren(wlist, 2);

//
// Attach the app form to the separator
//
   args.Reset();
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   args.TopAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_WIDGET, buttonSep);
   XtSetValues(appForm, ARGS);

//
// Now set the real gravity
//
   SetButtonGravity(gravity);

} // End AddButtonBox

/*----------------------------------------------------------------------
 * Method to set the button gravity
 */

void
HalShellC::SetButtonGravity(int gravity)
{
   if ( buttonGravity == gravity ) return;

   WArgList	args;
   Widget	buttonW = buttonFrame ? buttonFrame : buttonRC;

//
// Get size of separator
//
   Dimension	sepWd, sepHt;
   XtVaGetValues(buttonSep, XmNwidth, &sepWd, XmNheight, &sepHt, NULL);

#if 0
//
// Keep form from resizing
//
   unsigned char	policy;
   XtVaGetValues(topForm, XmNresizePolicy, &policy, NULL);
   XtVaSetValues(topForm, XmNresizePolicy, XmRESIZE_NONE, NULL);
#endif

#if 0
//
// Unmanage widgets
//
   XtUnmanageChild(appForm);
   XtUnmanageChild(buttonSep);
   XtUnmanageChild(buttonW);
   XtUnmanageChild(buttonRC);
   WidgetList   wlist;
   Cardinal     wcount;
   XtVaGetValues(buttonRC, XmNnumChildren, &wcount, XmNchildren, &wlist, 0);
   XtUnmanageChildren(wlist, wcount);
#endif

//
// Remove current attachments
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_NONE);
   XtSetValues(appForm,   ARGS);
   XtSetValues(buttonSep, ARGS);
   XtSetValues(buttonW,   ARGS);

//
// Set the new attachments based on the new gravity
//
   switch (gravity) {

      case (NorthWestGravity):
      case (SouthWestGravity):
	 gravity = WestGravity;
      case (WestGravity):

	 args.Reset();
	 args.Orientation(XmVERTICAL);
	 if ( buttonFrame )
	    args.ChildHorizontalSpacing(0);
	 else {
	    args.ResizeWidth(True);
	    args.ResizeHeight(False);
	 }
	 XtSetValues(buttonRC, ARGS);

	 args.Reset();
	 args.TopAttachment(XmATTACH_FORM);
	 args.LeftAttachment(XmATTACH_FORM);
	 args.RightAttachment(XmATTACH_NONE);
	 args.BottomAttachment(XmATTACH_FORM);
	 XtSetValues(buttonW, ARGS);

	 args.Reset();
	 args.TopAttachment(XmATTACH_FORM);
	 args.LeftAttachment(XmATTACH_WIDGET, buttonW);
	 args.RightAttachment(XmATTACH_NONE);
	 args.BottomAttachment(XmATTACH_FORM);
	 args.Orientation(XmVERTICAL);
	 if ( buttonGravity == NorthGravity ||
	      buttonGravity == SouthGravity ) {
	    args.Width(sepHt);
	    args.Height(sepWd);
	 }
	 XtSetValues(buttonSep, ARGS);

	 args.Reset();
	 args.TopAttachment(XmATTACH_FORM);
	 args.LeftAttachment(XmATTACH_WIDGET, buttonSep);
	 args.RightAttachment(XmATTACH_FORM);
	 args.BottomAttachment(XmATTACH_FORM);
	 XtSetValues(appForm, ARGS);
	 break;

      case (NorthEastGravity):
      case (SouthEastGravity):
	 gravity = EastGravity;
      case (EastGravity):

	 args.Reset();
	 args.Orientation(XmVERTICAL);
	 if ( buttonFrame )
	    args.ChildHorizontalSpacing(0);
	 else {
	    args.ResizeWidth(True);
	    args.ResizeHeight(False);
	 }
	 XtSetValues(buttonRC, ARGS);

	 args.Reset();
	 args.TopAttachment(XmATTACH_FORM);
	 args.LeftAttachment(XmATTACH_NONE);
	 args.RightAttachment(XmATTACH_FORM);
	 args.BottomAttachment(XmATTACH_FORM);
	 XtSetValues(buttonW, ARGS);

	 args.Reset();
	 args.TopAttachment(XmATTACH_FORM);
	 args.LeftAttachment(XmATTACH_NONE);
	 args.RightAttachment(XmATTACH_WIDGET, buttonW);
	 args.BottomAttachment(XmATTACH_FORM);
	 args.Orientation(XmVERTICAL);
	 if ( buttonGravity == NorthGravity ||
	      buttonGravity == SouthGravity ) {
	    args.Width(sepHt);
	    args.Height(sepWd);
	 }
	 XtSetValues(buttonSep, ARGS);

	 args.Reset();
	 args.TopAttachment(XmATTACH_FORM);
	 args.LeftAttachment(XmATTACH_FORM);
	 args.RightAttachment(XmATTACH_WIDGET, buttonSep);
	 args.BottomAttachment(XmATTACH_FORM);
	 XtSetValues(appForm, ARGS);
	 break;

      case (NorthGravity):
	 gravity = NorthGravity;

	 args.Reset();
	 args.Orientation(XmHORIZONTAL);
	 if ( buttonFrame )
	    args.ChildHorizontalSpacing(0);
	 else {
	    args.ResizeWidth(False);
	    args.ResizeHeight(True);
	 }
	 XtSetValues(buttonRC, ARGS);

	 args.Reset();
	 args.TopAttachment(XmATTACH_FORM);
	 args.LeftAttachment(XmATTACH_FORM);
	 args.RightAttachment(XmATTACH_FORM);
	 args.BottomAttachment(XmATTACH_NONE);
	 XtSetValues(buttonW, ARGS);

	 args.Reset();
	 args.TopAttachment(XmATTACH_WIDGET, buttonW);
	 args.LeftAttachment(XmATTACH_FORM);
	 args.RightAttachment(XmATTACH_FORM);
	 args.BottomAttachment(XmATTACH_NONE);
	 args.Orientation(XmHORIZONTAL);
	 if ( buttonGravity == EastGravity ||
	      buttonGravity == WestGravity ) {
	    args.Width(sepHt);
	    args.Height(sepWd);
	 }
	 XtSetValues(buttonSep, ARGS);

	 args.Reset();
	 args.TopAttachment(XmATTACH_WIDGET, buttonSep);
	 args.LeftAttachment(XmATTACH_FORM);
	 args.RightAttachment(XmATTACH_FORM);
	 args.BottomAttachment(XmATTACH_FORM);
	 XtSetValues(appForm, ARGS);
	 break;

      case (SouthGravity):
      default:
	 gravity = SouthGravity;

	 args.Reset();
	 args.Orientation(XmHORIZONTAL);
	 if ( buttonFrame )
	    args.ChildHorizontalSpacing(0);
	 else {
	    args.ResizeWidth(False);
	    args.ResizeHeight(True);
	 }
	 XtSetValues(buttonRC, ARGS);
	 
	 args.Reset();
	 args.TopAttachment(XmATTACH_NONE);
	 args.LeftAttachment(XmATTACH_FORM);
	 args.RightAttachment(XmATTACH_FORM);
	 args.BottomAttachment(XmATTACH_FORM);
	 XtSetValues(buttonW, ARGS);

	 args.Reset();
	 args.TopAttachment(XmATTACH_NONE);
	 args.LeftAttachment(XmATTACH_FORM);
	 args.RightAttachment(XmATTACH_FORM);
	 args.BottomAttachment(XmATTACH_WIDGET, buttonW);
	 args.Orientation(XmHORIZONTAL);
	 if ( buttonGravity == EastGravity ||
	      buttonGravity == WestGravity ) {
	    args.Width(sepHt);
	    args.Height(sepWd);
	 }
	 XtSetValues(buttonSep, ARGS);

	 args.Reset();
	 args.TopAttachment(XmATTACH_FORM);
	 args.LeftAttachment(XmATTACH_FORM);
	 args.RightAttachment(XmATTACH_FORM);
	 args.BottomAttachment(XmATTACH_WIDGET, buttonSep);
	 XtSetValues(appForm, ARGS);
	 break;

   } // End switch new gravity

#if 0
//
// Remanage widgets
//
   XtManageChildren(wlist, wcount);
   XtManageChild(buttonRC);
   XtManageChild(buttonFrame);
   XtManageChild(buttonSep);
   XtManageChild(appForm);
#endif

#if 0
   XtVaSetValues(topForm, XmNresizePolicy, policy, NULL);
#endif

   buttonGravity = gravity;

} // End SetButtonGravity

/*----------------------------------------------------------------------
 * Since the stippled cursor looks dumb in every text field, hide it in
 *    fields without the focus.
 */

static void
TextGainFocus(Widget w, XtPointer, XtPointer)
{
   XtVaSetValues(w, XmNcursorPositionVisible, True, NULL);
}

static void
TextLoseFocus(Widget w, XtPointer, XtPointer)
{
   XtVaSetValues(w, XmNcursorPositionVisible, False, NULL);
}

/*----------------------------------------------------------------------
 * Enable quick-help for specified widget
 */

void
HalShellC::HandleFocus(Widget w)
{
//
// Add event handlers to this widget for pointer events
//
   XtAddEventHandler(w, EnterWindowMask, False, (XtEventHandler)EnterFocus,
                     (XtPointer)this);
   XtAddEventHandler(w, LeaveWindowMask, False, (XtEventHandler)LeaveFocus,
                     (XtPointer)this);

//
// Add event handler for keyboard events
//
   XtAddEventHandler(w, FocusChangeMask, False, (XtEventHandler)ChangeFocus,
                     (XtPointer)this);

//
// Add callbacks for text widgets to hide stippled cursor in inactive text
//    fields
//
   if ( (XmIsText(w)      && XmTextGetEditable(w)) ||
	(XmIsTextField(w) && XmTextFieldGetEditable(w)) ) {
      XtAddCallback(w, XmNfocusCallback,       (XtCallbackProc)TextGainFocus,0);
      XtAddCallback(w, XmNlosingFocusCallback, (XtCallbackProc)TextLoseFocus,0);
   }

//
// Allow tabbing to this widget
//
   //XtVaSetValues(w, XmNtraversalOn, True, NULL);

} // End HandleFocus

/*----------------------------------------------------------------------
 * Disable traversal for specified widget
 */

void
HalShellC::UnhandleFocus(Widget w)
{
//
// Remove event handlers from this widget for pointer events
//
   XtRemoveEventHandler(w, EnterWindowMask, False, (XtEventHandler)EnterFocus,
                        (XtPointer)this);
   XtRemoveEventHandler(w, LeaveWindowMask, False, (XtEventHandler)LeaveFocus,
                        (XtPointer)this);

//
// Add event handler for keyboard events
//
   XtRemoveEventHandler(w, FocusChangeMask, False, (XtEventHandler)ChangeFocus,
                        (XtPointer)this);

//
// Disallow tabbing to this widget
//
   //XtVaSetValues(w, XmNtraversalOn, False, NULL);

} // End UnhandleFocus

/*----------------------------------------------------------------------
 * Add and remove on-line and quick help callbacks for specified widget
 */

#ifdef NO_HELP

void
HalShellC::HandleHelp(Widget w, Boolean)
{
   HandleFocus(w);
}

void
HalShellC::UnhandleHelp(Widget w, Boolean)
{
   UnhandleFocus(w);
}

#else

void
HalShellC::HandleHelp(Widget w, Boolean doChildren)
{
//
// If this is a composite and doChildren is true, loop through the children
//   and register them.
//
   if ( XtIsComposite(w) && doChildren ) {

      WidgetList	list;
      Cardinal		count;
      XtVaGetValues(w, XmNnumChildren, &count, XmNchildren, &list, NULL);

//
// Loop through children, but don't cross into other shells
//
      for (int i=0; i<count; i++) {
	 Widget	w = list[i];
	 if ( !XtIsShell(w) ) HandleHelp(w, doChildren);
      }

      if ( XmIsDrawingArea(w) )
	 HandleFocus(w);
   }
   
//
// If this is not a composite or doChildren is false, register this widget.
// Do not register gadgets.
//
   else if ( !XmIsGadget(w) && !XtIsSubclass(w, xmDragIconObjectClass) /*&&
	     (!XmIsLabel(w) || XmIsPushButton(w) || XmIsToggleButton(w) ||
			       XmIsCascadeButton(w) || XmIsDrawnButton(w))*/ ) {
      XtAddCallback(w, XmNhelpCallback, (XtCallbackProc)HalAppC::DoHelp, NULL);
      HandleFocus(w);
   }

} // End HalShellC HandleHelp

void
HalShellC::UnhandleHelp(Widget w, Boolean doChildren)
{
//
// If this is a composite and doChildren is true, loop through the children
//   and unregister them.
//
   if ( XtIsComposite(w) && doChildren ) {

      WidgetList	list;
      Cardinal		count;
      XtVaGetValues(w, XmNnumChildren, &count, XmNchildren, &list, NULL);

//
// Loop through children, but don't cross into other shells
//
      for (int i=0; i<count; i++) {
	 Widget	w = list[i];
	 if ( !XtIsShell(w) ) UnhandleHelp(w, doChildren);
      }

      if ( XmIsDrawingArea(w) )
	 UnhandleFocus(w);
   }
   
//
// If this is not a composite or doChildren is false, unregister this widget.
//
   else if ( !XmIsGadget(w) ) {
      XtRemoveCallback(w, XmNhelpCallback, (XtCallbackProc)HalAppC::DoHelp, 0);
      UnhandleFocus(w);
   }
}
#endif

void
HalShellC::HandleHelp()
{
   HandleHelp(shell, /*doChildren*/True);
}

void
HalShellC::UnhandleHelp()
{
   UnhandleHelp(shell, /*doChildren*/True);
}

/*----------------------------------------------------------------------
 *  Turn the busy cursor on or off
 */

void
HalShellC::BusyCursor(Boolean on)
{
   halApp->BusyCursor(on);
}

/*---------------------------------------------------------------
 *  Event handlers for input focus changes
 */

void
HalShellC::EnterFocus(Widget w, HalShellC *This, XEvent*, Boolean*)
{
   if ( halApp->quickHelpEnabled ) {
      StringC	str = get_string(w, "quickHelp", " ");
      if ( debuglev>2 )
	 cout <<"Quick help for " <<XtName(w) <<" is " <<str <<endl;
      This->QuickHelp(str);
   }
}

void
HalShellC::LeaveFocus(Widget, HalShellC *This, XEvent*, Boolean*)
{
   if ( halApp->quickHelpEnabled )
      This->ClearQuickHelp();
}

void
HalShellC::ChangeFocus(Widget w, HalShellC *This, XEvent *event, Boolean*)
{
   if ( event->type == FocusIn ) EnterFocus(w, This, NULL, NULL);
   else				 LeaveFocus(w, This, NULL, NULL);
}

/*----------------------------------------------------------------------
 *  Display message in popup window
 */

void
HalShellC::PopupMessage(const char *msg, unsigned char type)
{
   if ( shell )
      halApp->PopupMessage(msg, shell, type);
}

/*----------------------------------------------------------------------
 *  Display quick-help and info messages
 */

void
HalShellC::QuickHelp(const char *msg) const
{
   if ( !halApp->quickHelpEnabled || !quickHelp || !halApp->xRunning ) return;

   WXmString str(msg);
   XtVaSetValues(quickHelp, XmNlabelString, (XmString)str, NULL);
}

void
HalShellC::Message(const char *msg) const
{
   if ( !infoMsg || !halApp->xRunning ) return;

   if ( XmIsLabel(infoMsg) ) {

      WXmString str(msg);
      XtVaSetValues(infoMsg, XmNlabelString, (XmString)str, NULL);

//
// Process outstanding expose events in this window
//
      XEvent       event;
      while ( XCheckMaskEvent(halApp->display, ExposureMask, &event) )
	 XtDispatchEvent(&event);
   }

   else {
      XmTextFieldSetString(infoMsg, (char*)msg);
      XmUpdateDisplay(infoMsg);
   }
}

/*----------------------------------------------------------------------
 *  Display and hide window
 */

void
HalShellC::Show()
{
   if ( !halApp->xRunning ) return;

   XtPopup(shell, XtGrabNone);
   XMapRaised(halApp->display, XtWindow(shell));
   ForceDialog(shell);
   shown = True;
   halApp->RegisterShell(this);
   XUndefineCursor(halApp->display, *this);
}

void
HalShellC::Hide()
{
   if ( !shown || !halApp->xRunning ) return;

   CallHideCallbacks();

   halApp->UnregisterShell(this);
   XtPopdown(shell);

   shown  = False;
}

/*----------------------------------------------------------------------
 * Call hide callbacks before hiding
 */

void
HalShellC::DoHide(Widget, HalShellC *This, XtPointer)
{
   This->Hide();
}

/*----------------------------------------------------------------------
 * Find a widget, then display context help for it
 */

void
HalShellC::DoContextHelp(Widget, HalShellC *This, XtPointer)
{
   XEvent ev;
   Widget w = XmTrackingEvent(*This, halApp->helpCursor, False, &ev);
   if ( w != NULL ) HalAppC::DoHelp(w, NULL, NULL);
}

/*----------------------------------------------------------------------
 * Callback to detect map and unmap
 */

void
HalShellC::HandleMapChange(Widget, HalShellC *This, XEvent *ev, Boolean*)
{
//
// We can get this event after the X server has shutdown but before the
//    xRunning flag is updated.  Call XFlush and XSync to force a SIGPIPE
//
   XFlush(halApp->display);
   XSync(halApp->display, False);

   if ( !halApp->xRunning ) return;

   if ( ev->type == MapNotify ) {
      This->mapped = True;
      This->CallMapCallbacks();
   }

   else if ( ev->type == UnmapNotify ) {
      This->CallUnmapCallbacks();
      This->mapped = False;
   }
}

/*----------------------------------------------------------------------
 * Method to turn on quick help widget
 */

void
HalShellC::ShowQuickHelp()
{
   if ( quickHelp && XtIsManaged(quickHelp) ) return;

//
// Manage message area widget if necessary
//
   if ( !XtIsManaged(msgWin) ) {
      XtVaSetValues(mainWindow, XmNmessageWindow, msgWin, NULL);
      XtManageChild(msgWin);
   }

//
// Create quick help widget if necessary
//
   if ( !quickHelp ) {
      WArgList	args;
      args.Alignment(XmALIGNMENT_BEGINNING);
      args.RecomputeSize(False);
      args.PositionIndex(0);
      quickHelp = XmCreateLabel(msgWin, "quickHelp", ARGS);
   }

//
// Turn on quick help widget
//
   XtManageChild(quickHelp);

} // End ShowQuickHelp

/*----------------------------------------------------------------------
 * Method to turn off quick help widget
 */

void
HalShellC::HideQuickHelp()
{
   if ( quickHelp ) {
      if ( !XtIsManaged(quickHelp) ) return;
      XtUnmanageChild(quickHelp);
   }

   if ( !infoMsg || !XtIsManaged(infoMsg) ) {
      XtUnmanageChild(msgWin);
      XtVaSetValues(mainWindow, XmNmessageWindow, NULL, NULL);
      // XtVaSetValues(msgWin, XmNheight, 1, NULL);
   }
}

/*----------------------------------------------------------------------
 * Method to turn on info message widget
 */

void
HalShellC::ShowInfoMsg()
{
   if ( infoMsg && XtIsManaged(infoMsg) ) return;

//
// Manage message area widget if necessary
//
   if ( !XtIsManaged(msgWin) ) {
      XtVaSetValues(mainWindow, XmNmessageWindow, msgWin, NULL);
      XtManageChild(msgWin);
   }

//
// Create info message widget if necessary
//
   if ( !infoMsg ) {
      WArgList	args;
      args.PositionIndex(1);

      Boolean	useLabel = get_boolean(*this, "infoMsgIsLabel", False);
      if ( useLabel ) {
	 args.Alignment(XmALIGNMENT_BEGINNING);
	 args.RecomputeSize(False);
	 infoMsg = XmCreateLabel(msgWin, "infoMsg", ARGS);
      }

      else {
	 args.ShadowThickness(0);
	 args.Editable(False);
	 args.CursorPositionVisible(False);
	 args.TraversalOn(False);
	 args.NavigationType(XmNONE);
	 infoMsg = CreateTextField(msgWin, "infoMsg", ARGS);
      }
   }

//
// Turn on info message widget
//
   XtManageChild(infoMsg);

} // End ShowInfoMsg

/*----------------------------------------------------------------------
 * Method to turn off info message widget
 */

void
HalShellC::HideInfoMsg()
{
   if ( infoMsg ) {
      if ( !XtIsManaged(infoMsg) ) return;
      XtUnmanageChild(infoMsg);
   }

   if ( !quickHelp || !XtIsManaged(quickHelp) ) {
      XtUnmanageChild(msgWin);
      XtVaSetValues(mainWindow, XmNmessageWindow, NULL, NULL);
      // XtVaSetValues(msgWin, XmNheight, 1, NULL);
   }
}
