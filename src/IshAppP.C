/*
 *  $Id: IshAppP.C,v 1.9 2000/08/07 11:05:16 evgeny Exp $
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
#include "IshAppP.h"
#include "IshAppC.h"
#include "SigPrefC.h"
#include "MailPrefC.h"
#include "MailFile.h"
#include "AlertPrefC.h"
#include "AppPrefC.h"
#include "FolderC.h"
#include "FolderPrefC.h"
#include "MainWinC.h"
#include "Query.h"
#include "ReadWinC.h"
#include "SendWinC.h"
#include "CompPrefC.h"

#include "ishmail.xpm"
#include "ishmail_info.xpm"
#include "ishmail_question.xpm"
#include "ishmail_warning.xpm"
#include "ishmail_error.xpm"

#include <hgl/CharC.h>
#include <hgl/StringC.h>
#include <hgl/StringListC.h>
#include <hgl/PixmapC.h>
#include <hgl/WXmString.h>
#include <hgl/WArgList.h>
#include <hgl/RowColC.h>
#include <hgl/SysErr.h>
#include <hgl/TextMisc.h>
#include <hgl/rsrc.h>
#include <hgl/VBoxC.h>

#include <Xm/MessageB.h>
#include <Xm/SelectioB.h>
#include <Xm/Label.h>
#include <Xm/Protocols.h>
#include <Xm/TextF.h>
#include <Xm/RowColumn.h>
#include <Xm/Frame.h>
#include <Xm/ToggleB.h>
#include <Xm/AtomMgr.h>
#include <Xm/PushB.h>

#include <unistd.h>
#include <errno.h>


/*----------------------------------------------------------------------
 * Constructor
 */

IshAppP::IshAppP(IshAppC *ia)
{
   pub              = ia;

   messagePM        = NULL;
   warningPM        = NULL;
   questionPM       = NULL;
   errorPM          = NULL;
   checkTimer       = (XtIntervalId)NULL;
   lastMailCheck    = 0;

} // End constructor

/*----------------------------------------------------------------------
 * Destructor
 */

IshAppP::~IshAppP()
{
   delete messagePM;
   delete warningPM;
   delete questionPM;
   delete errorPM;
}

/*---------------------------------------------------------------
 * See if "Ishmail" needs to be added to beginning of resources.  Prior to
 *    version 1.1 we thought we didn't need it, but it looks like we really
 *    do.
 * Also check for duplicate lines.  If there are any, keep the last.
 */  

void
IshAppP::FixIshmailrc()
{
   StringC	data;
   if ( !data.ReadFile(pub->resFile) ) return;

//
// See if we need to add the "Ishmail*" prefix
//
   Boolean	addPrefix  = !data.Contains("\nIshmail*");

//
// See if we need to fix the "Ishmail**" prefix
//
   Boolean	fixPrefix  = data.Contains("\nIshmail**");

//
// See if we need to remove width and height resources
//
   Boolean	fixSize = (data.Contains("mainShell.width:")	||
			   data.Contains("mainShell.height:")	||
			   data.Contains("viewDA.width:")	||
			   data.Contains("viewDA.height:")	||
			   data.Contains("sendWin.width:")	||
			   data.Contains("sendWin.height:")	||
			   data.Contains("readWin.width:")	||
			   data.Contains("readWin.height:")	||
			   data.Contains("msgBox*scrollForm.height:")	||
			   data.Contains("msgBox*taskVBox.height:"));

//
// See if we need to change from font to fontList in the viewDA
//
   Boolean	fixFont  = !data.Contains("viewDA.fontList:");

//
// See if we need to remove duplicate lines
//
   Boolean	removeDups = HasDuplicateLines(data);

   if ( !addPrefix && !fixPrefix && !fixSize && !removeDups && !fixFont )
      return;

//
// Break the resources into lines
//
   StringListC	lineList;
   GetResLines(data, lineList, removeDups);

//
// Remove all .width and .height resources for main windows
//
   if ( fixSize ) {

      if ( debuglev > 0 )
	 cout <<"Removing bogus width and height resources." <<endl;

      u_int	count = lineList.size();
      int i=count-1; for (i=count-1; i>=0; i--) {
	 StringC	*line = lineList[i];
	 if ( (line->Contains(".width:") || line->Contains(".height:")) &&
	      (line->Contains("mainShell.width:")	||
	       line->Contains("mainShell.height:")	||
	       line->Contains("viewDA.width:")		||
	       line->Contains("viewDA.height:")		||
	       line->Contains("sendWin.width:")		||
	       line->Contains("sendWin.height:")	||
	       line->Contains("readWin.width:")		||
	       line->Contains("readWin.height:")	||
	       line->Contains("msgBox*scrollForm.height:")	||
	       line->Contains("msgBox*taskVBox.height:")) ) {
	    lineList.remove(i);
	 }
      }
   }

   if ( addPrefix ) {

      if ( debuglev > 0 ) cout <<"Adding Ishmail* prefix" <<endl;

//
// Add "Ishmail" to the beginning of any lines that don't have it
//
      u_int	count = lineList.size();
      int i=0; for (i=0; i<count; i++) {

	 StringC	*line = lineList[i];

//
// See if the line needs to be updated
//
	 if ( line->StartsWith("*") )
	    (*line)(0,0) = "Ishmail";
      }

   } // End if prefix needed

   if ( fixPrefix ) {

      if ( debuglev > 0 ) cout <<"Adding Ishmail* prefix" <<endl;

//
// Change "Ishmail**" to "Ishmail*"
//
      u_int	count = lineList.size();
      int i=0; for (i=0; i<count; i++) {

	 StringC	*line = lineList[i];

//
// See if the line needs to be updated
//
	 if ( line->StartsWith("Ishmail**") )
	    (*line)(7,1) = "";
      }

   } // End if prefix fix needed

   if ( fixFont ) {

      if ( debuglev > 0 )
	 cout <<"Changing viewDA.font to viewDA.fontList" <<endl;

//
// Change "viewDA.font:" to "viewDA.fontList:"
//
      u_int	count = lineList.size();
      int i=0; for (i=0; i<count; i++) {

	 StringC	*line = lineList[i];

//
// See if the line needs to be updated
//
	 if ( line->StartsWith("Ishmail*viewDA.font:") )
	    (*line)(19,0) = "List";
      }

   } // End if font fix needed

//
// Write out the resources
//
   WriteResFile(lineList);

} // End FixIshmailrc

