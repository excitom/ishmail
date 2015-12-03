/*
 *  $Id: ReadPart.C,v 1.3 2000/05/07 12:26:12 fnevgeny Exp $
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
#include "ReadWinC.h"
#include "ReadWinP.h"
#include "ReadIconC.h"
#include "MsgPartC.h"
#include "LoginWinC.h"
#include "ParamC.h"
#include "Mailcap.h"
#include "LocalTextWinC.h"
#include "MsgC.h"
#include "Query.h"
#include "Misc.h"
#include "Fork.h"
#include "FileMsgC.h"
#include "FileChooserWinC.h"
#include "FileMisc.h"
#include "IshAppC.h"
#include "AppPrefC.h"

#include <hgl/WXmString.h>
#include <hgl/PtrListC.h>
#include <hgl/WArgList.h>
#include <hgl/HalAppC.h>
#include <hgl/rsrc.h>
#include <hgl/SysErr.h>
#include <hgl/PixmapC.h>
#include <hgl/StringListC.h>
#include <hgl/TextMisc.h>

#include <Xm/RowColumn.h>	// For XmMenuPosition
#include <Xm/MessageB.h>
#include <Xm/PushB.h>
#include <Xm/Protocols.h>
#include <Xm/AtomMgr.h>

#include <unistd.h>
#include <errno.h>
#include <signal.h>

extern int	debuglev;

/*-----------------------------------------------------------------------
 *  Handle display of mime popup menu
 */

void
ReadWinP::PostPartMenu(ReadIconC *icon, ReadWinP *This)
{
   MsgPartC	*part = icon->part;
   MailcapC	*mcap = MailcapEntry(part);

   Boolean getting = (This->FetchData(part)   != NULL);
   Boolean showing = (This->DisplayData(icon) != NULL);
   Boolean showOk  = (!showing && !getting && !icon->textWin && !icon->msgWin);
   Boolean hideOk  = (showing || icon->textWin || icon->msgWin);
   Boolean fetchOk = (part->IsExternal() && !getting && !showing &&
		      !part->IsLocal());
   Boolean printOk = (!getting && (part->IsText() || part->IsPostScript() ||
				   part->Is822() ||
				   (mcap && mcap->print.size() > 0)) );

   XtSetSensitive(This->mimePUShowPB,  showOk);
   XtSetSensitive(This->mimePUHidePB,  hideOk);
   XtSetSensitive(This->mimePUPrintPB, printOk);
   XtSetSensitive(This->mimePUFetchPB, fetchOk);

   WXmString	wstr = (char *)icon->showStr;
   XtVaSetValues(This->mimePUShowPB, XmNlabelString, (XmString)wstr, NULL);
   wstr = (char *)icon->hideStr;
   XtVaSetValues(This->mimePUHidePB, XmNlabelString, (XmString)wstr, NULL);

//
// Display parameters
//
   StringC	desc;
   ParamC	*param = part->conParams;
   while ( param ) {
      if ( desc.size() > 0 ) desc += '\n';
      desc += param->full;
      param = param->next;
   }

   param = part->accParams;
   while ( param ) {
      if ( desc.size() > 0 ) desc += '\n';
      desc += param->full;
      param = param->next;
   }

   wstr = (char *)desc;
   XtVaSetValues(This->mimePULabel, XmNlabelString, (XmString)wstr, NULL);

   This->popupIcon = icon;

   XmMenuPosition(This->mimePU, icon->lastEvent);
   XtManageChild(This->mimePU);

} // End PostPartMenu

/*-----------------------------------------------------------------------
 *  Handle double-click on mime part
 */

void
ReadWinP::HandleDoubleClick(ReadIconC *icon, ReadWinP *This)
{
   This->popupIcon = icon;
   if ( icon->Highlighted() )
      DoHidePart(NULL, This, NULL);
   else
      DoShowPart(NULL, This, NULL);
}

/*-----------------------------------------------------------------------
 *  Callback request for display of mime attachment
 */

void
ReadWinP::DoShowPart(Widget, ReadWinP *This, XtPointer)
{
   This->ShowIcon(This->popupIcon);

//
// If this part is in a parallel multipart, show siblings.
//
   MsgPartC	*part = This->popupIcon->part;
   if ( part->parent && part->parent->IsParallel() ) {

      MsgPartC	*child = part->parent->child;
      while ( child ) {

//
// Display the child if it's not the original one and it's not already
//   displayed
//
	 ReadIconC	*icon = This->IconWithPart(child);
	 if ( child != part && !icon->Highlighted() )
	    This->ShowIcon(icon);

	 child = child->next;
      }

   } // End if part in parallel

} // End DoShowPart

/*-----------------------------------------------------------------------
 *  Method to display an attachment
 */

void
ReadWinP::ShowIcon(ReadIconC *icon)
{
   if ( icon->Highlighted() ) {
      if ( icon->textWin ) icon->textWin->Show();
      if ( icon->msgWin  ) icon->msgWin->Show();
      return;
   }

   MsgPartC	*part = icon->part;
   icon->Highlight();

//
// See if this part needs to be retrieved first
//
   if ( part->NeedFetch() ) {

//
// Ask the user if they want to retrieve it
//
      if ( !OkToGet(part, "getString") ) {
	 icon->Unhighlight();
	 return;
      }

//
// Execute the retrieval.  Finish the display when the fetch completes
//
      FetchPart(icon, NULL, (CallbackFn*)FinishShowPart);

   } // End if retrieval is needed

   else
      FinishShowPart(icon, this);

} // End ShowPart

