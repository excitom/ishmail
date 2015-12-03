/*
 *  $Id: MainMsg.C,v 1.6 2000/09/19 16:42:02 evgeny Exp $
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
#include "QuickMenu.h"
#include "SaveMgrC.h"
#include "AppPrefC.h"
#include "MsgC.h"
#include "MsgItemC.h"
#include "MsgStatus.h"
#include "UndelWinC.h"
#include "FolderC.h"
#include "SortMgrC.h"
#include "FolderPrefC.h"
#include "MsgFindWinC.h"
#include "PipeWinC.h"
#include "PrintWinC.h"
#include "SendWinC.h"
#include "MsgListC.h"

#include <hgl/WXmString.h>
#include <hgl/VBoxC.h>
#include <hgl/TBoxC.h>

#include <sys/stat.h>

/*---------------------------------------------------------------
 *  Callbacks to handle message-save
 */

void
MainWinP::DoMsgSave(Widget, MainWinP *This, XtPointer)
{
   ishApp->saveMgr->SaveMsgsToFolder(This->msgVBox->SelItems(),
   				     ishApp->saveMgr->curSaveFolder,
				     ishApp->appPrefs->deleteSaved);
}

void
MainWinP::DoMsgPUSave(Widget, MainWinP *This, XtPointer)
{
   if ( This->pub->popupOnSelected )
      ishApp->saveMgr->SaveMsgsToFolder(This->msgVBox->SelItems(),
					ishApp->saveMgr->curSaveFolder,
					ishApp->appPrefs->deleteSaved);

   else if ( This->pub->popupMsg )
      ishApp->saveMgr->SaveMsgToFolder(This->pub->popupMsg,
				       ishApp->saveMgr->curSaveFolder,
				       ishApp->appPrefs->deleteSaved);
}

/*---------------------------------------------------------------
 *  Callbacks to handle message-save-to-pattern
 */

void
MainWinP::DoMsgSavePat(Widget, MainWinP *This, XtPointer)
{
   StringC	curSaveFolder = ishApp->saveMgr->curSaveFolder;

//
// Loop through selected messages and save to the default folder for each
//
   VItemListC&	list	      = This->msgVBox->SelItems();
   u_int	count	      = list.size();
   for (int i=0; i<count; i++) {
      MsgItemC	*item = (MsgItemC*)list[i];
      ishApp->saveMgr->UpdateSaveFolder(item->msg);
      ishApp->saveMgr->SaveMsgToFolder(item->msg,
				       ishApp->saveMgr->curSaveFolder, False);
   }

   ishApp->saveMgr->curSaveFolder = curSaveFolder;

   if ( ishApp->appPrefs->deleteSaved )
      This->pub->DeleteItems(list);

} // End DoMsgSavePat

void
MainWinP::DoMsgPUSavePat(Widget, MainWinP *This, XtPointer)
{
   if ( This->pub->popupOnSelected )
      DoMsgSavePat(NULL, This, NULL);

   else if ( This->pub->popupMsg )
      ishApp->saveMgr->SaveMsgToFolder(This->pub->popupMsg,
				       ishApp->saveMgr->curSaveFolder,
				       ishApp->appPrefs->deleteSaved);
}

/*---------------------------------------------------------------
 *  Callback to handle message-save-to-selected
 */

void
MainWinP::DoMsgSaveSel(Widget, MainWinP *This, XtPointer)
{
//
// Temporarily postpone changes to the recent folders menu
//
   This->deferRecentMenu = True;

//
// Loop through folders
//
   Boolean	del    = False;	// Don't delete until last folder
   VItemListC&	slist  = This->folderVBox->SelItems();
   u_int	scount = slist.size();
   for (int s=0; s<scount; s++) {

      if ( s == scount-1 ) del = ishApp->appPrefs->deleteSaved;
      VItemC	*item = slist[s];

//
// Find the folder that contains this view item
//
      Boolean		saved  = False;
      FolderListC&	flist = ishApp->folderPrefs->OpenFolders();
      u_int		fcount = flist.size();
      for (int f=0; !saved && f<fcount; f++) {

//
// If this folder is selected, save to it
//
	 FolderC	*folder = flist[f];
	 if ( folder->icon == item ) {
	    ishApp->saveMgr->SaveMsgsToFolder(This->msgVBox->SelItems(),
					      folder, del);
	    saved = True;
	 }

      } // End for each open folder

   } // End for each selected folder

//
// Update recent folder menus now
//
   This->deferRecentMenu = False;

   if ( ishApp->appPrefs->recentFolderTime > This->recentMenuTime )
      This->BuildRecentFolderMenus();

} // End DoMsgSaveSel

void
MainWinP::DoMsgPUSaveSel(Widget, MainWinP *This, XtPointer)
{
//
// Save selected messages to selected folders
//
   if ( This->pub->popupOnSelected )
      DoMsgSaveSel(NULL, This, NULL);

   else if ( This->pub->popupMsg ) {

//
// Temporarily postpone changes to the recent folders menu
//
      This->deferRecentMenu = True;

//
// Loop through folders
//
      Boolean		del    = False;	// Don't delete until last folder
      VItemListC&	slist  = This->folderVBox->SelItems();
      u_int		scount = slist.size();
      for (int s=0; s<scount; s++) {

	 if ( s == scount-1 ) del = ishApp->appPrefs->deleteSaved;
	 VItemC	*item = slist[s];

//
// Find the folder that contains this view item
//
	 Boolean		saved  = False;
	 FolderListC&	flist  = ishApp->folderPrefs->OpenFolders();
	 u_int		fcount = flist.size();
	 for (int f=0; !saved && f<fcount; f++) {

//
// If this folder is selected, save to it
//
	    FolderC	*folder = flist[f];
	    if ( folder->icon == item ) {
	       ishApp->saveMgr->SaveMsgToFolder(This->pub->popupMsg, folder,
	       					del);
	       saved = True;
	    }

	 } // End for each open folder

      } // End for each selected folder

//
// Update recent folder menus now
//
      This->deferRecentMenu = False;

      if ( ishApp->appPrefs->recentFolderTime > This->recentMenuTime )
	 This->BuildRecentFolderMenus();

   } // End if saving popup message

} // End DoMsgPUSaveSel

