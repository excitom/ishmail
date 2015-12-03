/*
 *  $Id: PickAliasWinC.C,v 1.4 2000/08/07 11:05:16 evgeny Exp $
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

#include "PickAliasWinC.h"
#include "IshAppC.h"
#include "AliasPrefC.h"
#include "AppPrefC.h"

#include <hgl/WArgList.h>
#include <hgl/VBoxC.h>
#include <hgl/FieldViewC.h>
#include <hgl/VItemC.h>
#include <hgl/rsrc.h>
#include <hgl/WXmString.h>
#include <hgl/HalAppC.h>
#include <hgl/TextMisc.h>
#include <hgl/CharC.h>

#include <Xm/Frame.h>
#include <Xm/RowColumn.h>
#include <Xm/PushB.h>
#include <Xm/TextF.h>
#include <Xm/Label.h>
#include <Xm/Form.h>
#include <Xm/ToggleB.h>

/*---------------------------------------------------------------
 *  Main window constructor
 */

PickAliasWinC::PickAliasWinC(Widget parent) : HalDialogC("pickAliasWin", parent)
{
   WArgList	args;

//
// Create appForm hierarchy
//
// appForm
//    Label		titleLabel
//    VBoxC		msgBox
//    Form		findForm
//       Label	   	findLabel
//       TextField   	findTF
//       PushButton	findNextPB
//       PushButton	findPrevPB
//       PushButton	findAllPB
//    ToggleButton	expandTB
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_NONE);
   titleLabel = XmCreateLabel(appForm, "titleLabel", ARGS);

   args.Reset();
   args.TopAttachment(XmATTACH_NONE);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_FORM);
   expandTB = XmCreateToggleButton(appForm, "expandTB", ARGS);

   args.Reset();
   args.TopAttachment(XmATTACH_NONE);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_WIDGET, expandTB);
   Widget	findForm = XmCreateForm(appForm, "findForm", ARGS);

   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_FORM);
   Widget	findLabel = XmCreateLabel(findForm, "findLabel", ARGS);

   args.LeftAttachment(XmATTACH_NONE);
   args.RightAttachment(XmATTACH_FORM);
   Widget findAllPB  = XmCreatePushButton(findForm, "findAllPB",  ARGS);

   args.RightAttachment(XmATTACH_WIDGET, findAllPB);
   Widget findPrevPB = XmCreatePushButton(findForm, "findPrevPB", ARGS);

   args.RightAttachment(XmATTACH_WIDGET, findPrevPB);
   Widget findNextPB = XmCreatePushButton(findForm, "findNextPB", ARGS);

   args.LeftAttachment(XmATTACH_WIDGET, findLabel);
   args.RightAttachment(XmATTACH_WIDGET, findNextPB);
   findTF = CreateTextField(findForm, "findTF", ARGS);

   Widget	wlist[5];
   Cardinal	wcount = 0;
   wlist[wcount++] = findLabel;
   wlist[wcount++] = findTF;
   wlist[wcount++] = findNextPB;
   wlist[wcount++] = findPrevPB;
   wlist[wcount++] = findAllPB;
   XtManageChildren(wlist, wcount);
   XtManageChild(findForm);

   args.Reset();
   args.TopAttachment(XmATTACH_WIDGET, titleLabel);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_WIDGET, findForm);
   aliasBox = new VBoxC(appForm, "aliasBox", ARGS);
   aliasBox->DisablePopupMenu();
   aliasBox->HideStatus();
   aliasBox->AddSelectChangeCallback((CallbackFn *)ChangeSelection, this);
   aliasBox->SetCompareFunction((CompareFn)AliasCompare);

   XtManageChild(titleLabel);
   XtManageChild(expandTB);
   XtManageChild(*aliasBox);

//
// Add field view to aliasBox
//
   fieldView = new FieldViewC(aliasBox);
   viewType = aliasBox->AddView(*fieldView);

//
// Set field titles
//
   StringListC	titleList;
   titleList.AllowDuplicates(TRUE);
   StringC	tmpStr = get_string(*aliasBox, "keyColumnTitle", "Key");
   titleList.append(tmpStr);
   tmpStr = get_string(*aliasBox, "valueColumnTitle", "Value");
   titleList.append(tmpStr);
   fieldView->SetTitles(titleList);
   fieldView->HidePixmaps();

