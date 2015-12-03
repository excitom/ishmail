/*
 *  $Id: MainFolder.C,v 1.6 2000/09/19 14:45:27 evgeny Exp $
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
#include "FolderPrefC.h"
#include "AppPrefC.h"
#include "ConfPrefC.h"
#include "MsgItemC.h"
#include "MsgC.h"
#include "FileChooserWinC.h"
#include "MhFolderC.h"
#include "MmdfFolderC.h"
#include "UnixFolderC.h"
#include "ImapFolderC.h"
#include "ImapMisc.h"
#include "ReadWinC.h"

#include <hgl/WXmString.h>
#include <hgl/VBoxC.h>
#include <hgl/rsrc.h>
#include <hgl/WArgList.h>
#include <hgl/SysErr.h>

#include <Xm/MessageB.h>
#include <Xm/Frame.h>
#include <Xm/RowColumn.h>
#include <Xm/ToggleB.h>

#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

/*---------------------------------------------------------------
 *  Callback to handle display of folder->open quick pulldown
 */

void
MainWinP::PrepareOpenQuickMenu(Widget cb, MainWinP *This, XtPointer)
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
   if ( IsImapName(data->dir) || stat(data->dir, &dstats) != 0 ||
				 dstats.st_mtime > data->menuTime )
      BuildQuickFolderMenu(data, (XtCallbackProc)DoFolderOpenButton,
      			   (XtCallbackProc)PrepareOpenQuickMenu, This);

} // End PrepareOpenQuickMenu

/*---------------------------------------------------------------
 *  Callback to handle folder-open quick or recent
 */

void
MainWinP::DoFolderOpenButton(Widget w, MainWinP *This, XtPointer)
{
   ishApp->BusyCursor(True);

//
// Get the folder name from the button label
//
   XmString	xstr;
   XtVaGetValues(w, XmNlabelString, &xstr, NULL);
   WXmString	wstr(xstr);
   char		*cs = wstr;

//
// If this is a quick-open, we need to build the full pathname
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

   XtFree(cs);
   XmStringFree(xstr);

//
// Get a pointer to the folder
//
   FolderC *folder = ishApp->folderPrefs->GetFolder(name, False/*no create*/);
   if ( !folder ) {
      StringC	errmsg("Could not open folder: ");
      errmsg += name;
      errmsg += ".\n";
      if ( errno != 0 ) errmsg += SystemErrorMessage(errno);
      This->pub->PopupMessage(errmsg);
      ishApp->BusyCursor(False);
      return;
   }

   if ( folder->active ) {
      ishApp->BusyCursor(False);	// Already open
      return;
   }

//
// Close any unpinned reading windows
//
   u_int	count = ishApp->readWinList.size();
   for (int i=0; i<count; i++) {
      ReadWinC	*readWin = (ReadWinC*)*ishApp->readWinList[i];
      if ( !readWin->Pinned() ) readWin->Hide();
   }

//
// If the folder has not yet been displayed, show it.
//
   if ( !folder->icon ) This->pub->ShowFolder(folder);

//
// Switch folders
//
   This->pub->curFolder->Deactivate();
   folder->Activate();

   if ( folder->active ) {
      This->pub->SetCurrentFolder(folder);
      ishApp->appPrefs->AddRecentFolder(name);
   }
   else {
      ishApp->folderPrefs->RemoveFolder(folder);
      delete folder;
      This->pub->ActivateSystemFolder();
   }

   This->folderVBox->Refresh();
   This->msgVBox->Refresh();
   This->pub->BusyCursor(False);

} // End DoFolderOpenButton

/*---------------------------------------------------------------
 *  Callback to handle folder-activate system
 */

void
MainWinP::DoFolderActSys(Widget, MainWinP *This, XtPointer)
{
   OpenFolder(ishApp->systemFolder->icon, This);
}

/*---------------------------------------------------------------
 *  Callback to handle folder-activate selected
 */

void
MainWinP::DoFolderActSel(Widget, MainWinP *This, XtPointer)
{
   VItemC	*item = This->folderVBox->SelItems()[0];

   FolderListC&	list  = ishApp->folderPrefs->OpenFolders();
   u_int	count = list.size();
   for (int i=0; i<count; i++) {

      FolderC	*folder = list[i];
      if ( folder->icon == item ) {
	 OpenFolder(folder->icon, This);
	 return;
      }
   }

} // End DoFolderActSel

/*---------------------------------------------------------------
 *  Callback to handle folder-activate popup
 */

void
MainWinP::DoFolderPUAct(Widget, MainWinP *This, XtPointer)
{
   if ( !This->pub->popupOnSelected && This->pub->popupFolder )
      OpenFolder(This->pub->popupFolder->icon, This);
}

