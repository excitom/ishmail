/*
 *  $Id: ReadFile.C,v 1.4 2000/09/19 16:42:02 evgeny Exp $
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
#include "ReadWinP.h"
#include "ReadWinC.h"
#include "IshAppC.h"
#include "MainWinC.h"
#include "FolderC.h"
#include "MsgC.h"
#include "MsgItemC.h"
#include "AppPrefC.h"
#include "QuickMenu.h"
#include "SaveMgrC.h"
#include "FolderPrefC.h"
#include "PrintWinC.h"
#include "PipeWinC.h"
#include "sun2mime.h"
#include "FileMsgC.h"
#include "MsgPartC.h"
#include "Misc.h"
#include "SafeSystem.h"
#include "SendWinC.h"
#include "Fork.h"

#include <hgl/VBoxC.h>
#include <hgl/MimeRichTextC.h>
#include <hgl/WArgList.h>
#include <hgl/WXmString.h>

#include <Xm/PushB.h>

#include <sys/stat.h>
#include <unistd.h>

/*---------------------------------------------------------------
 *  Callback to handle close from menu
 */

void
ReadWinP::DoClose(Widget, ReadWinP *This, XtPointer)
{
   This->pub->Hide();
   ishApp->mainWin->MsgVBox().Refresh();
}

/*---------------------------------------------------------------
 *  Callback to handle close from window manager
 */

void
ReadWinP::DoHide(ReadWinC *This, void*)
{
   This->SetMessage((MsgC*)NULL);
   ishApp->mainWin->MsgVBox().Refresh();
}

/*---------------------------------------------------------------
 *  Callback to read next message
 */

void
ReadWinP::DoNext(Widget, ReadWinP *This, XtPointer)
{
   if ( This->pub->fullFunction ) {
      MsgC	*nextMsg = ishApp->mainWin->NextReadable(This->pub->msg);
      ishApp->DisplayMessage(nextMsg, This->pub->Pinned() ? (ReadWinC*)NULL
      							  : This->pub);
   }

   else
      This->pub->parentWin->priv->ShowNext822(This->pub);
}

/*---------------------------------------------------------------
 *  Callback to read next unread message
 */

void
ReadWinP::DoNextUnread(Widget, ReadWinP *This, XtPointer)
{
   MsgC	*nextMsg = ishApp->mainWin->NextUnread(This->pub->msg);
   ishApp->DisplayMessage(nextMsg, This->pub->Pinned() ? (ReadWinC*)NULL
						       : This->pub);
}

/*---------------------------------------------------------------
 *  Callback to read next message with same sender
 */

void
ReadWinP::DoNextSender(Widget, ReadWinP *This, XtPointer)
{
   MsgC	*nextMsg = ishApp->mainWin->NextSender(This->pub->msg);
   ishApp->DisplayMessage(nextMsg, This->pub->Pinned() ? (ReadWinC*)NULL
   						       : This->pub);
}

/*---------------------------------------------------------------
 *  Callback to read next message with same subject
 */

void
ReadWinP::DoNextSubject(Widget, ReadWinP *This, XtPointer)
{
   MsgC	*nextMsg = ishApp->mainWin->NextSubject(This->pub->msg);
   ishApp->DisplayMessage(nextMsg, This->pub->Pinned() ? (ReadWinC*)NULL
   						       : This->pub);
}

/*---------------------------------------------------------------
 *  Callback to read previous message
 */

void
ReadWinP::DoPrev(Widget, ReadWinP *This, XtPointer)
{
   if ( This->pub->fullFunction ) {
      MsgC	*prevMsg = ishApp->mainWin->PrevReadable(This->pub->msg);
      ishApp->DisplayMessage(prevMsg, This->pub->Pinned() ? (ReadWinC*)NULL
							  : This->pub);
   }

   else
      This->pub->parentWin->priv->ShowPrev822(This->pub);
}

/*---------------------------------------------------------------
 *  Callback to read previous unread message
 */

void
ReadWinP::DoPrevUnread(Widget, ReadWinP *This, XtPointer)
{
   MsgC	*prevMsg = ishApp->mainWin->PrevUnread(This->pub->msg);
   ishApp->DisplayMessage(prevMsg, This->pub->Pinned() ? (ReadWinC*)NULL
   						       : This->pub);
}

/*---------------------------------------------------------------
 *  Callback to read previous message from same sender
 */