/*---------------------------------------------------------------
 * See if mapped resource file contains any duplicate lines
 */  

Boolean
IshAppP::HasDuplicateLines(CharC data)
{
//
// Loop through lines
//
   u_int	offset = 0;
   CharC	line = data.NextWord(offset, "\n");
   while ( line.Length() > 0 ) {

      offset = line.Addr() - data.Addr() + line.Length();
      if ( line.StartsWith('*') || line.StartsWith("Ishmail*") ) {

//
// Remove the value portion of the resource 
//
	 int	pos = line.PosOf(':');
	 if ( pos >= 0 ) {

	    line = line(0,pos+1);

//
// Now look from here to the end to see if there is any copy of this line
//
	    pos = data.PosOf(line, offset);
	    if ( pos > 0 && data[pos-1] == '\n' )
	       return True;
	 }
      }

//
// Read the next line
//
      line = data.NextWord(offset, "\n");

   } // End for each line in file
   
   return False;

} // End HasDuplicateLines

/*---------------------------------------------------------------
 *  Function to read the resource lines from the given text data
 */

void
IshAppP::GetResLines(CharC data, StringListC& lineList, Boolean removeDups)
{
   if ( debuglev > 0 && removeDups ) cout <<"Removing duplicate lines" <<endl;

   lineList.removeAll();
   lineList.AllowDuplicates(TRUE);

   StringC	line;
   CharC	word;
   u_int	offset = 0;
   int		pos = data.PosOf('\n', offset);
   while ( pos >= 0 ) {

      word = data(offset, (u_int)pos-offset);
      offset = pos + 1;

//
// If the newline is escaped, read some more.
//
      if ( word.EndsWith("\\") ) {
	 word.CutEnd(1);
	 line += word;
      }
      else {
	 line += word;
	 if ( removeDups ) {
	    StringC	*lineP = FindLine(lineList, line);
	    if ( lineP ) *lineP = line;
	    else	 lineList.add(line);
	 }
	 else
	    lineList.add(line);
	 line.Clear();
      }

      pos = data.PosOf('\n', offset);

   } // End for each line in file

} // End GetResLines

/*---------------------------------------------------------------
 *  Function to see if the specified line already exists in the line list
 */

StringC*
IshAppP::FindLine(StringListC& list, CharC res)
{
   if ( !res.StartsWith('*') && !res.StartsWith("Ishmail*") ) return NULL;

//
// Remove the value portion of the resource 
//
   int	pos = res.PosOf(':');
   if ( pos < 0 ) return NULL;

   res = res(0,pos+1);

//
// Loop through the line list and see if any line starts with this resource
//
   u_int	count = list.size();
   int i=0; for (i=0; i<count; i++) {
      StringC	*line = list[i];
      if ( line->StartsWith(res) ) return line;
   }

   return NULL;

} // End FindLine

/*---------------------------------------------------------------
 *  Function to write the .ishmailrc file from a list of lines
 */

Boolean
IshAppP::WriteResFile(StringListC& lineList)
{
   if ( debuglev > 0 ) cout <<"Writing resource file: " <<pub->resFile <<endl;

//
// Write the information back to the file
//
   FILE *fp = fopen(pub->resFile, "w");
   if ( !fp ) {
      StringC	errmsg = "Could not open resource file: ";
      errmsg += pub->resFile;
      errmsg += '\n';
      errmsg += SystemErrorMessage(errno);
      halApp->PopupMessage(errmsg);
      return False;
   }

   CharC	nl("\n");
   unsigned	count = lineList.size();
   Boolean	error = False;
   int i=0; for (i=0; !error && i<count; i++) {

      StringC	*line = lineList[i];

//
// See if the line needs to be split
//
      if ( line->StartsWith("Ishmail*alerts:")		||
	   line->StartsWith("Ishmail*icons:")		||
	   line->StartsWith("Ishmail*saveRules:")	||
	   line->StartsWith("Ishmail*folderSortKeys:")	||
	   line->StartsWith("Ishmail*automaticFilingRules:") )
	 error = !WriteRules(fp, *line);
      else
	 error = !line->WriteFile(fp);

      if ( !error ) error = !nl.WriteFile(fp);
   }

   if ( !error ) error = (fclose(fp) != 0);

   if ( error ) {
      StringC	errmsg = "Could not write resource file: ";
      errmsg += pub->resFile;
      errmsg += '\n';
      errmsg += SystemErrorMessage(errno);
      halApp->PopupMessage(errmsg);
   }

   return !error;

} // End WriteResFile