/*---------------------------------------------------------------
 *  Callback to handle folder-save (remove deleted messages)
 */

void
MainWinP::DoFolderSaveCur(Widget, MainWinP *This, XtPointer)
{
   ishApp->Broadcast("");
   This->SaveFolder(This->pub->curFolder);
}

void
MainWinP::DoFolderPUSave(Widget, MainWinP *This, XtPointer)
{
   if ( This->pub->popupOnSelected ) {
      DoFolderSaveSel(NULL, This, NULL);
      return;
   }

   if ( !This->pub->popupFolder ) return;

   This->pub->BusyCursor(True);
   ishApp->Broadcast("");
   This->SaveFolder(This->pub->popupFolder);
   This->pub->BusyCursor(False);

} // End DoFolderPUSave

/*---------------------------------------------------------------
 *  Callback to handle folder-save-selected (remove deleted messages)
 */

void
MainWinP::DoFolderSaveSel(Widget, MainWinP *This, XtPointer)
{
   This->pub->BusyCursor(True);
   ishApp->Broadcast("");

//
// Save folders
//
   VItemListC&	slist = This->folderVBox->SelItems();
   FolderListC&	flist = ishApp->folderPrefs->OpenFolders();
   u_int	count = flist.size();
   for (int i=0; i<count; i++) {
      FolderC	*fp = flist[i];
      if ( fp->Changed() && slist.includes(fp->icon) ) This->SaveFolder(fp);
   }

   This->pub->BusyCursor(False);

} // End DoFolderSaveSel

/*---------------------------------------------------------------
 *  Callback to handle folder-save-all (remove deleted messages)
 */

void
MainWinP::DoFolderSaveAll(Widget, MainWinP *This, XtPointer)
{

   ishApp->Broadcast("");

//
// Save the in-box
//
   Boolean	changed = (ishApp->systemFolder->writable &&
			   ishApp->systemFolder->Changed());
   if ( changed ) This->SaveFolder(ishApp->systemFolder);

//
// Loop through the user folders
//
   FolderListC&	list  =	ishApp->folderPrefs->OpenFolders();
   unsigned	count = list.size();
   for (int i=0; i<count; i++) {

      FolderC	*folder = list[i];
      changed = (folder->writable && folder->Changed());
      if ( changed ) This->SaveFolder(folder);
   }

} // End DoFolderSaveAll

/*---------------------------------------------------------------
 *  Method to save an individual folder
 */

void
MainWinP::SaveFolder(FolderC *folder)
{
   pub->BusyCursor(True);

   StringC	errmsg;

//
// Close any reading window open to deleted messages
//

//
// See if we're archiving read messages
//
   if ( folder->isInBox && ishApp->appPrefs->archiveOnSave ) {

      errmsg = "Archiving read messages to ";
      errmsg += ishApp->appPrefs->ArchiveFolder();
      ishApp->Broadcast(errmsg);

      FolderC* archiveFolder =
	      ishApp->folderPrefs->GetFolder(ishApp->appPrefs->ArchiveFolder(),
					     True/*ok to create*/);
      if ( !archiveFolder ) {
	 StringC	errmsg("Could not create archive folder: ");
	 errmsg += ishApp->appPrefs->ArchiveFolder();
	 errmsg += ".\n";
	 if ( errno != 0 ) errmsg += SystemErrorMessage(errno);
	 pub->PopupMessage(errmsg);
      }

      else {

	 VItemListC&	visList = folder->MsgItemList();
	 u_int		count   = visList.size();
	 VItemListC	delList;
	 for (int i=count-1; i>=0; i--) {

	    MsgItemC	*item = (MsgItemC *)visList[i];
	    MsgC	*msg  = item->msg;

	    if ( msg->IsRead() && archiveFolder->AddMessage(msg) ) {
	       delList.add((VItemC*)item);
	       msg->SetDeleted();
	    }
	 }
      }

   } // End if archiving read messages

//
// Close any reading window open to deleted messages
//
   if ( folder->active && ishApp->xRunning ) {

      unsigned	count = ishApp->readWinList.size();
      for (int i=0; i<count; i++) {
	 ReadWinC	*readWin = (ReadWinC*)*ishApp->readWinList[i];
	 if ( readWin->IsShown() && readWin->msg->IsDeleted() )
	    readWin->Hide();
      }
   }

   if ( folder->isInBox )
      errmsg = "In-box ";
   else {
      errmsg = "Folder \"";
      errmsg += folder->abbrev;
      errmsg += "\" ";
   }

   if ( folder->Save() ) {

      lastDelList->removeAll();
      if ( ishApp->xRunning ) {
	 pub->EnableButtons();
	 pub->UpdateTitle();
      }

      errmsg += "updated";

      if ( folder->active && ishApp->xRunning ) {

//
// Update message numbers in reading windows
//
	 unsigned	count = ishApp->readWinList.size();
	 for (int i=0; i<count; i++) {
	    ReadWinC	*readWin = (ReadWinC*)*ishApp->readWinList[i];
	    if ( readWin->IsShown() )
	       readWin->SetMessageNumber(readWin->msg->Number());
	 }
      }

   } // End if folder saved

   else {
      errmsg += "NOT saved";
   }

   if ( ishApp->xRunning ) {
      if ( folder->active ) {
         msgVBox->Refresh();
      }
      VBoxC&	vbox = ishApp->mainWin->FolderVBox();
      vbox.Refresh();
   }

   ishApp->Broadcast(errmsg);
   pub->BusyCursor(False);

} // End SaveFolder

