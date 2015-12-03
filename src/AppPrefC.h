/*
 *  $Id: AppPrefC.h,v 1.2 2000/06/29 10:53:29 evgeny Exp $
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
#ifndef _AppPrefC_h
#define _AppPrefC_h

#include "PrefC.h"

#include <hgl/StringListC.h>
#include <hgl/CallbackC.h>

#include <time.h>

enum MsgSaveTypeT {
   SAVE_TO_FOLDER,
   SAVE_BY_USER,
   SAVE_BY_ADDRESS,
   SAVE_BY_PATTERN,
   SAVE_BY_PATTERN_OR_USER,
   SAVE_BY_PATTERN_OR_ADDRESS
};

class AppPrefWinC;

class AppPrefC : public PrefC {

//
// Private data.  We don't want anyone setting these directly.
//
   AppPrefWinC	*prefWin;

//
// As typed by the user
//
   struct {
      StringC	folderDir;	// Keep folders here
      StringC	saveFile;	// Use this file as default folder
      StringC	saveDir;	// Use this as save directory
      StringC	automountRoot;
      StringC	archiveFolder;
   } orig;

//
// Fully expanded
//
   StringC	folderDir;	// Keep folders here
   StringC	saveFile;	// Use this file as default folder
   StringC	saveDir;	// Use this as save directory
   StringC	automountRoot;
   StringC	archiveFolder;

public:

//
// Public data
//
   StringC	inBox;	        // In-box folder name
   MsgSaveTypeT	saveType;
   Boolean	deleteSaved;	// Automatically delete saved messages
   Boolean	hideDeleted;	// Automatically hide deleted messages
   StringC	printCmd;
   int		checkInterval;	// How often to check for new mail
   int		bellVolume;
   Boolean	scrollToNew;	// Auto scroll to new messages
   Boolean	rememberWindows;// Save window sizes and positions
   				//    when exiting
   Boolean	markNewAsUnread;// Change new messages to unread when
   				//    saving
   Boolean	archiveOnSave;	// Move read messages to archive folder
   				//    when saving in-box
   Boolean	usingImap;	// True if user wants to talk to IMAP server
   StringC	imapServer;
   Boolean	usingPop;	// True if user wants to talk to POP server
   Boolean	pwFieldFirst;	// True if POP/IMAP server login window
				// shows Password field first, False means 
				// show the Name field first.
   StringC	popServer;
   StringC	popclientCmd;

   int			recentFolderCount;	// Number to remember
   StringListC		recentFolders;
   time_t		recentFolderTime;
   CallbackListC	recentChangedCalls;	// When list changes

//
// Public methods
//
    AppPrefC();
   ~AppPrefC();

   void		SetFolderDir(const char*);
   void		SetSaveFile(const char*);
   void		SetSaveDir(const char*);
   void		SetAutomountRoot(const char*);
   void		SetArchiveFolder(const char*);

   StringC&	FolderDir()		{ return folderDir; }
   StringC&	SaveFile()		{ return saveFile; }
   StringC&	SaveDir()		{ return saveDir; }
   StringC&	AutomountRoot()		{ return automountRoot; }
   StringC&	ArchiveFolder()		{ return archiveFolder; }

   StringC&	OrigFolderDir()		{ return orig.folderDir; }
   StringC&	OrigSaveFile()		{ return orig.saveFile; }
   StringC&	OrigSaveDir()		{ return orig.saveDir; }
   StringC&	OrigAutomountRoot()	{ return orig.automountRoot; }
   StringC&	OrigArchiveFolder()	{ return orig.archiveFolder; }

   void		AddRecentFolder(StringC);
   void		AddRecentChangeCallback(CallbackFn *fn, void *data)
      { AddCallback(recentChangedCalls, fn, data); }
   void		Edit(Widget);
   void		ExpandValues();
   Boolean	WriteDatabase();
   Boolean	WriteFile();
   Boolean	WriteRecentFolders();	// To file
   Boolean	WriteWindowPositions();	// To file
   Boolean	WriteVersion();		// To file
};

#endif // _AppPrefC_h
