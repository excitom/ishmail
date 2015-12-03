/*
 * $Id: SendWinC.C,v 1.5 2001/07/28 18:26:03 evgeny Exp $
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
#include "CompPrefC.h"
#include "ReplyPrefC.h"
#include "MailPrefC.h"
#include "SigPrefC.h"
#include "MimeTypes.h"
#include "AddressC.h"
#include "MsgC.h"
#include "HeaderValC.h"
#include "Misc.h"
#include "SendIconC.h"
#include "MsgPartC.h"
#include "FileMsgC.h"
#include "FileMisc.h"
#include "ConfPrefC.h"
#include "HeaderC.h"
#ifdef HGLTERM
#include "TermWinC.h"
#endif

#include <hgl/MimeRichTextC.h>
#include <hgl/SysErr.h>

#include <Xm/ToggleB.h>

#include <unistd.h>
#include <errno.h>
#include <signal.h>

/*---------------------------------------------------------------
 *  Composition window public constructor
 */

SendWinC::SendWinC(const char *name, Widget parent, SendWinTypeT winType)
: HalTopLevelC(name, parent)
{
   type = winType;
   ccVis            = ishApp->compPrefs->showCc;
   bccVis           = ishApp->compPrefs->showBcc;
   fccVis           = ishApp->compPrefs->showFcc;
   otherVis         = ishApp->compPrefs->showOther;
   maxFieldsPerLine = ishApp->compPrefs->maxFieldsPerLine;
   changed          = False;

   priv = new SendWinP(this);

   priv->BuildMenus();
   priv->BuildWidgets();

} // End constructor

/*---------------------------------------------------------------
 *  Composition window public destructor
 */

SendWinC::~SendWinC()
{
   delete priv;
}

/*---------------------------------------------------------------
 *  Method to update visibility of header fields
 */

void
SendWinC::UpdateVisibleFields()
{
   Boolean	change = False;

   if ( ccVis != ishApp->compPrefs->showCc ) {
      ccVis = ishApp->compPrefs->showCc;
      XmToggleButtonSetState(priv->optCcTB, ishApp->compPrefs->showCc, False);
      change = True;
   }

   if ( bccVis != ishApp->compPrefs->showBcc ) {
      bccVis = ishApp->compPrefs->showBcc;
      XmToggleButtonSetState(priv->optBccTB, ishApp->compPrefs->showBcc, False);
      change = True;
   }

   if ( fccVis != ishApp->compPrefs->showFcc ) {
      fccVis = ishApp->compPrefs->showFcc;
      XmToggleButtonSetState(priv->optFccTB, ishApp->compPrefs->showFcc, False);
      change = True;
   }

   if ( otherVis != ishApp->compPrefs->showOther ) {
      otherVis = ishApp->compPrefs->showOther;
      XmToggleButtonSetState(priv->optOtherTB, ishApp->compPrefs->showOther,
      			     False);
      change = True;
   }

   if ( change ) priv->PlaceHeaderFields();

} // End UpdateVisibleFields

/*---------------------------------------------------------------
 *  Method to change key editing behavior
 */

