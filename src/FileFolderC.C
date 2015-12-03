/*
 *  $Id: FileFolderC.C,v 1.7 2000/08/07 11:05:16 evgeny Exp $
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
#include "FileFolderC.h"
#include "FileMisc.h"
#include "FilePartMsgC.h"
#include "IshAppC.h"
#include "AppPrefC.h"
#include "MainWinC.h"
#include "SortMgrC.h"
#include "UndelWinC.h"
#include "MsgListC.h"
#include "LoginWinC.h"

#include <hgl/SysErr.h>
#include <hgl/System.h>
#include <hgl/VBoxC.h>

#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <utime.h>
#include <signal.h>
#include <errno.h>

#include <sys/stat.h>

// FIXME: more fs access types should be here
#ifdef HAVE_STATVFS // SOLARIS || SVR4 || OSF1 || MIPS
#  include <sys/statvfs.h>
#  define FSTATFS(fd, bufp) fstatvfs(fd, bufp)
#  define STATFS statvfs
#else
#  include <sys/vfs.h>
#  define FSTATFS(fd, bufp) fstatfs(fd, bufp)
#  define STATFS statfs
#endif

#ifdef HAVE___VAL_IN_F_FSID
#  define GET_FS_ID(fsid) (fsid.__val[0])  
#else
#  define GET_FS_ID(fsid) (fsid)
#endif

extern int	debuglev;

/*------------------------------------------------------------------------
 * FileFolderC constructor
 */

