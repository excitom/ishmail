/*
 *  $Id: SendEdit.C,v 1.5 2000/05/07 12:26:12 fnevgeny Exp $
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
#include "ButtonMgrC.h"
#include "IncludeWinC.h"
#include "SendIconC.h"
#include "FileMsgC.h"
#include "MsgPartC.h"
#include "ConfPrefC.h"
#include "Misc.h"
#include "FileChooserWinC.h"
#include "Query.h"
#include "FileMisc.h"
#include "HeaderC.h"
#include "HeaderValC.h"
#include "CompPrefC.h"
#include "edit.h"
#include "SendMisc.h"
#ifdef HGLTERM
#include "TermWinC.h"
#endif

#include <hgl/MimeRichTextC.h>
#include <hgl/WXmString.h>
#include <hgl/WArgList.h>
#include <hgl/PixmapC.h>
#include <hgl/SysErr.h>
#include <hgl/rsrc.h>
#include <hgl/MemMap.h>

#include <Xm/MessageB.h>
#include <Xm/RowColumn.h>	// For XmMenuPosition
#include <Xm/ToggleB.h>

#include <unistd.h>
#include <errno.h>

/*---------------------------------------------------------------
 *  Callbacks to handle enriched editing commands
 */

void
SendWinP::DoUndelete(Widget, SendWinP *This, XtPointer)
{
   This->bodyText->Undelete();
   XtSetSensitive(This->editUndeletePB, False);
   This->buttMgr->SensitivityChanged(This->editUndeletePB);
}

void
SendWinP::DoPlain(Widget, SendWinP *This, XtPointer)
{
   This->bodyText->ChangeFont(FC_PLAIN);
}

void
SendWinP::DoBold(Widget, SendWinP *This, XtPointer)
{
   This->bodyText->ChangeFont(FC_BOLD);
}

void
SendWinP::DoItalic(Widget, SendWinP *This, XtPointer)
{
   This->bodyText->ChangeFont(FC_ITALIC);
}

void
SendWinP::DoFixed(Widget, SendWinP *This, XtPointer)
{
   This->bodyText->ChangeFont(FC_FIXED);
}

void
SendWinP::DoBigger(Widget, SendWinP *This, XtPointer)
{
   This->bodyText->ChangeFont(FC_BIGGER);
}

void
SendWinP::DoSmaller(Widget, SendWinP *This, XtPointer)
{
   This->bodyText->ChangeFont(FC_SMALLER);
}

void
SendWinP::DoUnderline(Widget, SendWinP *This, XtPointer)
{
   This->bodyText->ChangeFont(FC_UNDERLINE);
}

void
SendWinP::DoCenter(Widget, SendWinP *This, XtPointer)
{
   This->bodyText->ChangeJust(JC_CENTER);
}

void
SendWinP::DoFlushLeft(Widget, SendWinP *This, XtPointer)
{
   This->bodyText->ChangeJust(JC_LEFT);
}

void
SendWinP::DoFlushRight(Widget, SendWinP *This, XtPointer)
{
   This->bodyText->ChangeJust(JC_RIGHT);
}

void
SendWinP::DoFlushBoth(Widget, SendWinP *This, XtPointer)
{
   This->bodyText->ChangeJust(JC_BOTH);
}

void
SendWinP::DoNoFill(Widget, SendWinP *This, XtPointer)
{
   This->bodyText->ChangeJust(JC_NOFILL);
}

void
SendWinP::DoExcerptMore(Widget, SendWinP *This, XtPointer)
{
   This->bodyText->ChangeJust(JC_EXCERPT_MORE);
}

void
SendWinP::DoExcerptLess(Widget, SendWinP *This, XtPointer)
{
   This->bodyText->ChangeJust(JC_EXCERPT_LESS);
}

void
SendWinP::DoLeftMarginIn(Widget, SendWinP *This, XtPointer)
{
   This->bodyText->ChangeMargin(MC_LEFT_IN);
}

void
SendWinP::DoLeftMarginOut(Widget, SendWinP *This, XtPointer)
{
   This->bodyText->ChangeMargin(MC_LEFT_OUT);
}

void
SendWinP::DoRightMarginIn(Widget, SendWinP *This, XtPointer)
{
   This->bodyText->ChangeMargin(MC_RIGHT_IN);
}