/*---------------------------------------------------------------
 *  Callback to handle message-save-to
 */

void
MainWinP::DoMsgSaveTo(Widget, MainWinP *This, XtPointer)
{
   ishApp->saveMgr->SaveMsgsToFolder(This->msgVBox->SelItems(), *This->pub,
   				     This->pub->curFolder->writable);
}

void
MainWinP::DoMsgPUSaveTo(Widget, MainWinP *This, XtPointer)
{
   if ( This->pub->popupOnSelected )
      ishApp->saveMgr->SaveMsgsToFolder(This->msgVBox->SelItems(), *This->pub,
					This->pub->curFolder->writable);

   else if ( This->pub->popupMsg )
      ishApp->saveMgr->SaveMsgToFolder(This->pub->popupMsg, *This->pub,
				       This->pub->curFolder->writable);
}

void
MainWinP::DoMsgSaveToFile(Widget, MainWinP *This, XtPointer)
{
   ishApp->saveMgr->SaveMsgsToFile(This->msgVBox->SelItems(), *This->pub,
				   This->pub->curFolder->writable);
}

void
MainWinP::DoMsgPUSaveToFile(Widget, MainWinP *This, XtPointer)
{
   if ( This->pub->popupOnSelected )
      ishApp->saveMgr->SaveMsgsToFile(This->msgVBox->SelItems(), *This->pub,
				      This->pub->curFolder->writable);

   else if ( This->pub->popupMsg )
      ishApp->saveMgr->SaveMsgToFile(This->pub->popupMsg, *This->pub,
				     This->pub->curFolder->writable);
}

/*---------------------------------------------------------------
 *  Callback to handle display of message->save quick pulldown
 */

void
MainWinP::PrepareSaveQuickMenu(Widget cb, MainWinP *This, XtPointer)
{
//
// Get the record associated with this cascade button
//
   QuickInfoT	*data;
   XtVaGetValues(cb, XmNuserData, &data, NULL);

//
// See if the directory has changed since the menu was last built
//
   struct stat	dstats;
   if ( stat(data->dir, &dstats) != 0 || dstats.st_mtime > data->menuTime )
      BuildQuickFolderMenu(data, (XtCallbackProc)DoMsgSaveToButton,
      			   (XtCallbackProc)PrepareSaveQuickMenu, This);

} // End PrepareSaveQuickMenu

/*---------------------------------------------------------------
 *  Callback to handle display of message->save quick popup
 */

void
MainWinP::PreparePUSaveQuickMenu(Widget cb, MainWinP *This, XtPointer)
{
//
// Get the record associated with this cascade button
//
   QuickInfoT	*data;
   XtVaGetValues(cb, XmNuserData, &data, NULL);

//
// See if the directory has changed since the menu was last built
//
   struct stat	dstats;
   if ( stat(data->dir, &dstats) != 0 || dstats.st_mtime > data->menuTime )
      BuildQuickFolderMenu(data, (XtCallbackProc)DoMsgPUSaveToButton,
      			   (XtCallbackProc)PreparePUSaveQuickMenu, This);

} // End PreparePUSaveQuickMenu

/*---------------------------------------------------------------
 *  Callbacks to handle message-save recent and quick
 */

void
MainWinP::DoMsgSaveToButton(Widget w, MainWinP *This, XtPointer)
{
//
// Get the folder name from the button label
//
   XmString	xstr;
   XtVaGetValues(w, XmNlabelString, &xstr, NULL);
   WXmString	wstr(xstr);
   char		*cs = wstr;

//
// If this is a quick-save, we need to build the full pathname
//
   StringC	name = XtName(w);
   if ( name == "quickPB" ) {
      QuickInfoT	*cbData;
      XtVaGetValues(w, XmNuserData, &cbData, NULL);
      name = cbData->dir;
      if ( name.size() > 0 ) name += '/';
      name += cs;
   }

   else {
      name = cs;
   }

   ishApp->saveMgr->SaveMsgsToFolder(This->msgVBox->SelItems(), name,
				     ishApp->appPrefs->deleteSaved);

   XtFree(cs);
   XmStringFree(xstr);

} // End DoMsgSaveToButton

void
MainWinP::DoMsgPUSaveToButton(Widget w, MainWinP *This, XtPointer)
{
//
// Get the folder name from the button label
//
   XmString	xstr;
   XtVaGetValues(w, XmNlabelString, &xstr, NULL);
   WXmString	wstr(xstr);
   char		*cs = wstr;

//
// If this is a quick-save, we need to build the full pathname
//
   StringC	name = XtName(w);
   if ( name == "quickPB" ) {
      QuickInfoT	*cbData;
      XtVaGetValues(w, XmNuserData, &cbData, NULL);
      name = cbData->dir;
      if ( name.size() > 0 ) name += '/';
      name += cs;
   }

   else {
      name = cs;
   }

   if ( This->pub->popupOnSelected )
      ishApp->saveMgr->SaveMsgsToFolder(This->msgVBox->SelItems(), name,
					ishApp->appPrefs->deleteSaved);

   else if ( This->pub->popupMsg )
      ishApp->saveMgr->SaveMsgToFolder(This->pub->popupMsg, name,
				       ishApp->appPrefs->deleteSaved);

   XtFree(cs);
   XmStringFree(xstr);

} // End DoMsgPUSaveToButton

