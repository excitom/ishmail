/*
 * $Id: MsgFindWinC.C,v 1.3 2000/08/07 11:05:16 evgeny Exp $
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

#include "MsgFindWinC.h"
#include "MsgItemC.h"
#include "MsgC.h"
#include "IshAppC.h"
#include "MainWinC.h"
#include "FolderC.h"
#include "AppPrefC.h"
#include "HeaderC.h"
#include "HeaderValC.h"
#include "ImapFolderC.h"
#include "ImapServerC.h"
#include "ComplexMsgFindWinC.h"
#include "ComplexImapFindWinC.h"
#include "ReadWinC.h"

#include <hgl/WArgList.h>
#include <hgl/rsrc.h>
#include <hgl/HalAppC.h>
#include <hgl/FieldViewC.h>
#include <hgl/VBoxC.h>
#include <hgl/StrCase.h>
#include <hgl/TextMisc.h>
#include <hgl/RowColC.h>

#include <Xm/Form.h>
#include <Xm/PushB.h>
#include <Xm/TextF.h>
#include <Xm/Label.h>
#include <Xm/ToggleB.h>

/*---------------------------------------------------------------
 *  Constructor
 */

MsgFindWinC::MsgFindWinC(Widget parent) : HalDialogC("msgFindWin", parent)
{
   WArgList	args;
   Widget	wlist[6];

   complexWin     = NULL;
   complexImapWin = NULL;
   findIndex      = 0;
   imapOn         = False;
   stateChanged   = True;

   numList.AllowDuplicates(FALSE);
   numList.SetSorted(FALSE);

//
// Create appForm hierarchy
//
//   appForm
//      Label		patternLabel
//      Form		patternForm
//      RowColC		searchRC
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_NONE);
   Widget patternLabel = XmCreateLabel(appForm, "patternLabel", ARGS);

   args.Reset();
   args.TopAttachment(XmATTACH_WIDGET, patternLabel);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_NONE);
   Widget patternForm = XmCreateForm(appForm, "patternForm", ARGS);

   args.Reset();
   args.TopAttachment(XmATTACH_WIDGET, patternForm);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_NONE);
//
// Set up searchRC for 1 row with equal sized columns
//
   searchRC = new RowColC(appForm, "searchRC", ARGS);

   searchRC->Defer(True);
   searchRC->SetOrientation(RcCOL_MAJOR);
   searchRC->SetRowCount(1);
   searchRC->SetColAlignment(XmALIGNMENT_CENTER);
   searchRC->SetColWidthAdjust(RcADJUST_NONE);
   searchRC->SetColResize(True);
   searchRC->SetUniformCols(True);

//
// Create patternForm hierarchy
//
//   patternForm
//      TextField	patternTF
//      PushButton	complexPB
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_NONE);
   args.RightAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_FORM);
   Widget	complexPB = XmCreatePushButton(patternForm, "complexPB", ARGS);

   AddActivate(complexPB, DoComplex, this);

   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_WIDGET, complexPB);
   patternTF = CreateTextField(patternForm, "patternTF", ARGS);

   AddActivate    (patternTF, DoFind,	    this);
   AddValueChanged(patternTF, StateChanged, this);

   wlist[0] = patternTF;
   wlist[1] = complexPB;
   XtManageChildren(wlist, 2);	// patternForm children

//
// Create searchRC hierarchy
//
//   searchRC
//      ToggleButton		searchHeadTB
//      ToggleButton		searchBodyTB
//      ToggleButton		searchCaseTB
//
   searchHeadTB = XmCreateToggleButton(*searchRC, "searchHeadTB", 0,0);
   searchBodyTB = XmCreateToggleButton(*searchRC, "searchBodyTB", 0,0);
   searchCaseTB = XmCreateToggleButton(*searchRC, "searchCaseTB", 0,0);

   searchRC->AddChild(searchHeadTB);
   searchRC->AddChild(searchBodyTB);
   searchRC->AddChild(searchCaseTB);
   searchRC->Defer(False);

   AddValueChanged(searchHeadTB, StateChanged, this);
   AddValueChanged(searchBodyTB, StateChanged, this);
   AddValueChanged(searchCaseTB, StateChanged, this);

   wlist[0] = patternLabel;
   wlist[1] = patternForm;
   wlist[2] = *searchRC;
   XtManageChildren(wlist, 3);	// appForm children

   XtVaSetValues(appForm, XmNinitialFocus, patternTF, NULL);