void
SendWinP::DoRightMarginOut(Widget, SendWinP *This, XtPointer)
{
   This->bodyText->ChangeMargin(MC_RIGHT_OUT);
}

void
SendWinP::DoColorRed(Widget, SendWinP *This, XtPointer)
{
   This->bodyText->ChangeColor("Red");
}

void
SendWinP::DoColorGreen(Widget, SendWinP *This, XtPointer)
{
   This->bodyText->ChangeColor("Green");
}

void
SendWinP::DoColorBlue(Widget, SendWinP *This, XtPointer)
{
   This->bodyText->ChangeColor("Blue");
}

void
SendWinP::DoColorYellow(Widget, SendWinP *This, XtPointer)
{
   This->bodyText->ChangeColor("Yellow");
}

void
SendWinP::DoColorMagenta(Widget, SendWinP *This, XtPointer)
{
   This->bodyText->ChangeColor("Magenta");
}

void
SendWinP::DoColorCyan(Widget, SendWinP *This, XtPointer)
{
   This->bodyText->ChangeColor("Cyan");
}

void
SendWinP::DoColorBlack(Widget, SendWinP *This, XtPointer)
{
   This->bodyText->ChangeColor("Black");
}

void
SendWinP::DoColorWhite(Widget, SendWinP *This, XtPointer)
{
   This->bodyText->ChangeColor("White");
}

void
SendWinP::DoColorOther(Widget, SendWinP *This, XtPointer)
{
}

void
SendWinP::DoColorNone(Widget, SendWinP *This, XtPointer)
{
   This->bodyText->ChangeColor("None");
}

/*-----------------------------------------------------------------------
 *  Handle display of part popup menu
 */

void
SendWinP::PostPartMenu(SendIconC *icon, SendWinP *This)
{
   This->popupIcon = icon;
   MsgPartC	*part = icon->data;

   if ( part->IsMultipart() ) {
      XtUnmanageChild(This->mimePUEditPB);
      XtManageChild  (This->mimePUContPB);
      XtManageChild  (This->mimePUTypeCB);
   }
   else if ( part->Is822() ) {
      XtManageChild  (This->mimePUEditPB);
      XtManageChild  (This->mimePUContPB);
      XtUnmanageChild(This->mimePUTypeCB);
   }
   else {
      XtManageChild  (This->mimePUEditPB);
      XtUnmanageChild(This->mimePUContPB);
      XtUnmanageChild(This->mimePUTypeCB);
   }

   WXmString	wstr = (char *)This->popupIcon->labelStr;
   XtVaSetValues(This->mimePULabel, XmNlabelString, (XmString)wstr, NULL);
   XtManageChild(This->mimePULabel);

   XmMenuPosition(This->mimePU, icon->lastEvent);
   XtManageChild(This->mimePU);

} // End PostPartMenu

/*---------------------------------------------------------------
 *  Callback to handle double-click on a part icon
 */

void
SendWinP::OpenPart(SendIconC *icon, SendWinP *This)
{
   This->popupIcon = icon;
   MsgPartC	*part = icon->data;

//
// Call appropriate edit methods, depending on type
//
   if ( part->IsMultipart() || part->Is822() )
      DoEditContents(NULL, This, NULL);
   else
      DoEditPart(NULL, This, NULL);

} // End OpenPart

/*---------------------------------------------------------------
 *  Callback to handle edit of part attributes
 */

void
SendWinP::DoEditPart(Widget, SendWinP *This, XtPointer)
{
//
// Get pointer to this part
//
   SendIconC	*icon = This->popupIcon;

   This->pub->BusyCursor(True);

   This->modifying = True;
   This->modIcon   = icon;

   icon->Highlight();

//
// Set up include window
//
   if ( !This->fileDataWin ) {
      This->fileDataWin = new IncludeWinC(*This->pub, "fileDataWin");
      This->fileDataWin->AddOkCallback  ((CallbackFn *)IncludeFileOk,   This);
      This->fileDataWin->AddHideCallback((CallbackFn *)IncludeFileHide, This);
   }

   This->fileDataWin->Show(icon);

   This->LoadEditPixmaps(icon);
   icon->Animate(This->editPixmaps);

   This->pub->BusyCursor(False);

} // End DoEditPart

/*---------------------------------------------------------------
 *  Callback to handle edit of part contents
 */