/*-----------------------------------------------------------------------
 *  Finish request for display of MIME attachment.  This one is called
 *     after we know the file is present.
 */

void
ReadWinP::FinishShowPart(ReadIconC *icon, ReadWinP *This)
{
   This->pub->BusyCursor(True);

   MsgPartC	*part           = icon->part;
   Boolean	internalDisplay = False;

//
// Get the mailcap entry
//
   MailcapC	*mcap = MailcapEntry(part);
   if ( !mcap ) {

      if ( part->IsText() || part->Is822() )
	 internalDisplay = True;

      else {
	 This->NoMailcap(icon);
	 This->pub->BusyCursor(False);
	 return;
      }
   }

   else if ( mcap->present.size() == 0 ||
	     mcap->present.Equals("ishmail", IGNORE_CASE) )
      internalDisplay = True;

   This->LoadDisplayPixmaps(icon);
   icon->Animate(displayPixmaps);

//
// See if we are to display the text internally
//
   if ( internalDisplay ) {
      if ( part->Is822() ) This->Show822(icon);
      else		   This->ShowText(icon);
   }

   else
      This->ShowExternal(icon, mcap);

   This->pub->BusyCursor(False);

} // End FinishShowPart

/*---------------------------------------------------------------
 *  Method to display a text mime body part internally
 */

void
ReadWinP::ShowText(ReadIconC *icon)
{
   MsgPartC	*part = icon->part;

//
// Create a text window
//
   icon->textWin = GetTextWindow();

   StringC	label;
   part->GetLabel(label);
   icon->textWin->SetTitle(label);

//
// Display the text
//
   StringC	body;
   if ( !part->GetData(body) ) {
      HideIcon(icon);
      return;
   }

   MsgC		*msg  = part->parentMsg;
   icon->textWin->CheckFroms(msg->HasSafeFroms());

   if      ( part->IsRichText() ) icon->textWin->SetRichString(body);
   else if ( part->IsEnriched() ) icon->textWin->SetEnrichedString(body);
   else				  icon->textWin->SetString(body);

   icon->textWin->CheckFroms(False);

   icon->textWin->Show();

} // End ShowText

/*-----------------------------------------------------------------------
 *  Method to get the next available text window from the text window list.
 */

LocalTextWinC*
ReadWinP::GetTextWindow()
{
   LocalTextWinC	*tw = NULL;
   u_int		count = textWinList.size();
   Boolean		found = False;
   for (int i=0; !found && i<count; i++) {
      tw = (LocalTextWinC*)*textWinList[i];
      if ( !tw->IsShown() ) found = True;
   }

   if ( !found ) {
      tw = new LocalTextWinC(*pub);
      tw->AddHideCallback((CallbackFn*)HideTextWin, this);
      void	*tmp = (void*)tw;
      textWinList.add(tmp);
   }

   tw->SetWrap(pub->Wrapping());
   tw->SetSize(msgBodyText->RowCount(), msgBodyText->ColumnCount());

   return tw;

} // End GetTextWindow

/*---------------------------------------------------------------
 *  Method to display an RFC822 mime body part internally
 */

void
ReadWinP::Show822(ReadIconC *icon)
{
   MsgPartC	*part = icon->part;

//
// Create a message window
//
   icon->msgWin = GetMsgWindow();

//
// Display the message
//
   FileMsgC		*msg = part->ChildMsg();
   icon->msgWin->SetMessage(msg);
   icon->msgWin->Show();

} // End Show822

/*-----------------------------------------------------------------------
 *  Method to get the next available message window from the message
 *     window list.
 */

ReadWinC*
ReadWinP::GetMsgWindow()
{
   ReadWinC	*rw = NULL;
   u_int	count = msgWinList.size();
   Boolean	found = False;
   for (int i=0; !found && i<count; i++) {
      rw = (ReadWinC*)*msgWinList[i];
      if ( !rw->IsShown() ) found = True;
   }

   if ( !found ) {
      rw = new ReadWinC("readWin", *pub, pub);
      rw->AddHideCallback((CallbackFn*)HideMsgWin, this);
      void	*tmp = (void*)rw;
      msgWinList.add(tmp);
   }

   rw->SetWrap(pub->Wrapping());
   rw->SetViewType(pub->ViewType());
   rw->SetSize(msgHeadText->RowCount(),
	       msgBodyText->RowCount(), msgBodyText->ColumnCount());

   return rw;

} // End GetMsgWindow

/*---------------------------------------------------------------
 *  Method to display a mime body part externally
 */

void
ReadWinP::ShowExternal(ReadIconC *icon, MailcapC *mcap)
{
   MsgPartC	*part = icon->part;

//
// Make sure there is a data file
//
   if ( !part->CreateDataFile() ) {
      HideIcon(icon);
      return;
   }

//
// Double check that the file exists and is readable
//
   if ( access(part->dataFile, R_OK) != 0 ) {

      StringC	errmsg = "Could not display file: \"";
      errmsg += part->dataFile;
      errmsg += "\".\n";
      errmsg += SystemErrorMessage(errno);
      pub->PopupMessage(errmsg);

      HideIcon(icon);
      return;
   }

//
// Build a data record to store info about this display
//
   DisplayDataT	*data = new DisplayDataT;
   data->win  = this;
   data->icon = icon;

//
// Build the command for display.
//
   data->cmdStr = BuildCommand(mcap->present, part, part->dataFile);
   data->pid    = -1;

   CallbackC	doneCb((CallbackFn*)DisplayDone, data);
   data->pid = ForkIt(data->cmdStr, &doneCb);

//
// If the pid is < 0, an error occurred
//
   if ( data->pid < 0 ) {
      pub->PopupMessage(ForkStatusMsg(data->cmdStr, (int)data->pid));
      icon->Unhighlight();
      delete data;
   }

//
// Add this process to the list of running processes
//
   else {
      void	*tmp = (void*)data;
      displayDataList.add(tmp);
   }

} // End ShowExternal

