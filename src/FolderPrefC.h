/*
 *  $Id: FolderPrefC.h,v 1.1.1.1 2000/04/25 13:49:02 fnevgeny Exp $
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
#ifndef _FolderPrefC_h
#define _FolderPrefC_h

#include "PrefC.h"
#include "FolderListC.h"

#include <hgl/StringListC.h>
#include <hgl/PixmapC.h>	// For XpmT

enum FolderViewTypeT {
   FOLDER_VIEW_LARGE,
   FOLDER_VIEW_SMALL,
   FOLDER_VIEW_NAME
};

class FolderPrefWinC;

class FolderPrefC : public PrefC {

//
// Private data.  We don't want anyone setting these directly.
//
   FolderListC		openFolderList;	// Folders opened
   FolderListC		fileFolderList;	// Folders accessed but not opened
   FolderPrefWinC	*prefWin;
   FolderViewTypeT	folderViewType;
   Boolean		sortFolders;	// If sorted alphabetically

//
// As typed by the user
//
   struct {
      StringC		initFolderStr;	// Display these initially
   } orig;

//
// Expanded
//
   StringListC		initFolderList;	// Display these initially

   FolderTypeT		QueryFolderType();

public:

//
// Public data
//
   Boolean		rememberFolders; // Update initial folders as they
   					 //    are opened and closed
   Boolean		usingMh;	// True if user wants MH folders
   Boolean		usingUnix;	// True if user wants Unix folders
   Boolean		usingMmdf;	// True if user wants MMDF folders
   FolderTypeT		defFolderType;	// What type of folders to create
   Boolean		showStatus;	// For user folders, requires scan
   int			folderFileMask;	// file creation mask (def: 0600)

//
// Folder and mailbox pixmaps
//
   XpmT			sysClosedXpm;
   XpmT			sysClosedNewXpm;
   XpmT			sysOpenXpm;
   XpmT			sysOpenNewXpm;
   XpmT			userClosedXpm;
   XpmT			userClosedNewXpm;
   XpmT			userOpenXpm;
   XpmT			userOpenNewXpm;
   XpmT			smSysClosedXpm;
   XpmT			smSysClosedNewXpm;
   XpmT			smSysOpenXpm;
   XpmT			smSysOpenNewXpm;
   XpmT			smUserClosedXpm;
   XpmT			smUserClosedNewXpm;
   XpmT			smUserOpenXpm;
   XpmT			smUserOpenNewXpm;

//
// Public methods
//
    FolderPrefC();
   ~FolderPrefC();

   StringListC&	InitialFolders()	{ return initFolderList; }
   FolderListC&	OpenFolders()		{ return openFolderList; }
   FolderListC&	FileFolders()		{ return fileFolderList; }

   StringC&	OrigInitialFolders()	{ return orig.initFolderStr; }
   FolderViewTypeT	ViewType()	{ return folderViewType; }
   Boolean	SortFolders()		{ return sortFolders; }

   void		AddOpenFolder(FolderC*);
   void		AddFileFolder(FolderC*);
   void		Edit(Widget);
   void		ExpandValues();
   FolderTypeT	FolderType(char*, Boolean create=False);
   FolderC	*GetFolder(StringC, Boolean create=False);
   void		RemoveFolder(FolderC*);
   void		SetInitialFolders(StringC&);
   void		SetSorted(Boolean);
   void		SetViewType(FolderViewTypeT);
   Boolean	UsingLocal() { return (usingUnix || usingMh || usingMmdf); }
   Boolean	WriteDatabase();
   Boolean	WriteFile();
   Boolean	WriteInitialFolders();	// To file
};

#endif // _FolderPrefC_h
