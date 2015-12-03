/*
 *  $Id: ImapFolderC.C,v 1.12 2000/12/31 14:36:02 evgeny Exp $
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
#include "ImapFolderC.h"
#include "ImapServerC.h"
#include "ImapMsgC.h"
#include "FileMisc.h"
#include "IshAppC.h"
#include "AppPrefC.h"
#include "MainWinC.h"
#include "UndelWinC.h"
#include "SortMgrC.h"
#include "ImapMisc.h"
#include "MsgListC.h"
#include "MsgItemC.h"

#include <hgl/RegexC.h>
#include <hgl/CharC.h>
#include <hgl/StringListC.h>
#include <hgl/VBoxC.h>

#include <time.h>

/*------------------------------------------------------------------------
 * ImapFolderC constructor
 */

ImapFolderC::ImapFolderC(const char *n, Boolean create)
: FolderC(n, IMAP_FOLDER, create)
{
   server = NULL;
   opened = False;
   long serverPort;
   StringC userName;
   Boolean imaps;

//
// Get server name and folder name etc
//
   StringC folderName;
   if ( !ParseImapName(name, serverName, &serverPort, &imaps, userName, folderName) ) {
      return;
   }
   
   if ( isInBox ) {
      name = "INBOX";
   } else {
      name = folderName;
   }

//
// Connect to the server
//
   server = FindImapServer(serverName, serverPort, imaps, userName);

   if ( !server || !server->authenticated ) return;

   if ( !server->Exists(name) ) {
      // Create the folder if necessary (but we can't try to create INBOX)
      if ( !isInBox && create ) {

	 StringListC	output;
	 if ( !server->Create(name, output) ) return;

         // Check response

	 u_int	count = output.size();
	 int i;
	 for (i=0; i<count; i++) {
	     CharC	resp = *output[i];
	     if ( resp.StartsWith("NO ") ) return;
	 }
      } else {
         return;
      }
   }

   opened = True;

} // End ImapFolderC constructor

/*------------------------------------------------------------------------
 * ImapFolderC destructor
 */

ImapFolderC::~ImapFolderC()
{
   if ( delFiles && server ) {
      StringListC	output;
      server->Delete(name, output);
   }
}

/*------------------------------------------------------------------------
 * Method to build message list
 */

Boolean
ImapFolderC::Scan()
{
   if ( !server || !server->authenticated ) return False;

   time_t       timeIn = 0;
   if ( debuglev > 0 ) {
      cout <<"Entering ImapFolderC(" <<BaseName(name) <<")::Scan" <<endl;
      timeIn = time(0);
   }

//
// Select the folder
//
   msgCount = 0;
   //int	newCount = 0;

   StringListC	output;
   if ( !server->Select(name, output) ) return False;

   scanned = True;

//
// Read SELECT responses
//
   u_int	count = output.size();
   int i=0; for (i=0; i<count; i++) {

      CharC	resp = *output[i];
      if ( resp.StartsWith("* ") ) {
	 resp.CutBeg(2);
	 resp.Trim();
      }

      if ( resp.StartsWith("FLAGS", IGNORE_CASE) ||
	   resp.StartsWith("NO", IGNORE_CASE) )
	 ;
      else if ( resp.EndsWith("EXISTS", IGNORE_CASE) ) {
	 resp.CutEnd(6);
	 resp.Trim();
	 StringC	num = resp;
	 msgCount = atoi(num);
      }

      else if ( resp.EndsWith("RECENT", IGNORE_CASE) ) {
	 resp.CutEnd(6);
	 resp.Trim();
	 StringC	num = resp;
	 //newCount = atoi(num);
      }

      else if ( resp.StartsWith("OK", IGNORE_CASE) ) {
	 resp.CutBeg(2);
	 resp.Trim();
	 
         if ( resp.Contains("IGNORING DUPLICATE", IGNORE_CASE) ) {
            if ( debuglev > 0 ) {
               cout << "Enabling IMAP_BUG_DUPLICATE_SELECT workaround" <<endl;
            }
            server->bugs |= IMAP_BUG_DUPLICATE_SELECT;
            // Now, as the bug workaround has been enabled,
            // enter the same function again
            return Scan();
         }
         
         writable = !resp.Contains("[READ-ONLY]", IGNORE_CASE);
      }

      else {
	 server->Unexpected(resp);
      }

   } // End for each reply

   StringC	statusStr;
#if 0
   Boolean	showCount = (ishApp->mainWin->IsShown() &&
   			    !ishApp->mainWin->IsIconified());
#else
//
// This change was made to make the message status information show
// up while the folder is being opened at start-up time. There ought to
// be a better way...
//
   Boolean	showCount = True;
#endif

//
// Get message information
//
   fetchNeeded = True;
   for (i=0; i<msgCount; i++) {

//
// Update progress message if necessary
//
      if ( showCount ) {
	 statusStr = abbrev;
	 statusStr += " - Scanning messages: ";
	 statusStr += (int)msgList->size() + 1;
	 ishApp->mainWin->Message(statusStr);
      }

//
// Add a message to the list
//
      ImapMsgC	*msg = new ImapMsgC(this, server, i+1);
      msgList->add(msg);
   }

//
// Update progress message if necessary
//
   if ( showCount ) {
      statusStr = abbrev;
      statusStr += " - Scanning messages: ";
      statusStr += (int)msgList->size();
      ishApp->mainWin->Message(statusStr);
   }

//
// Clean up
//
   server->FetchFlush( fetchTag );
   SetChanged(False);
   UpdateIcon();

   ishApp->mainWin->ClearMessage();

   if ( debuglev > 0 ) {
      time_t	timeOut = time(0);
      cout <<"Leaving ImapFolderC(" <<BaseName(name) <<")::Scan after "
      	   <<(timeOut-timeIn) <<" seconds" <<endl;
   }

   return True;

} // End Scan