/*---------------------------------------------------------------
 *  Callback called when external display process stops
 */

void
ReadWinP::DisplayDone(int, DisplayDataT *data)
{
   ReadWinP	*This = data->win;

//
// Remove this process from the list of running processes
//
   void	*tmp = (void*)data;
   This->displayDataList.remove(tmp);

//
// Make sure the icon is still around
//
   ReadIconC	*icon = data->icon;
   PtrListC&	list  = This->msgBodyText->GraphicList();
   u_int	count = list.size();
   Boolean	found = False;
   for (int i=0; !found && i<count; i++) {
      ReadIconC	*ip = (ReadIconC*)*list[i];
      found = (ip == icon);
   }

   if ( found ) {

      if ( debuglev > 0 ) {
	 MsgPartC	*part = icon->part;
	 cout <<"DisplayDone for part: " <<part->partNum <<endl;
      }

//
// Turn off the display animation
//
      icon->AnimationOff();
      icon->Unhighlight();

#if 0
//
// Report status if reading window is longer visible and the process failed
//
      if ( This->pub->IsShown() && This->pub->msg == part->parentMsg &&
	   status != 0 ) {

	 StringC	statmsg("Part \"");
	 statmsg += name;
	 statmsg += "\" could not be displayed.\n";
	 statmsg += ForkStatusMsg(data->cmdStr, status, data->pid);
	 This->pub->PopupMessage(statmsg);

      } // End if display failed
#endif

   } // End if icon still exists

   delete data;
   return;

} // End DisplayDone

/*---------------------------------------------------------------
 *  Callback to handle press of DONE button in local text window
 */

void
ReadWinP::HideTextWin(LocalTextWinC *win, ReadWinP *This)
{
//
// Loop through the list of icons and see which one corresponds to this
//    window
//
   PtrListC&	list  = This->msgBodyText->GraphicList();
   u_int	count = list.size();
   for (int i=0; i<count; i++) {
      ReadIconC	*icon = (ReadIconC*)*list[i];
      if ( icon->textWin == win ) {
	 icon->textWin = NULL;	// So it doesn't get hidden again
	 This->HideIcon(icon);
	 return;
      }
   }

} // End HideTextWin

/*---------------------------------------------------------------
 *  Callback to handle press of close button in message window
 */

void
ReadWinP::HideMsgWin(ReadWinC *win, ReadWinP *This)
{
//
// Loop through the list of icons and see which one corresponds to this
//    window
//
   PtrListC&	list  = This->msgBodyText->GraphicList();
   u_int	count = list.size();
   for (int i=0; i<count; i++) {
      ReadIconC	*icon = (ReadIconC*)*list[i];
      if ( icon->msgWin == win ) {
	 icon->msgWin = NULL;	// So it doesn't get hidden again
	 This->HideIcon(icon);
	 return;
      }
   }

} // End HideMsgWin

/*-----------------------------------------------------------------------
 *  Handle request for hide of mime part
 */

void
ReadWinP::DoHidePart(Widget, ReadWinP *This, XtPointer)
{
   This->HideIcon(This->popupIcon);
}

/*-----------------------------------------------------------------------
 *  Method to hide the window for a mime attachment
 */

void
ReadWinP::HideIcon(ReadIconC *icon)
{
   if ( icon->textWin ) {
      LocalTextWinC	*win = icon->textWin;
      icon->textWin = NULL;
      win->Hide();
   }

   if ( icon->msgWin ) {
      ReadWinC	*win = icon->msgWin;
      icon->msgWin = NULL;
      win->Hide();
   }

//
// See if there is an external display process
//
   DisplayDataT	*data = DisplayData(icon);
   if ( data && data->pid > 0 )
      if ( kill(-data->pid, 0) == 0 ) kill(-data->pid, SIGKILL);

   icon->AnimationOff();
   icon->Unhighlight();

} // End HideIcon

/*-----------------------------------------------------------------------
 *  Handle request for save of mime part
 */

void
ReadWinP::DoSavePart(Widget, ReadWinP *This, XtPointer)
{
   MsgPartC	*part = This->popupIcon->part;

   This->pub->ClearMessage();

//
// See if this part needs to be retrieved first
//
   if ( part->NeedFetch() && !This->OkToGet(part, "getString") ) return;

   if ( !This->savePartWin ) {
      This->pub->BusyCursor(True);
      This->savePartWin = new FileChooserWinC(*This->pub, "savePartWin");
      This->savePartWin->AddOkCallback((CallbackFn*)FinishSavePart, This);
      This->savePartWin->SingleSelect(True);
      This->savePartWin->HideList();
      This->savePartWin->SetDirectory(".");
      This->savePartWin->SetDefaultDir(".");
      This->savePartWin->ShowDirsInFileList (False);
      This->savePartWin->ShowFilesInFileList(True);
      This->savePartWin->HideImap();
      This->pub->BusyCursor(False);
   }

   ParamC	*param = part->Param("filename", part->disParams);
   if ( !param ) param = part->Param("name",     part->disParams);
   if ( !param ) param = part->Param("name",     part->accParams);
   if ( !param ) param = part->Param("name");

   if ( param )
      TextFieldSetString(This->savePartWin->SelectTF(), param->val);

   This->savePartWin->Show();

} // End DoSavePart