/*---------------------------------------------------------------
 *  Write a resource that represents a rule dictionary and add line breaks.
 */

Boolean
IshAppP::WriteRules(FILE *fp, StringC& line)
{
//
// Add a newline after the first : and then after each successive ;
//
   char		*start = line;
   char		*end   = start + line.size();
   char		*cp    = strchr(start, ':');
   Boolean	error  = False;
   while ( !error && cp ) {

//
// Skip found character and following whitespace
//
      cp++;
      while ( cp < end && isspace(*cp) ) cp++;
      int	len = cp - start;

//
// Write up to this point, then write an escaped newline if there's more
//    text to come
//
      error = (fwrite(start, 1, len, fp) != len);
      if ( !error && cp < end ) error = (fwrite("\\\n", 1, 2, fp) != 2);

      start = cp;
      cp = strchr(start, ';');

   } // End for each ';'

   return !error;

} // End WriteRules

/*---------------------------------------------------------------
 *  Create ishmail pixmaps
 */

void
IshAppP::CreatePixmaps()
{
   Window	appWin = XtWindow(pub->appShell);
   Screen	*scr   = pub->screen;
   Pixel	black = BlackPixelOfScreen(scr);
   Pixel	white = WhitePixelOfScreen(scr);

   pub->ishmailPM = new PixmapC(ishmail_xpm, appWin);
   if ( !pub->ishmailPM->reg )
      pub->ishmailPM->Set("ishmail.xbm", black, white, white, black, scr,
      			  appWin);
   if ( pub->ishmailPM->reg == XmUNSPECIFIED_PIXMAP )
      pub->ishmailPM->reg = (Pixmap)NULL;

   messagePM = new PixmapC(ishmail_info_xpm, appWin);
   if ( !messagePM->reg )
      messagePM->Set("ishmail_info.xbm", black, white, white, black, scr,
		     appWin);
   if ( messagePM->reg == XmUNSPECIFIED_PIXMAP ) messagePM->reg = (Pixmap)NULL;

   questionPM = new PixmapC(ishmail_question_xpm, appWin);
   if ( !questionPM->reg )
      questionPM->Set("ishmail_question.xbm", black, white, white, black, scr,
		      appWin);
   if ( questionPM->reg == XmUNSPECIFIED_PIXMAP )
      questionPM->reg = (Pixmap)NULL;

   warningPM = new PixmapC(ishmail_warning_xpm, appWin);
   if ( !warningPM->reg )
      warningPM->Set("ishmail_warning.xbm", black, white, white, black, scr,
		     appWin);
   if ( warningPM->reg == XmUNSPECIFIED_PIXMAP ) warningPM->reg = (Pixmap)NULL;

   errorPM = new PixmapC(ishmail_error_xpm, appWin);
   if ( !errorPM->reg )
      errorPM->Set("ishmail_error.xbm", black, white, white, black, scr,
		   appWin);
   if ( errorPM->reg == XmUNSPECIFIED_PIXMAP ) errorPM->reg = (Pixmap)NULL;

} // End CreatePixmaps


/*---------------------------------------------------------------
 *  Timer proc to check for new mail
 */

void
IshAppP::CheckForNewMail(IshAppP *This, XtIntervalId*)
{
//
// Do this to protect against multiple threads.  This can happen if we
//    detect an external mod, are waiting for user acknowledgement and
//    VerifyMailCheck thinks the timer proc has gone south.
//
   static Boolean	inCheck = False;

   if ( inCheck ) return;
   inCheck = True;

   This->lastMailCheck = time(0);
   This->checkTimer    = (XtIntervalId)NULL;

//
// Check system folder
//
   if ( This->pub->systemFolder && This->pub->systemFolder->NewMail() )
      This->pub->mainWin->GetNewMail(This->pub->systemFolder);

//
// Check all folders if showing status
//
    if ( ishApp->folderPrefs->showStatus ) {
	FolderListC	list  = ishApp->folderPrefs->OpenFolders();
	u_int		count = list.size();
	for (int i=0; i<count; i++) {
	    FolderC	*folder = list[i];
	    if ( folder->NewMail() ) This->pub->mainWin->GetNewMail(folder);
	}
    }

//
// Check current folder if other than system
//
   else if ( This->pub->mainWin->curFolder != This->pub->systemFolder &&
	     This->pub->mainWin->curFolder->NewMail() )
      This->pub->mainWin->GetNewMail(This->pub->mainWin->curFolder);

   inCheck = False;

//
// Start another timer
//
   if ( This->pub->appPrefs->checkInterval > 0 )
      This->checkTimer =
	 XtAppAddTimeOut(This->pub->context,
	 		 This->pub->appPrefs->checkInterval*1000,
			 (XtTimerCallbackProc)CheckForNewMail,
			 (XtPointer)This);

} // End CheckForNewMail

/*---------------------------------------------------------------
 *  Method to re-open a dialog if the user closes it with the window
 *     manager close button
 */

