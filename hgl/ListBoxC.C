/*
 * $Id: ListBoxC.C,v 1.2 2000/05/07 12:26:10 fnevgeny Exp $
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
#include "ListBoxC.h"
#include "WXmString.h"
#include "StringListC.h"
#include "rsrc.h"
#include "HalShellC.h"
#include "TextMisc.h"

#include <Xm/TextF.h>
#include <Xm/List.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/RowColumn.h>
#include <Xm/PushB.h>
#include <Xm/ScrolledW.h>
#include <Xm/MessageB.h>

/*---------------------------------------------------------------
 *  Constructor
 */

ListBoxC::ListBoxC(Widget parent, const char *name, ArgList argv, Cardinal argc)
{
   WArgList	args;

   itemCount   = 0;
   addEnabled  = False;
   selectCount = 0;
   textLen     = 0;
   textUnique  = True;
   removeWin   = NULL;
   clearWin    = NULL;
   deferCallbacks = 0;

//
// Create widget and get resources
//
   mainForm = XmCreateForm(parent, (char *)name, argv, argc);

   sorted = get_boolean("ListBoxC", mainForm, "sorted",          False);
   dupOk  = get_boolean("ListBoxC", mainForm, "allowDuplicates", True);

   items.AllowDuplicates(dupOk);
   items.SetSorted(sorted);

/*---------------------------------------------------------------
 *  Create mainForm hierarchy
 *
 *  mainForm
 *     ScrolledList	itemList  
 *     TextField	itemText  
 *     Frame		buttonFrame  
 *        RowColumn	   buttonBox  
 */

   args.Reset();
   args.TopAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   args.MarginWidth(0);
   args.MarginHeight(0);
   args.ShadowThickness(0);
   Widget	buttonFrame = XmCreateFrame(mainForm, "buttonFrame", ARGS);

   args.Reset();
   args.ChildType(XmFRAME_TITLE_CHILD);
   args.ChildHorizontalAlignment(XmALIGNMENT_CENTER);
   args.Packing(XmPACK_COLUMN);
   args.Orientation(XmHORIZONTAL);
   args.EntryAlignment(XmALIGNMENT_CENTER);
   buttonBox = XmCreateRowColumn(buttonFrame, "buttonBox", ARGS);

   args.Reset();
   args.TopAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_WIDGET, buttonFrame);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   itemText = CreateTextField(mainForm, "itemText", ARGS);

   XtAddCallback(itemText, XmNvalueChangedCallback,
	         (XtCallbackProc)DoTextChanged, (XtPointer)this);
   XtAddCallback(itemText, XmNactivateCallback,
	         (XtCallbackProc)DoTextReturn, (XtPointer)this);

   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_WIDGET, itemText);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   args.SelectionPolicy(XmMULTIPLE_SELECT);
   args.ListSizePolicy(XmCONSTANT);
   itemList = XmCreateScrolledList(mainForm, "itemList", ARGS);
   itemWin = XtParent(itemList);

   XtAddCallback(itemList, XmNmultipleSelectionCallback,
		 (XtCallbackProc)DoSelect, (XtPointer)this);
   XtAddCallback(itemList, XmNdefaultActionCallback,
		 (XtCallbackProc)DoItemOpen, (XtPointer)this);

/*---------------------------------------------------------------
 *  Create buttonBox hierarchy
 *
 *  buttonBox
 *     PushButton	addButton  
 *     PushButton	remButton  
 *     PushButton	repButton  
 *     PushButton	clrButton  
 */

   addButton = XmCreatePushButton(buttonBox, "addButton", 0,0);
   remButton = XmCreatePushButton(buttonBox, "remButton", 0,0);
   repButton = XmCreatePushButton(buttonBox, "repButton", 0,0);
   clrButton = XmCreatePushButton(buttonBox, "clrButton", 0,0);

   XtAddCallback(addButton, XmNactivateCallback,
		 (XtCallbackProc)DoItemAdd, (XtPointer)this);
   XtAddCallback(remButton, XmNactivateCallback,
		 (XtCallbackProc)DoRemove, (XtPointer)this);
   XtAddCallback(repButton, XmNactivateCallback,
		 (XtCallbackProc)DoItemReplace, (XtPointer)this);
   XtAddCallback(clrButton, XmNactivateCallback,
		 (XtCallbackProc)DoClear, (XtPointer)this);

   UpdateButtons();