/*-----------------------------------------------------------------------
 *  Callback to handle ok of mime part save with filename
 */

void
ReadWinP::FinishSavePart(StringListC *list, ReadWinP *This)
{
   This->savePartWin->HideOk(True);

//
// See if the name is OK
//
   StringC	*nameStr = (*list)[0];

   switch ( OverwriteQuery(*nameStr, *This->pub) ) {

      case (QUERY_YES):
	 break;

      case (QUERY_NO):
      case (QUERY_NONE):
	 if ( debuglev > 0 ) cout <<"Didn't want to overwrite" NL;
	 This->savePartWin->HideOk(False);
	 return;

      case (QUERY_CANCEL):
	 if ( debuglev > 0 ) cout <<"Operation cancelled" NL;
	 return;

   } // End switch OverwriteQuery

//
// See if the part needs to be retrieved first.  We've already asked the user
//    if this is ok.
//
   MsgPartC	*part = This->popupIcon->part;
   if ( part->NeedFetch() )
      This->FetchPart(This->popupIcon, *nameStr, (CallbackFn*)SaveAfterFetch);
   else
      This->SavePart(This->popupIcon, *nameStr);

} // End FinishSavePart

/*-----------------------------------------------------------------------
 *  Finish request for save of MIME attachment.  This one is called
 *     after a fetch completes
 */

void
ReadWinP::SaveAfterFetch(ReadIconC *icon, ReadWinP*)
{
//
// We fetched directly to the destination so this is all we have to do
//
   icon->part->delDataFile = False;
}

/*-----------------------------------------------------------------------
 *  Finish request for save of MIME attachment.  This one is called
 *     when file is already available
 */

void
ReadWinP::SavePart(ReadIconC *icon, const char *name)
{
   pub->BusyCursor(True);

//
// Create the file and any directories that might be needed
//
   if ( !MakeFile(name) ) return;

   StringC	body;
   icon->part->GetData(body);

   StringC	statmsg;
   if ( !body.WriteFile((char*)name) ) {
      int	err = errno;
      statmsg = "Could not save to file: ";
      statmsg += name;
      statmsg += ".\n";
      statmsg += SystemErrorMessage(err);
      pub->PopupMessage(statmsg);
      pub->BusyCursor(False);
      return;
   }

   statmsg = "Part ";
   statmsg += icon->part->partNum;
   statmsg += " saved to file: ";
   statmsg += name;
   pub->Message(statmsg);
   pub->BusyCursor(False);

} // End SavePart

/*-----------------------------------------------------------------------
 *  Handle request for print of mime part
 */

void
ReadWinP::DoPrintPart(Widget, ReadWinP *This, XtPointer)
{
   MsgPartC	*part = This->popupIcon->part;

   This->pub->ClearMessage();

//
// See if this part needs to be retrieved first
//
   if ( part->NeedFetch() ) {

//
// Ask the user is this is OK
//
      if ( !This->OkToGet(part, "getString") ) return;

      This->FetchPart(This->popupIcon, NULL, (CallbackFn*)FinishPrintPart);
   }

   else
      FinishPrintPart(This->popupIcon, This);

} // End DoPrintPart

/*-----------------------------------------------------------------------
 *  Finish request for print of MIME attachment.  This one is called
 *     after we know the file is present.
 */

void
ReadWinP::FinishPrintPart(ReadIconC *icon, ReadWinP *This)
{
   MsgPartC	*part = icon->part;

//
// Get the part into an external file
//
   if ( !part->CreateDataFile() ) return;

//
// Get the mailcap entry
//
   StringC	cmdStr;
   MailcapC	*mcap = MailcapEntry(part);
   if ( mcap && mcap->print.size() > 0 ) cmdStr = mcap->print;
   else					 cmdStr = ishApp->appPrefs->printCmd;

//
// Create some callback data
//
   PrintDataT	*data = new PrintDataT;
   data->win    = This;
   data->part   = part;
   data->cmdStr = BuildCommand(cmdStr, part, part->dataFile);
   CallbackC	doneCb((CallbackFn*)PrintDone, data);

   data->pid = ForkIt(data->cmdStr, &doneCb);
   if ( data->pid < 0 ) {

      StringC	errmsg = "Could not print part ";
      errmsg += part->partNum;
      errmsg += "\n" + ForkStatusMsg(data->cmdStr, (int)data->pid);
      This->pub->PopupMessage(errmsg);

      delete data;
   }

   else {
      StringC	statmsg("Part ");
      statmsg += part->partNum;
      statmsg += " sent to printer.";
      This->pub->Message(statmsg);
   }

} // End FinishPrintPart

/*---------------------------------------------------------------
 *  Callback for completion of ForkIt print job
 */

void
ReadWinP::PrintDone(int status, PrintDataT *data)
{
   if ( debuglev > 0 ) cout <<"Print process finished" <<endl;

   ReadWinP	*This = data->win;
   MsgPartC	*part = data->part;

//
// Report status of operation
//
   if ( status != 0 ) {

      StringC	errmsg("Part ");
      errmsg += part->partNum;
      errmsg += " could not be printed:\n";
      errmsg += ForkStatusMsg(data->cmdStr, status, data->pid);
      This->pub->PopupMessage(errmsg);

   } // End if there is an error

   if ( data->file.size() > 0 ) {
      if ( debuglev > 0 ) cout <<"Unlinking temp file" <<endl;
      unlink(data->file);
   }

   delete data;

} // End PrintDone

