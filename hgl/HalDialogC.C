/*
 * $Id: HalDialogC.C,v 1.3 2000/06/01 17:19:03 evgeny Exp $
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

#include "HalDialogC.h"
#include "HalAppC.h"
#include "WArgList.h"
#include "ForceDialog.h"
#include "rsrc.h"

#include <X11/Shell.h>
#include <Xm/AtomMgr.h>
#include <Xm/Protocols.h>
#include <Xm/DialogS.h>
#include <Xm/MessageB.h>
#include <Xm/MainW.h>
#include <Xm/Form.h>
#include <Xm/RowColumn.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <X11/IntrinsicP.h>
#include <X11/CoreP.h>

/*----------------------------------------------------------------------
 * Constructor
 */

HalDialogC::HalDialogC(const char *name, Widget parent, ArgList argv,
		       Cardinal argc, int type) : HalShellC()
{
   WArgList	args;

   dialogType = type;
   realParent = parent;

   if( dialogType == 0 ) {
     args.Reset();
     args.DeleteResponse(XmDO_NOTHING);
     shell = XmCreateDialogShell(parent, (char *)name, ARGS);
   } else if( dialogType == 1 ) {
     msgBox = XmCreateTemplateDialog(parent, (char *)name, argv, argc);
     shell = XtParent(msgBox);
   } else {
     cerr << "Bad type passed to HalDialogC::HalDialogC" NL;
     exit( 1 );
   }

//
// Add an event handler to detect map and unmap
//
   XtAddEventHandler(shell, StructureNotifyMask, False,
		     (XtEventHandler)HalShellC::HandleMapChange,
		     (XtPointer)this);

   args.Reset();
   args.MarginWidth(0);
   args.MarginHeight(0);
   args.ShadowThickness(0);
   if( dialogType == 0 )
     mainBoard = XmCreateForm(shell, "mainBoard", ARGS);
   else
     mainBoard = XmCreateForm(msgBox, "mainBoard", ARGS);

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
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_FORM);
   mainWindow = XmCreateMainWindow(mainBoard, "mainWindow", ARGS);

//
// Create mainWindow children
//
// mainWindow
//   Frame		appFrame
//	Form		topForm
//   RowColumn		msgWin
//
   if( dialogType == 0 ) {
     appFrame = XmCreateFrame(mainWindow, "appFrame", 0,0);
     topForm  = XmCreateForm(appFrame, "topForm", 0,0);
   } else {
     args.Reset();
     args.MarginWidth(0);
     args.MarginHeight(0);
     args.ShadowThickness(0);
     topForm = XmCreateForm( mainWindow, "topForm", ARGS );
   }

   args.Reset();
   args.Orientation(XmVERTICAL);
   args.Packing(XmPACK_TIGHT);
   args.NumColumns(1);
   msgWin = XmCreateRowColumn(mainWindow, "msgWin", ARGS);

   if( dialogType == 0 ) {
     XtVaSetValues(mainWindow, XmNworkWindow,	appFrame,
		   XmNmessageWindow,	msgWin,
		   NULL);
   } else {
     XtVaSetValues(mainWindow, XmNworkWindow,	topForm,
		   XmNmessageWindow,	msgWin,
		   NULL);
   }

//
// Create topForm
// topForm
//    Form		appForm
//       (app specific interface components)
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_FORM);
   appForm = XmCreateForm(topForm, "appForm", ARGS);
   XtManageChild(appForm);

   XtManageChild(topForm);

   if( dialogType == 0 )
     XtManageChild(appFrame);

   XtManageChild(mainWindow);	// mainBoard children

   if ( halApp->quickHelpEnabled ) ShowQuickHelp();

} // End HalDialogC constructor

/*----------------------------------------------------------------------
 * Destructor
 */

HalDialogC::~HalDialogC()
{
   if ( halApp->xRunning )
      XtDestroyWidget(shell);
}

/*----------------------------------------------------------------------
 *  Display and hide window
 */

void
HalDialogC::Show(Widget parent)
{
//
// Set the dialog's parent so it pops up in the desired place
//
   shell->core.parent = parent;
   XtVaSetValues(shell, XmNtransientFor, parent, NULL);

   XtManageChild(mainBoard);
   if ( dialogType == 1 ) XtManageChild(msgBox);
   XMapRaised(halApp->display, XtWindow(shell));
   ForceDialog(shell);
   shown = True;
   halApp->RegisterShell(this);
}

void
HalDialogC::Show()
{
   Show(realParent);
}

void
HalDialogC::Hide()
{
   if ( !shown ) return;

   CallHideCallbacks();
   halApp->UnregisterShell(this);

   XtUnmanageChild(mainBoard);
   if( dialogType == 1 ) XtUnmanageChild(msgBox);
   shown = False;
}
