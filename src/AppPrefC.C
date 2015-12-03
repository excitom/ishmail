/*
 *  $Id: AppPrefC.C,v 1.6 2000/12/13 08:53:38 evgeny Exp $
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
#include "IshAppC.h"
#include "AppPrefC.h"
#include "AlertPrefC.h"
#include "ShellExp.h"
#include "Misc.h"
#include "MainWinC.h"
#include "AppPrefWinC.h"
#include "FolderPrefC.h"
#include "ImapMisc.h"
#include "ReadWinC.h"
#include "SendWinC.h"

#include <hgl/rsrc.h>
#include <hgl/CharC.h>
#include <hgl/StringListC.h>
#include <hgl/VBoxC.h>
#include <hgl/FieldViewC.h>

#ifndef MAIL_DIR
#  define MAIL_DIR	"/var/spool/mail"
#endif

/*---------------------------------------------------------------
 *  Constructor
 */

AppPrefC::AppPrefC() : PrefC()
{
   prefWin = NULL;

//
// Read resources
//
   orig.folderDir     = ishApp->home + "/Mail";
   orig.folderDir     = get_string(*halApp, "folderDirectory",	orig.folderDir);
   orig.saveDir       = get_string(*halApp, "saveDirectory",	orig.folderDir);
   orig.saveFile      = get_string(*halApp, "saveFile",		"+mbox");
   orig.archiveFolder = get_string(*halApp, "archiveFolder",	"+mbox");

   char	*defAuto = "";

   orig.automountRoot = get_string(*halApp, "automountRoot", defAuto);

   StringC	saveStr = get_string(*halApp, "saveType", "folder");
   if      ( saveStr.Equals("folder", IGNORE_CASE) )
      saveType = SAVE_TO_FOLDER;
   else if ( saveStr.Equals("user", IGNORE_CASE) )
      saveType = SAVE_BY_USER;
   else if ( saveStr.Equals("address", IGNORE_CASE) )
      saveType = SAVE_BY_ADDRESS;
   else if ( saveStr.Equals("pattern", IGNORE_CASE) )
      saveType = SAVE_BY_PATTERN;
   else if ( saveStr.Equals("pattern/user", IGNORE_CASE) )
      saveType = SAVE_BY_PATTERN_OR_USER;
   else if ( saveStr.Equals("pattern/address", IGNORE_CASE) )
      saveType = SAVE_BY_PATTERN_OR_ADDRESS;

   deleteSaved = get_boolean(*halApp, "deleteAfterSave",	False);
   hideDeleted = get_boolean(*halApp, "hideDeleted",		False);
   printCmd    = get_string (*halApp, "printCommand",		"pr | lp -s");

   char	*cs = getenv("MAILCHECK");
   int	mailcheck = cs ? atoi(cs) : 60;
   checkInterval = get_int(*halApp, "checkInterval", mailcheck); // seconds

//
// Get system mail folder name
//
   cs = getenv("MAIL");
   if ( cs && strlen(cs)>0 ) inBox = cs;
   else			     inBox = MAIL_DIR + ("/" + ishApp->userId);
   inBox = get_string (*halApp, "inBox", inBox);

   bellVolume        = get_int    (*halApp, "bellVolume",		0);
   scrollToNew       = get_boolean(*halApp, "scrollToNew",		True);
   rememberWindows   = get_boolean(*halApp, "rememberWindows",		False);
   markNewAsUnread   = get_boolean(*halApp, "markNewAsUnread",		True);
   archiveOnSave     = get_boolean(*halApp, "archiveOnSave",		False);
   recentFolderCount = get_int    (*halApp, "recentFolderCount",	10);
   recentFolders.AllowDuplicates(FALSE);
   StringC	tmp = get_string(*halApp, "recentFolders", "");
   ExtractList(tmp, recentFolders);
   recentFolderTime = time(0);

//
// Read IMAP settings
//
   usingImap  = get_boolean(*halApp, "usingImap", False);
   imapServer = get_string (*halApp, "imapServer", ishApp->host);

//
// Read POP settings
//
   usingPop     = get_boolean(*halApp, "usingPop", False);
   popServer    = get_string (*halApp, "popServer", ishApp->host);
   popclientCmd = get_string (*halApp, "popclientCommand",
			      "popclient -3 -s -u %user -p %pass -o $MAIL");

//
// For POP/IMAP login window, show password field first or login name field
//
   pwFieldFirst = get_boolean(*halApp, "pwFieldFirst", True);

} // End constructor

/*---------------------------------------------------------------
 *  Method to expand values.  Can only be called after folderPrefs object is
 *     created.
 */