void
IshAppP::PreventClose(Widget shell, Widget dialog, XtPointer)
{
   XtManageChild(dialog);
   XMapRaised(halApp->display, XtWindow(shell));
}

/*---------------------------------------------------------------
 *  Method to ask the user if they will be using an IMAP server
 */

void
IshAppP::QueryImap()
{
//
// See if this information is available in the in-box name
//
   if ( pub->appPrefs->inBox.StartsWith('{') ) {

      int	pos = pub->appPrefs->inBox.PosOf('}');
      if ( pos > 0 ) {
	 StringC	server = pub->appPrefs->inBox(1,pos-1);
	 pub->appPrefs->usingImap  = True;
	 pub->appPrefs->imapServer = server;
	 pub->appPrefs->WriteDatabase();
	 pub->appPrefs->WriteFile();
	 return;
      }
   }

   QueryAnswerT  answer;

//
// Ask if they will be using an IMAP server
//
   WArgList	args;
   args.DialogStyle(XmDIALOG_FULL_APPLICATION_MODAL);
   if ( halApp->questionPM ) args.SymbolPixmap(halApp->questionPM);
   Widget	dialog = XmCreateQuestionDialog(*pub, "queryImapWin", ARGS);

   XtAddCallback(dialog, XmNokCallback, (XtCallbackProc)AnswerQuery,
			 (XtPointer)&answer);
   XtAddCallback(dialog, XmNcancelCallback, (XtCallbackProc)AnswerQuery,
			 (XtPointer)&answer);
   XtAddCallback(dialog, XmNhelpCallback, (XtCallbackProc)HalAppC::DoHelp,
			 (char *) "helpcard");

//
// Don't allow window manager close function
//
   XmAddWMProtocolCallback(XtParent(dialog), halApp->delWinAtom,
			   (XtCallbackProc)PreventClose, dialog);

   XtManageChild(dialog);
   XMapRaised(halApp->display, XtWindow(XtParent(dialog)));

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

//
// Delete the dialog
//
   XtDestroyWidget(dialog);

   if ( answer == QUERY_YES ) {

//
// Get the name of the IMAP server
//
      args.Reset();
      args.DialogStyle(XmDIALOG_FULL_APPLICATION_MODAL);
      args.AutoUnmanage(False);
      dialog = XmCreatePromptDialog(*pub, "queryImapServerWin", ARGS);

      XtAddCallback(dialog, XmNokCallback, (XtCallbackProc)AnswerQuery,
			    (XtPointer)&answer);
      XtAddCallback(dialog, XmNcancelCallback, (XtCallbackProc)AnswerQuery,
			    (XtPointer)&answer);
      XtAddCallback(dialog, XmNhelpCallback, (XtCallbackProc)HalAppC::DoHelp,
			    (char *) "helpcard");

      Widget	serverTF = XmSelectionBoxGetChild(dialog, XmDIALOG_TEXT);

//
// Don't allow window manager close function
//
      XmAddWMProtocolCallback(XtParent(dialog), halApp->delWinAtom,
			      (XtCallbackProc)PreventClose, dialog);

      XtManageChild(dialog);
      XMapRaised(halApp->display, XtWindow(XtParent(dialog)));

//
// Loop until a name is entered or the dialog is cancelled
//
      Boolean	done = False;
      while ( !done ) {

//
// Simulate the main event loop and wait for the answer
//
	 answer = QUERY_NONE;
	 while ( answer == QUERY_NONE ) {
	    XtAppProcessEvent(halApp->context, XtIMXEvent);
	    XSync(halApp->display, False);
	 }

	 if ( answer == QUERY_CANCEL ) {
	    pub->appPrefs->usingImap = False;
	    done = True;
	 }

	 else {
	    char	*cs = XmTextFieldGetString(serverTF);
	    CharC	server(cs);
	    server.Trim();
	    if ( server.Length() == 0 ) {
	       set_invalid(serverTF, True, True);
	       pub->PopupMessage("Please enter the host name of your IMAP server.");
	    }
	    else {
	       pub->appPrefs->usingImap  = True;
	       pub->appPrefs->imapServer = cs;
	       done = True;
	    }
	    XtFree(cs);
	 }

      } // End while not done

      XtUnmanageChild(dialog);
      XSync(halApp->display, False);
      XmUpdateDisplay(dialog);

//
// Delete the dialog
//
      XtDestroyWidget(dialog);

   } // End if using an imap server

   else {	// Not using an IMAP server
      pub->appPrefs->usingImap = False;
   }

   pub->appPrefs->WriteDatabase();
   pub->appPrefs->WriteFile();

} // End QueryImap

/*---------------------------------------------------------------
 *  Method to ask the user if they will be using a POP server
 */

