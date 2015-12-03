/*
 *  $Id: SaveMgrC.C,v 1.5 2000/08/07 11:05:17 evgeny Exp $
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
#include "SaveMgrC.h"
#include "IshAppC.h"
#include "SavePrefC.h"
#include "AppPrefC.h"
#include "MainWinC.h"
#include "MsgItemC.h"
#include "MsgC.h"
#include "FolderC.h"
#include "ShellExp.h"
#include "AddressC.h"
#include "FolderPrefC.h"
#include "FileChooserWinC.h"
#include "ImapMisc.h"
#include "FileMisc.h"

#include <hgl/VItemC.h>
#include <hgl/VBoxC.h>
#include <hgl/SysErr.h>
#include <hgl/WArgList.h>
#include <hgl/rsrc.h>
#include <hgl/WXmString.h>

#include <Xm/Frame.h>
#include <Xm/RowColumn.h>
#include <Xm/ToggleB.h>
#include <Xm/PushB.h>
#include <Xm/MessageB.h>
#include <Xm/Protocols.h>
#include <Xm/AtomMgr.h>

#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

/*------------------------------------------------------------------------
 * Constructor
 */

SaveMgrC::SaveMgrC()
{
   saveFolderWin      = NULL;
   saveFileWin        = NULL;
   saveToFileQueryWin = NULL;
   saveMsg            = NULL;
   saveItemList       = new VItemListC;
   saveItemList->AllowDuplicates(FALSE);
}

/*------------------------------------------------------------------------
 * Destructor
 */

SaveMgrC::~SaveMgrC()
{
   delete saveItemList;
   delete saveFolderWin;
   delete saveFileWin;
}

/*---------------------------------------------------------------
 *  Method to set the current save folder.
 */

void
SaveMgrC::UpdateSaveFolder(MsgC *msg)
{
//
// Find the name of the default folder
//
   MsgSaveTypeT	saveType = ishApp->appPrefs->saveType;
   if ( !msg ) saveType = SAVE_TO_FOLDER;

//
// Pass first selected message through save rules 
//
   if ( saveType == SAVE_BY_PATTERN ||
        saveType == SAVE_BY_PATTERN_OR_USER ||
        saveType == SAVE_BY_PATTERN_OR_ADDRESS ) {

      StringC	*fn   = msg->Match(ishApp->savePrefs->saveRules);

      if ( fn ) {
	 curSaveFolder = *fn;
	 ishApp->ExpandFolderName(curSaveFolder, ishApp->appPrefs->SaveDir());
      }

      else if ( saveType == SAVE_BY_PATTERN_OR_USER )
	 saveType = SAVE_BY_USER;
      else if ( saveType == SAVE_BY_PATTERN_OR_ADDRESS )
	 saveType = SAVE_BY_ADDRESS;
      else
	 saveType = SAVE_TO_FOLDER;

   } // End if saving by pattern

//
// Get user name or address from message
//
   if ( saveType == SAVE_BY_USER || saveType == SAVE_BY_ADDRESS ) {

      AddressC	*from = msg->From();
      if ( from ) {

	 curSaveFolder = ishApp->appPrefs->SaveDir();
	 curSaveFolder += "/";

	 if ( saveType == SAVE_BY_USER )
	    curSaveFolder += from->mailbox;
	 else
	    curSaveFolder += from->addr;
      }
      else
	 saveType = SAVE_TO_FOLDER;

   } // End if saving by user or address

   if ( saveType == SAVE_TO_FOLDER )
      curSaveFolder = ishApp->appPrefs->SaveFile();

} // End UpdateSaveFolder

/*------------------------------------------------------------------------
 * Method to save a single message to an open folder
 */