void
SendWinC::SetKeys(Boolean emacs, Boolean delMeansBs)
{
   if ( emacs != ishApp->compPrefs->emacsMode ) {

      priv->bodyText->SetEmacsMode(emacs);
      if ( priv->headText ) priv->headText->SetEmacsMode(emacs);

      priv->oldHeadPane->toText->SetEmacsMode(emacs);
      priv->oldHeadPane->subText->SetEmacsMode(emacs);
      priv->oldHeadPane->ccText->SetEmacsMode(emacs);
      priv->oldHeadPane->bccText->SetEmacsMode(emacs);
      priv->oldHeadPane->fccText->SetEmacsMode(emacs);
      priv->oldHeadPane->otherText->SetEmacsMode(emacs);

      priv->newHeadPane->toText->SetEmacsMode(emacs);
      priv->newHeadPane->subText->SetEmacsMode(emacs);
      priv->newHeadPane->ccText->SetEmacsMode(emacs);
      priv->newHeadPane->bccText->SetEmacsMode(emacs);
      priv->newHeadPane->fccText->SetEmacsMode(emacs);
      priv->newHeadPane->otherText->SetEmacsMode(emacs);
   }

   if ( delMeansBs != ishApp->compPrefs->delMeansBs ) {

      priv->bodyText->SetDeleteLikeBackspace(delMeansBs);
      if ( priv->headText ) priv->headText->SetDeleteLikeBackspace(delMeansBs);

      priv->oldHeadPane->toText->SetDeleteLikeBackspace(delMeansBs);
      priv->oldHeadPane->subText->SetDeleteLikeBackspace(delMeansBs);
      priv->oldHeadPane->ccText->SetDeleteLikeBackspace(delMeansBs);
      priv->oldHeadPane->bccText->SetDeleteLikeBackspace(delMeansBs);
      priv->oldHeadPane->fccText->SetDeleteLikeBackspace(delMeansBs);
      priv->oldHeadPane->otherText->SetDeleteLikeBackspace(delMeansBs);

      priv->newHeadPane->toText->SetDeleteLikeBackspace(delMeansBs);
      priv->newHeadPane->subText->SetDeleteLikeBackspace(delMeansBs);
      priv->newHeadPane->ccText->SetDeleteLikeBackspace(delMeansBs);
      priv->newHeadPane->bccText->SetDeleteLikeBackspace(delMeansBs);
      priv->newHeadPane->fccText->SetDeleteLikeBackspace(delMeansBs);
      priv->newHeadPane->otherText->SetDeleteLikeBackspace(delMeansBs);
   }

//
// Update all edit windows
//
   u_int	count = priv->editWinList.size();
   int i=0; for (i=0; i<count; i++) {
      SendWinC	*editWin = (SendWinC *)*priv->editWinList[i];
      editWin->SetKeys(emacs, delMeansBs);
   }

} // End SetKeys

/*---------------------------------------------------------------
 *  Methods to return sizes of text
 */

int SendWinC::BodyRowCount() { return priv->bodyText->RowCount(); }
int SendWinC::HeadRowCount() { return priv->toText->RowCount(); }
int SendWinC::ColumnCount()  { return priv->bodyText->ColumnCount(); }

/*---------------------------------------------------------------
 *  Method to change the number of visible rows and columns
 */

void
SendWinC::SetSize(int rows, int cols)
{
   BusyCursor(True);

   priv->bodyText->SetSize(rows, cols, *this);

//
// Update all edit windows
//
   u_int	count = priv->editWinList.size();
   int i=0; for (i=0; i<count; i++) {
      SendWinC	*editWin = (SendWinC *)*priv->editWinList[i];
      editWin->SetSize(rows, cols);
   }

   BusyCursor(False);

} // End SetSize

/*---------------------------------------------------------------
 *  Routine to position the header fields
 */

void
SendWinC::PlaceHeaderFields()
{
   priv->PlaceHeaderFields();
}

/*---------------------------------------------------------------
 *  Method to return whether line wrapping is on
 */

Boolean
SendWinC::Wrapping()
{
   return XmToggleButtonGetState(priv->optWrapTB);
}

/*---------------------------------------------------------------
 *  Method to change wrapping
 */

void
SendWinC::SetWrap(Boolean val)
{
   BusyCursor(True);

   priv->bodyText->ResizeWidth(!val);

   if ( priv->optWrapTB )
      XmToggleButtonSetState(priv->optWrapTB, val, True);

//
// Update all edit windows
//
   u_int	count = priv->editWinList.size();
   int i=0; for (i=0; i<count; i++) {
      SendWinC	*editWin = (SendWinC *)*priv->editWinList[i];
      editWin->SetWrap(val);
   }

   BusyCursor(False);

} // End SetWrap

/*---------------------------------------------------------------
 *  Method to set address checking
 */

void
SendWinC::SetAddressChecking(Boolean val)
{
   XmToggleButtonSetState(priv->optCheckAddrTB, val, True);
}

/*---------------------------------------------------------------
 *  Method to query address checking
 */

Boolean
SendWinC::CheckingAddresses()
{
   return XmToggleButtonGetState(priv->optCheckAddrTB);
}

/*---------------------------------------------------------------
 *  Method to set the outgoing mail type
 */

void
SendWinC::SetMailType(OutgoingMailTypeT type)
{
   switch (type) {
      case MAIL_PLAIN:
	 XmToggleButtonSetState(priv->optMsgPlainTB, True, True);
	 break;
      case MAIL_MIME:
	 XmToggleButtonSetState(priv->optMsgMimeTB, True, True);
	 break;
      case MAIL_ALT:
	 XmToggleButtonSetState(priv->optMsgAltTB, True, True);
	 break;
   }
}

