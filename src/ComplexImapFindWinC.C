/*
 *  $Id: ComplexImapFindWinC.C,v 1.3 2000/08/07 11:05:16 evgeny Exp $
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
#include "ComplexImapFindWinC.h"
#include "IshAppC.h"
#include "MainWinC.h"
#include "AppPrefC.h"
#include "MsgItemC.h"
#include "MsgC.h"
#include "ImapFolderC.h"
#include "ImapServerC.h"
#include "Misc.h"
#include "ReadWinC.h"

#include <hgl/WArgList.h>
#include <hgl/RowColC.h>
#include <hgl/FieldViewC.h>
#include <hgl/VBoxC.h>
#include <hgl/TextMisc.h>
#include <hgl/rsrc.h>

#include <Xm/PushB.h>
#include <Xm/TextF.h>
#include <Xm/ToggleB.h>
#include <Xm/Label.h>

/*---------------------------------------------------------------
 *  Constructor
 */

ComplexImapFindWinC::ComplexImapFindWinC(Widget parent)
: HalDialogC("complexImapFindWin", parent)
{
   WArgList	args;

   findIndex    = 0;
   stateChanged = True;

   numList.AllowDuplicates(FALSE);
   numList.SetSorted(FALSE);

//
// Create appForm hierarchy
//
//   RowColC	termRC
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_FORM);
   RowColC	*termRC = new RowColC(appForm, "termRC", ARGS);
   termRC->Defer(True);

//
// Set up 2 columns
//
   termRC->SetOrientation(RcROW_MAJOR);
   termRC->SetColCount(2);
   termRC->SetColAlignment(XmALIGNMENT_CENTER);
   termRC->SetColAlignment(0, XmALIGNMENT_BEGINNING);
   termRC->SetColWidthAdjust(RcADJUST_EQUAL);
   termRC->SetColWidthAdjust(1, RcADJUST_ATTACH);
   termRC->SetColResize(False);
   termRC->SetColResize(1, True);

