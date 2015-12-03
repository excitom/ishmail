/*
 * $Id: SortDialogC.C,v 1.2 2000/05/07 12:26:11 fnevgeny Exp $
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

#include "SortDialogC.h"
#include "WArgList.h"
#include "VItemC.h"
#include "VBoxC.h"

#include <Xm/RowColumn.h>
#include <Xm/PushB.h>

/*-----------------------------------------------------------------------
 *  Constructor
 */

SortDialogC::SortDialogC(VBoxC *vb)
 : HalDialogC(SORT_DIALOG_NAME, vb->ViewForm())
{
   viewBox = vb;

   WArgList	args;

//
// Create selection buttons
//
   args.Reset();
   args.RadioBehavior(True);
   args.RadioAlwaysOne(True);
   args.Orientation(XmVERTICAL);
   args.Packing(XmPACK_COLUMN);
   Widget	typeRC = XmCreateRowColumn(appForm, "typeRC", ARGS);
   XtManageChild(typeRC);

   sortAZ = XmCreateToggleButton(typeRC, "sortAZ", 0,0);
   sortZA = XmCreateToggleButton(typeRC, "sortZA", 0,0);

   XtManageChild(sortAZ);
   XtManageChild(sortZA);

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
// Start with A-Z sorting
//
   XmToggleButtonSetState(sortAZ, True, False);
   type = sortAZ;
   viewBox->SetCompareFunction(SortDialogC::CompareAZ);

} // End SortDialogC Constructor

/*-----------------------------------------------------------------------
 *  Callback to handle OK
 */

void
SortDialogC::DoOk(Widget, SortDialogC *sd, XtPointer)
{
   sd->DoApply(NULL, sd, NULL);
   sd->DoCancel(NULL, sd, NULL);

} // End SortDialogC DoOk

/*-----------------------------------------------------------------------
 *  Callback to handle APPLY
 */

void
SortDialogC::DoApply(Widget, SortDialogC *sd, XtPointer)
{
//
// Read toggle buttons
//
   if ( XmToggleButtonGetState(sd->sortAZ) ) {
      if ( sd->type != sd->sortAZ ) {
	 sd->type = sd->sortAZ;
	 sd->viewBox->SetCompareFunction(SortDialogC::CompareAZ);
      }
   } else {
      if ( sd->type != sd->sortZA ) {
	 sd->type = sd->sortZA;
	 sd->viewBox->SetCompareFunction(SortDialogC::CompareZA);
      }
   }

   sd->viewBox->Sort();

} // End SortDialogC DoApply

/*-----------------------------------------------------------------------
 *  Callback to handle CANCEL
 */

void
SortDialogC::DoCancel(Widget, SortDialogC *sd, XtPointer)
{
   sd->Hide();

} // End SortDialogC DoCancel

/*-----------------------------------------------------------------------
 *  Compare function for forward alphabetical order
 */

int
SortDialogC::CompareAZ(const void *a, const void *b)
{
   VItemC	*via = *(VItemC **)a;
   VItemC	*vib = *(VItemC **)b;

   return ( via->Name().compare(vib->Name()) );

} // End SortDialogC::CompareAZ

/*-----------------------------------------------------------------------
 *  Compare function for reverse alphabetical order
 */

int
SortDialogC::CompareZA(const void *a, const void *b)
{
   VItemC	*via = *(VItemC **)a;
   VItemC	*vib = *(VItemC **)b;

   return ( vib->Name().compare(via->Name()) );

} // End CompareZA
