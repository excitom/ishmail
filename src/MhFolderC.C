/*
 * $Id: MhFolderC.C,v 1.4 2000/08/07 11:05:16 evgeny Exp $
 *
 * Copyright (c) 1994 HAL Computer Systems International, Ltd.
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

#include "MhFolderC.h"
#include "MhMsgC.h"
#include "FileMisc.h"
#include "IshAppC.h"
#include "AppPrefC.h"
#include "MainWinC.h"
#include "MsgItemC.h"
#include "SortMgrC.h"
#include "UndelWinC.h"
#include "MsgListC.h"
#include "MsgPartC.h"

#include <hgl/SysErr.h>
#include <hgl/VBoxC.h>

#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

extern int	debuglev;

/*------------------------------------------------------------------------
 * Function to determine if all the characters in name are numbers
 */

inline Boolean
AllNumbers(const char *name)
{
   if ( *name == ',' ) name++;	// An initial comma is allowed in deleted msgs

//
// strspn returns the length of the initial segment of name that consists
//    entirely of numbers.  If this is not the same as strlen(name), then
//    there are some non-numbers in the string.
//
   return (strspn(name, "0123456789") == strlen(name));
}

/*------------------------------------------------------------------------
 * Function to determine if a name corresponds to a message file
 */

inline Boolean
IsMessage(char *msg)
{
   return (AllNumbers(msg) && !IsDir(msg));
}

/*------------------------------------------------------------------------
 * Function to convert a file name into a message number
 */

inline int
MessageNumber(CharC name)
{
   CharC	file = BaseName(name);
   int		sign = 1;
   if ( file.StartsWith(',') ) {
      file.CutBeg(1);
      sign = -1;
   }

   return (sign * atoi(file.Addr()));
}

/*------------------------------------------------------------------------
 * Function to convert a message number into a file name
 */

inline void
GetMsgFile(CharC dir, int num, StringC& file)
{
   file = dir;
   if ( !file.EndsWith('/') ) file += '/';
   if ( num < 0 ) {
      file += ',';
      num = -num;
   }
   file += num;
}

/*------------------------------------------------------------------------
 * Function to determine if a message is in a message list
 */

inline Boolean
ListContains(MsgListC *list, int num)
{
   u_int	count = list->size();
   int i=0; for (i=0; i<count; i++) {
      MhMsgC	*msg = (MhMsgC*)(*list)[i];
      int	msgnum = MessageNumber(msg->filename);
      if ( num == msgnum ) return True;
   }

   return False;
}

/*------------------------------------------------------------------------
 * MhFolderC constructor
 */

MhFolderC::MhFolderC(const char *n, Boolean create)
: FolderC(n, MH_FOLDER, create)
{
   lastDirTime		= 0;
   changedWhileSleeping = False;

   dirList.AllowDuplicates(FALSE);
   dirList.SetSorted(FALSE);

//
// Get the date of the directory
//
   if ( stat(name, &stats) != 0 ) {

//
// Create the directory if necessary
//
      if ( errno == ENOENT && create ) {
	 if ( debuglev > 0 ) cout <<"Creating MH folder: " <<name <<endl;
	 if ( !MakeDir(name) ) return;
	 stat(name, &stats);
      }

      else {
	 StringC	errmsg = "Could not stat ";
	 errmsg += name;
	 errmsg += "\n" + SystemErrorMessage(errno);
	 halApp->PopupMessage(errmsg);
	 return;
      }
   }

//
// Make sure this is a directory
//
   if ( !S_ISDIR(stats.st_mode) ) {
      StringC	errmsg = name;
      errmsg += " is not an MH-format mail folder (a directory).";
      halApp->PopupMessage(errmsg);
      return;
   }

//
// See if directory is writable
//
   writable = (access(name, W_OK) == 0);

   if ( !writable ) {
      StringC	errmsg = "Folder \"";
      errmsg += name;
      errmsg += "\" is not writable and will be opened read-only";
      halApp->PopupMessage(errmsg, XmDIALOG_WARNING);
   }

   opened = True;

} // End MhFolderC constructor

/*------------------------------------------------------------------------
 * MhFolderC destructor
 */

MhFolderC::~MhFolderC()
{
   if ( delFiles ) RemoveDir(name);
}

/*------------------------------------------------------------------------
 * Method to build message list
 */

