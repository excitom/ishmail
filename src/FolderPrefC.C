/*
 *  $Id: FolderPrefC.C,v 1.5 2000/08/07 11:05:16 evgeny Exp $
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
#include "FolderPrefC.h"
#include "ConfPrefC.h"
#include "AppPrefC.h"
#include "Misc.h"
#include "ShellExp.h"
#include "FileMisc.h"
#include "UnixFolderC.h"
#include "MmdfFolderC.h"
#include "ImapFolderC.h"
#include "MhFolderC.h"
#include "FolderPrefWinC.h"
#include "MainWinC.h"
#include "Query.h"
#include "ImapMisc.h"

#include <hgl/rsrc.h>
#include <hgl/CharC.h>
#include <hgl/VBoxC.h>
#include <hgl/WArgList.h>
#include <hgl/WXmString.h>
#include <hgl/RegexC.h>

#include <Xm/MessageB.h>
#include <Xm/Frame.h>
#include <Xm/RowColumn.h>
#include <Xm/ToggleB.h>
#include <Xm/Protocols.h>
#include <Xm/AtomMgr.h>

#include <sys/stat.h>
#include <errno.h>

#include "system-closed-new-mail.xpm"
#include "system-closed-no-mail.xpm"
#include "system-open-new-mail.xpm"
#include "system-open-no-mail.xpm"
#include "user-closed-new-mail.xpm"
#include "user-closed-no-mail.xpm"
#include "user-open-new-mail.xpm"
#include "user-open-no-mail.xpm"
#include "sm-system-closed-new-mail.xpm"
#include "sm-system-closed-no-mail.xpm"
#include "sm-system-open-new-mail.xpm"
#include "sm-system-open-no-mail.xpm"
#include "sm-user-closed-new-mail.xpm"
#include "sm-user-closed-no-mail.xpm"
#include "sm-user-open-new-mail.xpm"
#include "sm-user-open-no-mail.xpm"

/*---------------------------------------------------------------
 *  Constructor
 */

FolderPrefC::FolderPrefC() : PrefC()
{
   prefWin = NULL;

//
// Read resources
//
   orig.initFolderStr = get_string(*halApp, "initialFolders");
   ExtractList(orig.initFolderStr, initFolderList);

   sortFolders     = get_boolean(*halApp, "sortFolders",	True);
   rememberFolders = get_boolean(*halApp, "rememberFolders",	False);
   showStatus	   = get_boolean(*halApp, "showFolderStatus",	False);

   Boolean oldVal  = get_boolean(*halApp, "showMHFolders",	True);
   usingMh         = get_boolean(*halApp, "usingMh",		oldVal);

   oldVal          = get_boolean(*halApp, "showUnixFolders",	True);
   usingUnix       = get_boolean(*halApp, "usingUnix",		oldVal);
   usingMmdf       = get_boolean(*halApp, "usingMmdf",		oldVal);

   StringC	tmpStr = get_string(*halApp, "defaultFolderType");
   if      ( ishApp->appPrefs->usingImap && tmpStr.Equals("imap", IGNORE_CASE) )
      defFolderType = IMAP_FOLDER;
   else if ( usingMh && tmpStr.Equals("mh", IGNORE_CASE) )
      defFolderType = MH_FOLDER;
   else if ( usingMmdf && tmpStr.Equals("mmdf", IGNORE_CASE) )
      defFolderType = MMDF_FOLDER;
   else if ( usingUnix && tmpStr.Equals("unix", IGNORE_CASE) )
      defFolderType = UNIX_FOLDER;
   else {
      if ( ishApp->appPrefs->usingImap )
	 defFolderType = IMAP_FOLDER;
      else if ( usingUnix )
	 defFolderType = UNIX_FOLDER;
      else if ( usingMh )
	 defFolderType = MH_FOLDER;
      else
	 defFolderType = MMDF_FOLDER;
   }

   tmpStr = get_string(*halApp, "folderViewType", "Large");
   if ( tmpStr.StartsWith("small", IGNORE_CASE) )
      folderViewType = FOLDER_VIEW_SMALL;
   else if ( tmpStr.StartsWith("name", IGNORE_CASE) )
      folderViewType = FOLDER_VIEW_NAME;
   else
      folderViewType = FOLDER_VIEW_LARGE;

   tmpStr = get_string(*halApp, "folderFileMask", "0600");
   (void)sscanf(tmpStr, "%4o", &folderFileMask);

//
// Initialize pixmaps
//
   sysClosedXpm	    = system_closed_no_mail_xpm;
   sysClosedNewXpm  = system_closed_new_mail_xpm;
   sysOpenXpm	    = system_open_no_mail_xpm;
   sysOpenNewXpm    = system_open_new_mail_xpm;
   userClosedXpm    = user_closed_no_mail_xpm;
   userClosedNewXpm = user_closed_new_mail_xpm;
   userOpenXpm	    = user_open_no_mail_xpm;
   userOpenNewXpm   = user_open_new_mail_xpm;

   if ( folderViewType == FOLDER_VIEW_NAME ) {
      smSysClosedXpm     = NULL;
      smSysClosedNewXpm  = NULL;
      smSysOpenXpm	 = NULL;
      smSysOpenNewXpm    = NULL;
      smUserClosedXpm    = NULL;
      smUserClosedNewXpm = NULL;
      smUserOpenXpm      = NULL;
      smUserOpenNewXpm   = NULL;
   }
   else {
      smSysClosedXpm     = sm_system_closed_no_mail_xpm;
      smSysClosedNewXpm  = sm_system_closed_new_mail_xpm;
      smSysOpenXpm	 = sm_system_open_no_mail_xpm;
      smSysOpenNewXpm    = sm_system_open_new_mail_xpm;
      smUserClosedXpm    = sm_user_closed_no_mail_xpm;
      smUserClosedNewXpm = sm_user_closed_new_mail_xpm;
      smUserOpenXpm      = sm_user_open_no_mail_xpm;
      smUserOpenNewXpm   = sm_user_open_new_mail_xpm;
   }

} // End constructor