//
// Create buttonRC hierarchy
//
//   buttonRC
//	PushButton	okPB
//	PushButton	applyPB
//	PushButton	donePB
//	PushButton	helpPB
//
   AddButtonBox();
   okPB	         = XmCreatePushButton(buttonRC, "okPB",    0,0);
   applyPB       = XmCreatePushButton(buttonRC, "applyPB", 0,0);
   Widget donePB = XmCreatePushButton(buttonRC, "donePB",  0,0);
   Widget helpPB = XmCreatePushButton(buttonRC, "helpPB",  0,0);

   XtManageChild(okPB);
   XtManageChild(applyPB);
   XtManageChild(donePB);
   XtManageChild(helpPB);

   XtAddCallback(okPB, XmNactivateCallback, (XtCallbackProc)DoOk,
          	 (XtPointer)this);
   XtAddCallback(applyPB, XmNactivateCallback, (XtCallbackProc)DoApply,
          	 (XtPointer)this);
   XtAddCallback(donePB, XmNactivateCallback, (XtCallbackProc)DoHide,
 		 (XtPointer)this);
   XtAddCallback(helpPB, XmNactivateCallback, (XtCallbackProc)HalAppC::DoHelp,
		 (char *) "helpcard");

   XtVaSetValues(appForm, XmNdefaultButton, okPB, NULL);

   ShowInfoMsg();
   HandleHelp();
   HandleHelp(*aliasBox);

   XtAddCallback(findTF,     XmNactivateCallback, (XtCallbackProc)DoFindNext,
		 this);
   XtAddCallback(findNextPB, XmNactivateCallback, (XtCallbackProc)DoFindNext,
		 this);
   XtAddCallback(findPrevPB, XmNactivateCallback, (XtCallbackProc)DoFindPrev,
		 this);
   XtAddCallback(findAllPB,  XmNactivateCallback, (XtCallbackProc)DoFindAll,
		 this);
   XtAddCallback(shell,   XmNpopupCallback,    (XtCallbackProc)DoPopup, this);

} // End constructor

/*---------------------------------------------------------------
 *  Destructor
 */

PickAliasWinC::~PickAliasWinC()
{
   delete fieldView;
   delete aliasBox;
}

/*---------------------------------------------------------------
 *  Method to set the callback to be called during the apply function
 */

void
PickAliasWinC::SetApplyCallback(CallbackFn *fn, void *data)
{
   applyCall.Set(fn, data);
}

/*---------------------------------------------------------------
 *  Method to display dialog
 */

void
PickAliasWinC::Show(const char *type)
{
   BusyCursor(True);

//
// Substitute the type string in the title label
//
   StringC titleStr = get_string(titleLabel, "labelString", "$FIELD field");
   int	   pos;
   while ( (pos=titleStr.PosOf("$FIELD")) >= 0 ) titleStr(pos,6) = type;

   WXmString	wstr((char*)titleStr);
   XtVaSetValues(titleLabel, XmNlabelString, (XmString)wstr, NULL);

//
// Build list of aliases
//
   aliasBox->RemoveAllItems();

   int	offset = 0;
   ProcessDict(ishApp->aliasPrefs->aliasDict, offset);
   offset += ishApp->aliasPrefs->aliasDict.size();

   ProcessDict(ishApp->aliasPrefs->groupAliasDict, offset);
   offset += ishApp->aliasPrefs->groupAliasDict.size();

   ProcessDict(ishApp->aliasPrefs->otherAliasDict, offset);
   offset += ishApp->aliasPrefs->otherAliasDict.size();

//
// Remove extra entries
//
   u_int	count = itemList.size();
   for (int i=offset; i<count; i++) {
      VItemC	*item = itemList[i];
      itemList.replace(NULL, i);
      delete item;
   }
   itemList.removeNulls();

   aliasBox->AddItems(itemList);
   aliasBox->Refresh();

   findIndex = 0;

   HalDialogC::Show();

   BusyCursor(False);

} // End Show

/*---------------------------------------------------------------
 *  Method to display aliases from a dictionary
 */