/*---------------------------------------------------------------
 *  Method to query the outgoing mail type
 */

OutgoingMailTypeT
SendWinC::MailType()
{
   if ( XmToggleButtonGetState(priv->optMimeTB) ) {
      if ( XmToggleButtonGetState(priv->optMsgAltTB) )
	 return MAIL_ALT;
      else
	 return MAIL_MIME;
   }
   else
      return MAIL_PLAIN;
}

/*---------------------------------------------------------------
 *  Method to set the outgoing text type
 */

void
SendWinC::SetTextType(MimeContentType type)
{
   if ( IsPlain(type) )
      XmToggleButtonSetState(priv->optTextPlainTB, True, True);
   else if ( IsEnriched(type) )
      XmToggleButtonSetState(priv->optTextRichTB, True, True);
}

/*---------------------------------------------------------------
 *  Method to query the outgoing text type
 */

MimeContentType
SendWinC::TextType()
{
   if ( XmToggleButtonGetState(priv->optTextPlainTB) )
      return CT_PLAIN;
   else if ( XmToggleButtonGetState(priv->optTextRichTB) )
      return CT_ENRICHED;
   else
      return CT_ALTERNATIVE;
}

/*---------------------------------------------------------------
 *  Routine to initialize the contents of the Fcc field
 */

void
SendWinC::UpdateFcc()
{
   priv->UpdateFcc();
}

/*---------------------------------------------------------------
 *  Method to close the window.  The "save" parameter is only used in edit
 *     windows.
 */

Boolean
SendWinC::Close(Boolean save)
{
   if ( !shown ) return True;

   ClearMessage();

//
// If this is an edit window, see if we need to save
//
   if ( IsEditOnly() ) {

      if ( save && Changed() ) {
	 if ( !priv->Save() ) return False;
      }

      if ( priv->editIcon ) {
	 priv->editIcon->AnimationOff();
	 priv->editIcon->Unhighlight();
      }

      priv->editIcon    = NULL;
      priv->editMsg     = NULL;
      priv->editMsgText = False;
      priv->UpdateEditButtons();
   }

//
// If this is not an edit window, see if there are un-sent changes.  If so,
//    confirm closing the window.
//
   else if ( !priv->OkToClose() )
      return False;

//
// Remove any auto-save timer
//
   if ( priv->autoSaveTimer ) XtRemoveTimeOut(priv->autoSaveTimer);
   priv->autoSaveTimer = (XtIntervalId)NULL;

//
// If we're closing with changes, save the composition to the dead letter file
//
   if ( !IsEditOnly() && ishApp->mailPrefs->saveOnInterrupt && Changed() )
      priv->Save(ishApp->mailPrefs->DeadFile());

//
// Hide any open edit windows.  We don't save changes because we've either
//    already done it or we don't want to.
//
   u_int	count = priv->editWinList.size();
   int i=0; for (i=0; i<count; i++) {
      SendWinC	*editWin = (SendWinC *)*priv->editWinList[i];
      if ( editWin->IsShown() ) editWin->Close();
   }

#ifdef HGLTERM
//
// Hide any open terminal windows.
//
   count = priv->termWinList.size();
   for (i=0; i<count; i++) {
      TermWinC	*termWin = (TermWinC *)*priv->termWinList[i];
      if ( termWin->IsShown() ) termWin->Hide();
   }
#endif

//
// If there is an edit in progress, kill it
//
   if ( priv->edit_pid > 0 ) {
      if ( debuglev > 0 ) cout <<"Killing process " <<priv->edit_pid NL;
      if ( kill(-priv->edit_pid, SIGKILL) != 0 ) {
	 int	err = errno;
	 StringC	errmsg = "Could not kill edit process: ";
	 errmsg += SystemErrorMessage(err);
	 PopupMessage(errmsg);
      }
      priv->edit_pid = 0;
   }

//
// If there is a spell-check in progress, kill it
//
   if ( priv->spell_pid > 0 ) {
      if ( debuglev > 0 ) cout <<"Killing process " <<priv->spell_pid NL;
      if ( kill(-priv->spell_pid, SIGKILL) != 0 ) {
	 int	err = errno;
	 StringC	errmsg = "Could not kill spell-check process: ";
	 errmsg += SystemErrorMessage(err);
	 PopupMessage(errmsg);
      }
      priv->spell_pid = 0;
   }

   priv->bodyText->Clear();
   priv->bodyText->SetTextType(TT_ENRICHED);

//
// Remove any resend files
//
   count = priv->resendFileList.size();
   for (i=0; i<count; i++) {
      StringC	*file = priv->resendFileList[i];
      unlink(*file);
   }
   priv->resendFileList.removeAll();

   priv->SetResendMode(False);
   if ( type == SEND_RESEND || type == SEND_FORWARD || type == SEND_REPLY )
      type = SEND_NEW;

   priv->msgList.removeAll();

//
// Remove any auto-save files
//
   if ( priv->autoSaveFile.size() > 0 )
      unlink(priv->autoSaveFile);

   changed = False;

//
// Close this window
//
   DoHide(NULL, this, NULL);
   return True;

} // End Close

