/*
 * $Id: SendFile.C,v 1.13 2001/07/28 18:26:03 evgeny Exp $
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

#include "SendWinC.h"
#include "SendWinP.h"
#include "IshAppC.h"
#include "MailPrefC.h"
#include "ConfPrefC.h"
#include "Misc.h"
#include "Query.h"
#include "AliasPrefC.h"
#include "AddressC.h"
#include "date.h"
#include "MailFile.h"
#include "QuotedP.h"
#include "Base64.h"
#include "SigPrefC.h"
#include "Fork.h"
#include "FileMisc.h"
#include "FolderPrefC.h"
#include "FolderC.h"
#include "AppPrefC.h"
#include "SendIconC.h"
#include "SendMisc.h"
#include "MsgPartC.h"
#include "HeaderC.h"
#include "SafeSystem.h"
#include "CompPrefC.h"
#include "ImapMisc.h"

#include <hgl/MimeRichTextC.h>
#include <hgl/WArgList.h>
#include <hgl/RegexC.h>
#include <hgl/WXmString.h>
#include <hgl/rsrc.h>
#include <hgl/SysErr.h>
#include <hgl/WorkingBoxC.h>

#include <Xm/MessageB.h>
#include <Xm/Protocols.h>
#include <Xm/PushB.h>
#include <Xm/ToggleB.h>
#include <Xm/AtomMgr.h>

#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>

/*---------------------------------------------------------------
 *  Callback to handle clear
 */

void
SendWinP::DoClear(Widget, SendWinP *This, XtPointer)
{
   if ( This->pub->IsEditOnly() ) {
      FinishClear(NULL, This, NULL);
      return;
   }

   if ( This->pub->otherVis ) {
      This->otherStr.Clear();
      This->otherText->GetString(This->otherStr, TT_PLAIN);
   }

   Boolean    ccEmpty = (!This->pub->ccVis  || This->ccText->IsEmpty());
   Boolean   bccEmpty = (!This->pub->bccVis || This->bccText->IsEmpty());
   Boolean   fccEmpty = (!This->pub->fccVis || This->fccText->IsEmpty());
   Boolean otherEmpty = (!This->pub->otherVis ||
   			  This->otherText->IsEmpty() ||
			  (This->otherStr == ishApp->mailPrefs->otherHeaders));

   Boolean needClear = ( !This->toText->IsEmpty()   ||
			 !This->subText->IsEmpty()  ||
			 !This->bodyText->IsEmpty() ||
			 !ccEmpty || !bccEmpty || !otherEmpty || !fccEmpty);
   if ( !needClear ) return;

   if ( !ishApp->confPrefs->confirmClearSend ) {
      FinishClear(NULL, This, NULL);
      return;
   }

//
// Create the dialog if necessary
//
   if ( !This->clearWin ) {

      This->pub->BusyCursor(True);

      WArgList	args;
      args.Reset();
      args.DialogStyle(XmDIALOG_FULL_APPLICATION_MODAL);
      if ( halApp->questionPM ) args.SymbolPixmap(halApp->questionPM);
      This->clearWin = XmCreateQuestionDialog(*This->pub, "clearWin", ARGS);

      XtAddCallback(This->clearWin, XmNokCallback, (XtCallbackProc)FinishClear,
      		    (XtPointer)This);
      XtAddCallback(This->clearWin, XmNhelpCallback,
		    (XtCallbackProc)HalAppC::DoHelp, (char *) "helpcard");

      This->pub->BusyCursor(False);

   } // End if query dialog not created

//
// Show the dialog
//
   PopupOver(This->clearWin, *This->pub);

} // End DoClear

/*---------------------------------------------------------------
 *  Callback routine to handle press of OK in clear confirmation dialog
 */

void
SendWinP::FinishClear(Widget, SendWinP *This, XtPointer)
{
//
// Close all open edit windows
//
   u_int	count = This->editWinList.size();
   for (int i=0; i<count; i++) {
      SendWinC	*editWin = (SendWinC *)*This->editWinList[i];
      if ( editWin->IsShown() ) editWin->Close();
   }

   This->bodyText->Clear();

//
// Clear header fields
//
   if ( This->pub->IsEditOnly() ) {
      This->headText->Clear();
      This->pub->changed = True;
   }
   else {
      This->toText->Clear();
      This->subText->Clear();
      This->ccText->Clear();
      This->bccText->Clear();
      This->UpdateFcc();
      This->otherText->SetString(ishApp->mailPrefs->otherHeaders);
      This->pub->changed = False;
   }

   This->containerType = CT_MIXED;

} // End FinishClear

/*---------------------------------------------------------------
 *  Callback to handle close of window
 */

void
SendWinP::DoClose(Widget, SendWinP *This, XtPointer)
{
   This->pub->Close(/*save=*/This->pub->IsEditOnly());
}

/*---------------------------------------------------------------
 *  Callback to handle cancel of edit window
 */

void
SendWinP::DoCancel(Widget, SendWinP *This, XtPointer)
{
   This->pub->Close();
}

/*---------------------------------------------------------------
 *  Callbacks to handle send
 */

void
SendWinP::DoSendClose(Widget, SendWinP *This, XtPointer)
{
   if ( This->Send(/*closing=*/True) ) {
      This->pub->Message("Message sent.");
      This->pub->Close();
   }
   else
      This->pub->Message("Message NOT sent.");
}

void
SendWinP::DoSend(Widget, SendWinP *This, XtPointer)
{
   if ( This->Send() ) {

      This->pub->Message("Message sent.");
      This->bodyText->Clear();
      This->toText->Clear();
      if ( !This->pub->IsResend()  ) This->subText->Clear();
      This->ccText->Clear();
      This->bccText->Clear();
      This->UpdateFcc();
      This->otherText->SetString(ishApp->mailPrefs->otherHeaders);
      This->pub->changed = False;

   } // End if send completed ok

   else
      This->pub->Message("Message NOT sent.");

} // End DoSend

void
SendWinP::DoSendKeep(Widget, SendWinP *This, XtPointer)
{
   if ( This->Send() )
      This->pub->Message("Message sent.");
   else
      This->pub->Message("Message NOT sent.");
}

/*---------------------------------------------------------------
 *  Method to handle send
 */

Boolean
SendWinP::Send(Boolean closing)
{
   Boolean retVal = True;

   if ( debuglev > 0 ) cout <<"Entering Send" NL;
   pub->ClearMessage();

   if ( edit_pid ) {
      StringC	errmsg("There is an external edit in progress.\n");
      errmsg += "Please exit the editor before sending this message.";
      pub->PopupMessage(errmsg);
      return False;
   }

   if ( spell_pid ) {
      StringC	errmsg("There is an external spell-check in progress.\n");
      errmsg += "Please exit the spell checker before sending this message.";
      pub->PopupMessage(errmsg);
      return False;
   }

   pub->BusyCursor(True);

//
// See if the body or subject header is null
//
   if ( !pub->IsResend() ) {

      if ( bodyText->IsEmpty() && !NullBodyOk() ) {
	 pub->BusyCursor(False);
	 return False;
      }

      if ( subText->IsEmpty() && !NullSubjectOk() ) {
	 pub->BusyCursor(False);
	 return False;
      }
   }

//
// Build the list of recipients, the address header strings and the list of
//    fcc folders.
//
   if ( !ProcessHeaderFields() ) {
      pub->BusyCursor(False);
      return False;
   }

   if ( recipientList.size() == 0 ) {
      set_invalid(toText, True, True);
      pub->PopupMessage("Please enter a recipient");
      pub->BusyCursor(False);
      return False;
   }

//
// Check the addresses if so desired
//
   if ( XmToggleButtonGetState(optCheckAddrTB) && !CheckAddresses() ) {
      pub->BusyCursor(False);
      return False;
   }

//
// Create an array of message headers.
//
   StringListC	headList;
   StringListC	contList;
   if ( !BuildHeaders(headList, contList) ) {
      pub->BusyCursor(False);
      return False;
   }

//
// If this is a resend, the messages are already present.  Just loop and
//    send them.
//
   if ( pub->IsResend() ) {

//
// Merge lists
//
      u_int	count = contList.size();
      int	i;
      for (i=0; i<count; i++)
	 AddHeader(*contList[i], headList);

//
// Write the message headers to a file
//
      char      *cs = tempnam(NULL, "head.");
      StringC	headFile(cs);
      free(cs);

      if ( !WriteHeaders(headList, headFile, /*addBlank=*/False) ) {
	 unlink(headFile);
	 pub->BusyCursor(False);
	 return False;
      }

//
// Pass the header and body files to the sendmail command.  If we're closing
//    this window, we want the body to be deleted when the command finishes.
//
      Boolean	success = True;
      count = resendFileList.size();
      for (i=0; i<count; i++) {
	 StringC	*bodyFile = resendFileList[i];
	 success = success &&
	           MailFiles(headFile, *bodyFile, recipientList,
			     ishApp->mailPrefs->SendmailCmd(), /*wait=*/False,
			     /*deleteHead=*/(i==count-1),
			     /*deleteBody=*/closing);
      }

// Remove the files from this list so that SendWinC::Close doesn't try to
//    delete them.

      if ( closing ) resendFileList.removeAll();

#if 0
      if ( success ) {
	 if ( count > 1 ) pub->Message("Messages sent");
	 else	          pub->Message("Message sent");
      }
#endif

   } // End if this is a resend

   else {

//
// Create a file to hold the message body.
//
      char	*cs = tempnam(NULL, "body.");
      StringC	bodyFile(cs);
      free(cs);

//
// Write the message body to the file.  This may also generate additional
//    headers.
//
      OutgoingMailTypeT	mailType;
      if ( !WriteBody(bodyFile, headList, contList, &mailType) ) {
	 unlink(bodyFile);
	 pub->BusyCursor(False);
	 return False;
      }

//
// If there are any fcc folders, create a hard link to the body file so we
//    will still have a copy if SendMessage deletes the original
//
      StringC	fccBodyFile;
      if ( fccList.size() > 0 ) {

	 cs = tempnam(NULL, "body.");
	 fccBodyFile = cs;
	 free(cs);

	 link(bodyFile, fccBodyFile);
      }

//
// See if we need any crypto processing
//
      Boolean	digSign = XmToggleButtonGetState(optDigSignTB);
      Boolean	encrypt = XmToggleButtonGetState(optEncryptTB);
      if ( digSign || encrypt ) {

//
// Create a file to hold the crypto output
//
	 StringC	cryptFile;
	 if ( !WriteCryptBody(bodyFile, cryptFile, contList, mailType, digSign,
			    encrypt) ) {
	    unlink(cryptFile);
	    unlink(bodyFile);
	    pub->BusyCursor(False);
	    return False;
	 }

//
// Remove the original body file and use the crypto file in its place
//
         unlink(bodyFile);
	 bodyFile = cryptFile;

      } // End if message needs crypto

//
// Merge header lists
//
      u_int	count = contList.size();
      int	i;
      for (i=0; i<count; i++)
	 AddHeader(*contList[i], headList);

//
// Send the message.  For some reason, sending the message upsets the mail
//    check timer and it has to be restarted.
//
      Boolean	sent = SendMessage(bodyFile, headList, mailType);

      if ( !sent ) {
	 unlink(bodyFile);
	 if ( fccBodyFile.size() > 0 ) unlink(fccBodyFile);
	 ishApp->VerifyMailCheck(True/*restart*/);
	 pub->BusyCursor(False);
	 return False;
      }

#if 0
      pub->Message("Message sent.");
#endif

//
// Copy to Fcc folders
//
      if ( fccList.size() > 0 ) {
//
// Loop through folder names
//
	 StringC	path;
	 count = fccList.size();
	 for (i=0; i<count; i++)
	    if ( !PerformFcc(headList, fccBodyFile, *fccList[i]) )
		   retVal = False;	// Signal that a function of sending failed

	 fccList.removeAll();
	 unlink(fccBodyFile);

      } // End if there are any fcc folders

   } // End if not a resend

//
// Update the message status
//
   u_int	count = msgList.size();
   for (int i=0; i<count; i++) {

      MsgC	*msg = msgList[i];
      switch (pub->type) {

	 case (SEND_REPLY):
	    msg->SetReplied();
	    break;

	 case (SEND_FORWARD):
	    msg->SetForwarded();
	    break;

	 case (SEND_RESEND):
	    msg->SetResent();
	    break;

	 case (SEND_NEW):
	 case (SEND_EDIT):
	    break;
      }
   }

   ishApp->VerifyMailCheck(True/*restart*/);
   pub->changed = False;

   pub->BusyCursor(False);

   return retVal;

} // End Send

