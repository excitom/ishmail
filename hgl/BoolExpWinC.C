/*
 * $Id: BoolExpWinC.C,v 1.5 2000/08/13 13:25:32 evgeny Exp $
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

#include <config.h>

#include "HalAppC.h"
#include "BoolExpWinC.h"
#include "WArgList.h"
#include "StringC.h"
#include "rsrc.h"

#include <Xm/RowColumn.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/PushB.h>
#include <Xm/DrawingA.h>
#include <Xm/Label.h>
#include <Xm/ScrolledW.h>
#include <X11/keysym.h>

XtTranslations	BoolExpWinC::translations;

XtActionsRec	BoolExpWinC::actions[10] = {
   { "BoolExpWinC-insert-lparen",	(XtActionProc)HandleInsertLParen },
   { "BoolExpWinC-insert-rparen",	(XtActionProc)HandleInsertRParen },
   { "BoolExpWinC-insert-and",		(XtActionProc)HandleInsertAnd },
   { "BoolExpWinC-insert-or",		(XtActionProc)HandleInsertOr },
   { "BoolExpWinC-insert-not",		(XtActionProc)HandleInsertNot },
   { "BoolExpWinC-delete-left",		(XtActionProc)HandleDeleteLeft },
   { "BoolExpWinC-delete-right",	(XtActionProc)HandleDeleteRight },
   { "BoolExpWinC-move-left",		(XtActionProc)HandleMoveLeft },
   { "BoolExpWinC-move-right",		(XtActionProc)HandleMoveRight },
   { "BoolExpWinC-move-pointer",	(XtActionProc)HandleMovePointer },
};

/*-------------------------------------------------------------------------
 * BoolExpWinC constructor
 */

BoolExpWinC::BoolExpWinC(Widget parent)
: HalDialogC("boolExpWin", parent)
{
   insertPos  = 0;
   resizeOk   = True;
   exp        = NULL;
   blinkTimer = (XtIntervalId)NULL;
   cursorOn   = False;
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
	 "<Key>osfLeft:		BoolExpWinC-move-left()	\n\
	  <Key>osfRight:	BoolExpWinC-move-right()");

   } // End if actions not added

//
// Get keyboard focus policy
//
   unsigned char	focusPolicy;
   XtVaGetValues(*this, XmNkeyboardFocusPolicy, &focusPolicy, NULL);

   WArgList	args;

//
// appForm
//    RowColumn		opRC
//    Form		termForm
//    ScrolledWindow	expWin
//
   args.Reset();
   args.Orientation(XmHORIZONTAL);
   args.Packing(XmPACK_COLUMN);
   args.EntryAlignment(XmALIGNMENT_CENTER);
   opRC = XmCreateRowColumn(appForm, "opRC", ARGS);

   args.Reset();
   args.TopAttachment(XmATTACH_WIDGET, opRC);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   termForm = XmCreateForm(appForm, "termForm", ARGS);

   args.Reset();
   args.TopAttachment(XmATTACH_WIDGET, termForm);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_FORM);
   args.ScrollingPolicy(XmAUTOMATIC);
   args.ScrollBarDisplayPolicy(XmSTATIC);
   expWin = XmCreateScrolledWindow(appForm, "expWin", ARGS);

   XtVaGetValues(expWin, XmNclipWindow, &clipWin, NULL);
   XtVaGetValues(clipWin, XmNwidth, &clipWd, XmNheight, &clipHt, NULL);
   XtVaSetValues(clipWin, XmNnavigationType, XmTAB_GROUP, XmNtraversalOn, True,
		 NULL);

   XtAddEventHandler(clipWin, StructureNotifyMask, False,
		     (XtEventHandler)HandleResize, (XtPointer)this);

