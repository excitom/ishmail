/*
 * $Id: UndelWinC.C,v 1.3 2000/08/07 11:05:17 evgeny Exp $
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

#include "UndelWinC.h"
#include "MsgItemC.h"
#include "IshAppC.h"
#include "SumPrefC.h"
#include "MainWinC.h"

#include <hgl/WArgList.h>
#include <hgl/TBoxC.h>
#include <hgl/VBoxC.h>
#include <hgl/FieldViewC.h>
#include <hgl/VItemC.h>
#include <hgl/rsrc.h>
#include <hgl/RowColC.h>

#include <Xm/Frame.h>
#include <Xm/RowColumn.h>
#include <Xm/PushB.h>

/*---------------------------------------------------------------
 *  Main window constructor
 */

UndelWinC::UndelWinC(Widget parent) : HalDialogC("undelWin", parent)
{
   WArgList	args;

//
// Create appForm hierarchy
//
// appForm
//    TBoxC	msgBox
//
   args.Reset();
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   args.TopAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_FORM);
   msgBox = new TBoxC(appForm, "msgBox");
   XtSetValues(*msgBox, ARGS);

//
// Create msgBox hierarchy
//
//   msgBox
//	PushButton	selectPB
//	PushButton	deselectPB
//
   Widget	bb = msgBox->ButtonBox();
   selectPB   = XmCreatePushButton(bb, "selectPB",   0,0);
   deselectPB = XmCreatePushButton(bb, "deselectPB", 0,0);

   XtAddCallback(selectPB, XmNactivateCallback, (XtCallbackProc)DoSelect,
          	 (XtPointer)this);
   XtAddCallback(deselectPB, XmNactivateCallback, (XtCallbackProc)DoDeselect,
          	 (XtPointer)this);

//
// Add field view to msgBox
//
   VBoxC&	viewBox = msgBox->VBox();
   viewBox.DisablePopupMenu();
   viewBox.HideStatus();
   viewBox.AddSelectChangeCallback((CallbackFn *)ChangeSelection, this);
   viewBox.SetCompareFunction((CompareFn)MsgItemC::MsgItemCompare);

   fieldView = new FieldViewC(&viewBox);
   viewType = viewBox.AddView(*fieldView);
   if ( ishApp->sumPrefs->showPixmaps ) fieldView->ShowPixmaps();
   else					fieldView->HidePixmaps();
   ishApp->sumPrefs->UpdateFields(fieldView);

//
// Create buttonRC hierarchy
//
//   buttonRC
//	PushButton	okPB
//	PushButton	applyPB
//	PushButton	cancelPB
//	PushButton	helpPB
//
   AddButtonBox();
   okPB	           = XmCreatePushButton(buttonRC, "okPB",     0,0);
   applyPB         = XmCreatePushButton(buttonRC, "applyPB",  0,0);
   Widget cancelPB = XmCreatePushButton(buttonRC, "cancelPB", 0,0);
   Widget helpPB   = XmCreatePushButton(buttonRC, "helpPB",   0,0);

   XtAddCallback(okPB, XmNactivateCallback, (XtCallbackProc)DoOk,
          	 (XtPointer)this);
   XtAddCallback(applyPB, XmNactivateCallback, (XtCallbackProc)DoApply,
          	 (XtPointer)this);
   XtAddCallback(cancelPB, XmNactivateCallback, (XtCallbackProc)DoCancel,
 		 (XtPointer)this);

   XtVaSetValues(appForm, XmNdefaultButton, okPB, NULL);

//
// Manage widgets
//
   Widget	list[4];

   list[0] = okPB;
   list[1] = applyPB;
   list[2] = cancelPB;
   list[3] = helpPB;
   XtManageChildren(list, 4);	// buttonRC children

   list[0] = selectPB;
   list[1] = deselectPB;
   XtManageChildren(list, 2);	// msgBox children

   XtManageChild(*msgBox);	// appForm children

   HandleHelp();

   XtAddCallback(shell, XmNpopupCallback, (XtCallbackProc)DoPopup,
		 (XtPointer)this);

} // End constructor

