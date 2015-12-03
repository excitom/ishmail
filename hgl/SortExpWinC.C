/*
 * $Id: SortExpWinC.C,v 1.3 2000/05/07 12:26:11 fnevgeny Exp $
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
#include "SortExpWinC.h"
#include "WArgList.h"
#include "WXmString.h"
#include "StringC.h"
#include "rsrc.h"

#include <Xm/RowColumn.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/PushB.h>
#include <Xm/DrawingA.h>
#include <Xm/Label.h>
#include <Xm/ScrolledW.h>
#include <Xm/MessageB.h>
#include <Xm/ToggleB.h>
#include <X11/keysym.h>

/*-------------------------------------------------------------------------
 * SortElemC constructors
 */

SortElemC::SortElemC(SortKeyC *k, Widget w)
{
   widget = w;
   key    = k;
   ht     = 0;
   XtVaGetValues(w, XmNheight, &ht, NULL);
}

/*-------------------------------------------------------------------------
 * SortElemC destructor
 */

SortElemC::~SortElemC()
{
   if ( halApp->xRunning ) {
      XtUnmanageChild(widget);
      XtDestroyWidget(widget);
   }
}

/*-------------------------------------------------------------------------
 * SortElemC assignment
 */

SortElemC&
SortElemC::operator=(const SortElemC& e)
{
   widget = e.widget;
   key    = e.key;

   return *this;
}

/*-------------------------------------------------------------------------
 * SortElemC equivalence
 */

int
SortElemC::operator==(const SortElemC& e) const
{
   return ( widget == e.widget && key == e.key );
}

XtTranslations	SortExpWinC::translations;

XtActionsRec	SortExpWinC::actions[5] = {
   "SortExpWinC-delete-left",	(XtActionProc)HandleDeleteLeft,
   "SortExpWinC-delete-right",	(XtActionProc)HandleDeleteRight,
   "SortExpWinC-move-left",	(XtActionProc)HandleMoveLeft,
   "SortExpWinC-move-right",	(XtActionProc)HandleMoveRight,
   "SortExpWinC-move-pointer",	(XtActionProc)HandleMovePointer,
};

/*-------------------------------------------------------------------------
 * SortExpWinC constructor
 */

SortExpWinC::SortExpWinC(Widget parent, const char *name)
: HalDialogC(name, parent)
{
   insertPos  = 0;
   resizeOk   = True;
   blinkTimer = (XtIntervalId)NULL;
   cursorOn   = False;
   applyAll   = True;
   writeFunc  = NULL;
   applyQueryWin = NULL;
   applyCalls.AllowDuplicates(TRUE);

//
// Add actions and translations if necessary
//
   static Boolean	actionsAdded = False;
   if ( !actionsAdded ) {
      XtAppAddActions(halApp->context, actions, XtNumber(actions));
      actionsAdded = True;

//
// These translations cannot be specified in a resource file due to the fact
//    that Motif overrides them after the resource file is processed.
//
      translations = XtParseTranslationTable(
	 "<Key>osfLeft:		SortExpWinC-move-left()	\n\
	  <Key>osfRight:	SortExpWinC-move-right()");

   } // End if actions not added

//
// Get keyboard focus policy
//
   unsigned char	focusPolicy;
   XtVaGetValues(*this, XmNkeyboardFocusPolicy, &focusPolicy, NULL);

   WArgList	args;

//
// appForm
//    Frame		expFrame
//    Label		keyOrderLabel
//    ScrolledWindow	expWin
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   Widget	expFrame = XmCreateFrame(appForm, "expFrame", ARGS);

   args.Reset();
   args.TopAttachment(XmATTACH_WIDGET, expFrame);
   args.LeftAttachment(XmATTACH_FORM);
   args.Alignment(XmALIGNMENT_BEGINNING);
   Widget	keyOrderLabel = XmCreateLabel(appForm, "keyOrderLabel", ARGS);

   args.Reset();
   args.TopAttachment(XmATTACH_WIDGET, keyOrderLabel);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_FORM);
   args.ScrollingPolicy(XmAUTOMATIC);
   args.ScrollBarDisplayPolicy(XmSTATIC);
   expWin = XmCreateScrolledWindow(appForm, "expWin", ARGS);

   XtVaGetValues(expWin, XmNclipWindow, &clipWin, NULL);
   XtVaGetValues(clipWin, XmNwidth, &clipWd, XmNheight, &clipHt, NULL);

   XtAddEventHandler(clipWin, StructureNotifyMask, False,
		     (XtEventHandler)HandleResize, (XtPointer)this);

