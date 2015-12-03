/*
 * $Id: FiltDialogC.C,v 1.2 2000/05/07 12:26:10 fnevgeny Exp $
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

#include "FiltDialogC.h"
#include "WArgList.h"
#include "VItemC.h"
#include "VBoxC.h"

#include <Xm/Label.h>
#include <Xm/TextF.h>
#include <Xm/RowColumn.h>
#include <Xm/PushB.h>

RegexC	FiltDialogC::filterPat;
Boolean	FiltDialogC::invert;

/*-----------------------------------------------------------------------
 *  Constructor
 */

FiltDialogC::FiltDialogC(VBoxC *vb)
 : HalDialogC(FILT_DIALOG_NAME, vb->ViewForm())
{
   viewBox = vb;

   WArgList	args;

   Widget	patLabel = XmCreateLabel(appForm, "patLabel", 0,0);

   args.Reset();
   args.LeftAttachment(XmATTACH_WIDGET, patLabel);
   args.RightAttachment(XmATTACH_FORM);
   patField = XmCreateTextField(appForm, "patField", ARGS);

   args.Reset();
   args.TopAttachment(XmATTACH_WIDGET, patField);
   args.RadioBehavior(True);
   args.RadioAlwaysOne(True);
   args.Orientation(XmVERTICAL);
   args.Packing(XmPACK_COLUMN);
   Widget	typeRC = XmCreateRowColumn(appForm, "typeRC", ARGS);

   XtManageChild(patLabel);
   XtManageChild(patField);
   XtManageChild(typeRC);

   pickPassTB = XmCreateToggleButton(typeRC, "selectTB",   0,0);
   pickFailTB = XmCreateToggleButton(typeRC, "pickFailTB", 0,0);
   hidePassTB = XmCreateToggleButton(typeRC, "hidePassTB", 0,0);
   hideFailTB = XmCreateToggleButton(typeRC, "displayTB",  0,0);

   XtManageChild(pickPassTB);
   XtManageChild(pickFailTB);
   XtManageChild(hidePassTB);
   XtManageChild(hideFailTB);

   XmToggleButtonSetState(pickPassTB, True, False);

//
// Create action buttons
//
   AddButtonBox();
   Widget
      okPB     = XmCreatePushButton(buttonRC, "okPB",     0,0),
      applyPB  = XmCreatePushButton(buttonRC, "applyPB",  0,0),
      cancelPB = XmCreatePushButton(buttonRC, "cancelPB", 0,0),
      helpPB   = XmCreatePushButton(buttonRC, "helpPB",   0,0);

   XtManageChild(okPB);
   XtManageChild(applyPB);
   XtManageChild(cancelPB);
   XtManageChild(helpPB);

   XtVaSetValues(appForm, XmNdefaultButton, okPB, NULL);

   XtAddCallback(okPB, XmNactivateCallback, (XtCallbackProc)DoOk,
		 (XtPointer)this);
   XtAddCallback(applyPB, XmNactivateCallback, (XtCallbackProc)DoApply,
		 (XtPointer)this);
   XtAddCallback(cancelPB, XmNactivateCallback, (XtCallbackProc)DoCancel,
		 (XtPointer)this);

   HandleHelp();

//
// Register the filter function
//
   viewBox->SetFilterFunction((Boolean (*)(const VItemC&))Filter);

} // End FiltDialogC Constructor

/*-----------------------------------------------------------------------
 *  Callback to handle OK
 */

void
FiltDialogC::DoOk(Widget, FiltDialogC *fd, XtPointer)
{
   fd->DoApply(NULL, fd, NULL);
   fd->DoCancel(NULL, fd, NULL);

} // End FiltDialogC DoOk

/*-----------------------------------------------------------------------
 *  Callback to handle APPLY
 */

void
FiltDialogC::DoApply(Widget, FiltDialogC *This, XtPointer)
{
//
// Get the filter pattern
//
   char	*cs = XmTextFieldGetString(This->patField);
   filterPat = cs;
   XtFree(cs);

   if ( XmToggleButtonGetState(This->pickPassTB) ||
	XmToggleButtonGetState(This->pickFailTB) )
      This->viewBox->FilterOutput(SELECT_FILTER_OUTPUT);
   else
      This->viewBox->FilterOutput(DISPLAY_FILTER_OUTPUT);

   if ( XmToggleButtonGetState(This->pickFailTB) ||
	XmToggleButtonGetState(This->hidePassTB) )
      invert = True;
   else
      invert = False;

   This->viewBox->Filter();

} // End FiltDialogC DoApply

/*-----------------------------------------------------------------------
 *  Callback to handle CANCEL
 */

void
FiltDialogC::DoCancel(Widget, FiltDialogC *fd, XtPointer)
{
   fd->Hide();
}

/*-----------------------------------------------------------------------
 *  Filter function
 */

Boolean
FiltDialogC::Filter(const VItemC& item)
{
   Boolean	pass = ( !filterPat.size() || filterPat.match(item.Name()) );
   return (invert ? !pass : pass);
}