void
SaveMgrC::SaveMsgToFolder(MsgC* msg, FolderC *folder, Boolean delAfter)
{
   ishApp->BusyCursor(True);

//
// Save the message
//
   StringC	msgStr("Message ");
   msgStr += msg->Number();
   Boolean	deleted = False;
   Boolean	copied  = folder->AddMessage(msg);

   if ( copied ) {

      msg->SetSaved();
      ishApp->appPrefs->AddRecentFolder(folder->name);

//
// Delete if requested
//
      deleted = delAfter && ishApp->mainWin->DeleteMsg(msg);

//
// Display status
//
      msgStr += " saved to ";
      msgStr += folder->abbrev;
      if ( deleted ) msgStr += " and deleted.";

   } // End if copied

   else {
      msgStr = " NOT saved.";
   }

   ishApp->Broadcast(msgStr);

   if ( msg->folder && msg->folder->active && msg->icon )
      ishApp->mainWin->MsgVBox().Refresh();

   ishApp->mainWin->UpdateTitle();
   ishApp->BusyCursor(False);

} // End SaveMsgToFolder

/*------------------------------------------------------------------------
 * Method to save several messages to an open folder
 */

void
SaveMgrC::SaveMsgsToFolder(VItemListC& list, FolderC *folder, Boolean delAfter)
{
   ishApp->BusyCursor(True);

//
// Save the messages
//
   VItemListC	saveList;
   unsigned	count = list.size();
   CharC	nl("\n");
   int	i;
   for (i=0; i<count; i++) {

      MsgItemC	*item = (MsgItemC*)list[i];
      MsgC	*msg = item->msg;
      Boolean	copied = False;
      if ( folder->AddMessage(msg) ) {
	 copied = True;
	 msg->SetSaved();
      }
      if ( copied ) saveList.add((VItemC*)item);
   }

   count = saveList.size();
   if ( count > 0 )
      ishApp->appPrefs->AddRecentFolder(folder->name);

//
// Delete saved messages if requested
//
   Boolean deleted = (delAfter && count>0 &&
		      ishApp->mainWin->DeleteItems(saveList));

   ishApp->mainWin->MsgVBox().Refresh();

//
// Build status message
//
   StringC	msgStr;
   if ( count > 0 ) {
      msgStr = "Message";
      if ( count > 1 ) msgStr += 's';
      msgStr += ' ';
      for (i=0; i<count; i++) {
	 if ( i>0 ) msgStr += ", ";
	 MsgItemC	*item = (MsgItemC*)saveList[i];
	 msgStr += item->msg->Number();
      }

      msgStr += " saved to ";
      msgStr += folder->abbrev;
      if ( deleted ) msgStr += " and deleted.";
   }
   else {
      msgStr = "NO messages saved";
   }
   ishApp->Broadcast(msgStr);

   ishApp->mainWin->UpdateTitle();
   ishApp->BusyCursor(False);

} // End SaveMsgsToFolder

/*------------------------------------------------------------------------
 * Method to save a single message to a named folder
 */

void
SaveMgrC::SaveMsgToFolder(MsgC* msg, const char *name, Boolean delAfter)
{
//
// Create a folder
//
   FolderC *folder = ishApp->folderPrefs->GetFolder(name, True/*ok to create*/);
   if ( folder )
      SaveMsgToFolder(msg, folder, delAfter);
   else {
      StringC	errmsg("Could not create folder: ");
      errmsg += name;
      errmsg += ".\n";
      if ( errno != 0 ) errmsg += SystemErrorMessage(errno);
      halApp->PopupMessage(errmsg);
   }

} // End SaveMsgToFolder

/*------------------------------------------------------------------------
 * Method to save several messages to a named folder
 */

void
SaveMgrC::SaveMsgsToFolder(VItemListC& list, const char *name, Boolean delAfter)
{
   FolderC *folder = ishApp->folderPrefs->GetFolder(name, True/*ok to create*/);
   if ( folder )
      SaveMsgsToFolder(list, folder, delAfter);
   else {
      StringC	errmsg("Could not create folder: ");
      errmsg += *name;
      errmsg += ".\n";
      if ( errno != 0 ) errmsg += SystemErrorMessage(errno);
      halApp->PopupMessage(errmsg);
   }

} // End SaveMsgsToFolder

/*------------------------------------------------------------------------
 * Method to save a single message to an unnamed folder
 */