void
PickAliasWinC::ProcessDict(StringDictC& dict, int itemOffset)
{
   StringListC	fieldList;

   u_int	count = dict.size();
   for (int i=0; i<count; i++) {

      StringC&	key = dict.keyOf(i);
      StringC&	val = dict.valOf(i);

      fieldList.removeAll();
      fieldList.add(key);
      fieldList.add(val);

      VItemC	*item;
      if ( i+itemOffset < itemList.size() ) {
	 item = itemList[i+itemOffset];
	 item->Name(key);
	 item->Label(key);
      }
      else {
	 item = new VItemC(key);
	 item->AddOpenCallback((CallbackFn *)PickAlias, this);
	 itemList.add(item);
      }

      item->FieldList(fieldList);

   } // End for each alias

} // End ProcessDict

/*---------------------------------------------------------------
 *  Callback routine to handle find next
 */

void
PickAliasWinC::DoFindNext(Widget, PickAliasWinC *This, XtPointer)
{
   This->ClearMessage();

   char	*cs = XmTextFieldGetString(This->findTF);
   StringC	pat(cs);
   pat.Trim();
   if ( pat.size() == 0 ) {
      set_invalid(This->findTF, True, True);
      This->PopupMessage("Please enter a search string");
      return;
   }
   
   Boolean	found = False;
   u_int	count = This->aliasBox->VisItems().size();
   if ( count > 0 ) {

      if ( This->findIndex > count-1 ) This->findIndex = 0;

//
// Look from the start index to the end of the list
//
      int	i;
      for (i=This->findIndex; !found && i<count; i++) {
	 VItemC		*item = This->aliasBox->VisItems()[i];
	 StringListC&	fieldList = item->FieldList();
	 StringC		*field1 = fieldList[0];
	 StringC		*field2 = fieldList[1];
	 if ( field1->Contains(pat, IGNORE_CASE) ||
	      field2->Contains(pat, IGNORE_CASE) ) {
	    This->aliasBox->SelectItemOnly(*item);
	    This->fieldView->ScrollToItem(*item);
	    This->findIndex = (i + 1) % count;
	    found = True;
	 }
      }

//
// Look from the beginning of the list to the start index
//
      for (i=0; !found && i<This->findIndex; i++) {
	 VItemC		*item = This->aliasBox->VisItems()[i];
	 StringListC&	fieldList = item->FieldList();
	 StringC		*field1 = fieldList[0];
	 StringC		*field2 = fieldList[1];
	 if ( field1->Contains(pat, IGNORE_CASE) ||
	      field2->Contains(pat, IGNORE_CASE) ) {
	    This->aliasBox->SelectItemOnly(*item);
	    This->fieldView->ScrollToItem(*item);
	    This->findIndex = (i + 1) % count;
	    found = True;
	 }
      }

   } // End if there are items in the list

   if ( !found ) {
      XBell(halApp->display, ishApp->appPrefs->bellVolume);
      This->Message("No match");
   }

} // End DoFindNext

/*---------------------------------------------------------------
 *  Callback routine to handle find previous
 */

void
PickAliasWinC::DoFindPrev(Widget, PickAliasWinC *This, XtPointer)
{
   This->ClearMessage();

   char	*cs = XmTextFieldGetString(This->findTF);
   StringC	pat(cs);
   pat.Trim();
   if ( pat.size() == 0 ) {
      set_invalid(This->findTF, True, True);
      This->PopupMessage("Please enter a search string");
      return;
   }
   
   Boolean	found = False;
   u_int	count = This->aliasBox->VisItems().size();

   if ( count > 0 ) {

      if ( This->findIndex > count-1 ) This->findIndex = 0;

//
// Look from the start index to the beginning of the list
//
      int	i;
      for (i=This->findIndex; !found && i>=0; i--) {
	 VItemC		*item = This->aliasBox->VisItems()[i];
	 StringListC&	fieldList = item->FieldList();
	 StringC		*field1 = fieldList[0];
	 StringC		*field2 = fieldList[1];
	 if ( field1->Contains(pat, IGNORE_CASE) ||
	      field2->Contains(pat, IGNORE_CASE) ) {
	    This->aliasBox->SelectItemOnly(*item);
	    This->fieldView->ScrollToItem(*item);
	    This->findIndex = i - 1;
	    if ( This->findIndex < 0 ) This->findIndex = count - 1;
	    found = True;
	 }
      }

//
// Look from the end of the list to the start index
//
      for (i=count-1; !found && i>This->findIndex; i--) {
	 VItemC		*item = This->aliasBox->VisItems()[i];
	 StringListC&	fieldList = item->FieldList();
	 StringC		*field1 = fieldList[0];
	 StringC		*field2 = fieldList[1];
	 if ( field1->Contains(pat, IGNORE_CASE) ||
	      field2->Contains(pat, IGNORE_CASE) ) {
	    This->aliasBox->SelectItemOnly(*item);
	    This->fieldView->ScrollToItem(*item);
	    This->findIndex = i - 1;
	    if ( This->findIndex < 0 ) This->findIndex = count - 1;
	    found = True;
	 }
      }

   } // End if there are items in the list

   if ( !found ) {
      XBell(halApp->display, ishApp->appPrefs->bellVolume);
      This->Message("No match");
   }

} // End DoFindNext