/*---------------------------------------------------------------
 *  Method to ask user if null message body is ok to send
 */

Boolean
SendWinP::NullBodyOk()
{
   if ( !ishApp->confPrefs->confirmSendNoBody ) return True;

   static QueryAnswerT	answer;
   static Widget	dialog = NULL;

//
// Create the dialog if necessary
//
   if ( !dialog ) {

      pub->BusyCursor(True);

      WArgList	args;
      args.DialogStyle(XmDIALOG_FULL_APPLICATION_MODAL);
      if ( halApp->questionPM ) args.SymbolPixmap(halApp->questionPM);
      dialog = XmCreateQuestionDialog(*pub, "nullBodyWin", ARGS);

      XtAddCallback(dialog, XmNokCallback, (XtCallbackProc)AnswerQuery,
		    (XtPointer)&answer);
      XtAddCallback(dialog, XmNcancelCallback,
		    (XtCallbackProc)AnswerQuery, (XtPointer)&answer);
      XtUnmanageChild(XmMessageBoxGetChild(dialog, XmDIALOG_HELP_BUTTON));

//
// Trap window manager close function
//
      XmAddWMProtocolCallback(XtParent(dialog), halApp->delWinAtom,
			      (XtCallbackProc)WmClose,
			      (caddr_t)&answer);

      pub->BusyCursor(False);

   } // End if query dialog not created

//
// Show the dialog
//
   PopupOver(dialog, *pub);

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

   return (answer == QUERY_YES);

} // End NullBodyOk

/*---------------------------------------------------------------
 *  Method to ask user if null subject is ok to send
 */

Boolean
SendWinP::NullSubjectOk()
{
   if ( !ishApp->confPrefs->confirmSendNoSubject ) return True;

   static QueryAnswerT	answer;
   static Widget	dialog = NULL;

//
// Create the dialog if necessary
//
   if ( !dialog ) {

      pub->BusyCursor(True);

      WArgList	args;
      args.DialogStyle(XmDIALOG_FULL_APPLICATION_MODAL);
      if ( halApp->questionPM ) args.SymbolPixmap(halApp->questionPM);
      dialog = XmCreateQuestionDialog(*pub, "nullSubjectWin", ARGS);

      XtAddCallback(dialog, XmNokCallback, (XtCallbackProc)AnswerQuery,
		    (XtPointer)&answer);
      XtAddCallback(dialog, XmNcancelCallback,
		    (XtCallbackProc)AnswerQuery, (XtPointer)&answer);
      XtUnmanageChild(XmMessageBoxGetChild(dialog, XmDIALOG_HELP_BUTTON));

//
// Trap window manager close function
//
      XmAddWMProtocolCallback(XtParent(dialog), halApp->delWinAtom,
			      (XtCallbackProc)WmClose,
			      (caddr_t)&answer);

      pub->BusyCursor(False);

   } // End if query dialog not created

//
// Show the dialog
//
   PopupOver(dialog, *pub);

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

   return (answer == QUERY_YES);

} // End NullSubjectOk

/*---------------------------------------------------------------
 *  Method to process address fields and extract real addresses,
 *     displayed addresses and file names.
 */

Boolean
SendWinP::ProcessHeaderFields()
{
   toStr.Clear();
   ccStr.Clear();
   otherStr.Clear();

//
// Get header strings
//
   StringC	tmpTo, tmpCc, tmpBcc;

   toText->GetString(tmpTo, TT_PLAIN);

   if ( pub->ccVis  )  ccText->GetString(tmpCc,  TT_PLAIN);
   if ( pub->bccVis ) bccText->GetString(tmpBcc, TT_PLAIN);

//
// Read fccStr if visible
//
   if ( pub->fccVis ) {
      fccStr.Clear();
      fccText->GetString(fccStr, TT_PLAIN);
   }
   fccList.removeAll();
   ExtractList(fccStr, fccList);

//
// Read the Additional headers field and look for Cc, Bcc or Fcc
// If "To:" is present, it will be used as the "visible" To field in the
//    mail message.  It is not checked for addresses.
//
   if ( pub->otherVis ) otherText->GetString(otherStr, TT_PLAIN);
   else			otherStr = ishApp->mailPrefs->otherHeaders;

   if ( otherStr.size() > 0 && 
        (otherStr.StartsWith("Cc:",  IGNORE_CASE) ||
         otherStr.StartsWith("Bcc:", IGNORE_CASE) ||
         otherStr.StartsWith("Fcc:", IGNORE_CASE) ||
         otherStr.Contains("\nCc:",  IGNORE_CASE) ||
         otherStr.Contains("\nBcc:", IGNORE_CASE) ||
         otherStr.Contains("\nFcc:", IGNORE_CASE)) ) {

      u_int	offset  = 0;
      CharC	lineStr = otherStr.NextWord(offset, '\n');
      StringC	headStr;
      Boolean	done = (lineStr.Length() == 0);
      while ( !done ) {

//
// See if we got a header
//
	 if ( !isspace(lineStr[0]) && headStr.size() > 0 ) {

	    if ( headStr.StartsWith("Cc:", IGNORE_CASE) ) {
	       headStr.CutBeg(3);
	       headStr.Trim();
	       if ( tmpCc.size() > 0 ) tmpCc += ", ";
	       tmpCc += headStr;
	    }

	    else if ( headStr.StartsWith("Bcc:", IGNORE_CASE) ) {
	       headStr.CutBeg(4);
	       headStr.Trim();
	       if ( tmpBcc.size() > 0 ) tmpBcc += ", ";
	       tmpBcc += headStr;
	    }

	    else if ( headStr.StartsWith("Fcc:", IGNORE_CASE) ) {
	       headStr.CutBeg(4);
	       headStr.Trim();
	       ExtractList(headStr, fccList);
	    }

	    headStr.Clear();

	 } // End if we got a new header

	 headStr += lineStr;

//
// Get another line
//
	 if ( lineStr.Length() == 0 )
	    done = True;
	 else {
	    offset = (lineStr.Addr() - (char*)otherStr) + lineStr.Length();
	    lineStr = otherStr.NextWord(offset, '\n');
	 }

      } // End for each line in otherStr

   } // End if otherStr contains addresses

//
// Replace newlines with commas
//
   RegexC npat = "[^,]\\([\n]+\\)[^,]"; // One or more newlines with no commas
   while ( npat.search( tmpTo) >= 0 )  tmpTo(npat[1]) = ", ";
   while ( npat.search( tmpCc) >= 0 )  tmpCc(npat[1]) = ", ";
   while ( npat.search(tmpBcc) >= 0 ) tmpBcc(npat[1]) = ", ";

//
// Look for file names in address fields.  Any that are found are added to
//   fccList
//
   RemoveFiles(tmpTo);
   RemoveFiles(tmpCc);
   RemoveFiles(tmpBcc);

//
// Expand for the text that will be visible to the message recipients.  We
//    don't expand hidden aliases here.
//
#define MAX_EXPANSION	16

   toStr = ishApp->aliasPrefs->ExpandAddress(tmpTo, MAX_EXPANSION, False);
   ccStr = ishApp->aliasPrefs->ExpandAddress(tmpCc, MAX_EXPANSION, False);

   RemoveQuotes(toStr);
   RemoveQuotes(ccStr);

//
// Fully expand to get actual addresses.
//
   tmpTo  = ishApp->aliasPrefs->ExpandAddress(tmpTo,  MAX_EXPANSION, True);
   tmpCc  = ishApp->aliasPrefs->ExpandAddress(tmpCc,  MAX_EXPANSION, True);
   tmpBcc = ishApp->aliasPrefs->ExpandAddress(tmpBcc, MAX_EXPANSION, True);

   RemoveQuotes(tmpTo);
   RemoveQuotes(tmpCc);
   RemoveQuotes(tmpBcc);

//
// Build list of recipients
//
   recipientList.removeAll();
   if ( !AddRecipients(tmpTo)  ) return False;
   if ( !AddRecipients(tmpCc)  ) return False;
   if ( !AddRecipients(tmpBcc) ) return False;

   return True;

} // End ProcessHeaderFields