/*---------------------------------------------------------------
 *  Method to expand values.  Value can be expanded only after this class
 *     is created
 */

void
FolderPrefC::ExpandValues()
{
   ExpandList(initFolderList);
}

/*---------------------------------------------------------------
 *  destructor
 */

FolderPrefC::~FolderPrefC()
{
   delete prefWin;
}

/*---------------------------------------------------------------
 *  Method to write resources to resource database
 */

Boolean
FolderPrefC::WriteDatabase()
{
   Store("initialFolders",	orig.initFolderStr);
   Store("sortFolders",		sortFolders);
   Store("rememberFolders",	rememberFolders);
   Store("showFolderStatus",	showStatus);
   Store("usingMh",		usingMh);
   Store("usingUnix",		usingUnix);
   Store("usingMmdf",		usingMmdf);
   Store("confirmFolderType",	ishApp->confPrefs->confirmFolderType);

   switch (defFolderType) {
      case (UNIX_FOLDER): Store("defaultFolderType", "Unix"); break;
      case (MH_FOLDER):   Store("defaultFolderType", "MH");   break;
      case (MMDF_FOLDER): Store("defaultFolderType", "MMDF"); break;
      case (IMAP_FOLDER): Store("defaultFolderType", "IMAP"); break;
      case (UNKNOWN_FOLDER): break;
   }

   StringC	tmpStr;
   switch (folderViewType) {
      case (FOLDER_VIEW_LARGE): tmpStr = "Large_Icon"; break;
      case (FOLDER_VIEW_SMALL): tmpStr = "Small_Icon"; break;
      case (FOLDER_VIEW_NAME):  tmpStr = "Name";       break;
   }
   Store("folderViewType",	tmpStr);

   return True;

} // End WriteDatabase

/*---------------------------------------------------------------
 *  Method to write resources to a file
 */

