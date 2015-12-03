/*
 *  $Id: MainWinC.C,v 1.7 2001/07/22 16:18:06 evgeny Exp $
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
#include "MainWinC.h"
#include "MainWinP.h"
#include "IshAppC.h"
#include "AppPrefC.h"
#include "FolderC.h"
#include "AddressC.h"
#include "MsgC.h"
#include "MsgItemC.h"
#include "SumPrefC.h"
#include "SortMgrC.h"
#include "SortPrefC.h"
#include "FolderPrefC.h"
#include "UndelWinC.h"
#include "ButtonMgrC.h"
#include "MainButtPrefC.h"
#include "AutoFilePrefC.h"
#include "QuickMenu.h"
#include "MsgFindWinC.h"
#include "ReadWinC.h"
#include "MsgListC.h"

#include <hgl/VItemC.h>
#include <hgl/VBoxC.h>
#include <hgl/TBoxC.h>
#include <hgl/rsrc.h>
#include <hgl/SysErr.h>
#include <hgl/FieldViewC.h>
#include <hgl/WXmString.h>

#include <unistd.h>
#include <errno.h>

/*---------------------------------------------------------------
 *  Main window constructor
 */

MainWinC::MainWinC(Widget parent) : HalMainWinC("mainShell", parent)
{
   priv		   = new MainWinP(this);
   curFolder       = NULL;
   curMsgList	   = new VItemListC;
   readingSelected = False;
   popupFolder	   = NULL;
   popupMsg	   = NULL;
   popupOnSelected = False;

   priv->BuildMenus();
   priv->BuildWidgets();
   priv->ReadResources();

//
// Create the message sort manager
//
   sortMgr = new SortMgrC(ishApp->sortPrefs->sortKeys);

//
// Create the custom button manager
//
   buttMgr = new ButtonMgrC(this, menuBar, priv->msgTBox->ButtonBox(),
			    ishApp->mainButtPrefs->buttonStr, priv->msgTBox);

   AddMapCallback((CallbackFn *)MainWinP::DoMap, priv);

} // End constructor

/*---------------------------------------------------------------
 *  Main window destructor
 */

MainWinC::~MainWinC()
{
   delete priv;
   delete curMsgList;
   delete sortMgr;
   delete buttMgr;

} // End destructor

/*---------------------------------------------------------------
 *  Method to add a folder icon to the display
 */

void
MainWinC::ShowFolder(FolderC *folder)
{
   if ( folder->icon ) return;

#if 0
//
// We need to scan the folder if we want the correct status icon
//
   if ( ishApp->folderPrefs->showStatus && !folder->scanned )
      folder->Scan();
#endif

   folder->CreateIcon();

   folder->icon->AddOpenCallback((CallbackFn *)MainWinP::OpenFolder,     priv);
   folder->icon->AddMenuCallback((CallbackFn *)MainWinP::PostFolderMenu, priv);

   folder->icon->SetMenu(priv->folderPU);

   if ( !folder->isInBox )
      ishApp->folderPrefs->AddOpenFolder(folder);

   priv->folderVBox->AddItem(*folder->icon);

} // End ShowFolder

/*---------------------------------------------------------------
 *  Method to add our callbacks to the message icon
 */

void
MainWinC::RegisterMsgIcon(VItemC *icon)
{
   icon->AddOpenCallback((CallbackFn *)MainWinP::OpenMessage, priv);
   icon->AddMenuCallback((CallbackFn *)MainWinP::PostMsgMenu, priv);

   icon->SetMenu(priv->msgPU);
}

/*------------------------------------------------------------------------
 * Method to update summary field characteristics
 */

void
MainWinC::UpdateFields()
{
   BusyCursor(True);

//
// Sort the field columns by position
//
   ishApp->sumPrefs->UpdateFields(priv->fieldView);

//
// Loop through all message items and reset the fields
//
   if ( ishApp->systemFolder ) ishApp->systemFolder->UpdateFields();
   u_int	fcount = ishApp->folderPrefs->OpenFolders().size();
   for (int f=0; f<fcount; f++) {
      FolderC	*folder = ishApp->folderPrefs->OpenFolders()[f];
      folder->UpdateFields();
   }

   priv->msgVBox->Refresh();
   BusyCursor(False);

} // End UpdateFields

/*---------------------------------------------------------------
 *  Method to update the title string
 */