/*---------------------------------------------------------------
 *  Callback to handle folder-read
 */

void
MainWinP::DoFolderReadCur(Widget, MainWinP *This, XtPointer)
{
   ishApp->Broadcast("");
   This->pub->curFolder->Rescan();
}

void
MainWinP::DoFolderPURead(Widget, MainWinP *This, XtPointer)
{
   if ( This->pub->popupOnSelected ) {
      DoFolderReadSel(NULL, This, NULL);
      return;
   }

   if ( !This->pub->popupFolder ) return;

   This->pub->BusyCursor(True);
   ishApp->Broadcast("");
   if ( This->pub->popupFolder->IsImap() ) {
      ImapFolderC *If = (ImapFolderC*) This->pub->popupFolder;
      if ( !If->server || !If->server->authenticated ) {
         If->server->EstablishSession();
      }
   }
   This->pub->popupFolder->Rescan();
   This->pub->BusyCursor(False);

} // End DoFolderPURead

/*---------------------------------------------------------------
 *  Callback to handle folder-read-selected
 */

void
MainWinP::DoFolderReadSel(Widget, MainWinP *This, XtPointer)
{
   This->pub->BusyCursor(True);
   ishApp->Broadcast("");

//
// Read folders
//
   VItemListC&	slist = This->folderVBox->SelItems();
   FolderListC&	flist = ishApp->folderPrefs->OpenFolders();
   unsigned	count = flist.size();
   for (int i=0; i<count; i++) {

      FolderC	*fp = flist[i];
      if ( slist.includes(fp->icon) ) fp->Rescan();

   } // End for each user folder

   This->pub->BusyCursor(False);

} // End DoFolderReadSel

/*---------------------------------------------------------------
 *  Callback to handle folder-read-all
 */

void
MainWinP::DoFolderReadAll(Widget, MainWinP*, XtPointer)
{
   u_int	count;
   int		i;

   ishApp->Broadcast("");

//
// Read the in-box
//
   ishApp->systemFolder->Rescan();

//
// Loop through the user folders
//
   count = ishApp->folderPrefs->OpenFolders().size();
   for (i=0; i<count; i++) {

      FolderC	*folder = ishApp->folderPrefs->OpenFolders()[i];
      folder->Rescan();
   }

} // End DoFolderReadAll

/*---------------------------------------------------------------
 *  Callback to handle folder-close-current
 */

void
MainWinP::DoFolderCloseCur(Widget, MainWinP *This, XtPointer)
{
   FolderListC	folders;
   folders.append(This->pub->curFolder);

//
// Check for saving changes
//
   switch ( This->SaveQuery(&folders) ) {

      case (QUERY_YES):
	 if ( !This->pub->curFolder->Save() ) return;
	 break;

      case (QUERY_NO):
	 break;

      case (QUERY_CANCEL):
      case (QUERY_NONE):
	 return;
   }

   This->pub->BusyCursor(True);

//
// Close any open reading windows
//
   unsigned	count = ishApp->readWinList.size();
   for (int i=0; i<count; i++) {
      ReadWinC	*readWin = (ReadWinC*)*ishApp->readWinList[i];
      readWin->Hide();
   }

//
// Delete folder
//
   ishApp->folderPrefs->RemoveFolder(This->pub->curFolder);
   delete This->pub->curFolder;
   This->pub->curFolder = NULL;

//
// Switch to the in-box
//
   This->pub->ActivateSystemFolder();

   This->folderVBox->Refresh();
   This->pub->BusyCursor(False);

} // End DoFolderCloseCur

/*---------------------------------------------------------------
 *  Callback to handle folder-close-popup
 */

