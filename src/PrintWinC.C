/*
 * $Id: PrintWinC.C,v 1.3 2000/05/07 12:26:12 fnevgeny Exp $
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

#include "PrintWinC.h"
#include "IshAppC.h"
#include "AppPrefC.h"
#include "MainWinC.h"
#include "MsgItemC.h"
#include "MsgC.h"
#include "LoginWinC.h"
#include "MsgPartC.h"
#include "Mailcap.h"
#include "Fork.h"
#include "FolderC.h"
#include "HeadPrefC.h"
#include "HeaderC.h"
#include "Query.h"
#include "FileMsgC.h"
#include "ParamC.h"

#include <hgl/WArgList.h>
#include <hgl/rsrc.h>
#include <hgl/VBoxC.h>
#include <hgl/VItemC.h>
#include <hgl/RegexC.h>
#include <hgl/WXmString.h>
#include <hgl/TextMisc.h>
#include <hgl/SysErr.h>
#include <hgl/PtrListC.h>

#include <Xm/Frame.h>
#include <Xm/RowColumn.h>
#include <Xm/ToggleB.h>
#include <Xm/PushB.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/TextF.h>
#include <Xm/MessageB.h>
#include <Xm/AtomMgr.h>
#include <Xm/Protocols.h>

#include <unistd.h>     // For sleep
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>

PtrListC	*PrintWinC::fetchProcList = NULL;

/*---------------------------------------------------------------
 *  Function to determine if a mime message contains any enriched text
 */

Boolean
ContainsEnriched(MsgPartC *part, Boolean doNext=True)
{
   if ( !part ) return False;

   if ( debuglev > 0 )
      cout <<"Checking for enriched text in " <<part->conStr <<endl;

   Boolean	enriched = False;
   if ( part->IsMultipart() ) {

      if ( part->IsAlternative() ) {
	 MsgPartC	*child = part->BestAlternative(True/*forPrinting*/);
	 enriched = ContainsEnriched(child, False/*doNext*/);
      }

      else if ( ContainsEnriched(part->child) )
	 enriched = True;
   }

   else
      enriched = part->IsEnriched();

   if ( !enriched && doNext && part->next )
      enriched = ContainsEnriched(part->next);

   if ( debuglev > 0 && enriched ) cout <<"Found one" <<endl;
   return enriched;

} // End ContainsEnriched

/*---------------------------------------------------------------
 *  Method to determine if a mime message contains any richtext
 */

Boolean
ContainsRich(MsgPartC *part, Boolean doNext=True)
{
   if ( !part ) return False;

   if ( debuglev > 0 )
      cout <<"Checking for richtext in " <<part->conStr <<endl;

   Boolean	isRich = False;
   if ( part->IsMultipart() ) {

      if ( part->IsAlternative() ) {
	 MsgPartC	*child = part->BestAlternative(True/*forPrinting*/);
	 isRich = ContainsRich(child, False/*doNext*/);
      }

      else if ( ContainsRich(part->child) )
	 isRich = True;
   }

   else
      isRich = part->IsRichText();

   if ( !isRich && doNext && part->next )
      isRich = ContainsRich(part->next);

   if ( debuglev > 0 && isRich ) cout <<"Found one" <<endl;
   return isRich;

} // End ContainsRich

/*---------------------------------------------------------------
 *  Function to convert from text to enriched text in a string
 */

void
MakeEnriched(StringC& string)
{
   if ( string.size() == 0 ) return;

   static RegexC	*ltnl = NULL;
   if ( !ltnl ) ltnl = new RegexC("[<\n]");

//
// Count the number of '<'s and '\n's
//
   int	ltCount = string.NumberOf('<');
   int	nlCount = string.NumberOf('\n');

//
// Allocate space to hold the result
//
   unsigned	strSize = string.size() + ltCount*2 + nlCount*2 + 1;
   StringC	newStr(strSize);
   StringC	sub;

//
// Loop through string and make the following substitutions:
//    '<'  becomes <<
//    x*'\n' becomes (x+1)*'\n'
//
   int	pos, startPos = 0;
   int	endPos = string.size() - 1;
   while ( (pos=ltnl->search(string,startPos)) >= 0 ) {

//
// Copy text up to (but not including) the found character
//
      int	len = pos - startPos;
      if ( len > 0 ) {
	 sub = string(startPos, len);
	 newStr += sub;
      }
      
      if ( string[pos] == '<' ) newStr += "<<";
      else {

	 newStr += "\n\n";	// Add one extra

//
// Copy any more newlines
//
	 while ( pos < endPos && string[pos+1] == '\n' ) {
	    newStr += "\n";
	    pos++;
	 }
      }

      startPos = pos + 1;

   } // End for each '<' or '\n'

//
// Add any remaining text
//
   if ( startPos < string.size() ) {
      int       len = string.size() - startPos;
      if ( len > 0 ) {
	 sub = string(startPos, len);
	 newStr += sub;
      }
   }

   string = newStr;

} // End MakeEnriched

/*---------------------------------------------------------------
 *  Function to convert from text to rich text in a string
 */

void
MakeRich(StringC& string)
{
   if ( string.size() == 0 ) return;

   static RegexC	*ltnl = NULL;
   if ( !ltnl ) ltnl = new RegexC("[<\n]");

//
// Count the number of '<'s and '\n's
//
   int	ltCount = string.NumberOf('<');
   int	nlCount = string.NumberOf('\n');

//
// Allocate space to hold the result
//
   unsigned	strSize = string.size() + ltCount*3 + nlCount*4 + 1;
   StringC	newStr(strSize);
   StringC	sub;

//
// Loop through string and make the following substitutions:
//    '<'  becomes <lt>
//    '\n' becomes <nl>\n
//
   int	pos, startPos = 0;
   while ( (pos=ltnl->search(string,startPos)) >= 0 ) {

//
// Copy text up to (but not including) the found character
//
      int	len = pos - startPos;
      if ( len > 0 ) {
	 sub = string(startPos, len);
	 newStr += sub;
      }
      
      if ( string[pos] == '<' ) newStr += "<lt>";
      else		        newStr += "<nl>\n";

      startPos = pos + 1;

   } // End for each '<' or '\n'

//
// Add any remaining text
//
   if ( startPos < string.size() ) {
      int       len = string.size() - startPos;
      if ( len > 0 ) {
	 sub = string(startPos, len);
	 newStr += sub;
      }
   }

   string = newStr;

} // End MakeRich