//
// Create termRC children
//
//   ToggleButton	toTB
//   TextField		toTF
//   ToggleButton	ccTB
//   TextField		ccTF
//   ToggleButton	bccTB
//   TextField		bccTF
//   ToggleButton	fromTB
//   TextField		fromTF
//   ToggleButton	subjTB
//   TextField		subjTF
//   ToggleButton	bodyTB
//   TextField		bodyTF
//   ToggleButton	msgTB
//   TextField		msgTF
//   ToggleButton	befDateTB
//   TextField		befDateTF
//   ToggleButton	onDateTB
//   TextField		onDateTF
//   ToggleButton	aftDateTB
//   TextField		aftDateTF
//   ToggleButton	recTB
//   ToggleButton	notRecTB
//   ToggleButton	seenTB
//   ToggleButton	notSeenTB
//   ToggleButton	repTB
//   ToggleButton	notRepTB
//   ToggleButton	delTB
//   ToggleButton	notDelTB
//
   toTB      = XmCreateToggleButton(*termRC, "toTB",      0,0);
   toTF      = CreateTextField     (*termRC, "toTF",      0,0);
   ccTB      = XmCreateToggleButton(*termRC, "ccTB",      0,0);
   ccTF      = CreateTextField     (*termRC, "ccTF",      0,0);
   bccTB     = XmCreateToggleButton(*termRC, "bccTB",     0,0);
   bccTF     = CreateTextField     (*termRC, "bccTF",     0,0);
   fromTB    = XmCreateToggleButton(*termRC, "fromTB",    0,0);
   fromTF    = CreateTextField     (*termRC, "fromTF",    0,0);
   subjTB    = XmCreateToggleButton(*termRC, "subjTB",    0,0);
   subjTF    = CreateTextField     (*termRC, "subjTF",    0,0);
   bodyTB    = XmCreateToggleButton(*termRC, "bodyTB",    0,0);
   bodyTF    = CreateTextField     (*termRC, "bodyTF",    0,0);
   msgTB     = XmCreateToggleButton(*termRC, "msgTB",     0,0);
   msgTF     = CreateTextField     (*termRC, "msgTF",     0,0);
   befDateTB = XmCreateToggleButton(*termRC, "befDateTB", 0,0);
   befDateTF = CreateTextField     (*termRC, "befDateTF", 0,0);
   onDateTB  = XmCreateToggleButton(*termRC, "onDateTB",  0,0);
   onDateTF  = CreateTextField     (*termRC, "onDateTF",  0,0);
   aftDateTB = XmCreateToggleButton(*termRC, "aftDateTB", 0,0);
   aftDateTF = CreateTextField     (*termRC, "aftDateTF", 0,0);
   recTB     = XmCreateToggleButton(*termRC, "recTB",     0,0);
   notRecTB  = XmCreateToggleButton(*termRC, "notRecTB",  0,0);
   seenTB    = XmCreateToggleButton(*termRC, "seenTB",    0,0);
   notSeenTB = XmCreateToggleButton(*termRC, "notSeenTB", 0,0);
   repTB     = XmCreateToggleButton(*termRC, "repTB",     0,0);
   notRepTB  = XmCreateToggleButton(*termRC, "notRepTB",  0,0);
   delTB     = XmCreateToggleButton(*termRC, "delTB",     0,0);
   notDelTB  = XmCreateToggleButton(*termRC, "notDelTB",  0,0);

   AddValueChanged(toTF,      AutoSelect,   toTB);
   AddValueChanged(ccTF,      AutoSelect,   ccTB);
   AddValueChanged(bccTF,     AutoSelect,   bccTB);
   AddValueChanged(fromTF,    AutoSelect,   fromTB);
   AddValueChanged(subjTF,    AutoSelect,   subjTB);
   AddValueChanged(bodyTF,    AutoSelect,   bodyTB);
   AddValueChanged(msgTF,     AutoSelect,   msgTB);
   AddValueChanged(befDateTF, AutoSelect,   befDateTB);
   AddValueChanged(onDateTF,  AutoSelect,   onDateTB);
   AddValueChanged(aftDateTF, AutoSelect,   aftDateTB);

   AddValueChanged(recTB,     AutoDeselect, notRecTB);
   AddValueChanged(notRecTB,  AutoDeselect, recTB);
   AddValueChanged(seenTB,    AutoDeselect, notSeenTB);
   AddValueChanged(notSeenTB, AutoDeselect, seenTB);
   AddValueChanged(repTB,     AutoDeselect, notRepTB);
   AddValueChanged(notRepTB,  AutoDeselect, repTB);
   AddValueChanged(delTB,     AutoDeselect, notDelTB);
   AddValueChanged(notDelTB,  AutoDeselect, delTB);

   AddValueChanged(toTF,      StateChanged, this);
   AddValueChanged(ccTF,      StateChanged, this);
   AddValueChanged(bccTF,     StateChanged, this);
   AddValueChanged(fromTF,    StateChanged, this);
   AddValueChanged(subjTF,    StateChanged, this);
   AddValueChanged(bodyTF,    StateChanged, this);
   AddValueChanged(msgTF,     StateChanged, this);
   AddValueChanged(befDateTF, StateChanged, this);
   AddValueChanged(onDateTF,  StateChanged, this);
   AddValueChanged(aftDateTF, StateChanged, this);
   AddValueChanged(toTB,      StateChanged, this);
   AddValueChanged(ccTB,      StateChanged, this);
   AddValueChanged(bccTB,     StateChanged, this);
   AddValueChanged(fromTB,    StateChanged, this);
   AddValueChanged(subjTB,    StateChanged, this);
   AddValueChanged(bodyTB,    StateChanged, this);
   AddValueChanged(msgTB,     StateChanged, this);
   AddValueChanged(befDateTB, StateChanged, this);
   AddValueChanged(onDateTB,  StateChanged, this);
   AddValueChanged(aftDateTB, StateChanged, this);
   AddValueChanged(recTB,     StateChanged, this);
   AddValueChanged(notRecTB,  StateChanged, this);
   AddValueChanged(seenTB,    StateChanged, this);
   AddValueChanged(notSeenTB, StateChanged, this);
   AddValueChanged(repTB,     StateChanged, this);
   AddValueChanged(notRepTB,  StateChanged, this);
   AddValueChanged(delTB,     StateChanged, this);
   AddValueChanged(notDelTB,  StateChanged, this);

   int	wcount = 0;
   Widget	wlist[30];
   wlist[wcount++] = toTB;
   wlist[wcount++] = toTF;
   wlist[wcount++] = ccTB;
   wlist[wcount++] = ccTF;
   wlist[wcount++] = bccTB;
   wlist[wcount++] = bccTF;
   wlist[wcount++] = fromTB;
   wlist[wcount++] = fromTF;
   wlist[wcount++] = subjTB;
   wlist[wcount++] = subjTF;
   wlist[wcount++] = bodyTB;
   wlist[wcount++] = bodyTF;
   wlist[wcount++] = msgTB;
   wlist[wcount++] = msgTF;
   wlist[wcount++] = befDateTB;
   wlist[wcount++] = befDateTF;
   wlist[wcount++] = onDateTB;
   wlist[wcount++] = onDateTF;
   wlist[wcount++] = aftDateTB;
   wlist[wcount++] = aftDateTF;
   wlist[wcount++] = recTB;
   wlist[wcount++] = notRecTB;
   wlist[wcount++] = seenTB;
   wlist[wcount++] = notSeenTB;
   wlist[wcount++] = repTB;
   wlist[wcount++] = notRepTB;
   wlist[wcount++] = delTB;
   wlist[wcount++] = notDelTB;
   termRC->SetChildren(wlist, wcount);
   termRC->Defer(False);

   XtManageChild(*termRC);	// termForm children