/*---------------------------------------------------------------
 *  Callbacks to handle message-delete
 */

void
MainWinP::DoMsgDel(Widget, MainWinP *This, XtPointer)
{
   This->pub->DeleteItems(This->msgVBox->SelItems());
}

void
MainWinP::DoMsgPUDel(Widget, MainWinP *This, XtPointer)
{
   if ( This->pub->popupOnSelected )
      This->pub->DeleteItems(This->msgVBox->SelItems());
   else if ( This->pub->popupMsg ) {
      This->pub->DeleteMsg(This->pub->popupMsg);
   }
}

/*---------------------------------------------------------------
 *  Callback to handle message-undelete last
 */

void
MainWinP::DoMsgUndelLast(Widget, MainWinP *This, XtPointer)
{
   This->pub->BusyCursor(True);
   This->pub->UndeleteItems(*This->lastDelList);
   This->lastDelList->removeAll();
   This->pub->Refresh();
   This->pub->BusyCursor(False);
}

/*---------------------------------------------------------------
 *  Callback to handle selected-message-undelete
 */

void
MainWinP::DoMsgUndelSel(Widget, MainWinP *This, XtPointer)
{
   This->UndeleteVisible();
}

/*---------------------------------------------------------------
 *  Callback to handle message-undelete from list
 */

void
MainWinP::DoMsgUndelList(Widget, MainWinP *This, XtPointer)
{
   This->UndeleteHidden();
}

/*---------------------------------------------------------------
 *  Callback to handle message-undelete popup
 */

void
MainWinP::DoMsgPUUndel(Widget, MainWinP *This, XtPointer)
{

   if ( This->pub->popupOnSelected ) {
      This->UndeleteVisible();
   }
   else if ( This->pub->popupMsg ) {
      This->pub->UndeleteMsg(This->pub->popupMsg);
      This->lastDelList->removeAll();
      This->pub->Refresh();
   }
}

/*---------------------------------------------------------------
 *  Method to undelete selected items
 */

void
MainWinP::UndeleteVisible()
{
   pub->BusyCursor(True);
   ishApp->Broadcast("");

//
// Update status
//
   VItemListC&	list = msgVBox->SelItems();
   u_int	count = list.size();
   for (int i=0; i<count; i++) {
      MsgItemC	*item = (MsgItemC*)list[i];
      item->msg->ClearDeleted();
   }
   
   pub->Refresh();

   lastDelList->removeAll();
   ishApp->Broadcast("Message(s) restored");

   pub->BusyCursor(False);

} // End UndeleteVisible

/*---------------------------------------------------------------
 *  Method to undelete from list
 */

void
MainWinP::UndeleteHidden()
{
   pub->BusyCursor(True);
   ishApp->Broadcast("");

//
// Create window if necessary
//
   if ( !ishApp->undelWin ) ishApp->undelWin = new UndelWinC(*ishApp);

//
// Show it with list.  Show it before adding the items so the pixmaps will
//    create properly.	This should probably be fixed in the view box.
//
   ishApp->undelWin->Show();
   ishApp->undelWin->SetItems(pub->curFolder->DelItemList());

   pub->BusyCursor(False);

} // End UndeleteHidden

/*---------------------------------------------------------------
 *  Callbacks to handle message-status
 */

void
MainWinP::DoMsgMarkRead(Widget, MainWinP *This, XtPointer)
{
   VItemListC&	slist = This->msgVBox->SelItems();
   u_int	count = slist.size();
   for (int i=0; i<count; i++) {
      MsgItemC	*item = (MsgItemC*)slist[i];
      item->msg->SetRead();
   }

   SortMgrC	*sortMgr = This->pub->curFolder->SortMgr();
   if ( sortMgr->Threaded() && sortMgr->StatusKey() )
      This->msgVBox->Sort();

   This->pub->Refresh();
}

void
MainWinP::DoMsgPUMarkRead(Widget, MainWinP *This, XtPointer)
{
   if ( This->pub->popupOnSelected )
      DoMsgMarkRead(NULL, This, NULL);

   else if ( This->pub->popupMsg ) {
      This->pub->popupMsg->SetRead();
      SortMgrC	*sortMgr = This->pub->curFolder->SortMgr();
      if ( sortMgr->Threaded() && sortMgr->StatusKey() )
	 This->msgVBox->Sort();
      This->pub->Refresh();
   }
}

void
MainWinP::DoMsgMarkUnread(Widget, MainWinP *This, XtPointer)
{
   VItemListC&	slist = This->msgVBox->SelItems();
   u_int	count = slist.size();
   for (int i=0; i<count; i++) {
      MsgItemC	*item = (MsgItemC*)slist[i];
      item->msg->ClearNew();
      item->msg->ClearRead();
   }

   SortMgrC	*sortMgr = This->pub->curFolder->SortMgr();
   if ( sortMgr->Threaded() && sortMgr->StatusKey() )
      This->msgVBox->Sort();
   This->pub->Refresh();
}

void
MainWinP::DoMsgPUMarkUnread(Widget, MainWinP *This, XtPointer)
{
   if ( This->pub->popupOnSelected )
      DoMsgMarkUnread(NULL, This, NULL);

   else if ( This->pub->popupMsg ) {
      This->pub->popupMsg->ClearNew();
      This->pub->popupMsg->ClearRead();
      SortMgrC	*sortMgr = This->pub->curFolder->SortMgr();
      if ( sortMgr->Threaded() && sortMgr->StatusKey() )
	 This->msgVBox->Sort();
      This->pub->Refresh();
   }
}