//
// buttonRC
//    PushButton	okPB
//    PushButton	applyPB
//    PushButton	cancelPB
//    PushButton	helpPB
//
   AddButtonBox();
   okPB	    = XmCreatePushButton(buttonRC, "okPB",     0,0);
   applyPB  = XmCreatePushButton(buttonRC, "applyPB",  0,0);
   clearPB  = XmCreatePushButton(buttonRC, "clearPB",  0,0);
   cancelPB = XmCreatePushButton(buttonRC, "cancelPB", 0,0);
   helpPB   = XmCreatePushButton(buttonRC, "helpPB",   0,0);

   XtAddCallback(okPB, XmNactivateCallback, (XtCallbackProc)DoOk,
          	 (XtPointer)this);
   XtAddCallback(applyPB, XmNactivateCallback, (XtCallbackProc)DoApply,
          	 (XtPointer)this);
   XtAddCallback(clearPB, XmNactivateCallback, (XtCallbackProc)DoClear,
          	 (XtPointer)this);
   XtAddCallback(cancelPB, XmNactivateCallback, (XtCallbackProc)DoHide,
 		 (XtPointer)this);
   XtAddCallback(helpPB, XmNactivateCallback, (XtCallbackProc)HalAppC::DoHelp,
 		 (char *) "helpcard");

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
// opRC
//    PushButton	lparPB
//    PushButton	rparPB
//    PushButton	andPB
//    PushButton	orPB
//    PushButton	notPB
//
   lparPB = XmCreatePushButton(opRC, "lparPB", 0,0);
   rparPB = XmCreatePushButton(opRC, "rparPB", 0,0);
   andPB  = XmCreatePushButton(opRC, "andPB",  0,0);
   orPB   = XmCreatePushButton(opRC, "orPB",   0,0);
   notPB  = XmCreatePushButton(opRC, "notPB",  0,0);

   XtAddCallback(lparPB, XmNactivateCallback, (XtCallbackProc)DoLParen,
          	 (XtPointer)this);
   XtAddCallback(rparPB, XmNactivateCallback, (XtCallbackProc)DoRParen,
          	 (XtPointer)this);
   XtAddCallback(andPB, XmNactivateCallback, (XtCallbackProc)DoAnd,
 		 (XtPointer)this);
   XtAddCallback(orPB, XmNactivateCallback, (XtCallbackProc)DoOr,
 		 (XtPointer)this);
   XtAddCallback(notPB, XmNactivateCallback, (XtCallbackProc)DoNot,
 		 (XtPointer)this);

//
// Manage widgets
//
   Widget	list[5];

   list[0] = lparPB;
   list[1] = rparPB;
   list[2] = andPB;
   list[3] = orPB;
   list[4] = notPB;
   XtManageChildren(list, 5);	// opRC children

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

   list[0] = opRC;
   list[1] = termForm;
   list[2] = expWin;
   XtManageChildren(list, 3);	// appForm children

   HandleHelp();

//
// Read window specific resources
//
   float	rate = get_float("BoolExpWinC", *this, "cursorBlinkRate", 4.0);
   blinkInterval = (rate > 0.0) ? (int)((float)1000/rate) : 0;

   XtAddCallback(*this, XmNpopupCallback, (XtCallbackProc)DoPopup,
		 (XtPointer)this);

} // End BoolExpWinC constructor

/*-------------------------------------------------------------------------
 * Destructor
 */

BoolExpWinC::~BoolExpWinC()
{
   if ( blinkTimer && halApp->xRunning ) XtRemoveTimeOut(blinkTimer);
}

/*-------------------------------------------------------------------------
 * Callback to handle initial display of window
 */

void
BoolExpWinC::DoPopup(Widget, BoolExpWinC *be, XtPointer)
{
   XtRemoveCallback(*be, XmNpopupCallback, (XtCallbackProc)DoPopup,
		    (XtPointer)be);

} // End BoolExpWinC DoPopup

/*-------------------------------------------------------------------------
 * Callback to handle press of Ok button
 */

void
BoolExpWinC::DoOk(Widget, BoolExpWinC *be, XtPointer)
{
   if ( be->Apply() ) DoHide(NULL, be, NULL);
}

/*-------------------------------------------------------------------------
 * Callback to handle press of Apply button
 */

void
BoolExpWinC::DoApply(Widget, BoolExpWinC *be, XtPointer)
{
   be->Apply();
}