void
ReadWinP::DoPrevSender(Widget, ReadWinP *This, XtPointer)
{
   MsgC	*prevMsg = ishApp->mainWin->PrevSender(This->pub->msg);
   ishApp->DisplayMessage(prevMsg, This->pub->Pinned() ? (ReadWinC*)NULL
   						       : This->pub);
}

/*---------------------------------------------------------------
 *  Callback to read previous message with same subject
 */

void
ReadWinP::DoPrevSubject(Widget, ReadWinP *This, XtPointer)
{
   MsgC	*prevMsg = ishApp->mainWin->PrevSubject(This->pub->msg);
   ishApp->DisplayMessage(prevMsg, This->pub->Pinned() ? (ReadWinC*)NULL
   						       : This->pub);
}

/*---------------------------------------------------------------
 *  Callback to handle display of a recent folders menu
 */

void
ReadWinP::RecentListChanged(void*, ReadWinP *This)
{
//
// Update the menu only if this window is displayed
//
   if ( This->pub->IsShown() )
      This->BuildRecentFolderMenu();
}

/*------------------------------------------------------------------------
 * Method to build the menu of recent folders.
 */

void
ReadWinP::BuildRecentFolderMenu()
{
//
// Unmanage existing buttons
//
   WidgetList	wlist;
   Cardinal	wcount;
   XtVaGetValues(fileSaveRecentPD, XmNnumChildren, &wcount, XmNchildren, &wlist,
   		 NULL);
   //if ( wcount > 0 ) XtUnmanageChildren(wlist, wcount);

//
// Create new buttons
//
   WArgList	args;
   WXmString	wstr;
   u_int	count = ishApp->appPrefs->recentFolders.size();
   for (int i=0; i<count; i++) {

      StringC	*label = ishApp->appPrefs->recentFolders[i];
      wstr = (char*)*label;
      args.LabelString(wstr);

//
// Create a button in the menu bar message menu.  Re-use one if possible
//
      Widget	pb;
      if ( i < wcount ) {
	 pb = wlist[i];
	 XtSetValues(pb, ARGS);
      }
      else {
	 pb = XmCreatePushButton(fileSaveRecentPD, "recentPB", ARGS);
	 XtAddCallback(pb, XmNactivateCallback, (XtCallbackProc)DoSaveToButton,
		       this);
      }

      XtManageChild(pb);

   } // End for each button to be added

   EnableButtons();
   recentMenuTime = time(0);

} // End BuildRecentFolderMenu

/*---------------------------------------------------------------
 *  Callback to handle display of message->save quick pulldown
 */

void
ReadWinP::PrepareQuickMenu(Widget cb, ReadWinP *This, XtPointer)
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
      BuildQuickFolderMenu(data, (XtCallbackProc)DoSaveToButton,
      			   (XtCallbackProc)PrepareQuickMenu, This);

} // End PrepareQuickMenu

/*---------------------------------------------------------------
 *  Callback to save message
 */

void
ReadWinP::DoSave(Widget, ReadWinP *This, XtPointer)
{
   ishApp->saveMgr->SaveMsgToFolder(This->pub->msg,
		     ishApp->saveMgr->curSaveFolder,
		     This->pub->fullFunction && ishApp->appPrefs->deleteSaved);
}

/*---------------------------------------------------------------
 *  Callback to handle file-save-to-selected
 */

void
ReadWinP::DoSaveSel(Widget, ReadWinP *This, XtPointer)
{
//
// Loop through folders
//
   Boolean	del    = False;	// Don't delete until last folder
   VItemListC&	slist  = ishApp->mainWin->FolderVBox().SelItems();
   u_int	scount = slist.size();
   for (int s=0; s<scount; s++) {

      if ( s == scount-1 )
	 del = This->pub->fullFunction && ishApp->appPrefs->deleteSaved;
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
	    ishApp->saveMgr->SaveMsgToFolder(This->pub->msg, folder, del);
	    saved = True;
	 }

      } // End for each open folder

   } // End for each selected folder

} // End DoSaveSel

/*---------------------------------------------------------------
 *  Callback to save message to folder
 */

void
ReadWinP::DoSaveTo(Widget, ReadWinP *This, XtPointer)
{
   ishApp->saveMgr->SaveMsgToFolder(This->pub->msg, *This->pub,
   				    This->pub->msg->folder &&
				    This->pub->msg->folder->writable);
}

/*---------------------------------------------------------------
 *  Callback to save message to file
 */