Boolean
MhFolderC::Scan()
{
   time_t       timeIn = 0;
   if ( debuglev > 0 ) {
      cout <<"Entering MhFolderC(" <<BaseName(name) <<")::Scan" <<endl;
      timeIn = time(0);
   }

   Boolean	firstTime = (lastDirTime == 0);
   Boolean	showCount = (firstTime && ishApp->mainWin->IsShown() &&
					 !ishApp->mainWin->IsIconified());

//
// Read the directory
//
   time_t	saveTime = lastDirTime;
   if ( !ReadDirectory() ) return False;

//
// See if there are any changes
//
   if ( msgList->size() == 0 || lastDirTime != saveTime ) {

      if ( debuglev > 0 ) cout <<"Directory changed since last read" <<endl;
 
      VBoxC&	vbox = ishApp->mainWin->MsgVBox();
      MsgItemC	*firstNew = NULL;
      MsgItemC	*lastNew  = NULL;

//
// If there are any messages not in directory list, start from scratch
//
      Boolean	dirOk = True;
      u_int	count = msgList->size();
      int i=0; for (i=0; dirOk && i<count; i++) {

	 MhMsgC	*msg = (MhMsgC*)(*msgList)[i];
	 int		num = MessageNumber(msg->filename);

	 if ( !dirList.includes(num) ) {
	    StringC	errmsg = "Message ";
	    errmsg += msg->filename;
	    errmsg += " has disappeared.\nThe folder will be re-read.";
	    halApp->PopupMessage(errmsg, XmDIALOG_WARNING);
	    dirOk = False;
	 }
      }

      if ( !dirOk )
	 DeleteAllMessages();

//
// Create messages for directory entries that aren't in the message list.
//
      if ( debuglev > 0 ) cout <<"Checking for new messages" <<endl;
      StringC	file;
      StringC	statusStr;
      count = dirList.size();
      for (i=0; i<count; i++) {

//
// Update progress message if necessary
//
	 if ( showCount /*&& (i % 10) == 0*/ ) {
	    statusStr = abbrev;
	    statusStr += " - Scanning messages: ";
	    statusStr += i;
	    ishApp->mainWin->Message(statusStr);
	 }

	 int	num = *dirList[i];
	 if ( !ListContains(msgList, num) ) {

	    GetMsgFile(name, num, file);

	    if ( debuglev > 1 ) cout <<"Found new message: " <<num <<endl;
	    MhMsgC	*msg = new MhMsgC(this, file, num);
	    msgList->add(msg);

//
// Create a message icon for this message
//
	    if ( !firstTime && active ) {

	       msg->CreateIcon();
	       InsertInThread(msg->icon);

	       msgItemList->add(msg->icon);
	       vbox.AddItem(*msg->icon);

	       if ( !firstNew ) firstNew = msg->icon;
	       lastNew = msg->icon;

	    } // End if folder is active

//
// If this is a partial message, process it
//
	    if ( msg->IsPartial() ) AddPartial(msg);

	 } // End if new message found

      } // End for each directory entry

//
// Update progress message if necessary
//
      if ( showCount ) {
	 statusStr = abbrev;
	 statusStr += " - Scanning messages: ";
	 statusStr += i;
	 ishApp->mainWin->Message(statusStr);
      }

      if ( !firstTime && active ) {

//
// Scroll to the last then the first to display as many new messages
//    as possible
//
	 if ( lastNew ) {
	    vbox.View()->ScrollToItem(*lastNew);
	    vbox.View()->ScrollToItem(*firstNew);
	 }

	 vbox.Refresh();
      }

   } // End if directory changed

   scanned = True;
   SetChanged(False);

   if ( firstTime ) ScanIndex();
   else		    UpdateIndex();

//
// Make sure any comma files are marked as deleted
//
   u_int	count = msgList->size();
   int i=0; for (i=0; i<count; i++) {
      MsgC	*msg = (*msgList)[i];
      if ( msg->Number() < 0 && !msg->IsDeleted() )
	 msg->SetDeleted();
   }

   UpdateIcon();
   ishApp->mainWin->ClearMessage();

   if ( debuglev > 0 ) {
      time_t	timeOut = time(0);
      cout <<"Leaving MhFolderC(" <<BaseName(name) <<")::Scan after "
      	   <<(timeOut-timeIn) <<" seconds" <<endl;
   }

   return True;

} // End Scan

/*---------------------------------------------------------------
 *  Method to read the entries from the directory and sort them
 */