void
MainWinC::UpdateTitle()
{
   if ( ishApp->exiting || !curFolder ) return;

//
// Get the base string for the title
//
   StringC	label;
   if ( curFolder->isInBox ) label = priv->systemTitleStr;
   else			     label = priv->folderTitleStr;

   if ( !curFolder->isInBox ) {
      StringC	name = curFolder->name;
      ishApp->AbbreviateFolderName(name);
      if ( name.size() > 32 ) name += "\n";
      label.Replace("$NAME", name);
   }

//
// Add the total message count
//
   label += " (" + priv->msgCountStr;
   int	count = (int)curFolder->MsgItemList().size() +
		(int)curFolder->DelItemList().size();
   StringC	countStr;
   countStr += count;
   label.Replace("$COUNT", countStr);

//
// Add the new message count
//
   int	newCount = curFolder->NewMsgCount();
   if ( newCount > 0 ) {

      label += ", " + priv->newMsgCountStr;
      countStr.Clear();
      countStr += newCount;
      label.Replace("$COUNT", countStr);
   }

//
// Add the unread message count
//
   count = curFolder->UnreadMsgCount();
   if ( count > 0 ) {

      label += ", " + priv->unrMsgCountStr;
      countStr.Clear();
      countStr += count;
      label.Replace("$COUNT", countStr);
   }

//
// Add the saved message count
//
   count = curFolder->SavedMsgCount();
   if ( count > 0 ) {

      label += ", " + priv->savMsgCountStr;
      countStr.Clear();
      countStr += count;
      label.Replace("$COUNT", countStr);
   }

//
// Add the deleted message count
//
   count = curFolder->DeletedMsgCount();
   if ( count > 0 ) {

      label += ", " + priv->delMsgCountStr;
      countStr.Clear();
      countStr += count;
      label.Replace("$COUNT", countStr);
   }

   label += ")";
   priv->msgTBox->Title(label);

//
// Update in-box icon if there's no more new mail
//
   if ( curFolder->isInBox && mapped )
      curFolder->UpdateIcon();

//
// Update the application icon's title so summary info is available even
// if the program is iconified.
// (commented-out line updates the title at the top of the window, too,
//  though this seems a bit redundant in addition to the status line in
//  the main window.)
//
   label = "Ishmail: " + label;
   char *titleString = label;
   //XtVaSetValues(topLevel, XmNtitle, titleString, XmNiconName, titleString, NULL);
   XtVaSetValues(topLevel, XmNiconName, titleString, NULL);

//
// Update application icon.  We have to do it here because some window
//    managers show the icon even if the window is not iconified.
//
   PixmapC *pm;
   if      ( ishApp->systemFolder->HasNewMessages() )
      pm =  priv->NewMailPixmap();
   else if ( ishApp->systemFolder->HasUnreadMessages() )
      pm = priv->unreadMailPM;
   else if ( ishApp->systemFolder->MsgItemList().size() > 0 )
      pm = priv->readMailPM;
   else
      pm = priv->noMailPM;

   XtVaSetValues(topLevel, XmNiconPixmap, pm->reg,
                           XmNiconMask, pm->mask, NULL);

} // End UpdateTitle

/*---------------------------------------------------------------
 *  Method to set buttons sensitivities
 */