/*---------------------------------------------------------------
 *  Function to convert ">From " to "From " in string
 */

void
RestoreFroms(StringC& str)
{
//
// Look for occurrences of ">From " at the beginning of a line.  Replace
//    them with "From "
//
   if ( str.StartsWith(">From ") ) str.CutBeg(1);
   str.Replace("\n>From ", "\nFrom ");

} // End RestoreFroms

/*---------------------------------------------------------------
 *  Dialog constructor
 */

PrintWinC::PrintWinC(Widget parent) : HalDialogC("printWin", parent)
{
   printQueryWin = NULL;
   getQueryWin   = NULL;
   loginWin      = NULL;
   if ( !fetchProcList ) fetchProcList = new PtrListC;

   WArgList	args;
   Widget	list[4];

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
   Widget	cmdForm  = XmCreateForm(appRC, "cmdForm", 0,0);

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
//   list[1] = cmdForm;
   XtManageChildren(list, 1);	// appRC children

   XtManageChild(appRC);	// appForm children

//
// Create buttonRC hierarchy
//
//   buttonRC
//	PushButton	printPB
//	PushButton	cancelPB
//	PushButton	helpPB
//
   AddButtonBox();
   Widget printPB  = XmCreatePushButton(buttonRC, "printPB",  0,0);
   Widget cancelPB = XmCreatePushButton(buttonRC, "cancelPB", 0,0);
   Widget helpPB   = XmCreatePushButton(buttonRC, "helpPB",   0,0);

   list[0] = printPB;
   list[1] = cancelPB;
   list[2] = helpPB;
   XtManageChildren(list, 3);	// buttonRC children

   XtAddCallback(printPB, XmNactivateCallback, (XtCallbackProc)DoPrint,
          	 (XtPointer)this);
   XtAddCallback(cancelPB, XmNactivateCallback, (XtCallbackProc)DoHide,
 		 (XtPointer)this);
   XtAddCallback(helpPB, XmNactivateCallback, (XtCallbackProc)HalAppC::DoHelp,
 		 (char *) "helpcard");

   XtVaSetValues(appForm, XmNdefaultButton, printPB, NULL);

   HandleHelp();

//
// Initialize settings
//
   XmToggleButtonSetState(ordParallelTB, True, True);
   XmToggleButtonSetState(hdrDispTB, True, True);
   XmTextFieldSetString(cmdTF, ishApp->appPrefs->printCmd);

   printStr      = get_string(*ishApp, "printString",
				       "Message sent to printer");
   multiPrintStr = get_string(*ishApp, "multiPrintString",
				       "Messages sent to printer");

} // End PrintWinC constructor

/*---------------------------------------------------------------
 *  Destructor
 */

PrintWinC::~PrintWinC()
{
   if ( ishApp->exiting ) {

//
// Kill any retrieval processes still running
//
      u_int	count = fetchProcList->size();
      for (int i=count-1; i>=0; i--) {
	 FetchProcT	*data = (FetchProcT*)*(*fetchProcList)[i];
	 if ( kill(-data->pid, 0) == 0 ) kill(-data->pid, SIGKILL);
      }
   }
}

/*---------------------------------------------------------------
 *  Callback to handle print
 */

void
PrintWinC::DoPrint(Widget, PrintWinC *This, XtPointer)
{
   This->cancelled = False;

   VItemListC&	selList = ishApp->mainWin->MsgVBox().SelItems();
   unsigned	selCount = selList.size();
   Boolean	usePopup = ( ishApp->mainWin->popupMsg &&
   			    !ishApp->mainWin->popupOnSelected);
   unsigned	prtCount = usePopup ? 1 : selCount;

   Boolean	printed;
   StringC	statusMsg;

   if ( prtCount > 1 ) {

      Boolean parallel = XmToggleButtonGetState(This->ordParallelTB);

      if ( parallel )
	 printed = This->PrintParallel(selList);

      else {

//
// Loop through looking for mime messages
//
         Boolean	anyMime = False;
	 for (int i=0; !anyMime && i<selCount; i++) {
	    MsgItemC	*item = (MsgItemC*)selList[i];
	    anyMime = item->IsMime();
	 }

	 This->askAboutPrint = True;
	 if ( anyMime )
	    printed = This->PrintSerialMime(selList);
	 else
	    printed = This->PrintSerial(selList);

      } // End if serial

      if ( printed )
	 statusMsg = This->multiPrintStr;
   }

   else { // A single message

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

      This->askAboutPrint = True;
      if ( msg->IsMime() ) printed = This->PrintMime(msg);
      else                 printed = This->Print(msg);

      if ( printed && !This->cancelled )
	 statusMsg = This->printStr;

   } // End if a single message

//
// Report status
//
   if ( printed || This->cancelled ) {
      if ( !This->cancelled )
	 ishApp->Broadcast(statusMsg);
      This->Hide();
   }

} // End DoPrint

/*---------------------------------------------------------------
 *  Method to handle print of a single normal message
 */

Boolean
PrintWinC::Print(MsgC *msg)
{
   BusyCursor(True);

   MsgItemC	*item = msg->icon;
   int	msgnum = msg->Number();

//
// Create a temporary file for the message
//
   char	*tmpFile = tempnam(NULL, "prnt.");
   if ( !tmpFile ) {

      int	err = errno;
      StringC	errmsg = "Couldn't create temp file while printing message: ";
      if ( msgnum >= 0 ) errmsg += msgnum;
      errmsg += ".\n" + SystemErrorMessage(err);
      if ( IsShown() ) PopupMessage(errmsg);
      else	       halApp->PopupMessage(errmsg);

      BusyCursor(False);
      return False;
   }

//
// Write the message to the temp file
//
   if ( !msg->WriteFile(tmpFile, !XmToggleButtonGetState(hdrNoneTB),
			XmToggleButtonGetState(hdrAllTB),
			False/*no status header*/,
			False/*no blank line*/, False/*don't protect Froms*/) )
   {
      unlink(tmpFile);
      free(tmpFile);
      BusyCursor(False);
      return False;
   }

   Boolean printed = PrintFile(tmpFile, PRINT_PLAIN, True, msgnum);
   if ( printed ) {
      if ( item ) item->SetPrinted();
      else	  msg->SetPrinted();
   }

   BusyCursor(False);

   return printed;

} // End Print

/*---------------------------------------------------------------
 *  Method to handle print of a single MIME message
 */