Boolean
MhFolderC::ReadDirectory()
{
//
// If the time has not changed, our list is current.
//
   if ( stat(name, &stats) == 0 && stats.st_mtime <= lastDirTime )
      return True;

   dirList.removeAll();

//
// Open the directory
//
   if ( debuglev > 0 ) cout <<"Reading directory" <<endl;
   DIR	*dirp = opendir(name);
   if ( !dirp ) {
      int	err = errno;
      StringC   errmsg("Cannot open directory: ");
      errmsg += name + ".\n" + SystemErrorMessage(err);
      halApp->PopupMessage(errmsg);
      return False;
   }

   char savedir[MAXPATHLEN+1];
   getcwd(savedir, MAXPATHLEN);
   chdir(name);

//
// Gather names of new files.
//
   struct dirent	*dp;
   Boolean		error = False;
   while ( !error && (dp=readdir(dirp)) != NULL ) {

//
// Ignore names that aren't mail message files
//
      if ( !IsMessage(dp->d_name) ) continue;

      char	*np = dp->d_name;
      Boolean	deleted = False;
      if ( *np == ',' ) {
	 deleted = True;
	 np++;
      }

//
// Add this number to the list
//
      int	number = atoi(np);
      if ( deleted ) number = -number;
      dirList.add(number);

   } // End for each directory entry

   chdir(savedir);
   closedir(dirp);
   lastDirTime = stats.st_mtime;

//
//  Sort the list
//
   if ( debuglev > 0 ) cout <<"Sorting directory" <<endl;
   dirList.sort();
   return True;

} // End ReadDirectory

/*------------------------------------------------------------------------
 * Method to save the folder.  Used to update status, get rid of deleted
 *    messages and renumber remaining ones.
 */

Boolean
MhFolderC::Save()
{
   if ( !writable ) return False;

   if ( debuglev > 0 ) cout <<"Saving folder: " <<name NL;

   if ( !scanned ) Scan();

//
// Loop through messages
//
   StringC	statusStr;

   unsigned	count  = msgList->size();
   int		wcount = 0;
   int i=0; for (i=0; i<count; i++) {

      if ( wcount%10 == 0 ) {
	 statusStr = "Saving ";
	 statusStr = abbrev;
	 statusStr += " - ";
	 statusStr += wcount;
	 statusStr += " messages";
	 ishApp->mainWin->Message(statusStr);
      }

      MhMsgC	*msg = (MhMsgC*)(*msgList)[i];
      if ( msg->IsDeleted() ) continue;

      if ( msg->IsNew() && ishApp->appPrefs->markNewAsUnread )
	 msg->ClearStatus(MSG_NEW, False/*Don't update index file*/);

      if ( msg->IsChanged() ) {

	 if ( debuglev > 0 )
	    cout <<"   updating file: " <<msg->filename <<endl;
	 if ( Save(msg) ) wcount++;
      }

      else
	 wcount++;

   } // End for each message

   statusStr = "Saving ";
   statusStr = abbrev;
   statusStr += " - ";
   statusStr += wcount;
   statusStr += " messages";
   ishApp->mainWin->Message(statusStr);

//
// Remove deleted messages from display
//
   if ( active && !ishApp->exiting ) {
      ishApp->mainWin->MsgVBox().RemoveItems(*delItemList);
      if ( ishApp->undelWin ) ishApp->undelWin->Clear();
   }

//
// Get rid of deleted messages
//
   count = msgList->size();
   for (i=0; i<count; i++) {

      MhMsgC	*msg  = (MhMsgC*)(*msgList)[i];
      if ( !msg->IsDeleted() ) continue;

      if ( debuglev > 1 )
	 cout <<"   removing message: " <<msg->Number() <<endl;

      msg->deleteFile = True;
      delete msg;
      msgList->replace(NULL, i);
   }

   msgList->removeNulls();
   delItemList->removeAll();

//
// Update display
//
   if ( active && !ishApp->exiting ) {
      if ( SortMgr()->Threaded() ) ishApp->mainWin->MsgVBox().Sort();
      ishApp->mainWin->MsgVBox().Refresh();
   }

//
// Rebuild index file
//
   CreateIndex();
   NewMail();

   SetChanged(False);
   UpdateIcon();

   if ( debuglev > 0 ) cout <<"Folder save complete" <<endl;

   return True;

} // End Save

/*------------------------------------------------------------------------
 * Method to save the specified message
 */

