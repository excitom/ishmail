/*
 *  $Id: SendInc.C,v 1.4 2000/05/07 12:26:12 fnevgeny Exp $
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

#include "SendWinP.h"
#include "SendWinC.h"
#include "SigPrefC.h"
#include "SendIconC.h"
#include "MainWinC.h"
#include "FolderC.h"
#include "MsgC.h"
#include "FileChooserWinC.h"
#include "QuotedP.h"
#include "Base64.h"
#include "MimeEncode.h"
#include "IncludeWinC.h"
#include "Misc.h"
#include "FileMisc.h"

#include <hgl/MimeRichTextC.h>
#include <hgl/WArgList.h>
#include <hgl/rsrc.h>
#include <hgl/SysErr.h>

#include <Xm/SelectioB.h>
#include <Xm/Frame.h>
#include <Xm/RowColumn.h>
#include <Xm/ToggleB.h>

#include <errno.h>

/*---------------------------------------------------------------
 *  Callbacks to handle include signature
 */

void
SendWinP::DoIncludeExtPSig(Widget, SendWinP *This, XtPointer)
{
   StringC	sigStr = ishApp->sigPrefs->Sig();
   This->bodyText->SetTextType(TT_PLAIN);
   This->bodyText->InsertString(sigStr);
   This->bodyText->SetTextType(TT_ENRICHED);
}

void
SendWinP::DoIncludeExtESig(Widget, SendWinP *This, XtPointer)
{
   StringC	sigStr = ishApp->sigPrefs->EnrichedSig();
   This->bodyText->SetTextType(TT_ENRICHED);
   This->bodyText->InsertString(sigStr);
}

void
SendWinP::DoIncludeIntPSig(Widget, SendWinP *This, XtPointer)
{
   StringC	sigStr = ishApp->sigPrefs->InternalSig();
   This->bodyText->SetTextType(TT_PLAIN);
   This->bodyText->InsertString(sigStr);
   This->bodyText->SetTextType(TT_ENRICHED);
}

void
SendWinP::DoIncludeIntESig(Widget, SendWinP *This, XtPointer)
{
   StringC	sigStr = ishApp->sigPrefs->InternalEnrichedSig();
   This->bodyText->SetTextType(TT_ENRICHED);
   This->bodyText->InsertString(sigStr);
}

/*-----------------------------------------------------------------------
 *  Callbacks used to add multipart containers
 */

void
SendWinP::DoAddAlt(Widget, SendWinP *This, XtPointer)
{
   This->AddContainer(CT_ALTERNATIVE);
}

void
SendWinP::DoAddDigest(Widget, SendWinP *This, XtPointer)
{
   This->AddContainer(CT_DIGEST);
}

void
SendWinP::DoAddParallel(Widget, SendWinP *This, XtPointer)
{
   This->AddContainer(CT_PARALLEL);
}

void
SendWinP::DoAddMixed(Widget, SendWinP *This, XtPointer)
{
   This->AddContainer(CT_MIXED);
}

/*-----------------------------------------------------------------------
 *  Method used to add a multipart container
 */

void
SendWinP::AddContainer(MimeContentType contType)
{
//
// Create a multipart graphic for this container
//
   char	*ct;
   if      ( contType == CT_ALTERNATIVE ) ct = "multipart/alternative";
   else if ( contType == CT_DIGEST      ) ct = "multipart/digest";
   else if ( contType == CT_PARALLEL    ) ct = "multipart/parallel";
   else					  ct = "multipart/mixed";

   SendIconC	*icon = new SendIconC(pub, ct);
   icon->AddDoubleClickCallback((CallbackFn*)OpenPart,     this);
   icon->AddMenuCallback       ((CallbackFn*)PostPartMenu, this);
   bodyText->InsertGraphic(icon);

} // End AddContainer

/*---------------------------------------------------------------
 *  Callback to handle include message
 */