void
AppPrefC::ExpandValues()
{
//
// Expand settings that could contain abbreviations or environment vars
//
   folderDir = orig.folderDir;
   ShellExpand(folderDir, True/*oneWord*/);

   // pre-pend home directory to relative path names

   if ( !folderDir.StartsWith("/") ) {

     // don't pre-pend if IMAP folder 

     if ( !IsImapName(folderDir) )
	folderDir = ishApp->home + "/" + folderDir;
   }
   if ( folderDir.size() > 1 && folderDir.EndsWith("/") ) folderDir.CutEnd(1);
   if ( debuglev > 0 ) cout <<"Folder directory is: " <<folderDir <<endl;

   saveFile	 = orig.saveFile;	ishApp->ExpandFolderName(saveFile);
   saveDir	 = orig.saveDir;	ishApp->ExpandFolderName(saveDir);
   archiveFolder = orig.archiveFolder;	ishApp->ExpandFolderName(archiveFolder);
   automountRoot = orig.automountRoot;
   ShellExpand(automountRoot, True/*oneWord*/);

} // End ExpandValues

/*---------------------------------------------------------------
 *  destructor
 */

AppPrefC::~AppPrefC()
{
   delete prefWin;
}

/*---------------------------------------------------------------
 *  Method to write resources to resource database
 */

Boolean
AppPrefC::WriteDatabase()
{
   Store("inBox",		inBox);
   Store("folderDirectory",	orig.folderDir);
   Store("saveFile",		orig.saveFile);
   Store("saveDirectory",	orig.saveDir);
   switch (saveType) {
      case (SAVE_BY_USER):
	 Store("saveType", "User");
	 break;
      case (SAVE_BY_ADDRESS):
	 Store("saveType", "Address");
	 break;
      case (SAVE_BY_PATTERN):
	 Store("saveType", "Pattern");
	 break;
      case (SAVE_BY_PATTERN_OR_USER):
	 Store("saveType", "Pattern/User");
	 break;
      case (SAVE_BY_PATTERN_OR_ADDRESS):
	 Store("saveType", "Pattern/Address");
	 break;
      case (SAVE_TO_FOLDER):
      default:
	 Store("saveType", "Folder");
	 break;
   }

   Store("automountRoot",	orig.automountRoot);
   Store("deleteAfterSave",	deleteSaved);
   Store("hideDeleted",		hideDeleted);
   Store("printCommand",	printCmd);
   Store("checkInterval",	checkInterval);
   Store("bellVolume",		bellVolume);
   Store("scrollToNew",		scrollToNew);
   Store("rememberWindows",	rememberWindows);
   Store("markNewAsUnread",	markNewAsUnread);
   Store("archiveOnSave",	archiveOnSave);
   Store("archiveFolder",	orig.archiveFolder);
   Store("showQuickHelp",	halApp->quickHelpEnabled);
   Store("alertIfNewMail",	ishApp->alertPrefs->alertOn);
   Store("recentFolderCount",	recentFolderCount);
   Store("usingImap",		usingImap);
   Store("imapServer",		imapServer);
   Store("usingPop",		usingPop);
   Store("popServer",		popServer);
   Store("popclientCommand",	popclientCmd);

   return True;

} // End WriteDatabase

/*---------------------------------------------------------------
 *  Method to write resources to a file
 */