Boolean
BoolExpWinC::Apply()
{
//
// Make sure the parens balance
//
   unsigned	count = expList.size();
   int		pcount = 0;
   int	i;
   for (i=0; i<count; i++) {

      ExpElemC	*elem = expList[i];

      if      ( elem->Type() == ExpElemC::LPAR ) pcount++;
      else if ( elem->Type() == ExpElemC::RPAR ) pcount--;

      if ( pcount < 0 ) { // Error
	 XtVaSetValues(*elem, XmNborderWidth, 1, NULL);
	 StringC msg("Right parenthesis is missing a corresponding left.");
	 PopupMessage(msg);
	 return False;
      }

   } // End for each element

   if ( pcount > 0 ) { // Error
      StringC msg("Not enough right parentheses in expression.");
      PopupMessage(msg);
      return False;
   }

   delete exp;
   if ( expList.size() > 0 ) {
      exp = ParseExpList(0, expList.size()-1);
      if ( !exp ) return False;
   } else {	// Blank expression always returns True
      exp = new BoolExpC(&trueExp);
   }

//
// Let user apply filter
//
   CallApplyCallbacks();
   return True;

} // End BoolExpWinC Apply

/*-------------------------------------------------------------------------
 * Callback to handle press of Clear button
 */

void
BoolExpWinC::DoClear(Widget, BoolExpWinC *be, XtPointer)
{
   unsigned	count = be->expList.size();
   int	i;
   for (i=0; i<count; i++) delete be->expList[i];

   be->expList.removeAll();
   be->insertPos = 0;
   be->DrawExp();
}

/*-------------------------------------------------------------------------
 * Callback to handle press of left parenthesis button
 */

void
BoolExpWinC::DoLParen(Widget, BoolExpWinC *be, XtPointer)
{
   WArgList	args;
   args.Reset();
   args.UserData(be);
   Widget	w = XmCreateLabel(be->expDA, "lparLabel", ARGS);
   XtManageChild(w);
   XtOverrideTranslations(w, be->translations);

   ExpElemC	*elem = new ExpElemC(ExpElemC::LPAR, w);
   be->expList.insert(elem, be->insertPos);
   be->insertPos++;

   be->DrawExp();
}

/*-------------------------------------------------------------------------
 * Callback to handle press of right parenthesis button
 */

void
BoolExpWinC::DoRParen(Widget, BoolExpWinC *be, XtPointer)
{
   WArgList	args;
   args.Reset();
   args.UserData(be);
   Widget	w = XmCreateLabel(be->expDA, "rparLabel", ARGS);
   XtManageChild(w);
   XtOverrideTranslations(w, be->translations);

   ExpElemC	*elem = new ExpElemC(ExpElemC::RPAR, w);
   be->expList.insert(elem, be->insertPos);
   be->insertPos++;

   be->DrawExp();
}

/*-------------------------------------------------------------------------
 * Callback to handle press of And button
 */

void
BoolExpWinC::DoAnd(Widget, BoolExpWinC *be, XtPointer)
{
   WArgList	args;
   args.Reset();
   args.UserData(be);
   Widget	w = XmCreateLabel(be->expDA, "andLabel", ARGS);
   XtManageChild(w);
   XtOverrideTranslations(w, be->translations);

   ExpElemC	*elem = new ExpElemC(ExpElemC::AND, w);
   be->expList.insert(elem, be->insertPos);
   be->insertPos++;

   be->DrawExp();
}

/*-------------------------------------------------------------------------
 * Callback to handle press of Or button
 */

void
BoolExpWinC::DoOr(Widget, BoolExpWinC *be, XtPointer)
{
   WArgList	args;
   args.Reset();
   args.UserData(be);
   Widget	w = XmCreateLabel(be->expDA, "orLabel", ARGS);
   XtManageChild(w);
   XtOverrideTranslations(w, be->translations);

   ExpElemC	*elem = new ExpElemC(ExpElemC::OR, w);
   be->expList.insert(elem, be->insertPos);
   be->insertPos++;

   be->DrawExp();
}

/*-------------------------------------------------------------------------
 * Callback to handle press of Not button
 */

void
BoolExpWinC::DoNot(Widget, BoolExpWinC *be, XtPointer)
{
   WArgList	args;
   args.Reset();
   args.UserData(be);
   Widget	w = XmCreateLabel(be->expDA, "notLabel", ARGS);
   XtManageChild(w);
   XtOverrideTranslations(w, be->translations);

   ExpElemC	*elem = new ExpElemC(ExpElemC::NOT, w);
   be->expList.insert(elem, be->insertPos);
   be->insertPos++;

   be->DrawExp();
}

/*-------------------------------------------------------------------------
 * Method to handle insertion of a terminal expression
 */