//
// Create buttons
//
   AddButtonBox();

   Widget	findPB     = XmCreatePushButton(buttonRC, "findPB",     0,0);
   Widget	findPrevPB = XmCreatePushButton(buttonRC, "findPrevPB", 0,0);
   Widget	findAllPB  = XmCreatePushButton(buttonRC, "findAllPB",  0,0);
   Widget	donePB     = XmCreatePushButton(buttonRC, "donePB",     0,0);
   Widget	helpPB     = XmCreatePushButton(buttonRC, "helpPB",     0,0);

   AddActivate(findPB,     DoFind,	this);
   AddActivate(findPrevPB, DoFindPrev,	this);
   AddActivate(findAllPB,  DoFindAll,	this);
   AddActivate(donePB,     DoHide,	this);
   AddActivate(helpPB,     HalAppC::DoHelp, "helpcard");

   wlist[0] = findPB;
   wlist[1] = findPrevPB;
   wlist[2] = findAllPB;
   wlist[3] = donePB;
   wlist[4] = helpPB;
   XtManageChildren(wlist, 5);       // buttonRC children

   ShowInfoMsg();		// Add message line

   HandleHelp();

} // End constructor

/*---------------------------------------------------------------
 *  Constructor
 */

MsgFindWinC::~MsgFindWinC()
{
   delete complexWin;
   delete complexImapWin;
   delete searchRC;
}

/*---------------------------------------------------------------
 *  Method to display the dialog
 */

void
MsgFindWinC::Show()
{
   findIndex = 0;

   EnableWidgets();

   HalDialogC::Show();
}

/*---------------------------------------------------------------
 *  Method to display the correct widgets
 */

void
MsgFindWinC::EnableWidgets()
{
//
// Manage or unmanage widgets based on the current folder type
//
   Widget	wlist[3];
   Cardinal	wcount = 0;
   Boolean	needImap = ishApp->mainWin->curFolder->IsImap();
   if ( imapOn && !needImap ) {
      wlist[wcount++] = searchHeadTB;
      wlist[wcount++] = searchBodyTB;
      wlist[wcount++] = searchCaseTB;
      searchRC->SetChildren(wlist, wcount);
      imapOn = False;
   }
   else if ( needImap && !imapOn ) {
      wlist[wcount++] = searchHeadTB;
      wlist[wcount++] = searchBodyTB;
      searchRC->SetChildren(wlist, wcount);
      XtUnmanageChild(searchCaseTB);
      imapOn = True;
   }

} // End EnableWidgets

/*---------------------------------------------------------------
 *  Method to see if the new folders requires different widgets to be
 *     displayed
 */

void
MsgFindWinC::FolderChanged()
{
   if ( IsShown() )
      EnableWidgets();

   else {

//
// See if one of the complex search windows is shown
//
      Boolean	needImap = ishApp->mainWin->curFolder->IsImap();
      if ( complexWin && complexWin->IsShown() && needImap ) {
	 complexWin->Hide();
	 DoComplex(NULL, this, NULL);
      }

      else if ( complexImapWin && complexImapWin->IsShown() && !needImap ) {
	 complexImapWin->Hide();
	 DoComplex(NULL, this, NULL);
      }
   }

} // End FolderChanged

/*---------------------------------------------------------------
 *  Function to look through a message for the specified string
 */

Boolean
MsgFindWinC::MsgContains(MsgC *msg, CharC searchStr)
{
   StringC	val;
   Boolean	checkCase = XmToggleButtonGetState(searchCaseTB);

//
// Check the headers
//
   if ( XmToggleButtonGetState(searchHeadTB) ) {

      HeaderC	*head = msg->Headers();
      while ( head ) {

	 val.Clear();
	 head->GetValueText(val);
	 if ( val.Contains(searchStr, checkCase) ) return True;
	 head = head->next;
      }
   }

//
// Check the body
//
   if ( XmToggleButtonGetState(searchBodyTB) ) {

      val.Clear();
      msg->GetBodyText(val);
      if ( val.Contains(searchStr, checkCase) )
	 return True;
   }

   return False;

} // End MsgContains

