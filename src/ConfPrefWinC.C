/*
 *  $Id: ConfPrefWinC.C,v 1.2 2000/05/07 12:26:11 fnevgeny Exp $
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
#include "ConfPrefWinC.h"
#include "ConfPrefC.h"
#include "IshAppC.h"

#include <hgl/WArgList.h>

#include <Xm/ToggleB.h>
#include <Xm/RowColumn.h>

extern int	debug1, debug2;

/*---------------------------------------------------------------
 *  Main window constructor
 */

						      // historical name
ConfPrefWinC::ConfPrefWinC(Widget par) : OptWinC(par, "confirmWin")
{
   WArgList	args;
   Widget 	wlist[12];

//
// Create appForm hierarchy
//
// appForm
//    RowColumn		confirmRC
//       ToggleButton	   exitTB
//       ToggleButton	   saveOnExitTB
//       ToggleButton	   folderTypeTB
//       ToggleButton	   folderDelTB
//       ToggleButton	   sendNoSubjectTB
//       ToggleButton	   sendNoBodyTB
//       ToggleButton	   clearSendTB
//       ToggleButton	   closeSendTB
//       ToggleButton	   sendPlainTB
//       ToggleButton	   eightBitPlainTB
//       ToggleButton	   deleteGraphicTB
//    
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_FORM);
   Widget confirmRC = XmCreateRowColumn(appForm, "confirmRC", ARGS);

   exitTB          = XmCreateToggleButton(confirmRC, "exitTB",          0,0);
   saveOnExitTB    = XmCreateToggleButton(confirmRC, "saveOnExitTB",    0,0);
   folderTypeTB    = XmCreateToggleButton(confirmRC, "folderTypeTB",    0,0);
   folderDelTB     = XmCreateToggleButton(confirmRC, "folderDelTB",     0,0);
   sendNoSubjectTB = XmCreateToggleButton(confirmRC, "sendNoSubjectTB", 0,0);
   sendNoBodyTB    = XmCreateToggleButton(confirmRC, "sendNoBodyTB",    0,0);
   clearSendTB     = XmCreateToggleButton(confirmRC, "clearSendTB",     0,0);
   closeSendTB     = XmCreateToggleButton(confirmRC, "closeSendTB",     0,0);
   sendPlainTB     = XmCreateToggleButton(confirmRC, "sendPlainTB",     0,0);
   eightBitPlainTB = XmCreateToggleButton(confirmRC, "eightBitPlainTB", 0,0);
   deleteGraphicTB = XmCreateToggleButton(confirmRC, "deleteGraphicTB", 0,0);

   wlist[ 0] = exitTB;
   wlist[ 1] = saveOnExitTB;
   wlist[ 2] = folderTypeTB;
   wlist[ 3] = folderDelTB;
   wlist[ 4] = sendNoSubjectTB;
   wlist[ 5] = sendNoBodyTB;
   wlist[ 6] = clearSendTB;
   wlist[ 7] = closeSendTB;
   wlist[ 8] = sendPlainTB;
   wlist[ 9] = eightBitPlainTB;
   wlist[10] = deleteGraphicTB;
   XtManageChildren(wlist, 11);	// confirmRC children

   XtManageChild(confirmRC);	// appForm children

   HandleHelp();

} // End constructor

/*---------------------------------------------------------------
 *  Method to handle display
 */

void
ConfPrefWinC::Show(Widget parent)
{
   ConfPrefC	*prefs = ishApp->confPrefs;

//
// Initialize settings
//
   XmToggleButtonSetState(exitTB,	   prefs->confirmExit,		True);
   XmToggleButtonSetState(saveOnExitTB,	   prefs->confirmSaveOnExit,	True);
   XmToggleButtonSetState(folderTypeTB,	   prefs->confirmFolderType,	True);
   XmToggleButtonSetState(folderDelTB,	   prefs->confirmDeleteFolder,	True);
   XmToggleButtonSetState(sendNoSubjectTB, prefs->confirmSendNoSubject,	True);
   XmToggleButtonSetState(sendNoBodyTB,	   prefs->confirmSendNoBody,	True);
   XmToggleButtonSetState(clearSendTB,	   prefs->confirmClearSend,	True);
   XmToggleButtonSetState(closeSendTB,	   prefs->confirmCloseSend,	True);
   XmToggleButtonSetState(sendPlainTB,	   prefs->confirmSendPlain,	True);
   XmToggleButtonSetState(eightBitPlainTB, prefs->confirm8BitPlain,	True);
   XmToggleButtonSetState(deleteGraphicTB, prefs->confirmDeleteGraphic,	True);

   OptWinC::Show(parent);

} // End Show

void
ConfPrefWinC::Show()
{
   Show(XtParent(*this));
}

/*---------------------------------------------------------------
 *  Method to apply settings
 */

Boolean
ConfPrefWinC::Apply()
{
   ConfPrefC	*prefs = ishApp->confPrefs;

   BusyCursor(True);

//
// Update preferences
//
   prefs->confirmExit		= XmToggleButtonGetState(exitTB);
   prefs->confirmSaveOnExit	= XmToggleButtonGetState(saveOnExitTB);
   prefs->confirmFolderType	= XmToggleButtonGetState(folderTypeTB);
   prefs->confirmDeleteFolder	= XmToggleButtonGetState(folderDelTB);
   prefs->confirmSendNoSubject	= XmToggleButtonGetState(sendNoSubjectTB);
   prefs->confirmSendNoBody	= XmToggleButtonGetState(sendNoBodyTB);
   prefs->confirmClearSend	= XmToggleButtonGetState(clearSendTB);
   prefs->confirmCloseSend	= XmToggleButtonGetState(closeSendTB);
   prefs->confirmSendPlain	= XmToggleButtonGetState(sendPlainTB);
   prefs->confirm8BitPlain	= XmToggleButtonGetState(eightBitPlainTB);
   prefs->confirmDeleteGraphic	= XmToggleButtonGetState(deleteGraphicTB);

   prefs->WriteDatabase();

//
// Write to file if necessary
//
   if ( applyAll ) prefs->WriteFile();

   BusyCursor(False);

   return True;

} // End Apply