void
MainWinP::DoMsgMarkNew(Widget, MainWinP *This, XtPointer)
{
   VItemListC&	slist = This->msgVBox->SelItems();
   u_int	count = slist.size();
   for (int i=0; i<count; i++) {
      MsgItemC	*item = (MsgItemC*)slist[i];
      item->SetNew();
   }

   SortMgrC	*sortMgr = This->pub->curFolder->SortMgr();
   if ( sortMgr->Threaded() && sortMgr->StatusKey() )
      This->msgVBox->Sort();

   This->pub->Refresh();
}

void
MainWinP::DoMsgPUMarkNew(Widget, MainWinP *This, XtPointer)
{
   if ( This->pub->popupOnSelected )
      DoMsgMarkNew(NULL, This, NULL);

   else if ( This->pub->popupMsg ) {
      This->pub->popupMsg->SetNew();
      SortMgrC	*sortMgr = This->pub->curFolder->SortMgr();
      if ( sortMgr->Threaded() && sortMgr->StatusKey() )
	 This->msgVBox->Sort();
      This->pub->Refresh();
   }
}

void
MainWinP::DoMsgSetSaved(Widget, MainWinP *This, XtPointer)
{
   VItemListC&	slist = This->msgVBox->SelItems();
   u_int	count = slist.size();
   for (int i=0; i<count; i++) {
      MsgItemC	*item = (MsgItemC*)slist[i];
      item->SetSaved();
   }

   This->pub->Refresh();
}

void
MainWinP::DoMsgPUSetSaved(Widget, MainWinP *This, XtPointer)
{
   if ( This->pub->popupOnSelected )
      DoMsgSetSaved(NULL, This, NULL);

   else if ( This->pub->popupMsg ) {
      This->pub->popupMsg->SetSaved();
      This->pub->Refresh();
   }
}

void
MainWinP::DoMsgSetReplied(Widget, MainWinP *This, XtPointer)
{
   VItemListC&	slist = This->msgVBox->SelItems();
   u_int	count = slist.size();
   for (int i=0; i<count; i++) {
      MsgItemC	*item = (MsgItemC*)slist[i];
      item->SetReplied();
   }

   This->pub->Refresh();
}

void
MainWinP::DoMsgPUSetReplied(Widget, MainWinP *This, XtPointer)
{
   if ( This->pub->popupOnSelected )
      DoMsgSetReplied(NULL, This, NULL);

   else if ( This->pub->popupMsg )
      This->pub->popupMsg->SetReplied();
}

void
MainWinP::DoMsgSetForwarded(Widget, MainWinP *This, XtPointer)
{
   VItemListC&	slist = This->msgVBox->SelItems();
   u_int	count = slist.size();
   for (int i=0; i<count; i++) {
      MsgItemC	*item = (MsgItemC*)slist[i];
      item->SetForwarded();
   }

   This->pub->Refresh();
}

void
MainWinP::DoMsgPUSetForwarded(Widget, MainWinP *This, XtPointer)
{
   if ( This->pub->popupOnSelected )
      DoMsgSetForwarded(NULL, This, NULL);

   else if ( This->pub->popupMsg )
      This->pub->popupMsg->SetForwarded();
}

void
MainWinP::DoMsgSetResent(Widget, MainWinP *This, XtPointer)
{
   VItemListC&	slist = This->msgVBox->SelItems();
   u_int	count = slist.size();
   for (int i=0; i<count; i++) {
      MsgItemC	*item = (MsgItemC*)slist[i];
      item->SetResent();
   }

   This->pub->Refresh();
}

void
MainWinP::DoMsgPUSetResent(Widget, MainWinP *This, XtPointer)
{
   if ( This->pub->popupOnSelected )
      DoMsgSetResent(NULL, This, NULL);

   else if ( This->pub->popupMsg )
      This->pub->popupMsg->SetResent();
}

void
MainWinP::DoMsgSetPrinted(Widget, MainWinP *This, XtPointer)
{
   VItemListC&	slist = This->msgVBox->SelItems();
   u_int	count = slist.size();
   for (int i=0; i<count; i++) {
      MsgItemC	*item = (MsgItemC*)slist[i];
      item->SetPrinted();
   }

   This->pub->Refresh();
}

void
MainWinP::DoMsgPUSetPrinted(Widget, MainWinP *This, XtPointer)
{
   if ( This->pub->popupOnSelected )
      DoMsgSetPrinted(NULL, This, NULL);

   else if ( This->pub->popupMsg )
      This->pub->popupMsg->SetPrinted();
}

void
MainWinP::DoMsgSetFiltered(Widget, MainWinP *This, XtPointer)
{
   VItemListC&	slist = This->msgVBox->SelItems();
   u_int	count = slist.size();
   for (int i=0; i<count; i++) {
      MsgItemC	*item = (MsgItemC*)slist[i];
      item->SetFiltered();
   }

   This->pub->Refresh();
}

void
MainWinP::DoMsgPUSetFiltered(Widget, MainWinP *This, XtPointer)
{
   if ( This->pub->popupOnSelected )
      DoMsgSetFiltered(NULL, This, NULL);

   else if ( This->pub->popupMsg )
      This->pub->popupMsg->SetFiltered();
}

void
MainWinP::DoMsgClearAll(Widget, MainWinP *This, XtPointer)
{
   VItemListC&	slist = This->msgVBox->SelItems();
   u_int	count = slist.size();
   for (int i=0; i<count; i++) {
      MsgItemC	*item = (MsgItemC*)slist[i];
      item->ClearSaved();
      item->ClearReplied();
      item->ClearForwarded();
      item->ClearResent();
      item->ClearPrinted();
      item->ClearFiltered();
   }

   This->pub->Refresh();
}

