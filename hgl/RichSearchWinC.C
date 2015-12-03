/*
 *  $Id: RichSearchWinC.C,v 1.3 2000/05/07 12:26:11 fnevgeny Exp $
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
#include "RichSearchWinC.h"
#include "MimeRichTextC.h"
#include "MimeRichTextP.h"
#include "RegexC.h"
#include "WArgList.h"
#include "HalAppC.h"
#include "TextMisc.h"
#include "rsrc.h"

#include <Xm/Form.h>
#include <Xm/PushB.h>
#include <Xm/TextF.h>
#include <Xm/Label.h>
#include <Xm/ToggleB.h>

/*---------------------------------------------------------------
 *  Constructor
 */

RichSearchWinC::RichSearchWinC(Widget parent, const char *name, ArgList argv,
			       Cardinal argc)
	      : HalDialogC(name, parent, argv, argc)
{
   rich = NULL;

   WArgList	args;

//
// Create appForm hierarchy
//
//   appForm
//      Label		stringLabel
//      TextField	stringTF
//      ToggleButton	caseTB
//      ToggleButton	backTB
//      ToggleButton	wildTB
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_NONE);
   Widget	stringLabel = XmCreateLabel(appForm, "stringLabel", ARGS);

   args.TopAttachment(XmATTACH_WIDGET, stringLabel);
   args.RightAttachment(XmATTACH_FORM);
   stringTF = CreateTextField(appForm, "stringTF", ARGS);

   args.Reset();
   args.TopAttachment(XmATTACH_WIDGET, stringTF);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_NONE);
   caseTB = XmCreateToggleButton(appForm, "caseTB", ARGS);
   args.TopAttachment(XmATTACH_WIDGET, caseTB);
   wildTB = XmCreateToggleButton(appForm, "wildTB", ARGS);
   args.TopAttachment(XmATTACH_WIDGET, wildTB);
   backTB = XmCreateToggleButton(appForm, "backTB", ARGS);

   XtManageChild(stringLabel);
   XtManageChild(stringTF);
   XtManageChild(caseTB);
   XtManageChild(wildTB);
   XtManageChild(backTB);

   XtAddCallback(caseTB, XmNvalueChangedCallback, (XtCallbackProc)ToggleCase,
   		 this);
   XtAddCallback(wildTB, XmNvalueChangedCallback, (XtCallbackProc)ToggleWild,
   		 this);

//
// Create buttons
//
   AddButtonBox();

   Widget	findPB = XmCreatePushButton(buttonRC, "findPB", 0,0);
   Widget	donePB = XmCreatePushButton(buttonRC, "donePB", 0,0);
   Widget	helpPB = XmCreatePushButton(buttonRC, "helpPB", 0,0);

   XtAddCallback(findPB, XmNactivateCallback, (XtCallbackProc)DoFind,	this);
   XtAddCallback(donePB, XmNactivateCallback, (XtCallbackProc)DoHide,	this);
   XtAddCallback(helpPB, XmNactivateCallback, (XtCallbackProc)HalAppC::DoHelp,
   		 (char *) "helpcard");

   XtManageChild(findPB);
   XtManageChild(donePB);
   XtManageChild(helpPB);

   ShowInfoMsg();		// Add message line

   HandleHelp();

} // End constructor

/*---------------------------------------------------------------
 *  Method to display the search window
 */

void
RichSearchWinC::Show(MimeRichTextP *text)
{
   rich = text;
   HalDialogC::Show(rich->pub->MainWidget());
}


void
RichSearchWinC::Show(Widget over)
{
   HalDialogC::Show(over);
}

void
RichSearchWinC::Show()
{
   HalDialogC::Show();
}

/*---------------------------------------------------------------
 *  Callback to toggle of case-sensitive and wildcard buttons.  Only
 *     one of the two can be selected.
 */

void
RichSearchWinC::ToggleCase(Widget, RichSearchWinC *This, XtPointer data)
{
   XmToggleButtonCallbackStruct	*tb = (XmToggleButtonCallbackStruct*)data;
   if ( tb->set ) XmToggleButtonSetState(This->wildTB, False, False);
}

void
RichSearchWinC::ToggleWild(Widget, RichSearchWinC *This, XtPointer data)
{
   XmToggleButtonCallbackStruct	*tb = (XmToggleButtonCallbackStruct*)data;
   if ( tb->set ) XmToggleButtonSetState(This->caseTB, False, False);
}

/*---------------------------------------------------------------
 *  Callback to handle press of find button
 */

