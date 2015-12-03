/*
 * $Id: HalTopLevelC.C,v 1.4 2000/06/01 17:19:03 evgeny Exp $
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

#include "HalTopLevelC.h"
#include "WArgList.h"
#include "StringC.h"
#include "WXmString.h"
#include "RegexC.h"
#include "rsrc.h"
#include "HelpC.h"
#include "HelpResWinC.h"
#include "HalAppC.h"

#include <X11/Shell.h>
#include <Xm/Protocols.h>
#include <Xm/MainW.h>
#include <Xm/Frame.h>
#include <Xm/Form.h>
#include <Xm/RowColumn.h>
#include <Xm/Label.h>
#include <Xm/CascadeB.h>
#include <Xm/PushB.h>
#include <Xm/AtomMgr.h>
#include <Xm/MessageB.h>
#include <Xm/Separator.h>

extern int	debug1, debug2;

/*----------------------------------------------------------------------
 * Constructor
 */

HalTopLevelC::HalTopLevelC(const char *name, Widget parent) : HalShellC()
{
   exitWin = NULL;

//
// If there were any command line arguments, we might want to apply them to
//    this shell rather than the app shell (which is invisible).
//
// The ones we will check for are "iconic", "geometry" and "title".
//
   WArgList	args;
   args.DeleteResponse(XmDO_NOTHING);

   static Boolean	argsChecked = False;
   if ( !argsChecked ) {

      argsChecked = True;

      if ( debug1 ) cout <<"Checking command line arguments" <<endl;

      StringC	str = get_string(*halApp, "geometry", "None");
      if ( str != "None" ) {
	 String	geo;
	 XtVaGetValues(*halApp, XmNgeometry, &geo, NULL);
	 args.Geometry(geo);
	 if ( debug1 )
	    cout <<"Applying geometry resource " <<str <<" to " <<name <<endl;
      }

      str = get_string(*halApp, "iconic", "None");
      if ( str != "None" ) {
	 Boolean	iconic;
	 XtVaGetValues(*halApp, XmNiconic, &iconic, NULL);
	 args.Iconic(iconic);
	 if ( debug1 )
	    cout <<"Applying iconic resource " <<str <<" to " <<name <<endl;
      }

      str = get_string(*halApp, "title", "None");
      if ( str != "None" ) {
	 String	title;
	 XtVaGetValues(*halApp, XmNtitle, &title, NULL);
	 args.Title(title);
	 args.IconName(title);
	 if ( debug1 )
	    cout <<"Applying title resource " <<str <<" to " <<name <<endl;
      }

   } // End if there were command line arguments

   shell = topLevel =
      XtCreatePopupShell(name, topLevelShellWidgetClass, parent, ARGS);

//
// Add an event handler to detect map and unmap
//
   XtAddEventHandler(shell, StructureNotifyMask, False,
		     (XtEventHandler)HalShellC::HandleMapChange,
		     (XtPointer)this);

//
// Catch kill from the window manager
//
   XmAddWMProtocolCallback(shell, halApp->delWinAtom, (XtCallbackProc)DoHide,
			   (caddr_t)this);

//
// Create main window
//
   args.Reset();
   args.ShowSeparator(False);
   mainWindow = XmCreateMainWindow(shell, "mainWindow", ARGS);

//
// Create mainWindow children
//
// mainWindow
//   MenuBar		menuBar		(on demand)
//	CascadeButton	fileCB		(on demand)
//	PulldownMenu	filePD		(on demand)
//	CascadeButton	helpCB		(on demand)
//	PulldownMenu	helpPD		(on demand)
//   Frame		appFrame
//      Form		topForm
//   RowColumn		msgWin
//
   menuBar = fileCB = filePD = helpCB = helpPD = NULL;

   appFrame = XmCreateFrame(mainWindow, "appFrame", 0,0);
   topForm  = XmCreateForm(appFrame, "topForm", 0,0);

   args.Reset();
   args.Orientation(XmVERTICAL);
   args.Packing(XmPACK_TIGHT);
   args.NumColumns(1);
   msgWin = XmCreateRowColumn(mainWindow, "msgWin", ARGS);

   XtVaSetValues(mainWindow, XmNworkWindow,    appFrame,
			     XmNmessageWindow, msgWin,
                             NULL);

//
// Create topForm children
// topForm
//   Form		appForm
//      (app specific interface components)
//
   args.Reset();
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   args.TopAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_FORM);
   appForm   = XmCreateForm(topForm, "appForm", ARGS);
   XtManageChild(appForm);

   XtManageChild(topForm);