/*---------------------------------------------------------------
 *  Method to remove any file names that may be present and add them
 *     to the fcc list
 */

void
SendWinP::RemoveFiles(StringC& head)
{
   static RegexC     *namePat  = NULL;
   if ( !namePat ) namePat  = new RegexC("[ \t]*\\([^,]+\\),?");

//
// Look through names
//
   CharC	name;
   int		findPos, startPos = 0;
   while ( (findPos=namePat->search(head,startPos)) >= 0 ) {

      startPos = findPos;
      name = head((*namePat)[1]);
      if (  (name.StartsWith('/')   ||
	     name.StartsWith("./")  ||
	     name.StartsWith("../") ||
	     name.StartsWith('+')   ||
	     name.StartsWith('=')) &&
	   !(name.Contains(' ') || name.Contains('\t')) ) {

	 StringC	tmp(name);
	 fccList.add(tmp);
	 head((*namePat)[0]) = "";
	 head.Trim();
      }
      else
	 startPos += (*namePat)[0].length();

   } // End for each name

   if ( head.EndsWith(',') ) {
      head.CutEnd(1);
      head.Trim();
   }

} // End RemoveFiles

/*---------------------------------------------------------------
 *  Method to remove the pairs of quotes from an address string
 */

void
SendWinP::RemoveQuotes(StringC& head)
{
   static RegexC     *namePat  = NULL;
   if ( !namePat ) namePat  = new RegexC("[ \t]*\\([^,]+\\),?");

//
// Look through names
//
   int		findPos, lastPos, startPos = 0;
   while ( (findPos=namePat->search(head,startPos)) >= 0 ) {

      startPos = findPos + (*namePat)[0].length();
      findPos = (*namePat)[1].firstIndex();
      lastPos = (*namePat)[1].lastIndex();

//
// Remove quotes
//
      if ( head[findPos] == '"' && head[lastPos] == '"' ) {
	 head(lastPos,1) = "";
	 head(findPos,1) = "";
	 startPos -= 2;
      }

   } // End for each name

} // End RemoveQuotes

/*---------------------------------------------------------------
 *  Method to extract the recipients from the given address string
 */

Boolean
SendWinP::AddRecipients(StringC& head)
{
   AddressC	addrList(head);
   AddressC	*addr = &addrList;
   StringC	tmp;
   while ( addr ) {

      tmp = addr->addr;
      if ( tmp.size() > 0 ) {

	 if ( !ishApp->mailPrefs->confirmAddrList.includes(tmp) ||
	      OkToSendTo(tmp) )
	    recipientList.add(tmp);
	 else {
	    pub->Message("Mail NOT sent");
	    return False;
	 }
      }

      addr = addr->next;

   } // End for each address

   return True;

} // End AddRecipients

/*---------------------------------------------------------------
 *  Method to ask user if an address requiring confirmation is ok
 */

Boolean
SendWinP::OkToSendTo(CharC addr)
{
   static QueryAnswerT	answer;
   static Widget	dialog = NULL;

//
// Create the dialog if necessary
//
   if ( !dialog ) {

      pub->BusyCursor(True);

      WArgList	args;
      args.DialogStyle(XmDIALOG_FULL_APPLICATION_MODAL);
      if ( halApp->questionPM ) args.SymbolPixmap(halApp->questionPM);
      dialog = XmCreateQuestionDialog(*pub, "confirmAddrWin", ARGS);

      XtAddCallback(dialog, XmNokCallback, (XtCallbackProc)AnswerQuery,
		    (XtPointer)&answer);
      XtAddCallback(dialog, XmNcancelCallback,
		    (XtCallbackProc)AnswerQuery, (XtPointer)&answer);
      XtUnmanageChild(XmMessageBoxGetChild(dialog, XmDIALOG_HELP_BUTTON));

//
// Trap window manager close function
//
      XmAddWMProtocolCallback(XtParent(dialog), halApp->delWinAtom,
			      (XtCallbackProc)WmClose,
			      (caddr_t)&answer);

      pub->BusyCursor(False);

   } // End if query dialog not created

//
// Store the address in the dialog
//
   StringC msg = get_string(dialog, "messageString", "Really send to $ADDR?");
   msg.Replace("$ADDR", addr);

   WXmString	wstr = (char *)msg;
   XtVaSetValues(dialog, XmNmessageString, (XmString)wstr, NULL);

//
// Show the dialog
//
   PopupOver(dialog, *pub);

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

   return (answer == QUERY_YES);

} // End OkToSendTo

/*---------------------------------------------------------------
 *  Method to build a list of the headers for an outgoing message
 */

Boolean
SendWinP::BuildHeaders(StringListC& headList, StringListC& contList)
{
   StringC	headStr;

   Boolean	resend = pub->IsResend();

//
// Add the Date header
//
   headStr.Clear();
   if ( resend ) headStr += "Resent-";
   headStr += "Date: ";
   headStr += get_822_date();
   if ( !AddHeader(headStr, headList) ) return False;

//
// Add the To header.  If there is a To: header in the others, that will be
//    the one displayed.
//
   if ( toStr.size() > 0 && !otherStr.StartsWith("To:") &&
			    !otherStr.Contains("\nTo:") ) {
      headStr.Clear();
      if ( resend ) headStr += "Resent-";
      headStr += "To: ";
      headStr += toStr;
      if ( !AddHeader(headStr, headList, /*dupOk=*/True) ) return False;
   }

//
// Add the Cc header
//
   if ( ccStr.size() > 0 ) {
      headStr.Clear();
      if ( resend ) headStr += "Resent-";
      headStr = "Cc: ";
      headStr += ccStr;
      if ( !AddHeader(headStr, headList, /*dupOk=*/True) ) return False;
   }

//
// Add any additional headers
//
   if ( otherStr.size() > 0 ) {

      headStr.Clear();
      u_int		offset  = 0;
      CharC		lineStr = otherStr.NextWord(offset, '\n');
      Boolean	done = (lineStr.Length() == 0);
      while ( !done ) {

//
// See if we started a new header
//
	 if ( !isspace(lineStr[0]) && headStr.size() > 0 ) {

//
// Only certain headers can be added to a resent message
//
	    if ( resend ) {

	       if ( headStr.StartsWith("Cc:", IGNORE_CASE) ||
		    headStr.StartsWith("To:", IGNORE_CASE) ) {

		  headStr = "Resent-" + headStr;
		  if ( !AddHeader(headStr, headList, /*dupOk=*/True) )
		     return False;
	       }

	       else if ( headStr.StartsWith("Date:",		IGNORE_CASE) ||
			 headStr.StartsWith("From:",		IGNORE_CASE) ||
			 headStr.StartsWith("Message-id:",	IGNORE_CASE) ||
			 headStr.StartsWith("Reply-to:",	IGNORE_CASE) ||
			 headStr.StartsWith("Sender:",		IGNORE_CASE) ) {

		  headStr = "Resent-" + headStr;
		  if ( !AddHeader(headStr, headList, /*dupOk=*/False) )
		     return False;
	       }
               
               else {
	          // Don't add the "Resent-" prefix for the rest
                  if ( !AddHeader(headStr, headList, /*dupOk=*/False) )
                     return False;
               }
	    }

	    else
	       if ( !AddHeader(headStr, headList) ) return False;

	    headStr.Clear();

	 } // End if we got a new header

	 headStr += lineStr;

//
// Get another line
//
	 if ( lineStr.Length() == 0 )
	    done = True;
	 else {
	    offset = (lineStr.Addr() - (char*)otherStr) + lineStr.Length();
	    lineStr = otherStr.NextWord(offset, '\n');
	 }

      } // End for each line in otherStr

   } // End if there are other headers

//
// If this is a resend, we're done now.
//
   if ( resend ) return True;

//
// Add the Subject header
//
   headStr.Clear();
   subText->GetString(headStr, TT_PLAIN);
   if ( headStr.size() > 0 ) {
      headStr = "Subject: " + headStr;
      if ( !AddHeader(headStr, headList) ) return False;
   }

//
// Add the From: header
//
   StringC fromHead("From: ");
   fromHead += ishApp->mailPrefs->fromHeader;
   AddHeader(fromHead, headList);

//
// Add an In-Reply-To header if necessary
//
   if ( pub->IsReply() && msgList.size() > 0 ) {
      MsgC	*msg = msgList[0];
      if ( msg->Id().size() > 0 ) {
	 headStr = "In-Reply-To: ";
	 headStr += msg->Id();
	 if ( !AddHeader(headStr, headList) ) return False;
      }
   }

//
// Add the X-Mailer header
//
   headStr = "X-Mailer: Ishmail";
   headStr += ' ';
   headStr += version;

   static StringC	*tagStr = NULL;
   if ( !tagStr ) {
      tagStr = new StringC;
      *tagStr = get_string(*pub, "xMailerTagLine");
   }
   if ( tagStr->size() > 0 ) headStr += " " + *tagStr;

   if ( !AddHeader(headStr, headList) ) return False;

//
// Add the mime-version header
//
   headStr = "MIME-Version: 1.0\n";
   if ( !AddHeader(headStr, headList) ) return False;

//
// Remove Status header.  Also check for the presence of a content-type
//    header
//
   u_int	count = headList.size();
   Boolean	contFound = False;
   for (int i=count-1; i>=0; i--) {
      StringC	*str = headList[i];
      if ( str->StartsWith("Status:", IGNORE_CASE) )
	 headList.remove(i);
      else if ( str->StartsWith("Content-", IGNORE_CASE) ) {
	 if ( str->StartsWith("Content-type:", IGNORE_CASE) )
	    contFound = True;
	 contList.add(*str);
	 headList.remove(i);
      }
   }

//
// Add a content-type header if not present
//
   if ( !contFound ) {
      headStr = "Content-Type: text/plain\n";
      if ( !AddHeader(headStr, contList) ) return False;
   }

   return True;

} // End BuildHeaders