/*---------------------------------------------------------------
 *  Callback routine to handle press of find button in find message dialog
 */

void
MsgFindWinC::DoFind(Widget, MsgFindWinC *This, XtPointer)
{
   This->ClearMessage();

//
// Get pattern
//
   if ( XmTextFieldGetLastPosition(This->patternTF) <= 0 ) {
      set_invalid(This->patternTF, True, True);
      This->PopupMessage("Please enter a search string");
      return;
   }

   ishApp->mainWin->BusyCursor(True);

   char	*searchStr = XmTextFieldGetString(This->patternTF);

   MsgItemC	*item = NULL;
   if ( This->imapOn )
      item = This->FindNextImap(searchStr);
   else
      item = This->FindNext(searchStr);

   XtFree(searchStr);

//
// Select the item if it was found
//
   if ( item ) {

//
// See if there is an unpinned reading window displayed
//
      Boolean	found  = False;
      unsigned	rcount = ishApp->readWinList.size();
      for (int i=0; !found && i<rcount; i++) {
	 ReadWinC	*readWin = (ReadWinC*)*ishApp->readWinList[i];
	 found = (readWin->IsShown() && !readWin->Pinned());
      }

      if ( found ) {
	 ishApp->DisplayMessage(item->msg);
      }
      else {
	 ishApp->mainWin->FieldView().ScrollToItem(*item);
	 ishApp->mainWin->MsgVBox().SelectItemOnly(*item);
      }

   } // End if an item was found

   else {
      ishApp->mainWin->MsgVBox().DeselectAllItems();
      This->Message("No match");
      XBell(halApp->display, ishApp->appPrefs->bellVolume);
   }

   ishApp->mainWin->BusyCursor(False);
   This->stateChanged = False;

} // End DoFind

/*---------------------------------------------------------------
 *  Method to look for the next matching message in a non-IMAP folder
 */

MsgItemC*
MsgFindWinC::FindNext(const char *searchStr)
{
//
// Loop through visible messages
//
   VItemListC&	msgList = ishApp->mainWin->MsgVBox().VisItems();
   unsigned	count = msgList.size();
   Boolean	found = False;
   int		index;
   int		start;
   MsgItemC	*item;

//
// If there is a single selected message, use that as the start index
//
   VItemListC&	selList = ishApp->mainWin->MsgVBox().SelItems();
   if ( selList.size() == 1 ) {
      VItemC	*vi = selList[0];
      start = msgList.indexOf(vi) + 1;
      if ( start >= count ) start = 0;
   }
   else
      start = findIndex;

//
// Loop from the start position to the end of the list
//
   for (index=start; !found && index<count; index++) {
      item = (MsgItemC*)msgList[index];
      found = MsgContains(item->msg, searchStr);
   }

//
// Loop from the beginning of the list to the start position
//
   for (index=0; !found && index<start; index++) {
      item = (MsgItemC*)msgList[index];
      found = MsgContains(item->msg, searchStr);
   }

//                            
// Next time, start at message after this one
//
   if ( found ) {
      findIndex = index + 1;
      if ( findIndex >= count ) findIndex = 0;
   }
   else {
      findIndex = 0;
      item      = NULL;
   }

   return item;

} // End FindNext

/*---------------------------------------------------------------
 *  Method to find the next matching message in an IMAP folder
 */