void
MainWinP::DoMsgPUClearAll(Widget, MainWinP *This, XtPointer)
{
   if ( This->pub->popupOnSelected )
      DoMsgClearAll(NULL, This, NULL);

   else if ( This->pub->popupMsg ) {

      This->pub->popupMsg->ClearSaved();
      This->pub->popupMsg->ClearReplied();
      This->pub->popupMsg->ClearForwarded();
      This->pub->popupMsg->ClearResent();
      This->pub->popupMsg->ClearPrinted();
      This->pub->popupMsg->ClearFiltered();

      This->pub->Refresh();
   }
}

void
MainWinP::DoMsgClearSaved(Widget, MainWinP *This, XtPointer)
{
   VItemListC&	slist = This->msgVBox->SelItems();
   u_int	count = slist.size();
   for (int i=0; i<count; i++) {
      MsgItemC	*item = (MsgItemC*)slist[i];
      item->ClearSaved();
   }

   This->pub->Refresh();
}

void
MainWinP::DoMsgPUClearSaved(Widget, MainWinP *This, XtPointer)
{
   if ( This->pub->popupOnSelected )
      DoMsgClearSaved(NULL, This, NULL);

   else if ( This->pub->popupMsg ) {
      This->pub->popupMsg->ClearSaved();
      This->pub->Refresh();
   }
}

void
MainWinP::DoMsgClearReplied(Widget, MainWinP *This, XtPointer)
{
   VItemListC&	slist = This->msgVBox->SelItems();
   u_int	count = slist.size();
   for (int i=0; i<count; i++) {
      MsgItemC	*item = (MsgItemC*)slist[i];
      item->ClearReplied();
   }

   This->pub->Refresh();
}

void
MainWinP::DoMsgPUClearReplied(Widget, MainWinP *This, XtPointer)
{
   if ( This->pub->popupOnSelected )
      DoMsgClearReplied(NULL, This, NULL);

   else if ( This->pub->popupMsg )
      This->pub->popupMsg->ClearReplied();
}

void
MainWinP::DoMsgClearForwarded(Widget, MainWinP *This, XtPointer)
{
   VItemListC&	slist = This->msgVBox->SelItems();
   u_int	count = slist.size();
   for (int i=0; i<count; i++) {
      MsgItemC	*item = (MsgItemC*)slist[i];
      item->ClearForwarded();
   }

   This->pub->Refresh();
}

void
MainWinP::DoMsgPUClearForwarded(Widget, MainWinP *This, XtPointer)
{
   if ( This->pub->popupOnSelected )
      DoMsgClearForwarded(NULL, This, NULL);

   else if ( This->pub->popupMsg )
      This->pub->popupMsg->ClearForwarded();
}

void
MainWinP::DoMsgClearResent(Widget, MainWinP *This, XtPointer)
{
   VItemListC&	slist = This->msgVBox->SelItems();
   u_int	count = slist.size();
   for (int i=0; i<count; i++) {
      MsgItemC	*item = (MsgItemC*)slist[i];
      item->ClearResent();
   }

   This->pub->Refresh();
}

void
MainWinP::DoMsgPUClearResent(Widget, MainWinP *This, XtPointer)
{
   if ( This->pub->popupOnSelected )
      DoMsgClearResent(NULL, This, NULL);

   else if ( This->pub->popupMsg )
      This->pub->popupMsg->ClearResent();
}

void
MainWinP::DoMsgClearPrinted(Widget, MainWinP *This, XtPointer)
{
   VItemListC&	slist = This->msgVBox->SelItems();
   u_int	count = slist.size();
   for (int i=0; i<count; i++) {
      MsgItemC	*item = (MsgItemC*)slist[i];
      item->ClearPrinted();
   }

   This->pub->Refresh();
}

void
MainWinP::DoMsgPUClearPrinted(Widget, MainWinP *This, XtPointer)
{
   if ( This->pub->popupOnSelected )
      DoMsgClearPrinted(NULL, This, NULL);

   else if ( This->pub->popupMsg )
      This->pub->popupMsg->ClearPrinted();
}

void
MainWinP::DoMsgClearFiltered(Widget, MainWinP *This, XtPointer)
{
   VItemListC&	slist = This->msgVBox->SelItems();
   u_int	count = slist.size();
   for (int i=0; i<count; i++) {
      MsgItemC	*item = (MsgItemC*)slist[i];
      item->ClearFiltered();
   }

   This->pub->Refresh();
}

void
MainWinP::DoMsgPUClearFiltered(Widget, MainWinP *This, XtPointer)
{
   if ( This->pub->popupOnSelected )
      DoMsgClearFiltered(NULL, This, NULL);

   else if ( This->pub->popupMsg )
      This->pub->popupMsg->ClearFiltered();
}

/*---------------------------------------------------------------
 *  Callback to handle message-select all
 */

void
MainWinP::DoMsgSel(Widget, MainWinP *This, XtPointer)
{
   This->pub->BusyCursor(True);
   This->msgVBox->SelectItemsOnly(This->msgVBox->VisItems());
   This->pub->BusyCursor(False);
}

/*---------------------------------------------------------------
 *  Callback to handle message-deselect all
 */

void
MainWinP::DoMsgDesel(Widget, MainWinP *This, XtPointer)
{
   This->pub->BusyCursor(True);
   This->msgVBox->DeselectAllItems();
   This->pub->BusyCursor(False);
}

/*---------------------------------------------------------------
 *  Callback to handle simple message-find
 */

void
MainWinP::DoMsgFind(Widget, MainWinP *This, XtPointer)
{
   This->pub->BusyCursor(True);

//
// Create dialog if necessary
//
   if ( !This->msgFindWin )
      This->msgFindWin = new MsgFindWinC(*This->pub);

   This->msgFindWin->Show();
   This->pub->BusyCursor(False);

} // End DoMsgFind