//
// expFrame
//    Label		availKeyLabel
//    Form		expForm
//
   args.Reset();
   args.ChildType(XmFRAME_TITLE_CHILD);
   Widget	availKeyLabel = XmCreateLabel(expFrame, "availKeyLabel", ARGS);

   expForm = XmCreateForm(expFrame, "expForm", 0,0);

//
// buttonRC
//    PushButton	okPB
//    PushButton	applyPB
//    PushButton	clearPB
//    PushButton	cancelPB
//    PushButton	helpPB
//
   AddButtonBox();
   Widget	okPB	 = XmCreatePushButton(buttonRC, "okPB",     0,0);
   Widget	applyPB	 = XmCreatePushButton(buttonRC, "applyPB",  0,0);
   Widget	clearPB	 = XmCreatePushButton(buttonRC, "clearPB",  0,0);
   Widget	cancelPB = XmCreatePushButton(buttonRC, "cancelPB", 0,0);
   Widget	helpPB	 = XmCreatePushButton(buttonRC, "helpPB",   0,0);

   XtAddCallback(okPB, XmNactivateCallback, (XtCallbackProc)DoOk,
          	 (XtPointer)this);
   XtAddCallback(applyPB, XmNactivateCallback, (XtCallbackProc)DoApply,
          	 (XtPointer)this);
   XtAddCallback(clearPB, XmNactivateCallback, (XtCallbackProc)DoClear,
          	 (XtPointer)this);
   XtAddCallback(cancelPB, XmNactivateCallback, (XtCallbackProc)DoCancel,
 		 (XtPointer)this);
   XtAddCallback(helpPB, XmNactivateCallback,
		 (XtCallbackProc)HalAppC::DoHelp, (char *) "helpcard");

   XtVaSetValues(appForm, XmNdefaultButton, okPB, NULL);

//
// expWin
//    DrawingArea	expDA
//
   args.Reset();
   args.UserData(this);
   expDA = XmCreateDrawingArea(expWin, "expDA", ARGS);

   XtVaSetValues(expWin, XmNworkWindow, expDA, NULL);
   XtOverrideTranslations(expDA, translations);

//
// Add event handler for focus change
//
   if ( focusPolicy == XmPOINTER ) {
      XtAddEventHandler(clipWin, EnterWindowMask|LeaveWindowMask, False,
			(XtEventHandler)HandleFocusChange, (XtPointer)this);
   } else {
      XtAddEventHandler(expDA, FocusChangeMask, False,
			(XtEventHandler)HandleFocusChange, (XtPointer)this);
   }

   Pixel	expFG;
   XtVaGetValues(expDA, XmNmarginWidth, &marWd, XmNmarginHeight, &marHt,
			XmNborderWidth, &borWd, XmNforeground, &expFG, NULL);
   marWd += borWd;
   marHt += borWd;

//
// expDA
//    Label		cursorLabel
//    DrawingArea	cornerDA
//
   args.Reset();
   args.UserData(this);
   cursorLabel = XmCreateLabel(expDA, "cursor", ARGS);
   XtVaGetValues(cursorLabel, XmNwidth, &cursorWd, XmNheight, &cursorHt, NULL);