void
BoolExpWinC::AddTermExp(TermExpC *term)
{
   WArgList	args;
   args.Reset();
   args.UserData(this);
   Widget	w = term->CreateWidget(expDA, ARGS);
   XtManageChild(w);
   XtOverrideTranslations(w, translations);

   ExpElemC	*elem = new ExpElemC(term, w);
   expList.insert(elem, insertPos);
   insertPos++;

   DrawExp();
}

/*-------------------------------------------------------------------------
 * Method to calculate positions for all expression elements
 */

void
BoolExpWinC::DrawExp()
{
   resizeOk = False;		// Don't respond to self-induced resizes
   rowList.removeAll();
//   XtUnmanageChild(expDA);

//
// Get element sizes
//
   rowHt = cursorHt;
   unsigned	count = expList.size();
   int	i;
   for (i=0; i<count; i++) {

      ExpElemC	*elem = expList[i];

      XtVaSetValues(*elem, XmNborderWidth, 0, NULL);	// Clear border

      elem->GetSize();
      if ( elem->Height() > rowHt ) rowHt = elem->Height();
   }

//
// Position elements
//
   Position	x = marWd;
   Position	y = marHt;
   WArgList	args;

   for (i=0; i<count; i++) {

      ExpElemC	*elem = expList[i];

//
// Draw insertion cursor if necessary
//
      if ( insertPos == i ) {

	 if ( (int)x >= (int)marWd &&
	      (int)(x+cursorWd) >= (int)(clipWd-marWd) ) {
	    x = marWd;
	    y += rowHt;
	 }

	 if ( x == marWd ) rowList.append(i);

	 args.X(x);
	 args.Y(y + ((int)(rowHt - cursorHt)/(int)2)); // Center vertically
	 XtSetValues(cursorLabel, ARGS);

	 x += cursorWd;

      } // End if placing cursor here

//
// See if it will fit on this row
//
      if ( (int)x >= (int)marWd &&
	   (int)(x+elem->Width()) >= (int)(clipWd-marWd) ) {
	 x = marWd;
	 y += rowHt;
      }

      if ( x == marWd ) rowList.append(i);

//
// Set and update position
//
      args.X(x);
      args.Y(y + ((int)(rowHt - elem->Height())/(int)2)); // Center vertically
      XtSetValues(*elem, ARGS);
      x += elem->Width();

   } // End for each expression

   if ( insertPos == count ) {

      if ( (int)x >= (int)marWd && (int)(x+cursorWd) >= (int)(clipWd-marWd) ) {
	 x = marWd;
	 y += rowHt;
      }

      if ( x == marWd ) rowList.append(i);

      args.X(x);
      args.Y(y + ((int)(rowHt - cursorHt)/(int)2)); // Center vertically
      XtSetValues(cursorLabel, ARGS);
   }

//   XtManageChild(expDA);
   resizeOk = True;

} // End BoolExpWinC DrawExp

/*-----------------------------------------------------------------------
 *  Event handler for scrolled window resize
 */

void
BoolExpWinC::HandleResize(Widget, BoolExpWinC *be, XEvent*, Boolean*)
{
   XtVaGetValues(be->clipWin, XmNwidth, &be->clipWd, XmNheight, &be->clipHt, 0);
   XtVaSetValues(be->cornerDA, XmNx, (Position)(be->clipWd - be->borWd*2 - 1),
			       XmNy, (Position)(be->clipHt - be->borWd*2 - 1),
			       NULL);

   if ( be->resizeOk ) be->DrawExp();
}

/*-----------------------------------------------------------------------
 *  Method to build an expression structure for the specified portion of the
 *  expression list
 */