void
RichSearchWinC::DoFind(Widget, RichSearchWinC *This, XtPointer)
{
   This->ClearMessage();

//
// Get pattern
//
   if ( XmTextFieldGetLastPosition(This->stringTF) <= 0 ) {
      set_invalid(This->stringTF, True, True);
      This->PopupMessage("Please enter a search string.");
      return;
   }

   char		*searchStr = XmTextFieldGetString  (This->stringTF);
   Boolean	checkCase  = XmToggleButtonGetState(This->caseTB);
   Boolean	wildcard   = XmToggleButtonGetState(This->wildTB);
   Boolean	reverse    = XmToggleButtonGetState(This->backTB);

   TextLineC	*bline = This->rich->cursorPos.textLine;
   Boolean	found = False;
   TextLineC	*line;

//
// If they entered a wildcard pattern, use a regular expression
//
   if ( wildcard ) {

      RegexC	pat(searchStr);

//
// Look forward or backward from the current cursor position
//
      if ( reverse ) {

//
// Check lines above cursor
//
	 line = bline;
	 while ( !found && line ) { 
	    found = This->RevSearchLine(line, pat, False/*before cursor*/);
	    line = line->prev;
	 }

//
// Wrap to bottom of text and check lines below cursor
//
	 line = This->rich->botTextLine;
	 while ( !found && line && line != bline ) { 
	    found = This->RevSearchLine(line, pat, True/*after cursor*/);
	    line = line->prev;
	 }

      } // End if looking backward

      else { // Looking forward

//
// Check lines below cursor
//
	 line = bline;
	 while ( !found && line ) {
	    found = This->SearchLine(line, pat, True/*after cursor*/);
	    line = line->next;
	 }

//
// Wrap to top of text and check lines above cursor
//
	 line = This->rich->topTextLine;
	 while ( !found && line && line != bline ) {
	    found = This->SearchLine(line, pat, False/*before cursor*/);
	    line = line->next;
	 }

      } // End if looking forward

   } // End if using a wildcard

   else { // Not using a wildcard

      CharC	str = searchStr;

//
// Look forward or backward from the current cursor position
//
      if ( reverse ) {

//
// Check lines above cursor
//
	 line = bline;
	 while ( !found && line ) {
	    found = This->RevSearchLine(line, str, False/*before cursor*/,	
	    				checkCase);
	    line = line->prev;
	 }

//
// Wrap to bottom of text and check lines below cursor
//
	 line = This->rich->botTextLine;
	 while ( !found && line && line != bline ) {
	    found = This->RevSearchLine(line, str, True/*after cursor*/,
	    				checkCase);
	    line = line->prev;
	 }

      } // End if looking backward

      else { // Looking forward

//
// Check lines below cursor
//
	 line = bline;
	 while ( !found && line ) {
	    found = This->SearchLine(line, str, True/*after cursor*/,
				     checkCase);
	    line = line->next;
	 }

//
// Wrap to top of text and check lines above cursor
//
	 line = This->rich->topTextLine;
	 while ( !found && line && line != bline ) {
	    found = This->SearchLine(line, str, False/*before cursor*/,
				     checkCase);
	    line = line->next;
	 }

      } // End if looking forward

   } // End if not using a wildcard

   XtFree(searchStr);

   if ( !found ) {
      This->Message("No match");
      XBell(halApp->display, 0);
   }

} // End DoFind

/*---------------------------------------------------------------
 *  Method to search forward in a line for the given regex
 */

Boolean
RichSearchWinC::SearchLine(TextLineC *line, RegexC& pat, Boolean afterCursor)
{
   Boolean	found = False;
   u_int	cmdCount = line->cmdList.size();
   for (int i=0; !found && i<cmdCount; i++) {

      RichCmdC	*cmd = line->Cmd(i);
      if ( !cmd->IsText() ) continue;

      int	startPos = 0;
      if ( line == rich->cursorPos.textLine ) {

//
// Ignore text to the left of the cursor
//
	 if ( afterCursor ) {

	    if ( i < rich->cursorPos.cmdPos )
	       continue;
	    else if ( i == rich->cursorPos.cmdPos )
	       startPos = rich->cursorPos.strPos;
	 }

//
// Ignore text to the right of the cursor
//
	 else if ( i > rich->cursorPos.cmdPos )
	    continue;
      }

//
// Look for pattern in current text segment
//
      if ( startPos < cmd->text->size() ) {
	 int	findPos = pat.search(*cmd->text, startPos);
	 found = (findPos >= 0);
	 if ( found ) {
	    TextPosC	begPos(line, i, findPos);
	    TextPosC	endPos(line, i, findPos + pat[0].length());
	    UpdateSelection(begPos, endPos, endPos);
	 }
      }

   } // End for each text string in line

   return found;

} // End SearchLine

/*---------------------------------------------------------------
 *  Method to search backward in a line for the given regex
 */