void
MainWinC::EnableButtons()
{
   if ( ishApp->exiting || !curFolder || !curFolder->active ) return;

#if 1
//
// Get the list and count of messages
//
   VItemListC&	msgList = priv->msgVBox->VisItems();
   unsigned	msgCount = msgList.size();

//
// Get the list and count of selected messages
//
   VItemListC&	selList = priv->msgTBox->get_selected();
   unsigned	selCount = selList.size();

//
// Get the number of selected folders
//
   unsigned	folderSelCount = priv->folderVBox->SelItems().size();
   unsigned	folderCount    = priv->folderVBox->Items().size();

//
// See if changes have been made to the current folder
//
   Boolean saveNeeded = (curFolder->writable && curFolder->Changed());

//
// See if there are any changed folders
//
   Boolean anyReadOnly = !ishApp->systemFolder->writable;
   Boolean anyChanged  = saveNeeded || ishApp->systemFolder->Changed();
   Boolean selReadOnly = False;
   Boolean selChanged  = False;
   unsigned	count = ishApp->folderPrefs->OpenFolders().size();
   int	i;
   for (i=0; i<count; i++) {

      FolderC	*folder = ishApp->folderPrefs->OpenFolders()[i];
      VItemC	*icon   = folder->icon;
      if ( folder->isInBox || folder == curFolder ) continue;

      if ( !anyReadOnly ) 
	 anyReadOnly = !folder->writable;

      Boolean	thisSelected = priv->folderVBox->SelItems().includes(icon);
      if ( !selReadOnly ) 
	 selReadOnly = (thisSelected && !folder->writable);

      Boolean	thisChanged = (folder->writable && folder->Changed());
      if ( !anyChanged ) 
	 anyChanged = thisChanged;

      if ( !selChanged )
	 selChanged = (thisChanged && thisSelected);
   }

   if ( anyReadOnly ) anyChanged = False;
   if ( selReadOnly ) selChanged = False;

//
// Count the number of deleted items in the current folder
//
   unsigned	delCount = curFolder->DelItemList().size();

//
// Count the number of deleted items that are selected
//
   unsigned	delSelCount = 0;
   if ( curFolder->writable ) {
      for (i=0; i<selCount; i++) {
	 MsgItemC	*item = (MsgItemC*)selList[i];
	 if ( item->IsDeleted() ) delSelCount++;
      }
   }

//
// Update buttons sensitivities based on number of selected items
//
   XtSetSensitive(priv->filePrintPB, selCount>0 && delSelCount==0);

   XtSetSensitive(priv->folderOpenRecentCB,
   		  ishApp->appPrefs->recentFolders.size()>0);
   XtSetSensitive(priv->folderActSysPB,    !curFolder->isInBox);
   XtSetSensitive(priv->folderActSelPB,    folderSelCount==1);
   XtSetSensitive(priv->folderSaveCurPB,   saveNeeded);
   XtSetSensitive(priv->folderSaveSelPB,   selChanged);
   XtSetSensitive(priv->folderSaveAllPB,   anyChanged);
   XtSetSensitive(priv->folderReadSelPB,   folderSelCount>0);
   XtSetSensitive(priv->folderCloseCurPB,  !curFolder->isInBox);
   XtSetSensitive(priv->folderCloseSelPB,  folderSelCount>0);
   XtSetSensitive(priv->folderCloseAllPB,  folderCount>1);
   XtSetSensitive(priv->folderDelCurPB,
   		  !curFolder->isInBox && curFolder->writable);
   XtSetSensitive(priv->folderDelSelPB,    folderSelCount>0 && !selReadOnly);
   XtSetSensitive(priv->folderDelAllPB,    folderCount>1    && !anyReadOnly);
   XtSetSensitive(priv->folderSelPB,       folderSelCount<folderCount);
   XtSetSensitive(priv->folderDeselPB,     folderSelCount>0);

   XtSetSensitive(priv->msgReplyPB,       selCount==1);
   XtSetSensitive(priv->msgReplyAllPB,    selCount==1);
   XtSetSensitive(priv->msgReplyIncPB,    selCount==1);
   XtSetSensitive(priv->msgReplyAllIncPB, selCount==1);
   XtSetSensitive(priv->msgForwardPB,     selCount>0);
   XtSetSensitive(priv->msgForward822PB,  selCount>0);
   XtSetSensitive(priv->msgResendPB,      selCount>0);
   XtSetSensitive(priv->msgReadPB,        selCount>0);
   XtSetSensitive(priv->msgSavePB,        selCount>0);
   XtSetSensitive(priv->msgSavePatPB,     selCount>0);
   XtSetSensitive(priv->msgSaveSelPB,     selCount>0 && folderSelCount>0);
   XtSetSensitive(priv->msgSaveToPB,      selCount>0);
   XtSetSensitive(priv->msgSaveToFilePB,  selCount>0);
   XtSetSensitive(priv->msgSaveRecentCB,
		  selCount>0 && ishApp->appPrefs->recentFolders.size()>0);
   XtSetSensitive(priv->msgSaveQuickCB,   selCount>0);
   XtSetSensitive(priv->msgPrintPB,       selCount>0);
   XtSetSensitive(priv->msgPipePB,        selCount>0);
   XtSetSensitive(priv->msgStatCB,        curFolder->writable && selCount>0);
   XtSetSensitive(priv->msgDelPB,
   		  curFolder->writable && selCount>delSelCount);
   XtSetSensitive(priv->msgUndelLastPB,   priv->lastDelList->size()>0);
   XtSetSensitive(priv->msgUndelSelPB,
   		  !ishApp->appPrefs->hideDeleted && delSelCount>0);
   XtSetSensitive(priv->msgUndelListPB,
   		  ishApp->appPrefs->hideDeleted && delCount>0);
   XtSetSensitive(priv->msgSelPB,         selCount<msgCount);
   XtSetSensitive(priv->msgDeselPB,       selCount>0);
   XtSetSensitive(priv->msgFindPB,        msgCount>0);

//
// See if the next or previous buttons can be used.
//
   if ( curMsgList->size()==1 || selCount==1 ) {

      MsgItemC *msgItem = (MsgItemC*)(curMsgList->size()==1 ? (*curMsgList)[0]
							    : selList[0]);
      MsgC	*msg = msgItem->msg;

      XtSetSensitive(priv->msgNextPB,        NextReadable(msg) != NULL);
      XtSetSensitive(priv->msgNextUnreadPB,  NextUnread  (msg) != NULL);
      XtSetSensitive(priv->msgNextSenderPB,  NextSender  (msg) != NULL);
      XtSetSensitive(priv->msgNextSubjectPB, NextSubject (msg) != NULL);
      XtSetSensitive(priv->msgPrevPB,        PrevReadable(msg) != NULL);
      XtSetSensitive(priv->msgPrevUnreadPB,  PrevUnread  (msg) != NULL);
      XtSetSensitive(priv->msgPrevSenderPB,  PrevSender  (msg) != NULL);
      XtSetSensitive(priv->msgPrevSubjectPB, PrevSubject (msg) != NULL);

   } // End if one message is selected or displayed

   else {

      XtSetSensitive(priv->msgNextPB,        False);
      XtSetSensitive(priv->msgNextUnreadPB,  False);
      XtSetSensitive(priv->msgNextSenderPB,  False);
      XtSetSensitive(priv->msgNextSubjectPB, False);
      XtSetSensitive(priv->msgPrevPB,        False);
      XtSetSensitive(priv->msgPrevUnreadPB,  False);
      XtSetSensitive(priv->msgPrevSenderPB,  False);
      XtSetSensitive(priv->msgPrevSubjectPB, False);
      XtSetSensitive(priv->msgPrevPB,        False);
   }
#else
//
// Update buttons sensitivities based on number of selected items
//
   XtSetSensitive(priv->filePrintPB, True);

   XtSetSensitive(priv->folderOpenRecentCB, True);
   XtSetSensitive(priv->folderActSysPB,    True);
   XtSetSensitive(priv->folderActSelPB,    True);
   XtSetSensitive(priv->folderSaveCurPB,   True);
   XtSetSensitive(priv->folderSaveSelPB,   True);
   XtSetSensitive(priv->folderSaveAllPB,   True);
   XtSetSensitive(priv->folderReadSelPB,   True);
   XtSetSensitive(priv->folderCloseCurPB,  True);
   XtSetSensitive(priv->folderCloseSelPB,  True);
   XtSetSensitive(priv->folderCloseAllPB,  True);
   XtSetSensitive(priv->folderDelCurPB,	True);
   XtSetSensitive(priv->folderDelSelPB,    True);
   XtSetSensitive(priv->folderDelAllPB,    True);
   XtSetSensitive(priv->folderSelPB,       True);
   XtSetSensitive(priv->folderDeselPB,     True);
   XtSetSensitive(priv->msgReplyPB,       True);
   XtSetSensitive(priv->msgReplyAllPB,    True);
   XtSetSensitive(priv->msgReplyIncPB,    True);
   XtSetSensitive(priv->msgReplyAllIncPB, True);
   XtSetSensitive(priv->msgForwardPB,     True);
   XtSetSensitive(priv->msgForward822PB,  True);
   XtSetSensitive(priv->msgResendPB,      True);
   XtSetSensitive(priv->msgReadPB,        True);
   XtSetSensitive(priv->msgSavePB,        True);
   XtSetSensitive(priv->msgSavePatPB,     True);
   XtSetSensitive(priv->msgSaveSelPB,     True);
   XtSetSensitive(priv->msgSaveToPB,      True);
   XtSetSensitive(priv->msgSaveToFilePB,  True);
   XtSetSensitive(priv->msgSaveRecentCB, True);
   XtSetSensitive(priv->msgSaveQuickCB,   True);
   XtSetSensitive(priv->msgPrintPB,       True);
   XtSetSensitive(priv->msgPipePB,        True);
   XtSetSensitive(priv->msgStatCB,        True);
   XtSetSensitive(priv->msgDelPB, True);
   XtSetSensitive(priv->msgUndelLastPB,   True);
   XtSetSensitive(priv->msgUndelSelPB, True);
   XtSetSensitive(priv->msgUndelListPB, True);
   XtSetSensitive(priv->msgSelPB,         True);
   XtSetSensitive(priv->msgDeselPB,       True);
   XtSetSensitive(priv->msgFindPB,        True);
   XtSetSensitive(priv->msgNextPB,        True);
   XtSetSensitive(priv->msgNextUnreadPB,  True);
   XtSetSensitive(priv->msgNextSenderPB,  True);
   XtSetSensitive(priv->msgNextSubjectPB, True);
   XtSetSensitive(priv->msgPrevPB,        True);
   XtSetSensitive(priv->msgPrevUnreadPB,  True);
   XtSetSensitive(priv->msgPrevSenderPB,  True);
   XtSetSensitive(priv->msgPrevSubjectPB, True);
u_int	count;
int	i;
#endif

//
// Update the buttons in the reading window
//
   count = ishApp->readWinList.size();
   for (i=0; i<count; i++) {

      ReadWinC	*readWin = (ReadWinC*)*ishApp->readWinList[i];
      if ( readWin->IsShown() )
	 readWin->EnableButtons();

   } // End for each reading window

   buttMgr->EnableButtons();

} // End EnableButtons