/*---------------------------------------------------------------
 *  Method to set state of addSigTB
 */

void
SendWinC::AddSig(Boolean val)
{
   XmToggleButtonSetState(priv->optAddSigTB, val, False);
}

/*---------------------------------------------------------------
 *  Method to query state of addSigTB
 */

Boolean
SendWinC::AddingSig()
{
   return XmToggleButtonGetState(priv->optAddSigTB);
}

/*---------------------------------------------------------------
 *  Method to set body text from a character array
 */

void
SendWinC::SetBody(CharC c)
{
   priv->bodyText->SetTextType(TT_PLAIN);
   priv->bodyText->SetString(c);
}

/*---------------------------------------------------------------
 *  Methods to set Cc text
 */

void
SendWinC::SetCc(const char *cs)
{
   if ( IsEditOnly() ) return;

   AddressC	addr(cs);
   priv->SetField(priv->ccText, &addr);
}

void
SendWinC::SetCc(StringListC& ccList)
{
   if ( IsEditOnly() ) return;

//
// Loop through the cc entries
//
   StringC	str;
   u_int	count = ccList.size();
   int i=0; for (i=0; i<count; i++) {
      if ( str.size() > 0 ) str += ", ";
      str += *ccList[i];
   }

   AddressC	addr(str);
   priv->SetField(priv->ccText, &addr);
}

/*---------------------------------------------------------------
 *  Method to set Bcc text
 */

void
SendWinC::SetBcc(const char *cs)
{
   if ( IsEditOnly() ) return;

   AddressC	addr(cs);
   priv->SetField(priv->bccText, &addr);
}

/*---------------------------------------------------------------
 *  Method to set Subject text
 */

void
SendWinC::SetSubject(const char *cs)
{
   if ( IsEditOnly() ) return;

   priv->subText->SetString(cs);
}

/*---------------------------------------------------------------
 *  Method to set Other text
 */

void
SendWinC::SetOther(const char *cs)
{
   if ( IsEditOnly() ) return;

   StringC	otherStr = ishApp->mailPrefs->otherHeaders;
   if ( otherStr.size() > 0 && otherStr.LastChar() != '\n' && *cs != '\n' )
      otherStr += "\n";
   otherStr += cs;
   priv->otherText->SetString(otherStr);

   if ( cs && strlen(cs)>0 )
      XmToggleButtonSetState(priv->optOtherTB, True, True);
   else if ( !ishApp->compPrefs->showCc )
      XmToggleButtonSetState(priv->optOtherTB, False, True);
}

/*-----------------------------------------------------------------------
 *  Return pointer to button manager class
 */

ButtonMgrC*
SendWinC::ButtonMgr()
{
   return priv->buttMgr;
}

/*---------------------------------------------------------------
 *  Message to handle display
 */