/*---------------------------------------------------------------
 *  Callback to handle message-read
 */

void
MainWinP::DoMsgRead(Widget, MainWinP *This, XtPointer)
{
//
// Display selected messages.  If more than one is selected, set a flag so
//    we will display them in order.
//
   VItemListC&	list = This->msgVBox->SelItems();
   unsigned	count = list.size();

   This->pub->readingSelected = (count > 1);

//
// Display the first message
//
   MsgItemC	*item = (MsgItemC *)list[0];
   ishApp->DisplayMessage(item->msg);

} // End DoMsgRead

void
MainWinP::DoMsgPURead(Widget, MainWinP *This, XtPointer)
{
   if ( This->pub->popupOnSelected )
      DoMsgRead(NULL, This, NULL);
   else if ( This->pub->popupMsg ) {
      This->pub->readingSelected = False;
      ishApp->DisplayMessage(This->pub->popupMsg);
   }
}

/*---------------------------------------------------------------
 *  Callback to handle message-read-next
 */

void
MainWinP::DoMsgNext(Widget, MainWinP *This, XtPointer)
{
   ishApp->VerifyMailCheck();

//
// Find the next message
//
   MsgC		*next;
   MsgItemC	*item;
   VItemListC&	selList = This->msgVBox->SelItems();
   VItemListC&	visList = This->msgVBox->VisItems();

   if ( This->pub->curMsgList->size() == 1 ) {
      item = (MsgItemC*)(*This->pub->curMsgList)[0];
      next = This->pub->NextReadable(item->msg);
   }
   else if ( selList.size() > 0 ) {
      item = (MsgItemC*)selList[selList.size()-1];
      next = This->pub->NextReadable(item->msg);
      This->msgVBox->DeselectAllItems();
   }
   else if ( visList.size() > 0 ) {
      item = (MsgItemC*)visList[0];
      next = item->msg;
   }

   ishApp->DisplayMessage(next);

   This->pub->Refresh();

} // End DoMsgNext

/*---------------------------------------------------------------
 *  Callback to handle message-read-next-unread
 */

void
MainWinP::DoMsgNextUnread(Widget, MainWinP *This, XtPointer)
{
   ishApp->VerifyMailCheck();

//
// Find the next message
//
   MsgC		*next;
   MsgItemC	*item;
   VItemListC&	selList = This->msgVBox->SelItems();
   VItemListC&	visList = This->msgVBox->VisItems();

   if ( This->pub->curMsgList->size() == 1 ) {
      item = (MsgItemC*)(*This->pub->curMsgList)[0];
      next = This->pub->NextUnread(item->msg);
   }
   else if ( selList.size() > 0 ) {
      item = (MsgItemC*)selList[selList.size()-1];
      next = This->pub->NextUnread(item->msg);
   }
   else if ( visList.size() > 0 ) {
      item = (MsgItemC*)visList[0];
      next = item->msg;
   }

   ishApp->DisplayMessage(next);

} // End DoMsgNextUnread

/*---------------------------------------------------------------
 *  Callback to handle message-read-next-sender
 */

void
MainWinP::DoMsgNextSender(Widget, MainWinP *This, XtPointer)
{
   ishApp->VerifyMailCheck();

//
// Find the next message
//
   MsgC		*next;
   MsgItemC	*item;
   VItemListC&	selList = This->msgVBox->SelItems();
   VItemListC&	visList = This->msgVBox->VisItems();

   if ( This->pub->curMsgList->size() == 1 ) {
      item = (MsgItemC*)(*This->pub->curMsgList)[0];
      next = This->pub->NextSender(item->msg);
   }
   else if ( selList.size() > 0 ) {
      item = (MsgItemC*)selList[selList.size()-1];
      next = This->pub->NextSender(item->msg);
   }
   else if ( visList.size() > 0 ) {
      item = (MsgItemC*)visList[0];
      next = item->msg;
   }

   ishApp->DisplayMessage(next);

} // End DoMsgNextSender

/*---------------------------------------------------------------
 *  Callback to handle message-read-next-subject
 */

void
MainWinP::DoMsgNextSubject(Widget, MainWinP *This, XtPointer)
{
   ishApp->VerifyMailCheck();

//
// Find the next message
//
   MsgC		*next;
   MsgItemC	*item;
   VItemListC&	selList = This->msgVBox->SelItems();
   VItemListC&	visList = This->msgVBox->VisItems();

   if ( This->pub->curMsgList->size() == 1 ) {
      item = (MsgItemC*)(*This->pub->curMsgList)[0];
      next = This->pub->NextSubject(item->msg);
   }
   else if ( selList.size() > 0 ) {
      item = (MsgItemC*)selList[selList.size()-1];
      next = This->pub->NextSubject(item->msg);
   }
   else if ( visList.size() > 0 ) {
      item = (MsgItemC*)visList[0];
      next = item->msg;
   }

   ishApp->DisplayMessage(next);

} // End DoMsgNextSubject

/*---------------------------------------------------------------
 *  Callback to handle message-read-prev
 */

void
MainWinP::DoMsgPrev(Widget, MainWinP *This, XtPointer)
{
   ishApp->VerifyMailCheck();

//
// Find the previous message
//
   MsgC		*prev;
   MsgItemC	*item;
   VItemListC&	selList = This->msgVBox->SelItems();
   VItemListC&	visList = This->msgVBox->VisItems();

   if ( This->pub->curMsgList->size() == 1 ) {
      item = (MsgItemC*)(*This->pub->curMsgList)[0];
      prev = This->pub->PrevReadable(item->msg);
   }
   else if ( selList.size() > 0 ) {
      item = (MsgItemC*)selList[0];
      prev = This->pub->PrevReadable(item->msg);
   }
   else if ( visList.size() > 0 ) {
      item = (MsgItemC*)visList[visList.size()-1];
      prev = item->msg;
   }

   ishApp->DisplayMessage(prev);

} // End DoMsgPrev