Boolean
PrintWinC::PrintMime(MsgC *msg)
{
   BusyCursor(True);

   PtrListC	sepList;
   MsgItemC	*item = msg->icon;
   int	msgnum = msg->Number();

//
// Create a temporary file for storing messages
//
   char	*tmpFile = tempnam(NULL, "prnt.");
   if ( !tmpFile ) {

      int	err = errno;
      StringC	errmsg = "Couldn't create temp file while printing message: ";
      if ( item ) errmsg += msgnum;
      errmsg += ".\n" + SystemErrorMessage(err);

      BusyCursor(False);
      if ( IsShown() ) PopupMessage(errmsg);
      else	       halApp->PopupMessage(errmsg);
      return False;
   }

   FILE *tfp = fopen(tmpFile, "w+");
   if ( !tfp ) {

      int	err = errno;
      StringC	errmsg = "Couldn't create temp file while printing message: ";
      if ( item ) errmsg += msgnum;
      errmsg += ".\n" + SystemErrorMessage(err);

      free(tmpFile);
      BusyCursor(False);
      if ( IsShown() ) PopupMessage(errmsg);
      else	       halApp->PopupMessage(errmsg);
      return False;
   }

//
// Determine what text type will be used to print
//
   PrintTypeT	pType = PRINT_PLAIN;
   MsgPartC	*body = msg->Body();

   if      ( ContainsEnriched(body) ) pType = PRINT_ENRICHED;
   else if ( ContainsRich    (body) ) pType = PRINT_RICH;

//
// Write headers
//
   Boolean	printed = PrintHeaders(msg, tfp, pType);

//
// Add each text part of the message to the file.
//
   if ( printed ) {
      printed = PrintText(body, tfp, sepList, pType);
      if ( cancelled ) {
	 fclose(tfp);
	 if ( debuglev > 0 ) cout <<"Unlinking temp file" <<endl;
	 unlink(tmpFile);
	 free(tmpFile);
	 BusyCursor(False);
	 return False;
      }
   }

//
// Check for errors
//
   if ( !printed ) {

      int	err = errno;
      StringC	errmsg = "Couldn't write to temp file while printing message: ";
      if ( item ) errmsg += msgnum;
      errmsg += ".\n" + SystemErrorMessage(err);

      fclose(tfp);
      if ( debuglev > 0 ) cout <<"Unlinking temp file" <<endl;
      unlink(tmpFile);
      free(tmpFile);
      BusyCursor(False);
      if ( IsShown() ) PopupMessage(errmsg);
      else	       halApp->PopupMessage(errmsg);
      return False;
   }
   fclose(tfp);

//
// Print the temp file.  This routine will also delete it.
//
   printed = PrintFile(tmpFile, pType, True, msgnum);

   if ( !printed ) {
      BusyCursor(False);
      return False;
   }

//
// Now make a pass and print the separate parts
//
   unsigned	count = sepList.size();
   for (int i=0; !cancelled && i<count; i++) {
      MsgPartC	*part = (MsgPartC*)*sepList[i];
      PrintNonText(part);
   }

   if ( item ) item->SetPrinted();
   else	       msg->SetPrinted();

   BusyCursor(False);
   return True;

} // End PrintMime

/*---------------------------------------------------------------
 *  Method to print selected headers for given message
 */

Boolean
PrintWinC::PrintHeaders(MsgC *msg, FILE *fp, PrintTypeT pType)
{
   if ( XmToggleButtonGetState(hdrNoneTB) ) return True;

   Boolean	success = True;

   if ( pType == PRINT_PLAIN ) {
      success = msg->WriteHeaders(fp, XmToggleButtonGetState(hdrAllTB), False);
   }

   else {

//
// Read headers into a string so they can be richified
//
      StringC	headStr;
      Boolean	all = XmToggleButtonGetState(hdrAllTB);

      HeaderC	*head  = msg->Headers();
      Boolean	needNL = False;
      while ( head ) {

//
// See if this header should be printed
//
	 if ( all || ishApp->headPrefs->HeaderShown(head->key) ) {

	    if ( needNL ) headStr += '\n';

	    headStr += head->key;
	    headStr += ": ";
	    head->GetValueText(headStr);

	    needNL = True;

	 } // End if header is displayed

	 head = head->next;

      } // End for each header

      if ( headStr.size() > 0 ) {

	 headStr += '\n';
	 if      ( pType == PRINT_ENRICHED ) MakeEnriched(headStr);
	 else if ( pType == PRINT_RICH     ) MakeRich(headStr);

	 success = headStr.WriteFile(fp);
      }

   } // End if not printing as plain text

   return success;

} // End PrintHeaders

/*---------------------------------------------------------------
 *  Method to look for plaintext parts and print them
 */

Boolean
PrintWinC::PrintText(MsgPartC *part, FILE *fp, PtrListC& sepList,
		     PrintTypeT pType, Boolean doNext)
{
   if ( !part ) return True;

   if ( debuglev > 0 ) cout <<"Checking " <<part->conStr <<endl;

   Boolean	printed = False;
   if ( part->IsMultipart() ) {

//
// If this is an alternative, find the best choice.  If it is text, print it.
//
      if ( part->IsAlternative() ) {
	 MsgPartC	*child = part->BestAlternative(True/*forPrinting*/);
	 printed = PrintText(child, fp, sepList, pType, False/*doNext*/);
      }

//
// If this is not an alternative, print all the children, starting with
//    the first
//
      else
	 printed = PrintText(part->child, fp, sepList, pType);

   } // End if multipart
      
   else if ( !part->IsExternal() && part->IsText() ) {

      StringC	partStr;
      part->GetData(partStr);

      if ( pType == PRINT_ENRICHED ) {
	 if ( part->IsPlainText() ) MakeEnriched(partStr);
	 if ( !partStr.EndsWith("\n") ) partStr += "\n";
      }

      else if ( pType == PRINT_RICH ) {
	 if ( part->IsPlainText() ) MakeRich(partStr);
	 while ( partStr.EndsWith("\n") ) partStr.Clear(partStr.size()-1);
	 if ( !partStr.EndsWith("<nl>") ) partStr += "<nl>\n";
      }

      printed = partStr.WriteFile(fp);
   }

   else {
      printed = InsertMessage(part, fp, sepList, pType);
   }

   if ( doNext && !cancelled && part->next )
      printed = PrintText(part->next, fp, sepList, pType);

   return (printed && !cancelled);

} // End PrintText