Boolean
MhFolderC::Save(MhMsgC *msg)
{
//
// Create a save file.
//
   StringC	saveFile = msg->filename;
   saveFile += ".ish-sav";
   if ( !msg->WriteFile(saveFile, /*copyHead=*/True, /*allHead=*/True,
				  /*statHead=*/True,
      				  /*addBlank=*/False, /*protectFroms=*/False) )
      return False;

   int	err = 0;

//
// Apply original owner and group to new file
//
   struct stat	idBuf;
   if ( err == 0 && stat(msg->filename, &idBuf) != 0 )
      err = errno;
   if ( err == 0 && chown(saveFile, idBuf.st_uid, idBuf.st_gid) != 0 )
      err = errno;
   if ( err == 0 && chmod(saveFile, idBuf.st_mode) != 0 )
      err = errno;
   if ( err == 0 && unlink(msg->filename) != 0 )
      err = errno;
   if ( err == 0 && rename(saveFile, msg->filename) != 0 )
      err = errno;

   if ( err != 0 ) {
      StringC	errmsg("Could not save message: ");
      errmsg += BaseName(msg->filename);
      errmsg += " in folder: ";
      errmsg += name;
      errmsg += ".\n";
      errmsg += SystemErrorMessage(err);
      halApp->PopupMessage(errmsg);
      return False;
   }

   MsgPartC *body = msg->Body();
   body->headScanned = False;
   body->bodyScanned = False;

   return True;

} // End Save

/*------------------------------------------------------------------------
 * Method to copy the specified message into this folder
 */

Boolean
MhFolderC::AddMessage(MsgC *msg)
{
//
// Before we start, see if the index is valid
//
   Boolean	indexValid = IndexValid();

//
// Create a new file.  GetNextMsgFile generates a unique name
//
   StringC	file;
   GetNextMsgFile(file);

   if ( !msg->WriteFile(file, /*copyHead=*/True, /*allHead=*/True,
			      /*statHead=*/True,
			      /*addBlank=*/False, /*protectFroms=*/False) )
      return False;

//
// If the index is valid, add an entry
//
   if ( indexValid )
      AddIndexEntry(msg->status, msg->BodyBytes(), msg->BodyLines(), msg->Id());

   if ( scanned ) NewMail();
   else	{
      StringC	numStr;
      int	spos = file.RevPosOf('/');
      if ( spos >= 0 ) numStr = file(spos+1, file.length());
      else	       numStr = file;
      dirList.add(atoi(numStr));
      stat(name, &stats);
      UpdateIcon();
   }

   return True;

} // End AddMessage

/*------------------------------------------------------------------------
 * Method to copy the specified message into this folder
 */

Boolean
MhFolderC::AddMessage(StringListC& headList, char *bodyFile)
{
//
// Create a new file.  GetNextMsgFile generates a unique name
//
   StringC	file;
   GetNextMsgFile(file);

   FILE	*fp = fopen(file, "w");
   if ( !fp ) {
      StringC   errmsg("Could not create file: ");
      errmsg += file;
      errmsg += "\n" + SystemErrorMessage(errno);
      halApp->PopupMessage(errmsg);
      return False;
   }

   if ( !WriteHeaders(headList, file, /*addBlank=*/True, fp) )
      return False;

   if ( !CopyFile(bodyFile, file, False/*add blank*/, False/*protect Froms*/,
      		  NULL, fp) )
      return False;

   if ( scanned ) NewMail();
   else {
      stat(name, &stats);
      UpdateIcon();
   }

   return True;

} // End AddMessage

/*------------------------------------------------------------------------
 * Method to check for new mail
 */