/*-----------------------------------------------------------------------
 *  Handle request for fetch of mime part
 */

void
ReadWinP::DoFetchPart(Widget, ReadWinP *This, XtPointer)
{
   This->FetchPart(This->popupIcon, NULL, (CallbackFn*)NULL);
}

void
ReadWinP::FetchPart(ReadIconC *icon, const char *filename, CallbackFn *doneCall)
{
   MsgPartC	*part   = icon->part;
   Boolean	doFetch = True;

   icon->Highlight();

//
// If we've already done a fetch, see if the user wants to do it again
//
   if ( !part->NeedFetch() )
      doFetch = OkToGet(part, "getAgainString");

   if ( !doFetch ) {
      icon->Unhighlight();
      return;
   }

//
// Prompt for user name and password if necessary
//
   if ( part->IsFTP() && part->userName.size() == 0 ) {

//
// Look up the ftp address
//
      ParamC	*site = part->Param("site");
      if ( !site ) {
	 StringC errmsg("The FTP site name was not specified for file: ");
	 part->GetFileName(errmsg);
	 errmsg += ".\nThe file could not be retrieved.";
	 pub->PopupMessage(errmsg);
	 icon->Unhighlight();
	 return;
      }

//
// Prompt for login
//
      if ( !loginWin ) loginWin = new LoginWinC(*pub);

      if ( !loginWin->GetLogin(site->val, part->userName, part->userPass) ) {
	 icon->Unhighlight();
	 return;
      }

   } // End if login is needed

//
// For security reasons, confirm retrieval of mail-server attachments.
// We don't want someone sending an attachment that will cause someone
//   else to send obnoxious mail.
//
   if ( part->IsMail() ) {

      if ( !OkToSend(part) ) {
	 icon->Unhighlight();
	 return;
      }
   }

//
// Look for a mailcap entry for message/external-body
//
   MailcapC	*mcap = MailcapEntry(MESSAGE_S, EXTERNAL_BODY_S, part);
   if ( !mcap ) {
      StringC errmsg = "I could not find a mailcap entry for: ";
      errmsg += "message/external-body.";
      errmsg += ".\nThe file could not be retrieved.";
      pub->PopupMessage(errmsg);
      icon->Unhighlight();
      return;
   }

//
// See if there's a fetch command
//
   if ( mcap->present.size() == 0 ) {
      StringC errmsg = "The mailcap entry for message/external-body does ";
      errmsg += "not specify a command for getting the file.";
      errmsg += ".\nThe file could not be retrieved.";
      pub->PopupMessage(errmsg);
      icon->Unhighlight();
      return;
   }

//
// Clear out the old external file name
//
   if ( part->delDataFile && part->dataFile.size() > 0 ) {
      unlink(part->dataFile);
      part->dataFile.Clear();
      part->delDataFile = False;
   }

//
// Create a new name for the external file
//
   StringC	file;
   if ( filename )
      file = filename;
   else {
      char	*cs = tempnam(NULL, "ext.");
      file = cs;
      free(cs);
   }

//
// If this is a mail server attachment, write the commands to the temp file
//
   if ( part->IsMail() ) {

      StringC	body;
      part->GetText(body);
      if ( !body.WriteFile(file) ) {
	 StringC errmsg = "Could not write file \"";
	 errmsg += file;
	 errmsg += "\" to send to mail server.\n";
	 errmsg += SystemErrorMessage(errno);
	 errmsg += ".\nThe file could not be retrieved.";
	 pub->PopupMessage(errmsg);
	 icon->Unhighlight();
	 return;
      }

   } // End if mail server

//
// Build a data record to store info about this fetch
//
   FetchDataT	*data = new FetchDataT;
   data->win  = this;
   data->icon = icon;
   data->part = part;
   data->file = file;
   data->postFetchCall.Set(doneCall, (void*)this);

//
// Build the command for retrieval.  The present command is used since this
//   is a message/external-body type.
//
   data->cmdStr = BuildCommand(mcap->present, part, file);
   data->pid    = -1;

   CallbackC	doneCb((CallbackFn*)FetchDone, data);
   data->pid = ForkIt(data->cmdStr, &doneCb);

//
// If the pid is < 0, an error occurred
//
   if ( data->pid < 0 ) {
      pub->PopupMessage(ForkStatusMsg(data->cmdStr, (int)data->pid));
      icon->Unhighlight();
      delete data;
   }

//
// Add this process to the list of running processes
//
   else {

      void	*tmp = (void*)data;
      fetchDataList->add(tmp);

//
// Start the retrieval animation
//
      LoadFetchPixmaps(icon);
      icon->Animate(fetchPixmaps);
   }

} // End FetchPart

/*---------------------------------------------------------------
 *  Callback called when retrieval process stops
 */

