/*
 * $Id: DictEditWinC.C,v 1.5 2000/08/07 11:05:16 evgeny Exp $
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

#include "DictEditWinC.h"
#include "EntryEditWinC.h"
#include "Misc.h"

#include <hgl/WArgList.h>
#include <hgl/TBoxC.h>
#include <hgl/VBoxC.h>
#include <hgl/FieldViewC.h>
#include <hgl/rsrc.h>
#include <hgl/HalAppC.h>
#include <hgl/RowColC.h>

#include <Xm/Form.h>
#include <Xm/PushB.h>
#include <Xm/Label.h>
#include <Xm/TextF.h>
#include <Xm/RowColumn.h>
#include <Xm/MessageB.h>
#include <Xm/AtomMgr.h>
#include <Xm/Protocols.h>
#include <X11/IntrinsicP.h>
#include <X11/CoreP.h>

/*---------------------------------------------------------------
 *  Main window constructor
 */

DictEditWinC::DictEditWinC(Widget parent, const char *name)
: OptWinC(parent, name)
{
   WArgList	args;
   Widget	wlist[4];

   itemList.AllowDuplicates(FALSE);
   addList.AllowDuplicates(FALSE);
   modList.AllowDuplicates(FALSE);
   remList.AllowDuplicates(FALSE);
   keyList.AllowDuplicates(FALSE);
   selectedItem = NULL;
   lastAction = NONE;

//
// Create appForm hierarchy
//
// appForm
//    TBoxC	entryBox
//
   args.Reset();
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   args.TopAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_FORM);
   taskBox = new TBoxC(appForm, "entryBox");
   XtSetValues(*taskBox, ARGS);
   entryBox = &taskBox->VBox();
   entryBox->DisablePopupMenu();
   entryBox->HideStatus();

//
// Add field view to entryBox
//
   entryBox->AddSelectChangeCallback((CallbackFn *)ChangeSelection, this);

   fieldView = new FieldViewC(entryBox);
   viewType  = entryBox->AddView(*fieldView);

//
// Set field titles
//
   StringListC	titleList;
   titleList.AllowDuplicates(TRUE);
   titleList.append(get_string(*entryBox, "keyColumnTitle", "Key"));
   titleList.append(get_string(*entryBox, "valueColumnTitle", "Value"));
   fieldView->SetTitles(titleList);
   fieldView->HidePixmaps();

//
// Create entryBox buttons
//	PushButton	addPB
//	PushButton	insertPB
//	PushButton	modifyPB
//	PushButton	removePB
//
   Widget	bb = taskBox->ButtonBox();
   addPB    = XmCreatePushButton(bb, "addPB",    0,0);
   insertPB = XmCreatePushButton(bb, "insertPB", 0,0);
   modifyPB = XmCreatePushButton(bb, "modifyPB", 0,0);
   removePB = XmCreatePushButton(bb, "removePB", 0,0);
   XtAddCallback(addPB, XmNactivateCallback, (XtCallbackProc)DoAdd,
          	 (XtPointer)this);
   XtAddCallback(insertPB, XmNactivateCallback, (XtCallbackProc)DoInsert,
 		 (XtPointer)this);
   XtAddCallback(modifyPB, XmNactivateCallback, (XtCallbackProc)DoModify,
 		 (XtPointer)this);
   XtAddCallback(removePB, XmNactivateCallback, (XtCallbackProc)DoRemove,
 		 (XtPointer)this);

   XtSetSensitive(insertPB, False);
   XtSetSensitive(modifyPB, False);
   XtSetSensitive(removePB, False);

   wlist[0] = addPB;
   wlist[1] = insertPB;
   wlist[2] = modifyPB;
   wlist[3] = removePB;
   XtManageChildren(wlist, 4);	// taskBox->ButtonBox() children

   wlist[0] = *taskBox;
   XtManageChildren(wlist, 1);	// appForm children

   HandleHelp();
   HandleHelp(*entryBox);

//
// Create dialog for editing entries
//
   StringC	ename = name;
   ename += "EntryWin";
   entryEditWin = new EntryEditWinC(*this, ename);

} // End DictEditWinC constructor

/*---------------------------------------------------------------
 *  Destructor
 */

DictEditWinC::~DictEditWinC()
{
   delete fieldView;
   delete taskBox;
   delete entryEditWin;

   unsigned	count = itemList.size();
   for (int i=0; i<count; i++) delete itemList[i];
}

/*---------------------------------------------------------------
 *  Callback routine to handle selection of messages
 */