void
SendWinP::DoIncludeMsg(Widget, SendWinP *This, XtPointer)
{
//
// Prompt for message number
//
   if ( !This->incMsgWin ) {

      WArgList	args;
      args.Reset();
      args.DialogStyle(XmDIALOG_FULL_APPLICATION_MODAL);
      args.AutoUnmanage(False);
      This->incMsgWin = XmCreatePromptDialog(*This->pub, "includeMsgWin",ARGS);

      XtAddCallback(This->incMsgWin, XmNokCallback,
      		    (XtCallbackProc)IncludeMsgOk, (XtPointer)This);
      XtAddCallback(This->incMsgWin, XmNcancelCallback,
      		    (XtCallbackProc)IncludeMsgCancel, (XtPointer)This);
      XtAddCallback(This->incMsgWin, XmNhelpCallback,
		    (XtCallbackProc)HalAppC::DoHelp, (char *) "helpcard");

//
// Create toggle buttons to choose in-line/icon
//
      Widget	frame = XmCreateFrame(This->incMsgWin, "typeFrame", 0,0);
      Widget	radio = XmCreateRadioBox(frame, "typeRadio", 0,0);
      This->incMsgAsTextTB = XmCreateToggleButton(radio, "incMsgAsTextTB", 0,0);
      This->incMsgAsIconTB = XmCreateToggleButton(radio, "incMsgAsIconTB", 0,0);
      XtManageChild(This->incMsgAsTextTB);
      XtManageChild(This->incMsgAsIconTB);
      XtManageChild(radio);
      XtManageChild(frame);
      XmToggleButtonSetState(This->incMsgAsIconTB, True, True);

   } // End if dialog needed

//
// Show the dialog
//
   PopupOver(This->incMsgWin, *This->pub);

} // End DoIncludeMsg

/*---------------------------------------------------------------
 *  Callback to finish include message
 */

void
SendWinP::IncludeMsgOk(Widget, SendWinP *This, XtPointer)
{
//
// Read number from text field
//
   XmString	xmstr;
   XtVaGetValues(This->incMsgWin, XmNtextString, &xmstr, NULL);

   char	*cs;
   XmStringGetLtoR(xmstr, XmFONTLIST_DEFAULT_TAG, &cs);
   StringC	numStr(cs);
   XtFree(cs);

//
// See if a number was entered
//
   if ( numStr.size() == 0 ) {
      Widget w = XmSelectionBoxGetChild(This->incMsgWin, XmDIALOG_TEXT);
      set_invalid(w, True, True);
      This->pub->PopupMessage("Please enter a message number.");
      return;
   }

//
// Get list of message numbers
//
   StringListC	numList;
   ExtractList(numStr, numList);

   Boolean	error    = False;
   MsgListC&	msgList  = *ishApp->mainWin->curFolder->msgList;
   u_int	msgCount = msgList.size();
   u_int	numCount = numList.size();
   for (int n=0; n<numCount; n++) {

//
// See if we can find the message in the current folder
//
      int	num      = atoi(*numList[n]);
      MsgC	*numMsg  = NULL;
      for (int i=0; !numMsg && i<msgCount; i++) {
	 MsgC	*msg = msgList[i];
	 if ( num == msg->Number() ) numMsg = msg;
      }

      if ( !numMsg ) {
	 Widget w = XmSelectionBoxGetChild(This->incMsgWin, XmDIALOG_TEXT);
	 set_invalid(w, True, True);
	 StringC	msgStr("Message \"");
	 msgStr += numStr;
	 msgStr += "\" does not exist in the current folder.";
	 This->pub->PopupMessage(msgStr);
	 error = True;
      }

      else if ( XmToggleButtonGetState(This->incMsgAsTextTB) ) {

//
// Read in the text
//
	 This->bodyText->SetTextType(TT_PLAIN);
	 StringC	body;
	 numMsg->GetBodyText(body);
	 This->bodyText->InsertString(body);
	 This->bodyText->SetTextType(TT_ENRICHED);

      } // End if message to be included as plain text

      else {

//
// Create an icon for this message
//
	 SendIconC	*icon = new SendIconC(This->pub, numMsg);
	 icon->AddDoubleClickCallback((CallbackFn*)OpenPart,     This);
	 icon->AddMenuCallback       ((CallbackFn*)PostPartMenu, This);
	 This->bodyText->InsertGraphic(icon);

      } // End if file not included as plain text

   } // End for each listed message

   if ( !error )
      XtUnmanageChild(This->incMsgWin);

} // End IncludeMsgOk

/*---------------------------------------------------------------
 *  Callback to cancel include message
 */

void
SendWinP::IncludeMsgCancel(Widget, SendWinP *This, XtPointer)
{
   XtUnmanageChild(This->incMsgWin);
}

/*-----------------------------------------------------------------------
 *  Handle resize of extra frame in include text dialog
 */