void
SendWinP::DoEditContents(Widget, SendWinP *This, XtPointer)
{
//
// See if this part is already being edited
//
   u_int	count = This->editWinList.size();
   for (int i=0; i<count; i++) {
      SendWinC	*editWin = (SendWinC*)*This->editWinList[i];
      if ( editWin->priv->editIcon == This->popupIcon ) {
	 editWin->HalTopLevelC::Show();
	 return;
      }
   }

   This->pub->BusyCursor(True);

   This->popupIcon->Highlight();

//
// Bring up a window showing the contents
//
   SendWinC	*editWin = This->GetEditWin();
   editWin->priv->Edit(This->popupIcon);

   This->LoadEditPixmaps(This->popupIcon);
   This->popupIcon->Animate(This->editPixmaps);

   This->pub->BusyCursor(False);

} // End DoEditContents

/*---------------------------------------------------------------
 *  Method to find or create a new edit window
 */

SendWinC*
SendWinP::GetEditWin()
{
//
// First look for an existing, unused window
//
   SendWinC	*editWin = NULL;
   unsigned	count = editWinList.size();
   Boolean	found = False;
   for (int i=0; !found && i<count; i++) {

      editWin = (SendWinC*)*editWinList[i];
      if ( !editWin->IsShown() ) found = True;

   } // End for each existing edit window

//
// If we didn't find a window, we need to create a new one
//
   if ( !found ) {

      pub->BusyCursor(True);

      editWin = new SendWinC("sendWin", *pub, SEND_EDIT);
      void	*tmp = (void*)editWin;
      editWinList.add(tmp);

//
// Set the values to support queries
//
      editWin->priv->optCcTB        = optCcTB;
      editWin->priv->optBccTB       = optBccTB;
      editWin->priv->optFccTB       = optFccTB;
      editWin->priv->optOtherTB     = optOtherTB;
      editWin->priv->optMimeTB      = optMimeTB;
      editWin->priv->optAddSigTB    = optAddSigTB;
      editWin->priv->optDigSignTB   = optDigSignTB;
      editWin->priv->optEncryptTB   = optEncryptTB;
      editWin->priv->optCheckAddrTB = optCheckAddrTB;
      editWin->priv->optMsgAltTB    = optMsgAltTB;
      editWin->priv->optMsgMimeTB   = optMsgMimeTB;
      editWin->priv->optMsgPlainTB  = optMsgPlainTB;
      editWin->priv->optTextRichTB  = optTextRichTB;
      editWin->priv->optTextPlainTB = optTextPlainTB;

      pub->BusyCursor(False);
   }

   return editWin;

} // End GetEditWin

/*---------------------------------------------------------------
 *  Method used to edit an rfc822 part or container
 */

void
SendWinP::Edit(SendIconC *icon)
{
   pub->HalTopLevelC::Show();

   editIcon = icon;

   bodyText->Defer(True);
   bodyText->Clear();

   if ( icon->data->Is822() ) {

//
// Turn on the header text field
//
      XtManageChild(headText->MainWidget());

//
// If there's no message, create one
//
      FileMsgC	*msg = icon->data->ChildMsg();

      StringC	title;
      msg->GetSubjectText(title);
      XtVaSetValues(*pub, XmNtitle, (char*)title, NULL);

//
// Show the headers
//
      StringC	headStr;
      msg->GetHeaderText(headStr);
      headText->SetString(headStr);

//
// Show the body
//
      DisplayBody(msg);

   } // End if we're editing a message

   else {

      containerType = icon->data->conType;

//
// Turn off the header text field
//
      XtUnmanageChild(headText->MainWidget());

      XtVaSetValues(*pub, XmNtitle, (char*)icon->data->conStr, NULL);

//
// Display the tree
//
      AddBodyTree(icon->data);

   } // End if we're not editing a message

   bodyText->ScrollTop();
   bodyText->Defer(False);

   pub->changed = False;

} // End Edit

/*---------------------------------------------------------------
 *  Callback to handle part deletion
 */

void
SendWinP::DoDeletePart(Widget, SendWinP *This, XtPointer)
{
   if ( !ishApp->confPrefs->confirmDeleteGraphic ) {
      FinishDelete(NULL, This, NULL);
      return;
   }

//
// Build verification dialog if necessary
//
   if ( !This->deleteWin ) {

      WArgList  args;
      args.AutoUnmanage(True);
      if ( halApp->questionPM ) args.SymbolPixmap(halApp->questionPM);
      This->deleteWin = XmCreateQuestionDialog(*This->pub, "deleteWin", ARGS);
      XtAddCallback(This->deleteWin, XmNokCallback,
                    (XtCallbackProc)FinishDelete, (XtPointer)This);
      XtAddCallback(This->deleteWin, XmNhelpCallback,
                    (XtCallbackProc)HalAppC::DoHelp, (char *) "helpcard");
   }

   PopupOver(This->deleteWin, *This->pub);

} // End DeletePart

