/*
 * $Id: FolderC.C,v 1.10 2001/07/22 16:04:57 evgeny Exp $
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

#include "FolderC.h"
#include "IshAppC.h"
#include "MainWinC.h"
#include "MsgC.h"
#include "FileMisc.h"
#include "FolderPrefC.h"
#include "AppPrefC.h"
#include "MsgItemC.h"
#include "SortMgrC.h"
#include "SortPrefC.h"
#include "UndelWinC.h"
#include "ReadWinC.h"
#include "PartialMsgDictC.h"
#include "ParamC.h"
#include "MsgPartC.h"
#include "HeaderC.h"
#include "FileMsgC.h"
#include "MsgStatus.h"
#include "ImapFolderC.h"

#include <hgl/SysErr.h>
#include <hgl/VBoxC.h>
#include <hgl/VItemC.h>
#include <hgl/ViewC.h>
#include <hgl/RegexC.h>

#include <Xm/ScrollBar.h>

#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

PartialMsgDictC *FolderC::partialMsgDict = NULL;

/*------------------------------------------------------------------------
 * FolderC constructor
 */

FolderC::FolderC(const char *n, FolderTypeT t,
		 Boolean /*create - used by derived class*/)
{
   type = t;
   if ( !partialMsgDict ) partialMsgDict = new PartialMsgDictC;

   name = n;

   isInBox = (name == ishApp->appPrefs->inBox);
   if ( isInBox ) abbrev = "In-box";
   else {
      abbrev = name;
      ishApp->AbbreviateFolderName(abbrev);
   }

   if ( debuglev > 0 ) cout <<"Opening folder " <<abbrev <<endl;

   writable    = True;
   scanned     = False;
   changed     = False;
   opened      = False;
   active      = False;
   delFiles    = False;
   lastVScroll = 0;

//
// These aren't created until the folder is activated
//
   icon = NULL;

   msgList = new MsgListC;
   msgList->AllowDuplicates(FALSE);
   msgList->SetSorted(FALSE);

   msgItemList = new VItemListC;
   msgItemList->AllowDuplicates(FALSE);
   msgItemList->SetSorted(FALSE);

   delItemList = new VItemListC;
   delItemList->AllowDuplicates(FALSE);
   delItemList->SetSorted(FALSE);

   lastSelList = new VItemListC;
   lastSelList->AllowDuplicates(FALSE);
   lastSelList->SetSorted(FALSE);

//
// Set up index file
//
   if ( type != IMAP_FOLDER ) {

//
// If this folder is a symbolic link, we'll use the index file for the
//    real folder
//
      StringC		realFile = name;
      struct stat	lstats;
      if ( lstat(name, &lstats) == 0 && S_ISLNK(lstats.st_mode) ) {
	 realFile = FullPathname(name);
	 if ( debuglev > 0 ) cout <<"Folder is a link to: " <<realFile <<endl;
      }

//
// Set index file directory to name specified by $ISHIDX, or to
// the same directory as the associated folder if $ISHIDX not set.
//
      CharC	dir  = ishApp->idxDir;
      if ( dir == "" ) dir  = DirName(realFile);
      CharC	base = BaseName(realFile);
      if ( debuglev > 0 ) {
	 cout <<"Dir  name is \"" <<dir  <<"\"" <<endl;
	 cout <<"Base name is \"" <<base <<"\"" <<endl;
      }

//
// Remove old toc file if present
//
      StringC	tocFile(dir);
      tocFile += "/.";
      tocFile += base;
      tocFile += "-toc";

      if ( access(tocFile, F_OK) == 0 ) {
	 if ( debuglev > 0 )
	    cout <<"Removing old toc file \"" <<tocFile <<"\"" <<endl;
	 unlink(tocFile);
      }

//
// Generate name for index file
//
      indexFile = dir;
      indexFile += "/.";
      indexFile += base;
      indexFile += "-idx";
      if ( debuglev > 0 )
	 cout <<"Index file name is \"" <<indexFile <<"\"" <<endl;

      stat(indexFile, &istats);

   } // End if not an IMAP folder

   indexOpenLevel = 0;
   indexfp	  = NULL;
   indexSum       = 0;
   indexSumTime   = 0;

//
// Create a sort manager if this folder has it's own sort keys
//
   RegexC	tmp = name;
   StringC	*keys = ishApp->sortPrefs->folderSortKeys.definitionOf(tmp);
   if ( keys ) sortMgr = new SortMgrC(*keys);
   else	       sortMgr = NULL;

} // End constructor

/*------------------------------------------------------------------------
 * FolderC destructor
 */

FolderC::~FolderC()
{
   Reset();
   CloseIndex(True/*force*/);

   delete lastSelList;
   delete msgItemList;
   delete delItemList;
   delete msgList;

   if ( icon ) {
      ishApp->mainWin->FolderVBox().RemoveItem(*icon);
      delete icon;
   }

   delete sortMgr;

   if ( delFiles && indexFile.size() > 0 ) unlink(indexFile);

} // End destructor

/*------------------------------------------------------------------------
 * Method to delete all messages
 */

void
FolderC::DeleteAllMessages()
{
   u_int	count;
   int		i;

//
// These are deleted when messages are deleted
//
#if 0
   u_int	count = msgItemList->size();
   int i=0; for (i=0; i<count; i++) {
      MsgItemC	*item = (MsgItemC *)(*msgItemList)[i];
      delete item;
   }

   count = delItemList->size();
   for (i=0; i<count; i++) {
      MsgItemC	*item = (MsgItemC *)(*delItemList)[i];
      delete item;
   }
#endif

   count = msgList->size();
   for (i=0; i<count; i++) {
      MsgC	*msg = (*msgList)[i];
      delete msg;
   }

   msgItemList->removeAll();;
   delItemList->removeAll();
   msgList->removeAll();

//
// Remove any partial message entries for this folder
//
   count = partialMsgDict->size();
   for (i=count-1; i>=0; i--) {

      PartialMsgPtr&	pm = partialMsgDict->valOf(i);
      if ( pm->folder != this ) continue;

      StringC&	id = partialMsgDict->keyOf(i);
      partialMsgDict->remove(id);
      delete pm;

   } // End for each incomplete partial

   UpdateIcon();

} // End DeleteAllMessages

