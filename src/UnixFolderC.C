/*
 * $Id: UnixFolderC.C,v 1.3 2000/08/07 11:05:17 evgeny Exp $
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
#include "UnixFolderC.h"
#include "UnixMsgC.h"
#include "FileMisc.h"
#include "HeaderC.h"
#include "IshAppC.h"
#include "MainWinC.h"
#include "MsgItemC.h"
#include "AddressC.h"
#include "AppPrefC.h"
#include "MsgListC.h"

#include <hgl/CharC.h>
#include <hgl/RegexC.h>
#include <hgl/SysErr.h>
#include <hgl/VBoxC.h>
#include <hgl/ViewC.h>

#include <Xm/Xm.h>

#include <time.h>
#include <unistd.h>

RegexC  *UnixFolderC::fromPat = NULL;

#define MAXLINE 255

/*------------------------------------------------------------------------
 * UnixFolderC constructor
 */

UnixFolderC::UnixFolderC(const char *n, Boolean create)
: FileFolderC(n, UNIX_FOLDER, create)
{
   if ( !fromPat ) {

      char	*nonWhite = "\\([^ \t\n]+\\)";
      char	*white    = "[ \t]+";
      char	*alpha    = "[a-zA-Z]+";
      char	*num      = "[0-9]+";
      char	*time     = "[0-9][0-9]:[0-9][0-9]:?[0-9]?[0-9]?";
      char	*etc      = ".*\n?";

      StringC	pat = "From ";
      pat += nonWhite;	// Address
      pat += white;
      pat += alpha;	// Weekday
      pat += white;
      pat += alpha;	// Month
      pat += white;
      pat += num;	// Daynum
      pat += white;
      pat += time;	// Time
      pat += white;
      pat += etc;	// [tz | offset] year [remote from addr]  -or-
      			// year [tz | offset] [remote from addr]

      fromPat = new RegexC(pat);
   }

   if ( !opened ) return;

//
// File should exist at this point.  Make sure it's a Unix folder.  It should
//    start with a From line
//
   if ( stats.st_size > 0 ) {

      if ( !OpenFile() ) return;

      char	line[MAXLINE+1];
      fgets(line, MAXLINE, fp);
      if ( !fromPat->match(line) ) {
	 StringC	errmsg = name;
	 errmsg += " is not a Unix-format mail folder.\n";
	 errmsg += "A Unix folder begins with a line starting with \"From \".";
	 halApp->PopupMessage(errmsg);
	 opened = False;
	 return;
      }

      CloseFile();
   }

} // End constructor

/*------------------------------------------------------------------------
 * UnixFolderC destructor
 */

UnixFolderC::~UnixFolderC()
{
}

/*------------------------------------------------------------------------
 * Method to extract messages from file
 */