void
SendWinP::FinishDelete(Widget, SendWinP *This, XtPointer)
{
   This->bodyText->RemoveGraphic(This->popupIcon);
   This->popupIcon = NULL;
}

/*---------------------------------------------------------------
 *  Callbacks to handle modification of container type
 */

void
SendWinP::DoModMixed(Widget, SendWinP *This, XtPointer)
{
   This->popupIcon->data->SetType("multipart/mixed");
}

void
SendWinP::DoModDigest(Widget, SendWinP *This, XtPointer)
{
   This->popupIcon->data->SetType("multipart/digest");
}

void
SendWinP::DoModAlt(Widget, SendWinP *This, XtPointer)
{
   This->popupIcon->data->SetType("multipart/alternative");
}

void
SendWinP::DoModParallel(Widget, SendWinP *This, XtPointer)
{
   This->popupIcon->data->SetType("multipart/parallel");
}

/*---------------------------------------------------------------
 *  Method to load pixmaps used for attachment editing animation
 */

void
SendWinP::LoadEditPixmaps(SendIconC *icon)
{
   if ( editPixmaps->size() > 0 ) return;

   Window       win = RootWindowOfScreen(halApp->screen);
   if ( !win ) return;

   StringC	names;
   StringListC	nameList;
   nameList.AllowDuplicates(TRUE);

   names = get_string(*pub, "editPixmaps", "edit-0.xbm" );
   ExtractList(names, nameList);;

   u_int	count = nameList.size();
   for (int i=0; i<count; i++) {

      StringC	*name = nameList[i];
      PixmapC	*pm   = new PixmapC(*name, icon->regFgColor, icon->regBgColor,
      					   icon->invFgColor, icon->invBgColor,
					   halApp->screen, win);
      void	*tmp = (void*)pm;
      editPixmaps->add(tmp);
   }

} // End LoadEditPixmaps

/*---------------------------------------------------------------
 *  Callback to save composition to a file
 */

void
SendWinP::DoSaveFile(Widget, SendWinP *This, XtPointer)
{
//
// Create dialog for getting file name
//
   if ( !This->saveFileWin ) {

      This->saveFileWin = new FileChooserWinC(*This->pub, "saveCompositionWin");
      This->saveFileWin->AddOkCallback((CallbackFn*)FinishSaveFile, This);
      This->saveFileWin->ShowDirsInFileList(False);
      This->saveFileWin->ShowFilesInFileList(True);
      This->saveFileWin->SetDirectory(".");
      This->saveFileWin->HideImap();

   } // End if saveFileWin needed

   This->saveFileWin->Show(*This->pub);

} // End DoSaveFile

/*---------------------------------------------------------------
 *  Callback to handle selection of file in save dialog
 */

void
SendWinP::FinishSaveFile(StringListC *list, SendWinP *This)
{
   StringC	name = *((*list)[0]);
   if ( !This->OkToSaveTo(name) ) return;

   if ( !This->Save(name) )
      This->saveFileWin->HideOk(False);
   else {
      StringC	msg("Composition saved to ");
      msg += name;
      This->pub->Message(msg);
   }

} // End FinishSaveFile

/*---------------------------------------------------------------
 *  Method to determine whether it's ok to save to the given file
 */

Boolean
SendWinP::OkToSaveTo(StringC& file)
{
//
// If the file doesn't exist, we're fine
//
   if ( access(file, F_OK) != 0 ) return True;

//
// Make sure name is not a directory
//
   if ( IsDir(file) ) {
      StringC errmsg = file;
      errmsg += " is a directory.";
      pub->PopupMessage(errmsg);
      return False;
   }

//
// See whether the file can be overwritten
//
   if ( OverwriteQuery(file, *pub) == QUERY_YES ) {
      unlink(file);
      return True;
   }

   return False;

} // End OkToSaveTo

/*---------------------------------------------------------------
 *  Callback to load composition from a file
 */