/*------------------------------------------------------------------------
 * Method to show this folder in the folder window
 */

void
FolderC::CreateIcon()
{
   if ( icon ) return;
   icon = new VItemC(abbrev);
   icon->SetUserData(this);

   UpdateIcon();

} // End CreateIcon

/*------------------------------------------------------------------------
 * Method to set the icon pixmap according to the current state
 */

void
FolderC::UpdateIcon()
{
   if ( !icon ) return;

   XpmT	lgIcon;
   XpmT smIcon;

   Boolean	showStatus = (isInBox || ishApp->folderPrefs->showStatus);

   Boolean	someNew = showStatus && HasNewMessages();

   if ( isInBox ) {
      if ( active ) {
	 if ( someNew ) {
	    lgIcon = ishApp->folderPrefs->sysOpenNewXpm;
	    smIcon = ishApp->folderPrefs->smSysOpenNewXpm;
	 }
	 else {
	    lgIcon = ishApp->folderPrefs->sysOpenXpm;
	    smIcon = ishApp->folderPrefs->smSysOpenXpm;
	 }
      }
      else {
	 if ( someNew ) {
	    lgIcon = ishApp->folderPrefs->sysClosedNewXpm;
	    smIcon = ishApp->folderPrefs->smSysClosedNewXpm;
	 }
	 else {
	    lgIcon = ishApp->folderPrefs->sysClosedXpm;
	    smIcon = ishApp->folderPrefs->smSysClosedXpm;
	 }
      }
   }

   else {	// Not in-box
      if ( active ) {
	 if ( someNew ) {
	    lgIcon = ishApp->folderPrefs->userOpenNewXpm;
	    smIcon = ishApp->folderPrefs->smUserOpenNewXpm;
	 }
	 else {
	    lgIcon = ishApp->folderPrefs->userOpenXpm;
	    smIcon = ishApp->folderPrefs->smUserOpenXpm;
	 }
      }
      else {
	 if ( someNew ) {
	    lgIcon = ishApp->folderPrefs->userClosedNewXpm;
	    smIcon = ishApp->folderPrefs->smUserClosedNewXpm;
	 }
	 else {
	    lgIcon = ishApp->folderPrefs->userClosedXpm;
	    smIcon = ishApp->folderPrefs->smUserClosedXpm;
	 }
      }
   }

   icon->SetPixmaps(lgIcon, smIcon);
   icon->ValidDropSite(!active && !isInBox && writable);

   if ( ishApp->folderPrefs->showStatus ) {
      char	*tag;
      if ( Changed() ) tag = (char *) (someNew ? "bold-italic" : "bold");
      else	       tag = (char *) (someNew ? "italic"      : "plain");
      icon->SetLabelTag(tag);
   }

} // End UpdateIcon

/*------------------------------------------------------------------------
 * Method to delete all message objects
 */

void
FolderC::Reset()
{
   if ( active ) {
      ishApp->mainWin->MsgVBox().RemoveAllItems();
      if ( ishApp->undelWin ) ishApp->undelWin->Clear();
   }

   DeleteAllMessages();
   scanned = False;

} // End Reset

/*------------------------------------------------------------------------
 * Method to re-scan folder
 */

Boolean
FolderC::Rescan()
{
   Lock(True);

   if ( active ) {

      halApp->BusyCursor(True);

//
// Close any reading windows open to this folder
//
      u_int	count = ishApp->readWinList.size();
      int i=0; for (i=0; i<count; i++) {
	 ReadWinC	*readWin = (ReadWinC*)*ishApp->readWinList[i];
	 if ( readWin->IsShown() && readWin->msg->folder == this )
	    readWin->Hide();
      }
   }

   Reset();
   if ( indexFile.size() > 0 ) unlink(indexFile);

   Boolean	success = Scan();

   if ( active ) {
      CreateIcons();
      ishApp->mainWin->MsgVBox().Refresh();
      halApp->BusyCursor(False);
   }

   ishApp->mainWin->FolderVBox().Refresh();
   ishApp->mainWin->EnableButtons();
   ishApp->mainWin->UpdateTitle();

   Lock(False);

   return success;
}

/*---------------------------------------------------------------
 *  Method to open index file
 */

Boolean
FolderC::OpenIndex()
{
   indexOpenLevel++;
   if ( indexfp ) return True;

//
// Open the file
//
   if ( debuglev > 1 ) cout <<"Opening index file " <<indexFile <<endl;
   indexfp = fopen(indexFile, writable ? "r+" : "r");
   if ( !indexfp ) {
      if ( errno != ENOENT ) {
	 StringC   errmsg("Could not open index file: ");
	 errmsg += indexFile;
	 errmsg += "\n" + SystemErrorMessage(errno);
	 halApp->PopupMessage(errmsg);
      }
      indexOpenLevel = 0;
      return False;
   }

   return True;

} // End OpenIndex

/*---------------------------------------------------------------
 *  Method to close index file
 */

void
FolderC::CloseIndex(Boolean force)
{
   if ( !indexfp ) return;

   if ( force ) indexOpenLevel = 0;
   else		indexOpenLevel--;
   if ( indexOpenLevel > 0 ) return;
   if ( indexOpenLevel < 0 ) indexOpenLevel = 0;

   if ( debuglev > 1 ) cout <<"Closing index file " <<indexFile <<endl;
   fclose(indexfp);
   indexfp = NULL;
}

/*------------------------------------------------------------------------
 * Method to initialize the index file
 */