Boolean
FolderPrefC::WriteFile()
{
   StringListC	lineList;
   ReadResFile(lineList);

   Update(lineList, "initialFolders",	 orig.initFolderStr);
   Update(lineList, "sortFolders",	 sortFolders);
   Update(lineList, "rememberFolders",	 rememberFolders);
   Update(lineList, "showFolderStatus",	 showStatus);
   Update(lineList, "usingMh",		 usingMh);
   Update(lineList, "usingUnix",	 usingUnix);
   Update(lineList, "usingMmdf",	 usingMmdf);
   Update(lineList, "confirmFolderType", ishApp->confPrefs->confirmFolderType);

   switch (defFolderType) {
      case (UNIX_FOLDER): Update(lineList, "defaultFolderType", "Unix"); break;
      case (MH_FOLDER):   Update(lineList, "defaultFolderType", "MH");   break;
      case (MMDF_FOLDER): Update(lineList, "defaultFolderType", "MMDF"); break;
      case (IMAP_FOLDER): Update(lineList, "defaultFolderType", "IMAP"); break;
      case (UNKNOWN_FOLDER): break;
   }

   StringC	tmpStr;
   switch (folderViewType) {
      case (FOLDER_VIEW_LARGE): tmpStr = "Large_Icon"; break;
      case (FOLDER_VIEW_SMALL): tmpStr = "Small_Icon"; break;
      case (FOLDER_VIEW_NAME):  tmpStr = "Name";       break;
   }
   Update(lineList, "folderViewType",	tmpStr);

//
// Write the information back to the file
//
   return WriteResFile(lineList);

} // End WriteFile

/*---------------------------------------------------------------
 *  Method to write initial folder list to a file
 */

Boolean
FolderPrefC::WriteInitialFolders()
{
   StringListC	lineList;
   ReadResFile(lineList);

   Update(lineList, "initialFolders", orig.initFolderStr);

//
// Write the information back to the file
//
   return WriteResFile(lineList);

} // End WriteInitialFolders

/*---------------------------------------------------------------
 *  Function to query for type of new folder
 */