//
// Create a small child in the lower right corner to make sure the DA is always
//    the right size
//
   args.Reset();
   args.Width(1);
   args.Height(1);
   args.X((Position)clipWd);
   args.Y((Position)clipHt);
   args.NavigationType(XmNONE);
   args.TraversalOn(False);
   args.UserData(this);
   cornerDA = XmCreateDrawingArea(expDA, "cornerDA", ARGS);

//
// Create a frame between the button box and the button separator to show
//    the current/future choices
//
// topForm
//    Separator		buttonSep	(already created)
//    Frame		centerFrame
//       Frame		applyFrame
//          RadioBox	   applyRadio
//             ToggleButton      applyAllTB
//             ToggleButton      applyCurrentTB
//    Frame		buttonFrame	(already created)
//
   args.Reset();
   args.ShadowThickness(0);
   args.MarginWidth(0);
   args.MarginHeight(0);
   args.TopAttachment(XmATTACH_NONE);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_WIDGET, buttonRC);
   Widget centerFrame = XmCreateFrame(topForm, "centerFrame", ARGS);

   args.Reset();
   args.ChildType(XmFRAME_TITLE_CHILD);
   args.ChildHorizontalAlignment(XmALIGNMENT_CENTER);
   args.ChildHorizontalSpacing(0);
   args.ChildVerticalAlignment(XmALIGNMENT_WIDGET_TOP);
   Widget applyFrame = XmCreateFrame(centerFrame, "applyFrame", ARGS);

   args.Reset();
   args.Orientation(XmHORIZONTAL);
   args.Packing(XmPACK_TIGHT);
   Widget applyRadio = XmCreateRadioBox(applyFrame, "applyRadio", ARGS);

   Widget applyAllTB     = XmCreateToggleButton(applyRadio, "applyAllTB", 0,0);
   Widget applyCurrentTB = XmCreateToggleButton(applyRadio, "applyCurrentTB",
   						0,0);
   XmToggleButtonSetState(applyAllTB, True, False);

   XtAddCallback(applyAllTB, XmNvalueChangedCallback,
		 (XtCallbackProc)SetApplyAll, (XtPointer)this);
   XtAddCallback(applyCurrentTB, XmNvalueChangedCallback,
		 (XtCallbackProc)SetApplyCurrent, (XtPointer)this);

   XtManageChild(applyAllTB);
   XtManageChild(applyCurrentTB);
   XtManageChild(applyRadio);
   XtManageChild(applyFrame);
   XtManageChild(centerFrame);

   args.Reset();
   args.BottomAttachment(XmATTACH_WIDGET, centerFrame);
   XtSetValues(buttonSep, ARGS);

//
// Manage widgets
//
   Widget	list[5];

   list[0] = cursorLabel;
   list[1] = cornerDA;
   XtManageChildren(list, 2);	// expDA children

   XtManageChild(expDA);	// expWin children

   list[0] = okPB;
   list[1] = applyPB;
   list[2] = clearPB;
   list[3] = cancelPB;
   list[4] = helpPB;
   XtManageChildren(list, 5);	// buttonRC children

   list[0] = availKeyLabel;
   list[1] = expForm;
   XtManageChildren(list, 2);	// expFrame children

   list[0] = expFrame;
   list[1] = keyOrderLabel;
   list[2] = expWin;
   XtManageChildren(list, 3);	// appForm children

   HandleHelp();

//
// Read window specific resources
//
   float	rate = get_float("SortExpWinC", *this, "cursorBlinkRate", 4.0);
   blinkInterval = (rate > 0.0) ? (int)((float)1000/rate) : 0;

} // End constructor

/*-------------------------------------------------------------------------
 * Destructor
 */

SortExpWinC::~SortExpWinC()
{
   if ( blinkTimer && halApp->xRunning ) XtRemoveTimeOut(blinkTimer);

   unsigned	count = expList.size();
   for (int i=0; i<count; i++) delete expList[i];

   DeleteCallbacks(applyCalls);
}

