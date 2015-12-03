/*
 *  $Id: QuickMenu.C,v 1.3 2000/11/22 12:38:37 evgeny Exp $
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
#include "QuickMenu.h"
#include "IshAppC.h"
#include "AppPrefC.h"
#include "FolderPrefC.h"
#include "FileMisc.h"
#include "ImapServerC.h"
#include "ImapMisc.h"

#include <hgl/WArgList.h>
#include <hgl/WXmString.h>
#include <hgl/WidgetListC.h>
#include <hgl/SysErr.h>
#include <hgl/StringListC.h>
#include <hgl/CharC.h>

#include <Xm/PushB.h>
#include <Xm/CascadeB.h>
#include <Xm/RowColumn.h>

#include <dirent.h>
#include <errno.h>
#include <math.h>
#include <unistd.h>

//
// Macros that really should be subroutine but they aren't because they would
//    require too many arguments
//
#define CREATE_ERR_PB { \
 \
   wstr = (char*)errmsg; \
   args.LabelString(wstr); \
   args.UserData(data); \
 \
/* Create a button for the error message. */ \
 \
   Widget	pb; \
   if ( pbList.size() > 0 ) { \
      pb = *pbList[pbList.size()-1]; \
      XtSetValues(pb, ARGS); \
      pbList.removeLast(); \
   } \
   else { \
      pb = XmCreatePushButton(data->menu, "quickPB", ARGS); \
      XtAddCallback(pb, XmNactivateCallback, pbCall, clientData); \
   } \
 \
   XtVaGetValues(pb, XmNwidth, &wd, XmNheight, &ht, NULL); \
   if ( wd > wMax ) wMax = wd; \
   if ( ht > hMax ) hMax = ht; \
 \
   manageList.add(pb); \
   dirSize++; \
}

/*---------------------------------------------------------------
 *  Method to build a popup menu for the directory specified
 */

