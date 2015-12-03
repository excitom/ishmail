/*
 *  $Id: MainFile.C,v 1.4 2001/07/28 18:26:03 evgeny Exp $
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
#include "MainWinC.h"
#include "MainWinP.h"
#include "IshAppC.h"
#include "AppPrefC.h"
#include "ConfPrefC.h"
#include "FolderC.h"
#include "FolderPrefC.h"
#include "ReadWinC.h"
#include "SendWinC.h"
#include "SigPrefC.h"
#include "PipeWinC.h"
#include "PrintWinC.h"
#include "FileChooserWinC.h"

#include <hgl/rsrc.h>
#include <hgl/WArgList.h>
#include <hgl/WXmString.h>
#include <hgl/CharC.h>

#include <Xm/MessageB.h>

#include <signal.h>

/*---------------------------------------------------------------
 *  Callback to handle file->Sleep
 */

void
MainWinP::DoFileSleep(Widget, MainWinP *This, XtPointer)
{
//
// See if we need to save any folders
//
   switch ( This->SaveQuery() ) {

      case (QUERY_YES):
	 DoFolderSaveAll(NULL, This, NULL);
	 break;

      case (QUERY_CANCEL):
	 return;

      case (QUERY_NO):
      case (QUERY_NONE):
	 break;
   }

//
// Close all reading and composition windows
//
   u_int	count = ishApp->readWinList.size();
   int	i;
   for (i=0; i<count; i++) {
      ReadWinC	*readWin = (ReadWinC *)*ishApp->readWinList[i];
      readWin->Hide();
   }

   count = ishApp->sendWinList.size();
   for (i=0; i<count; i++) {
      SendWinC	*sendWin = (SendWinC *)*ishApp->sendWinList[i];
      sendWin->Close();
   }

//
// Iconify the main window
//
   XIconifyWindow(ishApp->display, (Window)*This->pub, ishApp->screenNum);

   ishApp->sleeping = True;

//
// Deactivate current folder
//
   This->pub->curFolder->Deactivate();

} // End DoFileSleep

/*---------------------------------------------------------------
 *  Callback to handle file->Quit
 */

static Boolean	quitting = False;

void
MainWinP::DoFileQuit(Widget, MainWinP *This, XtPointer)
{
   quitting = True;
   if ( !ishApp->confPrefs->confirmExit ) {
      ishApp->DoExit(NULL, False/*not interrupted*/, NULL);
      return;
   }

//
// Build verification dialog if necessary
//
   static Widget	quitWin = NULL;
   if ( !quitWin ) {

      WArgList	args;
      args.DialogStyle(XmDIALOG_FULL_APPLICATION_MODAL);
      if ( ishApp->questionPM ) args.SymbolPixmap(ishApp->questionPM);
      quitWin = XmCreateQuestionDialog(*This->pub, "quitWin", ARGS);

      XtUnmanageChild(XmMessageBoxGetChild(quitWin, XmDIALOG_HELP_BUTTON));
      XtAddCallback(quitWin, XmNokCallback, (XtCallbackProc)HalAppC::DoExit,
      		    (XtPointer)False/*interrupted*/);

   } // End if exit window not yet created

//
// See if there are any changed folders
//
   Boolean	changed = ishApp->systemFolder->Changed();
   unsigned	count   = ishApp->folderPrefs->OpenFolders().size();
   for (int i=0; !changed && i<count; i++) {

      FolderC	*folder = ishApp->folderPrefs->OpenFolders()[i];
      if ( folder->isInBox ) continue;

      changed = (folder->writable && folder->Changed());
   }

//
// Prepare message
//
   StringC msg;
   if (changed )
      msg = get_string(quitWin, "unsavedMessageString", "Really quit $APP");
   else
      msg = get_string(quitWin, "messageString", "Really quit $APP");

   msg.Replace("$APP", ishApp->name);

   WXmString	wstr(msg);
   XtVaSetValues(quitWin, XmNmessageString, (XmString)wstr, NULL);

   XtManageChild(quitWin);
   XMapRaised(ishApp->display, XtWindow(XtParent(quitWin)));

} // End DoFileQuit

/*---------------------------------------------------------------
 *  Callback to handle file->Exit
 */