void
SaveMgrC::SaveMsgToFolder(MsgC* msg, Widget dialogParent, Boolean moveOk)
{
   halApp->BusyCursor(True);

   if ( !saveFolderWin ) BuildSaveFolderWin(dialogParent);

   if ( (!ishApp->appPrefs->usingImap) ||
	(!ishApp->folderPrefs->UsingLocal()) )
       saveFolderWin->SetDefaultDir(ishApp->appPrefs->FolderDir());
   else
       saveFolderWin->SetDefaultDir("");

   saveFolderWin->ShowDirsInFileList (ishApp->folderPrefs->usingMh);
   saveFolderWin->ShowFilesInFileList(ishApp->folderPrefs->usingUnix ||
				      ishApp->folderPrefs->usingMmdf);

//
// See if Move/Copy buttons are displayed
//
   if ( moveOk ) {

      XtManageChild(folderOpFrame);

//
// Set move or copy depending on whether saved messages are automatically
//    deleted (save == move)
//
      if ( ishApp->appPrefs->deleteSaved )
	 XmToggleButtonSetState(folderMoveTB, True, True);
      else
	 XmToggleButtonSetState(folderCopyTB, True, True);
   }

   else {
      XtUnmanageChild(folderOpFrame);
   }

//
// Display dialog
//
   saveMsg = msg;
   saveItemList->removeAll();
   saveFolderWin->Show(dialogParent);

   halApp->BusyCursor(False);

} // End SaveMsgToFolder

/*------------------------------------------------------------------------
 * Method to save several messages to an unnamed folder
 */

void
SaveMgrC::SaveMsgsToFolder(VItemListC& msgList, Widget dialogParent,
			   Boolean moveOk)
{
   halApp->BusyCursor(True);

   if ( !saveFolderWin ) BuildSaveFolderWin(dialogParent);

   if ( (!ishApp->appPrefs->usingImap) ||
	(!ishApp->folderPrefs->UsingLocal()) )
       saveFolderWin->SetDefaultDir(ishApp->appPrefs->FolderDir());
   else
       saveFolderWin->SetDefaultDir("");

   saveFolderWin->ShowDirsInFileList (ishApp->folderPrefs->usingMh);
   saveFolderWin->ShowFilesInFileList(ishApp->folderPrefs->usingUnix ||
				      ishApp->folderPrefs->usingMmdf);

//
// See if Move/Copy buttons are displayed
//
   if ( moveOk ) {

      XtManageChild(folderOpFrame);

//
// Set move or copy depending on whether saved messages are automatically
//    deleted (save == move)
//
      if ( ishApp->appPrefs->deleteSaved )
	 XmToggleButtonSetState(folderMoveTB, True, True);
      else
	 XmToggleButtonSetState(folderCopyTB, True, True);
   }

   else {
      XtUnmanageChild(folderOpFrame);
   }

//
// Display dialog
//
   saveMsg       = NULL;
   *saveItemList = msgList;
   saveFolderWin->Show(dialogParent);

   halApp->BusyCursor(False);

} // End SaveMsgsToFolder

/*------------------------------------------------------------------------
 * Method to build the save-to-folder dialog
 */

void
SaveMgrC::BuildSaveFolderWin(Widget parent)
{
   if ( saveFolderWin ) return;

//
// Find the starting directory
//
    StringC	dir = ishApp->appPrefs->FolderDir();
    if ( access(dir, R_OK|W_OK) != 0 ) {
	dir = ishApp->home;
	if ( access(dir, R_OK|W_OK) != 0 ) dir = ".";
    }

//
// Create dialog
//
   saveFolderWin = new FileChooserWinC(parent, "saveFolderWin");

//
// Add pane for Move/Copy choices in the last position
//
   Cardinal	childCount;
   XtVaGetValues(saveFolderWin->PanedWin(), XmNnumChildren, &childCount, NULL);

   WArgList	args;
   args.PositionIndex(childCount+1);
   args.ShadowThickness(0);
   args.AllowResize(True);
   folderOpFrame = XmCreateFrame(saveFolderWin->PanedWin(), "opFrame", ARGS);

   args.Reset();
   args.ChildType(XmFRAME_TITLE_CHILD);
   args.ChildHorizontalAlignment(XmALIGNMENT_CENTER);
   Widget	radioFrame = XmCreateFrame(folderOpFrame, "radioFrame", ARGS);

   args.Reset();
   args.Orientation(XmHORIZONTAL);
   args.Packing(XmPACK_TIGHT);
   Widget radio = XmCreateRadioBox(radioFrame, "opRadio", ARGS);

   folderMoveTB = XmCreateToggleButton(radio, "moveTB", 0,0);
   folderCopyTB = XmCreateToggleButton(radio, "copyTB", 0,0);

   XtManageChild(folderMoveTB);
   XtManageChild(folderCopyTB);
   XtManageChild(radio);	// radioFrame children
   XtManageChild(radioFrame);	// folderOpFrame children

   saveFolderWin->HandleHelp(folderMoveTB);
   saveFolderWin->HandleHelp(folderCopyTB);

   saveFolderWin->SingleSelect(True);
   saveFolderWin->AddVerifyCallback((CallbackFn*)VerifySaveFolders, this);
   saveFolderWin->AddOkCallback    ((CallbackFn*)FinishSaveFolder,  this);
   saveFolderWin->HideList();
   saveFolderWin->SetDirectory(dir);

} // End BuildSaveFolderWin