void
SendWinC::Show()
{
   if ( IsEditOnly() ) {
      ClearMessage();
      HalTopLevelC::Show();
      return;
   }

//
// Initialize option buttons
//
   switch (ishApp->mailPrefs->mailType) {

      case (MAIL_PLAIN):
	 XmToggleButtonSetState(priv->optMsgPlainTB, True, True);
	 break;
      case (MAIL_MIME):
	 XmToggleButtonSetState(priv->optMsgMimeTB, True, True);
	 break;
      case (MAIL_ALT):
	 XmToggleButtonSetState(priv->optMsgAltTB, True, True);
	 break;
   }

   switch (ishApp->mailPrefs->textType) {

      case (CT_PLAIN):
	 XmToggleButtonSetState(priv->optTextPlainTB, True, True);
	 break;
      case (CT_ENRICHED):
	 XmToggleButtonSetState(priv->optTextRichTB, True, True);
	 break;
   }

   XmToggleButtonSetState(priv->optAddSigTB, ishApp->sigPrefs->appendSig,False);
   XmToggleButtonSetState(priv->optCheckAddrTB,
   			  ishApp->mailPrefs->verifyAddresses, False);
   XmToggleButtonSetState(priv->optWrapTB, ishApp->compPrefs->wrap, True);

   priv->UpdateFcc();

#if 0
   priv->bodyText->SetSize(ishApp->compPrefs->bodyRows,
   			   ishApp->compPrefs->bodyCols, *this);
#endif

   ClearMessage();
   HalTopLevelC::Show();

#if 0
//
// Do this because sometimes all fields don't show up until a resize
//
   Dimension	wd;
   XtVaGetValues(*this, XmNwidth, &wd, NULL);
   XtVaSetValues(*this, XmNwidth, wd+1, NULL);
//   XtVaSetValues(*this, XmNwidth, wd, NULL);
#endif

//
// Start autoSave timer
//
   if ( ishApp->compPrefs->autoSave && ishApp->compPrefs->autoSaveRate > 0 ) {
      priv->keystrokeCount = 0;
      priv->autoSaveTimer = XtAppAddTimeOut(ishApp->context,
			       ishApp->compPrefs->autoSaveRate*1000/*ms*/,
			       (XtTimerCallbackProc)SendWinP::CheckAutoSave,
			       (XtPointer)priv);
   }

} // End Show

/*---------------------------------------------------------------
 *  Method to set To text
 */

void
SendWinC::SetTo(const char *cs)
{
   if ( IsEditOnly() ) return;

   AddressC	addr(cs);
   priv->SetField(priv->toText, &addr);
}

/*---------------------------------------------------------------
 *  Method to determine if this window is unused
 */

Boolean
SendWinC::IsEmpty()
{
   if ( IsEditOnly() ) return False;

//
// Check header fields
//
   StringC	otherStr, otherHeaders;
   if ( otherVis ) {
     priv->otherText->GetString(otherStr, TT_PLAIN);
     otherHeaders = ishApp->mailPrefs->otherHeaders;
     if( otherStr.EndsWith("\n")) otherStr.CutEnd(1);
     if( otherHeaders.EndsWith("\n")) otherHeaders.CutEnd(1);
   }

   Boolean    toEmpty = (priv->toText->IsEmpty());
   Boolean    ccEmpty = (!ccVis    || priv->ccText->IsEmpty());
   Boolean   bccEmpty = (!bccVis   || priv->bccText->IsEmpty());
   Boolean otherEmpty = (!otherVis || priv->otherText->IsEmpty() ||
			  (otherStr == otherHeaders));

   Boolean empty = ( toEmpty && priv->subText->IsEmpty() && ccEmpty &&
   		     bccEmpty && otherEmpty && priv->bodyText->IsEmpty() );
   return empty;

} // End IsEmpty

/*---------------------------------------------------------------
 *  Method to reply to a particular message
 */