void
FolderC::ScanIndex()
{
//
// If the folder isn't writable, we won't try to write the index either
//
   if ( !writable ) {
      ReadIndex();
      return;
   }

//
// If the index file doesn't exist, create it from scratch
//
   if ( access(indexFile, F_OK) != 0 ) {
      CreateIndex();
      return;
   }

   Lock(True);

//
// If the index file does exist, open it and read any entries that are present
//
   if ( !OpenIndex() ) {
      Lock(False);
      return;
   }

//
// Loop through each message.
//
   u_int	msgCount = msgList->size();
   Boolean	hitEOF = feof(indexfp);
   int i=0; for (i=0; i<msgCount; i++) {

      MsgC	*msg = (*msgList)[i];

//
// See if the entry for this message can be read
//
      if ( !hitEOF ) {

	 msg->indexOffset = ftell(indexfp);
	 if ( !msg->ReadIndex(indexfp) ) {

//
// If we're at eof, it means we need to create a new entry
//
	    if ( feof(indexfp) )
	       hitEOF = True;

//
// If we're not at eof, it means we have a bogus file
//
	    else {
	       msg->indexOffset = -1;
	       CloseIndex();
	       CreateIndex();
	       Lock(False);
	       return;
	    }
	 }
      }

//
// If we hit the end of the file, we need to create a new entry
//
      if ( hitEOF )
	 msg->indexOffset = AddIndexEntry(msg->status, msg->BodyBytes(),
	 				  msg->BodyLines(), msg->Id());

   } // End for each message

   CloseIndex();
   Lock(False);

} // End ScanIndex

/*------------------------------------------------------------------------
 * Method to bring the index file up to date
 */

void
FolderC::UpdateIndex()
{
//
// IMAP folders don't have an index
//
   if ( IsImap() ) return;

   if ( !writable ) return;

//
// If the index file doesn't exist, create it from scratch
//
   if ( access(indexFile, F_OK) != 0 ) {
      CreateIndex();
      return;
   }

   Lock(True);

   if ( debuglev > 0 )
      cout <<"Updating index file \"" <<indexFile <<"\"" <<endl;

//
// If the index file does exist, open it and read any entries that are present
//
   if ( !OpenIndex() ) {
      Lock(False);
      return;
   }

//
// Loop through each message.  If there isn't an index entry, create one
//
   u_int	msgCount = msgList->size();
   int i=0; for (i=0; i<msgCount; i++) {

      MsgC	*msg = (*msgList)[i];
      if ( msg->indexOffset < 0 )
	 msg->indexOffset = AddIndexEntry(msg->status, msg->BodyBytes(),
	 				  msg->BodyLines(), msg->Id());

   } // End for each message

   CloseIndex();

   Lock(False);

} // End UpdateIndex

/*------------------------------------------------------------------------
 * Method to create the index file
 */

void
FolderC::CreateIndex()
{
   Lock(True);

   CloseIndex(/*force=*/True);

   if ( debuglev > 0 ) cout <<"Creating index file " <<indexFile <<endl;

   indexfp = fopen(indexFile, "w");
   if ( !indexfp ) {
      StringC	errmsg("Could not create index file \"");
      errmsg += indexFile;
      errmsg += "\".\n";
      errmsg += SystemErrorMessage(errno);
      halApp->PopupMessage(errmsg);
      Lock(False);
      return;
   }

   indexOpenLevel = 1;

//
// Loop through messages and create an index entry for each
//
   u_int	msgCount = msgList->size();
   int i=0; for (i=0; i<msgCount; i++) {
      MsgC	*msg = (*msgList)[i];
      msg->indexOffset = AddIndexEntry(msg->status, msg->BodyBytes(),
				       msg->BodyLines(), msg->Id());
   }

   CloseIndex();

   chmod(indexFile, ishApp->folderPrefs->folderFileMask);
   stat(indexFile, &istats);

   Lock(False);

} // End CreateIndex

/*------------------------------------------------------------------------
 * Method to read the index file
 */

void
FolderC::ReadIndex()
{
   if ( debuglev > 0 ) cout <<"Reading index file \"" <<indexFile <<"\"" <<endl;

   if ( !OpenIndex() ) return;

//
// Loop through messages and read index entry for each
//
   u_int	msgCount = msgList->size();
   int i=0; for (i=0; i<msgCount; i++) {

      MsgC	*msg = (*msgList)[i];

//
// Set the offset to the current position
//
      msg->indexOffset = ftell(indexfp);

//
// If the entry cannot be read, ignore the rest
//
      if ( !msg->ReadIndex(indexfp) ) {
	 msg->indexOffset = -1;
	 CloseIndex();
	 return;
      }

   } // End for each message

   CloseIndex();

} // End ReadIndex

/*------------------------------------------------------------------------
 * Method to print information
 */

void
FolderC::Print(ostream& strm) const
{
   strm <<"Folder: " <<name;
   switch (type) {
      case (UNIX_FOLDER):	strm <<" (Unix)";	break;
      case (MH_FOLDER):		strm <<" (MH)";		break;
      case (MMDF_FOLDER):	strm <<" (MMDF)";	break;
      case (IMAP_FOLDER):	strm <<" (IMAP)";	break;
      case (UNKNOWN_FOLDER):	strm <<" (Unknown)";	break;
   }
   if ( !writable ) strm <<" read-only";
   strm <<endl;

   u_int	count = msgList->size();
   int i=0; for (i=0; i<count; i++) {
      MsgC	*msg = (*msgList)[i];
      strm <<*msg <<endl;
   }

} // End Print

/*------------------------------------------------------------------------
 * Method to return total number of messages
 */

int FolderC::MsgCount() const { return msgList->size(); }

/*------------------------------------------------------------------------
 * Method to return n'th message
 */

MsgC *FolderC::MsgWithIndex(u_int n) const { return (*msgList)[n]; }

/*------------------------------------------------------------------------
 * Method to return numbered message
 */

MsgC *FolderC::MsgWithNumber(int n) const
{
   u_int	count = msgList->size();
   int i=0; for (i=0; i<count; i++) {
      MsgC	*msg = (*msgList)[i];
      if ( msg->Number() == n ) return msg;
   }

   return NULL;
}

