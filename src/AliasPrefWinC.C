/*
 *  $Id: AliasPrefWinC.C,v 1.3 2000/08/07 11:05:16 evgeny Exp $
 *  
 *  Copyright (c) 1994 HAL Computer Systems International, Ltd.
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
#include "AliasPrefWinC.h"
#include "AliasPrefC.h"
#include "IshAppC.h"
#include "EntryEditWinC.h"

#include <hgl/VBoxC.h>
#include <hgl/TBoxC.h>
#include <hgl/RegexC.h>
#include <hgl/VItemC.h>
#include <hgl/HalAppC.h>
#include <hgl/WXmString.h>
#include <hgl/WArgList.h>
#include <hgl/rsrc.h>
#include <hgl/RowColC.h>

#include <Xm/PushB.h>
#include <Xm/MessageB.h>

#include <unistd.h>

/*---------------------------------------------------------------
 *  Main window constructor
 */

AliasPrefWinC::AliasPrefWinC(Widget parent, StringDictC& dict)
		       // historical name
: DictEditWinC(parent, "aliasWin"), aliasDict(dict)
{
//
// Turn on code to automatically add commas between aliases
//
   entryEditWin->EnableAliases();

//
// Add a toggle button for turning sorting on and off
//
   WArgList	args;
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_NONE);
   args.RightAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_NONE);
   sortTB = XmCreateToggleButton(appForm, "sortTB", ARGS);
   XtManageChild(sortTB);

   XtAddCallback(sortTB, XmNvalueChangedCallback, (XtCallbackProc)ToggleSort,
   		 this);

   HandleHelp(sortTB);

//
// Assign the function to compare two entries
//
   entryBox->SetCompareFunction((CompareFn)AliasCompare);

   args.Reset();
   args.TopAttachment(XmATTACH_WIDGET, sortTB);
   XtSetValues(*taskBox, ARGS);

   XtAddCallback(*this, XmNpopupCallback, (XtCallbackProc)DoPopup,
		 (XtPointer)this);

} // End constructor

/*---------------------------------------------------------------
 *  Callback routine to handle initial display.
 */

void
AliasPrefWinC::DoPopup(Widget, AliasPrefWinC *This, XtPointer)
{
   XtRemoveCallback(*This, XmNpopupCallback, (XtCallbackProc)DoPopup,
		    (XtPointer)This);

   This->itemList.SetSorted(FALSE);
   This->keyList.SetSorted(FALSE);

//
// Get list of aliases
//
   StringListC	fieldList;
   fieldList.AllowDuplicates(TRUE);

   unsigned	count = This->aliasDict.size();
   for (int i=0; i<count; i++) {

      StringC&	key = This->aliasDict.keyOf(i);
      StringC&	val = This->aliasDict.valOf(i);

      fieldList.removeAll();
      fieldList.add(key);
      fieldList.add(val);
      fieldList.add(key);
      fieldList.add(val);

      VItemC	*item = new VItemC(key);
      item->FieldList(fieldList);
      item->AddOpenCallback((CallbackFn *)OpenEntry, This);
      This->itemList.add(item);
      This->keyList.add(item->Name());

   } // End for each alias

   This->itemList.SetSorted(ishApp->aliasPrefs->sortAliases);
   This->keyList.SetSorted(ishApp->aliasPrefs->sortAliases);

   This->entryBox->AddItems(This->itemList);

} // End DoPopup

/*---------------------------------------------------------------
 *  Method to toggle sorting
 */

void
AliasPrefWinC::ToggleSort(Widget, AliasPrefWinC *This,
				  XmToggleButtonCallbackStruct *tb)
{
   This->BusyCursor(True);

   This->entryBox->RemoveAllItems();

//
// If sorting is being turned off, and there are not changes, rebuild the
//    item list.
//
   if ( !tb->set && This->addList.size() == 0 &&
        This->remList.size() == 0 && This->modList.size() == 0 ) {

//
// Rebuild the item list
//
      This->itemList.SetSorted(FALSE);
      This->keyList.SetSorted(FALSE);

//
// Get list of aliases
//
      StringListC	fieldList;
      fieldList.AllowDuplicates(TRUE);

      unsigned	count = This->aliasDict.size();
      for (int i=0; i<count; i++) {

	 StringC&	key = This->aliasDict.keyOf(i);
	 StringC&	val = This->aliasDict.valOf(i);

	 fieldList.removeAll();
	 fieldList.add(key);
	 fieldList.add(val);
	 fieldList.add(key);
	 fieldList.add(val);

	 VItemC	*item = This->itemList[i];
	 item->Name(key);
	 item->Label(key);
	 item->FieldList(fieldList);

	 StringC	*ikey = This->keyList[i];
	 *ikey = item->Name();

      } // End for each alias

   } // End list needs to be and can be rebuilt

   This->itemList.SetSorted(tb->set);
   This->keyList.SetSorted(tb->set);
   This->entryBox->SetSorted(tb->set);

   This->entryBox->AddItems(This->itemList);
   This->entryBox->Refresh();

   XtSetSensitive(This->insertPB, !tb->set);

   This->BusyCursor(False);

} // End ToggleSort