/*-------------------------------------------------------------------------
 * Callback to handle press of Ok button
 */

void
SortExpWinC::DoOk(Widget, SortExpWinC *This, XtPointer)
{
   if ( This->Apply() ) DoCancel(NULL, This, NULL);
}

/*-------------------------------------------------------------------------
 * Callback to handle press of Apply button
 */

void
SortExpWinC::DoApply(Widget, SortExpWinC *This, XtPointer)
{
   This->Apply();
}

Boolean
SortExpWinC::Apply()
{
   if ( writeFunc && !ApplyQuery() ) return False;

   BusyCursor(True);
//
// Build fresh list of sort keys
//
   keyList.removeAll();
   unsigned	count = expList.size();
   for (int i=0; i<count; i++) keyList.append((SortKeyC *)*expList[i]);

//
// Let user apply sort
//
   CallApplyCallbacks();

//
// Write spec if necessary
//
   if ( writeFunc ) (*writeFunc)(this);

   BusyCursor(False);
   return True;

} // End Apply

/*-------------------------------------------------------------------------
 * Callback to handle press of Clear button
 */

void
SortExpWinC::DoClear(Widget, SortExpWinC *This, XtPointer)
{
   This->Clear();
}

void
SortExpWinC::Clear()
{
   unsigned	count = expList.size();
   for (int i=0; i<count; i++) delete expList[i];

   expList.removeAll();
   insertPos = 0;
   DrawExp();
}

/*-------------------------------------------------------------------------
 * Callback to handle press of Cancel button
 */

void
SortExpWinC::DoCancel(Widget, SortExpWinC *This, XtPointer)
{
   This->Hide();
}

/*-------------------------------------------------------------------------
 * Method to handle insertion of a terminal expression
 */

Widget
SortExpWinC::AddSortKey(SortKeyC *key, const char *cs)
{
   WArgList	args;
   WXmString	str(cs);

   args.Reset();
   args.UserData(this);
   args.LabelString((XmString)str);
   Widget	w = XmCreateLabel(expDA, "keyLabel", ARGS);
   XtManageChild(w);
   XtOverrideTranslations(w, translations);

   SortElemC	*elem = new SortElemC(key, w);
   expList.insert(elem, insertPos);
   insertPos++;

   DrawExp();

   return w;
}

/*-------------------------------------------------------------------------
 * Method to handle removal of a terminal expression
 */

void
SortExpWinC::RemoveSortKey(SortKeyC *key)
{
//
// Loop through expList looking for key
//
   Boolean	found = False;
   unsigned	count = expList.size();
   for (int i=0; !found && i<count; i++) {
      SortElemC	*elem = expList[i];

//
// If key matches, remove it and redraw
//
      if ( &elem->SortKey() == key ) {
	 if ( insertPos > i ) insertPos--;
	 delete elem;
	 expList.remove(elem);
	 DrawExp();
	 found = True;
      }

   } // End for each element

} // End RemoveSortKey

/*-------------------------------------------------------------------------
 * Method to calculate positions for all expression elements
 */

void
SortExpWinC::DrawExp()
{
   resizeOk = False;		// Don't respond to self-induced resizes

//
// Position elements
//
   Position	y = marHt;
   WArgList	args;

   unsigned	count = expList.size();
   for (int i=0; i<count; i++) {

      Position	x = marWd;
      SortElemC	*elem = expList[i];

//
// Draw insertion cursor if necessary
//
      if ( insertPos == i ) {

	 args.X(x);
	 args.Y(y + ((int)(elem->Height() - cursorHt)/(int)2));
							  // Center vertically
	 XtSetValues(cursorLabel, ARGS);

	 x += cursorWd;

      } // End if placing cursor here

//
// Set and update position
//
      args.X(x);
      args.Y(y);
      XtSetValues(*elem, ARGS);

      y += elem->Height();

   } // End for each expression

   if ( insertPos == count ) {
      args.X(marWd);
      args.Y(y);
      XtSetValues(cursorLabel, ARGS);
   }

   resizeOk = True;

} // End DrawExp