void
MainWinP::DoFolderPUClose(Widget, MainWinP *This, XtPointer)
{
   if ( !This->pub->popupOnSelected && !This->pub->popupFolder ) return;

//
// Build list of folders
//
   FolderListC	folders;

   if ( This->pub->popupOnSelected ) {
      VItemListC&	slist = This->folderVBox->SelItems();
      FolderListC&	flist = ishApp->folderPrefs->OpenFolders();
      unsigned		count = flist.size();
      for (int i=0; i<count; i++) {
	 FolderC	*fp = flist[i];
	 if ( slist.includes(fp->icon) ) folders.add(fp);
      }
   }
   else
      folders.add(This->pub->popupFolder);

//
// Check for saving changes
//
   Boolean	saveChanges = True;
   switch ( This->SaveQuery(&folders) ) {

      case (QUERY_YES):
	 saveChanges = True;
	 break;

      case (QUERY_NO):
	 saveChanges = False;
	 break;

      case (QUERY_CANCEL):
      case (QUERY_NONE):
	 return;
   }

   This->pub->BusyCursor(True);

//
// If the current folder will be closed, close any open reading windows
//
   if ( folders.includes(This->pub->curFolder) ) {

      unsigned	count = ishApp->readWinList.size();
      for (int i=0; i<count; i++) {
	 ReadWinC	*readWin = (ReadWinC*)*ishApp->readWinList[i];
	 readWin->Hide();
      }

      This->pub->curFolder = NULL;
   }

//
// Delete folder(s)
//
   unsigned	count = folders.size();
   for (int i=0; i<count; i++) {

      FolderC	*fp = folders[i];
      ishApp->folderPrefs->RemoveFolder(fp);

      if ( saveChanges && fp->writable && fp->Changed() )
	 fp->Save();

      delete fp;
   }

//
// Reset variables
//
   This->pub->popupFolder = NULL;
   This->pub->curMsgList->removeAll();

//
// If the current folder was closed, switch to the in-box.
//
   if ( !This->pub->curFolder ) 
      This->pub->ActivateSystemFolder();

   This->folderVBox->Refresh();
   This->pub->BusyCursor(False);

} // End DoFolderPUClose

/*---------------------------------------------------------------
 *  Callback to handle folder-close-selected
 */

void
MainWinP::DoFolderCloseSel(Widget, MainWinP *This, XtPointer)
{
   VItemListC	list = This->folderVBox->SelItems();

//
// Build list of folders
//
   FolderListC	folders;
   FolderListC&	flist = ishApp->folderPrefs->OpenFolders();
   unsigned	count = flist.size();
   int	i;
   for (i=count-1; i>=0; i--) {
      FolderC	*folder = flist[i];
      if ( list.includes(folder->icon) ) folders.append(folder);
   }

//
// Check for saving changes
//
   Boolean	saveChanges = True;
   switch ( This->SaveQuery(&folders) ) {

      case (QUERY_YES):
	 saveChanges = True;
	 break;

      case (QUERY_NO):
	 saveChanges = False;
	 break;

      case (QUERY_CANCEL):
      case (QUERY_NONE):
	 return;
   }

   This->pub->BusyCursor(True);

//
// If the current folder will be closed, close any open reading windows
//
   if ( folders.includes(This->pub->curFolder) ) {

      unsigned	count = ishApp->readWinList.size();
      for (i=0; i<count; i++) {
	 ReadWinC	*readWin = (ReadWinC*)*ishApp->readWinList[i];
	 readWin->Hide();
      }

      This->pub->curFolder = NULL;
   }

//
// Delete requested folders
//
   count = folders.size();
   for (i=0; i<count; i++) {

      FolderC	*fp = folders[i];
      ishApp->folderPrefs->RemoveFolder(fp);

      if ( saveChanges && fp->writable && fp->Changed() )
	 fp->Save();

      delete fp;
   }

//
// If the current folder was closed, switch to the in-box.
//
   if ( !This->pub->curFolder )
      This->pub->ActivateSystemFolder();

   This->folderVBox->Refresh();
   This->pub->BusyCursor(False);

} // End DoFolderCloseSel

/*---------------------------------------------------------------
 *  Callback to handle folder-close-all
 */