void
MainWinP::DoFileExit(Widget, MainWinP *This, XtPointer)
{
   quitting = False;
   if ( !ishApp->confPrefs->confirmExit ) {
      ishApp->DoExit(NULL, False/*not interrupted*/, NULL);
      return;
   }

//
// Build verification dialog if necessary
//
   static Widget	exitWin = NULL;
   if ( !exitWin ) {

      WArgList	args;
      args.DialogStyle(XmDIALOG_FULL_APPLICATION_MODAL);
      if ( ishApp->questionPM )
	 args.SymbolPixmap(ishApp->questionPM);
      exitWin = XmCreateQuestionDialog(*This->pub, "exitWin", ARGS);

      XtUnmanageChild(XmMessageBoxGetChild(exitWin, XmDIALOG_HELP_BUTTON));
      XtAddCallback(exitWin, XmNokCallback, (XtCallbackProc)HalAppC::DoExit,
      		    (XtPointer)False/*not interrupted*/);

   } // End if exit window not yet created

//
// Add app name to message
//
   StringC msg = get_string(exitWin, "messageString", "Really exit $APP?");

   msg.Replace("$APP", ishApp->name);

   WXmString	wstr(msg);
   XtVaSetValues(exitWin, XmNmessageString, (XmString)wstr, NULL);

   XtManageChild(exitWin);
   XMapRaised(ishApp->display, XtWindow(XtParent(exitWin)));

} // End DoFileExit

/*---------------------------------------------------------------
 *  Callback to handle acceptance of exit or quit
 */

void
MainWinP::Exit(void *arg, MainWinP *This)
{
   HalExitDataT	*data = (HalExitDataT*)arg;
   Boolean	saveChanges = False;

   if ( data->interrupted ) {
      saveChanges = (ishApp->exitSignal == SIGHUP);
   }
   else if ( quitting ) {
      saveChanges = False;
   }

   else switch ( This->SaveQuery() ) {

      case (QUERY_YES):
	 saveChanges = True;
	 break;

      case (QUERY_NO):
	 saveChanges = False;
	 break;

      case (QUERY_CANCEL):
	 data->cancelExit = True;
	 return;

      case (QUERY_NONE):
	 break;
   }

   if ( ishApp->xRunning ) {

//
// Save window positions if necessary
//
      if ( ishApp->appPrefs->rememberWindows )
	 ishApp->appPrefs->WriteWindowPositions();

//
// Try to close composition windows here.  If there are unfinished compositions,
//    the user may change their mind.
//
      if ( !data->interrupted ) {

	 unsigned	count = ishApp->sendWinList.size();
	 for (int i=0; i<count; i++) {
	    SendWinC	*sendWin = (SendWinC *)*ishApp->sendWinList[i];
	    if ( !sendWin->Close() ) {
	       data->cancelExit = True;
	       return;
	    }
	 }

      } // End if this is a clean exit

   } // End if X is still running

   ishApp->exiting = True;

//
// Save recent folder names
//
   ishApp->appPrefs->WriteRecentFolders();

//
// Save initial folders names
//
   if ( ishApp->folderPrefs->rememberFolders )
      ishApp->folderPrefs->WriteInitialFolders();

//
// Save folders that have changed
//
   if ( saveChanges )
      DoFolderSaveAll(NULL, This, NULL);

//
// Close and delete dialogs
//
   if ( halApp->xRunning ) {

//
// Close all reading windows
//
      u_int	count = ishApp->readWinList.size();
      for (int i=0; i<count; i++) {
	 ReadWinC	*readWin = (ReadWinC *)*ishApp->readWinList[i];
	 readWin->Hide();
      }

//
// Close remaining windows
//
      if ( This->pipeWin       ) This->pipeWin->Hide();
      if ( This->printWin      ) This->printWin->Hide();
      if ( This->openSelectWin ) This->openSelectWin->Hide();
      if ( This->newFolderWin  ) This->newFolderWin->Hide();

//
// Close this window
//
      This->pub->Hide();

#if 0
      while ( XtAppPending(halApp->context) ) {
	 XtAppProcessEvent(halApp->context, XtIMXEvent);
	 XSync(halApp->display, False);
      }
#endif

   } // End if X is running

} // End Exit

