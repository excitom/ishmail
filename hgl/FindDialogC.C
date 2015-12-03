/*
 * $Id: FindDialogC.C,v 1.2 2000/05/07 12:26:10 fnevgeny Exp $
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

#include "FindDialogC.h"
#include "WArgList.h"
#include "VItemC.h"
#include "VBoxC.h"

#include <Xm/Label.h>
#include <Xm/TextF.h>
#include <Xm/RowColumn.h>
#include <Xm/PushB.h>

RegexC	FindDialogC::findPat;

/*-----------------------------------------------------------------------
 *  Constructor
 */

FindDialogC::FindDialogC(VBoxC *vb)
 : HalDialogC(FIND_DIALOG_NAME, vb->ViewForm())
{
   viewBox = vb;

   WArgList	args;

   Widget	patLabel = XmCreateLabel(appForm, "patLabel", 0,0);

   args.Reset();
   args.LeftAttachment(XmATTACH_WIDGET, patLabel);
   args.RightAttachment(XmATTACH_FORM);
   patField = XmCreateTextField(appForm, "patField", ARGS);

   XtManageChild(patLabel);
   XtManageChild(patField);

   XtAddCallback(patField, XmNvalueChangedCallback, (XtCallbackProc)PatChange,
		 (XtPointer)this);

//
// Create action buttons
//
   AddButtonBox();
   Widget
      firstPB = XmCreatePushButton(buttonRC, "firstPB", 0,0),
      nextPB  = XmCreatePushButton(buttonRC, "nextPB",  0,0),
      donePB  = XmCreatePushButton(buttonRC, "donePB",  0,0),
      helpPB  = XmCreatePushButton(buttonRC, "helpPB",  0,0);

   XtManageChild(firstPB);
   XtManageChild(nextPB);
   XtManageChild(donePB);
   XtManageChild(helpPB);

   XtVaSetValues(appForm, XmNdefaultButton, firstPB, NULL);

   XtAddCallback(firstPB, XmNactivateCallback, (XtCallbackProc)DoFirst,
		 (XtPointer)this);
   XtAddCallback(nextPB, XmNactivateCallback, (XtCallbackProc)DoNext,
		 (XtPointer)this);
   XtAddCallback(donePB, XmNactivateCallback, (XtCallbackProc)DoDone,
		 (XtPointer)this);

   HandleHelp();

//
// Register the find function
//
   viewBox->SetFindFunction((Boolean (*)(const VItemC&))Find);

} // End FindDialogC Constructor

/*-----------------------------------------------------------------------
 *  Callback to handle FIRST
 */

void
FindDialogC::DoFirst(Widget, FindDialogC *fd, XtPointer)
{
   fd->viewBox->FindFirst();

} // End FindDialogC DoFirst

/*-----------------------------------------------------------------------
 *  Callback to handle NEXT
 */

void
FindDialogC::DoNext(Widget, FindDialogC *fd, XtPointer)
{
   fd->viewBox->FindNext();

} // End FindDialogC DoNext


/*-----------------------------------------------------------------------
 *  Callback to handle DONE
 */

void
FindDialogC::DoDone(Widget, FindDialogC *fd, XtPointer)
{
   fd->Hide();

} // End FindDialogC DoDone

/*-----------------------------------------------------------------------
 *  Callback to handle pattern change
 */

void
FindDialogC::PatChange(Widget, FindDialogC *fd, XtPointer)
{
//
// Get string from field
//
   char	*cs = XmTextFieldGetString(fd->patField);
   fd->findPat = cs;
   XtFree(cs);

} // End FindDialogC DoDone


/*-----------------------------------------------------------------------
 *  Find function
 */

Boolean
FindDialogC::Find(const VItemC& item)
{
   return ( !findPat.size() || findPat.match(item.Name()) );

} // End FindDialogC Find