/*---------------------------------------------------------------
 *  Method to see if there are any readable messages after this one.
 */

MsgC*
MainWinC::NextReadable(MsgC *msg)
{
   if ( !msg ) return NULL;

//
// See if we need to use the view items
//
   int	index;
   if ( msg->folder->active ) {

      VItemC	 *msgItem  = msg->icon;
      VItemListC *itemList = readingSelected ? &priv->msgVBox->SelItems()
					     : &priv->msgVBox->VisItems();
      index = itemList->indexOf(msgItem);
      if ( index == itemList->NULL_INDEX ) return NULL;

//
// Loop forward and see if there are any non-deleted messages.  Stop as soon
//    as we hit one.
//
      int	msgCount = itemList->size();
      for (int i=index+1; i<msgCount; i++) {
	 MsgItemC	*item = (MsgItemC *)(*itemList)[i];
	 if ( !item->IsDeleted() && !item->IsPartial() && !item->IsCurrent() )
	    return item->msg;
      }
   }

   else {

      MsgListC	*msgList = msg->folder->msgList;
      index    = msgList->indexOf(msg);
      if ( index == msgList->NULL_INDEX ) return NULL;

//
// Loop forward and see if there are any non-deleted messages.  Stop as soon
//    as we hit one.
//
      int	msgCount = msgList->size();
      for (int i=index+1; i<msgCount; i++) {
	 MsgC	*mp = (*msgList)[i];
	 if ( !mp->IsDeleted() && !mp->IsPartial() && !mp->IsViewed() )
	    return mp;
      }

   } // End if message folder not active

   return NULL;

} // End NextReadable

/*---------------------------------------------------------------
 *  Method to see if there are any unread messages after this one.
 */

MsgC*
MainWinC::NextUnread(MsgC *msg)
{
   if ( !msg ) return NULL;

//
// See if we need to use the view items
//
   int	index;
   if ( msg->folder->active ) {

      VItemC	 *msgItem  = msg->icon;
      VItemListC *itemList = readingSelected ? &priv->msgVBox->SelItems()
					     : &priv->msgVBox->VisItems();
      index = itemList->indexOf(msgItem);
      if ( index == itemList->NULL_INDEX ) return NULL;

//
// Loop forward and see if there are any unread messages.  Stop as soon
//    as we hit one.
//
      int	msgCount = itemList->size();
      for (int i=index+1; i<msgCount; i++) {
	 MsgItemC	*item = (MsgItemC *)(*itemList)[i];
	 if ( !item->IsDeleted() && !item->IsRead() )
	    return item->msg;
      }
   }

   else {

      MsgListC	*msgList = msg->folder->msgList;
      index    = msgList->indexOf(msg);
      if ( index == msgList->NULL_INDEX ) return NULL;

//
// Loop forward and see if there are any unread messages.  Stop as soon
//    as we hit one.
//
      int	msgCount = msgList->size();
      for (int i=index+1; i<msgCount; i++) {
	 MsgC	*mp = (*msgList)[i];
	 if ( !mp->IsDeleted() && !mp->IsRead() )
	    return mp;
      }

   } // End if message folder not active

   return NULL;

} // End NextUnread

/*---------------------------------------------------------------
 *  Method to see if there are any messages from the same sender after
 *     this one.
 */