void
SendWinP::DoLoadFile(Widget, SendWinP *This, XtPointer)
{
//
// If clears are to be confirmed, clear now.
//
   if ( ishApp->confPrefs->confirmClearSend ) {

      DoClear(NULL, This, NULL);

//
// If they didn't clear, return
//
      if ( !This->pub->IsEmpty() ) return;
   }

//
// Create dialog for getting file name
//
   if ( !This->loadFileWin ) {

      This->loadFileWin = new FileChooserWinC(*This->pub, "loadCompositionWin");
      This->loadFileWin->AddOkCallback((CallbackFn*)FinishLoadFile, This);
      This->loadFileWin->ShowDirsInFileList(False);
      This->loadFileWin->ShowFilesInFileList(True);
      This->loadFileWin->SetDirectory(".");
      This->loadFileWin->HideImap();

   } // End if loadFileWin needed

   This->loadFileWin->Show(*This->pub);

} // End DoLoadFile

/*---------------------------------------------------------------
 *  Callback to handle selection of file in load dialog
 */

void
SendWinP::FinishLoadFile(StringListC *list, SendWinP *This)
{
   StringC	name = *((*list)[0]);
   This->pub->LoadFile(name);
}

/*---------------------------------------------------------------
 *  Function to make sure all newlines are following by whitespace in a
 *     header field
 */

static void
CheckNewlines(StringC& str)
{
   str.Trim();

   int		pos = 0;
   u_int	off = 0;
   while ( (pos=str.PosOf('\n', off)) >= 0 ) {

//
// Remove consecutive newlines
//
      while ( str[pos+1] == '\n' ) str(pos+1,1) = "";

//
// Check for following whitespace
//
      if ( !isspace(str[pos+1]) ) str(pos+1,0) = " ";

      off = pos+1;
   }

} // End CheckNewlines

/*---------------------------------------------------------------
 *  Method to save the contents of this window to a file
 */

