/*
 * $Id: PipeWinC.C,v 1.3 2000/05/07 12:26:12 fnevgeny Exp $
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

#include "PipeWinC.h"
#include "IshAppC.h"
#include "MainWinC.h"
#include "MsgItemC.h"
#include "MsgC.h"
#include "Fork.h"
#include "Mailcap.h"

#include <hgl/WArgList.h>
#include <hgl/rsrc.h>
#include <hgl/VBoxC.h>
#include <hgl/TextMisc.h>
#include <hgl/VItemC.h>
#include <hgl/SysErr.h>

#include <Xm/Frame.h>
#include <Xm/RowColumn.h>
#include <Xm/ToggleB.h>
#include <Xm/PushB.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/TextF.h>

#include <unistd.h>     // For sleep
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>

/*---------------------------------------------------------------
 *  Dialog constructor
 */

PipeWinC::PipeWinC(Widget parent) : HalDialogC("pipeWin", parent)
{
   WArgList	args;
   Widget	list[4];

   errorShell = this;

//
// Create appForm hierarchy
//
// appForm
//    RowColumn	appRC
//       Frame	hdrFrame
//       Frame	ordFrame
//       Form	cmdForm
//
   args.Reset();
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   args.TopAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_FORM);
   args.Packing(XmPACK_TIGHT);
   args.Orientation(XmVERTICAL);
   Widget	appRC = XmCreateRowColumn(appForm, "appRC", ARGS);

   Widget	hdrFrame = XmCreateFrame(appRC, "hdrFrame", 0,0);
   		ordFrame = XmCreateFrame(appRC, "ordFrame", 0,0);
   Widget	cmdForm  = XmCreateForm (appRC, "cmdForm",  0,0);

//
// Create hdrFrame hierarchy
//
//   hdrFrame
//	RadioBox	hdrRadio
//	   ToggleButton	hdrAllTB
//	   ToggleButton	hdrDispTB
//	   ToggleButton	hdrNoneTB
//
   args.Reset();
   args.Packing(XmPACK_TIGHT);
   Widget hdrRadio = XmCreateRadioBox(hdrFrame, "hdrRadio", ARGS);

   hdrAllTB  = XmCreateToggleButton(hdrRadio, "hdrAllTB",  0,0);
   hdrDispTB = XmCreateToggleButton(hdrRadio, "hdrDispTB", 0,0);
   hdrNoneTB = XmCreateToggleButton(hdrRadio, "hdrNoneTB", 0,0);

   list[0] = hdrAllTB;
   list[1] = hdrDispTB;
   list[2] = hdrNoneTB;
   XtManageChildren(list, 3);
   XtManageChild(hdrRadio);

//
// Create ordFrame hierarchy
//
//   ordFrame
//	RadioBox	ordRadio
//	   ToggleButton	ordParallelTB
//	   ToggleButton	ordSerialTB
//
   args.Reset();
   args.Packing(XmPACK_TIGHT);
   Widget ordRadio = XmCreateRadioBox(ordFrame, "ordRadio", ARGS);

   ordParallelTB = XmCreateToggleButton(ordRadio, "ordParallelTB", 0,0);
   ordSerialTB   = XmCreateToggleButton(ordRadio, "ordSerialTB",   0,0);

   list[0] = ordParallelTB;
   list[1] = ordSerialTB;
   XtManageChildren(list, 2);
   XtManageChild(ordRadio);

//
// Create cmdForm hierarchy
//
//   cmdForm
//	Label		cmdLabel
//	TextField	cmdTF
//
   args.Reset();
   args.LeftAttachment(XmATTACH_FORM);
   args.TopAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_NONE);
   Widget	cmdLabel = XmCreateLabel(cmdForm, "cmdLabel", ARGS);

   args.Reset();
   args.LeftAttachment(XmATTACH_WIDGET, cmdLabel);
   args.TopAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   cmdTF = CreateTextField(cmdForm, "cmdTF", ARGS);

   list[0] = cmdTF;
   list[1] = cmdLabel;
   XtManageChildren(list, 2);	// cmdForm children

   list[0] = hdrFrame;
   list[1] = cmdForm;
   XtManageChildren(list, 2);	// appRC children

   XtManageChild(appRC);	// appForm children

//
// Create buttonRC hierarchy
//
//   buttonRC
//	PushButton	pipePB
//	PushButton	cancelPB
//	PushButton	helpPB
//
   AddButtonBox();
   Widget pipePB   = XmCreatePushButton(buttonRC, "pipePB",   0,0);
   Widget cancelPB = XmCreatePushButton(buttonRC, "cancelPB", 0,0);
   Widget helpPB   = XmCreatePushButton(buttonRC, "helpPB",   0,0);

   list[0] = pipePB;
   list[1] = cancelPB;
   list[2] = helpPB;
   XtManageChildren(list, 3);	// buttonRC children

   XtAddCallback(pipePB, XmNactivateCallback, (XtCallbackProc)DoPipe,
          	 (XtPointer)this);
   XtAddCallback(cancelPB, XmNactivateCallback, (XtCallbackProc)DoHide,
 		 (XtPointer)this);
   XtAddCallback(helpPB, XmNactivateCallback, (XtCallbackProc)HalAppC::DoHelp,
 		 (char *) "helpcard");

   XtVaSetValues(appForm, XmNdefaultButton, pipePB, NULL);

   HandleHelp();