//
// Manage children
//
   Widget	list[4];

   list[0] = addButton;
   list[1] = remButton;
   list[2] = repButton;
   list[3] = clrButton;
   XtManageChildren(list, 4);	// buttonBox children

   XtManageChild(buttonBox);	// buttonFrame children

   XtManageChild(itemList);	// itemWin children

   list[0] = itemText;
   list[1] = buttonFrame;
   XtManageChildren(list, 2);	// mainForm children

} // End ListBoxC Constructor

/*---------------------------------------------------------------
 *  Callback routine to handle CR in text field
 */

void
ListBoxC::DoTextReturn(Widget, ListBoxC *lb, XtPointer)
{
   lb->AddItem((Boolean)True /*Clear*/);
}

/*---------------------------------------------------------------
 *  Callback routine to handle press of ADD
 */

void
ListBoxC::DoItemAdd(Widget, ListBoxC *lb, XtPointer)
{
   lb->AddItem((Boolean)True /*Clear*/);
}

/*---------------------------------------------------------------
 *  Method to add item in text field to list
 */

void
ListBoxC::AddItem(Boolean clear)
{
//
// If clear is true, this was called because the return key was hit in the
//    field.  Only continue if adds are allowed
//
   if ( !addEnabled ) return;	// Keeps duplicates out

//
// Read text from field and add to list
//
   char		*str = XmTextFieldGetString(itemText);
   StringC	item(str);
   XtFree(str);

//
// Call verify callbacks if necessary
//
   if ( verifyCalls.size() > 0 ) {

      int		oldSize = item.size();
      ListBoxVerifyT	verifyData;
      verifyData.listBox = this;
      verifyData.text    = &item;
      verifyData.ok	 = True;

      CallbackListC	tmpCalls = verifyCalls; // In case a callback removes
						// itself
      unsigned	count = verifyCalls.size();
      for (int i=0; i<count && verifyData.ok; i++) {
	 (*tmpCalls[i])(&verifyData);
      }

      if ( !verifyData.ok ) return;

//
// Write the text back to the field in case it was changed, then make sure
//   it's still ok to add it.
//
      if ( item.size() != oldSize ) {

	 textLen = item.size();
	 XmTextFieldSetString(itemText, item);
	 textUnique = !items.includes(item);
	 UpdateButtons();

	 if ( !addEnabled ) {
	    int	pos = items.indexOf(item) + 1;
	    MakeVisible(pos);
	    return;
	 }

      } // End if text size changed

   } // End if text must be verified

   int *posList, posCount;
   XmListGetSelectedPos(itemList, &posList, &posCount);
   int	pos;
   if ( posCount == 0 ) {

      items.add(item);
      if ( !sorted || items.size() == 1 )
	 pos = 0;
      else
         pos = items.indexOf(item) + 1;
   }

   else {
      pos = posList[posCount-1];
      items.insert(item, pos-1);
   }

   WXmString	wstr((char *)item);
   XmListAddItem(itemList, wstr, pos);
//   if (items.size() == 1)
//     XmListSetKbdItemPos(itemList, 1);
   itemCount++;

//
// Clear the text if necessary
//
   if ( clear ) {
      XmTextFieldSetString(itemText, "");
      textLen    = 0;
      textUnique = True;
   }

   DeferCallbacks(True);

//
// Make the item visible
//
   MakeVisible(pos);

   itemChange = True;
   UpdateButtons();
   DeferCallbacks(False);

} // End ListBoxC AddItem

/*---------------------------------------------------------------
 *  Method to make the specified item visible
 */

void
ListBoxC::MakeVisible(int pos)
{
   int	topPos, botPos, visCount;
   XtVaGetValues(itemList, XmNtopItemPosition, &topPos,
			   XmNvisibleItemCount, &visCount, NULL);
   botPos = topPos + visCount - 1;
   if      ( pos < topPos ) XmListSetPos      (itemList, pos);
   else if ( pos > botPos ) XmListSetBottomPos(itemList, pos);
}

/*---------------------------------------------------------------
 *  Callback routine to handle press of REMOVE
 */

void
ListBoxC::DoRemove(Widget, ListBoxC *lb, XtPointer)
{
//
// Build verification dialog if necessary
//
   if ( !lb->removeWin ) {
      WArgList	args;
      args.AutoUnmanage(True);
      if ( halApp->questionPM ) args.SymbolPixmap(halApp->questionPM);
      lb->removeWin = XmCreateQuestionDialog(*lb, "removeWin", ARGS);
      Widget	ok = XmMessageBoxGetChild(lb->removeWin, XmDIALOG_OK_BUTTON);
      XtAddCallback(ok, XmNactivateCallback, (XtCallbackProc)FinishRemove,
                    (XtPointer)lb);
   }

   // unmanage the help button
   XtUnmanageChild(XmMessageBoxGetChild(lb->removeWin, XmDIALOG_HELP_BUTTON));

   XtManageChild(lb->removeWin);
   XMapRaised(halApp->display, XtWindow(XtParent(lb->removeWin)));

} // End ListBoxC DoRemove