/*------------------------------------------------------------------------
 * Method to see if there are non-deleted messages with the specified status
 */

Boolean
FolderC::HasMessagesWithStatus(StatusFn *statFn)
{
//
// If we haven't scanned, we might need to bring the index up to date.
//
   if ( !scanned && indexFile.size() > 0 && !IndexValid() ) {
      UpdateIndex();
   } // End if we might need to scan

//
// If we've already scanned this folder, use the message list
//
   if ( scanned ) {
      u_int	count = msgList->size();
      int i=0; for (i=0; i<count; i++) {
	 MsgC	*msg = (*msgList)[i];
	 if ( msg->IsNew() && !msg->IsDeleted() ) return True;
      }
   }

//
// If we haven't scanned, use the index file
//
   else if ( indexFile.size() > 0 ) {

//
// See if we can use the summary value
//
      if ( !IndexSumValid() ) {

	 if ( debuglev > 0 )
	    cout <<"Checking index status: \"" <<indexFile <<"\"" <<endl;

	 indexSum = 0;
	 if ( !OpenIndex() ) return False;

//
// Read lines
//

#define	MAXLINE		255
#define MSG_INDEX_RFMT	"%08x %08x %08x %[^\n]\n"

	 char	line[MAXLINE];
	 char	buf[128]; buf[0] = 0;
	 u_int	statVal;
	 int	bytes;
	 int	lines;
	 while ( fgets(line, MAXLINE-1, indexfp) ) {

	    sscanf(line, MSG_INDEX_RFMT, &statVal, &bytes, &lines, buf);
	    indexSum |= statVal;

	 } // End for each index line

	 CloseIndex();
	 indexSumTime = time(0);

      } // End if index summary value is not valid

      return (*statFn)(indexSum);

   } // End if folder not yet scanned

   return False;

} // End HasMessagesWithStatus

/*------------------------------------------------------------------------
 * Method to see if there are new messages
 */

static Boolean
StatusIsNew(u_int status)
{
   return ((status & MSG_NEW) && !(status & MSG_DELETED));
}

Boolean
FolderC::HasNewMessages()
{
   return HasMessagesWithStatus(StatusIsNew);
}

/*------------------------------------------------------------------------
 * Method to see if there are unread messages
 */

static Boolean
StatusIsUnread(u_int status)
{
   return !(status & (MSG_READ|MSG_DELETED));
}

Boolean
FolderC::HasUnreadMessages()
{
   return HasMessagesWithStatus(StatusIsUnread);
}

/*------------------------------------------------------------------------
 * Method to see if there are deleted messages
 */

static Boolean
StatusIsDeleted(u_int status)
{
   return (status & MSG_DELETED);
}

Boolean
FolderC::HasDeletedMessages()
{
   if ( msgItemList->size() > 0 || delItemList->size() > 0 )
      return (delItemList->size() > 0);
   else
      return HasMessagesWithStatus(StatusIsDeleted);
}

/*------------------------------------------------------------------------
 * Method to return number of new messages
 */

int
FolderC::NewMsgCount() const
{
   int		newCount = 0;
   u_int	count = msgList->size();
   int i=0; for (i=0; i<count; i++) {
      MsgC	*msg = (*msgList)[i];
      if ( !msg->IsDeleted() && msg->IsNew() ) newCount++;
   }

   return newCount;
}

/*------------------------------------------------------------------------
 * Method to return number of unread messages
 */

int
FolderC::UnreadMsgCount() const
{
   int		ucount = 0;
   u_int	count = msgList->size();
   int i=0; for (i=0; i<count; i++) {
      MsgC	*msg = (*msgList)[i];
      if ( !msg->IsDeleted() && !msg->IsRead() && !msg->IsNew() ) ucount++;
   }

   return ucount;
}

/*------------------------------------------------------------------------
 * Method to return number of saved messages
 */

int
FolderC::SavedMsgCount() const
{
   int		scount = 0;
   u_int	count = msgList->size();
   int i=0; for (i=0; i<count; i++) {
      MsgC	*msg = (*msgList)[i];
      if ( msg->IsSaved() ) scount++;
   }

   return scount;
}

/*------------------------------------------------------------------------
 * Method to return number of deleted messages
 */

int
FolderC::DeletedMsgCount() const
{
   int		dcount = 0;
   u_int	count = msgList->size();
   int i=0; for (i=0; i<count; i++) {
      MsgC	*msg = (*msgList)[i];
      if ( msg->IsDeleted() ) dcount++;
   }

   return dcount;
}

/*------------------------------------------------------------------------
 * Method to display messages in message list
 */

void
FolderC::Activate()
{
   if ( active || !icon ) return;

   ishApp->Broadcast("");

   ishApp->mainWin->FolderVBox().View()->ScrollToItem(*icon);

   VBoxC&	vbox = ishApp->mainWin->MsgVBox();

   vbox.RemoveAllItems();

//
// Read the messages if they haven't already been read.
// If they have been read, check for new mail unless we're using a POP server.
//
   Lock(True);
   if ( !scanned )
      Scan();
   else if ( !isInBox || !ishApp->appPrefs->usingPop ) {
      if ( IsImap() ) {
         ImapFolderC   *If = (ImapFolderC*)this;
         if ( !If->Select() ) return;
      }
      NewMail();
   }
   Lock(False);

   if ( scanned ) {
      active = True;
//
//    Update the folder icon
//
      UpdateIcon();
//
//    Create any message icons needed and select any items from last time
//
      CreateIcons();
      vbox.SelectItemsOnly(*lastSelList);
   }

//
// Refresh the message list
//
   vbox.Refresh();

//
// Add a time-out proc to Reset the scroll bars to their last positions unless
//    we scrolled to a new message.
//
   XtAppAddTimeOut(ishApp->context, 100/*ms*/,
		   (XtTimerCallbackProc)ResetScrollBar, (XtPointer)this);

} // End Activate

/*------------------------------------------------------------------------
 * Method to remove messages from display
 */