MsgC*
MainWinC::NextSender(MsgC *msg)
{
   if ( !msg ) return NULL;

   AddressC	*from1 = msg->From();
   if ( !from1 ) return NULL;

//
// See if we need to use the view items
//
   int	index;
   if ( msg->folder->active ) {

      VItemC	 *msgItem  = msg->icon;
      VItemListC *itemList = readingSelected ? &priv->msgVBox->SelItems()
					     : &priv->msgVBox->VisItems();
      index = itemList->indexOf(msgItem);
      if ( index == itemList->NULL_INDEX ) return NULL;

//
// Loop forward and see if any sender names match
//
      int	msgCount = itemList->size();
      for (int i=index+1; i<msgCount; i++) {
	 MsgItemC	*item = (MsgItemC *)(*itemList)[i];
	 if ( item->IsPartial() ) continue;

	 AddressC	*from2 = item->msg->From();
	 if ( from2 && from1->addr.Equals(from2->addr, IGNORE_CASE) )
	    return item->msg;
      }
   }

   else {

      MsgListC	*msgList = msg->folder->msgList;
      index    = msgList->indexOf(msg);
      if ( index == msgList->NULL_INDEX ) return NULL;

//
// Loop forward and see if any sender names match
//
      int	msgCount = msgList->size();
      for (int i=index+1; i<msgCount; i++) {

	 MsgC	*mp = (*msgList)[i];
	 if ( mp->IsPartial() ) continue;

	 AddressC	*from2 = mp->From();
	 if ( from2 && from1->addr.Equals(from2->addr, IGNORE_CASE) )
	    return mp;
      }

   } // End if message folder not active

   return NULL;

} // End NextSender

/*---------------------------------------------------------------
 *  Method to see if there are any messages with the same subject after
 *     this one.
 */

MsgC*
MainWinC::NextSubject(MsgC *msg)
{
   if ( !msg ) return NULL;

   CharC	sub1 = msg->Thread();

//
// See if we need to use the view items
//
   int	index;
   if ( msg->folder->active ) {

      VItemC	 *msgItem  = msg->icon;
      VItemListC *itemList = readingSelected ? &priv->msgVBox->SelItems()
					     : &priv->msgVBox->VisItems();
      index = itemList->indexOf(msgItem);
      if ( index == itemList->NULL_INDEX ) return NULL;

//
// Loop forward and see if any subjects match
//
      int	msgCount = itemList->size();
      for (int i=index+1; i<msgCount; i++) {

	 MsgItemC	*item = (MsgItemC *)(*itemList)[i];
	 if ( item->IsPartial() ) continue;

	 CharC	sub2 = item->msg->Thread();
	 if ( sub1.Equals(sub2, IGNORE_CASE) ) return item->msg;
      }
   }

   else {

      MsgListC	*msgList = msg->folder->msgList;
      index    = msgList->indexOf(msg);
      if ( index == msgList->NULL_INDEX ) return NULL;

//
// Loop forward and see if any subjects match
//
      int	msgCount = msgList->size();
      for (int i=index+1; i<msgCount; i++) {

	 MsgC	*mp = (*msgList)[i];
	 if ( mp->IsPartial() ) continue;

	 CharC	sub2 = mp->Thread();
	 if ( sub1.Equals(sub2, IGNORE_CASE) ) return mp;
      }

   } // End if message folder not active

   return NULL;

} // End NextSubject

/*---------------------------------------------------------------
 *  Method to see if there are any readable messages before this one.
 */

MsgC*
MainWinC::PrevReadable(MsgC *msg)
{
   if ( !msg ) return NULL;

//
// See if we need to use the view items
//
   int	index;
   if ( msg->folder->active ) {

      VItemC	 *msgItem  = msg->icon;
      VItemListC *itemList = readingSelected ? &priv->msgVBox->SelItems()
					     : &priv->msgVBox->VisItems();
      index = itemList->indexOf(msgItem);
      if ( index == itemList->NULL_INDEX ) return NULL;

//
// Loop backward and see if there are any non-deleted messages.  Stop as soon
//    as we hit one.
//
      for (int i=index-1; i>=0; i--) {
	 MsgItemC	*item = (MsgItemC *)(*itemList)[i];
	 if ( !item->IsDeleted() && !item->IsPartial() && !item->IsCurrent() )
	    return item->msg;
      }
   }

   else {

      MsgListC	*msgList = msg->folder->msgList;
      index    = msgList->indexOf(msg);
      if ( index == msgList->NULL_INDEX ) return NULL;

//
// Loop backward and see if there are any non-deleted messages.  Stop as soon
//    as we hit one.
//
      for (int i=index-1; i>=0; i--) {

	 MsgC	*mp = (*msgList)[i];
	 if ( !mp->IsDeleted() && !mp->IsPartial() && !mp->IsCurrent() )
	    return mp;
      }

   } // End if message folder not active

   return NULL;

} // End PrevReadable

/*---------------------------------------------------------------
 *  Method to see if there are any unread messages before this one.
 */