MsgItemC*
MsgFindWinC::FindNextImap(const char *searchStr)
{
//
// If we're looking at just the headers, it is quicker to do it locally
//
   Boolean	checkHead = XmToggleButtonGetState(searchHeadTB);
   Boolean	checkBody = XmToggleButtonGetState(searchBodyTB);
   if ( checkHead && !checkBody )
      return FindNext(searchStr);

   if ( !checkHead && !checkBody )
      return NULL;

//
// If the state has changed, query the server again
//
   if ( stateChanged )
      GetImapNumList(searchStr, checkHead);

   VBoxC&	vbox    = ishApp->mainWin->MsgVBox();
   VItemListC&	msgList = vbox.VisItems();
   u_int	count   = msgList.size();

//
// If there is a single selected message, use that as the start index
//
   int		start;
   VItemListC&	selList = ishApp->mainWin->MsgVBox().SelItems();
   if ( selList.size() == 1 ) {
      VItemC	*vi = selList[0];
      start = msgList.indexOf(vi) + 1;
      if ( start >= count ) start = 0;
   }
   else
      start = findIndex;

//
// Loop from the start position to the end of the list and look for the
//    next message whose number appears in the IMAP number list
//
   Boolean	found = False;
   MsgItemC	*item;
   int	index;
   for (index=start; !found && index<count; index++) {
      item = (MsgItemC*)msgList[index];
      found = numList.includes(item->msg->Number());
   }

//
// Loop from the beginning of the list to the start position
//
   for (index=0; !found && index<start; index++) {
      item = (MsgItemC*)msgList[index];
      found = numList.includes(item->msg->Number());
   }

//                            
// Next time, start at message after this one
//
   if ( found ) {
      findIndex = index + 1;
      if ( findIndex >= count ) findIndex = 0;
   }
   else {
      findIndex = 0;
      item      = NULL;
   }

   return item;

} // End FindNextImap

/*---------------------------------------------------------------
 *  Callback routine to handle press of find prev button in find message dialog
 */

void
MsgFindWinC::DoFindPrev(Widget, MsgFindWinC *This, XtPointer)
{
   This->ClearMessage();

//
// Get pattern
//
   if ( XmTextFieldGetLastPosition(This->patternTF) <= 0 ) {
      set_invalid(This->patternTF, True, True);
      This->PopupMessage("Please enter a search string");
      return;
   }

   ishApp->mainWin->BusyCursor(True);

   char	*searchStr = XmTextFieldGetString(This->patternTF);

   MsgItemC	*item = NULL;
   if ( This->imapOn )
      item = This->FindPrevImap(searchStr);
   else
      item = This->FindPrev(searchStr);

   XtFree(searchStr);

//
// Select the item if it was found
//
   if ( item ) {

//
// See if there is an unpinned reading window displayed
//
      Boolean	found = False;
      unsigned	rcount = ishApp->readWinList.size();
      for (int i=0; !found && i<rcount; i++) {
	 ReadWinC	*readWin = (ReadWinC*)*ishApp->readWinList[i];
	 found = (readWin->IsShown() && !readWin->Pinned());
      }

      if ( found ) {
	 ishApp->DisplayMessage(item->msg);
      }
      else {
	 ishApp->mainWin->FieldView().ScrollToItem(*item);
	 ishApp->mainWin->MsgVBox().SelectItemOnly(*item);
      }

   } // End if a matching item was found

   else {
      ishApp->mainWin->MsgVBox().DeselectAllItems();
      This->Message("No match");
      XBell(halApp->display, ishApp->appPrefs->bellVolume);
   }

   ishApp->mainWin->BusyCursor(False);
   This->stateChanged = False;

} // End DoFindPrev

/*---------------------------------------------------------------
 *  Method to find the previous matching message in a non-IMAP folder
 */

MsgItemC*
MsgFindWinC::FindPrev(const char *searchStr)
{
//
// Loop through visible messages
//
   VItemListC&	msgList = ishApp->mainWin->MsgVBox().VisItems();
   unsigned	count = msgList.size();
   MsgItemC	*item = NULL;
   Boolean	found = False;
   int		index;
   int		start;

//
// If there is a single selected message, use that as the start index
//
   VItemListC&	selList = ishApp->mainWin->MsgVBox().SelItems();
   if ( selList.size() == 1 ) {
      VItemC	*vi = selList[0];
      start = msgList.indexOf(vi) - 1;
      if ( start <= 0 ) start = count-1;
   }
   else
      start = findIndex;

//
// Loop from the start position to the beginning of the list
//
   for (index=start; !found && index>=0; index--) {
      item = (MsgItemC*)msgList[index];
      found = MsgContains(item->msg, searchStr);
   }

//
// Loop from the end of the list to the start position
//
   for (index=count-1; !found && index>start; index--) {
      item = (MsgItemC*)msgList[index];
      found = MsgContains(item->msg, searchStr);
   }

//                            
// Next time, start at message before this one
//
   if ( found ) {
      findIndex = index - 1;
      if ( findIndex <= 0 ) findIndex = count - 1;
   }
   else {
      findIndex = 0;
      item      = NULL;
   }

   return item;

} // End FindPrev