void
FolderC::Deactivate()
{
   if ( !active || !icon ) return;

   active = False;

//
// Save the position of the vertical scroll bar
//
   VBoxC&	vbox = ishApp->mainWin->MsgVBox();
   lastVScroll  = vbox.VScrollValue();
   *lastSelList = vbox.SelItems();

   vbox.RemoveAllItems();
   vbox.Refresh();

   if ( ishApp->undelWin ) ishApp->undelWin->Clear();

   UpdateIcon();

} // End Deactivate

/*---------------------------------------------------------------
 *  Method to create a display icons for messages
 */

void
FolderC::CreateIcons()
{
   Boolean      showCount = (ishApp->mainWin->IsShown() &&
			    /*!ishApp->mainWin->IsIconified() &&*/
			    msgItemList->size() == 0);

//
// Create any message icons needed
//
   u_int	count = msgList->size();
   StringC	statusStr;
   int i=0; for (i=0; i<count; i++) {

//
// Update progress message if necessary
//
      if ( showCount && (i % 10) == 0 ) {
	 statusStr = abbrev;
	 statusStr += " - Creating message icons: ";
	 statusStr += i;
	 ishApp->mainWin->Message(statusStr);
      }

//
// Create message icon
//
      MsgC	*msg = (*msgList)[i];
      if ( msg->icon ) continue;

      msg->CreateIcon();
      InsertInThread(msg->icon);

      if ( msg->IsDeleted() ) {
	 //cout <<"Adding message " <<msg->Number()
	 //     <<" to deleted list with icon " <<msg->icon <<endl;
	 delItemList->add(msg->icon);
      }
      else
	 msgItemList->add(msg->icon);

   } // End for each message

//
// Update progress message if necessary
//
   if ( showCount ) {
      statusStr = abbrev;
      statusStr += " - Creating message icons: ";
      statusStr += i;
      ishApp->mainWin->Message(statusStr);
   }

//
// Display items
//
   VBoxC&	vbox = ishApp->mainWin->MsgVBox();

   vbox.SetSorted(False);
   vbox.AddItems(*msgItemList);

   if ( ishApp->appPrefs->hideDeleted ) {
      if ( ishApp->undelWin ) ishApp->undelWin->SetItems(*delItemList);
   }
   else
      vbox.AddItems(*delItemList);

   vbox.SetSorted(True);

//
// See if we need to display new messages
//
   if ( ishApp->appPrefs->scrollToNew ) {

      MsgItemC		*firstNew = NULL;
      MsgItemC		*lastNew  = NULL;
      VItemListC&	list      = vbox.VisItems();
      u_int		count     = list.size();
      int i=0; for (i=0; i<count; i++) {
	 MsgItemC	*item = (MsgItemC*)list[i];
	 if ( !item->IsDeleted() && !item->IsRead() ) {
	    if ( !firstNew ) firstNew = item;
	    lastNew = item;
	 }
      }

//
// Scroll to the last then the first to display as many new messages
//    as possible
//
      if ( lastNew ) {
	 vbox.View()->ScrollToItem(*lastNew);
	 vbox.View()->ScrollToItem(*firstNew);
      }

   } // End if scrolling to new messages

   vbox.Refresh();

   UpdateIcon();

   ishApp->mainWin->ClearMessage();

} // End CreateIcons

/*---------------------------------------------------------------
 *  Timer proc to reset vertical scroll bar to its previous position
 */

void
FolderC::ResetScrollBar(FolderC *This, XtIntervalId*)
{
   if ( !This->icon ) return;

   VBoxC&	vbox = ishApp->mainWin->MsgVBox();
   MsgC	*lastNew = (ishApp->appPrefs->scrollToNew ? This->NewestMsg() : (MsgC*)NULL);
   if ( lastNew )
      vbox.View()->ScrollToItem(*lastNew->icon);

   else {

      Widget	sb = vbox.VScrollBar();
      int	val, size, inc, pinc, max;

      XmScrollBarGetValues(sb, &val, &size, &inc, &pinc);
      XtVaGetValues(sb, XmNmaximum, &max, NULL);
      if ( This->lastVScroll + size > max ) This->lastVScroll = max - size;

      XmScrollBarSetValues(sb, This->lastVScroll, size, inc, pinc,
      			   /*notify*/True);
   }

} // End ResetScrollBar

/*------------------------------------------------------------------------
 * Method to add the message item to a thread.
 */

void
FolderC::InsertInThread(MsgItemC *newItem)
{
//
// Try to find a message with the same thread
//
   u_int	count = msgList->size();
   MsgC		*newMsg   = newItem->msg;
   CharC	newThread = newMsg->Thread();
   int i=0; for (i=0; i<count; i++) {

//
// See if the thread matches
//
      MsgC	*msg = (*msgList)[i];
      if ( msg != newMsg && newThread.Equals(msg->Thread(), IGNORE_CASE) ) {

//
// Find the last item in the thread
//
	 MsgItemC	*item = msg->icon;
	 if ( !item ) return;

	 while ( item->nextInThread ) item = item->nextInThread;

//
// Add the new item to the thread
//
	 item->nextInThread = newItem;
	 newItem->prevInThread = item;

	 if ( debuglev > 1 ) cout <<"Inserted message " <<newMsg->Number()
	    			  <<" after " <<msg->Number() <<endl;
	 return;
      }
   }

} // End InsertInThread

/*------------------------------------------------------------------------
 * Method to update the fields for all message items
 */

void
FolderC::UpdateFields()
{
   if ( !icon ) return;

//
// Loop through all messages
//
   u_int	count = msgItemList->size();
   int i=0; for (i=0; i<count; i++) {
      MsgItemC	*item = (MsgItemC *)(*msgItemList)[i];
      item->LoadFields();
   }

   count = delItemList->size();
   for (i=0; i<count; i++) {
      MsgItemC	*item = (MsgItemC *)(*delItemList)[i];
      item->LoadFields();
   }

} // End UpdateFields

/*------------------------------------------------------------------------
 * Method to return a pointer to the newest message in the folder
 */