/*---------------------------------------------------------------
 *  Method to look for non-text parts and print them
 */

Boolean
PrintWinC::PrintNonText(MsgPartC *part)
{
//
// Retrieve the part if necessary
//
   if ( part->IsExternal() && (!GetPart(part) || cancelled) )
      return False;

//
// If this is an rfc822 message, pass it back through the top level routines
//
   if ( part->Is822() ) {

      FileMsgC	*msg822 = part->ChildMsg();

      Boolean	printed = False;
      if ( msg822->IsMime() ) printed = PrintMime(msg822);
      else                    printed = Print(msg822);

      return printed;

   } // End if printing an rfc822 message

//
// See if this type can be printed
//
   MailcapC	*mcap    = MailcapEntry(part);
   Boolean	canPrint = ((mcap && mcap->print.size() > 0) ||
			    (part->IsText() || part->IsPostScript()));
   if ( !canPrint ) return False;
   
//
// Use the part data file
//
   if ( !part->CreateDataFile() ) return False;

//
// See how this part is to be printed
//
   if ( mcap && mcap->print.size() > 0 ) {

//
// Create some callback data
//
      PrintProcT	*proc = new PrintProcT;
      proc->win    = this;
      proc->msgnum = -1;
      proc->cmdStr = BuildCommand(mcap->print, part, part->dataFile);
      CallbackC	doneCb((CallbackFn*)PrintDone, proc);

      proc->pid = ForkIt(proc->cmdStr, &doneCb);
      if ( proc->pid < 0 ) {
	 StringC	errmsg = "Could not print ";
	 part->GetLabel(errmsg);
	 errmsg += "\n" + ForkStatusMsg(proc->cmdStr, (int)proc->pid);
	 if ( IsShown() ) PopupMessage(errmsg);
	 else		  halApp->PopupMessage(errmsg);
	 delete proc;
	 return False;
      }
   }

   else {	// Must be text

      PrintTypeT	pType = PRINT_PLAIN;
      if ( part->IsEnriched() ) pType = PRINT_ENRICHED;
      if ( part->IsRichText() ) pType = PRINT_RICH;

      if ( !PrintFile(part->dataFile, pType, False) ) return False;

   } // End if no mailcap entry

   return True;

} // End PrintNonText

/*---------------------------------------------------------------
 *  Method to insert a message about a part that can't be printed or
 *     will be printed separately
 */

Boolean
PrintWinC::InsertMessage(MsgPartC *part, FILE *fp, PtrListC& sepList,
			 PrintTypeT pType)
{
   StringC		str;
   StringC		typeStr;
   Boolean		printable = False;
   Boolean		okToPrint = True;
   StringC		name;
   part->GetFileName(name);

   if      ( pType == PRINT_ENRICHED ) str = "\n\n<center>";
   else if ( pType == PRINT_RICH     ) str = "<nl>\n<center>";
   else				       str = "\n\t";

   typeStr = part->conStr;
   MailcapC	*mcap = MailcapEntry(part);
   printable = (mcap && mcap->print.size()>0);

   if ( part->IsExternal() ) {

      if      ( part->IsPlainText() ) {
	 typeStr = "Text";
	 printable = True;
      }
      else if ( part->IsEnriched() ) {
	 typeStr = "EnrichedText";
	 printable = True;
      }
      else if ( part->IsRichText() ) {
	 typeStr = "RichText";
	 printable = True;
      }
      else if ( part->Is822() ) {
	 typeStr = "Mail Message";
	 printable = True;
      }
      else if ( part->IsPostScript() ) {
	 typeStr = "PostScript";
	 printable = True;
      }
      else if ( part->IsBinary()     ) typeStr = "Binary";
      else if ( part->IsGIF()        ) typeStr = "GIF Image";
      else if ( part->IsJPEG()       ) typeStr = "JPEG Image";
      else if ( part->IsMPEG()       ) typeStr = "MPEG Video";
      else if ( part->IsBasicAudio() ) typeStr = "U-LAW Audio";

      str += "(Attached ";
      str += typeStr;
      str += " File: ";
      str += name;
      str += ')';

//
// If this part can be printed and hasn't been retrieved, see if the user
//    wants it retrieved.  If it has been retrieved, see if the user
//    wants it printed.
//
      if ( printable ) {
	 if ( part->NeedFetch() ) okToPrint = OkToGet(part);
	 else			  okToPrint = OkToPrint(part);
      }

   } // End if part is external
      
   else {

      Boolean	confirmPrint = True;
      if ( part->Is822() ) {
	 typeStr = "Mail Message";
	 printable = True;
	 confirmPrint = False;
	 FileMsgC	*msg = part->ChildMsg();
	 msg->GetSubjectText(name);
      }
      else if ( part->IsPostScript() ) {
	 typeStr = "PostScript File";
	 printable = True;
	 confirmPrint = False;
      }
      else if ( part->IsBinary()     ) typeStr = "Binary";
      else if ( part->IsGIF()        ) typeStr = "GIF Image";
      else if ( part->IsJPEG()       ) typeStr = "JPEG Image";
      else if ( part->IsMPEG()       ) typeStr = "MPEG Video";
      else if ( part->IsBasicAudio() ) typeStr = "U-LAW Audio";

      str += "(Included ";
      str += typeStr;
      str += ": ";
      str += name;
      str += ')';

//
// See if the user wants this part printed
//
      if ( printable && confirmPrint )
	 okToPrint = OkToPrint(part);

   } // End if part is not external

   if ( cancelled ) return False;

   if      ( pType == PRINT_ENRICHED ) str += "\n\n";
   else if ( pType == PRINT_RICH     ) str += "<nl>\n";
   else				       str += "\n\t";

//
// Figure out if the message will be printed or not
//
   if ( printable ) {
      if ( okToPrint ) {
	 str += "(Printed separately)";
	 void	*tmp = (void *)part;
	 sepList.add(tmp);
      }
      else
	 str += "(Not printed)";
   } else
      str += "(Not printable)";

   if      ( pType == PRINT_ENRICHED ) str += "</center>\n\n";
   else if ( pType == PRINT_RICH     ) str += "</center><nl>\n<nl>\n";
   else				       str += "\n\n";

   return str.WriteFile(fp);

} // End InsertMessage

/*---------------------------------------------------------------
 *  Method to query user to see if part should be printed
 */