/*---------------------------------------------------------------
 *  Method to find the previous matching message in an IMAP folder
 */

MsgItemC*
MsgFindWinC::FindPrevImap(const char *searchStr)
{
//
// If we're looking at just the headers, it is quicker to do it locally
//
   Boolean	checkHead = XmToggleButtonGetState(searchHeadTB);
   Boolean	checkBody = XmToggleButtonGetState(searchBodyTB);
   if ( checkHead && !checkBody )
      return FindPrev(searchStr);

   if ( !checkHead && !checkBody )
      return NULL;

//
// If the state has changed, query the server again
//
   if ( stateChanged )
      GetImapNumList(searchStr, checkHead);

   VBoxC&	vbox    = ishApp->mainWin->MsgVBox();
   VItemListC&	msgList = vbox.VisItems();
   u_int	count   = msgList.size();

//
// If there is a single selected message, use that as the start index
//
   int		start;
   VItemListC&	selList = ishApp->mainWin->MsgVBox().SelItems();
   if ( selList.size() == 1 ) {
      VItemC	*vi = selList[0];
      start = msgList.indexOf(vi) - 1;
      if ( start <= 0 ) start = count-1;
   }
   else
      start = findIndex;

//
// Loop from the start position to the beginning of the list and look for the
//    previous message whose number appears in the IMAP number list
//
   Boolean	found = False;
   MsgItemC	*item;
   int		index;
   for (index=start; !found && index>=0; index--) {
      item = (MsgItemC*)msgList[index];
      found = numList.includes(item->msg->Number());
   }

//
// Loop from the end of the list to the start position
//
   for (index=count-1; !found && index>start; index--) {
      item = (MsgItemC*)msgList[index];
      found = numList.includes(item->msg->Number());
   }

//                            
// Next time, start at message before this one
//
   if ( found ) {
      findIndex = index - 1;
      if ( findIndex <= 0 ) findIndex = count - 1;
   }
   else {
      findIndex = 0;
      item      = NULL;
   }

   return item;

} // End FindPrevImap

/*---------------------------------------------------------------
 *  Callback routine to handle press of "find all" button in find
 *     message dialog
 */

void
MsgFindWinC::DoFindAll(Widget, MsgFindWinC *This, XtPointer)
{
   This->ClearMessage();

//
// Get pattern
//
   if ( XmTextFieldGetLastPosition(This->patternTF) <= 0 ) {
      set_invalid(This->patternTF, True, True);
      This->PopupMessage("Please enter a search string");
      return;
   }

   ishApp->mainWin->BusyCursor(True);

   char	*searchStr = XmTextFieldGetString(This->patternTF);

   This->findIndex = 0;
   ishApp->mainWin->MsgVBox().DeselectAllItems(False);

   if ( This->imapOn )
      This->FindAllImap(searchStr);
   else
      This->FindAll(searchStr);

   XtFree(searchStr);

   VItemListC&	selItems = ishApp->mainWin->MsgVBox().SelItems();
   if ( selItems.size() > 0 ) {

      StringC	tmp;
      tmp += (int)selItems.size();
      tmp += " item";
      if ( selItems.size() > 1 ) tmp += "s";
      tmp += " selected";
      This->Message(tmp);

//
// See if there is an unpinned reading window displayed
//
      Boolean	found = False;
      unsigned	rcount = ishApp->readWinList.size();
      for (int i=0; !found && i<rcount; i++) {
	 ReadWinC	*readWin = (ReadWinC*)*ishApp->readWinList[i];
	 found = (readWin->IsShown() && !readWin->Pinned());
      }

      MsgItemC	*item = (MsgItemC*)selItems[0];
      if ( found )
	 ishApp->DisplayMessage(item->msg);
      else
	 ishApp->mainWin->FieldView().ScrollToItem(*item);
   }
   else {
      This->Message("No match");
      XBell(halApp->display, ishApp->appPrefs->bellVolume);
   }

   ishApp->mainWin->MsgVBox().Refresh();
   ishApp->mainWin->EnableButtons();

   ishApp->mainWin->BusyCursor(False);
   This->stateChanged = False;

} // End DoFindAll