MsgC*
FolderC::NewestMsg() const
{
//
// Return the most recent new message
//
   u_int	count = msgList->size();
   int i=count-1; for (i=count-1; i>=0; i--) {
      MsgC	*msg = (*msgList)[i];
      if ( msg->IsNew() && !msg->IsDeleted() )
	 return msg;
   }

   return NULL;

} // End NewestMsg

/*------------------------------------------------------------------------
 * Method to give us a new sort manager
 */

void
FolderC::SetSortMgr(SortMgrC *sort)
{
   delete sortMgr;
   sortMgr = sort;

//
// If there is a new sort manager, look up the keys for this folder
//
   RegexC	tmp = name;
   if ( sortMgr ) {

      StringC	*keys = ishApp->sortPrefs->folderSortKeys.definitionOf(tmp);

//
// If there is an existing entry, just update it.  If not, create a new one
//
      if ( keys )
	 *keys = sortMgr->KeyString();
      else
	 ishApp->sortPrefs->folderSortKeys.add(tmp, sortMgr->KeyString());

   } // End if there is a new sort manager

//
// If there is no new sort manager, remove the keys for this folder
//
   else
      ishApp->sortPrefs->folderSortKeys.remove(tmp);

} // End SetSortMgr

/*------------------------------------------------------------------------
 * Method to apply the sort specified by the given string
 */

void
FolderC::Sort(CharC keyString)
{
   if ( sortMgr ) sortMgr->Set(keyString);
   else		  sortMgr = new SortMgrC(keyString);
}

/*------------------------------------------------------------------------
 * Method to return the sort manager used for this folder
 */

SortMgrC*
FolderC::SortMgr() const
{
   if ( sortMgr ) return sortMgr;
   else		  return ishApp->mainWin->sortMgr;
}

/*------------------------------------------------------------------------
 * Method to move a message from the message list to the deleted list.
 */

void
FolderC::MsgDeleted(MsgC *msg)
{
//
// Update the lists
//
   if ( msg->icon ) {
      //cout <<"Adding message " <<msg->Number()
	//   <<" to deleted list with icon " <<msg->icon <<endl;
      delItemList->add(msg->icon);
      msgItemList->remove(msg->icon);
   }

   SetChanged(True);
   UpdateIcon();

} // End MsgDeleted

/*------------------------------------------------------------------------
 * Method to update folder icon when message status changes
 */

void
FolderC::MsgStatusChanged(MsgC*)
{
   SetChanged(True);
   if ( scanned ) UpdateIcon();
}

/*------------------------------------------------------------------------
 * Method to move the specified messages from the message list to the deleted
 *    list.
 */

void
FolderC::ItemsDeleted(VItemListC& list)
{
   Boolean	listChanged = False;
   u_int	count = list.size();
   int i=0; for (i=0; i<count; i++) {

      MsgItemC	*item = (MsgItemC *)list[i];
      if ( msgItemList->includes(item) ) {
	 //cout <<"Adding message " <<item->msg->Number()
	 //     <<" to deleted list with icon " <<item <<endl;
	 delItemList->add(item);
	 listChanged = True;
      }
   }

   if ( !listChanged ) return;

//
//
// Remove them from the message list
//
   for (i=0; i<count; i++) {
      MsgItemC	*item = (MsgItemC *)list[i];
      int	index = msgItemList->indexOf(item);
      if ( index != msgItemList->NULL_INDEX )
	 msgItemList->replace(NULL, index);
   }
   msgItemList->removeNulls();

   SetChanged(True);
   UpdateIcon();

} // End ItemsDeleted

/*------------------------------------------------------------------------
 * Function used to sort the message items by number.  This is not used
 *    to sort the visible list, only the internal list
 */

static int
SortByNumber(const void *a, const void *b)
{
   MsgItemC	*mia = *(MsgItemC **)a;
   MsgItemC	*mib = *(MsgItemC **)b;
   int	result = 0;

   if	   ( mia->msg->Number() < mib->msg->Number() ) result = -1;
   else if ( mia->msg->Number() > mib->msg->Number() ) result =  1;

   return result;
}

/*------------------------------------------------------------------------
 * Method to move a message from the deleted list to the message list.
 */

void
FolderC::MsgUndeleted(MsgC *msg)
{
//
// Update the lists
//
   if ( msg->icon ) {
      //cout <<"Removing message " <<msg->Number()
	//   <<" from deleted list with icon " <<msg->icon <<endl;
      delItemList->remove(msg->icon);
      msgItemList->add(msg->icon);
      msgItemList->sort(SortByNumber);
   }

   SetChanged(True);
   UpdateIcon();

} // End MsgUndeleted

/*------------------------------------------------------------------------
 * Method to move the specified messages from the deleted list to the message
 *    list.
 */

void
FolderC::ItemsUndeleted(VItemListC& list)
{
   Boolean	listChanged = False;
   u_int	count = list.size();
   int i=0; for (i=0; i<count; i++) {

      MsgItemC	*item = (MsgItemC *)list[i];
      if ( delItemList->includes(item) ) {
	 msgItemList->add(item);
	 listChanged = True;
      }
   }

   if ( !listChanged ) return;

//
//
// Remove them from the deleted list
//
   for (i=0; i<count; i++) {
      MsgItemC	*item = (MsgItemC *)list[i];
      int	index = delItemList->indexOf(item);
      //cout <<"Removing message " <<item->msg->Number()
	//   <<" from deleted list with icon " <<item <<endl;
      //cout <<"Adding NULL to deleted list in position " <<index <<endl;
      delItemList->replace(NULL, index);
   }
   //cout <<"Removing NULLs from deleted list" <<endl;
   delItemList->removeNulls();

//
// Re-sort the message list
//
   msgItemList->sort(SortByNumber);
   SetChanged(True);
   UpdateIcon();

} // End ItemsUndeleted

/*------------------------------------------------------------------------
 * Function to build a list of headers from a character array
 */