void
MainWinP::DoFolderCloseAll(Widget, MainWinP *This, XtPointer)
{
//
// Check for saving changes
//
   Boolean	saveChanges = True;
   switch ( This->SaveQuery(&ishApp->folderPrefs->OpenFolders()) ) {

      case (QUERY_YES):
	 saveChanges = True;
	 break;

      case (QUERY_NO):
	 saveChanges = False;
	 break;

      case (QUERY_CANCEL):
      case (QUERY_NONE):
	 return;
   }

   This->pub->BusyCursor(True);

   FolderListC&	flist =	ishApp->folderPrefs->OpenFolders();

//
// If the current folder will be closed, close any open reading windows
//
   if ( flist.includes(This->pub->curFolder) ) {

      unsigned	count = ishApp->readWinList.size();
      for (int i=0; i<count; i++) {
	 ReadWinC	*readWin = (ReadWinC*)*ishApp->readWinList[i];
	 readWin->Hide();
      }

      This->pub->curFolder = NULL;
   }

//
// Close requested folders
//
   u_int	count = flist.size();
   for (int i=count-1; i>=0; i--) {

      FolderC	*fp = flist[i];
      ishApp->folderPrefs->RemoveFolder(fp);

      if ( saveChanges && fp->writable && fp->Changed() )
	 fp->Save();

      delete fp;
   }

//
// If the current folder was closed, switch to the in-box.
//
   if ( !This->pub->curFolder )
      This->pub->ActivateSystemFolder();

   This->folderVBox->Refresh();
   This->pub->BusyCursor(False);

} // End DoFolderCloseAll

/*---------------------------------------------------------------
 *  Callback to handle folder-delete
 */

void
MainWinP::DoFolderDelCur(Widget, MainWinP *This, XtPointer)
{
   ishApp->Broadcast("");
   This->delFolderList->removeAll();
   This->delFolderList->add(This->pub->curFolder);
   This->DeleteFolders();
}

void
MainWinP::DoFolderPUDel(Widget, MainWinP *This, XtPointer)
{
   if ( This->pub->popupOnSelected ) {
      DoFolderDelSel(NULL, This, NULL);
      return;
   }

   if ( !This->pub->popupFolder ) return;

   This->pub->BusyCursor(True);
   ishApp->Broadcast("");

   This->delFolderList->removeAll();
   This->delFolderList->add(This->pub->popupFolder);
   This->DeleteFolders();

   This->pub->BusyCursor(False);

} // End DoFolderPUDel

/*---------------------------------------------------------------
 *  Callback to handle folder-delete-selected
 */

void
MainWinP::DoFolderDelSel(Widget, MainWinP *This, XtPointer)
{
   This->pub->BusyCursor(True);
   ishApp->Broadcast("");
   This->delFolderList->removeAll();

//
// Delete folder or folders
//
   VItemListC&	slist = This->folderVBox->SelItems();
   FolderListC&	flist = ishApp->folderPrefs->OpenFolders();
   unsigned	count = flist.size();
   for (int i=0; i<count; i++) {
      FolderC	*fp = flist[i];
      if ( slist.includes(fp->icon) ) This->delFolderList->add(fp);
   }

   This->DeleteFolders();
   This->pub->BusyCursor(False);

} // End DoFolderDelSel

/*---------------------------------------------------------------
 *  Callback to handle folder-delete-all
 */

void
MainWinP::DoFolderDelAll(Widget, MainWinP *This, XtPointer)
{
   ishApp->Broadcast("");
   *This->delFolderList = ishApp->folderPrefs->OpenFolders();
   This->DeleteFolders();

} // End DoFolderDelAll

/*---------------------------------------------------------------
 *  Method to confirm folder deletion
 */

void
MainWinP::DeleteFolders()
{
   if ( !ishApp->confPrefs->confirmDeleteFolder ) {
      FinishFolderDel(NULL, this, NULL);
      return;
   }

//
// Build verification dialog if necessary
//
   static Widget	delWin = NULL;
   if ( !delWin ) {

      WArgList	args;
      args.DialogStyle(XmDIALOG_FULL_APPLICATION_MODAL);
      if ( ishApp->questionPM ) args.SymbolPixmap(ishApp->questionPM);
      delWin = XmCreateQuestionDialog(*pub, "deleteFolderWin", ARGS);

      XtUnmanageChild(XmMessageBoxGetChild(delWin, XmDIALOG_HELP_BUTTON));
      XtAddCallback(delWin, XmNokCallback, (XtCallbackProc)FinishFolderDel,
      		    this);

   } // End if delete window not yet created

//
// Prepare message
//
   StringC msg = get_string(delWin, "messageString",
				    "Really delete these folders:\n\n");

//
// Add folder names to the message
//
   while ( !msg.EndsWith("\n\n") ) msg += '\n';
   u_int	count = delFolderList->size();
   for (int i=0; i<count; i++) {

      FolderC	*folder = (*delFolderList)[i];
      if ( folder->isInBox ) continue;

      if ( i > 0 ) msg += '\n';
      msg += folder->abbrev;
   }

   WXmString	wstr(msg);
   XtVaSetValues(delWin, XmNmessageString, (XmString)wstr, NULL);

   XtManageChild(delWin);
   XMapRaised(ishApp->display, XtWindow(XtParent(delWin)));

} // End DeleteFolders