MsgC*
MainWinC::PrevUnread(MsgC *msg)
{
   if ( !msg ) return NULL;

//
// See if we need to use the view items
//
   int	index;
   if ( msg->folder->active ) {

      VItemC	 *msgItem  = msg->icon;
      VItemListC *itemList = readingSelected ? &priv->msgVBox->SelItems()
					     : &priv->msgVBox->VisItems();
      index = itemList->indexOf(msgItem);
      if ( index == itemList->NULL_INDEX ) return NULL;

//
// Loop backward and see if there are any unread messages.  Stop as soon
//    as we hit one.
//
      for (int i=index-1; i>=0; i--) {
	 MsgItemC	*item = (MsgItemC *)(*itemList)[i];
	 if ( !item->IsDeleted() && !item->IsRead() )
	    return item->msg;
      }
   }

   else {

      MsgListC	*msgList = msg->folder->msgList;
      index    = msgList->indexOf(msg);
      if ( index == msgList->NULL_INDEX ) return NULL;

//
// Loop backward and see if there are any non-deleted messages.  Stop as soon
//    as we hit one.
//
      for (int i=index-1; i>=0; i--) {

	 MsgC	*mp = (*msgList)[i];
	 if ( !mp->IsDeleted() && !mp->IsRead() )
	    return mp;
      }

   } // End if message folder not active

   return NULL;

} // End PrevUnread

/*---------------------------------------------------------------
 *  Method to see if there are any messages from the same sender before
 *     this one.
 */

MsgC*
MainWinC::PrevSender(MsgC *msg)
{
   if ( !msg ) return NULL;

   AddressC	*from1 = msg->From();
   if ( !from1 ) return NULL;

//
// See if we need to use the view items
//
   int	index;
   if ( msg->folder->active ) {

      VItemC	 *msgItem  = msg->icon;
      VItemListC *itemList = readingSelected ? &priv->msgVBox->SelItems()
					     : &priv->msgVBox->VisItems();
      index = itemList->indexOf(msgItem);
      if ( index == itemList->NULL_INDEX ) return NULL;

//
// Loop backward and see if any sender names match
//
      for (int i=index-1; i>=0; i--) {
	 MsgItemC	*item = (MsgItemC *)(*itemList)[i];
	 if ( item->IsPartial() ) continue;

	 AddressC	*from2 = item->msg->From();
	 if ( from2 && from1->addr.Equals(from2->addr, IGNORE_CASE) )
	    return item->msg;
      }
   }

   else {

      MsgListC	*msgList = msg->folder->msgList;
      index    = msgList->indexOf(msg);
      if ( index == msgList->NULL_INDEX ) return NULL;

//
// Loop backward and see if any sender names match
//
      for (int i=index-1; i>=0; i--) {

	 MsgC	*mp = (*msgList)[i];
	 if ( mp->IsPartial() ) continue;

	 AddressC	*from2 = mp->From();
	 if ( from2 && from1->addr.Equals(from2->addr, IGNORE_CASE) )
	    return mp;
      }

   } // End if message folder not active

   return NULL;

} // End PrevSender

/*---------------------------------------------------------------
 *  Method to see if there are any messages with the same subject before
 *     this one.
 */

MsgC*
MainWinC::PrevSubject(MsgC *msg)
{
   if ( !msg ) return NULL;

   CharC	sub1 = msg->Thread();

//
// See if we need to use the view items
//
   int	index;
   if ( msg->folder->active ) {

      VItemC	 *msgItem = msg->icon;
      VItemListC *itemList = readingSelected ? &priv->msgVBox->SelItems()
					     : &priv->msgVBox->VisItems();
      index = itemList->indexOf(msgItem);
      if ( index == itemList->NULL_INDEX ) return NULL;

//
// Loop backward and see if any subjects match
//
      for (int i=index-1; i>=0; i--) {

	 MsgItemC	*item = (MsgItemC *)(*itemList)[i];
	 if ( item->IsPartial() ) continue;

	 CharC	sub2 = item->msg->Thread();
	 if ( sub1.Equals(sub2, IGNORE_CASE ) )
	    return item->msg;
      }
   }

   else {

      MsgListC	*msgList = msg->folder->msgList;
      index    = msgList->indexOf(msg);
      if ( index == msgList->NULL_INDEX ) return NULL;

//
// Loop backward and see if any subjects match
//
      for (int i=index-1; i>=0; i--) {

	 MsgC	*mp = (*msgList)[i];
	 if ( mp->IsPartial() ) continue;

	 CharC	sub2 = mp->Thread();
	 if ( sub1.Equals(sub2, IGNORE_CASE ) )
	    return mp;
      }

   } // End if message folder not active

   return NULL;

} // End PrevSubject

/*---------------------------------------------------------------
 *  Method to open an initial folder
 */

void
MainWinC::AddInitialFolder(const char *name)
{
//
// Create a folder
//
   FolderC *folder = ishApp->folderPrefs->GetFolder(name, True/*ok to create*/);
   if ( !folder ) {
      StringC	errmsg("Could not open folder: ");
      errmsg += name;
      errmsg += ".\n";
      if ( errno != 0 ) errmsg += SystemErrorMessage(errno);
      PopupMessage(errmsg);
   }

   else
      ShowFolder(folder);
}

/*------------------------------------------------------------------------
 * Method to set the current folder
 */

void
MainWinC::SetCurrentFolder(FolderC *folder)
{
   curFolder = folder;
   priv->lastDelList->removeAll();

   priv->folderVBox->SelectItemOnly(*folder->icon);

   UpdateTitle();
   EnableButtons();

   if ( priv->msgFindWin )
      priv->msgFindWin->FolderChanged();

} // End SetCurrentFolder

/*------------------------------------------------------------------------
 * Method to activate the system folder
 */

void
MainWinC::ActivateSystemFolder()
{
   ishApp->systemFolder->Activate();
   SetCurrentFolder(ishApp->systemFolder);
}

/*---------------------------------------------------------------
 *  Method gather and display new mail
 */