Boolean
UnixFolderC::Scan()
{
   time_t	timeIn = 0;
   if ( debuglev > 0 ) {
      cout <<"Entering UnixFolderC(" <<BaseName(name) <<")::Scan" <<endl;
      timeIn = time(0);
   }

   Boolean	showCount = (ishApp->mainWin->IsShown() &&
   			    !ishApp->mainWin->IsIconified());

   if ( !OpenFile() ) return False;
   fseek(fp, 0, SEEK_SET);

   Boolean	haveIndex = OpenIndex();
   Boolean	indexEOF  = False;
   if ( haveIndex ) fseek(indexfp, 0, SEEK_SET);

//
// Read lines until we hit the first From line
//
   char		line[MAXLINE+1];
   CharC	lineStr;
   StringC	statusStr;
   while ( fgets(line, MAXLINE, fp) ) {

      lineStr = line;
      if ( lineStr.StartsWith("From ") && fromPat->match(line) ) {

//
// Update progress message if necessary
//
	 if ( showCount /*&& (msgList->size() % 10) == 0*/ ) {
	    statusStr = abbrev;
	    statusStr += " - Scanning messages: ";
	    statusStr += (int)msgList->size();
	    ishApp->mainWin->Message(statusStr);
	 }

	 UnixMsgC *msg = new UnixMsgC(this, msgList->size()+1, 
	 			      (u_int)ftell(fp), line);
	 msgList->add(msg);

//
// If we have the index file, try to read the info for this message
//
         if ( haveIndex && !indexEOF ) {

//
// Try to read the index
//
	    msg->indexOffset = ftell(indexfp);
	    if ( !msg->ReadIndex(indexfp) ) {

	       if ( !feof(indexfp) )
		  ftruncate(fileno(indexfp), msg->indexOffset);

	       msg->indexOffset = -1;
	       indexEOF = True;
	    }

	 } // End if we have an index

//
// If this is a partial message, process it
//
         if ( msg->IsPartial() ) AddPartial(msg);

//
// Move to the end of this message
//
	 fseek(fp, msg->BodyOffset()+msg->BodyBytes(), SEEK_SET);

      } // End if we got a From line

   } // End for each line in file

//
// Update progress message if necessary
//
   if ( showCount ) {
      statusStr = abbrev;
      statusStr += " - Scanning messages: ";
      statusStr += (int)msgList->size();
      ishApp->mainWin->Message(statusStr);
   }

   if ( haveIndex ) CloseIndex();
   CloseFile();

   scanned = True;
   SetChanged(False);

   if ( haveIndex ) UpdateIndex();
   else		    ScanIndex();

   UpdateIcon();
   ishApp->mainWin->ClearMessage();

   if ( debuglev > 0 ) {
      time_t	timeOut = time(0);
      cout <<"Leaving UnixFolderC(" <<BaseName(name) <<")::Scan after "
	   <<(timeOut-timeIn) <<" seconds" <<endl;
   }

   return True;

} // End Scan

/*------------------------------------------------------------------------
 * Method to extract new messages from file
 */

Boolean
UnixFolderC::ScanNew()
{
   time_t	timeIn = 0;
   if ( debuglev > 0 ) {
      cout <<"Entering UnixFolderC(" <<BaseName(name) <<")::ScanNew" <<endl;
      timeIn = time(0);
   }

   if ( !OpenFile() ) return False;

   char		line[MAXLINE+1];
   CharC	lineStr;

//
// Seek to where we think the last Unix message should start.  There could
//    also be external File messages in the folder.
//
   UnixMsgC	*lastMsg = NULL;
   u_int	count = msgList->size();
   int i=count-1; for (i=count-1; !lastMsg && i>=0; i--) {
      MsgC	*msg = (*msgList)[i];
      if ( msg->IsUnix() ) lastMsg = (UnixMsgC*)msg;
   }

   if ( lastMsg ) {

      fseek(fp, lastMsg->HeadOffset(), SEEK_SET);

//
// Read a line and make sure it's the first header in the message
//
      Boolean	error = False;
      if ( !fgets(line, MAXLINE, fp) ) {
	 error = True;
      }

      else {

	 HeaderC	*firstHead = lastMsg->Headers();
	 if ( firstHead ) {
	    lineStr = line;
	    error = !lineStr.StartsWith(firstHead->key);
	 }
	 else {
	    error = (strlen(line) > 1);
	 }
      }

      if ( error ) {

	 StringC	errmsg = "Mail folder ";
	 errmsg += name;
	 errmsg += " appears to be different than I think it should be.\n";
	 errmsg += "I will now re-read it.";

	 if ( !ishApp->sleeping )
	    halApp->PopupMessage(errmsg, XmDIALOG_WARNING);

	 CloseFile();
	 Rescan();
	 return True;
      }

   } // End if there is a last message
   
   VBoxC&	vbox = ishApp->mainWin->MsgVBox();
   MsgItemC	*firstNew = NULL;
   MsgItemC	*lastNew  = NULL;

//
// Seek to where we think the first new message should start
//
   int	newPos = 0;
   if ( lastMsg ) newPos = lastMsg->BodyOffset() + lastMsg->BodyBytes();
   fseek(fp, newPos, SEEK_SET);

//
// Read lines until we hit the first From line
//
   while ( fgets(line, MAXLINE, fp) ) {

      lineStr = line;
      if ( lineStr.StartsWith("From ") && fromPat->match(line) ) {

	 UnixMsgC *msg = new UnixMsgC(this, msgList->size()+1,
	 			      (u_int)ftell(fp), line);
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

//
// Reset to the end of the last message since the next From line was already
//   read by the message.

	 fseek(fp, msg->BodyOffset()+msg->BodyBytes(), SEEK_SET);

      } // End if start of new message

   } // End for each new line

   CloseFile();

   scanned = True;

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
   }

   UpdateIndex();

   if ( debuglev > 0 ) {
      time_t	timeOut = time(0);
      cout <<"Leaving UnixFolderC(" <<BaseName(name) <<")::ScanNew after "
	   <<(timeOut-timeIn) <<" seconds" <<endl;
   }

   return True;

} // End ScanNew