/*---------------------------------------------------------------
 *  Method to write the body for an outgoing message to the given file.
 *  Some additional header may also be generated
 */

Boolean
SendWinP::WriteBody(char *bodyFile, StringListC& headList,
		    StringListC& contList, OutgoingMailTypeT *mailType,
		    Boolean saving)
{
//
// See what type of message to build
//
   MimeContentType	textType;
   Boolean		has8bit = False;

//
// Build the signature strings
//
   BuildSig(saving);

   Boolean richSig = (esigStr.size() > 0 &&
		      ishApp->sigPrefs->type == ENRICHED_SIG);

//
// If we're editing the source of a message from the reading window
//
   if ( editMsgText ) {
      *mailType = MAIL_PLAIN;
      textType  = CT_PLAIN;
   }

//
// If we're saving write a plain text message if possible
//
   else if ( saving || pub->IsEditOnly() ) {

      Boolean	hasGraphics = bodyText->HasGraphics();
      Boolean	hasMarkup   = bodyText->HasMarkup();
      has8bit               = bodyText->Has8Bit();

      if ( hasGraphics || hasMarkup || has8bit ) {
	 *mailType = MAIL_MIME;
	 textType  = CT_ENRICHED;
      }
      else {
	 *mailType = MAIL_PLAIN;
	 textType  = CT_PLAIN;
      }
   }

   else {

      if ( XmToggleButtonGetState(optMimeTB) ) {
	 if ( XmToggleButtonGetState(optMsgAltTB) ) *mailType = MAIL_ALT;
	 else					    *mailType = MAIL_MIME;
      }
      else
	 *mailType = MAIL_PLAIN;

      if ( XmToggleButtonGetState(optTextPlainTB) ) textType = CT_PLAIN;
      else					    textType = CT_ENRICHED;

//
// If the mail type is plain, make sure there is no markup or attachments
//
      if ( *mailType == MAIL_PLAIN ) {

	 Boolean canBePlain  = bodyText->CanBePlain();
	 Boolean hasGraphics = bodyText->HasGraphics();

	 if ( !canBePlain || hasGraphics || richSig ) {
	    if ( !OkToSendPlain(mailType) ) return False;
	 }

      } // End if trying to send plain mail

//
// If the text type is plain, make sure there is no markup
//
      if ( textType == CT_PLAIN &&
          (*mailType == MAIL_MIME || *mailType == MAIL_ALT) ) {

	 Boolean	canBePlain = bodyText->CanBePlain();
	 if ( !canBePlain || richSig ) {
	    if ( !OkToSendPlain(&textType) ) return False;
	 }

      } // End if trying to send plain text

//
// If the mail type is plain, make sure there are no 8-bit characters that
//    require encoding.
//
      if ( *mailType == MAIL_PLAIN ) {

	 has8bit = bodyText->Has8Bit() || Contains8Bit(sigStr);
	 if ( has8bit ) {
	    if ( !OkToSend8Bit(mailType) ) return False;
	 }

      } // End if trying to send plain mail

   } // End if we're not saving

//
// Open the body file
//
   FILE	*fp = fopen(bodyFile, "a");
   if ( !fp ) {
      StringC	errmsg("Could not open file \"");
      errmsg += bodyFile;
      errmsg += "\".\n";
      errmsg += SystemErrorMessage(errno);
      errmsg += "\nMessage will not be sent.";
      pub->PopupMessage(errmsg);
      return False;
   }

   Boolean error;
   if ( *mailType == MAIL_PLAIN )
      error = !WritePlainBody(fp, headList, contList, has8bit, saving);
   else
      error = !WriteMimeBody(fp, headList, contList, *mailType, textType,
			     saving);

   if ( error ) {
      StringC	errmsg("Could not write file \"");
      errmsg += bodyFile;
      errmsg += "\".\n";
      errmsg += SystemErrorMessage(errno);
      errmsg += "\nMessage will not be sent.";
      pub->PopupMessage(errmsg);
   }

   fclose(fp);
   return !error;

} // End WriteBody

/*---------------------------------------------------------------
 *  Method to build the plain and enriched signature strings
 */

void
SendWinP::BuildSig(Boolean saving)
{
   if ( saving || pub->IsEditOnly() || !XmToggleButtonGetState(optAddSigTB) ) {
      sigStr.Clear();
      esigStr.Clear();
      return;
   }

//
// See if any of the recipients is external.  If so we will use the external
//    signature.  External recipients are defined as those containing "@address"
//    where "address" is not the local domain.
//
   externalSig = False;
   u_int	count = recipientList.size();
   for (int i=0; i<count; i++) {

      CharC	recip = *recipientList[i];
      int	pos = recip.PosOf('@');

      if ( pos >= 0 ) {

	 recip.CutBeg(pos+1);
	 if ( recip.EndsWith(ishApp->domain, IGNORE_CASE) ) {
	    pos = recip.Length() - ishApp->domain.size() - 1;
	    if ( pos >= 0 && recip[pos] != '.' ) externalSig = True;
	 }
	 else
	    externalSig = True;
      }

   } // End for each recipient

//
// Build the plain signature string
//
   sigStr = "\n";
   if ( ishApp->sigPrefs->addPrefix ) sigStr += "-- \n";
   if ( externalSig ) sigStr += ishApp->sigPrefs->Sig();
   else		      sigStr += ishApp->sigPrefs->InternalSig();

//
// Build the enriched signature string if necessary
//
   esigStr.Clear();
   if ( ishApp->sigPrefs->type != PLAIN_SIG ) {
      esigStr = "\n\n";
      if ( ishApp->sigPrefs->addPrefix ) esigStr += "-- \n\n";
      if ( externalSig ) esigStr += ishApp->sigPrefs->EnrichedSig();
      else		 esigStr += ishApp->sigPrefs->InternalEnrichedSig();
   }

} // End BuildSig

/*---------------------------------------------------------------
 *  Method to ask user if a message with enriched text markup or attachments
 *     is ok to send as plain text.  Returns False if the send was cancelled.
 */

Boolean
SendWinP::OkToSendPlain(OutgoingMailTypeT *mailType)
{
   if ( !ishApp->confPrefs->confirmSendPlain ) return True;

   static QueryAnswerT	answer;
   static Widget	dialog = NULL;

//
// Create the dialog if necessary
//
   if ( !dialog ) {

      pub->BusyCursor(True);

      WArgList	args;
      args.DialogStyle(XmDIALOG_FULL_APPLICATION_MODAL);
      if ( halApp->questionPM ) args.SymbolPixmap(halApp->questionPM);
      dialog = XmCreateQuestionDialog(*pub, "confirmPlainWin", ARGS);

      XtAddCallback(dialog, XmNokCallback, (XtCallbackProc)AnswerQuery,
		    (XtPointer)&answer);
      XtAddCallback(dialog, XmNcancelCallback,
		    (XtCallbackProc)AnswerQuery, (XtPointer)&answer);
      XtUnmanageChild(XmMessageBoxGetChild(dialog, XmDIALOG_HELP_BUTTON));

      Widget mimePB = XmCreatePushButton(dialog, "mimePB", 0,0);
      XtManageChild(mimePB);

      AddActivate(mimePB, AnswerQuery, &answer);

//
// Trap window manager close function
//
      XmAddWMProtocolCallback(XtParent(dialog), halApp->delWinAtom,
			      (XtCallbackProc)WmClose,
			      (caddr_t)&answer);

      pub->BusyCursor(False);

   } // End if query dialog not created

//
// Show the dialog
//
   PopupOver(dialog, *pub);

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

   if      ( answer == QUERY_YES ) *mailType = MAIL_PLAIN;
   else if ( answer == QUERY_NO  ) *mailType = MAIL_MIME;
   else				   return False;

   return True;

} // End OkToSendPlain

/*---------------------------------------------------------------
 *  Method to ask user if a message with enriched text markup or attachments
 *     is ok to send as plain text.  Returns False if the send was cancelled.
 */