void
MainWinC::GetNewMail(FolderC *folder)
{
   BusyCursor(True);

   if ( folder == ishApp->systemFolder ) {

//
// Update icons
//
      PixmapC *pm = priv->NewMailPixmap();
      priv->NewMailAlert();
      XtVaSetValues(*this, XmNiconPixmap, pm->reg,
                           XmNiconMask, pm->mask, NULL);

      if ( !ishApp->sleeping ) {

//
// Loop through all messages and see if any new ones are to be auto-filed
//
	 if ( ishApp->autoFilePrefs->autoFileOn ) priv->AutoFileNewMail();

//
// If the last message in the folder is new (hasn't been filed), announce it
//
	 MsgC	*msg = folder->NewestMsg();
	 if ( msg && !msg->WasAnnounced() ) {
	    StringC	statusMsg("New mail from %fromName: %subject");
	    msg->ReplaceHeaders(statusMsg);
	    ishApp->Broadcast(statusMsg);
	    msg->SetAnnounced();
	 }

      } // End if not sleeping

   } // End if new mail arrived in system folder

//
// Sort messages if necessary
//
   if ( !ishApp->sleeping && folder == curFolder &&
      	 curFolder->SortMgr()->Threaded() )
      priv->msgVBox->Sort();

   priv->msgVBox->Refresh();

   if ( !ishApp->sleeping && folder == curFolder ) {
      EnableButtons();
      UpdateTitle();
   }

   BusyCursor(False);

} // End GetNewMail

/*---------------------------------------------------------------
 *  Method to delete a message
 */

Boolean
MainWinC::DeleteMsg(MsgC *msg)
{
   if ( !msg->folder ) return True;

   if ( !msg->folder->active ) {
      msg->SetDeleted();
      return True;
   }

   ishApp->VerifyMailCheck();

   ishApp->BusyCursor(True);
   ishApp->Broadcast("");

//
// Update status
//
   msg->SetDeleted();
   priv->lastDelList->removeAll();
   priv->lastDelList->add(msg->icon);

//
// See if the deleted message was being read
//
   ReadWinC	*msgWin = NULL;
   u_int	count = ishApp->readWinList.size();
   for (int i=0; !msgWin && i<count; i++) {

      ReadWinC	*readWin = (ReadWinC*)*ishApp->readWinList[i];
      if ( readWin->IsShown() && msg == readWin->msg )
	 msgWin = readWin;
   }

//
// Pick a new message if we need one
//
   MsgC	*nextMsg = NULL;
   if ( msgWin ) {
      curMsgList->remove(msg->icon);
      nextMsg = NextReadable(msg);
      if ( !nextMsg ) nextMsg = PrevReadable(msg);
      if ( !nextMsg ) msgWin->Hide();
   }

//
// If there is another to be read ...
//
   if ( nextMsg ) {

//
// ... see if it needs to be selected ...
//
      VItemListC&	selList = priv->msgVBox->SelItems();
      if ( selList.includes(msg->icon) ) {
	 if ( !readingSelected ) priv->msgVBox->SelectItemOnly(*nextMsg->icon);
	 priv->fieldView->ScrollToItem(*nextMsg->icon);
      }

//
// ... and display it.
//
      ishApp->DisplayMessage(nextMsg, msgWin);
   }

//
// Remove item from view box if necessary.  This will also update button
//    sensitivities through selection change callback.
//
   if ( ishApp->appPrefs->hideDeleted ) {

      priv->msgVBox->RemoveItems(curFolder->DelItemList());
      if ( ishApp->undelWin )
	 ishApp->undelWin->SetItems(curFolder->DelItemList());
   }

//
// Re-sort if all reading window are closed
//
   if ( curMsgList->size() == 0 && curFolder->SortMgr()->Threaded() )
      priv->msgVBox->Sort();

   Refresh();

   StringC	statStr("Message ");
   statStr += msg->Number();
   statStr += " deleted.";
   ishApp->Broadcast(statStr);
   ishApp->BusyCursor(False);

   return True;

} // End DeleteMsg

/*---------------------------------------------------------------
 *  Method to delete specified messages
 */

Boolean
MainWinC::DeleteItems(VItemListC& list)
{
   if ( list.size() == 1 ) {
      MsgItemC	*item = (MsgItemC*)list[0];
      return DeleteMsg(item->msg);
   }

   ishApp->VerifyMailCheck();

   ishApp->BusyCursor(True);
   ishApp->Broadcast("");

//
// Update status
//
   curFolder->ItemsDeleted(list);
   u_int	count = list.size();
   int	i;
   for (i=0; i<count; i++) {
      MsgItemC	*item = (MsgItemC*)list[i];
      item->msg->SetDeleted();
   }
   priv->lastDelList->removeAll();
   *priv->lastDelList = list;

//
// See if any reading windows are tied to deleted messages
//
   PtrListC	delWinList;
   count = ishApp->readWinList.size();
   for (i=0; i<count; i++) {
      ReadWinC	*readWin = (ReadWinC*)*ishApp->readWinList[i];
      if ( readWin->IsShown() && list.includes(readWin->msg->icon) ) {
	 void	*tmp = (void*)readWin;
	 delWinList.add(tmp);
	 readWin->Unpin();
      }
   }

//
// If all reading windows are being closed, see if there are any more messages
//    that can be read.  If there are, we'll keep one window open.
//
   MsgC	*nextMsg = NULL;
   if ( delWinList.size() > 0 && delWinList.size() == curMsgList->size() ) {

      MsgItemC	*lastItem = (MsgItemC*)(*curMsgList)[curMsgList->size()-1];
      nextMsg = NextReadable(lastItem->msg);
      if ( !nextMsg ) nextMsg = PrevReadable(lastItem->msg);

      if ( nextMsg ) {
	 delWinList.removeLast();
	 curMsgList->removeLast();
      }
   }

//
// Close all associated reading windows.
//
   count = delWinList.size();
   for (i=0; i<count; i++) {
      ReadWinC	*readWin = (ReadWinC*)*delWinList[i];
      readWin->Hide();
      curMsgList->remove(readWin->msg->icon);
   }

//
// Display next message
//
   if ( nextMsg ) ishApp->DisplayMessage(nextMsg);

//
// Remove items from view box if necessary.  This will also update button
//    sensitivities through selection change callback.
//
   if ( ishApp->appPrefs->hideDeleted ) {
      priv->msgVBox->RemoveItems(curFolder->DelItemList());
      if ( ishApp->undelWin )
	 ishApp->undelWin->SetItems(curFolder->DelItemList());
   }

//
// Re-sort if all reading window are closed
//
   if ( curMsgList->size() == 0 && curFolder->SortMgr()->Threaded() )
      priv->msgVBox->Sort();

   Refresh();

   ishApp->Broadcast("Message(s) deleted");
   ishApp->BusyCursor(False);

   return True;

} // End DeleteItems