/*---------------------------------------------------------------
 *  Callback to delete the folders in delFolderList
 */

void
MainWinP::FinishFolderDel(Widget, MainWinP *This, XtPointer)
{
   This->pub->BusyCursor(True);
   ishApp->Broadcast("");

//
// If the current folder will be closed, close any open reading windows
//
   if ( This->delFolderList->includes(This->pub->curFolder) ) {

      unsigned	count = ishApp->readWinList.size();
      for (int i=0; i<count; i++) {
	 ReadWinC	*readWin = (ReadWinC*)*ishApp->readWinList[i];
	 readWin->Hide();
      }

      This->pub->curFolder = NULL;
   }

//
// Loop through folders and delete them
//
   u_int	count = This->delFolderList->size();
   StringC	statMsg;
   for (int i=0; i<count; i++) {

      FolderC	*folder = (*This->delFolderList)[i];
      statMsg = folder->abbrev;

//
// Remove folder from user folder list
//
      ishApp->folderPrefs->RemoveFolder(folder);

//
// Delete folder
//
      folder->delFiles = True;
      delete folder;

      statMsg += " deleted";
      ishApp->Broadcast(statMsg);

   } // End for each folder

   This->delFolderList->removeAll();

//
// If the current folder was closed, switch to the in-box.
//
   if ( !This->pub->curFolder )
      This->pub->ActivateSystemFolder();

   This->folderVBox->Refresh();
   This->pub->BusyCursor(False);

} // End FinishFolderDel

/*---------------------------------------------------------------
 *  Callback to handle folder-select all
 */

void
MainWinP::DoFolderSel(Widget, MainWinP *This, XtPointer)
{
   This->pub->BusyCursor(True);
   This->folderVBox->SelectItemsOnly(This->folderVBox->VisItems());
   This->pub->BusyCursor(False);
}

/*---------------------------------------------------------------
 *  Callback to handle folder-deselect all
 */

void
MainWinP::DoFolderDesel(Widget, MainWinP *This, XtPointer)
{
   This->pub->BusyCursor(True);
   This->folderVBox->DeselectAllItems();
   This->pub->BusyCursor(False);
}

/*---------------------------------------------------------------
 *  Callback to handle folder-new
 */

void
MainWinP::DoFolderNew(Widget, MainWinP *This, XtPointer)
{
//
// Make sure directory is accessible
//
   StringC	dir = ishApp->appPrefs->FolderDir();
   if ((!ishApp->appPrefs->usingImap) || (ishApp->folderPrefs->UsingLocal())) {
      if ( access(dir, R_OK) != 0 ) {
	 dir = ishApp->home;
	 if ( access(dir, R_OK) != 0 )
	    dir = ".";
      }
   }

   if ( !This->newFolderWin ) {

      FileChooserWinC	*win = new FileChooserWinC(*This->pub, "newFolderWin");

      win->SingleSelect(True);
      win->AddOkCallback((CallbackFn*)FinishFolderNew, This);
      win->HideList();

      This->newFolderWin = win;

   } // End if window needs to be created

    if ((!ishApp->appPrefs->usingImap) || (!ishApp->folderPrefs->UsingLocal())) {
	This->newFolderWin->SetDirectory(dir);
	This->newFolderWin->SetDefaultDir(ishApp->appPrefs->FolderDir());
    }

    // if
    else {
	This->newFolderWin->SetDirectory("");
	This->newFolderWin->SetDefaultDir("");
    }

   This->newFolderWin->ShowDirsInFileList (ishApp->folderPrefs->usingMh);
   This->newFolderWin->ShowFilesInFileList(ishApp->folderPrefs->usingUnix ||
					   ishApp->folderPrefs->usingMmdf);
   This->newFolderWin->Show(*This->pub);

} // End DoFolderNew

/*---------------------------------------------------------------
 *  Callback routine to handle press of Ok in file selection box for new
 */