FolderTypeT
FolderPrefC::QueryFolderType()
{
//
// If we're not confirming, return the default type
//
   if ( !ishApp->confPrefs->confirmFolderType )
      return defFolderType;

//
// If the user is using only one folder type, return the default
//
   int	typeCount = (usingMh			 ? 1 : 0)
   		  + (usingUnix			 ? 1 : 0)
		  + (usingMmdf			 ? 1 : 0)
		  + (ishApp->appPrefs->usingImap ? 1 : 0);
   if ( typeCount <= 1 )
      return defFolderType;

//
// The user is using more than one folder type so we need to ask
//
   static QueryAnswerT	answer;
   static Widget	dialog = NULL;
   static Widget	unixTB;
   static Widget	mhTB;
   static Widget	mmdfTB;
   static Widget	imapTB;

//
// Create the dialog if necessary
//
   if ( !dialog ) {

      halApp->BusyCursor(True);

      WArgList	args;
      args.DialogStyle(XmDIALOG_FULL_APPLICATION_MODAL);
      args.AutoUnmanage(False);
      if ( halApp->questionPM ) args.SymbolPixmap(halApp->questionPM);
      dialog = XmCreateQuestionDialog(*halApp, "folderTypeWin", ARGS);

//
// Add buttons for folder type
//
      Widget frame = XmCreateFrame   (dialog, "folderTypeFrame", 0,0);
      Widget radio = XmCreateRadioBox(frame,  "folderTypeRadio", 0,0);

      imapTB = XmCreateToggleButton(radio, "imapFolderTB", 0,0);
      unixTB = XmCreateToggleButton(radio, "unixFolderTB", 0,0);
      mhTB   = XmCreateToggleButton(radio, "mhFolderTB",   0,0);
      mmdfTB = XmCreateToggleButton(radio, "mmdfFolderTB", 0,0);

      XtManageChild(unixTB);
      XtManageChild(mhTB);
      XtManageChild(mmdfTB);
      XtManageChild(imapTB);
      XtManageChild(radio);
      XtManageChild(frame);

      switch (defFolderType) {
	 case (UNIX_FOLDER):
	    XmToggleButtonSetState(unixTB, True, True);
	    break;
	 case (MMDF_FOLDER):
	    XmToggleButtonSetState(mmdfTB, True, True);
	    break;
	 case (IMAP_FOLDER):
	    XmToggleButtonSetState(imapTB, True, True);
	    break;
	 case (MH_FOLDER):
	    XmToggleButtonSetState(mhTB, True, True);
	    break;
	 case (UNKNOWN_FOLDER):
	    break;
      }

      XtAddCallback(dialog, XmNokCallback, (XtCallbackProc)AnswerQuery,
		    (XtPointer)&answer);
      XtAddCallback(dialog, XmNcancelCallback,
		    (XtCallbackProc)AnswerQuery, (XtPointer)&answer);
      XtAddCallback(dialog, XmNhelpCallback,
		    (XtCallbackProc)HalAppC::DoHelp, (char *) "helpcard");

//
// Trap window manager close function
//
      XmAddWMProtocolCallback(XtParent(dialog), halApp->delWinAtom,
			      (XtCallbackProc)WmClose,
			      (caddr_t)&answer);

      halApp->BusyCursor(False);

   } // End if create query dialog not created

//
// Manage appropriate choices
//
   if ( usingUnix ) XtManageChild  (unixTB);
   else		    XtUnmanageChild(unixTB);
   if ( usingMh )   XtManageChild  (mhTB);
   else		    XtUnmanageChild(mhTB);
   if ( usingMmdf ) XtManageChild  (mmdfTB);
   else		    XtUnmanageChild(mmdfTB);

//
// Labels are slightly different when an IMAP server is being used
//
   StringC	msgStr;                                         
   WXmString    wstr;
   if ( ishApp->appPrefs->usingImap ) {

      XtManageChild(imapTB);

      if ( usingUnix ) {
	 msgStr = get_string(unixTB, "imapLabelString","Unix on local machine");
	 wstr = (char*)msgStr;                      
	 XtVaSetValues(unixTB, XmNlabelString, (XmString)wstr, NULL);
      }

      if ( usingMh ) {
	 msgStr = get_string(mhTB, "imapLabelString", "MH on local machine");
	 wstr = (char*)msgStr;                      
	 XtVaSetValues(mhTB, XmNlabelString, (XmString)wstr, NULL);
      }

      if ( usingMmdf ) {
	 msgStr = get_string(mmdfTB, "imapLabelString","MMDF on local machine");
	 wstr = (char*)msgStr;                      
	 XtVaSetValues(mmdfTB, XmNlabelString, (XmString)wstr, NULL);
      }

   } // End if using an IMAP server

   else {

      XtUnmanageChild(imapTB);

      if ( usingUnix ) {
	 msgStr = get_string(unixTB, "labelString", "Unix");
	 wstr = (char*)msgStr;                      
	 XtVaSetValues(unixTB, XmNlabelString, (XmString)wstr, NULL);
      }

      if ( usingMh ) {
	 msgStr = get_string(mhTB, "labelString", "MH");
	 wstr = (char*)msgStr;                      
	 XtVaSetValues(mhTB, XmNlabelString, (XmString)wstr, NULL);
      }

      if ( usingMmdf ) {
	 msgStr = get_string(mmdfTB, "labelString", "MMDF");
	 wstr = (char*)msgStr;                      
	 XtVaSetValues(mmdfTB, XmNlabelString, (XmString)wstr, NULL);
      }

   } // End if not using an IMAP server

//
// Show the dialog
//
   XtManageChild(dialog);
   XMapRaised(halApp->display, XtWindow(XtParent(dialog)));

//
// Simulate the main event loop and wait for the answer
//
   answer = QUERY_NONE;
   while ( answer == QUERY_NONE ) {
      XtAppProcessEvent(halApp->context, XtIMXEvent);
      XSync(halApp->display, False);
   }

   XtUnmanageChild(dialog);
   XSync(halApp->display, False);
   XmUpdateDisplay(dialog);

   if ( answer != QUERY_YES )
      return UNKNOWN_FOLDER;

   if      ( usingUnix && XmToggleButtonGetState(unixTB) )
      return UNIX_FOLDER;
   else if ( usingMh   && XmToggleButtonGetState(mhTB)   )
      return MH_FOLDER;
   else if ( usingMmdf && XmToggleButtonGetState(mmdfTB) )
      return MMDF_FOLDER;
   else if ( ishApp->appPrefs->usingImap && XmToggleButtonGetState(imapTB) )
      return IMAP_FOLDER;

   return defFolderType;

} // End QueryFolderType

/*---------------------------------------------------------------
 *  Method to determine the type of folder represented by the specified
 *  pathname.
 */