void
SendWinC::Reply(MsgC *msg, Boolean toAll, Boolean includeText)
{
   BusyCursor(True);

//
// See who sent the message
//
   AddressC	*toAddr = msg->ReplyTo();
   if ( !toAddr ) toAddr = msg->From();
   if ( toAddr ) priv->SetField(priv->toText, toAddr);

//
// See if others are to be added
//
   if ( toAll ) {

//
// Put To: and Cc: addresses in the Cc: field
//
      toAddr = msg->To();
      AddressC	*ccAddr = msg->Cc();

//
// Temporarily link them together
//
      AddressC	*newcc = toAddr;
      AddressC	**newccend = &newcc;
      while ( *newccend ) newccend = &((*newccend)->next);
      *newccend = ccAddr;
      if ( newcc ) priv->SetField(priv->ccText, newcc);
      *newccend = NULL;

   } // End if reply to all

//
// Display the subject string
//
   HeaderValC	*val = msg->Subject();
   StringC	text;
   if ( val ) {

//
// See if there is an alternate character set.  If there is, this header was
//    RFC1522 encoded.
//
      if ( val->charset ) {

//
// If the alternate charset is compatible with the text field, display the
//    decoded value.  If the charset is not compatible, display the encoded
//    value.
//
	 if ( CharsetOk(val->charset, priv->subText->Charset()) )
	    val->GetValueText(text);
	 else
	    text = val->full;

      } // End if there is an alternate charset

      else
	 text = val->full;

//
// Update the Re: pattern
//
      Boolean	reOk = False;
      if ( text.StartsWith("re", IGNORE_CASE) ) {

//
// See if the Re: should be numbered
//
	 if ( ishApp->replyPrefs->numberReplies ) {

	    if ( text[2] == ':' ) {
	       text(2,0) = "[2]";
	       reOk = True;
	    }

	    else if ( text[2] == '[' || text[2] == '(' ) {

//
// Look for closing delimiter
//
	       int	pos;
	       if ( text[2] == '[' ) pos = text.PosOf(']', (u_int)2);
	       else		     pos = text.PosOf(')', (u_int)2);

//
// Increment number by 1
//
	       if ( pos > 2 ) {
		  StringC	numStr = text(3,pos-3);
		  int	num = atoi(numStr) + 1;
		  numStr.Clear();
		  numStr += num;
		  text(3,pos-3) = numStr;
		  reOk = True;
	       }

	    } // End if range found

	 } // End if numbering replies

	 else if ( text[2] == ':' || text[2] == '[' || text[2] == '(' )
	    reOk = True;

      } // End if "re" at beginning of subject

      if ( !reOk ) text(0,0) = "Re: ";

   } // End if there is a subject

//
// Update the field
//
   priv->subText->SetString(text);

//
// Clear other fields
//
   SetBcc("");
   SetOther("");

//
// Display the text if necessary
//
   priv->bodyText->Clear();
   if ( includeText ) {

      priv->bodyText->Defer(True);

//
// Add the attribution
//
      StringC	attrib = msg->Attribution();
      priv->bodyText->AddStringEnriched(attrib);
      if ( !attrib.EndsWith('\n') ) priv->bodyText->AddStringPlain("\n");

//
// Add the message text
//
      priv->bodyText->CheckFroms(msg->HasSafeFroms());
      priv->bodyText->AddStringEnriched("<excerpt>");

      if ( msg->IsMime() ) {
	 MsgPartC	*tree = msg->Body();
	 priv->AddBodyTree(tree);
	 priv->bodyText->AddStringPlain("\n\n");
      }

      else {
	 StringC	body;
	 msg->GetBodyText(body);
	 priv->bodyText->AddStringPlain(body);
      }

      priv->bodyText->AddStringEnriched("</excerpt>");
      priv->bodyText->CheckFroms(False);

      priv->bodyText->Defer(False);

   } // End if we want to display the body

   priv->msgList.removeAll();
   priv->msgList.add(msg);
   type = SEND_REPLY;

   Show();
   changed = True;

//
// Move the cursor to the to field
//
   XmProcessTraversal(priv->toText->TextArea(), XmTRAVERSE_CURRENT);

   BusyCursor(False);

} // End Reply

/*---------------------------------------------------------------
 *  Method to forward one or more messages
 */

void
SendWinC::Forward(MsgListC& msgList, Boolean encapsulate)
{
   BusyCursor(True);

   priv->SetField(priv->toText,  NULL);
   priv->SetField(priv->ccText,  NULL);
   priv->SetField(priv->bccText, NULL);
   SetOther("");

//
// Display the subject of the first message
//
   MsgC		*msg = msgList[0];
   HeaderValC	*val = msg->Subject();
   StringC	text;
   if ( val ) {

//
// See if there is an alternate character set.  If there is, this header was
//    RFC1522 encoded.
//
      if ( val->charset ) {

//
// If the alternate charset is compatible with the text field, display the
//    decoded value.  If the charset is not compatible, display the encoded
//    value.
//
	 if ( CharsetOk(val->charset, priv->subText->Charset()) )
	    val->GetValueText(text);
	 else
	    text = val->full;

      } // End if there is an alternate charset

      else
	 text = val->full;

//
// Add the (fwd) text if not already present
//
      if ( !text.Contains("(fwd)", IGNORE_CASE) ) {
	 text.Trim();
	 text += " (fwd)";
      }

   } // End if there is a subject

   priv->subText->SetString(text);

//
// Add the messages to the body
//
   priv->bodyText->Defer(True);
   priv->bodyText->SetTextType(TT_PLAIN);
   priv->bodyText->Clear();

#if 0
//
// Make this window a digest if we're encapsulating
//
   priv->containerType = encapsulate ? CT_DIGEST : CT_MIXED;
#endif

//
// Add messages to the window
//
   u_int	count = msgList.size();
   int i=0; for (i=0; i<count; i++) {

      MsgC	*msg = msgList[i];

      if ( encapsulate ) priv->ForwardMsgEncap(msg);
      else		 priv->ForwardMsgInline(msg);

      priv->bodyText->AddStringPlain("\n\n");

   } // End for each message to be forwarded

   priv->bodyText->Defer(False);

//
// Remember these messages
//
   priv->msgList = msgList;
   type = SEND_FORWARD;

   Show();
   changed = True;

//
// Move the cursor to the to field
//
   XmProcessTraversal(priv->toText->TextArea(), XmTRAVERSE_CURRENT);

   BusyCursor(False);

} // End Forward