Boolean
AppPrefC::WriteFile()
{
   StringListC	lineList;
   ReadResFile(lineList);

   Update(lineList, "inBox",		inBox);
   Update(lineList, "folderDirectory",	orig.folderDir);
   Update(lineList, "saveFile",		orig.saveFile);
   Update(lineList, "saveDirectory",	orig.saveDir);
   switch (saveType) {
      case (SAVE_BY_USER):
	 Update(lineList, "saveType", "User");
	 break;
      case (SAVE_BY_ADDRESS):
	 Update(lineList, "saveType", "Address");
	 break;
      case (SAVE_BY_PATTERN):
	 Update(lineList, "saveType", "Pattern");
	 break;
      case (SAVE_BY_PATTERN_OR_USER):
	 Update(lineList, "saveType", "Pattern/User");
	 break;
      case (SAVE_BY_PATTERN_OR_ADDRESS):
	 Update(lineList, "saveType", "Pattern/Address");
	 break;
      case (SAVE_TO_FOLDER):
      default:
	 Update(lineList, "saveType", "Folder");
	 break;
   }

   Update(lineList, "automountRoot",		orig.automountRoot);
   Update(lineList, "deleteAfterSave",		deleteSaved);
   Update(lineList, "hideDeleted",		hideDeleted);
   Update(lineList, "printCommand",		printCmd);
   Update(lineList, "checkInterval",		checkInterval);
   Update(lineList, "bellVolume",		bellVolume);
   Update(lineList, "scrollToNew",		scrollToNew);
   Update(lineList, "rememberWindows",		rememberWindows);
   Update(lineList, "markNewAsUnread",		markNewAsUnread);
   Update(lineList, "archiveOnSave",		archiveOnSave);
   Update(lineList, "archiveFolder",		orig.archiveFolder);
   Update(lineList, "showQuickHelp",		halApp->quickHelpEnabled);
   Update(lineList, "alertIfNewMail",		ishApp->alertPrefs->alertOn);
   Update(lineList, "recentFolderCount",	recentFolderCount);
   Update(lineList, "usingImap",		usingImap);
   Update(lineList, "imapServer",		imapServer);
   Update(lineList, "usingPop",			usingPop);
   Update(lineList, "popServer",		popServer);
   Update(lineList, "popclientCommand",		popclientCmd);

//
// Write the information back to the file
//
   return WriteResFile(lineList);

} // End WriteFile

/*---------------------------------------------------------------
 *  Method to write recent folder list to a file
 */

Boolean
AppPrefC::WriteRecentFolders()
{
   StringListC	lineList;
   ReadResFile(lineList);

   Update(lineList, "recentFolders", recentFolders);

//
// Write the information back to the file
//
   return WriteResFile(lineList);

} // End WriteRecentFolders

/*---------------------------------------------------------------
 *  Method to write window position resources to a file
 */

Boolean
AppPrefC::WriteWindowPositions()
{
   StringListC	lineList;
   ReadResFile(lineList);

   Position	x, y;
   Dimension	w, h;
   XtVaGetValues(*ishApp->mainWin, XmNx, &x, XmNy, &y, NULL);
   Update(lineList, "mainShell.x", x);
   Update(lineList, "mainShell.y", y);

   XtVaGetValues(ishApp->mainWin->FolderVBox().ScrollForm(),
   		 XmNwidth, &w, XmNheight, &h, NULL);
   Update(lineList, "mainShell.mainWindow*folderBox*scrollForm.width",  w);
   Update(lineList, "mainShell.mainWindow*folderBox*scrollForm.height", h);

#if 0
   XtVaGetValues(ishApp->mainWin->MsgVBox().ScrollForm(),
   		 XmNwidth, &w, XmNheight, &h, NULL);
   Update(lineList, "mainShell.mainWindow*msgBox*scrollForm.width",  w);
   Update(lineList, "mainShell.mainWindow*msgBox*scrollForm.height", h);
#else
   int	count = ishApp->mainWin->FieldView().VisibleItemCount();
   if ( count > 0 )
      Update(lineList, "mainShell.mainWindow*msgBox*visibleItemCount", count);
#endif

   if ( ishApp->sendWinList.size() > 0 ) {

      SendWinC	*sendWin = (SendWinC *)*ishApp->sendWinList[0];

      XtVaGetValues(*sendWin, XmNx, &x, XmNy, &y, NULL);

      Update(lineList, "sendWin.x", x);
      Update(lineList, "sendWin.y", y);
   }

   if ( ishApp->readWinList.size() > 0 ) {

      ReadWinC	*readWin = (ReadWinC *)*ishApp->readWinList[0];

      XtVaGetValues(*readWin, XmNx, &x, XmNy, &y, NULL);

      Update(lineList, "readWin.x", x);
      Update(lineList, "readWin.y", y);
   }

//
// Write the information back to the file
//
   return WriteResFile(lineList);

} // End WriteWindowPositions

/*---------------------------------------------------------------
 *  Method to write version information to the rc file
 */

Boolean
AppPrefC::WriteVersion()
{
   StringListC	lineList;
   ReadResFile(lineList);

   Update(lineList, "versionNumber", versionNumber);

//
// Write the information back to the file
//
   return WriteResFile(lineList);

} // End WriteVersion

/*------------------------------------------------------------------------
 * Method to add a folder name to the list of recent folders and update
 *    the timestamp.
 */