FolderTypeT
FolderPrefC::FolderType(char *name, Boolean create)
{
//
// IMAP folders start with {server}
//
   if ( IsImapName(name) ) return IMAP_FOLDER;

//
// If it's a directory, it's an MH folder
//
   if ( IsDir(name) ) {
      if ( usingMh ) return MH_FOLDER;
      else	     return UNKNOWN_FOLDER;
   }

//
// If the file doesn't exist see if we can figure out what type is should be.
//
   struct stat  stats;
   if ( stat(name, &stats) != 0 ) {

//
// If we're not allowed to create it, this is an error
//
      if ( !create || errno != ENOENT )
	 return UNKNOWN_FOLDER;

//
// If confirmation is on, ask the user
//
      if ( ishApp->confPrefs->confirmFolderType )
	 return QueryFolderType();

//
// If confirmation is off, use the default type
//
      return defFolderType;

   } // End if file could not be stat'd

//
// If the file is zero-length, return the user's default
//
   if ( stats.st_size == 0 )
      return defFolderType;

//
// Read the first line from the file
//
   FILE	*fp = fopen(name, "r");
   if ( !fp )
      return defFolderType;

   StringC	line;
   if ( !line.GetLine(fp) ) {
      fclose(fp);
      return defFolderType;
   }

   fclose(fp);

//
// An MMDF folder starts with four control-A's
//
   if ( line.Equals("") ) return MMDF_FOLDER;

//
// A Unix folder starts with a "From " line
//
   RegexC       *fromPat = UnixFolderC::fromPat;
   if ( line.StartsWith("From ") && (!fromPat || fromPat->match(line)) )
      return UNIX_FOLDER;

//
// We don't know what this is
//
   return UNKNOWN_FOLDER;

} // End FolderType

/*---------------------------------------------------------------
 *  Method find or open an appropriate folder for the given pathname
 */

FolderC*
FolderPrefC::GetFolder(StringC name, Boolean create)
{
   ishApp->ExpandFolderName(name);

//
// See if the name corresponds to the system folder
//
   if ( ishApp->appPrefs->inBox == name ) return ishApp->systemFolder;

//
// See if the name corresponds to an open folder
//
   FolderC	*folder = NULL;
   u_int	count = openFolderList.size();
   int	i;
   for (i=0; i<count; i++) {
      folder = openFolderList[i];
      if ( folder->name == name ) return folder;
   }

//
// See if the name corresponds to a file folder
//
   count = fileFolderList.size();
   for (i=0; i<count; i++) {
      folder = fileFolderList[i];
      if ( folder->name == name ) return folder;
   }

//
// Create a new folder
//
   FolderTypeT	type = FolderType(name, create);
   switch (type) {
      case (UNIX_FOLDER):	folder = new UnixFolderC(name, create);	break;
      case (MMDF_FOLDER):	folder = new MmdfFolderC(name, create);	break;
      case (IMAP_FOLDER):	folder = new ImapFolderC(name, create);	break;
      case (MH_FOLDER):		folder = new MhFolderC  (name, create);	break;
      case (UNKNOWN_FOLDER):	folder = NULL;
   }

   if ( folder && folder->opened )
      fileFolderList.add(folder);
   else if ( folder ) {
      delete folder;
      folder = NULL;
   }

   return folder;

} // End GetFolder

/*---------------------------------------------------------------
 *  Method to add a folder to the open list
 */

void
FolderPrefC::AddOpenFolder(FolderC *folder)
{
   fileFolderList.remove(folder);
   openFolderList.add(folder);

   if ( rememberFolders && !initFolderList.includes(folder->name) ) {
      initFolderList.add(folder->name);
      orig.initFolderStr += '\t';
      orig.initFolderStr += folder->abbrev;
   }
}

/*---------------------------------------------------------------
 *  Method to remove a folder from all lists
 */