/*---------------------------------------------------------------
 *  Callback routine to handle press of OK in remove confirmation dialog
 */

void
ListBoxC::FinishRemove(Widget, ListBoxC *lb, XtPointer)
{
   lb->RemoveSelected();
}

/*---------------------------------------------------------------
 *  Method to remove all selected items
 */

void
ListBoxC::RemoveSelected()
{
//
// Get selected items and remove them
//
   int *posList, posCount, count;
   XmListGetSelectedPos(itemList, &posList, &count);
   if(count) {

     DeferCallbacks(True);
     selectChange = False;

     //
     // Loop through items
     //
     for (int i=0; i<count; i++) {
       XmListGetSelectedPos(itemList, &posList, &posCount);
       int	index = posList[0];

       //
       // Delete requested item (which are moving around on us)
       //
       XmListDeletePos(itemList, index); 
       items.remove(index-1);
     }

     itemChange = (count>0);
     itemCount = items.size();

     //
     // Get number of selected items
     //
     XtVaGetValues(itemList, XmNselectedItemCount, &selectCount, NULL);

     UpdateButtons();
     DeferCallbacks(False);
   }
} // End ListBoxC RemoveSelected

/*---------------------------------------------------------------
 *  Callback routine to handle press of REPLACE
 */

void
ListBoxC::DoItemReplace(Widget, ListBoxC *lb, XtPointer)
{
//
// Get position of selected item
//
   int		posCount;
   int		*positions;
   XmListGetSelectedPos(lb->itemList, &positions, &posCount); 

//
// Get string to be added
//
   char		*cs = XmTextFieldGetString(lb->itemText);
   StringC	item(cs);
   XtFree(cs);

//
// Call verify callbacks if necessary
//
   if ( lb->verifyCalls.size() > 0 ) {

      int		oldSize = item.size();
      ListBoxVerifyT	verifyData;
      verifyData.listBox = lb;
      verifyData.text    = &item;
      verifyData.ok	 = True;

      CallbackListC	tmpCalls = lb->verifyCalls; // In case a callback
						    // removes itself
      unsigned	count = lb->verifyCalls.size();
      for (int i=0; i<count && verifyData.ok; i++) {
	 (*tmpCalls[i])(&verifyData);
      }

      if ( !verifyData.ok ) return;

//
// Write the text back to the field in case it was changed, then make sure
//   it's still ok to add it.
//
      if ( item.size() != oldSize ) {

	 lb->textLen = item.size();
	 XmTextFieldSetString(lb->itemText, item);
	 lb->textUnique = !lb->items.includes(item);
	 lb->UpdateButtons();

	 if ( !lb->addEnabled ) {
	    int	pos = lb->items.indexOf(item) + 1;
	    lb->MakeVisible(pos);
	    return;
	 }

      } // End if text size changed

   } // End if text must be verified

   lb->DeferCallbacks(True);

//
// Replace the selected item(s)
//
   XmString newString = XmStringCreateLtoR((char*)item, XmSTRING_DEFAULT_CHARSET);
   XmListReplaceItemsPos(lb->itemList, &newString, posCount, positions[0]);
   XmStringFree(newString);

//
// Replace the string
//
   int indx = positions[0] -1;
   lb->items.insert(item, indx);
   lb->items.remove(indx+1);

//
// remains selected
//
   XmListSelectPos(lb->itemList, positions[0], False);

//
// Make sure the item is visible
//
   lb->MakeVisible(positions[0]);

   lb->textUnique = False;
   lb->UpdateButtons();

   lb->selectChange = lb->itemChange = True;
   lb->DeferCallbacks(False);

} // End ListBoxC DoItemReplace

/*---------------------------------------------------------------
 *  Callback routine to handle entry of text
 */

void
ListBoxC::DoTextChanged(Widget, ListBoxC *lb, XtPointer)
{
   char		*str = XmTextFieldGetString(lb->itemText);
   WXmString	text(str);

   lb->textLen = strlen(str);
   lb->textUnique = !XmListItemExists(lb->itemList, text);
   lb->UpdateButtons();

   XtFree(str);

} // End ListBoxC DoTextChanged

/*---------------------------------------------------------------
 *  Callback routine to handle selection of list entries
 */

