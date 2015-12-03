/*
 *  $Id: AutoFileWinC.C,v 1.2 2000/05/07 12:26:11 fnevgeny Exp $
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
#include "AutoFileWinC.h"
#include "AutoFilePrefC.h"
#include "IshAppC.h"

#include <hgl/VBoxC.h>
#include <hgl/WArgList.h>
#include <hgl/TBoxC.h>

#include <Xm/TextF.h>
#include <Xm/ToggleB.h>

/*---------------------------------------------------------------
 *  Main window constructor
 */

AutoFileWinC::AutoFileWinC(Widget parent) : DictEditWinC(parent, "autoFileWin")
{
//
// Add a toggle button for turning auto-filing on and off
//
   WArgList	args;
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_NONE);
   args.RightAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_NONE);
   autoFileTB = XmCreateToggleButton(appForm, "autoFileTB", ARGS);
   XtManageChild(autoFileTB);

   HandleHelp(autoFileTB);

   args.Reset();
   args.TopAttachment(XmATTACH_WIDGET, autoFileTB);
   XtSetValues(*taskBox, ARGS);

   entryBox->SetCompareFunction((CompareFn)Compare);
   XtAddCallback(*this, XmNpopupCallback, (XtCallbackProc)DoPopup,
		 (XtPointer)this);
}

/*---------------------------------------------------------------
 *  Callback routine to handle initial display.
 */

void
AutoFileWinC::DoPopup(Widget, AutoFileWinC *This, XtPointer)
{
   XtRemoveCallback(*This, XmNpopupCallback, (XtCallbackProc)DoPopup,
		    (XtPointer)This);

   AutoFilePrefC	*prefs = ishApp->autoFilePrefs;
   XmToggleButtonSetState(This->autoFileTB, prefs->autoFileOn, True);

//
// Get list of auto-file rules
//
   StringListC	fieldList;
   fieldList.AllowDuplicates(TRUE);

   unsigned	count = prefs->autoFileRules.size();
   for (int i=0; i<count; i++) {

      RegexC&	key = prefs->autoFileRules.keyOf(i);
      StringC&	val = prefs->autoFileRules.valOf(i);

      fieldList.removeAll();
      fieldList.add(key);
      fieldList.add(val);
      fieldList.add(key);	// backup
      fieldList.add(val);	// backup

      VItemC	*item = new VItemC(key);
      item->SetUserData(This);
      item->FieldList(fieldList);
      item->AddOpenCallback((CallbackFn *)OpenEntry, This);
      This->itemList.add(item);
      This->keyList.add(item->Name());

   } // End for each rule

   This->entryBox->AddItems(This->itemList);

} // End AutoFileWinC DoPopup

/*---------------------------------------------------------------
 *  Method to apply settings
  */

Boolean
AutoFileWinC::Apply()
{
   AutoFilePrefC	*prefs = ishApp->autoFilePrefs;

   BusyCursor(True);

   prefs->autoFileOn = XmToggleButtonGetState(autoFileTB);

//
// Delete any rules that weren't also added.
//
   unsigned	count = remList.size();
   int	i;
   for (i=0; i<count; i++) {
      VItemC	*item = remList[i];
      if ( !addList.includes(item) ) {
	 RegexC	tmp = item->Name();
	 prefs->autoFileRules.remove(tmp);
      }
   }

//
// Add any rules that weren't later deleted.
//
   count = addList.size();
   for (i=0; i<count; i++) {
      VItemC	*item = addList[i];
      if ( !remList.includes(item) ) {
	 RegexC	tmp = item->Name();
	 prefs->autoFileRules.add(tmp, *(item->FieldList()[1]));
      }
   }

//
// Modify any rules that weren't added or deleted.
//
   count = modList.size();
   for (i=0; i<count; i++) {
      VItemC	*item = modList[i];
      if ( !addList.includes(item) && !remList.includes(item) ) {
	 RegexC	tmp = item->Name();
	 prefs->autoFileRules.modify(tmp, *(item->FieldList()[1]));
      }
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
      VItemC		*item = itemList[i];
      StringListC&	flist = item->FieldList();
      *flist[2] = *flist[0];
      *flist[3] = *flist[1];
   }

//
// Make sure the dictionary values are in the correct order
//
   count = prefs->autoFileRules.size();
   for (i=0; i<count; i++) {

      RegexC&   dictName = prefs->autoFileRules.keyOf(i);
      StringC&  itemName = itemList[i]->Name();

//
// If they're not the same, move the dictionary entry with the given itemName
//    to the current position
//
      if ( dictName != itemName ) {
	 RuleDictCEntry       entry = *prefs->autoFileRules.entryOf(itemName);
	 prefs->autoFileRules.remove(entry);
	 prefs->autoFileRules.insert(entry, i);
      }

   } // End for each entry

   prefs->WriteDatabase();

//
// Write to file if necessary
//
   if ( applyAll ) prefs->WriteFile();

   BusyCursor(False);

   return True;

} // End AutoFileWinC Apply

/*---------------------------------------------------------------
 *  Method to handle cancel
 */

void
AutoFileWinC::Cancel()
{
//
// Restore any modified items
//
   unsigned	count = modList.size();
   int	i;
   for (i=0; i<count; i++) {
      VItemC	*item = modList[i];
      StringC	*oldKey = item->FieldList()[2];
      StringC	*oldVal = item->FieldList()[3];
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

} // End AutoFileWinC Cancel

/*-----------------------------------------------------------------------
 *  Compare function for preference list order
 */

int
AutoFileWinC::Compare(const void *a, const void *b)
{
   VItemC	*via = *(VItemC **)a;
   VItemC	*vib = *(VItemC **)b;

   AutoFileWinC	*This = (AutoFileWinC *)via->UserData();
   int	ia = This->itemList.indexOf(via);
   int	ib = This->itemList.indexOf(vib);

   if ( ia < ib ) return -1;
   if ( ia > ib ) return  1;
   return 0;

} // End AutoFileWinC Compare