void
FolderPrefC::RemoveFolder(FolderC *folder)
{
   fileFolderList.remove(folder);
   openFolderList.remove(folder);

   if ( rememberFolders ) {

      initFolderList.remove(folder->name);

      int	pos = orig.initFolderStr.PosOf(folder->abbrev);
      int	len;
      Boolean	found = False;
      if ( pos >= 0 ) {
	 len = folder->abbrev.size();
	 if ( pos > 0 ) {
	    pos--;	// Remove space as well
	    len++;
	 }
	 orig.initFolderStr(pos,len) = "";
	 found = True;
      }

      if ( !found ) {
	 pos = orig.initFolderStr.PosOf(folder->name);
	 if ( pos >= 0 ) {
	    len = folder->name.size();
	    if ( pos > 0 ) {
	       pos--;	// Remove space as well
	       len++;
	    }
	    orig.initFolderStr(pos,len) = "";
	    found = True;
	 }
      }

//
// If the name could not be found in the original initial folders list, there
//    must be a wildcard expansion that produced the name.  We must use the
//    expanded names and abbreviate them to create a new original list.
//
      if ( !found ) {

	 orig.initFolderStr.Clear();

	 StringC	name;
	 u_int		count = initFolderList.size();
	 for (int i=0; i<count; i++) {
	    name = *initFolderList[i];
	    ishApp->AbbreviateFolderName(name);
	    if ( i>0 ) orig.initFolderStr += '\t';
	    orig.initFolderStr += name;
	 }

      } // End if folder not in orig list

   } // End if rememberFolders

} // End RemoveFolder

/*------------------------------------------------------------------------
 * Method to edit the preferences
 */

void
FolderPrefC::Edit(Widget parent)
{
   halApp->BusyCursor(True);

   if ( !prefWin ) prefWin = new FolderPrefWinC(parent);
   prefWin->Show();

   halApp->BusyCursor(False);
}

/*------------------------------------------------------------------------
 * Method to set initial folders
 */

void
FolderPrefC::SetInitialFolders(StringC& names)
{
   if ( names == orig.initFolderStr ) return;

   orig.initFolderStr = names;
   initFolderList.removeAll();

   ExtractList(orig.initFolderStr, initFolderList);
   ExpandList(initFolderList);
}

/*------------------------------------------------------------------------
 * Method to set view type
 */

void
FolderPrefC::SetViewType(FolderViewTypeT type)
{
   if ( type == folderViewType ) return;
   folderViewType = type;

   VBoxC&	vbox = ishApp->mainWin->FolderVBox();

//
// Update view type and icon pointers
//
   VTypeT	vboxType;
   switch ( folderViewType ) {

      case (FOLDER_VIEW_LARGE):
	 vboxType = 0;
	 break;

      case (FOLDER_VIEW_SMALL):
	 smSysClosedXpm     = sm_system_closed_no_mail_xpm;
	 smSysClosedNewXpm  = sm_system_closed_new_mail_xpm;
	 smSysOpenXpm	    = sm_system_open_no_mail_xpm;
	 smSysOpenNewXpm    = sm_system_open_new_mail_xpm;
	 smUserClosedXpm    = sm_user_closed_no_mail_xpm;
	 smUserClosedNewXpm = sm_user_closed_new_mail_xpm;
	 smUserOpenXpm      = sm_user_open_no_mail_xpm;
	 smUserOpenNewXpm   = sm_user_open_new_mail_xpm;
	 vboxType = 1;
	 break;

      case (FOLDER_VIEW_NAME):
	 smSysClosedXpm     = NULL;
	 smSysClosedNewXpm  = NULL;
	 smSysOpenXpm	    = NULL;
	 smSysOpenNewXpm    = NULL;
	 smUserClosedXpm    = NULL;
	 smUserClosedNewXpm = NULL;
	 smUserOpenXpm      = NULL;
	 smUserOpenNewXpm   = NULL;
	 vboxType = 1;
	 break;
   }

//
// Update icons for open folders
//
   if ( folderViewType != FOLDER_VIEW_LARGE ) {

      ishApp->systemFolder->UpdateIcon();

      u_int	count = openFolderList.size();
      for (int i=0; i<count; i++) {

	 FolderC	*folder = openFolderList[i];
	 folder->UpdateIcon();

      } // End for each user folder

   } // End if not large icon view

   vbox.ViewType(vboxType);
   vbox.Refresh();

} // End SetViewType

/*------------------------------------------------------------------------
 * Method to set folder icon sorting
 */

void
FolderPrefC::SetSorted(Boolean val)
{
   if ( val == sortFolders ) return;
   sortFolders = val;

   VBoxC&	vbox = ishApp->mainWin->FolderVBox();

   vbox.SetSorted(sortFolders);
   VItemListC	items = vbox.Items();
   vbox.RemoveAllItems();
   vbox.AddItems(items);

   vbox.Refresh();

} // End SetSorted