/*-----------------------------------------------------------------------
 *  Event handler for scrolled window resize
 */

void
SortExpWinC::HandleResize(Widget, SortExpWinC *This, XEvent*, Boolean*)
{
   XtVaGetValues(This->clipWin, XmNwidth, &This->clipWd,
				XmNheight, &This->clipHt, 0);
   XtVaSetValues(This->cornerDA,
		 XmNx, (Position)(This->clipWd - This->borWd*2 - 1),
		 XmNy, (Position)(This->clipHt - This->borWd*2 - 1),
		 NULL);

   if ( This->resizeOk ) This->DrawExp();
}

/*---------------------------------------------------------------
 *  Timer proc to blink insertion cursor
 */

void
SortExpWinC::BlinkProc(SortExpWinC *This, XtIntervalId*)
{
   if ( This->cursorOn ) {
      XtSetMappedWhenManaged(This->cursorLabel, False);
      This->cursorOn = False;
   } else {
      XtSetMappedWhenManaged(This->cursorLabel, True);
      This->cursorOn = True;
   }

   This->blinkTimer = XtAppAddTimeOut(halApp->context, This->blinkInterval,
				    (XtTimerCallbackProc)BlinkProc,
				    (XtPointer)This);

} // End BlinkProc

/*---------------------------------------------------------------
 *  Handle a keyboard focus change event in the area
 */

void
SortExpWinC::HandleFocusChange(Widget, SortExpWinC *This, XEvent *ev, Boolean*)
{
   if ( This->blinkTimer ) XtRemoveTimeOut(This->blinkTimer);

   switch (ev->type) {

      case (FocusIn):
      case (EnterNotify):
	 if ( This->blinkInterval > 0 ) BlinkProc(This, NULL);
	 break;

      case (FocusOut):
      case (LeaveNotify):
	 This->cursorOn = True;
	 XtSetMappedWhenManaged(This->cursorLabel, True);
	 break;
   }

} // End HandleFocusChange

void
SortExpWinC::HandleDeleteLeft(Widget w, XKeyEvent*, String*, Cardinal*)
{
   SortExpWinC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);

   if ( This->insertPos > 0 ) {
      This->insertPos--;
      delete This->expList[This->insertPos];
      This->expList.remove(This->insertPos);
      This->DrawExp();
   }
}

void
SortExpWinC::HandleDeleteRight(Widget w, XKeyEvent*, String*, Cardinal*)
{
   SortExpWinC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);

   if ( This->insertPos < This->expList.size() ) {
      delete This->expList[This->insertPos];
      This->expList.remove(This->insertPos);
      This->DrawExp();
   }
}

void
SortExpWinC::HandleMoveLeft(Widget w, XKeyEvent*, String*, Cardinal*)
{
   SortExpWinC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);

   if ( This->insertPos > 0 ) {
      This->insertPos--;
      This->DrawExp();
   }
}

void
SortExpWinC::HandleMoveRight(Widget w, XKeyEvent*, String*, Cardinal*)
{
   SortExpWinC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);

   if ( This->insertPos < This->expList.size() ) {
      This->insertPos++;
      This->DrawExp();
   }
}

void
SortExpWinC::HandleMovePointer(Widget w, XButtonEvent *ev, String*, Cardinal*)
{
   SortExpWinC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);

   XmProcessTraversal(This->expDA, XmTRAVERSE_CURRENT);

//
// Translate event coordinates to expDA if this is a child
//
   if ( XtParent(w) == This->expDA ) {
      Position	x, y;
      XtVaGetValues(w, XmNx, &x, XmNy, &y, NULL);
      ev->x += x;
      ev->y += y;
   }