void
ListBoxC::DoSelect(Widget, ListBoxC *lb, XmListCallbackStruct *list)
{
   lb->DeferCallbacks(True);
   lb->selectCount = list->selected_item_count;

   if(lb->selectCount == 1) {
     char *labelItem;
     XmStringGetLtoR(list->selected_items[0], XmFONTLIST_DEFAULT_TAG, &labelItem);
     XmTextFieldSetString(lb->itemText, labelItem);
     XtFree(labelItem);
   } else {
     XmTextFieldSetString(lb->itemText, "");
   }

   lb->textLen = 0;
   lb->selectChange = True;
   lb->UpdateButtons();
   lb->DeferCallbacks(False);
}

/*---------------------------------------------------------------
 *  Callback routine to handle double-click of list entry
 */

void
ListBoxC::DoItemOpen(Widget, ListBoxC *lb, XmListCallbackStruct *list)
{
//
// Copy text from selection to edit field
//
   WXmString	value(list->item);
   char		*str = value;
   XmTextFieldSetString(lb->itemText, str);
   lb->textLen = strlen(str);
   lb->textUnique = False;
   XtFree(str);

//
// Undo the selection
//
   if ( XmListPosSelected(lb->itemList, list->item_position) ) {
      XmListDeselectPos(lb->itemList, list->item_position);
      lb->selectCount--;
   } else {
      XmListSelectPos(lb->itemList, list->item_position, False);
      lb->selectCount++;
   }

   lb->UpdateButtons();

} // End ListBoxC DoItemOpen

/*---------------------------------------------------------------
 *  Method to add a new item to the list
 */

void
ListBoxC::AddItem(StringC name)
{
//
// Check for duplicate name if necessary
//
   if ( !dupOk && items.includes(name) ) return;

   DeferCallbacks(True);

   items.add(name);
   int pos;
   if ( !sorted || items.size() == 1 )
      pos = 0;          // append at bottom of list.
   else
      pos = items.indexOf(name) + 1;
   WXmString	text((char*)name);
   XmListAddItem(itemList, text, pos);
   itemCount++;

//
// Disable the add button if this item matches text in the entry field
//
   char		*str = XmTextFieldGetString(itemText);
   textUnique = (name != str);
   XtFree(str);

   itemChange = True;
   UpdateButtons();
   DeferCallbacks(False);

} // End ListBoxC AddItem

/*---------------------------------------------------------------
 *  Method to add new items to the list
 */

void
ListBoxC::AddItems(const StringListC& list)
{
   DeferCallbacks(True);
   unsigned	count = list.size();
   for (int i=0; i<count; i++) AddItem(*list[i]);
   itemChange = True;
   DeferCallbacks(False);
}

/*---------------------------------------------------------------
 *  Method to set the items in the list
 */

void
ListBoxC::SetItems(const StringListC& names)
{
   DeferCallbacks(True);
   int	scount = selectCount;
   int	icount = itemCount;
   Clear();

   int	count = names.size();
   for (int i=0; i<count; i++) AddItem(*names[i]);
   itemChange   = (count>0) && (icount>0);
   selectChange = (scount>0);
   DeferCallbacks(False);
}

/*---------------------------------------------------------------
 *  Method to append the items in the list box to the given list
 */

StringListC&
ListBoxC::GetItems(StringListC& list)
{
   list.SetSorted(sorted);
   for (int i=0; i<itemCount; i++) list.add(*items[i]);
   return(list);
}

/*---------------------------------------------------------------
 *  Method to append the selected items in the list box to the given list
 */

StringListC&
ListBoxC::GetSelectedItems(StringListC& list)
{
   list.SetSorted(sorted);

//
// Get the list of selected items
//
   XmStringTable	slist;
   int			scount;
   XtVaGetValues(itemList, XmNselectedItemCount, &scount,
			   XmNselectedItems,     &slist, NULL);

   StringC	item;
   for (int i=0; i<scount; i++) {
      char	*cs;
      XmStringGetLtoR(slist[i], XmFONTLIST_DEFAULT_TAG, &cs);
      item = cs;
      list.add(item);
      XtFree(cs);
   }

   return(list);

} // End ListBoxC GetSelectedItems

/*---------------------------------------------------------------
 *  Method to remove an item from the list
 */