void
IshAppP::QueryPop()
{
   QueryAnswerT  answer;

//
// Ask if they will be using a POP server
//
   WArgList	args;
   args.DialogStyle(XmDIALOG_FULL_APPLICATION_MODAL);
   if ( halApp->questionPM ) args.SymbolPixmap(halApp->questionPM);
   Widget	dialog = XmCreateQuestionDialog(*pub, "queryPopWin", ARGS);

   XtAddCallback(dialog, XmNokCallback, (XtCallbackProc)AnswerQuery,
			 (XtPointer)&answer);
   XtAddCallback(dialog, XmNcancelCallback, (XtCallbackProc)AnswerQuery,
			 (XtPointer)&answer);
   XtAddCallback(dialog, XmNhelpCallback, (XtCallbackProc)HalAppC::DoHelp,
			 (char *) "helpcard");

//
// Don't allow window manager close function
//
   XmAddWMProtocolCallback(XtParent(dialog), halApp->delWinAtom,
			   (XtCallbackProc)PreventClose, dialog);

   XtManageChild(dialog);
   XMapRaised(halApp->display, XtWindow(XtParent(dialog)));

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

//
// Delete the dialog
//
   XtDestroyWidget(dialog);

   if ( answer == QUERY_YES ) {

//
// Get the name and type of the POP server and the path to the "popclient"
//   command
//
      args.Reset();
      args.DialogStyle(XmDIALOG_FULL_APPLICATION_MODAL);
      args.AutoUnmanage(False);
      args.Add(XmNchildPlacement, XmPLACE_BELOW_SELECTION);
      dialog = XmCreatePromptDialog(*pub, "queryPopServerWin", ARGS);

      XtAddCallback(dialog, XmNokCallback, (XtCallbackProc)AnswerQuery,
			    (XtPointer)&answer);
      XtAddCallback(dialog, XmNcancelCallback, (XtCallbackProc)AnswerQuery,
			    (XtPointer)&answer);
      XtAddCallback(dialog, XmNhelpCallback, (XtCallbackProc)HalAppC::DoHelp,
			    (char *) "helpcard");

      Widget	serverTF = XmSelectionBoxGetChild(dialog, XmDIALOG_TEXT);

//
// Create a form for other choices
//
      args.Reset();
      args.Orientation(XmVERTICAL);
      args.Packing(XmPACK_TIGHT);
      Widget	popRC = XmCreateRowColumn(dialog, "popRC", ARGS);

//
// Add the POP2/POP3 choice buttons
//
      Widget	popTypeLabel = XmCreateLabel(popRC, "popTypeLabel", 0,0);
      Widget	popTypeFrame = XmCreateFrame(popRC, "popTypeFrame", 0,0);

      args.Reset();
      args.Orientation(XmHORIZONTAL);
      args.Packing(XmPACK_TIGHT);
      Widget	popTypeRadio = XmCreateRadioBox(popTypeFrame, "popTypeRadio",
      						ARGS);

      Widget popType2TB = XmCreateToggleButton(popTypeRadio, "popType2TB", 0,0);
      Widget popType3TB = XmCreateToggleButton(popTypeRadio, "popType3TB", 0,0);
      XtManageChild(popType2TB);
      XtManageChild(popType3TB);
      XtManageChild(popTypeRadio);

//
// Add the popclient command name
//
      Widget	popCmdLabel = XmCreateLabel    (popRC, "popCmdLabel", 0,0);
      Widget	popCmdTF    = XmCreateTextField(popRC, "popCmdTF",    0,0);

      XmTextFieldSetString(popCmdTF, pub->appPrefs->popclientCmd);

      XtManageChild(popTypeLabel);
      XtManageChild(popTypeFrame);
      XtManageChild(popCmdLabel);
      XtManageChild(popCmdTF);

      XtManageChild(popRC);

//
// Don't allow window manager close function
//
      XmAddWMProtocolCallback(XtParent(dialog), halApp->delWinAtom,
			      (XtCallbackProc)PreventClose, dialog);

      XtManageChild(dialog);
      XMapRaised(halApp->display, XtWindow(XtParent(dialog)));

//
// Loop until a name is entered or the dialog is cancelled
//
      Boolean	done = False;
      while ( !done ) {

//
// Simulate the main event loop and wait for the answer
//
	 answer = QUERY_NONE;
	 while ( answer == QUERY_NONE ) {
	    XtAppProcessEvent(halApp->context, XtIMXEvent);
	    XSync(halApp->display, False);
	 }

	 if ( answer == QUERY_CANCEL ) {
	    pub->appPrefs->usingPop = False;
	    done = True;
	 }

	 else {

	    char	*cs = XmTextFieldGetString(serverTF);
	    CharC	server(cs);
	    server.Trim();

	    if ( server.Length() == 0 ) {
	       set_invalid(serverTF, True, True);
	       pub->PopupMessage("Please enter the host name of your POP server.");
	    }
	    else {

	       pub->appPrefs->usingPop  = True;
	       pub->appPrefs->popServer = cs;
	       XtFree(cs);

	       cs = XmTextFieldGetString(popCmdTF);
	       CharC	cmd(cs);
	       cmd.Trim();
	       if ( cmd.Length() > 0 )
		  pub->appPrefs->popclientCmd = cmd;
	       else
		  pub->appPrefs->popclientCmd = "popclient -3";

	       if ( XmToggleButtonGetState(popType2TB) )
		  pub->appPrefs->popclientCmd.Replace(" -3", " -2");
	       else
		  pub->appPrefs->popclientCmd.Replace(" -2", " -3");

	       done = True;
	    }

	    XtFree(cs);
	 }

      } // End while not done

      XtUnmanageChild(dialog);
      XSync(halApp->display, False);
      XmUpdateDisplay(dialog);

//
// Delete the dialog
//
      XtDestroyWidget(dialog);

   } // End if using a pop server

   else {	// Not using a POP server
      pub->appPrefs->usingPop = False;
   }

   pub->appPrefs->WriteDatabase();
   pub->appPrefs->WriteFile();

} // End QueryPop