/*------------------------------------------------------------------------
 * Method to save the folder.  Used to get rid of deleted messages and
 *    renumber remaining ones.
 */

Boolean
ImapFolderC::Save()
{
   if ( !server || !server->authenticated ) return False;

   StringListC	output;

   StringC	saveFolder;
   if ( server->folder != name ) {
      saveFolder = server->folder;
      if ( !server->Select(name, output) ) return False;
   }

   if ( !server->Expunge(output) ) return False;

//
// Remove deleted messages from display
//
   if ( active && !ishApp->exiting ) {
      ishApp->mainWin->MsgVBox().RemoveItems(*delItemList);
      if ( ishApp->undelWin ) ishApp->undelWin->Clear();
   }

//
// Loop through message list and remove any deleted messages
//
   u_int	count  = msgList->size();
   u_int	number = 1;
   int i=0; for (i=0; i<count; i++) {

      ImapMsgC	*msg = (ImapMsgC*)(*msgList)[i];
      if ( msg->IsDeleted() ) {
	 delete msg;
	 msgList->replace(NULL, i);
      }

      else {

	 msg->SetNumber(number);
	 number++;

	 if ( msg->IsNew() && ishApp->appPrefs->markNewAsUnread )
	    msg->ClearNew();

      }

   } // End for each message

   msgList->removeNulls();
   delItemList->removeAll();

//
// Update display
//
   if ( active && !ishApp->exiting ) {
      if ( SortMgr()->Threaded() ) ishApp->mainWin->MsgVBox().Sort();
      ishApp->mainWin->MsgVBox().Refresh();
   }

   if ( saveFolder.size() > 0 ) {
      if ( !server->Select(saveFolder, output) ) return False;
   }

   SetChanged(False);
   UpdateIcon();

   return True;

} // End Save

/*------------------------------------------------------------------------
 * Method to check for new mail
 */