FileFolderC::FileFolderC(const char *n, FolderTypeT t, Boolean create)
: FolderC(n, t, create)
{
   openLevel    = 0;
   lockLevel    = 0;
   lockFile     = name + ".lock";
   fp           = NULL;
   changedWhileSleeping = False;

//
// Get the date of the file
//
   if ( stat(name, &stats) != 0 ) {

//
// Create the file if necessary
//
      if ( errno == ENOENT && create ) {
	 if ( debuglev > 0 ) cout <<"Creating folder: " <<name <<endl;
	 if ( !MakeFile(name) ) return;
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
// Make sure this is a file
//
   if ( !S_ISREG(stats.st_mode) && name != "/dev/null" ) {
      StringC	errmsg = name;
      errmsg += " is not a folder.";
      halApp->PopupMessage(errmsg);
      return;
   }

//
// See if file is writable
//
   writable = (access(name, W_OK) == 0);

   if ( !writable ) {
      StringC	errmsg = "Folder \"";
      errmsg += name;
      errmsg += "\" is not writable and will be opened read-only";
      halApp->PopupMessage(errmsg, XmDIALOG_WARNING);
   }

//
// See if lock file can be created
//
   if ( !Lock(True) ) {
      int	err = errno;
      StringC	errmsg = "Folder \"";
      errmsg += name;
      errmsg += "\" cannot be locked and will be opened read-only.";
      errmsg += "\n";
      errmsg += SystemErrorMessage(err);
      halApp->PopupMessage(errmsg, XmDIALOG_WARNING);

   } // End if lock file not created

   else {
      Lock(False);
   }

   opened = True;

} // End FileFolderC constructor

/*------------------------------------------------------------------------
 * FileFolderC destructor
 */

FileFolderC::~FileFolderC()
{
   if ( lockLevel > 0 ) unlink(lockFile);
   CloseFile(True/*force*/);

   if ( delFiles ) unlink(name);
}

/*------------------------------------------------------------------------
 * Method to create mail lock on folder
 */

Boolean
FileFolderC::Lock(Boolean val)
{
   if ( !isInBox ) return True;

   static SIG_PF oldhup  = (SIG_PF)SIG_DFL;
   static SIG_PF oldint  = (SIG_PF)SIG_DFL;
   static SIG_PF oldquit = (SIG_PF)SIG_DFL;
   static SIG_PF oldterm = (SIG_PF)SIG_DFL;
   static SIG_PF oldtstp = (SIG_PF)SIG_DFL;

   if ( !writable ) return True;

   if ( val ) {
	 lockLevel++;
	 if ( lockLevel > 1 && access(lockFile, F_OK) == 0 ) return True;
	    // Already locked
   } else {
	 if ( lockLevel < 1 ) return True;	// Not locked
	 lockLevel--;
	 if ( lockLevel > 0 ) return True;	// Not ready to unlock
   }

   if ( val ) {

      oldhup  = signal(SIGHUP,  (SIG_PF)SIG_IGN);
      oldint  = signal(SIGINT,  (SIG_PF)SIG_IGN);
      oldquit = signal(SIGQUIT, (SIG_PF)SIG_IGN);
      oldterm = signal(SIGTERM, (SIG_PF)SIG_IGN);
      oldtstp = signal(SIGTSTP, (SIG_PF)SIG_IGN);

      Boolean	success = CreateLock(lockFile);
      if ( !success ) {
	 lockLevel--;
	 signal(SIGHUP,  oldhup);
	 signal(SIGINT,  oldint);
	 signal(SIGQUIT, oldquit);
	 signal(SIGTERM, oldterm);
	 signal(SIGTSTP, oldtstp);
      }

      return success;
   }

   else {
      unlink(lockFile);
      signal(SIGHUP,  oldhup);
      signal(SIGINT,  oldint);
      signal(SIGQUIT, oldquit);
      signal(SIGTERM, oldterm);
      signal(SIGTSTP, oldtstp);
   }

   return True;

} // End Lock

/*------------------------------------------------------------------------
 * Method to create a lock file
 */

Boolean
FileFolderC::CreateLock(const char *file)
{
   Boolean	retVal = True;

//
// Loop while file exists.  Try for 60 seconds before giving up.
//
   int		count = 60;
   struct stat	statbuf;
   while ( (stat(file, &statbuf) >= 0) && (count > 0) ) {
      sleep(1);
      count--;
   }

//
// If we timed out, this is probably an old lock file, so just use it.
//
   if ( count > 0 ) {

      int	fd = open(file, O_WRONLY|O_CREAT|O_TRUNC, 0);
      if ( fd < 0 ) {
	 retVal = False;
      }
      else
	 close(fd);
   }

   return retVal;

} // End CreateLock

/*---------------------------------------------------------------
 *  Method to open file
 */

Boolean
FileFolderC::OpenFile()
{
   openLevel++;
   if ( fp ) return True;

   char	*mode;
   if ( writable ) {
      if ( access(name, F_OK) != 0 && errno == ENOENT ) mode = "w+";
      else						mode = "r+";
   }
   else
      mode = "r";

//
// Open the file
//
   if ( debuglev > 1 ) cout <<"Opening folder file " <<name <<endl;
   fp = fopen(name, mode);
   if ( !fp ) {
      StringC   errmsg("Could not open file: ");
      errmsg += name;
      errmsg += "\n" + SystemErrorMessage(errno);
      halApp->PopupMessage(errmsg);
      openLevel = 0;
      return False;
   }

   return True;
}

/*---------------------------------------------------------------
 *  Method to close file
 */

void
FileFolderC::CloseFile(Boolean force)
{
   if ( !fp ) return;

   if ( force ) openLevel = 0;
   else		openLevel--;
   if ( openLevel > 0 ) return;
   if ( openLevel < 0 ) openLevel = 0;

   if ( debuglev > 1 ) cout <<"Closing folder file " <<name <<endl;
   fclose(fp);
   fp = NULL;

//
// Get new stats.  This can cause loss of new mail!!
//
   // An unconditional update of the stats can lose new mail in the
   // following scenario:
   // 1. stats updated
   // 2. new mail arrives
   // 3. folder action results in close, stats updated *without* 
   //		checking for new mail
   // 4. mailcheck now will fail to see the new mail
   //stat(name, &stats);

} // End CloseFile

/*------------------------------------------------------------------------
 * Method to check for new mail
 */

Boolean
FileFolderC::NewMail()
{
   if ( debuglev > 1 )
       cout <<"Checking for new mail in folder " <<name <<endl;

   Boolean	newMail = False;

//
// If the external mods flag is set and we're not sleeping, this is the first
//    check since we woke up.  We need to reread the folder.
//
   if ( changedWhileSleeping && !ishApp->sleeping ) {
      if ( debuglev > 1 )
	 cout <<"Incorporating changes made while asleep." <<endl;
      Rescan();
      newMail = HasNewMessages();
      if ( newMail && !ishApp->exiting ) UpdateIcon();
      changedWhileSleeping = False;
      return newMail;
   }
//
// If there is a POP server specified, run popclient to suck over new mail
//
   if ( isInBox && ishApp->appPrefs->usingPop ) {

      StringC	cmd   = ishApp->appPrefs->popclientCmd;
      Boolean	doPop = True;
      if ( cmd.Contains("%user") || cmd.Contains("%pass") ) {
	 StringC	user, pass;
	 if ( GetLogin(ishApp->appPrefs->popServer, user, pass) ) {
	    cmd.Replace("%user", user);
	    cmd.Replace("%pass", pass);
	 }
	 else {
	    doPop = False;
	 }
      }

      if ( doPop ) {
	 cmd += ' ';
	 cmd += ishApp->appPrefs->popServer;

	 StringC	msg("Checking for mail on POP server ");
	 msg += ishApp->appPrefs->popServer;
	 msg += "...";
	 ishApp->Broadcast(msg);

	 System(cmd);
	 ishApp->Broadcast("");
      }

   } // End if using pop

//
// Open, read the last byte and close the file so that the stat() call
//    works over NFS.
//
   int	fd = open(name, O_RDWR);
   if ( fd >= 0 ) {
      lseek(fd, 1, SEEK_END);
      char	buf;
      read(fd, &buf, 1);
      close(fd);
   }

//
// Get the stats of the mail file
//
   struct stat	newStats;
   if ( stat(name, &newStats) != 0 ) {

      int	err = errno;
      if ( ishApp->sleeping )
	 changedWhileSleeping = True;

      else if ( err == ENOENT && stats.st_size > 0 ) {

//
// If the file is gone but was there before, this is a problem.
//
	 StringC	errmsg = "Mail folder ";
	 errmsg += name;
	 errmsg += " has disappeared.\n";
	 errmsg += "The last time we checked, it had a size of ";
	 errmsg += (int)stats.st_size;
	 errmsg += " bytes.";
	 halApp->PopupMessage(errmsg, XmDIALOG_WARNING);

	 Lock(True);
	 Reset();
	 MakeFile(name);
	 stat(name, &stats);
	 Lock(False);

      } // End if file was there before

      else if ( err != ENOENT ) {

	 StringC	errmsg = "Mail folder ";
	 errmsg += name;
	 errmsg += " could not be checked:\n";
	 errmsg += SystemErrorMessage(err);
	 halApp->PopupMessage(errmsg, XmDIALOG_ERROR);
      }

      else if ( debuglev > 1 )
	 cout <<"Folder not present" <<endl;

      return False;

   } // End if folder can't be stat'd

   if ( debuglev > 1 )
      cout <<"Folder is " <<newStats.st_size <<" bytes at time "
      			  <<newStats.st_mtime <<endl;

//
// Check file times
//
   if ( newStats.st_mtime == stats.st_mtime &&
	newStats.st_size  == stats.st_size ) {
      if ( debuglev > 1 ) cout <<"No change." <<endl;
      return False;
   }

//
// Something changed.  Let's find out what...
//
   if ( newStats.st_size < stats.st_size ) { // File shrunk!

      if ( ishApp->sleeping )
	 changedWhileSleeping = True;

      else {
	 StringC	errmsg = "Mail folder ";
	 errmsg += name;
	 errmsg += " has shrunk from ";
	 errmsg += (int)stats.st_size;
	 errmsg += " bytes to ";
	 errmsg += (int)newStats.st_size;
	 errmsg += " bytes.";
	 halApp->PopupMessage(errmsg, XmDIALOG_WARNING);

	 Rescan();
	 newMail = HasNewMessages();
	 if ( newMail && !ishApp->exiting ) UpdateIcon();

	 stats = newStats;

      } // End if not sleeping

      return newMail;

   } // End if mail file shrunk

   if ( debuglev > 1 ) cout <<"Folder changed." <<endl;

   stats = newStats;

   if ( ishApp->sleeping ) {
      changedWhileSleeping = True;
      return True;
   }

//
// Read in the new data
//
   Lock(True);

   int	oldMsgCount = NewMsgCount();
   if ( scanned ) ScanNew();
   else		  Scan();
   int	newMsgCount = NewMsgCount() - oldMsgCount;
   newMail = (newMsgCount > 0);

   Lock(False);

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
 * Method to save the folder.  Used to update status, get rid of deleted
 *    messages and renumber remaining ones.
 */

Boolean
FileFolderC::Save()
{
   if ( !writable ) return False;

   if ( debuglev > 0 ) cout <<"Saving folder: "<<name NL;

   Lock(True);

   if ( !scanned ) Scan();

//
// Gather all new mail before saving.  This is not necessary if we're using
//    a POP server.
//
   if ( !isInBox || !ishApp->appPrefs->usingPop ) {
      while ( NewMail() );
   }

//
// See how much space is needed.  Also mark new messages as unread at this time
//
   StringC	statusStr;

   statusStr = abbrev;
   statusStr += " - Checking space ...";
   ishApp->mainWin->Message(statusStr);

   u_long	needed = 0;
   unsigned	count = msgList->size();
   int i=0; for (i=0; i<count; i++) {

      FilePartMsgC	*msg  = (FilePartMsgC*)(*msgList)[i];
      if ( msg->IsDeleted() ) continue;

      if ( msg->IsNew() && ishApp->appPrefs->markNewAsUnread )
	 msg->ClearStatus(MSG_NEW, False/*Don't update index file*/);

      needed += msg->SpaceNeeded();

   } // End for each message

   if ( debuglev > 0 ) cout <<"need " <<needed <<" bytes for folder" <<endl;

   StringC	saveFile;
   Boolean	renameOk;
   if ( !CreateSaveFile(needed, saveFile, &renameOk) ) {
      if ( active && !ishApp->exiting ) ishApp->mainWin->MsgVBox().Refresh();
      Lock(False);
      return False;
   }

//
// Loop through messages and write them out
//
   if ( debuglev > 0 ) cout <<"Writing messages" <<endl;

//
// Open original file for reading
//
   if ( !OpenFile() ) {
      Lock(False);
      return False;
   }

//
// Open save file for writing
//
   FILE	*savefp = fopen(saveFile, "w+");
   if ( !fp ) {
      StringC   errmsg("Could not open file: ");
      errmsg += saveFile;
      errmsg += "\n";
      errmsg += SystemErrorMessage(errno);
      halApp->PopupMessage(errmsg);
      CloseFile();
      Lock(False);
      return False;
   }

   int		err    = 0;
   int		wcount = 0;
   Boolean	needRescan = False;
   for (i=0; !err && i<count; i++) {

      if ( wcount%10 == 0 ) {
	 statusStr = abbrev;
	 statusStr += " - Writing messages: ";
	 statusStr += wcount;
	 ishApp->mainWin->Message(statusStr);
      }

      FilePartMsgC	*msg  = (FilePartMsgC*)(*msgList)[i];
      if ( msg->IsDeleted() ) continue;

// If there are any file messages in the folder, we'll have to rescan

      if ( msg->IsFile() ) needRescan = True;

      if ( WriteMessage(msg, savefp) ) wcount++;
      else			       err = errno;

   } // End for each message

   statusStr = abbrev;
   statusStr += " - Writing messages: ";
   statusStr += wcount;
   ishApp->mainWin->Message(statusStr);

   CloseFile();
   fclose(savefp);

//
// See if write was successful.
//
   if ( err ) {
      StringC	errmsg("Could not save folder: ");
      errmsg += name;
      errmsg += ".\n";
      errmsg += SystemErrorMessage(err);
      halApp->PopupMessage(errmsg);
      if ( active && !ishApp->exiting ) ishApp->mainWin->MsgVBox().Refresh();
      Lock(False);
      return False;
   }

//
// If the folder is a link, get the name of the real file.
//
   StringC	realFile = name;
   struct stat	lstats;
   if ( lstat(name, &lstats) == 0 && S_ISLNK(lstats.st_mode) ) {
      realFile = FullPathname(name);
      if ( debuglev > 0 ) cout <<"Folder is a link to: " <<realFile <<endl;
   }

//
// Update original file.
//
   StringC	errmsg;
   if ( unlink(realFile) != 0 ) {
      err = errno;
      errmsg = "Could not unlink folder: ";
      errmsg += realFile;
      errmsg += ".\n";
   }

   if ( !err ) {

      if ( renameOk ) {
	 if ( debuglev > 0 )
	    cout <<"Updating original file using rename()" <<endl;
	 if ( rename(saveFile, realFile) != 0 ) {
	    err = errno;
	    errmsg = "Could not rename folder: ";
	    errmsg += saveFile;
	    errmsg += "\nto: ";
	    errmsg += realFile;
	    errmsg += ".\n";
	 }
      }

      else {

	 if ( debuglev > 0 ) cout <<"Updating original file using cp()" <<endl;
	 StringC	cmd("cp ");
	 cmd += saveFile;
	 cmd += " ";
	 cmd += realFile;

	 err = System(cmd);
	 if ( err != 0 ) {
	    errmsg = "Could not copy folder: ";
	    errmsg += saveFile;
	    errmsg += "\nto: ";
	    errmsg += realFile;
	    errmsg += ".\n";
	 }
	 else
	    unlink(saveFile);
      }
   }

   if ( err != 0 ) {
      errmsg += SystemErrorMessage(err);
      errmsg += "\n\nChanges are in: ";
      errmsg += saveFile;
      halApp->PopupMessage(errmsg);
      if ( active && !ishApp->exiting ) ishApp->mainWin->MsgVBox().Refresh();
      Lock(False);
      return False;
   }

//
// Get fresh stats
//
   if ( stat(name, &stats) != 0 ) stats.st_size  = 0;

//
// Set the access time equal to the modification time to other mailcheck
//    programs don't think there is new mail.
//
   struct utimbuf	times;	// access time, mod time
   times.actime  = stats.st_mtime;
   times.modtime = stats.st_mtime;
   utime(name, &times);

   if ( needRescan ) {
       Rescan();
   }

   else {

//
// Remove deleted messages from display
//
       if ( active && !ishApp->exiting ) {
	  ishApp->mainWin->MsgVBox().RemoveItems(*delItemList);
	  if ( ishApp->undelWin ) ishApp->undelWin->Clear();
       }

//
// Get rid of deleted messages and update message numbers and file offsets
//
       int	num = 1;
       count = msgList->size();
       for (i=0; i<count; i++) {
	  FilePartMsgC	*msg  = (FilePartMsgC*)(*msgList)[i];
	  if ( msg->IsDeleted() ) {
	     if ( debuglev > 1 )
		cout <<"   removing message: " <<msg->Number() <<endl;
	     delete msg;
	     msgList->replace(NULL, i);
	  }
	  else {
	     msg->SetNumber(num++);
	     msg->UpdateOffsets();
	  }
       }

       msgList->removeNulls();
       delItemList->removeAll();

//
// Update display
//
       if ( active && !ishApp->exiting ) {
	  if ( SortMgr()->Threaded() ) ishApp->mainWin->MsgVBox().Sort();
       }

       SetChanged(False);

//
// Rebuild index file
//
       CreateIndex();
       UpdateIcon();

   } // End if we don't need to rescan

   Lock(False);

   if ( debuglev > 0 ) cout <<"Folder save complete" <<endl;

   if ( active && !ishApp->exiting ) ishApp->mainWin->MsgVBox().Refresh();
   
   return True;

} // End Save

/*------------------------------------------------------------------------
 * Method to create a save file.  It will be created in the tmp directory
 *    if there is not enough space on the same filesystem as the folder.
 */

Boolean
FileFolderC::CreateSaveFile(u_long needed, StringC& saveFile, Boolean *renameOk)
{
   saveFile  = name + ".ish-sav";
   *renameOk = True;

   if ( !OpenFile() ) return False;

   struct STATFS	fsbuf;
   int	status = FSTATFS(fileno(fp), &fsbuf);
   u_long	srcfsid = GET_FS_ID(fsbuf.f_fsid);
   u_long	dstfsid = srcfsid;

   CloseFile();

//
// See if we need to use a temp file.
//
   u_long	avail = fsbuf.f_bavail * fsbuf.f_bsize;
   if ( status != 0 || avail < needed ) {
      char	*cs = tempnam(NULL, "save.");
      saveFile = cs;
      free(cs);
      *renameOk = False;
   }

//
// Try to create the save file
//
   FILE	*savefp = fopen(saveFile, "w+");
   if ( !savefp ) {
      int	err = errno;
      StringC	errmsg("Could not create save file for folder: ");
      errmsg += name;
      errmsg += ".\n";
      errmsg += SystemErrorMessage(err);
      halApp->PopupMessage(errmsg);
      return False;
   }
   fclose(savefp);

   if ( !*renameOk ) {

//
// Get the file system status again
//
      status = STATFS(saveFile, &fsbuf);
      srcfsid = GET_FS_ID(fsbuf.f_fsid);
      avail = fsbuf.f_bavail * fsbuf.f_bsize;

//
// See if we can use the temp file
//
      if ( status != 0 || avail < needed ) {

	 StringC	errmsg("There is not enough space in the ");
	 errmsg += "folder directory or the temp directory to save: ";
	 errmsg += name;
	 errmsg += ".\nPlease free up some space before saving this folder.";
	 halApp->PopupMessage(errmsg);

	 unlink(saveFile);
	 return False;
      }

      *renameOk = (dstfsid == srcfsid);

   } // End if not enough space in mail spool

   if ( debuglev > 0 ) cout <<"Save file is: " <<saveFile <<endl;

//
// Set owner and group as before
//
   chown(saveFile, stats.st_uid, stats.st_gid);
   chmod(saveFile, stats.st_mode);

   return True;

} // End CreateSaveFile

/*------------------------------------------------------------------------
 * Method to copy the specified message into this folder
 */

Boolean
FileFolderC::AddMessage(MsgC *msg)
{
   if ( !writable ) return False;

//
// Lock the file so no one else can write it
//
   Lock(True);

//
// Open the file
//
   if ( !OpenFile() ) {
      Lock(False);
      return False;
   }

   fseek(fp, 0, SEEK_END);

//
// Write the message
//
   if ( !WriteMessage(msg, fp) ) {

      CloseFile();
      Lock(False);

      StringC	errmsg("Could not save message to folder: ");
      errmsg += name;
      errmsg += ".\n";
      errmsg += SystemErrorMessage(errno);
      halApp->PopupMessage(errmsg);

      return False;
   }

//
// Read the new message
//
   if ( scanned ) NewMail();

   UpdateIcon();

   CloseFile();

   Lock(False);

   return True;

} // End AddMessage

/*------------------------------------------------------------------------
 * Method to add a new message to this folder
 */

Boolean
FileFolderC::AddMessage(StringListC& headList, char *bodyFile)
{
   if ( !writable ) return False;

//
// Lock the file so no one else can write it
//
   Lock(True);

//
// Open the file
//
   if ( !OpenFile() ) {
      Lock(False);
      return False;
   }

   fseek(fp, 0, SEEK_END);

//
// Write the message
//
   if ( !WriteMessage(headList, bodyFile, fp) ) {

      CloseFile();
      Lock(False);

      StringC	errmsg("Could not save message to folder: ");
      errmsg += name;
      errmsg += ".\n";
      errmsg += SystemErrorMessage(errno);
      halApp->PopupMessage(errmsg);

      return False;
   }

   CloseFile();

//
// Read the new message
//
   if ( scanned ) NewMail();
   UpdateIcon();

   Lock(False);

   return True;

} // End AddMessage