Boolean
MhFolderC::NewMail()
{
   if ( debuglev > 1 ) cout <<"Checking for new mail in folder " <<name <<endl;

   Boolean	newMail = False;

//
// If the folder changed while we were asleep, reread it.
//
   if ( changedWhileSleeping && !ishApp->sleeping ) {
      Rescan();
      newMail = HasNewMessages();
      if ( newMail && !ishApp->exiting ) UpdateIcon();
      changedWhileSleeping = False;
      return newMail;
   }

//
// Get the status of the directory
//
   struct stat	newStats;
   if ( stat(name, &newStats) != 0 ) {

      int	err = errno;
      if ( ishApp->sleeping )
	 changedWhileSleeping = True;

//
// If the directory is gone but was there before, this is a problem.
//
      else if ( err == ENOENT && stats.st_size > 0 ) {

	 StringC	errmsg = "Mail folder ";
	 errmsg += name;
	 errmsg += " has disappeared.\n";
	 errmsg += "The last time we checked, it had a size of ";
	 errmsg += (int)stats.st_size;
	 errmsg += " bytes.";
	 halApp->PopupMessage(errmsg, XmDIALOG_WARNING);

	 Reset();
	 MakeDir(name);
	 stat(name, &stats);

      } // End if dir was there before

      else if ( err != ENOENT ) {

	 StringC	errmsg = "Mail folder ";
	 errmsg += name;
	 errmsg += " could not be checked:\n";
	 errmsg += SystemErrorMessage(err);
	 halApp->PopupMessage(errmsg, XmDIALOG_ERROR);
      }

      return False;

   } // End if folder can't be stat'd

//
// Check times
//
   if ( newStats.st_mtime == stats.st_mtime )
      return False;

   if ( debuglev > 1 ) cout <<"Directory changed." <<endl;

   stats = newStats;

   if ( ishApp->sleeping ) {
      changedWhileSleeping = True;
      return True;
   }

//
// Everything looks good, so lets pull in the new data
//
   int       oldMsgCount = NewMsgCount();
   Scan();
   int       newMsgCount = NewMsgCount() - oldMsgCount;
   newMail = (newMsgCount > 0);

   if ( newMail && !ishApp->exiting )
      UpdateIcon();

   if ( debuglev > 1 && newMsgCount > 0 ) {
      cout <<newMsgCount <<" new message";
      if ( newMsgCount > 1 ) cout <<"s";
      cout <<endl;
   }

   return newMail;

} // End NewMail

/*------------------------------------------------------------------------
 * Method to find the next available file name in an MH folder
 */

void
MhFolderC::GetNextMsgFile(StringC& file)
{
//
// Scan the directory just in case
//
   ReadDirectory();

//
// Find the highest number in the directory list
//
   int		maxNum = 0;
   u_int	count  = dirList.size();
   int i=0; for (i=0; i<count; i++) {
      int	num = *dirList[i];
      if ( num > maxNum ) maxNum = num;
   }

//
// Make sure file doesn't already exist by accident
//
   GetMsgFile(name, maxNum+1, file);
   while ( access(file, F_OK) == 0 ) {
      maxNum++;
      GetMsgFile(name, maxNum+1, file);
   }

} // End GetNextMsgFile

#if 0
/*---------------------------------------------------------------
 *  Method to mark the specified message for deletion
 */

void
MhFolderC::MarkForDeletion(MsgC *msg)
{
//
// Try to move the file to the comma version
//
   int	oldNumber = msg->te->Number();
   if ( oldNumber < 0 ) return;

   int	newNumber = -oldNumber;

   StringC	file = MessageFile(newNumber);
   if ( access(file, F_OK) == 0 ) {
      do {
	 newNumber--;
	 file = MessageFile(newNumber);
      } while ( access(file, F_OK) == 0 );
   }

//
// Rename the file and get the directory stats again
//
   msg->RenameFile(file);
   stat(name, &stats);

//
// We also need to change the number in the message summary, the table of
//    contents and the directory listing
//
   if ( msg->Item() ) msg->Item()->SetNumber(newNumber);
   msg->te->SetNumber(newNumber);

   u_int	count = dirList.size();
   int i=0; for (i=0; i<count; i++) {
      int	*num = dirList[i];
      if ( *num == oldNumber ) {
	 *num = newNumber;
	 i = count;
      }
   }

} // End MarkForDeletion

/*---------------------------------------------------------------
 *  Method to undelete the specified message
 */

void
MhFolderC::Undelete(MsgC *msg)
{
//
// Try to move the file to the non-comma version
//
   int	oldNumber = msg->te->Number();
   if ( oldNumber > 0 ) return;

   int	newNumber = -oldNumber;

   StringC	file = MessageFile(newNumber);
   if ( access(file, F_OK) == 0 )
      file = NextMessageFile();

//
// Rename the file and get the directory stats again
//
   msg->RenameFile(file);
   stat(name, &stats);

//
// See what the new number is
//
   int	pos = file.RevPosOf('/');
   file.CutBeg(pos+1);
   newNumber = atoi(file);

//
// We also need to change the number in the message summary, the table of
//    contents and the directory listing
//
   if ( msg->Item() ) msg->Item()->SetNumber(newNumber);
   msg->te->SetNumber(newNumber);

   u_int	count = dirList.size();
   int i=0; for (i=0; i<count; i++) {
      int	*num = dirList[i];
      if ( *num == oldNumber ) {
	 *num = newNumber;
	 i = count;
      }
   }

} // End Undelete

#endif