/*---------------------------------------------------------------
 *  Method to display dialog
 */

void
AliasPrefWinC::Show()
{
   XmToggleButtonSetState(sortTB, ishApp->aliasPrefs->sortAliases, False);

//
// Inserting is not meaningful if sorting is on
//
   XtSetSensitive(insertPB, !ishApp->aliasPrefs->sortAliases);

   DictEditWinC::Show();
}

/*---------------------------------------------------------------
 *  Method to apply settings
 */

Boolean
AliasPrefWinC::Apply()
{
   BusyCursor(True);

//
// Delete any aliases that weren't also added.
//
   unsigned	count = remList.size();
   int	i;
   for (i=0; i<count; i++) {
      VItemC	*item = remList[i];
      if ( !addList.includes(item) )
	 aliasDict.remove(item->Name());
   }

//
// Add any aliases that weren't later deleted.
//
   count = addList.size();
   for (i=0; i<count; i++) {
      VItemC	*item = addList[i];
      if ( !remList.includes(item) )
	 aliasDict.add(item->Name(), *(item->FieldList()[1]));
   }

//
// Modify any aliases that weren't added or deleted.
//
   count = modList.size();
   for (i=0; i<count; i++) {
      VItemC	*item = modList[i];
      if ( !addList.includes(item) && !remList.includes(item) )
	 aliasDict.modify(item->Name(), *(item->FieldList()[1]));
   }

//
// Delete any removed items
//
   count = remList.size();
   for (i=0; i<count; i++) delete remList[i];

   addList.removeAll();
   modList.removeAll();
   remList.removeAll();

//
// Update backup values for remaining items
//
   count = itemList.size();
   for (i=0; i<count; i++) {
      VItemC            *item = itemList[i];
      StringListC&      flist = item->FieldList();
      *flist[2] = *flist[0];
      *flist[3] = *flist[1];
   }

//
// Make sure the dictionary values are in the correct order
//
   count = aliasDict.size();
   for (i=0; i<count; i++) {

      StringC&	dictName = aliasDict.keyOf(i);
      StringC&	itemName = itemList[i]->Name();

//
// If they're not the same, move the dictionary entry with the given itemName
//    to the current position
//
      if ( dictName != itemName ) {
	 StringDictCEntry	entry = *aliasDict.entryOf(itemName);
	 aliasDict.remove(entry);
	 aliasDict.insert(entry, i);
      }

   } // End for each entry

   ishApp->aliasPrefs->WriteDatabase();

//
// Write to file if necessary
//
   if ( applyAll ) ishApp->aliasPrefs->WriteFile();

   BusyCursor(False);

   return True;

} // End Apply

/*---------------------------------------------------------------
*  Method to handle cancel
*/

void
AliasPrefWinC::Cancel()
{
//
// Restore any modified items
//
   unsigned	count = modList.size();
   int	i;
   for (i=0; i<count; i++) {
      VItemC	*item = modList[i];
      StringC   *oldKey = item->FieldList()[2];
      StringC   *oldVal = item->FieldList()[3];
      int       index = keyList.indexOf(item->Name());
      *keyList[index] = *oldKey;
      item->Name(*oldKey);
      item->Field(0, *oldKey);
      item->Field(1, *oldVal);
   }

//
// Delete any added items
//
   count = addList.size();
   if ( count > 0 ) entryBox->RemoveItems(addList);
   for (i=0; i<count; i++) {
      VItemC	*item = addList[i];
      keyList.remove(item->Name());
      itemList.remove(item);
      delete item;
   }

//
// Re-add any removed items that weren't added during this session
//
   count = remList.size();
   for (i=0; i<count; i++) {
      VItemC	*item = remList[i];
      if ( !addList.includes(item) ) entryBox->AddItem(*item);
      keyList.add(item->Name());
      itemList.add(item);
   }

   addList.removeAll();
   modList.removeAll();
   remList.removeAll();

   Hide();

} // End Cancel

/*------------------------------------------------------------------------
 * Method to compare two aliases
 */

int
AliasPrefWinC::AliasCompare(const void *a, const void *b)
{
   VItemC	*via = *(VItemC **)a;
   VItemC	*vib = *(VItemC **)b;
   return via->Name().compare(vib->Name());

} // End AliasCompare