/*---------------------------------------------------------------
 *  Callback routine to handle find all
 */

void
PickAliasWinC::DoFindAll(Widget, PickAliasWinC *This, XtPointer)
{
   This->ClearMessage();

   char	*cs = XmTextFieldGetString(This->findTF);
   StringC	pat(cs);
   pat.Trim();
   if ( pat.size() == 0 ) {
      set_invalid(This->findTF, True, True);
      This->PopupMessage("Please enter a search string");
      return;
   }
   
//
// Look through the list, selecting all matches
//

   VItemC	*first = NULL;
   u_int	count = This->aliasBox->VisItems().size();
   for (int i=0; i<count; i++) {
      VItemC		*item = This->aliasBox->VisItems()[i];
      StringListC&	fieldList = item->FieldList();
      StringC		*field1 = fieldList[0];
      StringC		*field2 = fieldList[1];
      if ( field1->Contains(pat, IGNORE_CASE) ||
	   field2->Contains(pat, IGNORE_CASE) ) {
	 This->aliasBox->SelectItem(*item);
	 if ( !first ) first = item;
      }
   }

   if ( first ) This->fieldView->ScrollToItem(*first);
   This->aliasBox->Refresh();

   if ( !first ) {
      XBell(halApp->display, ishApp->appPrefs->bellVolume);
      This->Message("No match");
   }

} // End DoFindAll

/*---------------------------------------------------------------
 *  Callback routine to handle selection of items
 */

void
PickAliasWinC::ChangeSelection(VBoxC *vb, PickAliasWinC *This)
{
   unsigned	scount = vb->SelItems().size();

//
// Update buttons sensitivities based on number of selected items
//
   XtSetSensitive(This->okPB,    scount>0);
   XtSetSensitive(This->applyPB, scount>0);

} // End ChangeSelection

/*---------------------------------------------------------------
 *  Callback to handle double-click on entry item
 */

void
PickAliasWinC::PickAlias(VItemC *item, PickAliasWinC *This)
{
   This->aliasBox->SelectItemOnly(*item, True);
   DoOk(NULL, This, NULL);
}

/*---------------------------------------------------------------
 *  Callback routine to handle initial display.
 */

void
PickAliasWinC::DoPopup(Widget, PickAliasWinC *This, XtPointer)
{
   This->aliasBox->ViewType(This->viewType);
}

/*---------------------------------------------------------------
 *  Callback to handle ok
 */

void
PickAliasWinC::DoOk(Widget, PickAliasWinC *This, XtPointer)
{
   This->DoApply(NULL, This, NULL);
   This->Hide();
}

/*---------------------------------------------------------------
 *  Callback to handle apply
 */

void
PickAliasWinC::DoApply(Widget, PickAliasWinC *This, XtPointer)
{
//
// Build list of aliases
//
   This->aliasList.removeAll();

   StringC	name;
   Boolean	expand = XmToggleButtonGetState(This->expandTB);

   VItemListC&	list = This->aliasBox->SelItems();
   u_int	count = list.size();
   for (int i=0; i<count; i++) {
      VItemC	*item = list[i];
      if ( expand ) {
	 name = ishApp->aliasPrefs->ExpandAddress(item->Name(), 1);
	 This->aliasList.add(name);
      }
      else
	 This->aliasList.add(item->Name());
   }

//
// Tell user they're ready
//
   This->applyCall(This);

} // End DoApply

/*------------------------------------------------------------------------
 * Method to compare two aliases
 */

int
PickAliasWinC::AliasCompare(const void *a, const void *b)
{
   VItemC	*via = *(VItemC **)a;
   VItemC	*vib = *(VItemC **)b;
   return via->Name().compare(vib->Name());

} // End AliasCompare