Boolean
RichSearchWinC::RevSearchLine(TextLineC *line, RegexC& pat, Boolean afterCursor)
{
   Boolean	found = False;
   u_int	cmdCount = line->cmdList.size();
   for (int i=cmdCount-1; !found && i>=0; i--) {

      RichCmdC	*cmd = line->Cmd(i);
      if ( !cmd->IsText() ) continue;

      int	startPos = cmd->text->size()-1;
      if ( line == rich->cursorPos.textLine ) {

//
// Ignore text to the right of the cursor
//
	 if ( !afterCursor ) {

	    if ( i > rich->cursorPos.cmdPos )
	       continue;
	    else if ( i == rich->cursorPos.cmdPos )
	       startPos = rich->cursorPos.strPos - 1;
	 }

//
// Ignore text to the right of the cursor
//
	 else if ( i < rich->cursorPos.cmdPos )
	    continue;
      }

//
// Look for pattern in current text segment
//
      if ( startPos > 0 ) {
	 int	findPos = pat.search(*cmd->text, startPos, -startPos);
	 found = (findPos >= 0);
	 if ( found ) {
	    TextPosC	begPos(line, i, findPos);
	    TextPosC	endPos(line, i, findPos + pat[0].length());
	    UpdateSelection(begPos, endPos, begPos);
	 }
      }

   } // End for each text string in line

   return found;

} // End RevSearchLine

/*---------------------------------------------------------------
 *  Method to search forward in a line for the given character string
 */

Boolean
RichSearchWinC::SearchLine(TextLineC *line, CharC& str, Boolean afterCursor,
			   Boolean checkCase)
{
   Boolean	found = False;
   u_int	cmdCount = line->cmdList.size();
   for (int i=0; !found && i<cmdCount; i++) {

      RichCmdC	*cmd = line->Cmd(i);
      if ( !cmd->IsText() ) continue;

      u_int	startPos = 0;
      if ( line == rich->cursorPos.textLine ) {

//
// Ignore text to the left of the cursor
//
	 if ( afterCursor ) {

	    if ( i < rich->cursorPos.cmdPos )
	       continue;
	    else if ( i == rich->cursorPos.cmdPos )
	       startPos = rich->cursorPos.strPos;
	 }

//
// Ignore text to the right of the cursor
//
	 else if ( i > rich->cursorPos.cmdPos )
	    continue;
      }

//
// Look for string in current text segment
//
      if ( startPos < cmd->text->size() ) {
	 int	findPos = cmd->text->PosOf(str, startPos, checkCase);
	 found = (findPos >= 0);
	 if ( found ) {
	    TextPosC	begPos(line, i, findPos);
	    TextPosC	endPos(line, i, findPos + str.Length());
	    UpdateSelection(begPos, endPos, endPos);
	 }
      }

   } // End for each text string in line

   return found;

} // End SearchLine

/*---------------------------------------------------------------
 *  Method to search backward in a line for the given character string
 */

Boolean
RichSearchWinC::RevSearchLine(TextLineC *line, CharC& str, Boolean afterCursor,
			      Boolean checkCase)
{
   Boolean	found = False;
   u_int	cmdCount = line->cmdList.size();
   for (int i=cmdCount-1; !found && i>=0; i--) {

      RichCmdC	*cmd = line->Cmd(i);
      if ( !cmd->IsText() ) continue;

      u_int	startPos = cmd->text->size();
      if ( line == rich->cursorPos.textLine ) {

//
// Ignore text to the right of the cursor
//
	 if ( !afterCursor ) {

	    if ( i > rich->cursorPos.cmdPos )
	       continue;
	    else if ( i == rich->cursorPos.cmdPos )
	       startPos = rich->cursorPos.strPos;
	 }

//
// Ignore text to the right of the cursor
//
	 else if ( i < rich->cursorPos.cmdPos )
	    continue;
      }

//
// Look for string in current text segment
//
      if ( startPos > 0 ) {
	 int	findPos = cmd->text->RevPosOf(str, startPos, checkCase);
	 found = (findPos >= 0);
	 if ( found ) {
	    TextPosC	begPos(line, i, findPos);
	    TextPosC	endPos(line, i, findPos + str.Length());
	    UpdateSelection(begPos, endPos, begPos);
	 }
      }

   } // End for each text string in line

   return found;

} // End RevSearchLine

/*---------------------------------------------------------------
 *  Method to update the selection in the rich text widget
 */

void
RichSearchWinC::UpdateSelection(TextPosC& begPos, TextPosC& endPos,
				TextPosC& newCursorPos)
{
   ScreenPosC	spos = begPos;

   rich->HideCursor();
   if ( rich->selectOn ) {
      rich->DrawSelection();
      rich->selectOn = False;
   }
   rich->selectBegPos   = begPos;
   rich->selectEndPos   = endPos;
   rich->cursorPos      = newCursorPos;
   rich->desiredCursorX = spos.x;
   rich->ScrollToCursor();
   rich->DrawSelection();
   rich->selectOn       = True;
   rich->ShowCursor();
}