void
ReadWinP::DoSaveToFile(Widget, ReadWinP *This, XtPointer)
{
   ishApp->saveMgr->SaveMsgToFile(This->pub->msg, *This->pub,
				  This->pub->msg->folder &&
				  This->pub->msg->folder->writable);
}

/*---------------------------------------------------------------
 *  Callback to save message to the file named on the button that was
 *     pressed.  Used by Save->Recent and Save->Quick
 */

void
ReadWinP::DoSaveToButton(Widget w, ReadWinP *This, XtPointer)
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

   ishApp->saveMgr->SaveMsgToFolder(This->pub->msg, name,
		     This->pub->fullFunction && ishApp->appPrefs->deleteSaved);

   XtFree(cs);
   XmStringFree(xstr);

} // End DoSaveToButton

/*---------------------------------------------------------------
 *  Callback to delete message
 */

void
ReadWinP::DoDelete(Widget, ReadWinP *This, XtPointer)
{
   Boolean	pinned = This->pub->Pinned();
   This->pub->Unpin();

   ishApp->mainWin->DeleteMsg(This->pub->msg);

   if ( pinned ) This->pub->Pin();
}

/*---------------------------------------------------------------
 *  Callback to delete message and close window
 */

void
ReadWinP::DoDelClose(Widget, ReadWinP *This, XtPointer)
{
   MsgC	*msg = This->pub->msg;
   This->pub->Hide();
   ishApp->mainWin->DeleteMsg(msg);
}

/*---------------------------------------------------------------
 *  Callback to undelete message
 */

void
ReadWinP::DoUndelete(Widget, ReadWinP *This, XtPointer)
{
   ishApp->mainWin->UndeleteMsg(This->pub->msg);
}

/*---------------------------------------------------------------
 *  Callback to print message
 */

void
ReadWinP::DoPrint(Widget, ReadWinP *This, XtPointer)
{
   ishApp->mainWin->popupMsg = This->pub->msg;
   ishApp->mainWin->popupOnSelected = False;
   if ( !This->printWin ) This->printWin = new PrintWinC(*This->pub);
   This->printWin->HideOrder();
   This->printWin->Show(*This->pub);
}

/*---------------------------------------------------------------
 *  Callback to pipe message
 */

void
ReadWinP::DoPipe(Widget, ReadWinP *This, XtPointer)
{
   ishApp->mainWin->popupMsg = This->pub->msg;
   ishApp->mainWin->popupOnSelected = False;
   if ( !This->pipeWin ) This->pipeWin = new PipeWinC(*This->pub);
   This->pipeWin->HideOrder();
   This->pipeWin->Show(*This->pub);
}

/*---------------------------------------------------------------
 *  Callback to convert from Sun attachment format to MIME format
 */

void
ReadWinP::DoSunToMime(Widget, ReadWinP *This, XtPointer)
{
   This->pub->BusyCursor(True);

   char		*cs = tempnam(NULL, "mime.");
   StringC	newFile(cs);
   free(cs);

   StringC	bound;
   GenBoundary(bound);

   MsgC		*oldMsg = This->pub->msg;
   int		status;

   // If IMAP message, we have it in memory, It must first be written
   // to a temp file before sun2mime can handle it. For other folder
   // types, the message is already in a local file.

   if ( oldMsg->IsImap() ) {

      cs = tempnam(NULL, "imap.");
      StringC   msgFile(cs);
      free(cs);

      oldMsg->WriteFile(msgFile, True, True, False, False, False);
      status = sun2mime(msgFile, 0, 0 /* don't know size */, newFile, bound);
      unlink(msgFile);
   }
   else {
      MsgPartC  *bod = oldMsg->Body();
      status = sun2mime(bod->msgFile, bod->offset, bod->bytes, newFile, bound);
   }

   if ( status == 0 ) {

      int		len = 0;
      struct stat	stats;
      if ( stat(newFile, &stats) == 0 ) len = (int)stats.st_size;

      FileMsgC	*newMsg = new FileMsgC(newFile, 0, len, True, oldMsg->folder);
      newMsg->SetNumber(oldMsg->Number());
      ishApp->DisplayMessage(newMsg, This->pub);

      if ( oldMsg->folder )
	 oldMsg->folder->ReplaceMsg(oldMsg, newMsg);
   }

   This->pub->BusyCursor(False);

} // End DoSunToMime

/*---------------------------------------------------------------
 *  Callback for when decryption is complete
 */

void
ReadWinP::CryptDone(int, DisplayDataT *data)
{
   char	*file = (char*)data->icon;
   unlink(file);
   free(file);

   data->win->displayDataList.remove(data);
   delete data;
}