#if 0
//
// Create msgWin children
//
// msgWin
//    Label		quickHelp
//    Label		infoMsg
//
   args.Reset();
   args.Alignment(XmALIGNMENT_BEGINNING);
   args.RecomputeSize(False);
   quickHelp = XmCreateLabel(msgWin, "quickHelp", ARGS);
   infoMsg   = XmCreateLabel(msgWin, "infoMsg",	  ARGS);
#endif

//
// Manage mainWindow children
//
   XtManageChild(appFrame);	// mainWindow children
   XtManageChild(mainWindow);	// shell children

   if ( halApp->quickHelpEnabled ) ShowQuickHelp();

} // End HalTopLevelC constructor

/*----------------------------------------------------------------------
 * Destructor
 */

HalTopLevelC::~HalTopLevelC()
{
   if ( halApp->xRunning && shell != halApp->appShell )
      XtDestroyWidget(shell);
}

/*----------------------------------------------------------------------
 * Method to add menu bar
 */

void
HalTopLevelC::AddMenuBar()
{
   if ( menuBar ) return;

   WArgList	args;

   menuBar = XmCreateMenuBar(mainWindow, "menuBar", 0,0);

//
// Create menuBar children
//
// menuBar
//    PulldownMenu	filePD
//    CascadeButton	fileCB

   filePD = XmCreatePulldownMenu(menuBar, "filePD", 0,0);

   args.Reset();
   args.SubMenuId(filePD);
   fileCB = XmCreateCascadeButton(menuBar, "fileCB", ARGS);
   XtManageChild(fileCB);

   XtVaSetValues(mainWindow, XmNmenuBar,        menuBar,
                             XmNworkWindow,     appFrame,
			     XmNmessageWindow,  msgWin,
                             NULL);

   XtManageChild(menuBar);

} // End HalTopLevel AddMenuBar

/*----------------------------------------------------------------------
 * Callback to confirm application exit
 */

static void
DoHelpMod(Widget, HalTopLevelC *This, XtPointer)
{
   halApp->BusyCursor(True);
   halApp->HelpWin()->HelpResWin()->Show();
   halApp->BusyCursor(False);
}

/*----------------------------------------------------------------------
 * Method to add help menu
 */