static void
HandleIncTextExpose(Widget w, XtPointer, XEvent *ev, Boolean*)
{
   if ( ev->type != MapNotify ) return;

//
// Fix sizes of pane
//
   Dimension	ht;
   XtVaGetValues(w, XmNheight, &ht, NULL);
   XtVaSetValues(w, XmNpaneMinimum, ht, XmNpaneMaximum, ht, 0);
}

/*---------------------------------------------------------------
 *  Callback to handle include text file
 */

void
SendWinP::DoIncludeText(Widget, SendWinP *This, XtPointer)
{
   This->pub->BusyCursor(True);

   if ( !This->incTextWin ) {

      This->incTextWin = new FileChooserWinC(*This->pub, "includeTextWin");

//
// Add pane for inline/attach.  Put it in the last position.
//
      Cardinal	childCount;
      XtVaGetValues(This->incTextWin->PanedWin(), XmNnumChildren, &childCount,
      		    NULL);

      WArgList	args;
      args.PositionIndex(childCount+1);
      args.ShadowThickness(0);
      Widget centerFrame =
	 XmCreateFrame(This->incTextWin->PanedWin(), "centerFrame", ARGS);

      args.Reset();
      args.ChildType(XmFRAME_TITLE_CHILD);
      args.ChildHorizontalAlignment(XmALIGNMENT_CENTER);
      Widget frame = XmCreateFrame(centerFrame, "includeFrame", ARGS);

      args.Reset();
      args.Orientation(XmHORIZONTAL);
      args.Packing(XmPACK_TIGHT);
      Widget radio = XmCreateRadioBox(frame, "includeRadio", ARGS);

      This->incTextAsTextTB = XmCreateToggleButton(radio,"incTextAsTextTB",0,0);
      This->incTextAsIconTB = XmCreateToggleButton(radio,"incTextAsIconTB",0,0);

      XtManageChild(This->incTextAsTextTB);
      XtManageChild(This->incTextAsIconTB);
      XtManageChild(radio);	// frame children
      XtManageChild(frame);
      XtManageChild(centerFrame);

//
// Add a callback so we can fix the size of the pane we just added
//
      XtAddEventHandler(centerFrame, StructureNotifyMask, False,
			HandleIncTextExpose, NULL);

      XmToggleButtonSetState(This->incTextAsTextTB, True, True);

      This->incTextWin->HandleHelp(This->incTextAsTextTB);
      This->incTextWin->HandleHelp(This->incTextAsIconTB);

      This->incTextWin->AddOkCallback((CallbackFn*)FinishIncludeText, This);
      This->incTextWin->HideList();
      This->incTextWin->HideImap();
   }

   This->incTextWin->Show(*This->pub);

   This->pub->BusyCursor(False);

} // End DoIncludeText

/*---------------------------------------------------------------
 *  Callback to handle acceptance of text file names
 */

void
SendWinP::FinishIncludeText(StringListC *list, SendWinP *This)
{
   This->pub->BusyCursor(True);

//
// Loop through files
//
   StringC	fileBody;
   Boolean	error = False;
   u_int	count = list->size();
   for (int i=0; i<count; i++) {

      StringC	*fileName = (*list)[i];

//
// Read in the text or create a graphic
//
      if ( XmToggleButtonGetState(This->incTextAsTextTB) ) {

	 fileBody.Clear();
	 if ( !fileBody.ReadFile(*fileName) ) {
	    fileBody = "Could not read file: \"";
	    fileBody += *fileName;
	    fileBody += "\".\n";
	    fileBody += SystemErrorMessage(errno);
	    This->pub->PopupMessage(fileBody);
	    error = True;
	 }

	 else {
	    This->bodyText->SetTextType(TT_PLAIN);
	    This->bodyText->InsertString(fileBody);
	    This->bodyText->SetTextType(TT_ENRICHED);
	 }

      } // End if file to be included as text

      else {

//
// Create a file graphic for this file
//
	 SendIconC *icon = new SendIconC(This->pub, "text/plain", *fileName,
	 				 (char*)BaseName(*fileName).Addr());
	 icon->AddDoubleClickCallback((CallbackFn*)OpenPart,     This);
	 icon->AddMenuCallback       ((CallbackFn*)PostPartMenu, This);
	 This->bodyText->InsertGraphic(icon);

      } // End if file included as icon

   } // End for each file

   if ( error ) This->incTextWin->HideOk(False);

   This->pub->BusyCursor(False);

} // End FinishIncludeText