/*---------------------------------------------------------------
 *  Callbacks to handle selection of folder name for save
 */

void
SaveMgrC::FinishSaveFolder(StringListC *list, SaveMgrC *This)
{
   StringC	name = *((*list)[0]);
   if ( name.size() == 0 ) {
      StringC	msg = "Please enter a folder name.";
      set_invalid(This->saveFolderWin->SelectTF(), True, True);
      halApp->PopupMessage(msg);
      This->saveFolderWin->HideOk(False);
      return;
   }

//
// Don't allow spaces in the name
//
   if ( name.Contains(' ') || name.Contains('\t') ) {
      StringC	msg = "Spaces are not allowed in folder names.";
      set_invalid(This->saveFolderWin->SelectTF(), True, True);
      halApp->PopupMessage(msg);
      This->saveFolderWin->HideOk(False);
      return;
   }

   Boolean move = XtIsManaged(This->folderOpFrame) &&
		  XmToggleButtonGetState(This->folderMoveTB);

   if ( This->saveMsg )
      This->SaveMsgToFolder(This->saveMsg, name, move);
   else
      This->SaveMsgsToFolder(*This->saveItemList, name, move);

} // End FinishSaveFolder

/*------------------------------------------------------------------------
 * Method to save a single message to an unnamed file
 */

void
SaveMgrC::SaveMsgToFile(MsgC* msg, Widget dialogParent, Boolean moveOk)
{
   halApp->BusyCursor(True);

   if ( !saveFileWin ) BuildSaveFileWin(dialogParent);

//
// See if Move/Copy buttons are displayed
//
   if ( moveOk ) {

      XtManageChild(fileOpFrame);

//
// Set move or copy depending on whether saved messages are automatically
//    deleted (save == move)
//
      if ( ishApp->appPrefs->deleteSaved )
	 XmToggleButtonSetState(fileMoveTB, True, True);
      else
	 XmToggleButtonSetState(fileCopyTB, True, True);
   }

   else {
      XtUnmanageChild(fileOpFrame);
   }

//
// Display dialog
//
   saveMsg = msg;
   saveItemList->removeAll();
   saveFileWin->Show(dialogParent);

   halApp->BusyCursor(False);

} // End SaveMsgToFile

/*------------------------------------------------------------------------
 * Method to save several messages to an unnamed file
 */

void
SaveMgrC::SaveMsgsToFile(VItemListC& msgList, Widget dialogParent,
			 Boolean moveOk)
{
   halApp->BusyCursor(True);

   if ( !saveFileWin ) BuildSaveFileWin(dialogParent);

//
// See if Move/Copy buttons are displayed
//
   if ( moveOk ) {

      XtManageChild(fileOpFrame);

//
// Set move or copy depending on whether saved messages are automatically
//    deleted (save == move)
//
      if ( ishApp->appPrefs->deleteSaved )
	 XmToggleButtonSetState(fileMoveTB, True, True);
      else
	 XmToggleButtonSetState(fileCopyTB, True, True);
   }

   else {
      XtUnmanageChild(fileOpFrame);
   }

//
// Display dialog
//
   saveMsg       = NULL;
   *saveItemList = msgList;
   saveFileWin->Show(dialogParent);

   halApp->BusyCursor(False);

} // End SaveMsgsToFile