Boolean
SendWinP::Save(char *cs)
{
   if ( !cs && !editIcon && !editMsg ) {
      StringC	errmsg("Trying to save with no edit part and no file name");
      errmsg += "\nPlease report this error to Ishmail Support.";
      pub->PopupMessage(errmsg);
      return False;
   }

   pub->BusyCursor(True);
   pub->Message("Saving...");

//
// Save any child windows
//
   u_int	count = editWinList.size();
   int	i;
   for (i=0; i<count; i++) {
      SendWinC	*editWin = (SendWinC *)*editWinList[i];
      if ( editWin->IsShown() && editWin->Changed() ) {
	 if ( !editWin->priv->Save() ) {
	    pub->BusyCursor(False);
	    pub->Message("Composition not saved.");
	    return False;
	 }
      }
   }

   if ( edit_pid ) {
      StringC	errmsg("There is an external edit in progress.\n");
      errmsg += "Please exit the editor before saving this message.";
      pub->PopupMessage(errmsg);
      pub->BusyCursor(False);
      pub->Message("Composition not saved.");
      return False;
   }

   if ( spell_pid ) {
      StringC	errmsg("There is an external spell-check in progress.\n");
      errmsg += "Please exit the spell checker before saving this message.";
      pub->PopupMessage(errmsg);
      pub->BusyCursor(False);
      pub->Message("Composition not saved.");
      return False;
   }

//
// Read the headers from the address fields
//
   StringListC	headList;
   StringListC	contList;
   StringC	headStr;
   if ( !pub->IsEditOnly() ) {

      StringC	to, sub, cc, bcc, fcc, other;
       toText->GetString(to,  TT_PLAIN);
      subText->GetString(sub, TT_PLAIN);
      if ( pub->ccVis    )    ccText->GetString(cc,    TT_PLAIN);
      if ( pub->bccVis   )   bccText->GetString(bcc,   TT_PLAIN);
      if ( pub->fccVis   )   fccText->GetString(fcc,   TT_PLAIN);
      if ( pub->otherVis ) otherText->GetString(other, TT_PLAIN);

//
// Make sure newlines are followed by whitespace
//
      CheckNewlines(to);
      CheckNewlines(cc);
      CheckNewlines(bcc);
      CheckNewlines(fcc);

//
// Add headers to list
//
      if ( to.size() > 0 ) {
	 headStr = "To: ";
	 headStr += to;
	 headList.add(headStr);
      }
      if ( sub.size() > 0 ) {
	 headStr = "Subject: ";
	 headStr += sub;
	 headList.add(headStr);
      }
      if ( cc.size() > 0 ) {
	 headStr = "Cc: ";
	 headStr += cc;
	 headList.add(headStr);
      }
      if ( bcc.size() > 0 ) {
	 headStr = "Bcc: ";
	 headStr += bcc;
	 headList.add(headStr);
      }
      if ( fcc.size() > 0 ) {
	 headStr = "Fcc: ";
	 headStr += fcc;
	 headList.add(headStr);
      }

      if ( other.size() > 0 ) {
	 HeaderC	*head = ExtractHeaders(other);
	 while ( head ) {
	    headStr = head->full;
	    if ( head->key.StartsWith("Content-", IGNORE_CASE) )
	       contList.add(headStr);
	    else
	       headList.add(headStr);
	    head = head->next;
	 }
      }

   } // End if saving header fields

//
// Write the message body to a file.  This may also generate additional
//    headers.
//
   char	*bf = tempnam(NULL, "body.");
   StringC	bodyFile = bf;
   free(bf);

   OutgoingMailTypeT	mailType;
   if ( !WriteBody(bodyFile, headList, contList, &mailType, /*saving=*/True) ) {
      unlink(bodyFile);
      pub->BusyCursor(False);
      pub->Message("Composition not saved.");
      return False;
   }

//
// Create the final save file
//
   StringC	saveFile;
   if ( cs )
      saveFile = cs;

   else {
      cs = tempnam(NULL, "save.");
      saveFile = cs;
      free(cs);
      cs = NULL;
   }

//
// Open the save file
//
   FILE	*fp = fopen(saveFile, "a");
   if ( !fp ) {

      StringC	errmsg("Could not open file \"");
      errmsg += saveFile;
      errmsg += "\".\n";
      errmsg += SystemErrorMessage(errno);
      pub->PopupMessage(errmsg);

      unlink(bodyFile);
      pub->BusyCursor(False);
      pub->Message("Composition not saved.");
      return False;
   }

//
// Merge lists
//
   count = contList.size();
   for (i=0; i<count; i++)
      AddHeader(*contList[i], headList);

//
// Write the headers to the save file
//
   count = headList.size();
   Boolean	error = False;
   for (i=0; !error && i<count; i++) {
      headStr = *headList[i];
      if ( !headStr.EndsWith('\n') ) headStr += '\n';
      error = !headStr.WriteFile(fp);
   }

//
// Write a blank line
//
   if ( !error ) {
      headStr = "\n";
      error = !headStr.WriteFile(fp);
   }

//
// Copy the body file
//
   if ( !error ) error = !CopyFile(bodyFile, saveFile, False, False, NULL, fp);

   if ( error ) {

      StringC	errmsg("Error writing to save file \"");
      errmsg += saveFile;
      errmsg += "\".\n";
      errmsg += SystemErrorMessage(errno);
      pub->PopupMessage(errmsg);

      fclose(fp);
      unlink(bodyFile);
      pub->BusyCursor(False);
      pub->Message("Composition not saved.");
      return False;
   }

   fclose(fp);
   unlink(bodyFile);

//
//  If we created the save file, link it to the icon
//
   if ( !cs ) {
      if ( editIcon ) editIcon->SetSourceFile(saveFile, /*delete=*/True);
      else if ( editMsg ) editDoneCall((char*)saveFile);
      pub->changed = False;
   }

   pub->BusyCursor(False);

   pub->Message("Save complete.");
   return True;

} // End Save

/*---------------------------------------------------------------
 *  Callback to handle external editor
 */

void
SendWinP::DoEdit(Widget, SendWinP *This, XtPointer)
{
   This->edit_pid = This->FilterBody(ishApp->compPrefs->editCmd,
				     (CallbackFn*)EditFinished);
}

/*---------------------------------------------------------------
 *  Callback to handle spell checker
 */

void
SendWinP::DoSpell(Widget, SendWinP *This, XtPointer)
{
   This->spell_pid = This->FilterBody(ishApp->compPrefs->spellCmd,
				      (CallbackFn*)SpellFinished);
}

/*---------------------------------------------------------------
 *  Method to pass body text to a filter and read the result
 */