//
// Add buttons
//
   AddButtonBox();
   Widget findNextPB = XmCreatePushButton(buttonRC, "findNextPB", ARGS);
   Widget findPrevPB = XmCreatePushButton(buttonRC, "findPrevPB", ARGS);
   Widget findAllPB  = XmCreatePushButton(buttonRC, "findAllPB",  ARGS);
   Widget clearPB    = XmCreatePushButton(buttonRC, "clearPB",    ARGS);
   Widget donePB     = XmCreatePushButton(buttonRC, "donePB",     ARGS);
   Widget helpPB     = XmCreatePushButton(buttonRC, "helpPB",     ARGS);

   wcount = 0;
   wlist[wcount++] = findNextPB;
   wlist[wcount++] = findPrevPB;
   wlist[wcount++] = findAllPB;
   wlist[wcount++] = clearPB;
   wlist[wcount++] = donePB;
   wlist[wcount++] = helpPB;
   XtManageChildren(wlist, wcount);

   AddActivate(findNextPB, DoFindNext, this);
   AddActivate(findPrevPB, DoFindPrev, this);
   AddActivate(findAllPB,  DoFindAll,  this);
   AddActivate(clearPB,    DoClear,    this);
   AddActivate(donePB,     DoHide,     this);
   AddActivate(helpPB,     HalAppC::DoHelp, "helpcard");

   ShowInfoMsg();		// Add message line

   HandleHelp();

} // End constructor

/*---------------------------------------------------------------
 *  Method to display dialog
 */

void
ComplexImapFindWinC::Show()
{
   findIndex = 0;
   HalDialogC::Show();
}

/*---------------------------------------------------------------
 *  Callback to auto select toggle button when text field changes
 */

void
ComplexImapFindWinC::AutoSelect(Widget tf, Widget tb, XtPointer)
{
   XmToggleButtonSetState(tb, XmTextFieldGetLastPosition(tf) > 0, True);
}

/*---------------------------------------------------------------
 *  Callback to auto deselect a toggle button when another toggle button
 *     is pressed
 */

void
ComplexImapFindWinC::AutoDeselect(Widget, Widget tb2,
				  XmToggleButtonCallbackStruct *tb1)
{
   if ( tb1->set )
      XmToggleButtonSetState(tb2, False, True);
}

/*---------------------------------------------------------------
 *  Callback to update stateChanged flag
 */

void
ComplexImapFindWinC::StateChanged(Widget, ComplexImapFindWinC *This, XtPointer)
{
   This->stateChanged = True;
}