void
ReadWinP::FetchDone(int status, FetchDataT *data)
{
   ReadWinP	*This = data->win;

//
// Remove this data from the list of active fetches
//
   void	*tmp = (void*)data;
   fetchDataList->remove(tmp);

//
// Make sure the icon is still around
//
   ReadIconC	*icon = data->icon;
   PtrListC&	list  = This->msgBodyText->GraphicList();
   u_int	count = list.size();
   Boolean	found = False;
   for (int i=0; !found && i<count; i++) {
      ReadIconC	*ip = (ReadIconC*)*list[i];
      found = (ip == icon);
   }

   MsgPartC	*part = data->part;
   StringC	name;
   part->GetFileName(name);

   if ( debuglev > 0 ) cout <<"FetchDone for file: " <<name <<endl;

//
// Turn off the retrieval animation
//
   if ( found ) icon->AnimationOff();

//
// Report status if reading window is no longer visible or if the message
//    has changed or if this is a mail server attachment that we don't yet
//    have.
//
   if ( !This->pub->IsShown() || This->pub->msg != part->parentMsg ||
        status != 0 || part->IsMail() ) {

      StringC	statmsg("File \"");
      statmsg += name;

      if ( status != 0 ) {
	 statmsg += "\" could not be retrieved.\n";
	 statmsg += ForkStatusMsg(data->cmdStr, status, data->pid);
      }
      else if ( part->IsMail() )
	 statmsg += "\" will arrive by mail at a later time.";
      else
	 statmsg += "\" has arrived.";

      if ( This->pub->IsShown() )
	 This->pub->PopupMessage(statmsg);
      else
	 halApp->PopupMessage(statmsg);

   } // End if reading window is no longer visible or message has changed

//
// Save the file name if necessary
//
   if ( !part->IsMail() && status == 0 ) {

      part->dataFile    = data->file;
      part->delDataFile = True;

//
// See if there is a call to make now.  This could be a display or save.
//
      if ( found ) {
	 CallbackC	dummy(NULL, (void*)This);
	 if ( data->postFetchCall != dummy )
	    data->postFetchCall(icon);
	 else
	    icon->Unhighlight();
      }
   }

   else if ( found )
      icon->Unhighlight();

   delete data;
   return;

} // End FetchDone

/*---------------------------------------------------------------
 *  Method to display a dialog when there's no mailcap entry for a specified
 *     type.
 */

void
ReadWinP::NoMailcap(ReadIconC *icon)
{
   MsgPartC	*part = icon->part;
   static Widget	noMailcapWin = NULL;

   if ( !noMailcapWin ) {

      pub->BusyCursor(True);

      WArgList	args;
      args.DialogStyle(XmDIALOG_FULL_APPLICATION_MODAL);
      if ( halApp->questionPM ) args.SymbolPixmap(halApp->questionPM);
      noMailcapWin = XmCreateQuestionDialog(*pub, "noMailcapWin", ARGS);

      Widget textPB = XmCreatePushButton(noMailcapWin, "textPB", 0,0);
      XtManageChild(textPB);

      XtAddCallback(noMailcapWin, XmNokCallback, (XtCallbackProc)NoMailcapSave,
		    (XtPointer)this);
      XtAddCallback(noMailcapWin, XmNcancelCallback,
		    (XtCallbackProc)NoMailcapCancel, (XtPointer)this);
      XtAddCallback(textPB, XmNactivateCallback, (XtCallbackProc)NoMailcapText,
		    (XtPointer)this);
      XtAddCallback(noMailcapWin, XmNhelpCallback,
		    (XtCallbackProc)HalAppC::DoHelp, (char *) "helpcard");

      pub->BusyCursor(False);

   } // End if dialog not created

   noMailcapIcon = icon;

//
// Build the message string
//
   StringC	errmsg = "I could not find a mailcap entry for: ";
   errmsg += part->conStr;
   errmsg += "\nWould you like to save it to a file or display it as text?";

   WXmString    wstr = (char *)errmsg;
   XtVaSetValues(noMailcapWin, XmNmessageString, (XmString)wstr, NULL);

//
// Show the dialog
//
   PopupOver(noMailcapWin, *pub);

} // End NoMailcap

/*---------------------------------------------------------------
 *  Callback to save body part to file when there's no mailcap entry
 */

void
ReadWinP::NoMailcapSave(Widget, ReadWinP *This, XtPointer)
{
   This->noMailcapIcon->Unhighlight();
   This->popupIcon = This->noMailcapIcon;

   DoSavePart(NULL, This, NULL);
}

/*---------------------------------------------------------------
 *  Callback to display body part as text when there's no mailcap entry
 */

void
ReadWinP::NoMailcapText(Widget, ReadWinP *This, XtPointer)
{
   This->ShowText(This->noMailcapIcon);
}

/*---------------------------------------------------------------
 *  Callback to cancel body part save when there's no mailcap entry
 */

void
ReadWinP::NoMailcapCancel(Widget, ReadWinP *This, XtPointer)
{
   This->noMailcapIcon->Unhighlight();
}

/*---------------------------------------------------------------
 *  Method to query user to see if part should be retrieved
 */