/*---------------------------------------------------------------
 *  Method to resend one or more messages
 */

void
SendWinC::Resend(MsgListC& msgList)
{
   BusyCursor(True);

   priv->SetField(priv->toText,  NULL);
   priv->SetField(priv->ccText,  NULL);
   priv->SetField(priv->bccText, NULL);
   SetOther("");

//
// Display the subject of the first message
//
   MsgC		*msg = msgList[0];
   HeaderValC	*val = msg->Subject();
   StringC	text;
   if ( val ) {

//
// See if there is an alternate character set.  If there is, this header was
//    RFC1522 encoded.
//
      if ( val->charset ) {

//
// If the alternate charset is compatible with the text field, display the
//    decoded value.  If the charset is not compatible, display the encoded
//    value.
//
	 if ( CharsetOk(val->charset, priv->subText->Charset()) )
	    val->GetValueText(text);
	 else
	    text = val->full;

      } // End if there is an alternate charset

      else
	 text = val->full;

   } // End if there is a subject

   priv->subText->SetString(text);

//
// Remember these messages.   Create temporary files for the resend.
//
   priv->msgList = msgList;
   type = SEND_RESEND;

   StringC	file;
   u_int	count = msgList.size();
   int i;
   for (i=0; i<count; i++) {

      char      *cs = tempnam(NULL, "rsnd.");
      file = cs;
      free(cs);

      MsgC	*msg = msgList[i];
      if ( msg->WriteFile(file, /*copyHead=*/True, /*allHead=*/True,
				/*statHead=*/False, /*addBlank=*/True,
				/*protectFroms=*/False) )
	 priv->resendFileList.add(file);
   }

//
// Display the window and move the cursor to the to field
//
   Show();

//
// Turn off unnecessary fields.  Have to do this after shown to get window
//    the correct size.
//
   priv->SetResendMode(True);

   changed = True;

   XmProcessTraversal(priv->toText->TextArea(), XmTRAVERSE_CURRENT);

   BusyCursor(False);

} // End Resend

/*---------------------------------------------------------------
 *  Determine if this window or any child windows have changed
 */

Boolean
SendWinC::Changed()
{
   if ( changed && !IsEmpty() ) return True;

//
// See if there are any open edit windows with changes
//
   unsigned	count   = priv->editWinList.size();
   int i=0; for (i=0; i<count; i++) {
      SendWinC	*editWin = (SendWinC *)*priv->editWinList[i];
      if ( editWin->IsShown() && editWin->Changed() ) return True;
   }

   return False;

} // End Changed

/*---------------------------------------------------------------
 *  Method to return body text pointer
 */

MimeRichTextC*
SendWinC::BodyText()
{
   return priv->bodyText;
}

/*---------------------------------------------------------------
 *  Method to return current description template
 */

StringC&
SendWinC::DescTemplate()
{
   return priv->descTemplate;
}

/*---------------------------------------------------------------
 *  Method to initialize the window from a message file
 */