//
// Initialize settings
//
   XmToggleButtonSetState(ordParallelTB, True, True);
   XmToggleButtonSetState(hdrDispTB,     True, True);

} // End PipeWinC constructor

/*---------------------------------------------------------------
 *  Destructor
 */

PipeWinC::~PipeWinC()
{
}

/*---------------------------------------------------------------
 *  Callback to handle pipe
 */

void
PipeWinC::DoPipe(Widget, PipeWinC *This, XtPointer)
{
   char		*cmd = XmTextFieldGetString(This->cmdTF);
   StringC	cmdStr = cmd;
   XtFree(cmd);

//
// Remove accidental pipe symbols at beginning
//
   cmdStr.Trim();
   while ( cmdStr.size() > 0 && cmdStr[0] == '|' ) {
      cmdStr(0,1) = "";
      cmdStr.Trim();
   }

   if ( !cmdStr.size() ) {
      set_invalid(This->cmdTF, True, True);
      This->PopupMessage("Please enter a filter command.");
      return;
   }

   StringC	statusMsg;

   This->BusyCursor(True);

   Boolean	error     = False;
   VItemListC&	selList   = ishApp->mainWin->MsgVBox().SelItems();
   unsigned	selCount  = selList.size();
   Boolean	usePopup  = ( ishApp->mainWin->popupMsg &&
			     !ishApp->mainWin->popupOnSelected);
   unsigned	pipeCount = usePopup ? 1 : selCount;

   if ( pipeCount > 1 ) {

      Boolean	parallel = XmToggleButtonGetState(This->ordParallelTB);

      if ( parallel ) {
	 error = !This->PipeParallel(selList, cmdStr);
      }

      else {
	 error = !This->PipeSerial(selList, cmdStr);
      }

      if ( !error )
	 statusMsg = "Messages passed to filter.";

   } // End if more than one message

   else {

      MsgItemC	*item = NULL;
      MsgC	*msg  = NULL;
      if ( ishApp->mainWin->popupMsg ) {
	 msg  = ishApp->mainWin->popupMsg;
	 item = msg->icon;
      }
      else {
	 item = (MsgItemC*)selList[0];
	 msg = item->msg;
      }

      error = !This->Pipe(msg, cmdStr);

      if ( !error ) {
	 statusMsg = "Message passed to filter.";
	 item->SetFiltered();
      }

   } // End if a single message

//
// Report status
//
   if ( !error ) {
      ishApp->Broadcast(statusMsg);
      This->Hide();
   }

   This->BusyCursor(False);

} // End DoPipe

/*---------------------------------------------------------------
 *  Method to handle filtering of multiple messages in parallel
 */

Boolean
PipeWinC::PipeParallel(VItemListC& list, StringC& cmdStr)
{
   Boolean	error = False;
   unsigned	count = list.size();
   for (int i=0; i<count; i++) {

      MsgItemC	*item = (MsgItemC*)list[i];
      MsgC	*msg = item->msg;

      if ( !Pipe(msg, cmdStr) ) error = True;
      else			item->SetFiltered();
   }

   return !error;

} // End PipeParallel

/*---------------------------------------------------------------
 *  Method to handle filtering of multiple messages as a single file
 */