Boolean
PrintWinC::OkToPrint(MsgPartC *part)
{
//
// See if the user already answered.
//
   if ( !askAboutPrint ) return printAll;

   static QueryAnswerT	answer;

//
// Create the dialog if necessary
//
   if ( !printQueryWin ) {

      BusyCursor(True);

      WArgList	args;
      args.DialogStyle(XmDIALOG_FULL_APPLICATION_MODAL);
      if ( halApp->questionPM ) args.SymbolPixmap(halApp->questionPM);
      Widget	w = XmCreateQuestionDialog(*this, "printQueryWin", ARGS);

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

      Widget	choiceFrame = XmCreateFrame(w, "choiceFrame", 0,0);

      args.Reset();
      args.Orientation(XmVERTICAL);
      args.Packing(XmPACK_COLUMN);
      args.RadioBehavior(True);
      Widget choiceRadio = XmCreateRowColumn(choiceFrame, "choiceRadio", ARGS);

      printOneYesTB = XmCreateToggleButton(choiceRadio, "printOneYesTB", 0,0);
      printOneNoTB  = XmCreateToggleButton(choiceRadio, "printOneNoTB",  0,0);
      printAllYesTB = XmCreateToggleButton(choiceRadio, "printAllYesTB", 0,0);
      printAllNoTB  = XmCreateToggleButton(choiceRadio, "printAllNoTB",  0,0);

      Widget wlist[4];
      wlist[0] = printOneYesTB;
      wlist[1] = printOneNoTB;
      wlist[2] = printAllYesTB;
      wlist[3] = printAllNoTB;
      XtManageChildren(wlist, 4);
      XtManageChild(choiceRadio);
      XtManageChild(choiceFrame);

      printQueryWin = w;
      BusyCursor(False);

      XmToggleButtonSetState(printOneYesTB, True, True);

   } // End if dialog not created

//
// Set the strings, depending on this type
//
   StringC	msg;
   if ( part->IsExternal() )
      msg = get_string(printQueryWin, "attachmentMsg", "Print $NAME");
   else
      msg = get_string(printQueryWin, "inclusionMsg", "Print $NAME");

//
// Insert the file name
//
   StringC	name;
   part->GetFileName(name);
   msg.Replace("$NAME", name);

   WXmString	wxmstr = (char*)msg;
   XtVaSetValues(printQueryWin, XmNmessageString, (XmString)wxmstr, NULL);

//
// Show the dialog
//
   XtManageChild(printQueryWin);
   XMapRaised(halApp->display, XtWindow(XtParent(printQueryWin)));

//
// Simulate the main event loop and wait for the answer
//
   answer = QUERY_NONE;
   while ( answer == QUERY_NONE ) {
      XtAppProcessEvent(halApp->context, XtIMXEvent);
      XSync(halApp->display, False);
   }

   XtUnmanageChild(printQueryWin);
   XSync(halApp->display, False);
   XmUpdateDisplay(printQueryWin);

//
// Set state based on user's answer
//
   if ( answer == QUERY_CANCEL ) {
      cancelled = True;
      return False;
   }

   if ( XmToggleButtonGetState(printOneYesTB) ) return True;
   if ( XmToggleButtonGetState(printOneNoTB)  ) return False;
   if ( XmToggleButtonGetState(printAllYesTB) ) {
      askAboutPrint = False;
      printAll = True;
      return True;
   }
   if ( XmToggleButtonGetState(printAllNoTB) ) {
      askAboutPrint = False;
      printAll = False;
      return False;
   }

   return False;

} // End OkToPrint

/*---------------------------------------------------------------
 *  Method to query user to see if part should be retrieved
 */

Boolean
PrintWinC::OkToGet(MsgPartC *part)
{
//
// See if the user already answered.
//
   if ( !askAboutPrint ) return printAll;

   static QueryAnswerT	answer;

//
// Create the dialog if necessary
//
   if ( !getQueryWin ) {

      BusyCursor(True);

      WArgList	args;
      args.DialogStyle(XmDIALOG_FULL_APPLICATION_MODAL);
      if ( halApp->questionPM ) args.SymbolPixmap(halApp->questionPM);
      Widget	w = XmCreateQuestionDialog(*this, "getQueryWin", ARGS);

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

      Widget	choiceFrame = XmCreateFrame(w, "choiceFrame", 0,0);

      args.Reset();
      args.Orientation(XmVERTICAL);
      args.Packing(XmPACK_COLUMN);
      args.RadioBehavior(True);
      Widget choiceRadio = XmCreateRowColumn(choiceFrame, "choiceRadio", ARGS);

      getOneYesTB = XmCreateToggleButton(choiceRadio, "getOneYesTB", 0,0);
      getOneNoTB  = XmCreateToggleButton(choiceRadio, "getOneNoTB",  0,0);
      getAllYesTB = XmCreateToggleButton(choiceRadio, "getAllYesTB", 0,0);
      getAllNoTB  = XmCreateToggleButton(choiceRadio, "getAllNoTB",  0,0);

      Widget wlist[4];
      wlist[0] = getOneYesTB;
      wlist[1] = getOneNoTB;
      wlist[2] = getAllYesTB;
      wlist[3] = getAllNoTB;
      XtManageChildren(wlist, 4);
      XtManageChild(choiceRadio);
      XtManageChild(choiceFrame);

      getQueryWin = w;
      BusyCursor(False);

      XmToggleButtonSetState(getOneYesTB, True, True);

   } // End if dialog not created

//
// Stick in the file name
//
   StringC msg = get_string(getQueryWin, "messageString", "Retrieve $NAME");
   StringC name;
   part->GetFileName(name);
   msg.Replace("$NAME", name);

   WXmString	wxmstr = (char*)msg;
   XtVaSetValues(getQueryWin, XmNmessageString, (XmString)wxmstr, NULL);

//
// Show the dialog
//
   XtManageChild(getQueryWin);
   XMapRaised(halApp->display, XtWindow(XtParent(getQueryWin)));

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
   if ( answer == QUERY_CANCEL ) {
      cancelled = True;
      return False;
   }

   if ( XmToggleButtonGetState(getOneYesTB) ) return True;
   if ( XmToggleButtonGetState(getOneNoTB)  ) return False;
   if ( XmToggleButtonGetState(getAllYesTB) ) {
      askAboutPrint = False;
      printAll = True;
      return True;
   }
   if ( XmToggleButtonGetState(getAllNoTB) ) {
      askAboutPrint = False;
      printAll = False;
      return False;
   }

   return False;

} // End OkToGet