/*---------------------------------------------------------------
 *  Callback routine to handle press of find next
 */

void
ComplexImapFindWinC::DoFindNext(Widget, ComplexImapFindWinC *This, XtPointer)
{
   This->ClearMessage();

   ishApp->mainWin->BusyCursor(True);

   if ( This->stateChanged )
      if ( !This->GetNumList() ) return;

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
      start = This->findIndex;

//
// Loop from the start position to the end of the list and look for the
//    next message whose number appears in the IMAP number list
//
   Boolean	found = False;
   MsgItemC	*item;
   int	index;
   for (index=start; !found && index<count; index++) {
      item = (MsgItemC*)msgList[index];
      found = This->numList.includes(item->msg->Number());
   }

//
// Loop from the beginning of the list to the start position
//
   for (index=0; !found && index<start; index++) {
      item = (MsgItemC*)msgList[index];
      found = This->numList.includes(item->msg->Number());
   }

//
// Select the item if it was found
//
   if ( found ) {

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

//                            
// Next time, start at message after this one
//
      This->findIndex = index + 1;
      if ( This->findIndex >= count ) This->findIndex = 0;
   }

   else {
      This->findIndex = 0;
      ishApp->mainWin->MsgVBox().DeselectAllItems();
      This->Message("No match");
      XBell(halApp->display, ishApp->appPrefs->bellVolume);
   }

   ishApp->mainWin->BusyCursor(False);
   This->stateChanged = False;

} // End DoFindNext

/*---------------------------------------------------------------
 *  Callback routine to handle press of find prev
 */

void
ComplexImapFindWinC::DoFindPrev(Widget, ComplexImapFindWinC *This, XtPointer)
{
   This->ClearMessage();

   ishApp->mainWin->BusyCursor(True);

   if ( This->stateChanged )
      if ( !This->GetNumList() ) return;

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
      start = This->findIndex;

//
// Loop from the start position to the beginning of the list and look for the
//    previous message whose number appears in the IMAP number list
//
   Boolean	found = False;
   MsgItemC	*item;
   int		index;
   for (index=start; !found && index>=0; index--) {
      item = (MsgItemC*)msgList[index];
      found = This->numList.includes(item->msg->Number());
   }

//
// Loop from the end of the list to the start position
//
   for (index=count-1; !found && index>start; index--) {
      item = (MsgItemC*)msgList[index];
      found = This->numList.includes(item->msg->Number());
   }

   if ( found ) {

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

//                            
// Next time, start at message before this one
//
      This->findIndex = index - 1;
      if ( This->findIndex <= 0 ) This->findIndex = count - 1;
   }

   else {
      ishApp->mainWin->MsgVBox().DeselectAllItems();
      This->Message("No match");
      XBell(halApp->display, ishApp->appPrefs->bellVolume);
   }

   ishApp->mainWin->BusyCursor(False);
   This->stateChanged = False;

} // End DoFindPrev

/*---------------------------------------------------------------
 *  Callback routine to handle press of find all
 */

void
ComplexImapFindWinC::DoFindAll(Widget, ComplexImapFindWinC *This, XtPointer)
{
   This->ClearMessage();

   ishApp->mainWin->BusyCursor(True);

   if ( This->stateChanged )
      if ( !This->GetNumList() ) return;

   This->findIndex = 0;
   ishApp->mainWin->MsgVBox().DeselectAllItems(False);

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
      if ( This->numList.includes(item->msg->Number()) )
	 vbox.SelectItem(*item, False);
   }

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
 *  Method to query the IMAP server for a list of the numbers of matching
 *     messages.
 */