void
BuildQuickFolderMenu(QuickInfoT *data, XtCallbackProc pbCall,
		     XtCallbackProc cbCall, XtPointer clientData)
{
   halApp->BusyCursor(True);

//
// Unmanage existing buttons
//
   WidgetList	wlist;
   Cardinal	wcount;
   XtVaGetValues(data->menu, XmNnumChildren, &wcount, XmNchildren, &wlist,
   		 NULL);
   if ( wcount > 0 ) XtUnmanageChildren(wlist, wcount);

//
// Split the pushbuttons and cascade buttons into separate lists
//
   WidgetListC	cbList;
   WidgetListC	pbList;
   int i=0; for (i=0; i<wcount; i++) {
      Widget	w = wlist[i];
      if      ( XmIsCascadeButton(w) ) cbList.add(w);
      else if ( XmIsPushButton(w)    ) pbList.add(w);
   }

//
// Read the directory
//
   WArgList	args;
   WXmString	wstr;
   WidgetListC	manageList;
   Dimension	wMax = 0;	// Maximum button width
   Dimension	hMax = 0;	// Maximum button height
   Dimension	wd, ht;
   u_int	dirSize = 0;
   StringC	errmsg;

//
// See if this is a directory on the IMAP server
//
   if ( IsImapName(data->dir) ) {

//
// This must be the folder directory
//
      StringC		serverName = ImapServerPart(data->dir);
      ImapServerC	*serv      = FindImapServer(serverName);
      if ( !serv || !serv->connected ) {

	 errmsg = "Cannot read directory ";
	 errmsg += data->dir;
	 errmsg += ".\nIMAP server not connected.";

	 CREATE_ERR_PB

      } // End if server is not connected

      else {

	 StringC	pat = ImapPathPart(data->dir);
	 if ( pat.size() > 0 && !pat.EndsWith('/') ) pat += '/';
         pat += '*';

	 CharC	dir = pat;
	 dir.CutEnd(1);   

	 StringListC	fileList;
	 StringListC	output;
	 if ( !serv->ListMailboxes(pat, fileList, output) ) {

	    errmsg = "Cannot read directory ";
	    errmsg += data->dir;
	    errmsg += ":\n";
	    u_int	count = output.size();
	    for (i=0; i<count; i++) {
	       errmsg += *output[i];
	       errmsg += '\n';
	    }

	    CREATE_ERR_PB
	 }
	 
	 else if ( fileList.size() == 0 ) {

	    errmsg = "No entries in ";
	    errmsg += data->dir;

	    CREATE_ERR_PB

	 } // End if listing empty

	 else {

//
// Create a push button for each entry
//
	    args.Reset();
	    args.PositionIndex(XmLAST_POSITION);

	    fileList.sort();
	    u_int	count = fileList.size();
	    for (i=0; i<count; i++) {

//
// Remove directory name from beginning of files
//
	       StringC	*name = fileList[i];
	       if ( name->StartsWith(dir) ) name->CutBeg(dir.Length());

	       if ( name->StartsWith('.') ) continue;

//
// Create push button
//
	       wstr = (char*)*name;
	       args.LabelString(wstr);
	       args.UserData(data);

	       Widget	pb;
	       if ( pbList.size() > 0 ) {
		  pb = *pbList[pbList.size()-1];
		  XtSetValues(pb, ARGS);
		  pbList.removeLast();
	       }
	       else {
		  pb = XmCreatePushButton(data->menu, "quickPB", ARGS);
		  XtAddCallback(pb, XmNactivateCallback, pbCall, clientData);
	       }

	       XtVaGetValues(pb, XmNwidth, &wd, XmNheight, &ht, NULL);
	       if ( wd > wMax ) wMax = wd;
	       if ( ht > hMax ) hMax = ht;

	       manageList.add(pb);
	       dirSize++;

	    } // End for each entry

	 } // End if listing returned

      } // End if server connected

   } // End if directory is on IMAP server

   else {

      DIR	*dirp = opendir(data->dir);
      if ( !dirp ) {

	 int	err = errno;
	 StringC	errmsg("Cannot read directory");
	 if ( data->dir == ishApp->appPrefs->FolderDir() ) {
	    errmsg += " ";
	    errmsg += data->dir;
	 }
	 errmsg += ".  ";
	 errmsg += SystemErrorMessage(err);

	 CREATE_ERR_PB

      } // End if directory could not be opened

      else {

//
// Build a list of names, then sort them
//
	 StringC		entry;
	 StringListC	entryList;
	 entryList.AllowDuplicates(FALSE);
	 entryList.SetSorted(FALSE);

	 struct dirent	*dp;
	 while ( (dp=readdir(dirp)) != NULL) {

	    entry = dp->d_name;
	    if ( !entry.StartsWith('.') )
	       entryList.add(entry);

	 } // End for each directory entry

	 closedir(dirp);
	 entryList.sort();

//
// Change to the directory so we don't have to build full pathnames
//
	 chdir(data->dir);

//
// Loop through the names
//
	 u_int	count = entryList.size();
	 for (i=0; i<count; i++) {

	    args.Reset();
	    args.PositionIndex(XmLAST_POSITION);

	    StringC	*entryP = entryList[i];

//
// See if this is a file or a directory
//
	    if ( IsDir(*entryP) ) {

	       wstr = (char*)*entryP;
	       args.LabelString(wstr);

//
// If we're showing dirs and this dir is writable, add a push button for it
//    (for saving)
//
	       if ( ishApp->folderPrefs->usingMh &&
		    access(*entryP, W_OK) == 0 ) {

		  args.UserData(data);

		  Widget	pb;
		  if ( pbList.size() > 0 ) {
		     pb = *pbList[pbList.size()-1];
		     XtSetValues(pb, ARGS);
		     pbList.removeLast();
		  }
		  else {
		     pb = XmCreatePushButton(data->menu, "quickPB", ARGS);
		     XtAddCallback(pb, XmNactivateCallback, pbCall, clientData);
		  }

		  XtVaGetValues(pb, XmNwidth, &wd, XmNheight, &ht, NULL);
		  if ( wd > wMax ) wMax = wd;
		  if ( ht > hMax ) hMax = ht;

		  manageList.add(pb);
		  dirSize++;

	       } // End if entry is a writable directory

//
// Add a cascade button for navigating to the folder
//
	       Widget	cb;
	       QuickInfoT	*cbData;
	       if ( cbList.size() > 0 ) {

		  cb = *cbList[cbList.size()-1];
		  cbList.removeLast();

		  XtVaGetValues(cb, XmNuserData, &cbData, NULL);

		  args.UserData(cbData);
		  XtSetValues(cb, ARGS);
	       }
	       else {

//
// Add a data structure and a pulldown menu
//
		  cbData = new QuickInfoT;
		  cbData->menu = XmCreatePulldownMenu(data->menu, "quickPD", 0,0);

		  args.UserData(cbData);
		  args.SubMenuId(cbData->menu);

		  cb = XmCreateCascadeButton(data->menu, "quickCB", ARGS);
		  XtAddCallback(cb, XmNcascadingCallback, cbCall, clientData);
	       }

	       cbData->dir = data->dir;
	       cbData->dir += "/";
	       cbData->dir += *entryP;
	       cbData->menuTime = 0;

	       XtVaGetValues(cb, XmNwidth, &wd, XmNheight, &ht, NULL);
	       if ( wd > wMax ) wMax = wd;
	       if ( ht > hMax ) hMax = ht;

	       manageList.add(cb);
	       dirSize++;

	    } // End if entry is a directory

//
// If we're showing files and this file is writable, add a button for it
//
	    else if ( (ishApp->folderPrefs->usingUnix ||
		       ishApp->folderPrefs->usingMmdf) &&
		      access(*entryP, W_OK) == 0 ) {

	       wstr = (char*)*entryP;
	       args.LabelString(wstr);
	       args.UserData(data);

	       Widget	pb;
	       if ( pbList.size() > 0 ) {
		  pb = *pbList[pbList.size()-1];
		  XtSetValues(pb, ARGS);
		  pbList.removeLast();
	       }
	       else {
		  pb = XmCreatePushButton(data->menu, "quickPB", ARGS);
		  XtAddCallback(pb, XmNactivateCallback, pbCall, clientData);
	       }

	       XtVaGetValues(pb, XmNwidth, &wd, XmNheight, &ht, NULL);
	       if ( wd > wMax ) wMax = wd;
	       if ( ht > hMax ) hMax = ht;

	       manageList.add(pb);
	       dirSize++;

	    } // End if entry is a writable file

	 } // End for each directory entry

//
// Change back to the starting directory
//
	 chdir(ishApp->startupDir);

//
// Add a message if there were no entries in the directory
//
	 if ( dirSize == 0 ) {

	    StringC	errmsg("No entries ");
	    if ( data->dir == ishApp->appPrefs->FolderDir() ) {
	       errmsg += "in ";
	       errmsg += data->dir;
	    }

	    CREATE_ERR_PB

	 } // End if directory had no entries

      } // End if directory could be opened

   } // End if directory is local

//
// Try to figure out an attractive number of columns for the menu.  In
//    general, we want the menu to be square.  If each button is width wMax
//    and height hMax, then we need i columns and j rows as determined by:
//
//    i*wMax = j*hMax
//
// We also know that "i*j <= N" and "N/i >= j" where N is the total number
//    of buttons.  By substituting N/i for j we get:
//
//    i*wMax <= N*hMax/i
//
// This reduces to:
//
//    i <= sqrt(N*hMax/wMax)
//
   double	val  = sqrt((double)(dirSize * hMax) / (double)wMax);
   short	cols = (short)val;
   if ( cols <= 1 ) cols = 1;

   if ( debuglev > 0 ) cout <<"Using " <<cols <<" columns in quick menu" <<endl;
   XtVaSetValues(data->menu, XmNnumColumns, cols, NULL);

   if ( manageList.size() > 0 )
      XtManageChildren(manageList.start(), manageList.size());

   data->menuTime = time(0);

   halApp->BusyCursor(False);

} // End BuildQuickFolderMenu

/*---------------------------------------------------------------
 *  Method to update the directory name for a cascade button
 */

void
UpdateQuickDir(Widget cb, CharC dir)
{
//
// Get the data record
//
   QuickInfoT	*data;
   XtVaGetValues(cb, XmNuserData, &data, NULL);

//
// Update the directory and menu time
//
   data->dir      = dir;
   data->menuTime = 0;

//
// Loop through the pulldown menu and update any cascade buttons found
//
   WidgetList	wlist;
   Cardinal	wcount;
   XtVaGetValues(data->menu, XmNnumChildren, &wcount, XmNchildren, &wlist, 0);

   StringC	newDir;
   int i=0; for (i=0; i<wcount; i++) {

      Widget	w = wlist[i];
      if ( XmIsCascadeButton(w) ) {

//
// Get the data record
//
	 XtVaGetValues(w, XmNuserData, &data, NULL);

//
// Create the new directory name for this button
//
	 newDir = dir;
	 newDir += "/";
	 newDir += BaseName(data->dir);

	 UpdateQuickDir(w, newDir);

      } // End if widget is a cascade button

   } // End for each widget in pulldown menu

} // End UpdateQuickDir