Boolean
PipeWinC::PipeSerial(VItemListC& list, StringC& cmdStr)
{
   StringC	statusMsg;

//
// Create a temporary file for storing messages
//
   char	*tmpFile = tempnam(NULL, "pipe.");
   if ( !tmpFile ) {

      int	err = errno;
      statusMsg = "Could not create temporary file for filtering.\n";
      statusMsg += SystemErrorMessage(err);
      statusMsg += "\nWill attempt to filter in parallel.";
      errorShell->PopupMessage(statusMsg, XmDIALOG_WARNING);

      return PipeParallel(list, cmdStr);
   }

   FILE *tfp = fopen(tmpFile, "w+");
   if ( !tfp ) {

      int	err = errno;
      statusMsg = "Could not create temporary file for filtering.\n";
      statusMsg += SystemErrorMessage(err);
      statusMsg += "\nWill attempt to filter in parallel.";
      errorShell->PopupMessage(statusMsg, XmDIALOG_WARNING);

      free(tmpFile);

      return PipeParallel(list, cmdStr);
   }

   Boolean	allHeaders = XmToggleButtonGetState(hdrAllTB);
   Boolean	noHeaders  = XmToggleButtonGetState(hdrNoneTB);
   StringC	headStr;

//
// Store selected messages in temporary file
//
   Boolean	error = False;
   unsigned	count = list.size();
   int	i;
   for (i=0; !error && i<count; i++) {

      MsgItemC	*item = (MsgItemC*)list[i];
      MsgC	*msg = item->msg;
      int	index = msg->Number();

//
// Write headers and body
//
      error = !msg->WriteFile(tfp, !noHeaders, allHeaders,
			      True/*status header*/,
			      True/*add a blank line*/,
			      False/*don't protect Froms*/);

   } // End for each selected message

//
// Check for errors
//
   if ( error ) {

      int	err = errno;
      statusMsg = "Could not write to temporary file for filtering.\n";
      statusMsg += SystemErrorMessage(err);
      statusMsg += "\nWill attempt to filter in parallel.";
      errorShell->PopupMessage(statusMsg, XmDIALOG_WARNING);

      fclose(tfp);
      unlink(tmpFile);
      free(tmpFile);

      return PipeParallel(list, cmdStr);
   }

   fclose(tfp);

   error = !PipeFile(tmpFile, cmdStr);

//
// Update status
//
   if ( !error ) {
      count = list.size();
      for (i=0; i<count; i++) {
	 MsgItemC	*item = (MsgItemC*)list[i];
	 item->SetFiltered();
      }
   }

   return !error;

} // End PipeSerial

/*---------------------------------------------------------------
 *  Method to handle filtering of a single message
 */

Boolean
PipeWinC::Pipe(MsgC *msg, StringC& cmdStr)
{
   int	msgnum = msg->Number();

//
// Create a temporary file for the message
//
   char	*tmpFile = tempnam(NULL, "pipe.");
   if ( !tmpFile ) {

      int	err = errno;
      StringC	errmsg = "Couldn't create temp file while filtering message: ";
      if ( msgnum >= 0 ) errmsg += msgnum;
      errmsg += ".\n" + SystemErrorMessage(err);
      errorShell->PopupMessage(errmsg);

      return False;
   }

//
// Write the message to the temp file
//
   if ( !msg->WriteFile(tmpFile, !XmToggleButtonGetState(hdrNoneTB),
			XmToggleButtonGetState(hdrAllTB),
			True/*status header*/,
			False/*no blank line*/, False/*don't protect Froms*/) )
   {
      unlink(tmpFile);
      free(tmpFile);
      return False;
   }

   return PipeFile(tmpFile, cmdStr, msgnum);

} // End Pipe

/*---------------------------------------------------------------
 *  Method to filter a temporary file
 */

Boolean
PipeWinC::PipeFile(char *fileName, StringC& cmdStr, int msgnum)
{
//
// Build the command string
//
   StringC cmd = BuildCommand(cmdStr, fileName, NULL, NULL, NULL, NULL, NULL);

//
// Create some callback data
//
   PipeProcT	*proc = new PipeProcT;
   proc->win     = this;
   proc->msgnum  = msgnum;
   proc->cmdStr  = cmd;
   proc->tmpFile = fileName;
   CallbackC	doneCb((CallbackFn*)ForkDone, proc);

   proc->pid = ForkIt(cmd, &doneCb);

   if ( proc->pid < 0 ) {

      StringC	errmsg = "Could not filter message";
      if ( msgnum >= 0 ) {
	 errmsg += " ";
	 errmsg += msgnum;
      }
      else
	 errmsg += "s.";
      errmsg += "\n" + ForkStatusMsg(cmd, (int)proc->pid);
      errorShell->PopupMessage(errmsg);

      unlink(fileName);
      free(fileName);
      delete proc;

      return False;
   }

   return True;

} // End PipeFile

/*---------------------------------------------------------------
 *  Callback for completion of ForkIt
 */

void
PipeWinC::ForkDone(int status, PipeProcT *proc)
{
//
// Report status of operation
//
   if ( status != 0 ) {

      StringC	errmsg;
      if ( proc->msgnum >= 0 ) {
	 errmsg = "Message ";
	 errmsg += proc->msgnum;
      } else {
	 errmsg = "Messages";
      }
      errmsg += " not filtered.\n";
      errmsg += ForkStatusMsg(proc->cmdStr, status, proc->pid);
      proc->win->errorShell->PopupMessage(errmsg);

   } // End if there is an error

   if ( proc->tmpFile ) {
      unlink(proc->tmpFile);
      free(proc->tmpFile);
   }
   delete proc;

   return;

} // End ForkDone

/*---------------------------------------------------------------
 *  Method to remove ordFrame
 */

void
PipeWinC::HideOrder()
{
   XtUnmanageChild(ordFrame);
}

/*---------------------------------------------------------------
 *  Method to display ordFrame
 */

void
PipeWinC::ShowOrder()
{
   XtManageChild(ordFrame);
}