/*---------------------------------------------------------------
 *  Method to undelete a message
 */

Boolean
MainWinC::UndeleteMsg(MsgC *msg)
{
   ishApp->VerifyMailCheck();

   ishApp->BusyCursor(True);
   ishApp->Broadcast("");

//
// Update status
//
   msg->ClearDeleted();
   msg->icon->SetMenu(priv->msgPU);
   priv->msgVBox->AddItem(*msg->icon);

   if ( curMsgList->size() == 0 && curFolder->SortMgr()->Threaded() )
      priv->msgVBox->Sort();

   priv->lastDelList->removeAll();

   Refresh();

   ishApp->Broadcast("Message(s) undeleted");
   ishApp->BusyCursor(False);

   return True;

} // End UndeleteMsg

/*---------------------------------------------------------------
 *  Method to undelete specified messages
 */

Boolean
MainWinC::UndeleteItems(VItemListC& list)
{
   ishApp->VerifyMailCheck();

   ishApp->BusyCursor(True);
   ishApp->Broadcast("");

//
// Update status
//
   curFolder->ItemsUndeleted(list);
   u_int	count = list.size();
   for (int i=0; i<count; i++) {
      MsgItemC	*item = (MsgItemC*)list[i];
      item->msg->ClearDeleted();
      item->SetMenu(priv->msgPU);
   }
   priv->msgVBox->AddItems(list);

   if ( curMsgList->size() == 0 && curFolder->SortMgr()->Threaded() )
      priv->msgVBox->Sort();

   priv->msgVBox->Refresh();

   priv->lastDelList->removeAll();

   UpdateTitle();
   EnableButtons();

   ishApp->Broadcast("Message(s) undeleted");
   ishApp->BusyCursor(False);

   return True;

} // End UndeleteItems

/*---------------------------------------------------------------
 *  Method to change the message hiding option
 */

void
MainWinC::HideDeletedChanged()
{
   if ( ishApp->appPrefs->hideDeleted ) {

      XtManageChild  (priv->msgUndelListPB);
      XtUnmanageChild(priv->msgUndelSelPB);

      priv->msgVBox->RemoveItems(curFolder->DelItemList());
      if ( ishApp->undelWin )
	 ishApp->undelWin->SetItems(curFolder->DelItemList());
   }

   else {

      XtManageChild  (priv->msgUndelSelPB);
      XtUnmanageChild(priv->msgUndelListPB);

      if ( ishApp->undelWin ) {
	 ishApp->undelWin->Clear();
	 ishApp->undelWin->Hide();
      }
      priv->msgVBox->AddItems(curFolder->DelItemList());
   }

   if ( curFolder->SortMgr()->Threaded() )
      priv->msgVBox->Sort();

   priv->msgVBox->Refresh();

   EnableButtons();

} // End HideDeletedChanged

/*---------------------------------------------------------------
 *  Method to update display based on the fact that the folder directory
 *     has changed.
 */

void
MainWinC::FolderDirChanged()
{
//
// Loop though all folders and update abbreviations
//
   FolderListC&	list = ishApp->folderPrefs->OpenFolders();
   u_int	count = list.size();
   for (int i=0; i<count; i++) {

      FolderC	*fp = list[i];
      if ( fp->isInBox ) continue;

      fp->abbrev = fp->name;
      ishApp->AbbreviateFolderName(fp->abbrev);
      fp->icon->Label(fp->abbrev);
   }

   priv->folderVBox->Refresh();

//
// Update open and save quick menus
//
   UpdateQuickDir(priv->folderOpenQuickCB, ishApp->appPrefs->FolderDir());
   UpdateQuickDir(priv->msgSaveQuickCB,	   ishApp->appPrefs->FolderDir());
   UpdateQuickDir(priv->msgPUSaveQuickCB,  ishApp->appPrefs->FolderDir());

} // End FolderDirChanged

void
MainWinC::Refresh()
{
   priv->folderVBox->Refresh();
   priv->msgVBox->Refresh();
   UpdateTitle();
   EnableButtons();
}

/*---------------------------------------------------------------
 *  Query private stuff
 */

VBoxC&	    MainWinC::FolderVBox()	{ return *priv->folderVBox; }
VBoxC&	    MainWinC::MsgVBox()		{ return *priv->msgVBox; }
FieldViewC& MainWinC::FieldView()	{ return *priv->fieldView; }