//
// Find element which corresponds to this y value
//
   int		y = This->marHt;
   unsigned	count = This->expList.size();
   int	i;
   for (i=0; i<count && ev->y > y; i++) {

      SortElemC	*elem = This->expList[i];
      y += elem->Height();
   }

   if ( i>0 && ev->y <= y ) This->insertPos = i-1;
   else                     This->insertPos = i;

   This->DrawExp();

} // End HandleMovePointer

/*---------------------------------------------------------------
 *  Method to query for type of apply
 */

Boolean
SortExpWinC::ApplyQuery()
{
   return True;

#if 0
   static int	reason;

//
// Create the dialog if necessary
//
   if ( !applyQueryWin ) {

      WArgList	args;

      BusyCursor(True);
      args.Reset();
      args.DialogStyle(XmDIALOG_FULL_APPLICATION_MODAL);
      Widget w = XmCreateQuestionDialog(*this, "applyQueryWin", ARGS);

      XtAddCallback(w, XmNokCallback, (XtCallbackProc)WaitForAnswer,
		    (XtPointer)&reason);
      XtAddCallback(w, XmNcancelCallback, (XtCallbackProc)WaitForAnswer,
		    (XtPointer)&reason);

      Widget applyFrame = XmCreateFrame(w, "applyFrame", 0,0);

      args.Reset();
      args.Orientation(XmVERTICAL);
      args.Packing(XmPACK_COLUMN);
      Widget applyRadio = XmCreateRadioBox(applyFrame, "applyRadio", ARGS);

      Widget applyAllTB =
	 XmCreateToggleButton(applyRadio, "applyAllTB", 0,0);
      Widget applyCurrentTB =
	 XmCreateToggleButton(applyRadio, "applyCurrentTB", 0,0);
      XmToggleButtonSetState(applyAllTB, True, False);

      XtAddCallback(applyAllTB, XmNvalueChangedCallback,
		    (XtCallbackProc)SetApplyAll, (XtPointer)this);
      XtAddCallback(applyCurrentTB, XmNvalueChangedCallback,
		    (XtCallbackProc)SetApplyCurrent, (XtPointer)this);

      Widget	list[3];
      list[0] = applyAllTB;
      list[1] = applyCurrentTB;
      XtManageChildren(list, 2);
      XtManageChild(applyRadio);
      XtManageChild(applyFrame);

      applyQueryWin = w;
      BusyCursor(False);

   } // End if query dialog not created

   XtManageChild(applyQueryWin);
   XMapRaised(halApp->display, XtWindow(XtParent(applyQueryWin)));

//
// Simulate the main event loop and wait for the answer
//

   reason = XmCR_NONE;
   while ( reason == XmCR_NONE ) {
      XtAppProcessEvent(halApp->context, XtIMXEvent);
      XSync(halApp->display, False);
   }

   XtUnmanageChild(applyQueryWin);
   XSync(halApp->display, False);
   XmUpdateDisplay(applyQueryWin);

   return (reason == XmCR_OK);
#endif

} // End ApplyQuery

#if 0
/*---------------------------------------------------------------
 *  Callback routine to handle press of a APPLY QUERY answer
 */

void
SortExpWinC::WaitForAnswer(Widget, int *reason, XmAnyCallbackStruct *cbs)
{
   *reason = cbs->reason;
}
#endif

/*---------------------------------------------------------------
 *  Callbacks to handle setting of apply type
 */

void
SortExpWinC::SetApplyCurrent(Widget, SortExpWinC *sw,
			     XmToggleButtonCallbackStruct *tb)
{
   if ( !tb->set ) return;

   sw->applyAll = False;
}

void
SortExpWinC::SetApplyAll(Widget, SortExpWinC *sw,
			 XmToggleButtonCallbackStruct *tb)
{
   if ( !tb->set ) return;

   sw->applyAll = True;
}