/*------------------------------------------------------------------------
 * Method to build the save-to-file dialog
 */

void
SaveMgrC::BuildSaveFileWin(Widget parent)
{
   if ( saveFileWin ) return;

//
// Create dialog
//
   saveFileWin = new FileChooserWinC(parent, "saveFileWin");

//
// Add pane for header saving query.  Put it in the first position
//
   WArgList	args;
   args.PositionIndex(0);
   args.AllowResize(True);
   Widget headFrame = XmCreateFrame(saveFileWin->PanedWin(), "saveHdrFrame",
				    ARGS);

   Widget headRadio = XmCreateRadioBox(headFrame, "saveHdrRadio", 0,0);

   headAllTB  = XmCreateToggleButton(headRadio, "saveHdrAllTB",  0,0);
   headDispTB = XmCreateToggleButton(headRadio, "saveHdrDispTB", 0,0);
   headNoneTB = XmCreateToggleButton(headRadio, "saveHdrNoneTB", 0,0);

   XtManageChild(headAllTB);
   XtManageChild(headDispTB);
   XtManageChild(headNoneTB);
   XtManageChild(headRadio);	// headFrame children
   XtManageChild(headFrame);	// panedWin children

   saveFileWin->HandleHelp(headAllTB);
   saveFileWin->HandleHelp(headDispTB);
   saveFileWin->HandleHelp(headNoneTB);

   XmToggleButtonSetState(headNoneTB, True, True);

//
// Add pane for Move/Copy choices in the last position
//
   Cardinal	childCount;
   XtVaGetValues(saveFileWin->PanedWin(), XmNnumChildren, &childCount, NULL);

   args.Reset();
   args.PositionIndex(childCount+1);
   args.ShadowThickness(0);
   args.AllowResize(True);
   fileOpFrame = XmCreateFrame(saveFileWin->PanedWin(), "opFrame", ARGS);

   args.Reset();
   args.ChildType(XmFRAME_TITLE_CHILD);
   args.ChildHorizontalAlignment(XmALIGNMENT_CENTER);
   Widget	radioFrame = XmCreateFrame(fileOpFrame, "radioFrame", ARGS);

   args.Reset();
   args.Orientation(XmHORIZONTAL);
   args.Packing(XmPACK_TIGHT);
   Widget radio = XmCreateRadioBox(radioFrame, "opRadio", ARGS);

   fileMoveTB = XmCreateToggleButton(radio, "moveTB", 0,0);
   fileCopyTB = XmCreateToggleButton(radio, "copyTB", 0,0);

   XtManageChild(fileMoveTB);
   XtManageChild(fileCopyTB);
   XtManageChild(radio);	// radioFrame children
   XtManageChild(radioFrame);	// fileOpFrame children

   saveFileWin->HandleHelp(fileMoveTB);
   saveFileWin->HandleHelp(fileCopyTB);

   saveFileWin->SingleSelect(True);
   saveFileWin->AddVerifyCallback((CallbackFn*)VerifySaveFiles, this);
   saveFileWin->AddOkCallback    ((CallbackFn*)FinishSaveFile,  this);
   saveFileWin->HideList();
   saveFileWin->SetDirectory(".");
   saveFileWin->SetDefaultDir(".");
   saveFileWin->ShowDirsInFileList(False);
   saveFileWin->ShowFilesInFileList(True);

} // End BuildSaveFileWin

/*---------------------------------------------------------------
 *  Callbacks to handle selection of file name for save
 */