Boolean
SendWinP::OkToSendPlain(MimeContentType *textType)
{
   if ( !ishApp->confPrefs->confirmSendPlain ) return True;

   static QueryAnswerT	answer;
   static Widget	dialog = NULL;

//
// Create the dialog if necessary
//
   if ( !dialog ) {

      pub->BusyCursor(True);

      WArgList	args;
      args.DialogStyle(XmDIALOG_FULL_APPLICATION_MODAL);
      if ( halApp->questionPM ) args.SymbolPixmap(halApp->questionPM);
      dialog = XmCreateQuestionDialog(*pub, "confirmPlainTextWin", ARGS);

      XtAddCallback(dialog, XmNokCallback, (XtCallbackProc)AnswerQuery,
		    (XtPointer)&answer);
      XtAddCallback(dialog, XmNcancelCallback,
		    (XtCallbackProc)AnswerQuery, (XtPointer)&answer);
      XtUnmanageChild(XmMessageBoxGetChild(dialog, XmDIALOG_HELP_BUTTON));

      Widget richPB = XmCreatePushButton(dialog, "enrichedPB", 0,0);
      XtManageChild(richPB);

      AddActivate(richPB, AnswerQuery, &answer);

//
// Trap window manager close function
//
      XmAddWMProtocolCallback(XtParent(dialog), halApp->delWinAtom,
			      (XtCallbackProc)WmClose,
			      (caddr_t)&answer);

      pub->BusyCursor(False);

   } // End if query dialog not created

//
// Show the dialog
//
   PopupOver(dialog, *pub);

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

   if      ( answer == QUERY_YES ) *textType = CT_PLAIN;
   else if ( answer == QUERY_NO  ) *textType = CT_ENRICHED;
   else				   return False;

   return True;

} // End OkToSendPlain

/*---------------------------------------------------------------
 *  Method to ask user if a message with 8-bit characters is OK to
 *     send as plain text
 */

Boolean
SendWinP::OkToSend8Bit(OutgoingMailTypeT *mailType)
{
   if ( !ishApp->confPrefs->confirm8BitPlain ||
	 ishApp->mailPrefs->bodyEncType == ET_8BIT ) return True;

   static QueryAnswerT	answer;
   static Widget	dialog = NULL;

//
// Create the dialog if necessary
//
   if ( !dialog ) {

      pub->BusyCursor(True);

      WArgList	args;
      args.DialogStyle(XmDIALOG_FULL_APPLICATION_MODAL);
      if ( halApp->questionPM ) args.SymbolPixmap(halApp->questionPM);
      dialog = XmCreateQuestionDialog(*pub, "confirm8BitWin", ARGS);

      XtAddCallback(dialog, XmNokCallback, (XtCallbackProc)AnswerQuery,
		    (XtPointer)&answer);
      XtAddCallback(dialog, XmNcancelCallback,
		    (XtCallbackProc)AnswerQuery, (XtPointer)&answer);
      XtUnmanageChild(XmMessageBoxGetChild(dialog, XmDIALOG_HELP_BUTTON));

      Widget mimePB = XmCreatePushButton(dialog, "mimePB", 0,0);
      XtManageChild(mimePB);

      XtAddCallback(mimePB, XmNactivateCallback, (XtCallbackProc)AnswerQuery,
		    (XtPointer)&answer);

//
// Trap window manager close function
//
      XmAddWMProtocolCallback(XtParent(dialog), halApp->delWinAtom,
			      (XtCallbackProc)WmClose,
			      (caddr_t)&answer);

      pub->BusyCursor(False);

   } // End if query dialog not created

//
// Show the dialog
//
   PopupOver(dialog, *pub);

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

   if      ( answer == QUERY_YES ) *mailType = MAIL_PLAIN;
   else if ( answer == QUERY_NO  ) *mailType = MAIL_MIME;
   else				   return False;

   return True;

} // End OkToSend8Bit

/*---------------------------------------------------------------
 *  Method to write the body for an outgoing message to the given file.
 *  Some additional headers may also be generated
 */

Boolean
SendWinP::WritePlainBody(FILE *fp, StringListC& headList, StringListC& contList,
			 Boolean has8bit, Boolean saving)
{
   Boolean	error = False;

//
// Write headers
//
   if ( editMsg || (editIcon && editIcon->data->Is822()) ) {

      StringC	headData;
      headText->GetString(headData, TT_PLAIN);

      StringC	headStr;
      HeaderC	*head = ExtractHeaders(headData);
      while ( head && !error ) {

	 if ( editMsgText ||
	      (!head->key.StartsWith("Content-", IGNORE_CASE) &&
	       !head->key.Equals("Mime-Version", IGNORE_CASE)) ) {
	    headStr = head->full;
	    if ( headStr.size() > 0 ) {
	       if ( !headStr.EndsWith('\n') ) headStr += '\n';
	       error = !AddHeader(headStr, headList);
	    }
	 }

	 head = head->next;
      }

//
// Write a mime-version header
//
      if ( !editMsgText && !error ) {
	 headStr = "MIME-Version: 1.0\n";
	 error = !AddHeader(headStr, headList);
      }

      if ( error ) return False;

   } // End if we're writing an edited message part

//
// Write the body
//
   if ( !error ) {
      StringC	textStr;
//      bodyText->GetString(textStr, TT_PLAIN, saving ? 0 : 72);
      int wrapPos = 0;
      if ( XmToggleButtonGetState(optWrapTB) )
	 wrapPos = bodyText->ColumnCount();
      bodyText->GetString(textStr, TT_PLAIN, wrapPos);
      if ( !textStr.EndsWith("\n") ) textStr += '\n';
      error = !textStr.WriteFile(fp);
   }

//
// Write the signature
//
   if ( sigStr.size() > 0 && !error )
      error = !sigStr.WriteFile(fp);

//
// See if the body contains any 8-bit data
//
   if ( !editMsgText && has8bit && !error ) {

//
// Add the character set parameter if necessary
//
      StringC	headStr;
      CharC	charset = ishApp->mailPrefs->charset;
      Boolean	needsCharset = ( charset.Length() > 0 &&
			        !charset.Equals("us-ascii", IGNORE_CASE));
      if ( needsCharset ) {
	 headStr = "Content-Type: text/plain; charset=\"";
	 headStr += charset;
	 headStr += "\"";
	 error = !AddHeader(headStr, contList);
      }

//
// Add the encoding header
//
      if ( !error ) {
	 headStr = "Content-Transfer-Encoding: 8bit";
	 error = !AddHeader(headStr, contList);
      }

   } // End if the message body contains any 8 bit characters

   return !error;

} // End WritePlainBody

/*---------------------------------------------------------------
 *  Method to write the body for an outgoing MIME message to the given file.
 *  Some additional headers may also be generated
 */

