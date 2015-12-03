/*
 *  $Id: AlertPrefWinC.C,v 1.2 2000/05/07 12:26:11 fnevgeny Exp $
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
#include "AlertPrefWinC.h"
#include "AlertPrefC.h"
#include "IshAppC.h"

#include <hgl/VBoxC.h>
#include <hgl/WArgList.h>
#include <hgl/TBoxC.h>

#include <Xm/TextF.h>
#include <Xm/ToggleB.h>

/*---------------------------------------------------------------
 *  Main window constructor
 */

							     // historical name
AlertPrefWinC::AlertPrefWinC(Widget parent) : DictEditWinC(parent, "alertWin")
{
//
// Add a toggle button for turning alerting on and off
//
   WArgList	args;
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_NONE);
   args.RightAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_NONE);
   alertTB = XmCreateToggleButton(appForm, "alertTB", ARGS);
   XtManageChild(alertTB);

   HandleHelp(alertTB);

   args.Reset();
   args.TopAttachment(XmATTACH_WIDGET, alertTB);
   XtSetValues(*taskBox, ARGS);

   entryBox->SetCompareFunction((CompareFn)Compare);

//
// Get list of alerts
//
   StringListC	fieldList;
   fieldList.AllowDuplicates(TRUE);

   AlertPrefC	*prefs = ishApp->alertPrefs;
   unsigned	count = prefs->alertRules.size();
   for (int i=0; i<count; i++) {

      RegexC&	key = prefs->alertRules.keyOf(i);
      StringC&	val = prefs->alertRules.valOf(i);

      fieldList.removeAll();
      fieldList.add(key);
      fieldList.add(val);
      fieldList.add(key);
      fieldList.add(val);

      VItemC	*item = new VItemC(key);
      item->SetUserData(this);
      item->FieldList(fieldList);
      item->AddOpenCallback((CallbackFn *)OpenEntry, this);
      itemList.add(item);
      keyList.add(item->Name());

   } // End for each alert

   entryBox->AddItems(itemList);

} // End constructor

/*---------------------------------------------------------------
 *  Callback routine to handle initial display.
 */

void
AlertPrefWinC::Show()
{
   XmToggleButtonSetState(alertTB, ishApp->alertPrefs->alertOn, True);
   DictEditWinC::Show();
}

/*---------------------------------------------------------------
 *  Method to apply settings
  */

Boolean
AlertPrefWinC::Apply()
{
   AlertPrefC	*prefs = ishApp->alertPrefs;

   BusyCursor(True);

   prefs->alertOn = XmToggleButtonGetState(alertTB);

//
// Delete any alerts that weren't also added.
//
   unsigned	count = remList.size();
   int	i;
   for (i=0; i<count; i++) {
      VItemC	*item = remList[i];
      if ( !addList.includes(item) ) {
	 RegexC	tmp = item->Name();
	 prefs->alertRules.remove(tmp);
      }
   }

//
// Add any alerts that weren't later deleted.
//
   count = addList.size();
   for (i=0; i<count; i++) {
      VItemC	*item = addList[i];
      if ( !remList.includes(item) ) {
	 RegexC	tmp = item->Name();
	 prefs->alertRules.add(tmp, *(item->FieldList()[1]));
      }
   }

//
// Modify any alerts that weren't added or deleted.
//
   count = modList.size();
   for (i=0; i<count; i++) {
      VItemC	*item = modList[i];
      if ( !addList.includes(item) && !remList.includes(item) ) {
	 RegexC	tmp = item->Name();
	 prefs->alertRules.modify(tmp, *(item->FieldList()[1]));
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
   count = prefs->alertRules.size();
   for (i=0; i<count; i++) {

      RegexC&	dictName = prefs->alertRules.keyOf(i);
      StringC&	itemName = itemList[i]->Name();

//
// If they're not the same, move the dictionary entry with the given itemName
//    to the current position
//
      if ( dictName != itemName ) {
	 RuleDictCEntry	entry = *prefs->alertRules.entryOf(itemName);
	 prefs->alertRules.remove(entry);
	 prefs->alertRules.insert(entry, i);
      }

   } // End for each entry

   prefs->WriteDatabase();

//
// Write to file if necessary
//
   if ( applyAll ) prefs->WriteFile();

   BusyCursor(False);

   return True;

} // End Apply

/*---------------------------------------------------------------
 *  Method to handle cancel
 */

void
AlertPrefWinC::Cancel()
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
      int	index = keyList.indexOf(item->Name());
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

/*-----------------------------------------------------------------------
 *  Compare function for preference list order
 */

int
AlertPrefWinC::Compare(void *a, void *b)
{
   //cout <<"AlertPrefWinC::Compare" <<endl;

   VItemC	*via = *(VItemC **)a;
   VItemC	*vib = *(VItemC **)b;

   AlertPrefWinC	*This = (AlertPrefWinC *)via->UserData();
   int	ia = This->itemList.indexOf(via);
   int	ib = This->itemList.indexOf(vib);

   if ( ia < ib ) return -1;
   if ( ia > ib ) return  1;
   return 0;

} // End Compare