void
MainWinP::FinishFolderNew(StringListC *list, MainWinP *This)
{
   StringC	name = *(*list)[0];
   if ( name.size() == 0 ) {
      StringC	msg = "Please enter a folder name.";
      set_invalid(This->newFolderWin->SelectTF(), True, True);
      This->pub->PopupMessage(msg);
      This->newFolderWin->HideOk(False);
      return;
   }

//
// Don't allow spaces in the name
//
   if ( name.Contains(' ') || name.Contains('\t') ) {
      StringC	msg = "Spaces are not allowed in folder names.";
      set_invalid(This->newFolderWin->SelectTF(), True, True);
      This->pub->PopupMessage(msg);
      This->newFolderWin->HideOk(False);
      return;
   }

   ishApp->ExpandFolderName(name);

   if ( debuglev > 0 ) cout <<"Trying to create folder: " <<name <<endl;

//
// See if the folder already exists
//
   FolderTypeT	type = ishApp->folderPrefs->FolderType(name);
   if ( type != UNKNOWN_FOLDER && type != IMAP_FOLDER ) {
      StringC	msg = name;
      msg += " already exists";
      This->pub->PopupMessage(msg);
      This->newFolderWin->HideOk(False);
      return;
   }

   FolderC *newFolder = ishApp->folderPrefs->GetFolder(name, /*createOk=*/True);
   if ( newFolder ) {
      ishApp->folderPrefs->AddOpenFolder(newFolder);
      This->pub->ShowFolder(newFolder);
      ishApp->appPrefs->AddRecentFolder(newFolder->name);
   }
   else {
      StringC	errmsg("Could not create folder: ");
      errmsg += name;
      errmsg += ".\n";
      if ( errno != 0 ) errmsg += SystemErrorMessage(errno);
      This->pub->PopupMessage(errmsg);
   }

   This->pub->BusyCursor(False);

} // End FinishFolderNew

/*-----------------------------------------------------------------------
 *  Handle resize of extra frame in open folders dialog
 */

static void
HandleOpenExpose(Widget w, XtPointer, XEvent *ev, Boolean*)
{
   if ( ev->type != MapNotify ) return;

//
// Fix sizes of pane
//
   Dimension	ht;
   XtVaGetValues(w, XmNheight, &ht, NULL);
   XtVaSetValues(w, XmNpaneMinimum, ht, XmNpaneMaximum, ht, 0);
}

/*---------------------------------------------------------------
 *  Callback to handle folder-open
 */

void
MainWinP::DoFolderOpen(Widget, MainWinP *This, XtPointer)
{
   StringC	dir = ishApp->appPrefs->FolderDir();
   if ( !IsImapName(dir) ) {

//
// Make sure directory is accessible
//
      if ( access(dir, R_OK) != 0 ) {

	 int	err = errno;
	 StringC	msg("Could not open folder diretory: ");
	 msg += dir + ".\n" + SystemErrorMessage(err);
	 This->pub->PopupMessage(msg);

	 dir = ishApp->home;
	 if ( access(dir, R_OK) != 0 ) {
	    err = errno;
	    msg = "Could not open folder directory: " + dir
		+ ".\n" + SystemErrorMessage(err);
	    This->pub->PopupMessage(msg);
	    return;
	 }
      }

   } // End if not an IMAP directory

   if ( !This->openSelectWin ) {

      This->openSelectWin = new FileChooserWinC(*This->pub, "openSelectWin");

//
// Add pane for activation.  Put it in the last position.
//
      Cardinal	childCount;
      XtVaGetValues(This->openSelectWin->PanedWin(), XmNnumChildren,
      		    &childCount, NULL);

      WArgList	args;
      args.PositionIndex(childCount+1);
      args.ShadowThickness(0);
      Widget centerFrame =
	 XmCreateFrame(This->openSelectWin->PanedWin(), "centerFrame", ARGS);

      args.Reset();
      args.ChildType(XmFRAME_TITLE_CHILD);
      args.ChildHorizontalAlignment(XmALIGNMENT_CENTER);
      Widget frame = XmCreateFrame(centerFrame, "activateFrame", ARGS);

      args.Reset();
      args.Orientation(XmHORIZONTAL);
      args.Packing(XmPACK_TIGHT);
      Widget radio = XmCreateRadioBox(frame, "activateRadio", ARGS);

      This->openOnlyTB = XmCreateToggleButton(radio, "openOnlyTB", 0,0);
      This->openActTB  = XmCreateToggleButton(radio, "openActTB",  0,0);

      XtManageChild(This->openOnlyTB);
      XtManageChild(This->openActTB);
      XtManageChild(radio);	// frame children
      XtManageChild(frame);
      XtManageChild(centerFrame);

//
// Add a callback so we can fix the size of the pane we just added
//
      XtAddEventHandler(centerFrame, StructureNotifyMask, False,
			HandleOpenExpose, NULL);

      XmToggleButtonSetState(This->openActTB, True, True);

      This->openSelectWin->HandleHelp(This->openOnlyTB);
      This->openSelectWin->HandleHelp(This->openActTB);

      This->openSelectWin->SingleSelect(False);
      This->openSelectWin->AddVerifyCallback((CallbackFn*)VerifyOpenFiles,This);
      This->openSelectWin->AddOkCallback    ((CallbackFn*)FinishOpen,     This);
      This->openSelectWin->HideList();

   } // End if window not yet created

    if ( (!ishApp->appPrefs->usingImap) ||
	(!ishApp->folderPrefs->UsingLocal()) ) {
	This->openSelectWin->SetDirectory(dir);
	This->openSelectWin->SetDefaultDir(ishApp->appPrefs->FolderDir());
    }
    else {
	This->openSelectWin->SetDirectory("");
	This->openSelectWin->SetDefaultDir("");
    }

   This->openSelectWin->ShowDirsInFileList (ishApp->folderPrefs->usingMh);
   This->openSelectWin->ShowFilesInFileList(ishApp->folderPrefs->usingUnix ||
   					    ishApp->folderPrefs->usingMmdf);
   This->openSelectWin->Show(*This->pub);

} // End DoFolderOpen