pid_t
SendWinP::FilterBody(StringC& cmd, CallbackFn *doneFn)
{
   pub->BusyCursor(True);

//
// Create a temporary edit file
//
   char	*file = tempnam(NULL, "filt.");

//
// Set up a description template so that graphics will generate a description
//    string that we can later use to re-insert the graphic in the modified
//    text.
//
   descTemplate = " (GRAPHIC %a - DO NOT MODIFY) ";

//
// Display the text
//
   StringC	textStr;
   Boolean	plain;
   if ( optMsgPlainTB ) plain = (XmToggleButtonGetState(optMsgPlainTB) ||
				 XmToggleButtonGetState(optTextPlainTB));
   else			plain = (ishApp->mailPrefs->mailType == MAIL_PLAIN ||
				 ishApp->mailPrefs->textType == CT_PLAIN);
   if ( plain ) {
      bodyText->GetString(textStr, TT_PLAIN, XmToggleButtonGetState(optWrapTB)
      					     ? bodyText->ColumnCount() : 0);
      filterEnriched = False;
   }
   else {
      bodyText->GetString(textStr, TT_ENRICHED);
      filterEnriched = True;
   }
   if ( !textStr.EndsWith("\n") ) textStr += "\n";

//
// Go back to the normal way of generating descriptions
//
   descTemplate.Clear();

   if ( !textStr.WriteFile(file) ) {

      StringC	msg("Couldn't write temp file: ");
      msg += file;
      pub->PopupMessage(msg);
      pub->BusyCursor(False);
      free(file);
      return 0;
   }

//
// Make the text widget insensitive during the edit
//
   bodyText->SetSensitive(False);
   XtSetSensitive(pub->FilePulldown(), False);
   XtSetSensitive(editPD,	       False);
   buttMgr->EnableButtons();

//
// Start the edit process
//
   CallbackC	modCall ((CallbackFn *)FileModified, this);
   CallbackC	doneCall(doneFn,		     this);
   pid_t	pid = FilterFile(cmd, file, modCall, doneCall);
   if ( debuglev > 0 ) cout <<"Started process " <<pid NL;
   free(file);

   if ( pid <= 0 ) {
      bodyText->SetSensitive(True);
      XtSetSensitive(pub->FilePulldown(),  True);
      XtSetSensitive(editPD,		   True);
      buttMgr->EnableButtons();
   }

   pub->BusyCursor(False);
   return pid;

} // End FilterBody

/*---------------------------------------------------------------
 *  Callback routine to handle completion of edit
 */

void
SendWinP::EditFinished(char *file, SendWinP *This)
{
   if ( debuglev > 0 ) cout <<"Edit finished" <<endl;

//
// Make the text widget sensitive again
//
   This->bodyText->SetSensitive(True);
   XtSetSensitive(This->pub->FilePulldown(),  True);
   XtSetSensitive(This->editPD,		      True);
   This->buttMgr->EnableButtons();
   XmProcessTraversal(This->bodyText->TextArea(), XmTRAVERSE_CURRENT);

//
// Delete the temporary file
//
   unlink(file);
   This->edit_pid = 0;
}

/*---------------------------------------------------------------
 *  Callback routine to handle completion of spellcheck
 */

void
SendWinP::SpellFinished(char *file, SendWinP *This)
{
   if ( debuglev > 0 ) cout <<"Spellcheck finished" <<endl;

//
// Make the text widget sensitive again
//
   This->bodyText->SetSensitive(True);
   XtSetSensitive(This->pub->FilePulldown(),  True);
   XtSetSensitive(This->editPD,		      True);
   This->buttMgr->EnableButtons();
   XmProcessTraversal(This->bodyText->TextArea(), XmTRAVERSE_CURRENT);

//
// Delete the temporary file
//
   unlink(file);
   This->spell_pid = 0;
}

/*---------------------------------------------------------------
 *  Callback routine to handle modification of edited file
 */

void
SendWinP::FileModified(char *file, SendWinP *This)
{
   if ( debuglev > 0 ) cout <<"File " <<file <<" was modified" <<endl;

//
// Clear the text and read in the file
//
   MappedFileC	*mf = MapFile(file);
   if ( mf ) {
      This->ReadFilter(mf->data);
      UnmapFile(mf);
   }

   else {

      StringC	tmpStr;
      if ( !tmpStr.ReadFile(file) ) {
	 StringC	msg("Could not read file: ");
	 msg += file;
	 This->pub->PopupMessage(msg);
	 return;
      }

      This->ReadFilter(tmpStr);

   } // End if file not mapped

} // End FileModified