void
DictEditWinC::ChangeSelection(VBoxC *vbox, DictEditWinC *This)
{
   VItemListC&	selItems = vbox->SelItems();
   if ( selItems.size() > 0 ) This->selectedItem = selItems[0];
   else			      This->selectedItem = NULL;
   This->UpdateButtons();

//
// Display the new selected item if the operation is a modify.
//
   if ( This->entryEditWin->IsShown() && selItems.size() == 1 &&
	This->lastAction == MODIFY )
      DoModify(NULL, This, NULL);

} // End ChangeSelection

/*---------------------------------------------------------------
 *  Callback to handle requested add
 */

void
DictEditWinC::DoAdd(Widget, DictEditWinC *This, XtPointer)
{
   This->entryEditWin->SetKey("");
   This->entryEditWin->SetValue("");
   set_sensitivity(This->entryEditWin->KeyTF(), True);
   This->entryEditWin->Show((CallbackFn *)FinishAdd, This);
   This->lastAction = ADD;
}

/*---------------------------------------------------------------
 *  Callback to handle completed add
 */

void
DictEditWinC::FinishAdd(void*, DictEditWinC *This)
{
   This->AddItem(This->keyList.size());
}

/*---------------------------------------------------------------
 *  Method to add a new item
 */

void
DictEditWinC::AddItem(int pos)
{
   StringC&	keyString = entryEditWin->KeyString();
   int	index = keyList.indexOf(keyString);
   if ( index != keyList.NULL_INDEX ) {
      entryBox->SelectItemOnly(*entryBox->Items()[index]);
      entryEditWin->Show((CallbackFn *)FinishAdd, this);
      PopupMessage("Item already exists.");
      return;
   }

   VItemC *item = new VItemC(keyString);
   item->SetUserData(this);
   item->AddOpenCallback((CallbackFn *)OpenEntry, this);

   StringListC	fieldList;
   fieldList.AllowDuplicates(TRUE);
   fieldList.add(keyString);
   fieldList.add(entryEditWin->ValueString());
   fieldList.add(keyString);
   fieldList.add(entryEditWin->ValueString());
   item->FieldList(fieldList);

   itemList.insert(item, pos);
   keyList.insert(keyString, pos);
   addList.add(item);

   entryBox->RemoveAllItems();
   entryBox->AddItems(itemList);
   entryBox->Refresh();

   UpdateButtons();

} // End AddItem

/*---------------------------------------------------------------
 *  Callback to handle remove
 */

void
DictEditWinC::DoRemove(Widget, DictEditWinC *This, XtPointer)
{
   if ( !This->ConfirmDelete(*This) ) return;

//
// Get the list of selected items
//
   VItemListC&	list = This->entryBox->SelItems();

//
// Add items to removed list
//
   unsigned	count = list.size();
   for (int i=0; i<count; i++) {
      VItemC	*item = list[i];
      This->remList.add(item);
      This->itemList.remove(item);
      This->keyList.remove(item->Name());
   }

//
// Remove the items from the view box
//
   This->entryBox->RemoveSelectedItems();
   if ( This->selectedItem ) {
      This->selectedItem = NULL;
      This->entryEditWin->Hide();
   }

   This->UpdateButtons();
}

/*---------------------------------------------------------------
 *  Callback to handle requested modify
 */

void
DictEditWinC::DoModify(Widget, DictEditWinC *This, XtPointer)
{
   This->entryEditWin->SetKey(This->selectedItem->Name());
   This->entryEditWin->SetValue(*(This->selectedItem->FieldList()[1]));
   This->entryEditWin->Show((CallbackFn *)FinishModify, This);
   This->lastAction = MODIFY;
}

/*---------------------------------------------------------------
 *  Callback to handle completed modify
 */

void
DictEditWinC::FinishModify(void*, DictEditWinC *This)
{
//
// If the name has changed, delete the original and add the new
//
   if ( This->entryEditWin->KeyString() != This->selectedItem->Name() ) {

//
// Make sure the new value is not already defined
//
      StringC&	keyString = This->entryEditWin->KeyString();
      int	index = This->keyList.indexOf(keyString);
      if ( index != This->keyList.NULL_INDEX ) {
	 This->entryBox->SelectItemOnly(*This->entryBox->Items()[index]);
	 This->entryEditWin->Show((CallbackFn *)FinishModify, This);
	 This->PopupMessage("Item already exists.");
	 return;
      }

//
// Add a new item to the list
//
      VItemC	*item = This->selectedItem;
      This->AddItem(This->itemList.indexOf(item));

//
// Remove the old item from the list
//
      This->itemList.remove(item);
      This->keyList.remove(item->Name());
      This->entryBox->RemoveItem(*item);
      This->remList.add(item);

   } // End if name has changed
      
//
// If the name hasn't changed, just modify the value
//
   else {
      This->selectedItem->Field(1, This->entryEditWin->ValueString());
      This->modList.add(This->selectedItem);
   }

} // End FinishModify