/*---------------------------------------------------------------
 *  Callback to pass message through decryptor
 */

void
ReadWinP::DoDecrypt(Widget, ReadWinP *This, XtPointer)
{
   This->pub->BusyCursor(True);

//
// Write message body to a temp file
//
   char		*newFile = tempnam(NULL, "cryp.");

   if ( !This->pub->msg->WriteBody(newFile, False, False) ) {
      free(newFile);
      return;
   }

//
// Build a data record to store info about this process
//
   DisplayDataT	*data = new DisplayDataT;
   data->win  = This;
   data->icon = (ReadIconC*)newFile;

//
// Build the command for display.
//
   data->cmdStr = ishApp->readPrefs->decryptCmd;
   data->cmdStr.Replace("%s", newFile);
   data->pid    = -1;

   CallbackC	doneCb((CallbackFn*)CryptDone, data);
   data->pid = ForkIt(data->cmdStr, &doneCb);

//
// If the pid is < 0, an error occurred
//
   if ( data->pid < 0 ) {
      This->pub->PopupMessage(ForkStatusMsg(data->cmdStr, (int)data->pid));
      delete data;
   }

//
// Add this process to the list of running processes
//
   else {
      void	*tmp = (void*)data;
      This->displayDataList.add(tmp);
   }

   This->pub->BusyCursor(False);

} // End DoDecrypt

/*---------------------------------------------------------------
 *  Callback to pass message through authenticator
 */

void
ReadWinP::DoAuth(Widget, ReadWinP *This, XtPointer)
{
   This->pub->BusyCursor(True);

//
// Write message body to a temp file
//
   char		*newFile = tempnam(NULL, "auth.");

   if ( !This->pub->msg->WriteBody(newFile, False, False) ) {
      free(newFile);
      return;
   }

//
// Build a data record to store info about this process
//
   DisplayDataT	*data = new DisplayDataT;
   data->win  = This;
   data->icon = (ReadIconC*)newFile;

//
// Build the command for display.
//
   data->cmdStr = ishApp->readPrefs->authCmd;
   data->cmdStr.Replace("%s", newFile);
   data->pid    = -1;

   CallbackC	doneCb((CallbackFn*)CryptDone, data);
   data->pid = ForkIt(data->cmdStr, &doneCb);

//
// If the pid is < 0, an error occurred
//
   if ( data->pid < 0 ) {
      This->pub->PopupMessage(ForkStatusMsg(data->cmdStr, (int)data->pid));
      delete data;
   }

//
// Add this process to the list of running processes
//
   else {
      void	*tmp = (void*)data;
      This->displayDataList.add(tmp);
   }

   This->pub->BusyCursor(False);

} // End DoAuth

/*---------------------------------------------------------------
 *  Callback to edit message
 */

void
ReadWinP::DoEdit(Widget, ReadWinP *This, XtPointer)
{
   This->pub->BusyCursor(True);

   if ( !This->editWin )
      This->editWin = new SendWinC("sendWin", *This->pub, SEND_EDIT);

   Boolean	editOk = !This->editWin->IsShown();
   This->editWin->Show();

   if ( editOk ) {

      MsgC	*msg  = This->pub->msg;
      MsgPartC	*body = msg->Body();

      Boolean	editSource = (This->viewType == READ_VIEW_SOURCE ||
			      (!msg->IsMime() && !body->IsEncoded()));

      This->editWin->SetWrap(XmToggleButtonGetState(This->optWrapTB));
      This->editWin->EditMsg(msg, editSource, (CallbackFn*)EditFinished,
      			     (void*)This);
   }

   This->pub->BusyCursor(False);
}

/*---------------------------------------------------------------
 *  Callback routine to handle completion of edit
 */

void
ReadWinP::EditFinished(char *file, ReadWinP *This)
{
   This->pub->BusyCursor(True);

   MsgC		*oldMsg = This->pub->msg;
   int		len     = 0;
   struct stat	stats;
   if ( stat(file, &stats) == 0 ) len = (int)stats.st_size;

   FileMsgC	*newMsg = new FileMsgC(file, 0, len, True, oldMsg->folder);
   newMsg->SetNumber(oldMsg->Number());
   ishApp->DisplayMessage(newMsg, This->pub);

   if ( oldMsg->folder )
      oldMsg->folder->ReplaceMsg(oldMsg, newMsg);

   This->pub->BusyCursor(False);

} // End EditFinished