/*---------------------------------------------------------------
 *  Callback to handle include file
 */

void
SendWinP::DoIncludeFile(Widget, SendWinP *This, XtPointer)
{
   This->pub->BusyCursor(True);

   if ( !This->incFileWin ) {
      This->incFileWin = new FileChooserWinC(*This->pub, "includeFileWin");
      This->incFileWin->AddOkCallback((CallbackFn*)FinishIncludeFile, This);
      This->incFileWin->HideList();
      This->incFileWin->HideImap();
   }

   This->incFileWin->Show(*This->pub);

   This->pub->BusyCursor(False);

} // End DoIncludeFile

/*---------------------------------------------------------------
 *  Callback to handle acceptance of file names
 */

void
SendWinP::FinishIncludeFile(StringListC *list, SendWinP *This)
{
   This->pub->BusyCursor(True);

//
// Create dialog if necessary
//
   if ( !This->fileDataWin ) {
      This->fileDataWin = new IncludeWinC(*This->pub, "fileDataWin");
      This->fileDataWin->AddOkCallback  ((CallbackFn*)IncludeFileOk,   This);
      This->fileDataWin->AddHideCallback((CallbackFn*)IncludeFileHide, This);
   }

   This->modifying = False;
   This->fileDataWin->Show(list);
   This->pub->BusyCursor(False);

} // End FinishIncludeFile

/*---------------------------------------------------------------
 *  Method to accept data for an included file
 */

void
SendWinP::IncludeFileOk(IncludeWinC *iw, SendWinP *This)
{
   if ( This->modifying ) {

      This->modIcon->AnimationOff();
      This->modIcon->Unhighlight();

//
// If we're changing to inline text, delete the graphic.
//
      if ( iw->IncludeAsText() ) {
	 This->bodyText->MoveCursor(This->modIcon, 0);
	 This->bodyText->RemoveGraphic(This->modIcon);
	 This->modIcon = NULL;
      }

      else {
	 This->modIcon->Update(iw);
	 return;
      }

   } // End if modifying

//
// Read in the text or create a graphic
//
   StringC	fileName;
   iw->GetFileName(fileName);
   if ( iw->IncludeAsText() ) {

      if ( iw->NoEncoding() ) {

	 TextTypeT	ttype = TT_PLAIN;
	 if      ( iw->IsTextEnriched() ) ttype = TT_ENRICHED;
	 else if ( iw->IsTextRichtext() ) ttype = TT_RICH;
	 This->bodyText->SetTextType(ttype);
      }

      else {

	 This->bodyText->SetTextType(TT_PLAIN);

	 if ( !iw->AlreadyEncoded() ) {

//
// Encode the file and store in a temp file
//
	    char	*tmpFile = tempnam(NULL, "enc.");
	    if ( iw->IsQP() )
	       FileToFileQP(fileName, tmpFile);
	    else if ( iw->IsBase64()   )
	       FileToFile64(fileName, tmpFile, iw->IsText());
	    else if ( iw->IsUUencode() )
	       FileToFileUU(fileName, tmpFile);
	    else if ( iw->IsBinHex() )
	       FileToFileBH(fileName, tmpFile);

	    fileName = tmpFile;
	    free(tmpFile);

	 } // End if file not already encoded

      } // End if file to be encoded before inclusion

//
// Display the body of the file
//
      StringC	fileBody;
      fileBody.ReadFile(fileName);
      This->bodyText->InsertString(fileBody);

      This->bodyText->SetTextType(TT_ENRICHED);

   } // End if file to be included as plain text

   else {

//
// Create a file graphic for this file
//
      SendIconC	*icon = new SendIconC(This->pub, iw);
      icon->AddDoubleClickCallback((CallbackFn*)OpenPart,     This);
      icon->AddMenuCallback       ((CallbackFn*)PostPartMenu, This);
      This->bodyText->InsertGraphic(icon);

   } // End if file not included as plain text

} // End IncludeFileOk

/*---------------------------------------------------------------
 *  Method to handle close of include file window
 */

void
SendWinP::IncludeFileHide(IncludeWinC *iw, SendWinP *This)
{
   if ( This->modifying && This->modIcon ) {
      This->modIcon->AnimationOff();
      This->modIcon->Unhighlight();
   }
}