void
ListBoxC::RemoveItem(StringC name)
{
//
// Get position of item to be deleted
//
   int	index = items.indexOf(name);
   if ( index == items.NULL_INDEX ) return;

   DeferCallbacks(True);
   selectChange = XmListPosSelected(itemList, index+1);
   itemChange = True;

//
// Delete requested item
//
   XmListDeletePos(itemList, index+1); 
   items.remove(index);
   itemCount--;

//
// Get number of selected items
//
   XtVaGetValues(itemList, XmNselectedItemCount, &selectCount, NULL);

   UpdateButtons();
   DeferCallbacks(False);

} // End ListBoxC RemoveItem

/*---------------------------------------------------------------
 *  Method to remove items from the list
 */

void
ListBoxC::RemoveItems(const StringListC& list)
{
   DeferCallbacks(True);
   selectChange = False;

//
// Loop through items
//
   unsigned	count = list.size();
   for (int i=0; i<count; i++) {

      StringC	*name = list[i];
      int	index = items.indexOf(*name);
      if ( index != items.NULL_INDEX ) {

	 selectChange = selectChange || XmListPosSelected(itemList, index+1);

//
// Delete requested item
//
	 XmListDeletePos(itemList, index+1); 
	 items.remove(index);
      }
   }

   itemChange = (count>0);
   itemCount = items.size();

//
// Get number of selected items
//
   XtVaGetValues(itemList, XmNselectedItemCount, &selectCount, NULL);

   UpdateButtons();
   DeferCallbacks(False);

} // End ListBoxC RemoveItem

/*---------------------------------------------------------------
 *  Callback routine to handle press of CLEAR
 */

void
ListBoxC::DoClear(Widget, ListBoxC *lb, XtPointer)
{
//
// Build verification dialog if necessary
//
   if ( !lb->clearWin ) {
      WArgList	args;
      args.AutoUnmanage(True);
      if ( halApp->questionPM ) args.SymbolPixmap(halApp->questionPM);
      lb->clearWin = XmCreateQuestionDialog(*lb, "clearWin", ARGS);
      Widget	ok = XmMessageBoxGetChild(lb->clearWin, XmDIALOG_OK_BUTTON);
      XtAddCallback(ok, XmNactivateCallback, (XtCallbackProc)FinishClear,
                    (XtPointer)lb);
   }

   // unmanage the help button
   XtUnmanageChild(XmMessageBoxGetChild(lb->clearWin, XmDIALOG_HELP_BUTTON));

   XtManageChild(lb->clearWin);
   XMapRaised(halApp->display, XtWindow(XtParent(lb->clearWin)));

} // End ListBoxC DoClear

/*---------------------------------------------------------------
 *  Callback routine to handle press of OK in clear confirmation dialog
 */

void
ListBoxC::FinishClear(Widget, ListBoxC *lb, XtPointer)
{
   lb->Clear();
}

/*---------------------------------------------------------------
 *  Method to remove all items from the list
 */

void
ListBoxC::Clear()
{
   DeferCallbacks(True);
   selectChange = (selectCount > 0);
   itemChange   = (itemCount   > 0);

   XmListDeleteAllItems(itemList);
   items.removeAll();
   itemCount   = 0;
   selectCount = 0;
   textUnique = True;

   UpdateButtons();
   DeferCallbacks(False);
}

/*---------------------------------------------------------------
 *  Method to set sensitivities of buttons
 */

void
ListBoxC::UpdateButtons()
{
   addEnabled = textLen>0 && (dupOk || textUnique);
   XtSetSensitive(addButton, addEnabled);
   XtSetSensitive(remButton, selectCount>0);
   XtSetSensitive(repButton, selectCount==1 && addEnabled);
   XtSetSensitive(clrButton, itemCount>0);
}

/*---------------------------------------------------------------
 *  Method to enable quick-help for components
 */

void
ListBoxC::HandleHelp(HalShellC *win, Help*)
{
   win->HandleHelp(itemText);
   win->HandleHelp(itemList);
   win->HandleHelp(addButton);
   win->HandleHelp(remButton);
   win->HandleHelp(repButton);
   win->HandleHelp(clrButton);
}

/*----------------------------------------------------------------------
 *  Set a flag so that callbacks don't get called until all changes are
 *      complete.
 */

void
ListBoxC::DeferCallbacks(Boolean on)
{
   if ( on ) {

      deferCallbacks++;		// Up the count

   } else if ( deferCallbacks > 0 ) {

      deferCallbacks--;		// Lower the count
      if ( deferCallbacks == 0 ) {
	 if ( selectChange ) CallSelectChangeCallbacks();
	 if ( itemChange   ) CallItemChangeCallbacks();
	 selectChange =
	 itemChange   = False;
      }
   }

} // End VBoxC DeferCallbacks