Boolean
SendWinC::LoadFile(const char *name)
{
//
// Make sure name is not a directory
//
   if ( IsDir((char*)name) ) {
      StringC errmsg = name;
      errmsg += " is a directory.";
      PopupMessage(errmsg);
      return False;
   }

//
// If the file does not exist, that's bad
//
   if ( access(name, F_OK) != 0 ) {

      StringC errmsg = name;
      errmsg += " does not exist.";
      PopupMessage(errmsg);
      return False;
   }

//
// If the file can't be read, that's bad
//
   if ( access(name, R_OK) != 0 ) {

      StringC errmsg = name;
      errmsg += " is not readable.";
      PopupMessage(errmsg);
      return False;
   }

   BusyCursor(True);

//
// If clears are not confirmed, clear now.
//
   if ( !ishApp->confPrefs->confirmClearSend )
      priv->DoClear(NULL, priv, NULL);

//
// Create a message from the file
//
   FileMsgC	*msg = new FileMsgC(name, 0, -1, False);

//
// Load the header fields
//
   HeaderC	*head     = msg->Headers();
   HeaderC	*fccHead  = NULL;
   Boolean	showCc    = ishApp->compPrefs->showCc;
   Boolean	showBcc   = ishApp->compPrefs->showBcc;
   Boolean	showFcc   = ishApp->compPrefs->showFcc;
   Boolean	showOther = ishApp->compPrefs->showOther;
   while ( head ) {

      CharC	key = head->key;
      CharC	val = head->value->full;
      if ( key.Equals("Subject", IGNORE_CASE) )
	 priv->subText->SetString(val);

      else if ( key.Equals("To", IGNORE_CASE) ) {
	 if ( !priv->toText->IsEmpty() ) priv->toText->AddString(", ");
	 priv->toText->AddString(val);
      }

      else if ( key.Equals("Cc", IGNORE_CASE) ) {
	 if ( !priv->ccText->IsEmpty() ) priv->ccText->AddString(", ");
	 priv->ccText->AddString(val);
	 showCc = True;
      }

      else if ( key.Equals("Bcc", IGNORE_CASE) ) {
	 if ( !priv->bccText->IsEmpty() ) priv->bccText->AddString(", ");
	 priv->bccText->AddString(val);
	 showBcc = True;
      }

      else if ( key.Equals("Fcc", IGNORE_CASE) ) {
	 fccHead = head;	// Do this later.  Must be done after others
      }

      else if ( !key.Equals("Date",		IGNORE_CASE) &&
		!key.Equals("X-Mailer",		IGNORE_CASE) &&
		!key.Equals("MIME-Version",	IGNORE_CASE) &&
		!key.StartsWith("Resent-",	IGNORE_CASE) &&
		!key.StartsWith("Content-",	IGNORE_CASE) ) {
	 if ( !priv->otherText->IsEmpty() ) priv->otherText->AddString("\n");
	 priv->otherText->AddString(head->full);
	 showOther = True;
      }

      head = head->next;

   } // End for each header

   if ( fccHead ) {
      priv->fccText->SetString(fccHead->value->full);
      showFcc = True;
   }

   XmToggleButtonSetState(priv->optCcTB,    showCc,    True);
   XmToggleButtonSetState(priv->optBccTB,   showBcc,   True);
   XmToggleButtonSetState(priv->optFccTB,   showFcc,   True);
   XmToggleButtonSetState(priv->optOtherTB, showOther, True);

//
// Display the body
//
   MsgPartC	*tree = msg->Body();
   priv->AddBodyTree(tree);

   delete msg;

   BusyCursor(False);

   return True;

} // End LoadFile

/*---------------------------------------------------------------
 *  Method to set the name of the auto-save file
 */

void
SendWinC::SetAutoSaveFile(const char *name)
{
   if ( priv->autoSaveFile.size() > 0 ) unlink(priv->autoSaveFile);
   priv->autoSaveFile = name;
}

/*---------------------------------------------------------------
 *  Method to edit a message with no delivery intended
 */

void
SendWinC::EditMsg(MsgC *msg, Boolean editText, CallbackFn *doneFn,
		  void *userData)
{
   if ( !IsEditOnly() ) return;

   BusyCursor(True);

   priv->editDoneCall.Set(doneFn, userData);
   priv->editMsg     = msg;
   priv->editMsgText = editText;

//
// Turn on the header text field
//
   XtManageChild(priv->headText->MainWidget());

   StringC	title;
   msg->GetSubjectText(title);
   XtVaSetValues(*this, XmNtitle, (char*)title, NULL);

//
// Show the headers
//
   StringC	textStr;
   msg->GetHeaderText(textStr);
   priv->headText->SetString(textStr);

//
// Show the body
//
   priv->DisplayBody(msg, editText);

   priv->UpdateEditButtons();

   BusyCursor(False);

} // End EditMsg

/*---------------------------------------------------------------
 *  Method to receive notice that a message has been deleted
 */

void
SendWinC::MessageDeleted(MsgC *msg)
{
   if ( priv->msgList.includes(msg) ) priv->msgList.remove(msg);
}