Boolean
SendWinP::WriteMimeBody(FILE *fp, StringListC& headList, StringListC& contList,
			OutgoingMailTypeT mailType, MimeContentType textType,
			Boolean saving)
{
//
// See if we need a multipart/alternative wrapper
//
   Boolean hasSig      = (ishApp->sigPrefs->type == PLAIN_SIG
			       ? (sigStr.size() > 0)
			       : (esigStr.size() > 0));
   Boolean hasGraphics = bodyText->HasGraphics();

   if ( hasSig && !hasGraphics ) textType = CT_MIXED;

//
// See if the text will need a charset parameter
//
   CharC	charset      = ishApp->mailPrefs->charset;
   Boolean	needsCharset =
	    !(charset.Length() == 0 || charset.Equals("us-ascii", IGNORE_CASE));

//
// Start writing
//
   Boolean	useHeadList = True;
   Boolean	error       = False;
   StringC	altBound;
   StringC	headStr;
   StringC	bodyStr;
   CharC	nlStr("\n");
   headStr.AutoShrink(FALSE);
   bodyStr.AutoShrink(FALSE);

//
// If this is a message edit window, write all but the content headers
//
   if ( editIcon && editIcon->data->Is822() ) {

      headStr = "Content-Type: message/rfc822";
      if ( !AddHeader(headStr, contList) ) return False;

      useHeadList = False;
   }

   if ( editMsg || (editIcon && editIcon->data->Is822()) ) {

      StringC	headData;
      headText->GetString(headData, TT_PLAIN);

      HeaderC	*head = ExtractHeaders(headData);
      while ( head && !error ) {

	 if ( !head->key.StartsWith("Content-", IGNORE_CASE) &&
	      !head->key.Equals("Mime-Version", IGNORE_CASE) ) {
	    headStr = head->full;
	    if ( headStr.size() > 0 ) {
	       if ( !headStr.EndsWith('\n') ) headStr += '\n';
	       if ( useHeadList ) error = !AddHeader(headStr, headList);
	       else		  error = !headStr.WriteFile(fp);
	    }
	 }

	 head = head->next;
      }

//
// Write a mime-version header
//
      if ( !error ) {
	 headStr = "MIME-Version: 1.0\n";
	 if ( useHeadList ) error = !AddHeader(headStr, headList);
	 else		    error = !headStr.WriteFile(fp);
      }

      if ( error ) return False;

   } // End if we're writing an edited message part

//
// See if we need a multipart/alternative
//
   else if ( mailType == MAIL_ALT ) {

//
// Build the boundary string and Content-Type header
//
      GenBoundary(altBound);
      AddQuotedVal(headStr, "Content-Type: multipart/alternative;\n boundary=",
      		   altBound);
      if ( !AddHeader(headStr, contList) ) return False;

      useHeadList = False;

//
// Write the non-MIME blurb and the first boundary
//
      bodyStr = get_string(*pub, "altMimeString");
      bodyStr += "\n--";
      bodyStr += altBound;
      bodyStr += '\n';
      if ( !bodyStr.WriteFile(fp) ) return False;

//
// Add the plain text version of the message
//
      bodyStr.Clear();
//      bodyText->GetString(bodyStr, TT_PLAIN, saving ? 0 : 72);
      bodyText->GetString(bodyStr, TT_PLAIN, 72);
      if ( !bodyStr.EndsWith('\n') ) bodyStr += "\n";
      bodyStr += sigStr;

//
// Add headers if the body contains any 8-bit data
//
      Boolean	has8Bit = Contains8Bit(bodyStr);
      if ( has8Bit ) {

	 headStr = "Content-Type: text/plain";

//
// Add the character set parameter if necessary
//
	 if ( needsCharset ) {
	    headStr += "; charset=\"";
	    headStr += charset;
	    headStr += '"';
	 }
	 headStr += "\n";
	 if ( !headStr.WriteFile(fp) ) return False;
      }

//
// Mark the 8-bit data or encode it.
//
      if ( has8Bit ) {
	 headStr = "Content-Transfer-Encoding: ";
	 if ( ishApp->mailPrefs->bodyEncType == ET_8BIT )
	    headStr += "8bit\n";
	 else
	    headStr += "quoted-printable\n";
	 if ( !headStr.WriteFile(fp) ) return False;
      }

//
// Add a blank line
//
      if ( !nlStr.WriteFile(fp) ) return False;

//
// Write the body
//
      if ( has8Bit && ishApp->mailPrefs->bodyEncType == ET_QP )
	 error = !TextToFileQP(bodyStr, "mailfile", fp);
      else
	 error = !bodyStr.WriteFile(fp);
      if ( error ) return False;

//
// Add the next boundary
//
      headStr = "\n--";
      headStr += altBound;
      headStr += '\n';
      if ( !headStr.WriteFile(fp) ) return False;

   } // End if an alternative is needed

//
// If we have no graphics and no sig, it's just text
//
   if ( !hasGraphics && !hasSig ) {

//
// Get the text
//
      bodyStr.Clear();
      int	wrapPos = 0;
      if ( textType == CT_PLAIN ) {
//	 bodyText->GetString(bodyStr, TT_PLAIN, saving ? 0 : 72);
	 if ( XmToggleButtonGetState(optWrapTB) )
	    wrapPos = bodyText->ColumnCount();
	 bodyText->GetString(bodyStr, TT_PLAIN, wrapPos);
      }
      else {
//	 bodyText->GetString(bodyStr, TT_ENRICHED, saving ? 0 : 72);
	 if ( XmToggleButtonGetState(optWrapTB) ) wrapPos = 72;
	 bodyText->GetString(bodyStr, TT_ENRICHED, wrapPos);
      }
      if ( !bodyStr.EndsWith('\n') ) bodyStr += '\n';

//
// Write the text
//
      if ( !WriteTextPart(fp, textType, bodyStr,
			  needsCharset ? &charset  : (CharC*)NULL,
			  useHeadList  ? &contList : (StringListC*)NULL) )
         return False;

   } // End if single text part

//
// If the message is a single graphic, send that as the body
//
   else if ( hasGraphics && bodyText->IsSingleGraphic() ) {

      PtrListC&	list = bodyText->GraphicList();
      SendIconC	*icon = (SendIconC*)*list[0];

      if ( !icon ||
	   !icon->Write(fp, useHeadList ? &contList : (StringListC*)NULL) )
	 return False;

   } // End if message is a single graphic

//
// If we get to this point, we need a multipart
//
   else {

//
// Build the boundary string and Content-Type header
//
      StringC	bound;
      GenBoundary(bound);
      headStr = "Content-Type: multipart/";
      switch (containerType) {
	 case (CT_DIGEST):	headStr += "digest";		break;
#if 0	
// special version
	 case (CT_ALTERNATIVE):	headStr += "alternative";	break;
#endif
	 case (CT_PARALLEL):	headStr += "parallel";		break;
	 default:		headStr += "mixed";		break;
      }
      AddQuotedVal(headStr, ";\n boundary=", bound);
      headStr += '\n';

      if ( useHeadList ) error = !AddHeader(headStr, contList);
      else		 error = !headStr.WriteFile(fp);
      if ( error ) return False;

//
// Add non-mime blurb if one hasn't already been added
//
      if ( mailType != MAIL_ALT && !editIcon ) {
	 bodyStr = get_string(*pub, "nonMimeString");
	 if ( !bodyStr.EndsWith('\n') ) bodyStr += '\n';
	 if ( !bodyStr.WriteFile(fp) ) return False;
      }

//
// Add the text and graphics
//
      if ( !WriteMultipartBody(fp, bound, textType,
			       needsCharset ? &charset : (CharC*)NULL, saving) )
	 return False;

//
// Add the signature
//
      if ( hasSig ) {

//
// Add a boundary
//
	 headStr = "\n--";
	 headStr += bound;
	 headStr += "\nContent-Description: Signature\n";
	 if ( !headStr.WriteFile(fp) ) return False;

//
// Write the signature text
//
	 MimeContentType	sigType = textType;
	 if ( sigType == CT_ENRICHED && esigStr.size() == 0 )
	    sigType = CT_PLAIN;

	 StringC	*sig = (sigType == CT_ENRICHED ? &esigStr : &sigStr);
	 if ( !WriteTextPart(fp, sigType, *sig,
	     		     needsCharset ? &charset : (CharC*)NULL, NULL) )
	    return False;

      } // End if adding signature

//
// Add the closing boundary
//
      headStr  = "\n--";
      headStr += bound;
      headStr += "--\n";
      if ( !headStr.WriteFile(fp) ) return False;

   } // End if sig added

//
// Add the closing boundary of the alternative multipart if necessary
//
   if ( mailType == MAIL_ALT ) {
      headStr  = "\n--";
      headStr += altBound;
      headStr += "--\n";
      if ( !headStr.WriteFile(fp) ) return False;
   }

   return True;

} // End WriteMimeBody

/*---------------------------------------------------------------
 *  Method to read the parts from the bodyText widget and build
 *     a MIME multipart/mixed message body.  The headers have already
 *     been added.
 */

Boolean
SendWinP::WriteMultipartBody(FILE *fp, CharC bound, MimeContentType textType,
			     CharC *charset, Boolean saving)
{
//
// Loop through the body parts
//
   bodyText->ResetPartIndex();

   StringC	textStr;
   StringC	headStr;
   Boolean	done = False;
   TextTypeT	ttype = (textType == CT_PLAIN ? TT_PLAIN : TT_ENRICHED);
   int		wrapPos = 0;
   if ( ttype == TT_PLAIN ) {
      if ( XmToggleButtonGetState(optWrapTB) )
	 wrapPos = bodyText->ColumnCount();
   }
   else {
      if ( XmToggleButtonGetState(optWrapTB) ) wrapPos = 72;
   }

   while ( !done ) {

//
// Look for the next part
//
      textStr.Clear();
      RichGraphicC *rg;
      RichCmdTypeT curType = bodyText->GetNextPart(textStr, ttype, &rg,wrapPos);

//
// Add a boundary before this part if we will be adding the part
//
      if ( (curType == RC_GRAPHIC && rg) ||
	   (curType == RC_TEXT    && textStr.size() > 0) ) {
	 headStr  = "\n--";
	 headStr += bound;
	 headStr += "\n";
	 if ( !headStr.WriteFile(fp) ) return False;
      }

//
// Process the part
//
      if ( curType == RC_GRAPHIC ) {
	 SendIconC	*icon = (SendIconC*)rg;
	 if      ( !icon            ) done = True;
	 else if ( !icon->Write(fp) ) return False;
      }

      else if ( textStr.size() > 0 ) {

//
// Write this text part
//
	 if ( !WriteTextPart(fp, textType, textStr, charset, NULL) )
	    return False;

      } // End if we got a text part

   } // End while not done

#if 0
//
// End last part with a blank line
//
   while ( !bodyStr.EndsWith("\n\n") ) bodyStr += '\n';
#endif

   return True;

} // End WriteMultipartBody

/*---------------------------------------------------------------
 *  Method to write out a text part.  We may need an alternative.
 */

Boolean
SendWinP::WriteTextPart(FILE *fp, MimeContentType textType, StringC& textStr,
			CharC *charset, StringListC *contList)
{
   StringC	headStr;
   StringC	bodyStr;
   Boolean	error = False;

//
// Add the content-type header.  Add the character set parameter if necessary
//
   headStr = "Content-Type: text/";
   if ( textType == CT_PLAIN ) headStr += "plain";
   else			       headStr += "enriched";

   if ( charset ) {
      headStr += "; charset=\"";
      headStr += *charset;
      headStr += '"';
   }
   headStr += '\n';

   if ( contList ) error = !AddHeader(headStr, *contList);
   else		   error = !headStr.WriteFile(fp);
   if ( error ) return False;

//
// See if we need to add a CTE header
//
   Boolean	has8Bit = (textStr.size() > 0 && Contains8Bit(textStr));
   if ( has8Bit ) {

      headStr = "Content-Transfer-Encoding: ";
      if ( ishApp->mailPrefs->bodyEncType == ET_QP )
	 headStr += "quoted-printable\n";
      else
	 headStr += "8bit\n";

      if ( contList ) error = !AddHeader(headStr, *contList);
      else	      error = !headStr.WriteFile(fp);
      if ( error ) return False;

   } // End if 8-bit data is present

//
// Write a blank line
//
   if ( !contList ) {
      headStr = "\n";
      if ( !headStr.WriteFile(fp) ) return False;
   }

//
// Write the text
//
   if ( has8Bit && ishApp->mailPrefs->bodyEncType == ET_QP )
      error = !TextToFileQP(textStr, "mailfile", fp);
   else
      error = !textStr.WriteFile(fp);

   return !error;

} // End WriteTextPart

/*---------------------------------------------------------------
 *  Method to deliver message
 */