/*---------------------------------------------------------------
 *  Callback to handle message-read-prev-unread
 */

void
MainWinP::DoMsgPrevUnread(Widget, MainWinP *This, XtPointer)
{
   ishApp->VerifyMailCheck();

//
// Find the previous message
//
   MsgC		*prev;
   MsgItemC	*item;
   VItemListC&	selList = This->msgVBox->SelItems();
   VItemListC&	visList = This->msgVBox->VisItems();

   if ( This->pub->curMsgList->size() == 1 ) {
      item = (MsgItemC*)(*This->pub->curMsgList)[0];
      prev = This->pub->PrevUnread(item->msg);
   }
   else if ( selList.size() > 0 ) {
      item = (MsgItemC*)selList[0];
      prev = This->pub->PrevUnread(item->msg);
   }
   else if ( visList.size() > 0 ) {
      item = (MsgItemC*)visList[visList.size()-1];
      prev = item->msg;
   }

   ishApp->DisplayMessage(prev);

} // End DoMsgPrevUnread

/*---------------------------------------------------------------
 *  Callback to handle message-read-prev-sender
 */

void
MainWinP::DoMsgPrevSender(Widget, MainWinP *This, XtPointer)
{
   ishApp->VerifyMailCheck();

//
// Find the previous message
//
   MsgC		*prev;
   MsgItemC	*item;
   VItemListC&	selList = This->msgVBox->SelItems();
   VItemListC&	visList = This->msgVBox->VisItems();

   if ( This->pub->curMsgList->size() == 1 ) {
      item = (MsgItemC*)(*This->pub->curMsgList)[0];
      prev = This->pub->PrevSender(item->msg);
   }
   else if ( selList.size() > 0 ) {
      item = (MsgItemC*)selList[0];
      prev = This->pub->PrevSender(item->msg);
   }
   else if ( visList.size() > 0 ) {
      item = (MsgItemC*)visList[visList.size()-1];
      prev = item->msg;
   }

   ishApp->DisplayMessage(prev);

} // End DoMsgPrevSender

/*---------------------------------------------------------------
 *  Callback to handle message-read-prev-subject
 */

void
MainWinP::DoMsgPrevSubject(Widget, MainWinP *This, XtPointer)
{
   ishApp->VerifyMailCheck();

//
// Find the previous message
//
   MsgC		*prev;
   MsgItemC	*item;
   VItemListC&	selList = This->msgVBox->SelItems();
   VItemListC&	visList = This->msgVBox->VisItems();

   if ( This->pub->curMsgList->size() == 1 ) {
      item = (MsgItemC*)(*This->pub->curMsgList)[0];
      prev = This->pub->PrevSubject(item->msg);
   }
   else if ( selList.size() > 0 ) {
      item = (MsgItemC*)selList[0];
      prev = This->pub->PrevSubject(item->msg);
   }
   else if ( visList.size() > 0 ) {
      item = (MsgItemC*)visList[visList.size()-1];
      prev = item->msg;
   }

   ishApp->DisplayMessage(prev);

} // End DoMsgPrevSubject

/*---------------------------------------------------------------
 *  Callback to handle file-print and message-print
 */

void
MainWinP::DoMsgPrint(Widget, MainWinP *This, XtPointer)
{
   This->pub->popupMsg = NULL;
   if ( !This->printWin ) This->printWin = new PrintWinC(*This->pub);
   if ( This->msgTBox->num_selected() > 1 ) This->printWin->ShowOrder();
   else					    This->printWin->HideOrder();
   This->printWin->Show(*This->pub);
}

void
MainWinP::DoMsgPUPrint(Widget, MainWinP *This, XtPointer)
{
   if ( !This->printWin ) This->printWin = new PrintWinC(*This->pub);
   if ( This->pub->popupOnSelected && This->msgTBox->num_selected() > 1 )
      This->printWin->ShowOrder();
   else
      This->printWin->HideOrder();
   This->printWin->Show(*This->pub);
}

/*---------------------------------------------------------------
 *  Callback to handle file-pipe and message-pipe
 */

void
MainWinP::DoMsgPipe(Widget, MainWinP *This, XtPointer)
{
   This->pub->popupMsg = NULL;
   if ( !This->pipeWin ) This->pipeWin = new PipeWinC(*This->pub);
   if ( This->msgTBox->num_selected() > 1 ) This->pipeWin->ShowOrder();
   else					    This->pipeWin->HideOrder();
   This->pipeWin->Show(*This->pub);
}

void
MainWinP::DoMsgPUPipe(Widget, MainWinP *This, XtPointer)
{
   if ( !This->pipeWin ) This->pipeWin = new PipeWinC(*This->pub);
   if ( This->pub->popupOnSelected && This->msgTBox->num_selected() > 1 )
      This->pipeWin->ShowOrder();
   else
      This->pipeWin->HideOrder();
   This->pipeWin->Show(*This->pub);
}

/*---------------------------------------------------------------
 *  Callback to handle file-compose
 */

void
MainWinP::DoMsgCompose(Widget, MainWinP*, XtPointer)
{
   SendWinC	*sendWin = ishApp->GetSendWin();
   sendWin->Show();
}

/*---------------------------------------------------------------
 *  Callback to handle message-reply
 */