void
AppPrefC::AddRecentFolder(StringC name)
{
   ishApp->AbbreviateFolderName(name);

//
// If the name is already at the top of the list, return.
//
   int	index = recentFolders.indexOf(name);
   if ( index == 0 ) return;

//
// If the name is elsewhere in the list, remove it so it will get added to
//    the top
//
   if ( index > 0 ) recentFolders.remove(index);

//
// Add the name to the top of the list
//
   recentFolders.prepend(name);

//
// Remove a name if there are too many
//
   if ( recentFolders.size() > recentFolderCount )
      recentFolders.remove(recentFolders.size()-1);

   recentFolderTime = time(0);

//
// Call people who may want to know about this
//
   CallCallbacks(recentChangedCalls, this);

} // End AddRecentFolder

/*------------------------------------------------------------------------
 * Method to edit the preferences
 */

void
AppPrefC::Edit(Widget parent)
{
   halApp->BusyCursor(True);

   if ( !prefWin ) prefWin = new AppPrefWinC(parent);
   prefWin->Show();

   halApp->BusyCursor(False);
}

/*------------------------------------------------------------------------
 * Method to set folder directory
 */

void
AppPrefC::SetFolderDir(const char *dir)
{
   if ( orig.folderDir == dir ) return;

   folderDir = orig.folderDir = dir;

   ShellExpand(folderDir, True/*oneWord*/);

   // pre-pend home directory to relative path names

   if ( !folderDir.StartsWith("/") ) {

     // don't pre-pend if IMAP folder and using mixed local/remote folders

     if ( (!IsImapName(folderDir)) || (!ishApp->folderPrefs->UsingLocal()) )
	folderDir = ishApp->home + "/" + folderDir;
   }

   if ( folderDir.size() > 1 && folderDir.EndsWith("/") )
      folderDir.CutEnd(1);

   ishApp->mainWin->FolderDirChanged();

//
// Update all reading windows
//
   u_int	count = ishApp->readWinList.size();
   for (int i=0; i<count; i++) {
      ReadWinC	*readWin = (ReadWinC *)*ishApp->readWinList[i];
      readWin->FolderDirChanged();
   }

   if ( debuglev > 0 ) cout <<"New folder directory is: " <<folderDir <<endl;

} // End SetFolderDir

/*------------------------------------------------------------------------
 * Method to set default save folder
 */

void
AppPrefC::SetSaveFile(const char *name)
{
   if ( orig.saveFile == name ) return;

   saveFile = orig.saveFile = name;
   ishApp->ExpandFolderName(saveFile);
}

/*------------------------------------------------------------------------
 * Method to set save pattern directory name
 */

void
AppPrefC::SetSaveDir(const char *name)
{
   if ( orig.saveDir == name ) return;

   saveDir = orig.saveDir = name;
   ishApp->ExpandFolderName(saveDir);
}

/*------------------------------------------------------------------------
 * Method to set automounter root prefix
 */

void
AppPrefC::SetAutomountRoot(const char *name)
{
   if ( orig.automountRoot == name ) return;

   automountRoot = orig.automountRoot = name;
   ShellExpand(automountRoot, True/*oneWord*/);
}

/*------------------------------------------------------------------------
 * Method to set archive folder name
 */

void
AppPrefC::SetArchiveFolder(const char *name)
{
   if ( orig.archiveFolder == name ) return;

   archiveFolder = orig.archiveFolder = name;
   ishApp->ExpandFolderName(archiveFolder);
}

#if 0
/*------------------------------------------------------------------------
 * Method to set IMAP server name
 */

void
AppPrefC::SetImap(Boolean using, const char *name)
{
   if ( using == usingImap && imapServer && imapServer->name == name ) return;

   delete imapServer;
   usingImap = using;

   if ( usingImap ) {

      imapServer = new ImapServerC(name);

//
// Update inBox and folderDir with server name if necessary
//
      if ( !ishApp->folderPrefs->usingUnix &&
	   !ishApp->folderPrefs->usingMh   &&
	   !ishApp->folderPrefs->usingMmdf ) {

	 if ( ishApp->inBox.StartsWith('{') ) {
	    int	pos = ishApp->inBox.PosOf('}');
	    ishApp->inBox(1,pos-1) = name;
	 }
	 else {
	    ishApp->inBox = "{" + ImapServerName() + "}" + ishApp->inBox;
	 }

	 if ( folderDir.StartsWith('{') ) {
	    int	pos = folderDir.PosOf('}');
	    folderDir(1,pos-1) = name;
	 }
	 else {
	    folderDir = "{" + ImapServerName() + "}" + folderDir;
	 }

      } // End if not using local folders

   } // End if using IMAP

} // End SetImap

/*------------------------------------------------------------------------
 * Method to query IMAP server name
 */

CharC
AppPrefC::ImapServerName()
{
   if ( imapServer ) return imapServer->name;
   else		     return "";
}
#endif