Boolean
SendWinP::SendMessage(char *bodyFile, StringListC& headList,
		      OutgoingMailTypeT mailType)
{
   struct stat	bodyStats;
   if ( stat(bodyFile, &bodyStats) != 0 ) {
      StringC	errmsg("Could not read file \"");
      errmsg += bodyFile;
      errmsg += "\".\n";
      errmsg += SystemErrorMessage(errno);
      errmsg += "\nMessage will not be sent.";
      pub->PopupMessage(errmsg);
      return False;
   }

//
// Get size of message
//
   int	msgSize = (int)bodyStats.st_size;

   u_int	count = headList.size();
   for (int i=0; i<count; i++) {
      StringC	*headStr = headList[i];
      msgSize += headStr->size();
   }
   msgSize++;	// Blank between headers and body

//
// See if message is to be split
//
   Boolean	sent = False;
   if ( ishApp->mailPrefs->split && msgSize > ishApp->mailPrefs->splitSize ) {

//
// Make sure they want to split plain text messages
//
      if ( mailType == MAIL_PLAIN && !OkToSplitPlain(&mailType) )
	 return False;

//
// Only split if the mail type is not plain
//
      if ( mailType != MAIL_PLAIN )
	 return SendSplitMessage(bodyFile, headList);

   } // End if message needs to be split

//
// Mail the files as-is
//
   if ( !sent ) {

//
// Write the message headers to a file
//
      char      *cs = tempnam(NULL, "head.");
      StringC	headFile(cs);
      free(cs);

      if ( !WriteHeaders(headList, headFile, /*addBlank=*/True) ) {
	 unlink(headFile);
	 return False;
      }

//
// Mail the header and body files
//
      sent = MailFiles(headFile, bodyFile, recipientList,
		       ishApp->mailPrefs->SendmailCmd(), /*wait=*/False,
		       /*deleteHead=*/True, /*deleteBody=*/True);

   } // End if need to send message

   return sent;

} // End SendMessage

/*---------------------------------------------------------------
 *  Method to ask user if a large plaintext message should be split
 *     into MIME partials.
 */

Boolean
SendWinP::OkToSplitPlain(OutgoingMailTypeT *mailType)
{
   static QueryAnswerT	answer;
   static Widget	dialog = NULL;

//
// Create the dialog if necessary
//
   if ( !dialog ) {

      pub->BusyCursor(True);

      WArgList	args;
      args.DialogStyle(XmDIALOG_FULL_APPLICATION_MODAL);
      if ( halApp->questionPM ) args.SymbolPixmap(halApp->questionPM);
      dialog = XmCreateQuestionDialog(*pub, "confirmSplitWin", ARGS);

      XtAddCallback(dialog, XmNokCallback, (XtCallbackProc)AnswerQuery,
		    (XtPointer)&answer);
      XtAddCallback(dialog, XmNcancelCallback,
		    (XtCallbackProc)AnswerQuery, (XtPointer)&answer);
      XtUnmanageChild(XmMessageBoxGetChild(dialog, XmDIALOG_HELP_BUTTON));

      Widget mimePB = XmCreatePushButton(dialog, "mimePB", 0,0);
      XtManageChild(mimePB);

      XtAddCallback(mimePB, XmNactivateCallback, (XtCallbackProc)AnswerQuery,
		    (XtPointer)&answer);

//
// Trap window manager close function
//
      XmAddWMProtocolCallback(XtParent(dialog), halApp->delWinAtom,
			      (XtCallbackProc)WmClose,
			      (caddr_t)&answer);

      pub->BusyCursor(False);

   } // End if query dialog not created

//
// Show the dialog
//
   PopupOver(dialog, *pub);

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

   if      ( answer == QUERY_YES ) *mailType = MAIL_PLAIN;
   else if ( answer == QUERY_NO  ) *mailType = MAIL_MIME;
   else				   return False;

   return True;

} // End OkToSplitPlain

/*---------------------------------------------------------------
 *  Method to deliver message in several parts
 */

Boolean
SendWinP::SendSplitMessage(char *bodyFile, StringListC headList)
{
//
// Open the body file
//
   FILE	*bfp = fopen(bodyFile, "r");
   if ( !bfp ) {
      StringC	errmsg("Could not open file \"");
      errmsg += bodyFile;
      errmsg += "\".\n";
      errmsg += SystemErrorMessage(errno);
      errmsg += "\nMessage will not be sent.";
      pub->PopupMessage(errmsg);
      return False;
   }

//
// Remove any "Content-*", "Encrypted", "Message-ID" and "MIME-Version"
//    headers from the original header list.  We'll put them in the first
//    partial message along with the subject and a new message id.
//
   StringC	subHead;
   StringListC	internalList;
   u_int	count = headList.size();
   int	i;
   for (i=count-1; i>=0; i--) {

      StringC	*str = headList[i];

      if ( str->StartsWith("Content-",      IGNORE_CASE) ||
	   str->StartsWith("Encrypted:",    IGNORE_CASE) ||
	   str->StartsWith("Message-Id:",   IGNORE_CASE) ||
	   str->StartsWith("Subject:",      IGNORE_CASE) ||
	   str->StartsWith("Mime-Version:", IGNORE_CASE) ) {

	 if ( str->StartsWith("Subject:", IGNORE_CASE) )
	    subHead = *str;

	 internalList.prepend(*str);
	 headList.remove(i);
      }

   } // End for each header

//
// Add a MIME-Version header to the main list.
//
   StringC	headStr;
   headStr = "MIME-Version: 1.0\n";
   if ( !AddHeader(headStr, headList) ) return False;

//
// Add subject and message-id headers to the internal list
//
   if ( subHead.size() > 0 ) internalList.add(subHead);
   if ( subHead.EndsWith('\n') ) subHead.CutEnd(1);

   StringC	idStr;
   GenId(idStr);
   headStr = "Message-Id: <";
   headStr += idStr;
   headStr += ">";
   if ( !AddHeader(headStr, internalList) ) return False;

//
// Create a base content-type header
//
   StringC	baseCT("Content-Type: message/partial; id=\"");
   baseCT += idStr;
   baseCT += "\"; number=";

//
// Get the sizes of the various message parts.
//
   int		headSize = 0;
   count = headList.size();
   for (i=0; i<count; i++) {
      StringC	*str = headList[i];
      headSize += str->size();
   }

   int		internalSize = 0;
   count = internalList.size();
   for (i=0; i<count; i++) {
      StringC	*str = internalList[i];
      internalSize += str->size();
   }

   fseek(bfp, 0, SEEK_END);
   int	bodySize = (int)ftell(bfp);
   fseek(bfp, 0, SEEK_SET);

   int	partHeadSize = headSize + baseCT.size() +
		       16/*number, total*/ + 1/*blank line*/;
   int	partSize = ishApp->mailPrefs->splitSize;
   if ( partSize < partHeadSize*2 ) partSize = partHeadSize*2;
   int	partBodySize = partSize - partHeadSize;
   int	effBodySize  = bodySize + internalSize + 1/*blank line*/;
   int	partCount    = effBodySize / partBodySize;
   if ( (effBodySize % partBodySize) != 0 ) partCount++;

//
// Loop until full message is sent out
//
   Boolean	error = False;
   CharC	nlStr("\n");
   StringC	partFile;
   StringC	statmsg;
   for (int partNum=0; !error && partNum<partCount; partNum++) {

//
// See where we stand
//
      Boolean	firstPart = (partNum == 0);
      Boolean	lastPart  = (partNum == partCount-1);

//
// Open temporary files for storing message
//
      char	*cs = tempnam(NULL, "part.");
      partFile = cs;
      free(cs);

      statmsg = "Sending part ";
      statmsg += (partNum+1);
      statmsg += " of ";
      statmsg += partCount;
      if ( !lastPart ) statmsg += " ...";
      pub->Message(statmsg);

//
// Create the part file
//
      FILE	*pfp = fopen(partFile, "a");
      if ( !pfp ) {
	 error = True;
	 continue;
      }

      int	availSpace = partSize;

//
// Add temporary subject header
//
      headStr = subHead;
      headStr += " (part ";
      headStr += (partNum+1);
      headStr += " of ";
      headStr += partCount;
      headStr += ")\n";
      error =  !AddHeader(headStr, headList);
      if ( error ) continue;

      availSpace -= headStr.size();

//
// Finalize content-type header.
//
      headStr = baseCT;
      headStr += (partNum+1);
      headStr += "; total=";
      headStr += partCount;
      headStr += "\n";
      error =  !AddHeader(headStr, headList);
      if ( error ) continue;

      availSpace -= headStr.size();
      availSpace--; /*blank line*/

//
// Write out the headers
//
      error = !WriteHeaders(headList, partFile, /*addBlank=*/True, pfp);
      if ( error ) continue;

//
// If this is the first part, write out the internal headers
//
      if ( firstPart ) {
	 error = !WriteHeaders(internalList, partFile, /*addBlank=*/True, pfp);
	 if ( error ) continue;
	 availSpace -= (internalSize + 1/*blank line*/);
      }

//
// Write out the next portion of the body.  If this is the last part,
//    write everything remaining.  Otherwise, write as much as we can
//
      if ( lastPart ) availSpace = bodySize - (int)ftell(bfp);
      error = !CopyFilePart(bfp, pfp, availSpace);

//
// Close the file for this part
//
      fclose(pfp);

      if ( error ) {
	 StringC errmsg("Could not write to temp file ");
	 errmsg += partFile;
	 errmsg += "\".\n";
	 errmsg += SystemErrorMessage(errno);
	 errmsg += "\nMessage will not be sent.";
	 pub->PopupMessage(errmsg);

	 if ( partNum > 0 ) pub->Message("Not all parts were sent.");
	 else		    pub->Message("Message NOT sent");

	 unlink(partFile);
	 return False;
      }

//
// Send this file
//
#if 0
      statmsg = "Sending part ";
      statmsg += (partNum+1);
      statmsg += " of ";
      statmsg += (int)partCount;
      if ( !lastPart ) statmsg += " ...";
      pub->Message(statmsg);
#endif

      if ( !MailFile(partFile, recipientList, ishApp->mailPrefs->SendmailCmd(),
		     /*wait=*/True, /*deleteFile=*/True) ) {

	 if ( partNum > 0 ) pub->Message("Not all parts were sent.");
	 else		    pub->Message("Message NOT sent");

	 unlink(partFile);
	 return False;
      }

   } // End for each file to be sent

#if 0
   pub->Message("Message sent.");
#endif
   return True;

} // End SendSplitMessage

/*---------------------------------------------------------------
 *  Method to copy the specified number of bytes from one file to
 *     another.  The files must already be open
 */