Boolean
ReadWinP::OkToGet(MsgPartC *part, char *messageString)
{
   static QueryAnswerT	answer;
   static Widget	getQueryWin = NULL;

//
// Create the dialog if necessary
//
   if ( !getQueryWin ) {

      pub->BusyCursor(True);

      WArgList	args;
      args.DialogStyle(XmDIALOG_FULL_APPLICATION_MODAL);
      if ( halApp->questionPM ) args.SymbolPixmap(halApp->questionPM);
      Widget	w = XmCreateQuestionDialog(*pub, "getQueryWin", ARGS);

      XtAddCallback(w, XmNokCallback, (XtCallbackProc)AnswerQuery,
				      (XtPointer)&answer);
      XtAddCallback(w, XmNcancelCallback, (XtCallbackProc)AnswerQuery,
					  (XtPointer)&answer);
      XtAddCallback(w, XmNhelpCallback, (XtCallbackProc)HalAppC::DoHelp,
		    (char *) "helpcard");

//
// Trap window manager close function
//
      XmAddWMProtocolCallback(XtParent(w), halApp->delWinAtom,
			      (XtCallbackProc)WmClose,
			      (caddr_t)&answer);

      getQueryWin = w;
      pub->BusyCursor(False);

   } // End if dialog not created

//
// Insert the file name
//
   StringC	str = get_string(getQueryWin, messageString, "Retrieve $NAME?");
   StringC	name;
   part->GetFileName(name);
   str.Replace("$NAME", name);

   WXmString	wxmstr = (char*)str;
   XtVaSetValues(getQueryWin, XmNmessageString, (XmString)wxmstr, NULL);

//
// Show the dialog
//
   PopupOver(getQueryWin, *pub);

//
// Simulate the main event loop and wait for the answer
//
   answer = QUERY_NONE;
   while ( answer == QUERY_NONE ) {
      XtAppProcessEvent(halApp->context, XtIMXEvent);
      XSync(halApp->display, False);
   }

   XtUnmanageChild(getQueryWin);
   XSync(halApp->display, False);
   XmUpdateDisplay(getQueryWin);

//
// Set state based on user's answer
//
   return (answer == QUERY_YES);

} // End OkToGet

/*---------------------------------------------------------------
 *  Method to query user to see if mail-server message should be sent
 */

Boolean
ReadWinP::OkToSend(MsgPartC *part)
{
   static QueryAnswerT	answer;
   static Widget	sendQueryWin = NULL;

//
// Create the dialog if necessary
//
   if ( !sendQueryWin ) {

      pub->BusyCursor(True);

      WArgList	args;
      args.DialogStyle(XmDIALOG_FULL_APPLICATION_MODAL);
      if ( halApp->questionPM ) args.SymbolPixmap(halApp->questionPM);
      Widget	w = XmCreateQuestionDialog(*pub, "sendQueryWin", ARGS);

      XtAddCallback(w, XmNokCallback, (XtCallbackProc)AnswerQuery,
				      (XtPointer)&answer);
      XtAddCallback(w, XmNcancelCallback, (XtCallbackProc)AnswerQuery,
					  (XtPointer)&answer);
      XtAddCallback(w, XmNhelpCallback, (XtCallbackProc)HalAppC::DoHelp,
					(char *) "helpcard");

//
// Trap window manager close function
//
      XmAddWMProtocolCallback(XtParent(w), halApp->delWinAtom,
			      (XtCallbackProc)WmClose,
			      (caddr_t)&answer);

      sendQueryWin = w;
      pub->BusyCursor(False);

   } // End if dialog not created

//
// Add the recipient name
//
   StringC	label = get_string(sendQueryWin, "messageString",
			"Send following message to \"$SERVER\".\n\n$MESSAGE");

   ParamC	*server = part->Param("server");
   if ( !server ) {
      label = "There is no mail server specified for this body part.";
      pub->PopupMessage(label);
      return False;
   }

   label.Replace("$SERVER", server->val);

//
// Add the body
//
   StringC	body;
   part->GetText(body);

   int	pos = label.PosOf("$MESSAGE");
   if ( pos < 0 ) {
      while ( !label.EndsWith("\n\n") ) label += '\n';
      label += body;
   }
   else
      label(pos, 8) = body;

   WXmString	wxmstr = (char*)label;
   XtVaSetValues(sendQueryWin, XmNmessageString, (XmString)wxmstr, NULL);

//
// Show the dialog
//
   PopupOver(sendQueryWin, *pub);

//
// Simulate the main event loop and wait for the answer
//
   answer = QUERY_NONE;
   while ( answer == QUERY_NONE ) {
      XtAppProcessEvent(halApp->context, XtIMXEvent);
      XSync(halApp->display, False);
   }

   XtUnmanageChild(sendQueryWin);
   XSync(halApp->display, False);
   XmUpdateDisplay(sendQueryWin);

//
// Set state based on user's answer
//
   return (answer == QUERY_YES);

} // End OkToSend

/*---------------------------------------------------------------
 *  Method to load pixmaps used for file retrieval animation
 */

void
ReadWinP::LoadFetchPixmaps(ReadIconC *icon)
{
   if ( fetchPixmaps->size() > 0 ) return;

   Window       win = RootWindowOfScreen(halApp->screen);
   if ( !win ) return;

   StringC	names;
   StringListC	nameList;
   nameList.AllowDuplicates(TRUE);

   names = get_string(*pub, "retrievalPixmaps", "retrieval-0.xbm" );
   ExtractList(names, nameList);;

   u_int	count = nameList.size();
   for (int i=0; i<count; i++) {

      StringC	*name = nameList[i];
      PixmapC	*pm   = new PixmapC(*name, icon->regFgColor, icon->regBgColor,
      					   icon->invFgColor, icon->invBgColor,
					   halApp->screen, win);
      void	*tmp = (void*)pm;
      fetchPixmaps->add(tmp);
   }

} // End LoadFetchPixmaps

/*---------------------------------------------------------------
 *  Method to load pixmaps used for part display animation
 */