/*------------------------------------------------------------------------
 * Function to build a From line for a message
 */

static void
GetFromLine(MsgC *msg, StringC& fromLine)
{
   fromLine = "From ";

   AddressC	*from = msg->From();
   if ( from ) fromLine += from->addr;
   else	       fromLine += "unknown";

   fromLine += ' ';
   time_t	epochTime = msg->Time();
   fromLine += (char*)ctime(&epochTime);

   if ( fromLine.EndsWith('\n') ) fromLine.CutEnd(1);

} // End GetFromLine

/*------------------------------------------------------------------------
 * Method to write a message to this open folder
 */

Boolean
UnixFolderC::WriteMessage(MsgC *msg, FILE *fp)
{
#if 0
//
// Before we start, see if the index is valid
//
   Boolean	indexValid = IndexValid();
   u_int	msgOff     = (u_int)ftell(fp);
#endif

   StringC	fromLine;
   if ( msg->type == UNIX_MSG ) fromLine = ((UnixMsgC*)msg)->fromLine;
   else				GetFromLine(msg, fromLine);
   if ( !fromLine.WriteFile(fp) ) return False;

   CharC	nl("\n");
   if ( !nl.WriteFile(fp)       ) return False;

   msg->headWritePos = ftell(fp);
   if ( !msg->WriteHeaders(fp)  ) return False;

   msg->bodyWritePos = ftell(fp);
   if ( !msg->WriteBody(fp, True/*add blank*/, True/*protect Froms*/) )
      return False;

#if 0
//
// If the index is valid, add an entry
//
   if ( indexValid ) {

      long	indexOff = AddIndexEntry(msg->status, msg->BodyBytes(),
      					 msg->BodyLines(), msg->Id());

//
// Display the new message
//
      if ( scanned ) {

	 UnixMsgC	*newMsg = new UnixMsgC(this, msgList->size()+1, msgOff,
	 				       fromLine, indexOff);
	 msgList->add(newMsg);

//
// Create a message icon for this message
//
	 if ( active ) {

	    newMsg->CreateIcon();
	    InsertInThread(newMsg->icon);

	    msgItemList->add(newMsg->icon);
	    ishApp->mainWin->MsgVBox().AddItem(*newMsg->icon);

	 } // End if folder is active

//
// If this is a partial message, process it
//
         if ( newMsg->IsPartial() ) AddPartial(newMsg);

      } // End if folder has been scanned
   } // End if the index file is good

   else if ( scanned )
      ScanNew();
#endif

   return True;

} // End WriteMessage

/*------------------------------------------------------------------------
 * Method to write a message to this open folder
 */

Boolean
UnixFolderC::WriteMessage(StringListC& headList, char *bodyFile, FILE *fp)
{
//
// Write "From " line
//
   StringC	fromStr = "From ";
   fromStr += ishApp->userAtHost;
   fromStr += " ";

   time_t	now = time(0);
   fromStr += ctime(&now);
   if ( !fromStr.WriteFile(fp) ) return False;

   if ( !WriteHeaders(headList, name, /*addBlank=*/True, fp) )
      return False;

   if ( !CopyFile(bodyFile, name, True/*add blank*/, True/*protect Froms*/,
      		  NULL, fp) )
      return False;

   return True;
}