/*---------------------------------------------------------------
 *  Callback to auto-deselect a toggle button
 */

static void
AutoDeselect(Widget, Widget dst, XmToggleButtonCallbackStruct *src)
{
   if ( src->set )
      XmToggleButtonSetState(dst, False, False);
}

/*---------------------------------------------------------------
 *  Method to ask the user what folder type(s) they will be using
 */

void
IshAppP::QueryFolderType()
{
   QueryAnswerT  answer;

   WArgList	args;
   args.DialogStyle(XmDIALOG_FULL_APPLICATION_MODAL);
   args.AutoUnmanage(False);
   if ( halApp->questionPM ) args.SymbolPixmap(halApp->questionPM);
   Widget dialog = XmCreateQuestionDialog(*pub, "queryFolderTypeWin", ARGS);

   XtAddCallback(dialog, XmNokCallback, (XtCallbackProc)AnswerQuery,
			 (XtPointer)&answer);
   XtUnmanageChild(XmMessageBoxGetChild(dialog, XmDIALOG_CANCEL_BUTTON));
   XtAddCallback(dialog, XmNhelpCallback, (XtCallbackProc)HalAppC::DoHelp,
			 (char *) "helpcard");

//
// Don't allow window manager close function
//
   XmAddWMProtocolCallback(XtParent(dialog), halApp->delWinAtom,
			   (XtCallbackProc)PreventClose, dialog);

//
// Add toggle buttons for folder type choices
//
   Widget	typeFrame = XmCreateFrame(dialog, "typeFrame", 0,0);

//
// Set up typeRC for 1 row with equal sized columns
//
   RowColC	*typeRC = new RowColC(typeFrame, "typeRC", ARGS);

   typeRC->Defer(True);
   typeRC->SetOrientation(RcCOL_MAJOR);
   typeRC->SetRowCount(1);
   typeRC->SetColAlignment(XmALIGNMENT_CENTER);
   typeRC->SetColWidthAdjust(RcADJUST_NONE);
   typeRC->SetColResize(True);
   typeRC->SetUniformCols(True);

   Widget	unixTB = XmCreateToggleButton(*typeRC, "unixTB", 0,0);
   Widget	mhTB   = XmCreateToggleButton(*typeRC, "mhTB",   0,0);
   Widget	mmdfTB = XmCreateToggleButton(*typeRC, "mmdfTB", 0,0);
   Widget	noneTB = XmCreateToggleButton(*typeRC, "noneTB", 0,0);

   AddValueChanged(noneTB, AutoDeselect, unixTB);
   AddValueChanged(noneTB, AutoDeselect, mhTB);
   AddValueChanged(noneTB, AutoDeselect, mmdfTB);
   AddValueChanged(unixTB, AutoDeselect, noneTB);
   AddValueChanged(mmdfTB, AutoDeselect, noneTB);
   AddValueChanged(mhTB,   AutoDeselect, noneTB);

   typeRC->AddChild(unixTB);
   typeRC->AddChild(mhTB);
   typeRC->AddChild(mmdfTB);
   if ( pub->appPrefs->usingImap ) typeRC->AddChild(noneTB);

   typeRC->Defer(False);

   XtManageChild(*typeRC);
   XtManageChild(typeFrame);

//
// Display the appropriate message
//
   StringC	msgStr;
   if ( pub->appPrefs->usingImap )
      msgStr = get_string(dialog, "imapMessageString",
	     "Which folder type(s) will you be using on your local machine?");
   else
      msgStr = get_string(dialog, "messageString",
				  "Which folder type(s) will you be using?");

   WXmString	wstr = (char*)msgStr;
   XtVaSetValues(dialog, XmNmessageString, (XmString)wstr, NULL);

   XtManageChild(dialog);
   XMapRaised(halApp->display, XtWindow(XtParent(dialog)));

//
// Loop until at least one type is selected
//
   int		typeCount = 0;
   while ( typeCount == 0 ) {

//
// Simulate the main event loop and wait for the answer
//
      answer = QUERY_NONE;
      while ( answer == QUERY_NONE ) {
	 XtAppProcessEvent(halApp->context, XtIMXEvent);
	 XSync(halApp->display, False);
      }

      Boolean	usingUnix = XmToggleButtonGetState(unixTB);
      Boolean	usingMh   = XmToggleButtonGetState(mhTB);
      Boolean	usingMmdf = XmToggleButtonGetState(mmdfTB);

      typeCount = (usingMh		    ? 1 : 0)
		+ (usingUnix		    ? 1 : 0)
		+ (usingMmdf		    ? 1 : 0)
		+ (pub->appPrefs->usingImap ? 1 : 0);
      if ( typeCount == 0 )
	 pub->PopupMessage("Please select at least one folder type.");
      else {
	 pub->folderPrefs->usingUnix = usingUnix;
	 pub->folderPrefs->usingMh   = usingMh;
	 pub->folderPrefs->usingMmdf = usingMmdf;
      }

   } // End while not done

   XtUnmanageChild(dialog);
   XSync(halApp->display, False);
   XmUpdateDisplay(dialog);

//
// Delete the dialog
//
   delete typeRC;
   XtDestroyWidget(dialog);

//
// If there was more than one type selected, find out which one will be
//    the default type
//
   if ( typeCount > 1 ) {

      Widget	unixTB = NULL;
      Widget	mhTB   = NULL;
      Widget	mmdfTB = NULL;
      Widget	imapTB = NULL;

      args.Reset();
      args.DialogStyle(XmDIALOG_FULL_APPLICATION_MODAL);
      args.AutoUnmanage(False);
      if ( halApp->questionPM ) args.SymbolPixmap(halApp->questionPM);
      dialog = XmCreateQuestionDialog(*halApp, "defFolderTypeWin", ARGS);

//
// Add buttons for folder type
//
      Widget frame = XmCreateFrame   (dialog, "typeFrame", 0,0);
      Widget radio = XmCreateRadioBox(frame,  "typeRadio", 0,0);

      if ( pub->appPrefs->usingImap ) {
	 imapTB = XmCreateToggleButton(radio, "imapTB", 0,0);
	 XtManageChild(imapTB);
      }

      if ( pub->folderPrefs->usingUnix ) {

	 unixTB = XmCreateToggleButton(radio, "unixTB", 0,0);
	 XtManageChild(unixTB);

	 if ( pub->appPrefs->usingImap )
	    msgStr = get_string(unixTB, "imapLabelString",
	    				"Unix on local machine");
	 else
	    msgStr = get_string(unixTB, "labelString", "Unix");

	 wstr = (char*)msgStr;                      
	 XtVaSetValues(unixTB, XmNlabelString, (XmString)wstr, NULL);
      }

      if ( pub->folderPrefs->usingMh ) {

	 mhTB   = XmCreateToggleButton(radio, "mhTB",   0,0);
	 XtManageChild(mhTB);

	 if ( pub->appPrefs->usingImap )
	    msgStr = get_string(mhTB, "imapLabelString",
				      "MH on local machine");
	 else
	    msgStr = get_string(mhTB, "labelString", "MH");

	 wstr = (char*)msgStr;                      
	 XtVaSetValues(mhTB, XmNlabelString, (XmString)wstr, NULL);
      }

      if ( pub->folderPrefs->usingMmdf ) {

	 mmdfTB = XmCreateToggleButton(radio, "mmdfTB", 0,0);
	 XtManageChild(mmdfTB);

	 if ( pub->appPrefs->usingImap )
	    msgStr = get_string(mmdfTB, "imapLabelString",
	    				"MMDF on local machine");
	 else
	    msgStr = get_string(mmdfTB, "labelString", "MMDF");

	 wstr = (char*)msgStr;                      
	 XtVaSetValues(mmdfTB, XmNlabelString, (XmString)wstr, NULL);
      }

      if ( pub->appPrefs->usingImap )
	 XmToggleButtonSetState(imapTB, True, True);
      else if ( pub->folderPrefs->usingUnix )
	 XmToggleButtonSetState(unixTB, True, True);
      else if ( pub->folderPrefs->usingMh )
	 XmToggleButtonSetState(mhTB, True, True);
      else
	 XmToggleButtonSetState(mmdfTB, True, True);

      XtManageChild(radio);
      XtManageChild(frame);

      XtAddCallback(dialog, XmNokCallback, (XtCallbackProc)AnswerQuery,
		    (XtPointer)&answer);
      XtUnmanageChild(XmMessageBoxGetChild(dialog, XmDIALOG_CANCEL_BUTTON));
      XtAddCallback(dialog, XmNhelpCallback,
		    (XtCallbackProc)HalAppC::DoHelp, (char *) "helpcard");

//
// Trap window manager close function
//
      XmAddWMProtocolCallback(XtParent(dialog), halApp->delWinAtom,
			      (XtCallbackProc)WmClose,
			      (caddr_t)&answer);

//
// Show the dialog
//
      XtManageChild(dialog);
      XMapRaised(halApp->display, XtWindow(XtParent(dialog)));

//
// Loop until a type is selected
//
      Boolean	done = False;
      while ( !done ) {

//
// Simulate the main event loop and wait for the answer
//
	 answer = QUERY_NONE;
	 while ( answer == QUERY_NONE ) {
	    XtAppProcessEvent(halApp->context, XtIMXEvent);
	    XSync(halApp->display, False);
	 }

	 Boolean	useImap = imapTB && XmToggleButtonGetState(imapTB);
	 Boolean	useUnix = unixTB && XmToggleButtonGetState(unixTB);
	 Boolean	useMh   = mhTB   && XmToggleButtonGetState(mhTB);
	 Boolean	useMmdf = mmdfTB && XmToggleButtonGetState(mmdfTB);

	 done = True;
	 if ( pub->folderPrefs->usingMh && useMh )
	    pub->folderPrefs->defFolderType = MH_FOLDER;
	 else if ( pub->folderPrefs->usingUnix && useUnix )
	    pub->folderPrefs->defFolderType = UNIX_FOLDER;
	 else if ( pub->folderPrefs->usingMmdf && useMmdf )
	    pub->folderPrefs->defFolderType = MMDF_FOLDER;
	 else if ( pub->appPrefs->usingImap && useImap )
	    pub->folderPrefs->defFolderType = IMAP_FOLDER;
	 else {
	    pub->PopupMessage("Please select a folder type.");
	    done = False;
	 }

      } // End while not done

      XtUnmanageChild(dialog);
      XSync(halApp->display, False);
      XmUpdateDisplay(dialog);

      XtDestroyWidget(dialog);

   } // End if we need to ask for the default folder type

   else {

      if ( pub->appPrefs->usingImap )
	 pub->folderPrefs->defFolderType = IMAP_FOLDER;
      else if ( pub->folderPrefs->usingMh )
	 pub->folderPrefs->defFolderType = MH_FOLDER;
      else if ( pub->folderPrefs->usingMmdf )
	 pub->folderPrefs->defFolderType = MMDF_FOLDER;
      else
	 pub->folderPrefs->defFolderType = UNIX_FOLDER;
   }

   pub->folderPrefs->WriteDatabase();
   pub->folderPrefs->WriteFile();

} // End QueryFolderType