/*---------------------------------------------------------------
 *  Method to read in the changes made during and external edit or spell
 *     session.
 */

void
SendWinP::ReadFilter(CharC data)
{
   bodyText->Defer(True);

   if ( filterEnriched ) bodyText->SetTextType(TT_ENRICHED);
   else		         bodyText->SetTextType(TT_PLAIN);

//
// Loop through and save any existing graphics.  Also mark them so they
//    won't be deleted
//
   PtrListC	graphicList = bodyText->GraphicList();
   u_int	count       = graphicList.size();
   int	i;
   for (i=0; i<count; i++) {
      SendIconC	*icon = (SendIconC*)*graphicList[i];
      icon->okToDelete = False;
   }

//
// Clear the composition window
//
   bodyText->Clear();

//
// Scan the text for any graphic tags
//
   static RegexC	*pat = NULL;
   if ( !pat ) pat =
      new RegexC("(GRAPHIC[ \n]\\([0-9]+\\)[ \n]-[ \n]DO[ \n]NOT[ \n]MODIFY)");

   int		pos;
   int		start = 0;
   u_int	len;
   StringC	addrStr;
   while ( (pos=pat->search(data, start)) >= 0 ) {

//
// Display the text between "start" and "pos"
//
      len = pos-start;
      if ( len > 0 ) bodyText->AddString(data(start, len));

//
// Look for a graphic with the specified tag
//
      addrStr = data((*pat)[1]);
      void	*tmp = (void*)atoi(addrStr);

      int	index = graphicList.indexOf(tmp);
      if ( index == graphicList.NULL_INDEX ) {
	 StringC	errmsg("The graphic attachment: ");
	 errmsg += (int)tmp;
	 errmsg += " could not be found.\n";
	 errmsg += "Perhaps the address was modified by accident.";
	 pub->PopupMessage(errmsg);
      }

//
// If the graphic was found, add it to the composition window and remove it
//    from the list.
//
      else {

	 SendIconC	*icon = (SendIconC*)tmp;
	 icon->okToDelete = True;
	 bodyText->AddGraphic(icon);
	 graphicList.remove(index);

      } // End if graphic was found

      start = pos + (*pat)[0].length();

   } // End for each tag

//
// Display the text between "start" and the end
//
   len = data.Length() - start;
   if ( len > 0 ) bodyText->AddString(data(start, len));

   bodyText->Defer(False);

//
// If there are any graphics remaining in the list, delete them
//
   if ( graphicList.size() == 1 ) {

      SendIconC	*icon = (SendIconC*)*graphicList[0];
      StringC	label;
      icon->data->GetLabel(label);

      StringC	errmsg("The graphic attachment: ");
      errmsg += label;
      errmsg += "\nis missing from the modified text and will be deleted.\n";
      pub->PopupMessage(errmsg);

      delete icon;
   }

   else if ( graphicList.size() > 1 ) {

      SendIconC	*icon;
      StringC	label;

      StringC	errmsg("The following graphic attachments are missing from");
      errmsg += "\nthe modified text and will be deleted:\n\n";

      u_int	count = graphicList.size();
      for (i=0; i<count; i++) {
	 icon = (SendIconC*)*graphicList[i];
	 icon->data->GetLabel(label);
	 errmsg += label;
	 errmsg += '\n';
      }
      pub->PopupMessage(errmsg);

      count = graphicList.size();
      for (i=0; i<count; i++) {
	 icon = (SendIconC*)*graphicList[i];
	 delete icon;
      }
   }

} // End ReadFilter

#ifdef HGLTERM
/*---------------------------------------------------------------
 *  Method to find or create a new terminal window
 */

TermWinC*
SendWinP::GetTermWin()
{
//
// First look for an existing, unused window
//
   TermWinC	*termWin = NULL;
   unsigned	count = termWinList.size();
   Boolean	found = False;
   for (int i=0; !found && i<count; i++) {

      termWin = (TermWinC*)*termWinList[i];
      if ( !termWin->IsShown() ) found = True;

   } // End for each existing terminal window

//
// If we didn't find a window, we need to create a new one
//
   if ( !found ) {

      pub->BusyCursor(True);

      termWin = new TermWinC("termWin", *pub);
      void	*tmp = (void*)termWin;
      termWinList.add(tmp);

      pub->BusyCursor(False);
   }

   return termWin;

} // End GetTermWin
#endif