/*---------------------------------------------------------------
 *  Callback routine to double check file list in file chooser
 */

void
MainWinP::VerifyOpenFiles(StringListC *list, MainWinP *This)
{
   FileChooserWinC	*fileWin = This->openSelectWin;
   StringC		dname = fileWin->Directory();

   if ( !fileWin->UsingImap() ) {

//
// Set directory to one in file chooser
//
      chdir(dname);

//
// Remove lock files and unreadable files
//
      u_int	count = list->size();
      for (int i=count-1; i>=0; i--) {
	 StringC	*name = (*list)[i];
	 if ( name->EndsWith(".lock") || access(*name, R_OK) != 0 )
	    list->remove(i);
      }

      chdir(ishApp->startupDir);

   } // End if not using IMAP

//
// Remove folders that are already open
//
   if ( !dname.EndsWith("/") ) dname += "/";

   u_int	count = ishApp->folderPrefs->OpenFolders().size();
   StringC	fname;
   for (int i=0; i<count; i++) {

      FolderC	*folder = ishApp->folderPrefs->OpenFolders()[i];
      CharC	name    = folder->name;

//
// See if folder is in same directory as file chooser
//
      if ( name.StartsWith(dname) ) {
	 name.CutBeg(dname.size());
	 fname = name;
	 int	index = list->indexOf(fname);
	 if ( index != list->NULL_INDEX ) list->remove(index);
      }
   }

} // End VerifyOpenFiles

/*---------------------------------------------------------------
 *  Callback routine to handle press of Ok in file selection box for open
 */

void
MainWinP::FinishOpen(StringListC *list, MainWinP *This)
{
   This->pub->BusyCursor(True);

//
// Temporarily postpone changes to the recent folders menu
//
   This->deferRecentMenu = True;

   This->openSelectWin->HideOk(True);

   StringC	name;
   Boolean	needAct = XmToggleButtonGetState(This->openActTB);
   unsigned	count   = list->size();
   for (int i=0; i<count; i++) {

      StringC	*name = (*list)[i];
      FolderC	*folder = ishApp->folderPrefs->GetFolder(*name,
							 /*createOk=*/False);
      if ( folder ) {

	 This->pub->ShowFolder(folder);
	 ishApp->appPrefs->AddRecentFolder(folder->name);

	 if ( needAct && folder->opened ) {

//
// Close any unpinned reading windows
//
	    count = ishApp->readWinList.size();
	    for (i=0; i<count; i++) {
	       ReadWinC	*readWin = (ReadWinC*)*ishApp->readWinList[i];
	       if ( !readWin->Pinned() ) readWin->Hide();
	    }

//
// Switch folders
//
	    This->pub->curFolder->Deactivate();
	    folder->Activate();

	    if ( folder->active ) {
	       This->pub->SetCurrentFolder(folder);
	       needAct = False;
	    }
	    else {
	       folder->Deactivate();
	       This->pub->curFolder->Activate();
	    }

	 } // End if we need to activate a folder

      } // End if folder opened

      else {

	 This->openSelectWin->HideOk(False);

	 StringC	errmsg("Could not open folder: ");
	 errmsg += *name;
	 errmsg += ".\n";
	 if ( errno != 0 ) errmsg += SystemErrorMessage(errno);
	 This->pub->PopupMessage(errmsg);
      }

   } // End for each name in list

   This->folderVBox->Refresh();
   This->msgVBox->Refresh();
   This->pub->BusyCursor(False);

//
// Update recent folder menus now
//
   This->deferRecentMenu = False;

   if ( ishApp->appPrefs->recentFolderTime > This->recentMenuTime )
      This->BuildRecentFolderMenus();

} // End FinishOpen