/*---------------------------------------------------------------
 *  Method to query user to see if mail-server message should be sent
 */

Boolean
PrintWinC::OkToSend(MsgPartC *part)
{
   static QueryAnswerT	answer;
   static Widget	sendQueryWin = NULL;

//
// Create the dialog if necessary
//
   if ( !sendQueryWin ) {

      BusyCursor(True);

      WArgList	args;
      args.DialogStyle(XmDIALOG_FULL_APPLICATION_MODAL);
      if ( halApp->questionPM ) args.SymbolPixmap(halApp->questionPM);
      Widget	w = XmCreateQuestionDialog(*this, "sendQueryWin", ARGS);

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
      BusyCursor(False);

   } // End if dialog not created

//
// Add the recipient name
//
   StringC	label = get_string(sendQueryWin, "messageString",
			"Send following message to \"$SERVER\".\n\n$MESSAGE");

   ParamC	*server = part->Param("server");
   if ( !server ) {
      label = "There is no mail server specified for this body part.";
      PopupMessage(label);
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
   XtManageChild(sendQueryWin);
   XMapRaised(halApp->display, XtWindow(XtParent(sendQueryWin)));

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
 *  Method to retrieve part
 */

Boolean
PrintWinC::GetPart(MsgPartC *part)
{
   if ( !part->NeedFetch() ) return True;

//
// Prompt user for name and password if necessary
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
	 PopupMessage(errmsg);
	 return False;
      }

//
// Prompt for login
//
      if ( !loginWin ) loginWin = new LoginWinC(*this);

      if ( !loginWin->GetLogin(site->val, part->userName, part->userPass) ) {
	 cancelled = True;
	 return False;
      }

   } // End if password needed

//
// For security reasons, confirm retrieval of mail-server attachments.
// We don't want someone sending an attachment that will cause someone
//   else to send obnoxious mail.
//
   if ( part->IsMail() && !OkToSend(part) ) return False;

//
// Look for a mailcap entry for message/external-body
//
   MailcapC	*mcap = MailcapEntry(MESSAGE_S, EXTERNAL_BODY_S, part);
   if ( !mcap ) {
      StringC errmsg = "I could not find a mailcap entry for: ";
      errmsg += "message/external-body.";
      errmsg += ".\nExternal files cannot be retrieved.";
      PopupMessage(errmsg);
      return False;
   }

//
// See if there's a fetch command
//
   if ( mcap->present.size() == 0 ) {
      StringC errmsg = "The mailcap entry for message/external-body does ";
      errmsg += "not specify a command for getting the file.";
      errmsg += ".\nExternal files cannot be retrieved.";
      PopupMessage(errmsg);
      return False;
   }

//
// Clear out the old external file name and create a new name
//
   StringC	file;
   if ( part->delDataFile && part->dataFile.size() > 0 ) {
      file = part->dataFile;	// Reuse name
      unlink(part->dataFile);
      part->dataFile.Clear();
      part->delDataFile = False;
   }
   else {
      char	*cs = tempnam(NULL, "prnt.");
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
	 PopupMessage(errmsg);
	 return False;
      }

   } // End if mail server

//
// Build a data record to store info about this fetch
//
   FetchProcT	*data = new FetchProcT;
   data->win  = this;
   data->part = part;
   data->file = file;

//
// Build the command for retrieval.  The present command is used since this
//   is a message/external-body type.
//
   data->cmdStr = BuildCommand(mcap->present, part, file);
   data->pid    = -1;

   CallbackC	doneCb((CallbackFn*)FinishedGet, data);
   data->pid = ForkIt(data->cmdStr, &doneCb);

//
// If the pid is < 0, an error occurred
//
   if ( data->pid < 0 ) {
      PopupMessage(ForkStatusMsg(data->cmdStr, (int)data->pid));
      delete data;
   }

//
// Add this process to the list of running processes
//
   else {
      void	*tmp = (void*)data;
      fetchProcList->add(tmp);
   }

   return False;

} // End GetPart

/*---------------------------------------------------------------
 *  Signal handler to catch completion of external file retrieval
 */

void
PrintWinC::FinishedGet(int status, FetchProcT *data)
{
   PrintWinC	*This = data->win;

//
// Remove this data from the list of active fetches
//
   void	*tmp = (void*)data;
   fetchProcList->remove(tmp);

   MsgPartC	*part = data->part;
   StringC	name;
   part->GetFileName(name);

   if ( debuglev > 0 ) cout <<"FinishedGet for file: " <<name <<endl;

   if ( status != 0 || part->IsMail() ) {

      StringC	statmsg("File \"");
      statmsg += name;

      if ( status != 0 ) {
	 statmsg += "\" could not be retrieved.\n";
	 statmsg += ForkStatusMsg(data->cmdStr, status, data->pid);
      }
      else if ( part->IsMail() )
	 statmsg += "\" will arrive by mail a later time.";

      if ( This->IsShown() )
	 This->PopupMessage(statmsg);
      else
	 halApp->PopupMessage(statmsg);
   }

   else {

      part->dataFile    = data->file;
      part->delDataFile = True;

      if ( !This->IsShown() ) {
	 StringC	statmsg("File \"");
	 statmsg += name;
	 statmsg += "\" has arrived and will now be printed.";
	 halApp->PopupMessage(statmsg);
      }

      This->PrintNonText(part);
   }

   delete data;

} // End FinishedGet

/*---------------------------------------------------------------
 *  Method to handle print of multiple messages in parallel
 */

Boolean
PrintWinC::PrintParallel(VItemListC& list)
{
   BusyCursor(True);

   Boolean	printed = True;
   unsigned	count = list.size();
   for (int i=0; !cancelled && i<count; i++) {

      MsgItemC	*item = (MsgItemC*)list[i];
      MsgC	*msg = item->msg;

      askAboutPrint = True;
      if ( msg->IsMime() ) {
	 if ( !PrintMime(msg) ) printed = False;
      } else {
	 if ( !Print(msg) ) printed = False;
      }
   }

   BusyCursor(False);
   return printed;

} // End PrintParallel

/*---------------------------------------------------------------
 *  Method to handle print of multiple messages as a single file
 */