void
ReadWinP::LoadDisplayPixmaps(ReadIconC *icon)
{
   if ( displayPixmaps->size() > 0 ) return;

   Window       win = RootWindowOfScreen(halApp->screen);
   if ( !win ) return;

   StringC	names;
   StringListC	nameList;
   nameList.AllowDuplicates(TRUE);

   names = get_string(*pub, "displayPixmaps", "display-0.xbm" );
   ExtractList(names, nameList);;

   u_int	count = nameList.size();
   for (int i=0; i<count; i++) {

      StringC	*name = nameList[i];
      PixmapC	*pm   = new PixmapC(*name, icon->regFgColor, icon->regBgColor,
      					   icon->invFgColor, icon->invBgColor,
					   halApp->screen, win);
      void	*tmp = (void*)pm;
      displayPixmaps->add(tmp);
   }

} // End LoadDisplayPixmaps

/*---------------------------------------------------------------
 *  Method to determine if a fetch is in progress for the specified part
 */

FetchDataT*
ReadWinP::FetchData(MsgPartC *part)
{
   u_int	count = fetchDataList->size();
   for (int i=0; i<count; i++) {
      FetchDataT	*data = (FetchDataT*)*(*fetchDataList)[i];
      if ( data->part == part ) return data;
   }

   return NULL;
}

/*---------------------------------------------------------------
 *  Method to determine if a display is in progress for the specified icon
 */

DisplayDataT*
ReadWinP::DisplayData(ReadIconC *icon)
{
   u_int	count = displayDataList.size();
   for (int i=0; i<count; i++) {
      DisplayDataT	*data = (DisplayDataT*)*displayDataList[i];
      if ( data->icon == icon ) return data;
   }

   return NULL;
}

/*---------------------------------------------------------------
 *  Method to display the next message for a child rfc822 window.
 */

void
ReadWinP::ShowNext822(ReadWinC *win)
{
   ReadIconC	*thisIcon = IconWithWin(win);
   ReadIconC	*nextIcon = NextReadableIcon(win);
   if ( !nextIcon ) return;

   if ( thisIcon ) {
      thisIcon->msgWin = NULL;
      thisIcon->AnimationOff();
      thisIcon->Unhighlight();
   }

   nextIcon->msgWin = win;
   nextIcon->Animate(displayPixmaps);
   nextIcon->Highlight();

   FileMsgC		*msg = nextIcon->part->ChildMsg();
   win->SetMessage(msg);

} // End ShowNext822

/*---------------------------------------------------------------
 *  Method to display the previous message for a child rfc822 window.
 */

void
ReadWinP::ShowPrev822(ReadWinC *win)
{
   ReadIconC	*thisIcon = IconWithWin(win);
   ReadIconC	*prevIcon = PrevReadableIcon(win);
   if ( !prevIcon ) return;

   if ( thisIcon ) {
      thisIcon->msgWin = NULL;
      thisIcon->AnimationOff();
      thisIcon->Unhighlight();
   }

   prevIcon->msgWin = win;
   prevIcon->Animate(displayPixmaps);
   prevIcon->Highlight();

   FileMsgC		*msg = prevIcon->part->ChildMsg();
   win->SetMessage(msg);

} // End ShowPrev822

/*---------------------------------------------------------------
 *  Method to find the next readable message icon for a child rfc822 window.
 */

ReadIconC*
ReadWinP::NextReadableIcon(ReadWinC *win)
{
   ReadIconC	*icon = IconWithWin(win);
   if ( !icon ) return NULL;

//
// Find the next rfc822 part that doesn't have a message window
//
   MsgPartC	*nextPart = icon->part->next;
   ReadIconC	*nextIcon = NULL;

   while ( nextPart ) {

      if ( nextPart->Is822() ) {
	 nextIcon = IconWithPart(nextPart);
	 if ( nextIcon && nextIcon->msgWin == NULL )
	    return nextIcon;
      }

      nextPart = nextPart->next;

   } // End for each part

   return NULL;

} // End NextReadableIcon

/*---------------------------------------------------------------
 *  Method to find the previous readable message icon for a child rfc822 window.
 */

ReadIconC*
ReadWinP::PrevReadableIcon(ReadWinC *win)
{
   ReadIconC	*icon = IconWithWin(win);
   if ( !icon ) return NULL;

//
// Find the previous rfc822 part that doesn't have a message window
//
   MsgPartC	*prevPart = icon->part->prev;
   ReadIconC	*prevIcon = NULL;

   while ( prevPart ) {

      if ( prevPart->Is822() ) {
	 prevIcon = IconWithPart(prevPart);
	 if ( prevIcon && prevIcon->msgWin == NULL )
	    return prevIcon;
      }

      prevPart = prevPart->prev;

   } // End for each part

   return NULL;

} // End PrevReadableIcon

/*---------------------------------------------------------------
 *  Method to find the icon that is associated with the given
 *     reading window
 */

ReadIconC*
ReadWinP::IconWithWin(ReadWinC *win)
{
   PtrListC&	list  = msgBodyText->GraphicList();
   u_int	count = list.size();
   for (int i=0; i<count; i++) {
      ReadIconC	*icon  = (ReadIconC*)*list[i];
      if ( icon->msgWin == win ) return icon;
   }

   return NULL;

} // End IconWithWin

/*---------------------------------------------------------------
 *  Method to find the icon that is associated with the given message part
 */

ReadIconC*
ReadWinP::IconWithPart(MsgPartC *part)
{
   PtrListC&	list  = msgBodyText->GraphicList();
   u_int	count = list.size();
   for (int i=0; i<count; i++) {
      ReadIconC	*icon  = (ReadIconC*)*list[i];
      if ( icon->part == part ) return icon;
   }

   return NULL;

} // End IconWithPart