/*---------------------------------------------------------------
 *  Destructor
 */

UndelWinC::~UndelWinC()
{
   delete fieldView;
   delete msgBox;
}

/*---------------------------------------------------------------
 *  Callback routine to handle initial display.
 */

void
UndelWinC::DoPopup(Widget, UndelWinC *This, XtPointer)
{
   This->msgBox->VBox().ViewType(This->viewType);
}

/*---------------------------------------------------------------
 *  Method to set list of deleted messages
 */

void
UndelWinC::SetItems(VItemListC& list)
{
   VBoxC&	box = msgBox->VBox();
   box.RemoveAllItems();
   box.AddItems(list);
   box.Refresh();

   unsigned	count = list.size();
   for (int i=0; i<count; i++)
      list[i]->SetMenu(NULL);
}

/*---------------------------------------------------------------
 *  Method to set add a deleted message
 */

void
UndelWinC::AddItem(VItemC& item)
{
   msgBox->VBox().AddItem(item);
   item.SetMenu(NULL);
}

/*---------------------------------------------------------------
 *  Method to remove all messages
 */

void
UndelWinC::Clear()
{
   VBoxC&	box = msgBox->VBox();
   box.RemoveAllItems();
   box.Refresh();
}

/*---------------------------------------------------------------
 *  Callback to handle select all
 */

void
UndelWinC::DoSelect(Widget, UndelWinC *This, XtPointer)
{
   This->msgBox->VBox().SelectAllItems();
}

/*---------------------------------------------------------------
 *  Callback to handle deselect all
 */

void
UndelWinC::DoDeselect(Widget, UndelWinC *This, XtPointer)
{
   This->msgBox->VBox().DeselectAllItems();
}

/*---------------------------------------------------------------
 *  Callback to handle ok
 */

void
UndelWinC::DoOk(Widget, UndelWinC *This, XtPointer)
{
   This->DoApply(NULL, This, NULL);
   This->Hide();
}

/*---------------------------------------------------------------
 *  Callback to handle apply
 */

void
UndelWinC::DoApply(Widget, UndelWinC *This, XtPointer)
{
   VBoxC&	box = This->msgBox->VBox();
   VItemListC	list = box.SelItems();

//
// Remove selected items from this view box
//
   box.RemoveItems(list);
   box.Refresh();

//
// Add them to the main view box
//
   ishApp->mainWin->UndeleteItems(list);

} // End DoApply

/*---------------------------------------------------------------
 *  Callback to handle cancel
 */

void
UndelWinC::DoCancel(Widget, UndelWinC *This, XtPointer)
{
   This->Hide();
}

/*---------------------------------------------------------------
 *  Callback routine to handle selection of messages
 */

void
UndelWinC::ChangeSelection(VBoxC *vb, UndelWinC *This)
{
   unsigned	scount = vb->SelItems().size();
   unsigned	icount = vb->Items().size();

//
// Update buttons sensitivities based on number of selected items
//
   XtSetSensitive(This->selectPB,   scount<icount);
   XtSetSensitive(This->deselectPB, scount>0);
   XtSetSensitive(This->okPB,       scount>0);
   XtSetSensitive(This->applyPB,    scount>0);

} // End ChangeSelection

/*------------------------------------------------------------------------
 * Method to update summary field characteristics
 */

void
UndelWinC::UpdateFields()
{
   VBoxC&	box = msgBox->VBox();

   BusyCursor(True);

   ishApp->sumPrefs->UpdateFields(fieldView);

//
// Loop through all message items and reset the fields
//
   VItemListC	list = box.Items();
   unsigned	count = list.size();
   for (int i=0; i<count; i++) {
      MsgItemC	*item = (MsgItemC *)list[i];
      item->LoadFields();
   }

   box.Refresh();
   BusyCursor(False);

} // End UpdateFields