Boolean
PrintWinC::PrintSerial(VItemListC& list)
{
   BusyCursor(True);

   StringC	statusMsg;

//
// Create a temporary file for storing messages
//
   char	*tmpFile = tempnam(NULL, "prnt.");
   if ( !tmpFile ) {

      int	err = errno;
      statusMsg = "Could not create temporary file for printing.\n";
      statusMsg += SystemErrorMessage(err);
      statusMsg += "\nWill attempt to print in parallel.";
      PopupMessage(statusMsg, XmDIALOG_WARNING);

      BusyCursor(False);

      return PrintParallel(list);
   }

   FILE *tfp = fopen(tmpFile, "w+");
   if ( !tfp ) {

      int	err = errno;
      statusMsg = "Could not create temporary file for printing.\n";
      statusMsg += SystemErrorMessage(err);
      statusMsg += "\nWill attempt to print in parallel.";
      PopupMessage(statusMsg, XmDIALOG_WARNING);

      free(tmpFile);
      BusyCursor(False);

      return PrintParallel(list);
   }

   Boolean	allHeaders = XmToggleButtonGetState(hdrAllTB);
   Boolean	noHeaders  = XmToggleButtonGetState(hdrNoneTB);
   StringC	headStr;

//
// Read message separator
//
   StringC	banner = get_string(*this, "messageBanner",
   				    "\n-----------------------------------\n");
   StringC	banStr;

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
// Write banner
//
      if ( banner.size() > 0 ) {
	 banStr = banner;
	 msg->ReplaceHeaders(banStr);
	 error = !banStr.WriteFile(tfp);
      }

//
// Write headers and body
//
      if ( !error )
	 error = !msg->WriteFile(tfp, !noHeaders, allHeaders,
				 False/*no status header*/,
				 True/*add a blank line*/,
				 False/*don't protect Froms*/);

   } // End for each selected message

//
// Check for errors
//
   if ( error ) {

      int	err = errno;
      statusMsg = "Could not write to temporary file for printing.\n";
      statusMsg += SystemErrorMessage(err);
      statusMsg += "\nWill attempt to print in parallel.";
      PopupMessage(statusMsg, XmDIALOG_WARNING);

      fclose(tfp);
      if ( debuglev > 0 ) cout <<"Unlinking temp file" <<endl;
      unlink(tmpFile);
      free(tmpFile);

      BusyCursor(False);
      return PrintParallel(list);
   }

   fclose(tfp);
   error = !PrintFile(tmpFile);

//
// Update status
//
   if ( !error ) {
      count = list.size();
      for (i=0; i<count; i++) {
	 MsgItemC	*item = (MsgItemC*)list[i];
	 item->SetPrinted();
      }
   }

   BusyCursor(False);
   return !error;

} // End PrintSerial

/*---------------------------------------------------------------
 *  Method to handle print of multiple messages as a single file,
 *     taking into account that one or more of them is a MIME
 *     message
 */

Boolean
PrintWinC::PrintSerialMime(VItemListC& list)
{
   PtrListC	sepList;

   StringC	statusMsg;
   BusyCursor(True);

//
// Create a temporary file for storing messages
//
   char	*tmpFile = tempnam(NULL, "prnt.");
   if ( !tmpFile ) {

      int	err = errno;
      statusMsg = "Could not create temporary file for printing.\n";
      statusMsg += SystemErrorMessage(err);
      statusMsg += "\nWill attempt to print in parallel.";
      PopupMessage(statusMsg, XmDIALOG_WARNING);

      BusyCursor(False);

      return PrintParallel(list);
   }

   FILE *tfp = fopen(tmpFile, "w+");
   if ( !tfp ) {

      int	err = errno;
      statusMsg = "Could not create temporary file for printing.\n";
      statusMsg += SystemErrorMessage(err);
      statusMsg += "\nWill attempt to print in parallel.";
      PopupMessage(statusMsg, XmDIALOG_WARNING);

      free(tmpFile);
      BusyCursor(False);

      return PrintParallel(list);
   }

//
// Determine what text type will be used for printing
//
   PrintTypeT	pType = PRINT_PLAIN;
   unsigned	count = list.size();
   int	i;
   for (i=0; pType != PRINT_ENRICHED && i<count; i++) {

      MsgItemC	*item = (MsgItemC*)list[i];
      MsgC	*msg = item->msg;
      MsgPartC	*body = msg->Body();
      if ( msg->IsMime() ) {
	 if      ( ContainsEnriched(body) ) pType = PRINT_ENRICHED;
	 else if ( ContainsRich    (body) ) pType = PRINT_RICH;
      }
   }

//
// Read message separator
//
   StringC	banner;
   if ( pType == PRINT_ENRICHED )
      banner = get_string(*this, "enrichedBanner",
				 "\n\n-----------------------------------\n\n");
   else if ( pType == PRINT_RICH )
      banner = get_string(*this, "richtextBanner",
				 "<nl>-----------------------------------<nl>");
   else
      banner = get_string(*this, "messageBanner",
				 "\n-----------------------------------\n");
   StringC	banStr;

//
// Store selected messages in temporary file
//
   StringC	bodyStr;
   Boolean	error = False;
   for (i=0; !error && !cancelled && i<count; i++) {

      MsgItemC	*item = (MsgItemC*)list[i];
      MsgC	*msg  = item->msg;
      int	index = msg->Number();

//
// Write banner
//
      if ( banner.size() > 0 ) {
	 banStr = banner;
	 msg->ReplaceHeaders(banStr);
	 error = !banStr.WriteFile(tfp);
      }
      if ( error ) continue;

//
// Write headers
//
      error = !PrintHeaders(msg, tfp, pType);
      if ( error ) continue;

//
// Write body
//
      if ( msg->IsMime() )
	 error = !PrintText(msg->Body(), tfp, sepList, pType);

      else if ( pType != PRINT_PLAIN ) {

	 bodyStr.Clear();
	 msg->GetBodyText(bodyStr);

	 if ( pType == PRINT_ENRICHED ) MakeEnriched(bodyStr);
	 else			        MakeRich(bodyStr);

	 error = !bodyStr.WriteFile(tfp);
      }

      else
	 error = !msg->WriteBody(tfp, True/*add a blank line*/,
	 			 False/*don't protect froms*/);

//
// Check for errors
//
      if ( error ) {

	 int	err = errno;
	 StringC errmsg = "Couldn't write to temp file while printing message: ";
	 errmsg += msg->Number();
	 errmsg += ".\n" + SystemErrorMessage(err);
	 PopupMessage(errmsg);

	 fclose(tfp);
	 if ( debuglev > 0 ) cout <<"Unlinking temp file" <<endl;
	 unlink(tmpFile);
	 free(tmpFile);

	 BusyCursor(False);
	 return False;
      }

   } // End for each message

//
// Exit if the operation was cancelled
//
   if ( cancelled ) {

      fclose(tfp);
      if ( debuglev > 0 ) cout <<"Unlinking temp file" <<endl;
      unlink(tmpFile);
      free(tmpFile);

      BusyCursor(False);
      return False;
   }
   fclose(tfp);

//
// Print the temporary file.
//
   error = !PrintFile(tmpFile, pType);

   if ( error ) {
      BusyCursor(False);
      return False;
   }

//
// Now make a pass and print the separate parts
//
   count = sepList.size();
   for (i=0; !cancelled && i<count; i++) {
      MsgPartC	*part = (MsgPartC*)*sepList[i];
      PrintNonText(part);
   }

//
// Update status
//
   count = list.size();
   for (i=0; i<count; i++) {
      MsgItemC	*item = (MsgItemC*)list[i];
      item->SetPrinted();
   }

   BusyCursor(False);
   return True;

} // End PrintSerialMime