/*---------------------------------------------------------------
 *  Callback to handle requested insert
 */

void
DictEditWinC::DoInsert(Widget, DictEditWinC *This, XtPointer)
{
   This->entryEditWin->SetKey("");
   This->entryEditWin->SetValue("");
   set_sensitivity(This->entryEditWin->KeyTF(), True);
   This->entryEditWin->Show((CallbackFn *)FinishInsert, This);
   This->lastAction = INSERT;
}

/*---------------------------------------------------------------
 *  Callback to handle completed insert
 */

void
DictEditWinC::FinishInsert(void*, DictEditWinC *This)
{
   This->AddItem(This->itemList.indexOf(This->selectedItem));
}

/*---------------------------------------------------------------
 *  Callback to handle double-click on entry item
 */

void
DictEditWinC::OpenEntry(VItemC *item, DictEditWinC *This)
{
   This->entryBox->SelectItemOnly(*item, True);
   This->entryEditWin->SetKey(item->Name());
   This->entryEditWin->SetValue(*(item->FieldList()[1]));
   This->entryEditWin->Show((CallbackFn *)FinishModify, This);
   This->lastAction = MODIFY;
}

/*---------------------------------------------------------------
 *  Method to set button sensitivities
 */

void
DictEditWinC::UpdateButtons()
{
//
// Update buttons sensitivities based on number of selected items
//
   unsigned	scount = entryBox->SelItems().size();
   XtSetSensitive(insertPB, scount==1);
   XtSetSensitive(modifyPB, scount==1);
   XtSetSensitive(removePB, scount>0);

//
// Close the edit window if insert or modify is invalid and the operation
//    is not an add.
//
   if ( scount != 1 && lastAction != ADD ) entryEditWin->Hide();
}

/*---------------------------------------------------------------
 *  Callback routine to handle press of a DELETE answer
 */

void
DictEditWinC::AnswerDelete(Widget, int *answer, XmAnyCallbackStruct *cbs)
{
   *answer = cbs->reason;
}

static void
WmClose(Widget, int *answer, XtPointer)
{
   *answer = XmCR_CANCEL;
}

/*---------------------------------------------------------------
 *  Method to confirm deletion of entries
 */

Boolean
DictEditWinC::ConfirmDelete(Widget parent)
{
   static int		answer;
   static Widget	dialog = NULL;
//
// Create the dialog if necessary
//
   if ( !dialog ) {

      halApp->BusyCursor(True);
      WArgList	args;
      args.DialogStyle(XmDIALOG_FULL_APPLICATION_MODAL);
      if ( halApp->questionPM ) args.SymbolPixmap(halApp->questionPM);
      dialog = XmCreateQuestionDialog(*halApp, "confirmDeleteWin", ARGS);

      XtAddCallback(dialog, XmNokCallback, (XtCallbackProc)AnswerDelete,
		    (XtPointer)&answer);
      XtAddCallback(dialog, XmNcancelCallback, (XtCallbackProc)AnswerDelete,
      		    (XtPointer)&answer);
      XtAddCallback(dialog, XmNhelpCallback, (XtCallbackProc)HalAppC::DoHelp,
      		    (char *) "helpcard");

//
// Trap window manager close function
//
      XmAddWMProtocolCallback(XtParent(dialog), halApp->delWinAtom,
			      (XtCallbackProc)WmClose,
			      (XtPointer)&answer);

      halApp->BusyCursor(False);

   } // End if delete confirm dialog not created

//
// Set the dialog's parent so it pops up in the desired place
//
   PopupOver(dialog, parent);

//
// Simulate the main event loop and wait for the answer
//
   answer = XmCR_NONE;
   while ( answer == XmCR_NONE ) {
      XtAppProcessEvent(halApp->context, XtIMXEvent);
      XSync(halApp->display, False);
   }

   XtUnmanageChild(dialog);
   XSync(halApp->display, False);
   XmUpdateDisplay(dialog);

   return (answer == XmCR_OK);

} // End ConfirmDelete