/*---------------------------------------------------------------
 *  Method to return a usable reading window
 */

ReadWinC*
IshAppP::GetReadWin()
{
//
// First look for a displayed, unpinned window
//
   ReadWinC	*readWin = NULL;
   unsigned	count = pub->readWinList.size();
   Boolean	found = False;
   int i=0; for (i=0; !found && i<count; i++) {

      readWin = (ReadWinC*)*pub->readWinList[i];
      if ( readWin->IsShown() && !readWin->Pinned() ) {
	 if ( !readWin->IsIconified() )
	    readWin->HalTopLevelC::Show();	// Pop it to the top
	 found = True;
      }

   } // End for each existing composition window

//
// Then look for an undisplayed window
//
   for (i=0; !found && i<count; i++) {
      readWin = (ReadWinC*)*pub->readWinList[i];
      found = !readWin->IsShown();
   }

//
// If we didn't find a window, we need to create a new one
//
   if ( !found ) {
      pub->BusyCursor(True);
      if ( debuglev > 0 ) cout <<"Creating reading window" NL;
      readWin = new ReadWinC("readWin", *halApp);
      void	*tmp = (void*)readWin;
      pub->readWinList.add(tmp);
      pub->BusyCursor(False);
   }

   return readWin;

} // End GetReadWin