BoolExpC*
BoolExpWinC::ParseExpList(int first, int last)
{
   if ( last < first ) return NULL;

//
// Try to split the expression list into: (left side) operation (right side).
// Start reading from the right.  This will insure that left to right
// precedence is followed.
//
   BoolExpC	*expB;

//
// Look for a right parenthesis
//
   ExpElemC		*elem = expList[last];
   ExpElemC::ExpElemT	type = elem->Type();
   if ( type == ExpElemC::RPAR ) {

//
// Back up to the corresponding left parenthesis
//
      int	lpos = last-1;
      int	rcount = 1;
      while ( rcount > 0 && lpos >= first ) {

	 switch (expList[lpos]->Type()) {
	    case (ExpElemC::RPAR):	rcount++;	break;
	    case (ExpElemC::LPAR):	rcount--;	break;
	 }

	 if ( rcount != 0 ) lpos--;

      } // End while matching left not found

//
// Check for missing left parenthesis
//
      if ( rcount != 0 ) {
	 XtVaSetValues(*elem, XmNborderWidth, 1, NULL);
	 StringC msg("Right parenthesis is missing a corresponding left.");
	 PopupMessage(msg);
	 return NULL;
      }

//
// Check for empty expression 
//
      if ( lpos == last-1 ) {
	 XtVaSetValues(*elem, XmNborderWidth, 1, NULL);
	 StringC msg("Missing expression.");
	 PopupMessage(msg);
	 return NULL;
      }

//
// Create a new expression from the elements between the parentheses
//
      expB = ParseExpList(lpos+1, last-1);
      if ( !expB ) return NULL;

      last = lpos-1;

   } // End if last element is a right parenthesis

   else if ( type == ExpElemC::TERMINAL ) {

      expB = new BoolExpC((TermExpC*)*elem);
      last--;

   } else {
      XtVaSetValues(*elem, XmNborderWidth, 1, NULL);
      StringC msg("Syntax error.  Expecting ')' or terminal expression.");
      PopupMessage(msg);
      return NULL;
   }

//
// See if we're done
//
   if ( last < first ) return expB;

//
// Look for possible NOTs
//
   elem = expList[last];
   type = elem->Type();
   while ( type == ExpElemC::NOT ) {

//
// Negate current expression
//
      expB = new BoolExpC(BoolExpC::NOT, expB);
      last--;

//
// See if we're done
//
      if ( last < first ) return expB;

//
// Get next element
//
      elem = expList[last];
      type = elem->Type();

   } // End for each NOT element

//
// Look for an operation
//
   BoolExpC::BoolOpT	op;

   if ( type == ExpElemC::AND ) {
      op = BoolExpC::AND;
      last--;
   } else if ( type == ExpElemC::OR ) {
      op = BoolExpC::OR;
      last--;
   } else {
      XtVaSetValues(*elem, XmNborderWidth, 1, NULL);
      StringC msg("Syntax error.  Expecting AND or OR.");
      PopupMessage(msg);
      delete expB;
      return NULL;
   }

//
// Look for left side of operation
//
   if ( last < first ) {
      XtVaSetValues(*elem, XmNborderWidth, 1, NULL);
      StringC
	 msg("Syntax error.  Expecting first argument for binary operator.");
      PopupMessage(msg);
      delete expB;
      return NULL;
   }

//
// Everything else is left side
//
   BoolExpC	*expA = ParseExpList(first, last);
   if ( !expA ) {
      delete expB;
      return NULL;
   }

   return new BoolExpC(expA, op, expB);

} // End BoolExpWinC ParseExpList

/*---------------------------------------------------------------
 *  Timer proc to blink insertion cursor
 */

void
BoolExpWinC::BlinkProc(BoolExpWinC *be, XtIntervalId*)
{
   if ( be->cursorOn ) {
      XtSetMappedWhenManaged(be->cursorLabel, False);
      be->cursorOn = False;
   } else {
      XtSetMappedWhenManaged(be->cursorLabel, True);
      be->cursorOn = True;
   }

   be->blinkTimer = XtAppAddTimeOut(halApp->context, be->blinkInterval,
				    (XtTimerCallbackProc)BlinkProc,
				    (XtPointer)be);

} // End BoolExpWinC BlinkProc

/*---------------------------------------------------------------
 *  Handle a keyboard focus change event in the area
 */

void
BoolExpWinC::HandleFocusChange(Widget, BoolExpWinC *be, XEvent *ev, Boolean*)
{
   if ( be->blinkTimer ) XtRemoveTimeOut(be->blinkTimer);

   switch (ev->type) {

      case (FocusIn):
      case (EnterNotify):
	 if ( be->blinkInterval > 0 ) BlinkProc(be, NULL);
	 break;

      case (FocusOut):
      case (LeaveNotify):
	 be->cursorOn = True;
	 XtSetMappedWhenManaged(be->cursorLabel, True);
	 break;
   }

} // End BoolExpWinC HandleFocusChange

void
BoolExpWinC::HandleInsertLParen(Widget w, XKeyEvent*, String*, Cardinal*)
{
   BoolExpWinC	*be;
   XtVaGetValues(w, XmNuserData, &be, NULL);

   DoLParen(NULL, be, NULL);
}

