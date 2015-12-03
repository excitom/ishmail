/*
 * $Id: ButtonEditWinC.C,v 1.2 2000/05/07 12:26:11 fnevgeny Exp $
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

#include <config.h>

#include "ButtonEditWinC.h"
#include "ButtonEntryC.h"

#include <hgl/WArgList.h>
#include <hgl/RowColC.h>
#include <hgl/HalAppC.h>
#include <hgl/rsrc.h>
#include <hgl/TextMisc.h>

#include <Xm/PushB.h>
#include <Xm/Label.h>
#include <Xm/TextF.h>

/*---------------------------------------------------------------
 *  Main window constructor
 */

ButtonEditWinC::ButtonEditWinC(Widget parent)
: HalDialogC("buttonEditWin", parent)
{
   WArgList	args;

//
// Create appForm hierarchy
//
// appForm
//    RowColC	entryRC
//	Label		labelLabel
//	TextField	labelTF
//	Label		abbrevLabel
//	TextField	abbrevTF
//	Label		orderLabel
//	TextField	orderTF
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_FORM);
   RowColC	*entryRC = new RowColC(appForm, "entryRC", ARGS);
   entryRC->Defer(True);

//
// Set up 2 Columns
//
   entryRC->SetOrientation(RcROW_MAJOR);
   entryRC->SetColCount(2);
   entryRC->SetColAlignment(0, XmALIGNMENT_END);
   entryRC->SetColAlignment(1, XmALIGNMENT_CENTER);
   entryRC->SetColWidthAdjust(0, RcADJUST_NONE);
   entryRC->SetColWidthAdjust(1, RcADJUST_ATTACH);
   entryRC->SetColResize(0, False);
   entryRC->SetColResize(1, True);

   Widget	labelLabel = XmCreateLabel(*entryRC, "labelLabel", 0,0);
   labelTF  = CreateTextField(*entryRC, "labelTF", 0,0);
   Widget	abbrevLabel = XmCreateLabel(*entryRC, "abbrevLabel", 0,0);
   abbrevTF = CreateTextField(*entryRC, "abbrevTF", 0,0);
   Widget	orderLabel = XmCreateLabel(*entryRC, "orderLabel", 0,0);
   orderTF  = CreateTextField(*entryRC, "orderTF", 0,0);

   XtVaSetValues(labelTF, XmNeditable, False, NULL);

   Widget	wlist[6];
   wlist[0] = labelLabel;
   wlist[1] = labelTF;
   wlist[2] = abbrevLabel;
   wlist[3] = abbrevTF;
   wlist[4] = orderLabel;
   wlist[5] = orderTF;
   XtManageChildren(wlist, 6);	// entryRC children

   XtManageChild(*entryRC);	// appForm children
   entryRC->SetChildren(wlist, 6);
   entryRC->Defer(False);

//
// Create buttonRC hierarchy
//
// buttonRC
//    PushButton	okPB
//    PushButton	applyPB
//    PushButton	cancelPB
//    PushButton	helpPB
//
   AddButtonBox();
   Widget
      okPB     = XmCreatePushButton(buttonRC, "okPB",	  0,0),
      applyPB  = XmCreatePushButton(buttonRC, "applyPB",  0,0),
      cancelPB = XmCreatePushButton(buttonRC, "cancelPB", 0,0),
      helpPB   = XmCreatePushButton(buttonRC, "helpPB",	  0,0);

   wlist[0] = okPB;
   wlist[1] = applyPB;
   wlist[2] = cancelPB;
   wlist[3] = helpPB;
   XtManageChildren(wlist, 4);	// buttonRC children

   XtAddCallback(okPB, XmNactivateCallback,	(XtCallbackProc)DoOk,
   		 (XtPointer)this);
   XtAddCallback(applyPB, XmNactivateCallback,	(XtCallbackProc)DoApply,
   		 (XtPointer)this);
   XtAddCallback(cancelPB, XmNactivateCallback, (XtCallbackProc)DoHide,
   		 (XtPointer)this);
   XtAddCallback(helpPB, XmNactivateCallback, (XtCallbackProc)HalAppC::DoHelp,
   		 (XtPointer)"helpcard");

   HandleHelp();

} // End ButtonEditWinC constructor

/*---------------------------------------------------------------
 *  Method to display dialog
 */

void
ButtonEditWinC::Show(ButtonEntryC *be)
{
   entry = be;

   XmTextFieldSetString(labelTF,  be->label);
   XmTextFieldSetString(abbrevTF, be->abbrev);

   StringC	order;
   order += be->posIndex;
   XmTextFieldSetString(orderTF, order);

   HalDialogC::Show();
}

/*---------------------------------------------------------------
 *  Callback to handle ok
 */

void
ButtonEditWinC::DoOk(Widget, ButtonEditWinC *This, XtPointer)
{
   if ( This->Apply() ) This->Hide();
}

/*---------------------------------------------------------------
 *  Callback to handle apply
 */

void
ButtonEditWinC::DoApply(Widget, ButtonEditWinC *This, XtPointer)
{
   This->Apply();
}

/*---------------------------------------------------------------
 *  Method to handle apply
 */

Boolean
ButtonEditWinC::Apply()
{
//
// Check the abbreviation field
//
   if ( XmTextFieldGetLastPosition(abbrevTF) <= 0 )
      entry->abbrev = entry->label;
   else {
      char	*cs = XmTextFieldGetString(abbrevTF);
      entry->abbrev = cs;
      XtFree(cs);
   }

//
// Check the value field
//
   if ( XmTextFieldGetLastPosition(orderTF) > 0 ) {
      char	*cs = XmTextFieldGetString(orderTF);
      entry->posIndex = atoi(cs);
      XtFree(cs);
   }

//
// Call callbacks
//
   CallApplyCallbacks();

   return True;

} // End Apply