/*---------------------------------------------------------------
 *  Method to see if the user want's to restart any crashed edits.
 */

void
IshAppP::EditSavedCompositions()
{
   int		count = compFileList.size();
   if ( count < 1 ) return;

   WArgList	args;
   args.DialogStyle(XmDIALOG_FULL_APPLICATION_MODAL);
   args.AutoUnmanage(True);
   if ( halApp->questionPM ) args.SymbolPixmap(halApp->questionPM);
   Widget dialog = XmCreateQuestionDialog(*pub, "queryEditCompWin", ARGS);

   XtAddCallback(dialog, XmNokCallback, (XtCallbackProc)FinishEditSavedComp,
			 this);
   XtAddCallback(dialog, XmNcancelCallback, (XtCallbackProc)DeleteSavedComp,
			 this);
   XtAddCallback(dialog, XmNhelpCallback, (XtCallbackProc)HalAppC::DoHelp,
			 (char *) "helpcard");

   Widget	noPB = XmCreatePushButton(dialog, "noPB", 0,0);
   AddActivate(noPB, CancelEditSavedComp, this);
   XtManageChild(noPB);

   StringC	label = get_string(dialog, XmNmessageString,
				  "Resume edit of composition(s) in progress?");
   StringC	countStr;
   countStr += (int)compFileList.size();
   label.Replace("$COUNT", countStr);

   WXmString	wstr = (char*)label;
   XtVaSetValues(dialog, XmNmessageString, (XmString)wstr, NULL);

   XtManageChild(dialog);

} // End EditSavedCompositions

/*---------------------------------------------------------------
 *  Callback to reopen user's crashed edits.
 */

void
IshAppP::FinishEditSavedComp(Widget dialog, IshAppP *This, XtPointer)
{
   u_int	count = This->compFileList.size();
   StringC	file;
   int i=0; for (i=0; i<count; i++) {

      file = ishApp->compPrefs->AutoSaveDir();
      if ( !file.EndsWith('/') ) file += '/';
      file += *This->compFileList[i];

      SendWinC	*win  = This->pub->GetSendWin();
      win->LoadFile(file);
      win->SetAutoSaveFile(file);
      win->Show();
   }
   This->compFileList.removeAll();

   XtDestroyWidget(dialog);

} // End FinishEditSavedComp

/*---------------------------------------------------------------
 *  Callback to deleted user's crashed edits.
 */

void
IshAppP::DeleteSavedComp(Widget dialog, IshAppP *This, XtPointer)
{
   u_int	count = This->compFileList.size();
   StringC	file;
   int i=0; for (i=0; i<count; i++) {

      file = ishApp->compPrefs->AutoSaveDir();
      if ( !file.EndsWith('/') ) file += '/';
      file += *This->compFileList[i];

      unlink(file);
   }
   This->compFileList.removeAll();

   XtDestroyWidget(dialog);

} // End DeleteSavedComp

/*---------------------------------------------------------------
 *  Callback to delay user's crashed edits.
 */

void
IshAppP::CancelEditSavedComp(Widget dialog, IshAppP*, XtPointer)
{
   XtDestroyWidget(dialog);
}