void
BoolExpWinC::HandleInsertRParen(Widget w, XKeyEvent*, String*, Cardinal*)
{
   BoolExpWinC	*be;
   XtVaGetValues(w, XmNuserData, &be, NULL);

   DoRParen(NULL, be, NULL);
}

void
BoolExpWinC::HandleInsertAnd(Widget w, XKeyEvent*, String*, Cardinal*)
{
   BoolExpWinC	*be;
   XtVaGetValues(w, XmNuserData, &be, NULL);

   DoAnd(NULL, be, NULL);
}

void
BoolExpWinC::HandleInsertOr(Widget w, XKeyEvent*, String*, Cardinal*)
{
   BoolExpWinC	*be;
   XtVaGetValues(w, XmNuserData, &be, NULL);

   DoOr(NULL, be, NULL);
}

void
BoolExpWinC::HandleInsertNot(Widget w, XKeyEvent*, String*, Cardinal*)
{
   BoolExpWinC	*be;
   XtVaGetValues(w, XmNuserData, &be, NULL);

   DoNot(NULL, be, NULL);
}

void
BoolExpWinC::HandleDeleteLeft(Widget w, XKeyEvent*, String*, Cardinal*)
{
   BoolExpWinC	*be;
   XtVaGetValues(w, XmNuserData, &be, NULL);

   if ( be->insertPos > 0 ) {
      be->insertPos--;
      delete be->expList[be->insertPos];
      be->expList.remove(be->insertPos);
      be->DrawExp();
   }
}

void
BoolExpWinC::HandleDeleteRight(Widget w, XKeyEvent*, String*, Cardinal*)
{
   BoolExpWinC	*be;
   XtVaGetValues(w, XmNuserData, &be, NULL);

   if ( be->insertPos < be->expList.size() ) {
      delete be->expList[be->insertPos];
      be->expList.remove(be->insertPos);
      be->DrawExp();
   }
}

void
BoolExpWinC::HandleMoveLeft(Widget w, XKeyEvent*, String*, Cardinal*)
{
   BoolExpWinC	*be;
   XtVaGetValues(w, XmNuserData, &be, NULL);

   if ( be->insertPos > 0 ) {
      be->insertPos--;
      be->DrawExp();
   }
}

void
BoolExpWinC::HandleMoveRight(Widget w, XKeyEvent*, String*, Cardinal*)
{
   BoolExpWinC	*be;
   XtVaGetValues(w, XmNuserData, &be, NULL);

   if ( be->insertPos < be->expList.size() ) {
      be->insertPos++;
      be->DrawExp();
   }
}

void
BoolExpWinC::HandleMovePointer(Widget w, XButtonEvent *ev, String*, Cardinal*)
{
   BoolExpWinC	*be;
   XtVaGetValues(w, XmNuserData, &be, NULL);

   XmProcessTraversal(be->expDA, XmTRAVERSE_CURRENT);

//
// Translate event coordinates to expDA if this is a child
//
   if ( XtParent(w) == be->expDA ) {
      Position	x, y;
      XtVaGetValues(w, XmNx, &x, XmNy, &y, NULL);
      ev->x += x;
      ev->y += y;
   }

   int	row = 0;
   int	y = be->marHt + be->rowHt;
   while ( ev->y > y ) {
      y += be->rowHt;
      row++;
   }

   unsigned	rowCount = be->rowList.size();
   if ( row >= rowCount ) row = rowCount - 1;

   int	first = *be->rowList[row];
   int	last;
   if ( row == rowCount - 1 ) last = be->expList.size() - 1;
   else			      last = *be->rowList[row+1] - 1;
   
   int	x = be->marWd;
   Boolean	found = (ev->x <= x);
   int	i;
   for (i=first; !found && i<=last; i++) {

      ExpElemC	*elem = be->expList[i];

//
// Add space for insertion cursor if necessary
//
      if ( i != first && be->insertPos == i ) x += be->cursorWd;

//
// Add space for current element
//
      x += elem->Width();
      found = (ev->x <= x);

   } // End for each expression

   if (found) i--;

//
// Place insertion cursor in front of i
//
   be->insertPos = i;
   be->DrawExp();

} // End BoolExpWinC HandleMovePointer