/*---------------------------------------------------------------
 *  Method to print a temporary file
 */

Boolean
PrintWinC::PrintFile(char *fileName, PrintTypeT pType, Boolean isTmp,
		     int msgnum)
{
//
// Build the print command
//
   StringC	typeStr;
   MailcapC	*mcap = NULL;
   if ( pType == PRINT_ENRICHED ) {
      mcap    = MailcapEntry("text", "enriched");
      typeStr = "text/enriched";
   }
   else if ( pType == PRINT_RICH ) {
      mcap    = MailcapEntry("text", "richtext");
      typeStr = "text/richtext";
   }
   else {
      mcap    = MailcapEntry("text", "plain");
      typeStr = "text/plain";
   }

   StringC	cmd;
   if ( mcap && mcap->print.size()>0 ) cmd = mcap->print;
   else				       cmd = ishApp->appPrefs->printCmd;

   cmd = BuildCommand(cmd, fileName, NULL, typeStr, NULL, NULL, NULL);

//
// Create some callback data
//
   PrintProcT	*proc = new PrintProcT;
   proc->win     = this;
   proc->msgnum  = msgnum;
   proc->cmdStr  = cmd;
   if ( isTmp ) proc->tmpFile = fileName;
   CallbackC	doneCb((CallbackFn*)PrintDone, proc);

   proc->pid = ForkIt(cmd, &doneCb);

   if ( proc->pid < 0 ) {

      StringC	errmsg = "Could not print message";
      if ( msgnum >= 0 ) {
	 errmsg += " ";
	 errmsg += msgnum;
      }
      else
	 errmsg += "s.";
      errmsg += "\n" + ForkStatusMsg(cmd, (int)proc->pid);
      if ( IsShown() ) PopupMessage(errmsg);
      else	       halApp->PopupMessage(errmsg);

      if ( isTmp ) {
	 unlink(fileName);
	 free(fileName);
      }
      delete proc;
      return False;
   }

   return True;

} // End PrintFile

/*---------------------------------------------------------------
 *  Callback for completion of ForkIt
 */

void
PrintWinC::PrintDone(int status, PrintProcT *proc)
{
   if ( debuglev > 0 ) cout <<"Print process finished" <<endl;

//
// Report status of operation
//
   if ( status != 0 ) {

      StringC	errmsg;
      if ( proc->msgnum >= 0 ) {
	 errmsg = "Message ";
	 errmsg += proc->msgnum;
      }
      else
	 errmsg = "Messages";
      errmsg += " not printed.\n";
      errmsg += ForkStatusMsg(proc->cmdStr, status, proc->pid);
      proc->win->PopupMessage(errmsg);

   } // End if there is an error

   if ( proc->tmpFile.size() > 0 ) {
      if ( debuglev > 0 ) cout <<"Unlinking temp file" <<endl;
      unlink(proc->tmpFile);
   }
   delete proc;

   return;

} // End PrintDone

/*---------------------------------------------------------------
 *  Method to remove ordFrame
 */

void
PrintWinC::HideOrder()
{
   XtUnmanageChild(ordFrame);
}

/*---------------------------------------------------------------
 *  Method to display ordFrame
 */

void
PrintWinC::ShowOrder()
{
   XtManageChild(ordFrame);
}

#if 0
/*---------------------------------------------------------------
 *  Method to display dialog
 */

void
PrintWinC::Show(Widget parent)
{
//
// Figure out what the default command will be
//
   StringC	cmd;
   Boolean	isMime   = False;
   Boolean	richtext = False;
   Boolean	enriched = False;
   if ( mainWin->popupMsg && !mainWin->popupOnSelected ) {

      if ( mainWin->popupMsg->IsMime() ) {
	 isMime = True;
         if ( ContainsEnriched(mainWin->popupMsg->Tree()) )
	    enriched = True;
         else if ( ContainsRich(mainWin->popupMsg->Tree()) )
	    richtext = True;
      }

   } else {

      VItemListC&	selList = mainWin->msgTBox->get_selected();
      unsigned		selCount = selList.size();

      for (int i=0; !richtext && !enriched && i<selCount; i++) {

	 MsgItemC	*item = (MsgItemC*)selList[i];
	 MsgC		*msg = item->Message();
	 if ( msg->IsMime() ) {
	    isMime = True;
	    if      ( ContainsEnriched(msg->Tree()) ) enriched = True;
	    else if ( ContainsRich    (msg->Tree()) ) richtext = True;
	 }
      }
   }

   printEnriched = False;
   printAsRich   = False;
   useForkIt     = False;
   if ( isMime ) {

      if      ( enriched ) printEnriched = True;
      else if ( richtext ) printAsRich   = True;

      MailcapC	*mcap = MailcapEntry("text", enriched ? "enriched" :
      					    (richtext ? "richtext" : "plain"));
      if ( mcap && mcap->print.size()>0 ) {
	 cmd = mcap->print;
	 useForkIt = True;
      }
   }

   if ( cmd.size() == 0 ) {
      cmd = prefs->printCmd;
      useForkIt = False;
   }

   XmTextFieldSetString(cmdTF, cmd);

   HalDialogC::Show(parent);
}
#endif