Boolean
ComplexImapFindWinC::GetNumList()
{
//
// Build query string.  At this point we know we're checking the body
//
   StringC	query;
   CharC	str;

   if ( !AddTerm(query, "TO",      toTB,      toTF)      ) return False;
   if ( !AddTerm(query, "CC",      ccTB,      ccTF)      ) return False;
   if ( !AddTerm(query, "BCC",     bccTB,     bccTF)     ) return False;
   if ( !AddTerm(query, "FROM",    fromTB,    fromTF)    ) return False;
   if ( !AddTerm(query, "SUBJECT", subjTB,    subjTF)    ) return False;
   if ( !AddTerm(query, "BODY",    bodyTB,    bodyTF)    ) return False;
   if ( !AddTerm(query, "TEXT",    msgTB,     msgTF)     ) return False;
   if ( !AddTerm(query, "BEFORE",  befDateTB, befDateTF) ) return False;
   if ( !AddTerm(query, "ON",      onDateTB,  onDateTF)  ) return False;
   if ( !AddTerm(query, "SINCE",   aftDateTB, aftDateTF) ) return False;

   if ( !AddTerm(query, "RECENT",     recTB)     ) return False;
   if ( !AddTerm(query, "OLD",        notRecTB)  ) return False;
   if ( !AddTerm(query, "SEEN",       seenTB)    ) return False;
   if ( !AddTerm(query, "UNSEEN",     notSeenTB) ) return False;
   if ( !AddTerm(query, "ANSWERED",   repTB)     ) return False;
   if ( !AddTerm(query, "UNANSWERED", notRepTB)  ) return False;
   if ( !AddTerm(query, "DELETED",    delTB)     ) return False;
   if ( !AddTerm(query, "UNDELETED",  notDelTB)  ) return False;

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
      return False;
   }

   return True;

} // End GetNumList

/*---------------------------------------------------------------
 *  Method to add a query term for the specified text field
 */

Boolean
ComplexImapFindWinC::AddTerm(StringC& query, const char *key, Widget tb,
			     Widget tf)
{
   if ( !XmToggleButtonGetState(tb) ) return True;

   char		*cs = XmTextFieldGetString(tf);
   CharC	str(cs);
   str.Trim();

   if ( str.Length() == 0 ) {
      set_invalid(tf, True, True);
      StringC	errmsg("Please enter a search string.");
      PopupMessage(errmsg);
      return False;
   }

   StringListC	valList;
   ExtractList(str, valList, ",");

   u_int	count = valList.size();
   for (int i=0; i<count; i++) {

      StringC	*val = valList[i];

//
// Fix escaped commas
//
      int	pos = val->PosOf("\\,");
      while ( pos >= 0 ) {
	 (*val)(pos,2) = ",";
	 pos = val->PosOf("\\,", (u_int)(pos+1));
      }

//
// Add term
//
      query += key;
      query += " {";
      query += (int)val->size();
      query += "}\r\n";
      query += *val;
      query += ' ';

   } // End for each value

   return True;

} // End AddTerm

/*---------------------------------------------------------------
 *  Method to add a query term for the specified toggle
 */

Boolean
ComplexImapFindWinC::AddTerm(StringC& query, const char *key, Widget tb)
{
   if ( XmToggleButtonGetState(tb) ) {
      query += key;
      query += ' ';
   }

   return True;

} // End AddTerm

/*---------------------------------------------------------------
 *  Callback routine to handle press of clear
 */

void
ComplexImapFindWinC::DoClear(Widget, ComplexImapFindWinC *This, XtPointer)
{
   XmTextFieldSetString(This->toTF,      "");
   XmTextFieldSetString(This->ccTF,      "");
   XmTextFieldSetString(This->bccTF,     "");
   XmTextFieldSetString(This->fromTF,    "");
   XmTextFieldSetString(This->subjTF,    "");
   XmTextFieldSetString(This->bodyTF,    "");
   XmTextFieldSetString(This->msgTF,     "");
   XmTextFieldSetString(This->befDateTF, "");
   XmTextFieldSetString(This->onDateTF,  "");
   XmTextFieldSetString(This->aftDateTF, "");

   XmToggleButtonSetState(This->recTB,     False, True);
   XmToggleButtonSetState(This->notRecTB,  False, True);
   XmToggleButtonSetState(This->seenTB,    False, True);
   XmToggleButtonSetState(This->notSeenTB, False, True);
   XmToggleButtonSetState(This->repTB,     False, True);
   XmToggleButtonSetState(This->notRepTB,  False, True);
   XmToggleButtonSetState(This->delTB,     False, True);
   XmToggleButtonSetState(This->notDelTB,  False, True);
}