void
SaveMgrC::FinishSaveFile(StringListC *list, SaveMgrC *This)
{
   StringC	name = *((*list)[0]);
   if ( name.size() == 0 ) {
      StringC	msg = "Please enter a file name.";
      set_invalid(This->saveFileWin->SelectTF(), True, True);
      halApp->PopupMessage(msg);
      This->saveFileWin->HideOk(False);
      return;
   }

//
// If the file already exists, see if they want it replaced or appended
//
   if ( access(name, F_OK) == 0 ) {

//
// Make sure name is not a directory
//
      if ( IsDir(name) ) {
	 StringC errmsg(name);
	 errmsg += " is a directory.";
	 halApp->PopupMessage(errmsg);
	 return;
      }

//
// See whether they want to replace or append
//
      switch ( This->SaveToFileQuery(name) ) {

	 case (QUERY_YES):	// Means APPEND
	    break;

	 case (QUERY_NO): {	// Means REPLACE
	    int	fd = open(name, O_WRONLY|O_TRUNC, 0);
	    if ( fd > 0 ) close(fd);
	 } break;

	 case (QUERY_CANCEL):
	 case (QUERY_NONE):
	 default:
	    return;
      }

   } // End if file already exists

   else if ( !MakeFile(name) )
      return;

   Boolean move = XtIsManaged(This->fileOpFrame) &&
		  XmToggleButtonGetState(This->fileMoveTB);

   Boolean	copyHead = !XmToggleButtonGetState(This->headNoneTB);
   Boolean	allHead  =  XmToggleButtonGetState(This->headAllTB);

   if ( This->saveMsg )
      This->SaveMsgToFile(This->saveMsg, name, move, copyHead, allHead);
   else
      This->SaveMsgsToFile(*This->saveItemList, name, move, copyHead, allHead);

} // End FinishSaveFile

/*---------------------------------------------------------------
 *  Callback routines to double check file list in file chooser
 */

void
SaveMgrC::VerifySaveFolders(StringListC *list, SaveMgrC *This)
{
   This->VerifySaveList(*list, This->saveFolderWin);
}

void
SaveMgrC::VerifySaveFiles(StringListC *list, SaveMgrC *This)
{
   This->VerifySaveList(*list, This->saveFileWin);
}

void
SaveMgrC::VerifySaveList(StringListC& list, FileChooserWinC *win)
{
   StringC	dname = win->Directory();
   if ( !win->UsingImap() ) {

      chdir(dname);

//
// Remove lock files and unwritable files
//
      unsigned	count = list.size();
      for (int i=count-1; i>=0; i--) {
	 StringC	*name = list[i];
	 if ( name->EndsWith(".lock") || access(*name, W_OK) != 0 )
	    list.remove(i);
      }

      chdir(ishApp->startupDir);
   }

//
// Remove the current folder
//
   if ( !dname.EndsWith("/") ) dname += "/";

   int		dlen = dname.size();
   StringC	name = ishApp->mainWin->curFolder->name;

//
// See if folder is in same directory as file chooser
//
   if ( name.StartsWith(dname) ) {
      name.CutBeg(dname.size());
      int	index = list.indexOf(name);
      if ( index != list.NULL_INDEX ) list.remove(index);
   }

} // End VerifySaveList

/*------------------------------------------------------------------------
 * Method to save a single message to a named file
 */

void
SaveMgrC::SaveMsgToFile(MsgC* msg, const char *name, Boolean delAfter,
			Boolean copyHead, Boolean allHead)
{
   ishApp->BusyCursor(True);

//
// Save the message
//
   StringC	msgStr("Message ");
   msgStr += msg->Number();
   Boolean deleted = False;
   Boolean copied = msg->WriteFile(name, copyHead, allHead, /*statHead=*/False,
				   /*addBlank*/False, /*protectFrom*/False);
   if ( copied ) {

      msg->SetSaved();

//
// Delete if requested
//
      deleted = delAfter && ishApp->mainWin->DeleteMsg(msg);

//
// Display status
//
      msgStr += " saved to ";
      msgStr += name;
      if ( deleted ) msgStr += " and deleted.";

   } // End if copied

   else {
      msgStr = " NOT saved.";
   }

   ishApp->Broadcast(msgStr);

   if ( msg->folder && msg->folder->active && msg->icon )
      ishApp->mainWin->MsgVBox().Refresh();

   ishApp->mainWin->UpdateTitle();
   ishApp->BusyCursor(False);

} // End SaveMsgToFile

/*------------------------------------------------------------------------
 * Method to save several messages to an open folder
 */