Boolean
ImapFolderC::NewMail()
{
   if ( !server || !server->authenticated ) return False;

    StringC	saveFolder;
    if ( server->folder != name ) {
	saveFolder = server->folder;
	StringListC	output;
	if ( !server->Select(name, output) ) return False;
    }

   int		newCount = 0;
   Boolean	newMail  = False;

//
// Send NOOP command
//
   StringListC	output;
   output.AllowDuplicates(True);
   server->Noop(output);

   u_int	count = output.size();
   msgCount =	msgList->size();
   u_int	numExpunges = 0;
   Boolean	updateTitle = False;
   int i=0; for (i=0; i<count; i++) {

      CharC	resp = *output[i];
      if ( resp.StartsWith("* ") ) {
	 resp.CutBeg(2);
	 resp.Trim();
      }

      if ( resp.StartsWith("FLAGS", IGNORE_CASE) ||
	   resp.StartsWith("NO", IGNORE_CASE) )
	 ;

      else if ( resp.EndsWith("EXISTS", IGNORE_CASE) ) {
	 resp.CutEnd(6);
	 resp.Trim();
	 StringC	num = resp;
	 msgCount = atoi(num);
      }

      else if ( resp.EndsWith("RECENT", IGNORE_CASE) ) {
	 resp.CutEnd(6);
	 resp.Trim();
	 StringC	num = resp;
	 newCount = atoi(num);
      }

      else if ( resp.Contains("FETCH (FLAGS", IGNORE_CASE) ) {
	 /* The flags of a message changed on the server */
	 CharC msgNum = resp.NextWord(0);
	 CharC flags = resp.NextWord(resp.PosOf("FLAGS ") + 7, ')');
	 if ( debuglev > 0 )
	    cout << "Change flags on msg " << msgNum << " to " << flags << endl;
	 // Get the Message
	 int x=atoi(msgNum.Addr()) - 1;
	 /*Imap*/MsgC *msg = /*(ImapMsgC*)*/(*msgList)[x];
	 /* No flags means that message should be flagged as unread */
	 if (flags.Len() == 0)
	    msg->ClearStatus(MSG_READ, False);
	 // Parse and set flags
	 while (flags.Len() > 0) {
	    // Trim off the \ at the beginning
	    flags.CutBeg(1);
	    flags.Trim();
	    if (flags.StartsWith("Seen", IGNORE_CASE) ) {
	       msg->ClearStatus(MSG_NEW, False);
	       msg->SetStatus(MSG_READ, False);
	       updateTitle=True;
	       flags.CutBeg(4);
	       flags.Trim();
	    } else if (flags.StartsWith("Deleted", IGNORE_CASE) ) {
#if 0
// methinks this is quick, efficient but incomplete as it does not adjust
// "save" button sensitivies
	       msg->SetStatus(MSG_DELETED, False);
	       SetChanged(True);
	       updateTitle=True;
#else
// this is likely The Right Thing To Do"(tm) but it does have the ugly side
// effect that if you are reading a message and it is deleted by another,
// your reading window will move on to the next message.  hmmmmm
	       ishApp->mainWin->DeleteMsg(msg);
#endif
	       flags.CutBeg(7);
	       flags.Trim();
	    } else if (flags.StartsWith("Recent", IGNORE_CASE) ) {
	       msg->SetStatus(MSG_NEW, False);
	       flags.CutBeg(6);
	       flags.Trim();
	    } else if (flags.StartsWith("Answered", IGNORE_CASE) ) {
	       msg->SetStatus(MSG_REPLIED, False);
	       flags.CutBeg(8);
	       flags.Trim();
	    } else if (flags.StartsWith("Saved", IGNORE_CASE) ) {
	       msg->SetStatus(MSG_SAVED, False);
	       flags.CutBeg(5);
	       flags.Trim();
	    } else {
	       int flagLen = flags.PosOf(' ', 0, IGNORE_CASE);
	       if (flagLen == -1)
		  flagLen = flags.Len();
	       CharC flag(flags.Addr(), flagLen);
	       cerr << "Unknown msg flag returned from server \"\\" << flag <<
		  "\"\n";
	       flags.CutBeg(flagLen);
	       flags.Trim();
	    }
	 }
	 if ( msg->icon )
	    msg->icon->UpdateStatus();
      }

      else if ( resp.EndsWith("EXPUNGE", IGNORE_CASE) ) {
	 // A message was removed from the IMAP folder
	 numExpunges++;
	 resp.CutEnd(7);
	 resp.Trim();
	 StringC	msgNum = resp;
	 int x=atoi(msgNum) - 1;
	 ImapMsgC *msg = (ImapMsgC*)(*msgList)[x];
	 // if it's deleted and expunged quickly there will be no
	 // FETCH (FLAGS (\Deleted)) message, so we have to do this here and
	 // depend on the list weeding out duplicates (which it does happily
	 delItemList->add(msg->icon);
	 // Delete the expunged message
        if ( active && !ishApp->exiting )
	    ishApp->mainWin->MsgVBox().RemoveItem(*msg->icon);
         delItemList->remove(msg->icon);
	 delete msg;
	 msgList->remove(x);
	 // Renumber remaining nessages
         u_int	count  = msgList->size();
         u_int	number = x+1;
	 u_int	j;
         for (j=x; j<count; j++) {
	    msg = (ImapMsgC*)(*msgList)[j];
	    msg->SetNumber(number);
	    number++;
         } // End for each message
      }

      else if ( resp.StartsWith("BYE ", IGNORE_CASE) ) {
         return False;
      }

      else if ( !resp.StartsWith("OK", IGNORE_CASE) ) {
	 server->Unexpected(resp);
      }

   } // End for each reply

   // Did at least one expunge, update display
   if (numExpunges) {
      if ( ishApp->undelWin )
	 ishApp->undelWin->Clear();

      //
      // Update display
      //
      if ( active && !ishApp->exiting ) {
	 if ( SortMgr()->Threaded() )
	    ishApp->mainWin->MsgVBox().Sort();
         ishApp->mainWin->MsgVBox().Refresh();
      }
    
      updateTitle = True;
      SetChanged(False);
      UpdateIcon();
   }

    if ( msgCount == msgList->size() ) {
	if ( saveFolder.size() > 0 ) {
	    StringListC	output;
	    server->Select(saveFolder, output);
	}
	if (updateTitle)
	   ishApp->mainWin->UpdateTitle();   
	return False;
    }

//
// See if the mailbox grew
//
   if ( msgCount > msgList->size() ) {

      VBoxC&	vbox      = ishApp->mainWin->MsgVBox();
      MsgItemC	*firstNew = NULL;
      MsgItemC	*lastNew  = NULL;

//
// Get message information
//
      fetchNeeded = True;
      for (i=msgList->size(); i<msgCount; i++) {

	 ImapMsgC	*msg = new ImapMsgC(this, server, i+1);
	 msgList->add(msg);

//
// Create a message icon for this message
//
	 if ( active ) {

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

      } // End for each new message

      scanned = True;
      server->FetchFlush( fetchTag );

      if ( active ) {

//
// Scroll to the last then the first to display as many new messages
//    as possible
//
	 if ( lastNew && ishApp->appPrefs->scrollToNew ) {
	    vbox.View()->ScrollToItem(*lastNew);
	    vbox.View()->ScrollToItem(*firstNew);
	 }

	 vbox.Refresh();
	 if (updateTitle)
	    ishApp->mainWin->UpdateTitle();   
      }

   } // End if mailbox grew

   newMail = (newCount > 0);
   if ( newMail && !ishApp->exiting ) UpdateIcon();
   if ( saveFolder.size() > 0 ) server->Select(saveFolder, output);

   return newMail;

} // End NewMail

/*------------------------------------------------------------------------
 * Method to copy the specified message into this folder
 */

Boolean
ImapFolderC::AddMessage(MsgC *msg)
{
   if ( !server || !server->authenticated ) return False;

   StringListC	output;

//
// If the source message is in an IMAP folder on the same server, we can use
//    the copy command.
//
   if ( msg->type == IMAP_MSG ) {

      ImapFolderC	*srcFolder = (ImapFolderC*)msg->folder;
      if ( srcFolder->server->name == server->name ) {

//
// Select source folder if necessary
//
	 StringC	saveFolder;
	 if ( server->folder != srcFolder->name ) {
	    saveFolder = server->folder;
	    if ( !server->Select(srcFolder->name, output) ) return False;
	 }

//
// Send COPY command
//
	 if ( !server->Copy(msg->Number(), name, output) ) return False;

	 if ( saveFolder.size() > 0 ) {
	    if ( !server->Select(saveFolder, output) ) return False;
	 }
//
// Make sure the folder's message list is up to date
//
         if (scanned) {
           Rescan();
//
// Make sure the folder is marked as needing to be saved
//
           Changed();
         }

	 return True;

      } // End if message is on same server as this folder

   } // End if adding an IMAP message

//
// We must use the APPEND command for non-IMAP messages or messages that are
//    on a different IMAP server
//
   StringC	data;
   msg->GetHeaderText(data);
   data += '\n';
   msg->GetBodyText(data);

   if ( !server->Append(name, data, output) ) return False;
//
// Make sure the folder's message list is up to date
//
   if (scanned) {
     Rescan();
//
// Make sure the folder is marked as needing to be saved
//
     Changed();
   }

   return True;

} // End AddMessage

/*------------------------------------------------------------------------
 * Method to copy the specified message into this folder
 */

Boolean
ImapFolderC::AddMessage(StringListC& headList, char *bodyFile)
{
   if ( !server || !server->authenticated ) return False;

//
// We must use the APPEND command
//
   StringListC	output;
   if ( !server->Append(name, headList, bodyFile, output) ) {
      StringC	errmsg = "Error while trying to copy message to folder ";
      errmsg += name;
      errmsg += " on IMAP server ";
      errmsg += serverName;
      errmsg += "\n\n";
      int i;
      for (i=0; i<output.size(); i++) {
         errmsg += *output[i];
         errmsg += "\n";
      }
      halApp->PopupMessage(errmsg, XmDIALOG_WARNING);

      return False;
   }
//
// Make sure the folder's message list is up to date
//
   if (scanned) {
     Rescan();
//
// Make sure the folder is marked as needing to be saved
//
     Changed();
   }

   return True;

} // End AddMessage

/*------------------------------------------------------------------------
 * Activate folder on Imap server
 */
 
Boolean
ImapFolderC::Select()
{
   if ( !server || !server->authenticated ) return False;

   StringListC  output;
   if ( !server->Select(name, output) ) {

      StringC	errmsg = "Mail folder: ";
      errmsg += name;
      errmsg += " on IMAP server: ";
      errmsg += serverName;
      errmsg += " could not be selected.";
      halApp->PopupMessage(errmsg, XmDIALOG_WARNING);

      return False;
   }

   return True;

} // End Select