Boolean
SendWinP::CopyFilePart(FILE *ifp, FILE *ofp, u_int length)
{
   StringC	lineStr;
   CharC	nlStr("\n");
   Boolean	error     = False;
   Boolean	firstLine = True;
   while ( !error && length > 0 && lineStr.GetLine(ifp) != EOF ) {

//
// If we're under the limit or no lines have been written, copy this line out
//
      u_int	size = lineStr.size() + 1/*nl*/;
      if ( firstLine || size <= length ) {
	 error = (!lineStr.WriteFile(ofp) || !nlStr.WriteFile(ofp));
	 firstLine = False;
	 if ( size <= length ) length -= size;
	 else		       length = 0;
      }

//
// If we're over the limit, reset to the beginning of the line
//
      else {
	 length = 0;
	 fseek(ifp, -size, SEEK_CUR);
      }

      lineStr.Clear();

   } // End for each line in input (up to length bytes)

   return !error;

} // End CopyFilePart

/*---------------------------------------------------------------
 *  Callback to check validity of addresses
 */

void
SendWinP::DoCheckNow(Widget, SendWinP *This, XtPointer)
{
   if ( !This->ProcessHeaderFields() || This->recipientList.size() == 0 )
      return;

   This->CheckAddresses(True/*report success*/);

} // End DoCheckNow

/*---------------------------------------------------------------
 *  Callback to check validity of addresses in the recipient list
 */

Boolean
SendWinP::CheckAddresses(Boolean reportSuccess)
{
   if ( recipientList.size() == 0 ) return True;

   pub->BusyCursor(True);

//
// Use sendmail -bv to check the addresses
//
   StringC	cmdStr = ishApp->mailPrefs->SendmailCmd();
   cmdStr += " -bv ";

   u_int	count = recipientList.size();
   for (u_int i=0; i<count; i++) {
      cmdStr += "'";
      cmdStr += *recipientList[i];
      cmdStr += "' ";
   }

   if ( debuglev > 0 ) cout <<cmdStr <<endl;

//
// Run it in a separate process
//
   checkDone   = False;
   checkStatus = 0;
   CallbackC	doneCb((CallbackFn*)CheckDone, this);
   checkPid = ForkIt(cmdStr, &doneCb);
   if ( checkPid < 0 ) {
      StringC errStr("Could not fork process to check addresses:\n");
      errStr += SystemErrorMessage(-(int)checkPid);
      pub->PopupMessage(errStr);
      pub->BusyCursor(False);
      return False;
   }

//
// Wait for process to finish
//
   WorkingBoxC	*workBox = halApp->WorkBox();
   workBox->Message("Verifying addresses.\nThis may take several minutes.");
   workBox->HideScale();
   workBox->Show(*pub);
   Boolean	cancelled = False;
   while ( !checkDone && !(cancelled=workBox->Cancelled()) );
   workBox->Hide();

   pub->BusyCursor(False);

   if ( cancelled ) 
      kill(-checkPid, SIGKILL);

   else if ( (checkStatus != 0 || reportSuccess) && checkOutput.size() > 0 ) {
      pub->PopupMessage(checkOutput, XmDIALOG_INFORMATION);
      if ( checkStatus != 0 ) return False;
   }

   return True;

} // End CheckAddresses

/*---------------------------------------------------------------
 *  Callback to handle completion of address check
 */

void
SendWinP::CheckDone(int status, SendWinP *This)
{
   This->checkDone   = True;
   This->checkOutput = ForkOutput(This->checkPid);
   This->checkStatus = status;
}

/*---------------------------------------------------------------
 *  Save a copy of a message to a folder
 */

Boolean
SendWinP::PerformFcc(StringListC& headList, char *bodyFile, CharC fccFile)
{
   StringC	path;

//
// Check for a full pathname or an IMAP folder
//
   if ( fccFile.StartsWith('/')   ||
	fccFile.StartsWith("./")  ||
	fccFile.StartsWith("../") ||
	IsImapName(fccFile) )
      path = fccFile;

//
// Check for an abbreviated folder name
//
   else if ( fccFile.StartsWith('+') ||
	     fccFile.StartsWith('=') ) {
      path = fccFile;
      ishApp->ExpandFolderName(path);
   }

//
// Add the fcc directory name if necessary
//
   else if ( ishApp->mailPrefs->fccType != FCC_NONE &&
	     ishApp->mailPrefs->fccType != FCC_TO_FOLDER ) {
      path = ishApp->mailPrefs->FccFolderDir();
      if ( !path.EndsWith("/") ) path += '/';
      path += fccFile;
   }

   else
      path = fccFile;

//
// Create or open the folder and add the message
//
   FolderC *folder = ishApp->folderPrefs->GetFolder(path, /*createOk=*/True);
   if ( folder ) {
      if ( !folder->AddMessage(headList, bodyFile) )
         return False;
      ishApp->appPrefs->AddRecentFolder(path);
   }
   
   return True;

} // End PerformFcc

/*---------------------------------------------------------------
 *  Method to pass the body to encryptor for signing and/or encoding
 */

Boolean
SendWinP::WriteCryptBody(char *bodyFile, StringC& cryptFile,
			 StringListC& contList, OutgoingMailTypeT mailType,
			 Boolean sign, Boolean hide)
{
   StringC	cmd;
   int		status;

//
// See what type of message will be sent
//
   if ( mailType == MAIL_PLAIN ) {

      char	*cs = tempnam(NULL, "cryp.");
      cryptFile = cs;
      free(cs);

      if ( hide && sign ) cmd = ishApp->compPrefs->encryptSignCmd;
      else if ( hide )    cmd = ishApp->compPrefs->encryptCmd;
      else		  cmd = ishApp->compPrefs->digSignCmd;

      cmd.Replace("%i", bodyFile);
      cmd.Replace("%o", cryptFile);
      cmd.Replace("%u", ishApp->mailPrefs->fromHeader);

      if ( hide ) {
	 u_int	count = recipientList.size();
         StringC buf;
         for (int i=0; i<count; i++) {
            buf += ' '; buf += *recipientList[i];
         }
         cmd.Replace("%r", buf);
      }

      XtSetSensitive(*pub, False);

      status = SafeSystem(cmd);
      XtSetSensitive(*pub, True);
      if ( status != 0 ) {
         unlink(cryptFile);
         return False;
      }

   } // End if sending plain text

   else {	// Sending MIME

      StringC	partFile;
      Boolean	signOnly = sign && !hide;
      char	*cs;
      if ( signOnly ) {

//
// Create a file containing the content type headers and the body
//
	 cs = tempnam(NULL, "part.");
	 partFile = cs;
	 free(cs);

//
// Add headers to file
//
	 if ( !WriteHeaders(contList, partFile, /*addBlank=*/True) ) {
	    unlink(partFile);
	    return False;
	 }

//
// Copy body to file
//
	 if ( !CopyFile(bodyFile, partFile) ) {
	    unlink(partFile);
	    return False;
	 }

      } // End if signing only

      else
	 partFile = bodyFile;

//
// Create yet another body file to hold the whole multipart
//
      cs = tempnam(NULL, "cryp.");
      cryptFile = cs;
      free(cs);

      StringC	headFile = cryptFile;
      headFile += ".head";

//
// Get unique boundary
//
      StringC	bound;
      GenBoundary(bound);

//
// Generate MIME security part
//
      if ( hide && sign ) cmd = ishApp->compPrefs->mimeEncryptSignCmd;
      else if ( hide )    cmd = ishApp->compPrefs->mimeEncryptCmd;
      else		  cmd = ishApp->compPrefs->mimeDigSignCmd;

      cmd.Replace("%i", partFile);
      cmd.Replace("%o", cryptFile);
      cmd.Replace("%u", ishApp->mailPrefs->fromHeader);
      cmd.Replace("%b", bound);

      if ( hide ) {
	 u_int	count = recipientList.size();
         StringC buf;
         for (int i=0; i<count; i++) {
            buf += ' '; buf += *recipientList[i];
         }
         cmd.Replace("%r", buf);
      }

      status = SafeSystem(cmd);
      XtSetSensitive(*pub, True);
      if ( status != 0 ) {
	 if ( signOnly ) unlink(partFile);
	 unlink(cryptFile);
	 unlink(headFile);
	 return False;
      }

//
// Open header file
//
      FILE	*fp = fopen(headFile, "r");
      if ( !fp && errno != ENOENT ) {
	 StringC	errmsg("Could not open file \"");
	 errmsg += headFile;
	 errmsg += "\".\n";
	 errmsg += SystemErrorMessage(errno);
	 errmsg += "\nMessage will not be sent.";
	 pub->PopupMessage(errmsg);

	 if ( signOnly ) unlink(partFile);
	 unlink(cryptFile);
	 unlink(headFile);
	 return False;
      }

//
// Read the headers
//
      StringC	headStr;
      if ( fp ) {

	 if ( !headStr.ReadFile(fp) ) {

	    StringC	errmsg("Could not read file \"");
	    errmsg += headFile;
	    errmsg += "\".\n";
	    errmsg += SystemErrorMessage(errno);
	    errmsg += "\nMessage will not be sent.";
	    pub->PopupMessage(errmsg);

	    if ( signOnly ) unlink(partFile);
	    unlink(cryptFile);
	    unlink(headFile);
	    return False;
	 }

	 fclose(fp);

	 HeaderC	*head = ExtractHeaders(headStr);
	 while ( head ) {

	    if ( !AddHeader(head->full, contList) ) {
	       if ( signOnly ) unlink(partFile);
	       unlink(headFile);
	       unlink(cryptFile);
	       return False;
	    }

	    head = head->next;
	 }

      } // End if header file opened

      if ( signOnly ) unlink(partFile);
      unlink(headFile);

   } // End if MIME

   return True;

} // End WriteCryptBody