/*---------------------------------------------------------------
 *  Method to find all matching messages in a non-IMAP folder
 */

void
MsgFindWinC::FindAll(const char *searchStr)
{
//
// Loop through the visible messages
//
   VBoxC&	vbox = ishApp->mainWin->MsgVBox();
   VItemListC&	msgList = vbox.VisItems();
   unsigned	count = msgList.size();
   for (int i=0; i<count; i++) {

//
// See if this one matches
//
      MsgItemC	*item = (MsgItemC*)msgList[i];

      if ( MsgContains(item->msg, searchStr) )
	 vbox.SelectItem(*item, False);
   }

} // End FindAll

/*---------------------------------------------------------------
 *  Method to find all matching messages in an IMAP folder
 */

void
MsgFindWinC::FindAllImap(const char *searchStr)
{
//
// If we're looking at just the headers, it is quicker to do it locally
//
   Boolean	checkHead = XmToggleButtonGetState(searchHeadTB);
   Boolean	checkBody = XmToggleButtonGetState(searchBodyTB);
   if ( checkHead && !checkBody ) {
      FindAll(searchStr);
      return;
   }

   if ( !checkHead && !checkBody )
      return;

//
// If the state has changed, query the server again
//
   if ( stateChanged )
      GetImapNumList(searchStr, checkHead);

//
// Loop through the visible messages.  Select any that appear in the numList
//
   VBoxC&	vbox = ishApp->mainWin->MsgVBox();
   VItemListC&	msgList = vbox.VisItems();
   unsigned	count = msgList.size();
   for (int i=0; i<count; i++) {

//
// See if this one matches
//
      MsgItemC	*item = (MsgItemC*)msgList[i];
      if ( numList.includes(item->msg->Number()) )
	 vbox.SelectItem(*item, False);
   }

} // End FindAllImap

/*---------------------------------------------------------------
 *  Callback to handle complex message-find
 */

void
MsgFindWinC::DoComplex(Widget, MsgFindWinC *This, XtPointer)
{
   This->BusyCursor(True);

   This->Hide();

//
// See which window to show
//
   Boolean	needImap = ishApp->mainWin->curFolder->IsImap();
   if ( needImap ) {
      if ( !This->complexImapWin )
	 This->complexImapWin = new ComplexImapFindWinC(*This);
      This->complexImapWin->Show();
   }
   else {
      if ( !This->complexWin )
	 This->complexWin = new ComplexMsgFindWinC(*This);
      This->complexWin->Show();
   }

   This->BusyCursor(False);

} // End DoComplex

/*---------------------------------------------------------------
 *  Callback to mark a change to the pattern text or the state of the
 *     toggle buttons
 */

void
MsgFindWinC::StateChanged(Widget, MsgFindWinC *This, XtPointer)
{
   This->stateChanged = True;
}

/*---------------------------------------------------------------
 *  Method to query the IMAP server for a list of the numbers of matching
 *     messages.
 */

void
MsgFindWinC::GetImapNumList(const char *searchStr, Boolean checkHead)
{
//
// Build query string.  At this point we know we're checking the body
//
   StringC	query;

   if ( checkHead ) {
      query = "TEXT ";
      query += searchStr;
   }

   else {
      query = "BODY ";
      query += searchStr;
   }

//
// Run search
//
   ImapFolderC	*folder = (ImapFolderC*)ishApp->mainWin->curFolder;
   StringListC	output;

   numList.removeAll();
   if ( !folder->server->Search(query, numList, output) ) {
      StringC	errmsg("Could not communicate with IMAP server ");
      errmsg += folder->server->name;
      PopupMessage(errmsg);
   }

} // End GetImapNumList