static HeaderC*
CutHeaders(CharC& data)
{
   if ( data.Length() == 0 ) return NULL;

   if ( data.StartsWith('\n') ) {
      data.CutBeg(1);
      return NULL;
   }

//
// Read a header line
//
   StringC	headStr = data.NextWord(0, '\n');
   data.CutBeg(headStr.size()+1);

   if ( headStr.size() == 0 ) return NULL;

//
// Read continuation lines
//
   CharC	lineStr = data.NextWord(0, '\n');
   while ( isspace(lineStr[0]) ) {
      headStr += lineStr;
      data.CutBeg(lineStr.Length()+1);
      lineStr = data.NextWord(0, '\n');
   }

//
// Create a new header object
//
   HeaderC	*newHead = new HeaderC(headStr);

//
// Read and set links to additional headers
//
   newHead->next = CutHeaders(data);

   return newHead;

} // End CutHeaders

/*------------------------------------------------------------------------
 * Method to handle a partial mime message
 */

Boolean
FolderC::AddPartial(MsgC *msg)
{
//
// Parse the content-type header
//
   MsgPartC	*body     = msg->QuickBody();	// Just need headers
   ParamC	*idParam  = body->Param("id");
   ParamC	*numParam = body->Param("number");

   if ( !idParam || !numParam ) return False;

   ParamC	*totParam = body->Param("total");

//
// See if id exists in partial dictionary
//
   StringC		idVal = idParam->val;
   PartialMsgPtr	*ptr  = partialMsgDict->definitionOf(idVal);
   PartialMsgC		*pm   = (ptr ? *ptr : (PartialMsgC*)NULL);

//
// If no matching id was found, create a new entry
//
   if ( !pm ) {
      pm = new PartialMsgC;
      pm->folder = this;
      partialMsgDict->add(idVal, pm);
   }

//
// Insert part in list
//
   StringC	numVal    = numParam->val;
   int		partNum   = atoi(numVal);
   int		partIndex = partNum-1;

//
// Add blank entries until the list is large enough to hold this part
//
   MsgC	*tmp = NULL;
   while ( pm->msgList.size() <= partIndex ) pm->msgList.add(tmp);

//
// Store this message at its index
//
   pm->msgList.replace(msg, partIndex);
   
//
// See if all the parts have arrived.
//
   if ( pm->expected == 0 && totParam ) {
      StringC	totVal = totParam->val;
      pm->expected = atoi(totVal);
   }

   if ( pm->msgList.size() != pm->expected ) return True;

//
// Count the number of non-null pointers.  If we hit any nulls, we can return
//    immediately
//
   int	received = 0;
   MsgC	*msgP;
   int i=0; for (i=0; i<pm->expected; i++) {
      msgP = pm->msgList[i];
      if ( !msgP ) return True;
      received++;
   }

//
// If we don't have all the parts, return
//
   if ( received != pm->expected ) return True;

//
// Create a temporary file for the complete message
//
   char	*cs = tempnam(NULL, "part.");
   StringC	tmpfile(cs);
   free(cs);

   FILE *fp = fopen(tmpfile, "w+");
   if ( !fp ) {
      StringC	errmsg("Could not create temp file \"");
      errmsg += tmpfile;
      errmsg += "\" while combining partial messages.\n";
      errmsg += SystemErrorMessage(errno);
      halApp->PopupMessage(errmsg);
      return False;
   }

//
// Create a list of headers for the new message.  Start with the headers from
// part 1.
//
   StringListC	headList;
   MsgC		*part1      = pm->msgList[0];
   HeaderC	*extHeaders = part1->Headers();

//
// Write all external headers except for "content-*", "subject", "message-id",
//    "encrypted" and "mime-version".
//
   Boolean	error = False;
   HeaderC	*head = extHeaders;
   CharC	headStr;
   CharC	nlStr("\n");
   while ( head && !error ) {

      Boolean	skipIt = (head->key.StartsWith("Content-", IGNORE_CASE) ||
			  head->key.Equals("Subject",      IGNORE_CASE) ||
			  head->key.Equals("Message-Id",   IGNORE_CASE) ||
			  head->key.Equals("Encrypted",    IGNORE_CASE) ||
			  head->key.Equals("Mime-Version", IGNORE_CASE));

      if ( !skipIt ) {
	 headStr = head->full;
	 error = (!headStr.WriteFile(fp) || !nlStr.WriteFile(fp));
      }

      head = head->next;
   }

//
// Separate the internal headers from the first body
//
   StringC	bodyStr;
   part1->GetBodyText(bodyStr);
   CharC	realBody(bodyStr);

   HeaderC	*intHeaders = NULL;
   if ( !bodyStr.StartsWith('\n') ) 	// No headers if starts with '\n'
      intHeaders = CutHeaders(realBody);

//
// Write all internal headers that match: "content-*", "subject", "message-id",
//    "encrypted" and "mime-version".
//
   head = intHeaders;
   while ( head && !error ) {

      Boolean	writeIt = (head->key.StartsWith("Content-", IGNORE_CASE) ||
			   head->key.Equals("Subject",      IGNORE_CASE) ||
			   head->key.Equals("Message-Id",   IGNORE_CASE) ||
			   head->key.Equals("Encrypted",    IGNORE_CASE) ||
			   head->key.Equals("Mime-Version", IGNORE_CASE));

      if ( writeIt ) {
	 headStr = head->full;
	 error = (!headStr.WriteFile(fp) || !nlStr.WriteFile(fp));
      }

      head = head->next;
   }

//
// Write a blank line
//
   if ( !error ) error = !nlStr.WriteFile(fp);

//
// Write the "non-header" portion of the body of the first part.
//
   if ( !error ) error = !realBody.WriteFile(fp);

//
// Write the bodies for the remaining parts
//
   u_int	count = pm->expected;
   for (i=1; !error && i<count; i++) {
      msgP = pm->msgList[i];
      error = !msgP->WriteBody(fp, /*addBlank=*/False, /*protectFroms=*/False);
   }

   long	size = 0;
   if ( !error ) {
      fseek(fp, 0, SEEK_END);
      size = ftell(fp);
   }
   fclose(fp);

   if ( error ) {
      StringC	errmsg("Could not write temp file \"");
      errmsg += tmpfile;
      errmsg += "\" while combining partial messages.\n";
      errmsg += SystemErrorMessage(errno);
      halApp->PopupMessage(errmsg);
      unlink(tmpfile);
      return False;
   }

//
// Create a new message item from that file and add it to the folder
//
   FileMsgC *newMsg = new FileMsgC(tmpfile, 0, (int)size, /*del=*/True, this);
   msgList->add(newMsg);
   newMsg->SetNumber(msgList->size());

   VBoxC&    vbox   = ishApp->mainWin->MsgVBox();

   if ( active ) {

      newMsg->CreateIcon();
      InsertInThread(newMsg->icon);
      msgItemList->add(newMsg->icon);

      vbox.AddItem(*newMsg->icon);
   }

//
// Mark the partials for deletion
//
   VItemListC	list;
   count = pm->expected;
   for (i=0; i<count; i++) {
      msgP = pm->msgList[i];
      msgP->SetDeleted();
      if ( active ) list.add(msgP->icon);
      else	    MsgDeleted(msgP);
   }

   if ( active ) {
      ishApp->mainWin->DeleteItems(list);
      if ( ishApp->appPrefs->scrollToNew )
	 vbox.View()->ScrollToItem(*newMsg->icon);
      vbox.Refresh();
   }

   ishApp->mainWin->GetNewMail(this);

   partialMsgDict->remove(idVal);
   delete pm;

   UpdateIndex();

   return True;

} // End AddPartial