void
HalTopLevelC::AddHelpMenu()
{
   if ( helpCB ) return;

//
// Create cascade button
//
//  menuBar
//     CascadeButton	helpCB
//
   helpCB = XmCreateCascadeButton(menuBar, "helpCB", 0,0);
   XtManageChild(helpCB);

   XtVaSetValues(menuBar, XmNmenuHelpWidget, helpCB, NULL);

//
// Create helpPulldown children
//
//  menuBar
//     PulldownMenu	helpPD
//        PushButton	helpContextPB
//        PushButton	helpWindowPB
//        PushButton	helpAppPB
//        PushButton	helpIndexPB
//        Separator	helpSep1
//        PushButton	helpHelpPB
//        PushButton	helpKeysPB
//        PushButton	helpVerPB
//
   if ( halApp->HasHelp() || halApp->HelpWin()->Editable() ) {

      helpPD = XmCreatePulldownMenu(menuBar, "helpPD", 0,0);
      helpContextPB   = XmCreatePushButton(helpPD, "helpContextPB", 0,0);
      helpWindowPB    = XmCreatePushButton(helpPD, "helpWindowPB",  0,0);
      helpAppPB       = XmCreatePushButton(helpPD, "helpAppPB",     0,0);
      helpIndexPB     = XmCreatePushButton(helpPD, "helpIndexPB",   0,0);
      Widget helpSep1 = XmCreateSeparator (helpPD, "helpSep1",      0,0);
      helpHelpPB      = XmCreatePushButton(helpPD, "helpHelpPB",    0,0);
      helpKeysPB      = XmCreatePushButton(helpPD, "helpKeysPB",    0,0);
      helpVerPB       = XmCreatePushButton(helpPD, "helpVerPB",     0,0);

//
// Manage widgets
//
      Widget	list[8];
      list[0] = helpContextPB;
      list[1] = helpWindowPB;
      list[2] = helpAppPB;
      list[3] = helpIndexPB;
      list[4] = helpSep1;
      list[5] = helpHelpPB;
      list[6] = helpKeysPB;
      list[7] = helpVerPB;
      XtManageChildren(list, 8);	// helpPD children

      XtAddCallback(helpContextPB, XmNactivateCallback,
		    (XtCallbackProc)DoContextHelp, (XtPointer)this);
      XtAddCallback(helpWindowPB,  XmNactivateCallback,
		    (XtCallbackProc)HalAppC::DoHelp, (char *) "helpcard");
      XtAddCallback(helpAppPB,     XmNactivateCallback,
		    (XtCallbackProc)HalAppC::DoHelp, (char *) "helpcard");
      XtAddCallback(helpIndexPB,   XmNactivateCallback,
		    (XtCallbackProc)HalAppC::DoIndexHelp, NULL);
      XtAddCallback(helpHelpPB,    XmNactivateCallback,
		    (XtCallbackProc)HalAppC::DoHelp, (char *) "helpcard");
      XtAddCallback(helpKeysPB,    XmNactivateCallback,
		    (XtCallbackProc)HalAppC::DoHelp, (char *) "helpcard");
      XtAddCallback(helpVerPB,     XmNactivateCallback,
		    (XtCallbackProc)HalAppC::DoHelp, (char *) "helpcard");

      XtVaSetValues(helpCB, XmNsubMenuId, helpPD, NULL);

      if ( halApp->HelpWin()->Editable() ) {
	 Widget helpModPB = XmCreatePushButton(helpPD, "helpModPB", 0,0);
	 XtManageChild(helpModPB);
	 XtAddCallback(helpModPB, XmNactivateCallback,
		       (XtCallbackProc)DoHelpMod, this);
      }
   }

   else
      XtSetSensitive(helpCB, False);

} // End HalTopLevelC AddHelpMenu

/*----------------------------------------------------------------------
 * Method to add exit button to file menu
 */

void
HalTopLevelC::AddExitButton()
{
//
// Create exit button
//
//  filePD
//     PushButton	fileExitPB
//
   fileExitPB = XmCreatePushButton(filePD, "fileExitPB", 0,0);
   XtManageChild(fileExitPB);

   XtAddCallback(fileExitPB, XmNactivateCallback, (XtCallbackProc)DoExit,
		 (XtPointer)this);

} // End HalTopLevelC AddExitButton

/*----------------------------------------------------------------------
 * Callback to confirm application exit
 */

void
HalTopLevelC::DoExit(Widget, HalTopLevelC *tl, XtPointer)
{
   Boolean	confirmExit = get_boolean(*halApp, "confirmExit", True);
   if ( !confirmExit ) {
      HalAppC::DoExit(NULL, False/*not interrupted*/, NULL);
      return;
   }

//
// Build verification dialog if necessary
//
   if ( !tl->exitWin ) {

      WArgList	args;
      args.DialogStyle(XmDIALOG_FULL_APPLICATION_MODAL);
      if ( halApp->questionPM )
	 args.SymbolPixmap(halApp->questionPM);
      tl->exitWin = XmCreateQuestionDialog(*halApp, "exitWin", ARGS);

//
// Add app name to message
//
      StringC msg =
         get_string(tl->exitWin, "messageString", "Really exit $APP?");

      RegexC	appPat("$APP");
      while ( appPat.search(msg) >= 0 ) msg(appPat[0]) = halApp->name;

      WXmString	wstr(msg);
      XtVaSetValues(tl->exitWin, XmNmessageString, (XmString)wstr, NULL);

      XtAddCallback(tl->exitWin, XmNokCallback, (XtCallbackProc)HalAppC::DoExit,
		    (XtPointer)False/*not interrupted*/);


   } // End if exit window not yet created

   XtManageChild(tl->exitWin);

   // unmanage the help button
   XtUnmanageChild(XmMessageBoxGetChild(tl->exitWin, XmDIALOG_HELP_BUTTON));

   XMapRaised(halApp->display, XtWindow(XtParent(tl->exitWin)));

} // End HalTopLevelC DoExit