void
MainWinP::DoMsgReply(Widget, MainWinP *This, XtPointer)
{
   SendWinC	*sendWin = ishApp->GetSendWin();
   VItemListC&	list = This->msgVBox->SelItems();
   MsgItemC	*item = (MsgItemC*)list[0];
   sendWin->Reply(item->msg, /*all*/False, /*inc*/False);
}

void
MainWinP::DoMsgPUReply(Widget, MainWinP *This, XtPointer)
{
   SendWinC	*sendWin = ishApp->GetSendWin();
   sendWin->Reply(This->pub->popupMsg, /*all*/False, /*inc*/False);
}

/*---------------------------------------------------------------
 *  Callback to handle message-reply-include
 */

void
MainWinP::DoMsgReplyInc(Widget, MainWinP *This, XtPointer)
{
   SendWinC	*sendWin = ishApp->GetSendWin();
   VItemListC&	list = This->msgVBox->SelItems();
   MsgItemC	*item = (MsgItemC*)list[0];
   sendWin->Reply(item->msg, /*all*/False, /*inc*/True);
}

void
MainWinP::DoMsgPUReplyInc(Widget, MainWinP *This, XtPointer)
{
   SendWinC	*sendWin = ishApp->GetSendWin();
   sendWin->Reply(This->pub->popupMsg, /*all*/False, /*inc*/True);
}

/*---------------------------------------------------------------
 *  Callback to handle message-reply-all
 */

void
MainWinP::DoMsgReplyAll(Widget, MainWinP *This, XtPointer)
{
   SendWinC	*sendWin = ishApp->GetSendWin();
   VItemListC&	list = This->msgVBox->SelItems();
   MsgItemC	*item = (MsgItemC*)list[0];
   sendWin->Reply(item->msg, /*all*/True, /*inc*/False);
}

void
MainWinP::DoMsgPUReplyAll(Widget, MainWinP *This, XtPointer)
{
   SendWinC	*sendWin = ishApp->GetSendWin();
   sendWin->Reply(This->pub->popupMsg, /*all*/True, /*inc*/False);
}

/*---------------------------------------------------------------
 *  Callback to handle message-reply-all-include
 */

void
MainWinP::DoMsgReplyAllInc(Widget, MainWinP *This, XtPointer)
{
   SendWinC	*sendWin = ishApp->GetSendWin();
   VItemListC&	list = This->msgVBox->SelItems();
   MsgItemC	*item = (MsgItemC*)list[0];
   sendWin->Reply(item->msg, /*all*/True, /*inc*/True);
}

void
MainWinP::DoMsgPUReplyAllInc(Widget, MainWinP *This, XtPointer)
{
   SendWinC	*sendWin = ishApp->GetSendWin();
   sendWin->Reply(This->pub->popupMsg, /*all*/True, /*inc*/True);
}

/*---------------------------------------------------------------
 *  Callbacks to handle message-forward
 */

void
MainWinP::DoMsgForward(Widget, MainWinP *This, XtPointer)
{
   SendWinC	*sendWin = ishApp->GetSendWin();

   VItemListC&	itemList = This->msgVBox->SelItems();
   u_int	count = itemList.size();
   MsgListC	msgList;
   for (int i=0; i<count; i++) {
      MsgItemC	*item = (MsgItemC*)itemList[i];
      msgList.add(item->msg);
   }

   sendWin->Forward(msgList, /*encapsulate*/False);
}

void
MainWinP::DoMsgPUForward(Widget, MainWinP *This, XtPointer)
{
   if ( This->pub->popupOnSelected )
      DoMsgForward(NULL, This, NULL);

   else {

      SendWinC	*sendWin = ishApp->GetSendWin();

      MsgListC	msgList;
      msgList.add(This->pub->popupMsg);

      sendWin->Forward(msgList, /*encapsulate*/False);
   }
}

/*---------------------------------------------------------------
 *  Callbacks to handle message-forward-encapsulated
 */

void
MainWinP::DoMsgForward822(Widget, MainWinP *This, XtPointer)
{
   SendWinC	*sendWin = ishApp->GetSendWin();

   VItemListC&	itemList = This->msgVBox->SelItems();
   u_int	count = itemList.size();
   MsgListC	msgList;
   for (int i=0; i<count; i++) {
      MsgItemC	*item = (MsgItemC*)itemList[i];
      msgList.add(item->msg);
   }

   sendWin->Forward(msgList, /*encapsulate*/True);
}

void
MainWinP::DoMsgPUForward822(Widget, MainWinP *This, XtPointer)
{
   if ( This->pub->popupOnSelected )
      DoMsgForward822(NULL, This, NULL);

   else {

      SendWinC	*sendWin = ishApp->GetSendWin();

      MsgListC	msgList;
      msgList.add(This->pub->popupMsg);

      sendWin->Forward(msgList, /*encapsulate*/True);
   }
}

/*---------------------------------------------------------------
 *  Callback to handle message-resend
 */

void
MainWinP::DoMsgResend(Widget, MainWinP *This, XtPointer)
{
   SendWinC	*sendWin = ishApp->GetSendWin();

   VItemListC&	itemList = This->msgVBox->SelItems();
   u_int	count = itemList.size();
   MsgListC	msgList;
   for (int i=0; i<count; i++) {
      MsgItemC	*item = (MsgItemC*)itemList[i];
      msgList.add(item->msg);
   }

   sendWin->Resend(msgList);
}

void
MainWinP::DoMsgPUResend(Widget, MainWinP *This, XtPointer)
{
   if ( This->pub->popupOnSelected )
      DoMsgResend(NULL, This, NULL);

   else {

      SendWinC	*sendWin = ishApp->GetSendWin();

      MsgListC	msgList;
      msgList.add(This->pub->popupMsg);

      sendWin->Resend(msgList);
   }
}