/*------------------------------------------------------------------------
 * Method to insert a new message in place of another.  This could happen
 *    after a MIME conversion or an edit.
 */

void
FolderC::ReplaceMsg(MsgC *oldMsg, MsgC *newMsg)
{
//
// Insert the new message in the message list
//
   int	index = msgList->indexOf(oldMsg);
   msgList->insert(newMsg, index+1);

//
// Create a new message item from that file and add it to the folder
//
   VBoxC&    vbox = ishApp->mainWin->MsgVBox();
   if ( active ) {

      newMsg->CreateIcon();
      InsertInThread(newMsg->icon);
      msgItemList->add(newMsg->icon);

      vbox.AddItem(*newMsg->icon);
   }

//
// Mark the old message for deletion
//
   oldMsg->SetDeleted();

   if ( active ) {
      ishApp->mainWin->DeleteMsg(oldMsg);
      vbox.View()->ScrollToItem(*newMsg->icon);
      vbox.Refresh();
   }

   else
      MsgDeleted(oldMsg);

   UpdateIndex();

} // End ReplaceMsg

/*------------------------------------------------------------------------
 * Method to return value of changed flag
 */

Boolean
FolderC::Changed()
{
   return (changed || HasDeletedMessages());
}

/*------------------------------------------------------------------------
 * Method to update value of changed flag
 */

void
FolderC::SetChanged(Boolean val)
{
   if ( changed == val ) return;

   changed = val;

//
// Use the Changed() call because it checks other things
//
   Boolean	someNew = HasNewMessages();
   char		*tag;
   if ( Changed() ) tag = (char *) (someNew ? "bold-italic" : "bold");
   else		    tag = (char *) (someNew ? "italic"      : "plain");

   icon->SetLabelTag(tag);
}

/*------------------------------------------------------------------------
 * Method to determine if the index is up-to-date
 */

Boolean
FolderC::IndexValid()
{
   Boolean	valid = False;
#if 0
   struct stat	istats;

   if ( stat(indexFile, &istats) == 0 ) {
#endif
      time_t	folderTime = ModTime();
      valid = (istats.st_mtime >= folderTime);
#if 0
   }
#endif

   return valid;
}

/*------------------------------------------------------------------------
 * Method to determine if the index summary value is up-to-date
 */

Boolean
FolderC::IndexSumValid()
{
   Boolean	valid = False;
#if 0
   struct stat	istats;

   if ( stat(indexFile, &istats) == 0 )
#endif
      valid = (indexSumTime >= istats.st_mtime);

   return valid;
}

/*------------------------------------------------------------------------
 * Method to add a new entry to the index file
 */

long
FolderC::AddIndexEntry(u_int status, int bytes, int lines, char *id)
{
   if ( !OpenIndex() ) return -1;

//
// Don't write out the Viewed bit
//
   status = (status & (u_int)~MSG_VIEWED);

   fseek(indexfp, 0, SEEK_END);
   long	offset = ftell(indexfp);
   fprintf(indexfp, MSG_INDEX_WFMT, status, bytes, lines, id);

   CloseIndex();

   if ( indexOpenLevel == 0 )
      stat(indexFile, &istats);

//
// Add this status to the sum
//
   indexSum |= status;
   indexSumTime = time(0);

   return offset;

} // End AddIndexEntry

/*------------------------------------------------------------------------
 * Method to update an existing entry in the index file
 */

Boolean
FolderC::UpdateIndexEntry(MsgC *msg)
{
//
// IMAP folders don't have an index
//
   if ( msg->IsImap() ) return True;

   if ( !OpenIndex() ) return False;

   if ( fseek(indexfp, msg->indexOffset, SEEK_SET) != 0 ) {
      CloseIndex();
      return False;
   }

//
// Don't write out the Viewed bit
//
   u_int	status = (msg->status & (u_int)~MSG_VIEWED);
   int		bytes  = msg->BodyBytes();
   int		lines  = msg->BodyLines();
   char		*id    = msg->Id();

   fprintf(indexfp, MSG_INDEX_WFMT, status, bytes, lines, id);

   CloseIndex();

   if ( indexOpenLevel == 0 )
      stat(indexFile, &istats);

//
// Updating invalidates the sum
//
   indexSum     = 0;
   indexSumTime = 0;

   return True;

} // End UpdateIndexEntry