void
SaveMgrC::SaveMsgsToFile(VItemListC& list, const char *name, Boolean delAfter,
			 Boolean copyHead, Boolean allHead)
{
   ishApp->BusyCursor(True);

//
// Save the messages
//
   VItemListC	saveList;
   unsigned	count = list.size();
   CharC	nl("\n");
   int	i;
   for (i=0; i<count; i++) {

      MsgItemC	*item = (MsgItemC*)list[i];
      MsgC	*msg = item->msg;
      Boolean	copied = msg->WriteFile(name, copyHead, allHead,
      					/*statHead=*/False,
      					/*addBlank*/True, /*protectFrom*/False);
      if ( copied ) {
	 msg->SetSaved();
	 saveList.add((VItemC*)item);
      }

   } // End for each message

   count = saveList.size();

//
// Delete saved messages if requested
//
   Boolean deleted = (delAfter && count>0 &&
		      ishApp->mainWin->DeleteItems(saveList));

   ishApp->mainWin->MsgVBox().Refresh();

//
// Build status message
//
   StringC	msgStr;
   if ( count > 0 ) {
      msgStr = "Message";
      if ( count > 1 ) msgStr += 's';
      msgStr += ' ';
      for (i=0; i<count; i++) {
	 if ( i>0 ) msgStr += ", ";
	 MsgItemC	*item = (MsgItemC*)saveList[i];
	 msgStr += item->msg->Number();
      }

      msgStr += " saved to ";
      msgStr += name;
      if ( deleted ) msgStr += " and deleted.";
   }
   else {
      msgStr = "NO messages saved";
   }
   ishApp->Broadcast(msgStr);

   ishApp->mainWin->UpdateTitle();
   ishApp->BusyCursor(False);

} // End SaveMsgsToFile

/*---------------------------------------------------------------
 *  Method to query for replace or append during save-to-file
 */

QueryAnswerT
SaveMgrC::SaveToFileQuery(const char *file)
{
   static QueryAnswerT	answer;

//
// Create the dialog if necessary
//
   if ( !saveToFileQueryWin ) {

      halApp->BusyCursor(True);

      WArgList	args;
      args.DialogStyle(XmDIALOG_FULL_APPLICATION_MODAL);
      if ( halApp->questionPM ) args.SymbolPixmap(halApp->questionPM);
      Widget w = XmCreateQuestionDialog(*halApp, "saveToFileQueryWin", ARGS);

      XtAddCallback(w, XmNokCallback, (XtCallbackProc)AnswerQuery,
		    (XtPointer)&answer);
      XtAddCallback(w, XmNcancelCallback, (XtCallbackProc)AnswerQuery,
		    (XtPointer)&answer);
      XtAddCallback(w, XmNhelpCallback, (XtCallbackProc)HalAppC::DoHelp,
      		    (char *) "helpcard");

      Widget replacePB = XmCreatePushButton(w, "replacePB", 0,0);
      XtManageChild(replacePB);

      XtAddCallback(replacePB, XmNactivateCallback, (XtCallbackProc)AnswerQuery,
     		    (XtPointer)&answer);

//
// Trap window manager close function
//
      XmAddWMProtocolCallback(XtParent(w), halApp->delWinAtom,
			      (XtCallbackProc)WmClose,
			      (caddr_t)&answer);

      saveToFileQueryWin = w;

      halApp->BusyCursor(False);

   } // End if save query dialog not created

//
// Add the file name to the message string
//
   StringC	msg = get_string(saveToFileQueryWin, "messageString",
				 "$FILE exists.");
   msg.Replace("$FILE", file);

   WXmString	wstr = (char *)msg;
   XtVaSetValues(saveToFileQueryWin, XmNmessageString, (XmString)wstr, NULL);
   XtManageChild(saveToFileQueryWin);
   XMapRaised(halApp->display, XtWindow(XtParent(saveToFileQueryWin)));

//
// Simulate the main event loop and wait for the answer
//

   answer = QUERY_NONE;
   while ( answer == QUERY_NONE ) {
      XtAppProcessEvent(halApp->context, XtIMXEvent);
      XSync(halApp->display, False);
   }

   XtUnmanageChild(saveToFileQueryWin);
   XSync(halApp->display, False);
   XmUpdateDisplay(saveToFileQueryWin);

   return answer;

} // End SaveToFileQuery

