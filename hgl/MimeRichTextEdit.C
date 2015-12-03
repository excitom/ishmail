/*
 *  $Id: MimeRichTextEdit.C,v 1.3 2000/11/12 12:32:25 evgeny Exp $
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
#include "MimeRichTextC.h"
#include "MimeRichTextP.h"
#include "HalAppC.h"
#include "CharC.h"
#include "WArgList.h"
#include "RichSearchWinC.h"
#include "rsrc.h"

#include <Xm/AtomMgr.h>
#include <Xm/ScrollBar.h>
#include <Xm/CutPaste.h>

#include <X11/Xatom.h>
#include <X11/Xmu/Atoms.h>
#include <X11/keysym.h>

extern int	debuglev;

/*-----------------------------------------------------------------------
 *  Insert the character string at the current cursor position.
 */

void
MimeRichTextC::InsertString(CharC cs)
{
   priv->HideCursor();

//
// Clear the current selection
//
   if ( priv->selectOn ) priv->DrawSelection();

//
// Add a line record if necessary
//
   priv->inputLine = priv->cursorPos.textLine;
   if ( !priv->inputLine ) {
      priv->NewInputLine();
      priv->cursorPos.Set(priv->inputLine, 0, 0);
   }

//
// Point to the command under the cursor.
//
   priv->inputCmd = priv->cursorPos.Cmd();

//
// If the cursor is not at the end of a text command, split it
//
   TextPosC	newPos;
   if ( priv->inputCmd->IsText() &&
        priv->cursorPos.strPos < priv->inputCmd->LastPos()) {
      priv->SplitCommand(priv->cursorPos, &newPos);
      priv->cursorPos = newPos;
   }

//
// If we're not in a text command, add one
//
   else {

//
// See if the text should be inserted before or after the current command.
//
      Boolean	after = (priv->cursorPos.strPos>0);
      priv->InsertCommand(RC_TEXT, after, priv->cursorPos, &newPos);
      priv->inputCmd = newPos.Cmd();

   } // End if cursor is not on a text command

//
// Add string based on type
//
   TextLineC	*bline = priv->inputLine;

   priv->defer++;
   if ( priv->textType == TT_ENRICHED  ) AddStringEnriched(cs);
   else if ( priv->textType == TT_RICH ) AddStringRichtext(cs);
   else if ( priv->textType == TT_HTML ) AddStringHTML(cs);
   else				         AddStringPlain(cs);
   priv->defer--;

   if ( !priv->defer )
      priv->LinesChanged(bline, priv->inputLine);

//
// Keep the cursor visible
//
   priv->ScrollToCursor();
   ScreenPosC        spos = priv->cursorPos;
   priv->desiredCursorX = spos.x;

//
// Redisplay the current selection
//
   if ( priv->selectOn ) priv->DrawSelection();

   priv->ShowCursor();

} // End InsertString

/*-----------------------------------------------------------------------
 *  Delete the text in the selection range
 */

void
MimeRichTextP::DeleteSelection()
{
   if ( !editable ) {
      XBell(halApp->display, 0);
      return;
   }

   if ( !selectOn ) return;

   DrawSelection();
   selectOn = False;

//
// Only delete the text if the cursor is inside the selection
//
   if ( cursorPos >= selectBegPos && cursorPos <= selectEndPos )
      DeleteRange(selectBegPos, selectEndPos);

} // End DeleteSelection

/*---------------------------------------------------------------
 *  Move the cursor one character to the left
 */

void
MimeRichTextP::ActMoveLeftChar(Widget w, XKeyEvent*, String*, Cardinal*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);
   This->MoveLeftChar();
}

void
MimeRichTextC::MoveLeftChar()
{
   priv->HideCursor();

   if ( priv->selectOn ) priv->DrawSelection();
   priv->selectOn = False;

   TextPosC	newPos;
   if ( priv->FindPosPrevChar(priv->cursorPos, &newPos) ) {
      priv->cursorPos = newPos;
      priv->ScrollToCursor();
      ScreenPosC        spos = priv->cursorPos;
      priv->desiredCursorX = spos.x;
   }

   priv->ShowCursor();

} // End MoveLeftChar

/*---------------------------------------------------------------
 *  Move the cursor one word to the left.  Move to the first non-white
 *     character after the first white character or to the beginning
 *     of the line.
 */

void
MimeRichTextP::ActMoveLeftWord(Widget w, XKeyEvent*, String*, Cardinal*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);
   This->MoveLeftWord();
}

void
MimeRichTextC::MoveLeftWord()
{
   priv->HideCursor();

   if ( priv->selectOn ) priv->DrawSelection();
   priv->selectOn = False;

   TextPosC	newPos;
   if ( priv->FindPosPrevWord(priv->cursorPos, &newPos) ) {
      priv->cursorPos = newPos;
      priv->ScrollToCursor();
      ScreenPosC        spos = priv->cursorPos;
      priv->desiredCursorX = spos.x;
   }

   priv->ShowCursor();

} // End MoveLeftWord

/*---------------------------------------------------------------
 *  Move the cursor one character to the right
 */

void
MimeRichTextP::ActMoveRightChar(Widget w, XKeyEvent*, String*, Cardinal*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);
   This->MoveRightChar();
}

void
MimeRichTextC::MoveRightChar()
{
   priv->HideCursor();

   if ( priv->selectOn ) priv->DrawSelection();
   priv->selectOn = False;

   TextPosC	newPos;
   if ( priv->FindPosNextChar(priv->cursorPos, &newPos) ) {
      priv->cursorPos = newPos;
      priv->ScrollToCursor();
      ScreenPosC        spos = priv->cursorPos;
      priv->desiredCursorX = spos.x;
   }

   priv->ShowCursor();

} // End MoveRightChar

/*---------------------------------------------------------------
 *  Move the cursor one word to the right.  Move to the first whitespace
 *     character after the first non-white character.
 */

void
MimeRichTextP::ActMoveRightWord(Widget w, XKeyEvent*, String*, Cardinal*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);
   This->MoveRightWord();
}

void
MimeRichTextC::MoveRightWord()
{
   priv->HideCursor();

   if ( priv->selectOn ) priv->DrawSelection();
   priv->selectOn = False;

   TextPosC	newPos;
   if ( priv->FindPosNextWord(priv->cursorPos, &newPos) ) {
      priv->cursorPos = newPos;
      priv->ScrollToCursor();
      ScreenPosC        spos = priv->cursorPos;
      priv->desiredCursorX = spos.x;
   }

   priv->ShowCursor();

} // End MoveRightWord

/*---------------------------------------------------------------
 *  Move the cursor to the start of the line
 */

void
MimeRichTextP::ActMoveLineBeg(Widget w, XKeyEvent*, String*, Cardinal*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);
   This->MoveLineBeg();
}

void
MimeRichTextC::MoveLineBeg()
{
   priv->HideCursor();

   if ( priv->selectOn ) priv->DrawSelection();
   priv->selectOn = False;

   ScreenPosC	spos = priv->cursorPos;
   spos.Set(spos.softLine, spos.softLine->bounds.xmin);
   priv->cursorPos = spos;
   priv->ScrollToCursor();
   priv->desiredCursorX = spos.x;

   priv->ShowCursor();

} // End MoveLineBeg

/*---------------------------------------------------------------
 *  Move the cursor to the end of the line
 */

void
MimeRichTextP::ActMoveLineEnd(Widget w, XKeyEvent*, String*, Cardinal*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);
   This->MoveLineEnd();
}

void
MimeRichTextC::MoveLineEnd()
{
   priv->HideCursor();

   if ( priv->selectOn ) priv->DrawSelection();
   priv->selectOn = False;

   ScreenPosC	spos = priv->cursorPos;
   spos.Set(spos.softLine, spos.softLine->bounds.xmax);
   priv->cursorPos = spos;
   priv->ScrollToCursor();
   priv->desiredCursorX = spos.x;

   priv->ShowCursor();

} // End MoveLineEnd

/*---------------------------------------------------------------
 *  Move the cursor up one line
 */

void
MimeRichTextP::ActMoveUpLine(Widget w, XKeyEvent*, String*, Cardinal*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);
   This->MoveUpLine();
}

void
MimeRichTextC::MoveUpLine()
{
   priv->HideCursor();

   if ( priv->selectOn ) priv->DrawSelection();
   priv->selectOn = False;

   ScreenPosC	spos = priv->cursorPos;
   SoftLineC	*prev = spos.softLine->prev;
   if ( prev ) {
      spos.Set(prev, priv->desiredCursorX);
      priv->cursorPos = spos;
      priv->ScrollToCursor();
   }

   priv->ShowCursor();

} // End MoveUpLine

/*---------------------------------------------------------------
 *  Move the cursor up one paragraph
 */

void
MimeRichTextP::ActMoveUpPara(Widget w, XKeyEvent*, String*, Cardinal*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);
   This->MoveUpPara();
}

void
MimeRichTextC::MoveUpPara()
{
#if 0
   priv->HideCursor();

   if ( priv->selectOn ) priv->DrawSelection();
   priv->selectOn = False;

   priv->ShowCursor();
#else
   MoveUpLine();
#endif

} // End MoveUpPara

/*---------------------------------------------------------------
 *  Move the cursor down one line
 */

void
MimeRichTextP::ActMoveDownLine(Widget w, XKeyEvent*, String*, Cardinal*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);
   This->MoveDownLine();
}

void
MimeRichTextC::MoveDownLine()
{
   priv->HideCursor();

   if ( priv->selectOn ) priv->DrawSelection();
   priv->selectOn = False;

   ScreenPosC	spos = priv->cursorPos;
   SoftLineC	*next = spos.softLine->next;
   if ( next ) {
      spos.Set(next, priv->desiredCursorX);
      priv->cursorPos = spos;
      priv->ScrollToCursor();
   }

   priv->ShowCursor();

} // End MoveDownLine

/*---------------------------------------------------------------
 *  Move the cursor down one paragraph
 */

void
MimeRichTextP::ActMoveDownPara(Widget w, XKeyEvent*, String*, Cardinal*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);
   This->MoveDownPara();
}

void
MimeRichTextC::MoveDownPara()
{
#if 0
   priv->HideCursor();

   if ( priv->selectOn ) priv->DrawSelection();
   priv->selectOn = False;

   priv->ShowCursor();
#else
   MoveDownLine();
#endif

} // End MoveDownPara

/*---------------------------------------------------------------
 *  Move the cursor to the top of the file
 */

void
MimeRichTextP::ActMoveFileBeg(Widget w, XKeyEvent*, String*, Cardinal*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);
   This->MoveFileBeg();
}

void
MimeRichTextC::MoveFileBeg()
{
   priv->HideCursor();

   if ( priv->selectOn ) priv->DrawSelection();
   priv->selectOn = False;

   priv->cursorPos.Set(priv->topTextLine, 0, 0);
   priv->ScrollToCursor();
   priv->desiredCursorX = 0;

   priv->ShowCursor();

} // End MoveFileBeg

/*---------------------------------------------------------------
 *  Move the cursor to the bottom of the file
 */

void
MimeRichTextP::ActMoveFileEnd(Widget w, XKeyEvent*, String*, Cardinal*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);
   This->MoveFileEnd();
}

void
MimeRichTextC::MoveFileEnd()
{
   priv->HideCursor();

   if ( priv->selectOn ) priv->DrawSelection();
   priv->selectOn = False;

   if ( priv->botTextLine ) {

      TextLineC	*tline   = priv->botTextLine;
      unsigned	cmdCount = tline->cmdList.size();
      RichCmdC	*cmd     = tline->Cmd(cmdCount-1);
      unsigned	strSize  = cmd->LastPos();

      priv->cursorPos.Set(tline, cmdCount-1, strSize);
      priv->ScrollToCursor();
      ScreenPosC	spos = priv->cursorPos;
      priv->desiredCursorX = spos.x;
   }

   priv->ShowCursor();

} // End MoveFileEnd

/*---------------------------------------------------------------
 *  Delete the character to the left of the cursor
 */

void
MimeRichTextP::ActDeleteLeftChar(Widget w, XKeyEvent*, String*, Cardinal*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);
   This->DeleteLeftChar();
}

void
MimeRichTextC::DeleteLeftChar()
{
   if ( !priv->editable ) {
      XBell(halApp->display, 0);
      return;
   }

//
// Delete the current selection
//
   if ( priv->selectOn ) {
      priv->DeleteSelection();
      return;
   }

   TextPosC	newPos;
   if ( !priv->FindPosPrevChar(priv->cursorPos, &newPos) ) return;

   priv->DeleteRange(newPos, priv->cursorPos);

} // End DeleteLeftChar

/*---------------------------------------------------------------
 *  Delete the word to the left of the cursor
 */

void
MimeRichTextP::ActDeleteLeftWord(Widget w, XKeyEvent*, String*, Cardinal*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);
   This->DeleteLeftWord();
}

void
MimeRichTextC::DeleteLeftWord()
{
   if ( !priv->editable ) {
      XBell(halApp->display, 0);
      return;
   }

//
// Delete the current selection
//
   if ( priv->selectOn ) {
      priv->DeleteSelection();
      return;
   }

   TextPosC	newPos;
   if ( !priv->FindPosPrevWord(priv->cursorPos, &newPos) ) return;

   priv->DeleteRange(newPos, priv->cursorPos);

} // End DeleteLeftWord

/*---------------------------------------------------------------
 *  Delete the character to the right of the cursor
 */

void
MimeRichTextP::ActDeleteRightChar(Widget w, XKeyEvent*, String*, Cardinal*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);
   This->DeleteRightChar();
}

void
MimeRichTextC::DeleteRightChar()
{
   if ( !priv->editable ) {
      XBell(halApp->display, 0);
      return;
   }

//
// Delete the current selection
//
   if ( priv->selectOn ) {
      priv->DeleteSelection();
      return;
   }

   TextPosC	newPos;
   if ( !priv->FindPosNextChar(priv->cursorPos, &newPos) ) return;

   priv->DeleteRange(priv->cursorPos, newPos);

} // End DeleteRightChar

/*---------------------------------------------------------------
 *  Delete the word to the right of the cursor
 */

void
MimeRichTextP::ActDeleteRightWord(Widget w, XKeyEvent*, String*, Cardinal*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);
   This->DeleteRightWord();
}

void
MimeRichTextC::DeleteRightWord()
{
   if ( !priv->editable ) {
      XBell(halApp->display, 0);
      return;
   }

//
// Delete the current selection
//
   if ( priv->selectOn ) {
      priv->DeleteSelection();
      return;
   }

   TextPosC	newPos;
   if ( !priv->FindPosNextWord(priv->cursorPos, &newPos) ) return;

   priv->DeleteRange(priv->cursorPos, newPos);

} // End DeleteRightWord

/*---------------------------------------------------------------
 *  Delete to the beginning of the line
 */

void
MimeRichTextP::ActDeleteLineBeg(Widget w, XKeyEvent*, String*, Cardinal*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);
   This->DeleteLineBeg();
}

void
MimeRichTextC::DeleteLineBeg()
{
   if ( !priv->editable ) {
      XBell(halApp->display, 0);
      return;
   }

//
// Delete the current selection
//
   if ( priv->selectOn ) {
      priv->DeleteSelection();
      return;
   }

   ScreenPosC	sp = priv->cursorPos;
   sp.Set(sp.softLine, sp.softLine->bounds.xmin);
   TextPosC	newPos = sp;

   priv->DeleteRange(newPos, priv->cursorPos);

} // End DeleteLineBeg

/*---------------------------------------------------------------
 *  Delete to the end of the line
 */

void
MimeRichTextP::ActDeleteLineEnd(Widget w, XKeyEvent*, String*, Cardinal*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);
   This->DeleteLineEnd();
}

void
MimeRichTextC::DeleteLineEnd()
{
   if ( !priv->editable ) {
      XBell(halApp->display, 0);
      return;
   }

//
// Delete the current selection
//
   if ( priv->selectOn ) {
      priv->DeleteSelection();
      return;
   }

   ScreenPosC	sp = priv->cursorPos;
   sp.Set(sp.softLine, sp.softLine->bounds.xmax);
   TextPosC	newPos = sp;

   priv->DeleteRange(priv->cursorPos, newPos);

} // End DeleteLineEnd

/*---------------------------------------------------------------
 *  Delete the current selection
 */

void
MimeRichTextP::ActDeleteSelection(Widget w, XKeyEvent*, String*, Cardinal*)
{
   ActCutSelection(w, NULL, NULL, NULL);

   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);

//
// Clear out the clipboard
//
   XmClipboardUndoCopy(halApp->display, XtWindow(This->priv->textDA));

} // End ActDeleteSelection

/*---------------------------------------------------------------
 *  Delete the current selection and place a copy on the clipboard
 */

void
MimeRichTextP::ActCutSelection(Widget w, XKeyEvent*, String*, Cardinal*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);

   if ( !This->priv->editable ) {
      XBell(halApp->display, 0);
      return;
   }

   if ( !This->priv->selectOn ) return;

//
// Force the cursor to the beginning of the selection so that the text gets
//   deleted.  It only gets cleared if the cursor is not in it.
//
   This->priv->HideCursor();
   This->priv->cursorPos = This->priv->selectBegPos;
   This->priv->DeleteSelection();
   This->priv->ShowCursor();

//
// We don't need to worry about the clipboard because the item was copied
//    there when it was selected.
//
} // End ActCutSelection

/*---------------------------------------------------------------
 *  Delete the text between the 2 positions
 */

void
MimeRichTextP::DeleteRange(TextPosC& pos1, TextPosC& pos2)
{
   if ( !editable ) {
      XBell(halApp->display, 0);
      return;
   }

   if ( pos1 == pos2 ) return;

   TextPosC	begPos = pos1;
   TextPosC	endPos = pos2;
   if ( pos1 > pos2 ) {
      begPos = pos2;
      endPos = pos1;
   }

   HideCursor();

//
// Erase the current selection
//
   if ( selectOn ) {
      DrawSelection();
      selectOn = False;
   }

//
// Prepare to save the stuff to be deleted so that we can recover
//
   TextLineC	*tline = topSaveLine;
   while ( tline ) {
      TextLineC	*next = tline->next;
      delete tline;
      tline = next;
   }
   lastDelPos = begPos;

//
// See if we're deleting on just one line or spanning more than one
//
   Boolean	forcePlace;
   if ( begPos.textLine == endPos.textLine ) {

//
// Create a text line for the saved data
//
      TextLineC	*saveLine = new TextLineC;
      saveLine->DeleteCmd(0);
      topSaveLine = botSaveLine = saveLine;

//
// See if we're deleting within just one command
//
      if ( begPos.cmdPos == endPos.cmdPos ) {

//
// Delete between the 2 character positions
//
	 if ( begPos.Cmd()->IsText() ) {
	    int	len = endPos.strPos - begPos.strPos;
	    MoveText(begPos, len, saveLine);
	    FixPosAfterDelText(&endPos, begPos, len);
	 }

	 else {
	    begPos.Cmd()->graphic->Hide();
	    MoveCommand(begPos, saveLine);
	    FixPosAfterDelCmd(&begPos, begPos);
	    FixPosAfterDelCmd(&endPos, begPos);
	 }

      } // End if deleting within one text command

//
// We're deleting more than one command
//
      else {

//
// Delete all commands between the beginning and end commands (not inclusive)
//
	 int	begDelPos = begPos.cmdPos + 1;
	 int	endDelPos = endPos.cmdPos - 1;

	 TextPosC	delPos = begPos;
	 for (int i=endDelPos; i>=begDelPos; i--) {
	    delPos.cmdPos = i;
	    if ( delPos.Cmd()->IsGraphic() ) delPos.Cmd()->graphic->Hide();
	    MoveCommand(delPos, saveLine, 0);
	    FixPosAfterDelCmd(&endPos, delPos);
	 }

//
// Delete to the end of the beginning command.
//
	 if ( begPos.strPos == 0 ) {
	    if ( begPos.Cmd()->IsGraphic() ) begPos.Cmd()->graphic->Hide();
	    MoveCommand(begPos, saveLine, 0);
	    FixPosAfterDelCmd(&begPos, begPos);
	    FixPosAfterDelCmd(&endPos, begPos);
	 }
	 else if ( begPos.Cmd()->IsText() ) {
	    int	len = begPos.Cmd()->text->size() - begPos.strPos;
	    MoveText(begPos, len, saveLine, 0);
	    FixPosAfterDelText(&endPos, begPos, len);
	 }

//
// Delete from the beginning of the end command
//
	 if ( endPos.Cmd()->IsText() ) {
	    int	len = endPos.strPos;
	    endPos.strPos = 0;
	    MoveText(endPos, len, saveLine);
	 }
	 else if ( endPos.strPos > 0 ) {
	    endPos.Cmd()->graphic->Hide();
	    MoveCommand(endPos, saveLine);
	    FixPosAfterDelCmd(&endPos, endPos);
	 }

      } // End if deleting more than one text command

      forcePlace = False;

   } // End if deleting within one line

   else {

//
// Create lines for saving
//
      TextLineC	*begSaveLine = new TextLineC;
      TextLineC	*endSaveLine = new TextLineC;
      begSaveLine->DeleteCmd(0);
      endSaveLine->DeleteCmd(0);

//
// Delete all commands in the beginning line after the beginning command
//
      RichCmdC	*cmd;
      TextPosC	delPos = begPos;
      unsigned	count = begPos.textLine->cmdList.size();
      int i=count-1; for (i=count-1; i>begPos.cmdPos; i--) {
	 delPos.cmdPos = i;
	 cmd = delPos.Cmd();
	 if ( cmd->IsGraphic() ) cmd->graphic->Hide();
	 MoveCommand(delPos, begSaveLine, 0);
      }

//
// Delete to the end of the beginning command.
//
      cmd = begPos.Cmd();
      if ( begPos.strPos == 0 ) {
	 if ( cmd->IsGraphic() ) cmd->graphic->Hide();
	 MoveCommand(begPos, begSaveLine, 0);
	 FixPosAfterDelCmd(&begPos, begPos);
      }
      else if ( cmd->IsText() ) {
	 int	len = cmd->text->size() - begPos.strPos;
	 MoveText(begPos, len, begSaveLine, 0);
      }

//
// Delete all commands in the end line before the end command
//
      delPos = endPos;
      for (i=endPos.cmdPos-1; i>=0; i--) {
	 delPos.cmdPos = i;
	 cmd = delPos.Cmd();
	 if ( cmd->IsGraphic() ) cmd->graphic->Hide();
	 MoveCommand(delPos, endSaveLine, 0);
	 FixPosAfterDelCmd(&endPos, delPos);
      }

//
// Delete from the beginning of the end command
//
      cmd = endPos.Cmd();
      if ( cmd->IsText() ) {
	 int	len = endPos.strPos;
	 endPos.strPos = 0;
	 MoveText(endPos, len, endSaveLine);
      }
      else if ( endPos.strPos > 0 ) {
	 cmd->graphic->Hide();
	 MoveCommand(endPos, endSaveLine);
	 FixPosAfterDelCmd(&endPos, endPos);
      }

//
// Merge the end line with the beginning line
//
      MergeLines(begPos.textLine, endPos.textLine);

//
// Delete lines up to and including the end line
//
      topSaveLine = botSaveLine = begSaveLine;

      TextLineC	*delLine = begPos.textLine->next;
      TextLineC	*endLine = endPos.textLine;

      RemoveLines(delLine, endLine);

      while ( delLine && delLine != endLine ) {

	 u_int		ccount = delLine->cmdList.size();
	 for (int c=0; c<ccount; c++) {
	    RichCmdC	*delCmd = delLine->Cmd(c);
	    if ( delCmd->IsGraphic() ) delCmd->graphic->Hide();
	 }

	 botSaveLine->next = delLine;
	 delLine->prev = botSaveLine;
	 botSaveLine = delLine;

	 delLine = delLine->next;
	 botSaveLine->next = NULL;
      }

      DeleteLine(endLine);

      botSaveLine->next = endSaveLine;
      endSaveLine->prev = botSaveLine;
      botSaveLine = endSaveLine;
      botSaveLine->next = NULL;

      endPos = begPos;

      forcePlace = True;

   } // End if deleting on more than one line

//
// Make the line as efficient as possible.  The beginning line should be
//    the only remaining line.
//
   CompactLine(begPos.textLine);

   LineChanged(begPos.textLine, forcePlace);

//
// Keep the cursor visible
//
   ScrollToCursor();
   ScreenPosC	spos = cursorPos;
   desiredCursorX = spos.x;

   ShowCursor();

} // End DeleteRange

/*---------------------------------------------------------------
 * In the specified line, remove any blank text blocks and merge any
 *    text blocks with the same state.
 */

void
MimeRichTextP::CompactLine(TextLineC *line)
{
   TextPosC	curPos(line, 0, 0);
   u_int	count;
   int		i;

// If we take this code out, excerpting is not drawn correctly
#if 1
//
// Remove any blank text blocks, making sure to keep at least one.
//
   RichCmdC	*cmd;
   count = line->cmdList.size();
   for (i=0; i<count; ) {

      curPos.cmdPos = i;
      cmd = curPos.Cmd();

      if ( cmd->IsText() && cmd->text->size() == 0 && count > 1 ) {
	 DeleteCommand(curPos);
	 count--;
      }

      else
	 i++;

   } // End for each remaining command
#endif

//
// Merge any text blocks with the same state.
//
   count = line->cmdList.size();

   TextPosC	prevPos(line, 0, 0);
   curPos = prevPos;
   for (i=1; i<count; ) {

      prevPos.cmdPos = i-1;
      curPos.cmdPos  = i;
      if ( prevPos.Cmd()->IsText() && curPos.Cmd()->IsText() &&
	   prevPos.Cmd()->state == curPos.Cmd()->state ) {

//
// Add the beginning and end strings together
//
	 MergeCommands(prevPos, curPos);
	 DeleteCommand(curPos);
	 count--;

      } // End if states are the same

      else {
	 i++;
      }

   } // End for each state command

} // End CompactLine

/*---------------------------------------------------------------
 * Make the selection range as tight as possible.
 */

void
MimeRichTextP::CompactSelection()
{
//
// If the beginning position is at the end of a non-blank string, we can
//    move to the start of the next one
//
   RichCmdC	*cmd = selectBegPos.Cmd();
   TextPosC	newPos;
   if ( selectBegPos.strPos > 0 && selectBegPos.strPos >= cmd->LastPos() &&
	FindPosNextCmd(selectBegPos, &newPos) ) {
      selectBegPos = newPos;
   }

//
// If the end position is at the beginning of a non-blank string, we can
//    move to the end of the previous one
//
   cmd = selectEndPos.Cmd();
   if ( selectEndPos.strPos == 0 && cmd->LastPos() > 0 &&
	FindPosPrevCmd(selectEndPos, &newPos) ) {
      selectEndPos = newPos;
   }

   if      ( cursorPos <= selectBegPos ) cursorPos = selectBegPos;
   else if ( cursorPos >= selectEndPos ) cursorPos = selectEndPos;

} // End CompactSelection

/*---------------------------------------------------------------
 *  Handle general key press
 */

void
MimeRichTextP::ActInsertSelf(Widget w, XKeyEvent *ev, String*, Cardinal*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);

//
// Translate event into characters
//
   char		buf[BUFSIZ];
   KeySym	keysym;
   Status	status;
   int	bufSize = XmImMbLookupString(w, ev, buf, BUFSIZ-1, &keysym, &status);
   if ( status == XBufferOverflow ) {
      char		*altbuf = new char[bufSize+1];
      bufSize = XmImMbLookupString(w, ev, buf, bufSize, &keysym, &status);
   }
   if ( status == XLookupNone ) return;

   if ( bufSize <= 0 ) return;
   buf[bufSize] = 0;

//
// If editable is False, TAB and NEWLINE will cause traversal.
//
   if ( !This->priv->editable ) {
      if ( buf[0] == '\t' ) {
	 if ( ev->state & ShiftMask )
	    XmProcessTraversal(This->priv->textDA, XmTRAVERSE_PREV_TAB_GROUP);
	 else
	    XmProcessTraversal(This->priv->textDA, XmTRAVERSE_NEXT_TAB_GROUP);
      }
      else if ( buf[0] == '\n' || buf[0] == '\r' )
	 XmProcessTraversal(This->priv->textDA, XmTRAVERSE_NEXT_TAB_GROUP);
      else
	 XBell(halApp->display, 0);
      return;
   }

//
// If tabTraverses is True, TAB will cause traversal.
//
   else if ( buf[0] == '\t' && This->priv->tabTraverses ) {
      if ( ev->state & ShiftMask )
	 XmProcessTraversal(This->priv->textDA, XmTRAVERSE_PREV_TAB_GROUP);
      else
	 XmProcessTraversal(This->priv->textDA, XmTRAVERSE_NEXT_TAB_GROUP);
      return;
   }

//
// If singleLine is True, NEWLINE will cause traversal.
//
   else if ( buf[0] == '\n' || buf[0] == '\r' && This->priv->singleLine ) {
      XmProcessTraversal(This->priv->textDA, XmTRAVERSE_NEXT_TAB_GROUP);
      return;
   }

//
// This will catch the escape key.  I'm still not sure why we get an event
//    for this.
//
   if ( keysym == XK_Escape ) return;

   This->priv->HideCursor();

//
// Delete the current selection
//
   if ( This->priv->selectOn ) This->priv->DeleteSelection();

//
// Add a line record if necessary
//
   TextLineC	*tline = This->priv->cursorPos.textLine;
   if ( !tline ) {
      tline = new TextLineC;
      This->priv->cursorPos.Set(tline, 0, 0);
   }

//
// Point to the command under the cursor.
//
   RichCmdC	*curCmd = This->priv->cursorPos.Cmd();
   TextPosC	newPos;

//
// If this isn't a text command, insert one
//
   if ( curCmd->IsGraphic() ) {

//
// See if the text should be inserted before or after the graphic
//
      Boolean	after = (This->priv->cursorPos.strPos>0);
      This->priv->InsertCommand(RC_TEXT, after, This->priv->cursorPos, &newPos);
      This->priv->cursorPos = newPos;
      curCmd = This->priv->cursorPos.Cmd();

   } // End if cursor is not on a text command

//
// If this is a line break, split the line at the cursor position
//
   TextLineC	*newLine = NULL;
   if ( buf[0] == '\n' || buf[0] == '\r' ) {

      This->priv->InsertLineBreak(This->priv->cursorPos, &newPos);
      newLine = newPos.textLine;

//
// If the new line or the old line is blank, remove any excerpting
//
      if ( newLine->IsBlank() )
	 newLine->RemoveExcerpt();
      else if ( This->priv->cursorPos.textLine->IsBlank() )
	 This->priv->cursorPos.textLine->RemoveExcerpt();
   }

   else {

//
// Insert new text into the current text segment
//
      if ( This->priv->cursorPos.strPos < curCmd->LastPos() )
	 (*curCmd->text)(This->priv->cursorPos.strPos, 0) = buf;
      else
	 *curCmd->text += buf;

      This->priv->cursorPos.strPos += bufSize;
   }

   This->priv->LinesChanged(tline, newLine ? newLine : tline,
			    newLine != NULL);

//
// Keep the cursor in the window
//
   This->priv->ScrollToCursor();
   ScreenPosC        spos = This->priv->cursorPos;
   This->priv->desiredCursorX = spos.x;

   This->priv->ShowCursor();

} // End ActInsertSelf

#if 0
/*----------------------------------------------------------------------
 * Action proc to insert cut buffer at cursor position
 */

void
MimeRichTextP::ActPaste(Widget w, XButtonEvent*, String*, Cardinal*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);

   if ( !This->priv->editable ) {
      XBell(halApp->display, 0);
      return;
   }

   This->priv->InsertSavedLines(This->priv->cursorPos);

} // End ActPaste
#endif

/*---------------------------------------------------------------
 *  Redraw the window
 */

void
MimeRichTextP::ActRefresh(Widget w, XKeyEvent*, String*, Cardinal*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);

   RectC area(0, 0, This->priv->drawWd, This->priv->drawHt);
   This->priv->DrawScreen(area);

} // End ActRefresh

/*---------------------------------------------------------------
 *  Restore the last deletion
 */

void
MimeRichTextP::ActUndo(Widget w, XKeyEvent*, String*, Cardinal*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);
   This->Undelete();
}

void
MimeRichTextC::Undelete()
{
   if ( !priv->editable ) {
      XBell(halApp->display, 0);
      return;
   }

   priv->InsertSavedLines(priv->lastDelPos);
}

/*---------------------------------------------------------------
 *  Insert the saved lines at the specified position
 */

void
MimeRichTextP::InsertSavedLines(TextPosC insPos)
{
   if ( !topSaveLine ) {
      XBell(halApp->display, 0);
      return;
   }

#if 0
   if ( debuglev > 2 ) {
      cout <<"Inserting saved lines at pos " <<insPos.cmdPos <<", "
					     <<insPos.strPos
	   <<" in line " <<insPos.textLine <<endl;
      if ( insPos.textLine )
	  cout <<"   next line is " <<insPos.textLine->next <<endl;
   }
#endif

   HideCursor();

//
// Clear the current selection
//
   if ( selectOn ) DrawSelection();

//
// If there is more than one saved line, insert a line break at the insertion
//    position.  If there is only one saved line, split the command at
//    the insertion position.
//
   int		dstCmdPos;
   TextPosC	newPos;
   if ( topSaveLine != botSaveLine ) {
//      if ( debuglev > 2 ) cout <<"   inserting line break" <<endl;
      InsertLineBreak(insPos, &newPos);
      dstCmdPos = insPos.cmdPos + 1;
   }
   else {
//      if ( debuglev > 2 ) cout <<"   splitting command" <<endl;
      SplitCommand(insPos, &newPos);
      dstCmdPos = newPos.cmdPos;
   }

   cursorPos = newPos;

//
// For the first saved line, add the commands to the end of the line before
//   the break
//
   TextLineC	*dstLine = insPos.textLine;
   TextLineC	*tline   = topSaveLine;
   unsigned	ccount   = tline->cmdList.size();
   RichCmdC	*cmd;
   int c=0; for (c=0; c<ccount; c++) {

      cmd = tline->Cmd(c);
      if ( cmd->IsGraphic() ) cmd->graphic->Show();
//      if ( debuglev > 2 ) cout <<"   adding command to first line" <<endl;
      dstLine->AddCommand(cmd, dstCmdPos);
      dstCmdPos++;

   } // End for each command

//
// Make the first line as efficient as possible.
//
//   if ( debuglev > 2 ) cout <<"   compacting first line" <<endl;
   CompactLine(dstLine);
   TextLineC	*begLine = dstLine;

//
// For the in-between lines, just move the line to the line list.
//
   RichCmdC	*lastCmd;
   TextLineC	*nline;
   tline = topSaveLine->next;
   while ( tline && tline != botSaveLine ) {

//
// Turn on graphics in line
//
      ccount = tline->cmdList.size();
      for (c=0; c<ccount; c++) {
	 cmd = tline->Cmd(c);
	 if ( cmd->IsGraphic() ) cmd->graphic->Show();
      }

      nline = tline->next;

//      if ( debuglev > 2 ) cout <<"   adding additional line" <<endl;
      AddLine(tline, dstLine);
      dstLine = tline;
      lastCmd = dstLine->Cmd(ccount-1);

      tline = nline;

   } // End for each intermediate line

//
// For the last line of several, we'll add the commands to the beginning
//    of the line after the break
//
   if ( topSaveLine != botSaveLine ) {

      tline     = botSaveLine/*->prev*/;
      dstLine   = newPos.textLine;
      dstCmdPos = 0;

//
// Loop through the commands in this line
//
      ccount = tline->cmdList.size();
      for (c=0; c<ccount; c++) {

	 cmd = tline->Cmd(c);
	 if ( cmd->IsGraphic() ) cmd->graphic->Show();
//	 if ( debuglev > 2 ) cout <<"   adding command to last line" <<endl;
	 dstLine->AddCommand(cmd, dstCmdPos);
	 dstCmdPos++;

      } // End for each command

//
// Make the last line as efficient as possible.
//
//      if ( debuglev > 2 ) cout <<"   compacting last line" <<endl;
      CompactLine(dstLine);

   } // End if the last line needs processing

   TextLineC	*endLine = dstLine;

   topSaveLine = botSaveLine = NULL;

   LinesChanged(begLine, endLine, True/*force layout*/);

//
// Keep the cursor visible
//
   ScrollToCursor();
   ScreenPosC	spos = cursorPos;
   desiredCursorX = spos.x;

//
// Restore the current selection
//
   if ( selectOn ) DrawSelection();

   ShowCursor();

} // End InsertSavedLines

/*---------------------------------------------------------------
 *  Scroll the window up one line
 */

void
MimeRichTextP::ActScrollUpLine(Widget w, XKeyEvent*, String*, Cardinal*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);
   This->ScrollUpLine();
}

void
MimeRichTextC::ScrollUpLine()
{
   if ( priv->textVSB ) {
      int	val, ss, i, pi;
      XmScrollBarGetValues(priv->textVSB, &val, &ss, &i, &pi);
      if ( priv->vsbVal > 0 ) {
	 val = priv->vsbVal - i;
	 if ( val < 0 ) val = 0;
	 XmScrollBarSetValues(priv->textVSB, val, ss, i, pi, True);
      }
   }

#if 0
   ScreenPosC	spos = priv->cursorPos;

//
// Find the first non-visible line above this one
//
   SoftLineC	*line = spos.softLine;
   if ( !line ) return;

   line = line->prev;
   while ( line && priv->LineFullyVisible(line) ) line = line->prev;

   if ( !line ) return;

//
// Scroll to make the line visible
//
   spos.Set(line, priv->desiredCursorX);
   priv->ScrollToPosition(spos);

//
// If the cursor went off the screen, move it up
//
   spos = priv->cursorPos;
   line = spos.softLine;
   while ( line && !priv->LineFullyVisible(line) ) line = line->prev;
   if ( line && line != spos.softLine ) {
      spos.Set(line, priv->desiredCursorX);
      priv->HideCursor();
      priv->cursorPos = spos;
      priv->ShowCursor();
   }
#endif

} // End ScrollUpLine

/*---------------------------------------------------------------
 *  Scroll the window up one page
 */

void
MimeRichTextP::ActScrollUpPage(Widget w, XKeyEvent*, String*, Cardinal*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);
   This->ScrollUpPage();
}

void
MimeRichTextC::ScrollUpPage()
{
   if ( priv->textVSB ) {
      int	val, ss, i, pi;
      XmScrollBarGetValues(priv->textVSB, &val, &ss, &i, &pi);
      if ( priv->vsbVal > 0 ) {
	 val = priv->vsbVal - pi;
	 if ( val < 0 ) val = 0;
	 XmScrollBarSetValues(priv->textVSB, val, ss, i, pi, True);
      }
   }

} // End ScrollUpPage

/*---------------------------------------------------------------
 *  Scroll the window down one line
 */

void
MimeRichTextP::ActScrollDownLine(Widget w, XKeyEvent*, String*, Cardinal*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);
   This->ScrollDownLine();
}

void
MimeRichTextC::ScrollDownLine()
{
   if ( priv->textVSB ) {
      int	val, ss, i, pi;
      XmScrollBarGetValues(priv->textVSB, &val, &ss, &i, &pi);
      int	max = priv->vsbMax - ss;
      if ( priv->vsbVal < max ) {
	 val = priv->vsbVal + i;
	 if ( val > max ) val = max;
	 XmScrollBarSetValues(priv->textVSB, val, ss, i, pi, True);
      }
   }

#if 0
   ScreenPosC	spos = priv->cursorPos;

//
// Find the first non-visible line below this one
//
   SoftLineC	*line = spos.softLine;
   if ( !line ) return;

   line = line->next;
   while ( line && priv->LineFullyVisible(line) ) line = line->next;

   if ( !line ) return;

//
// Scroll to make the line visible
//
   spos.Set(line, priv->desiredCursorX);
   priv->ScrollToPosition(spos);

//
// If the cursor went off the screen, move it down
//
   spos = priv->cursorPos;
   line = spos.softLine;
   while ( line && !priv->LineFullyVisible(line) ) line = line->next;
   if ( line && line != spos.softLine ) {
      spos.Set(line, priv->desiredCursorX);
      priv->HideCursor();
      priv->cursorPos = spos;
      priv->ShowCursor();
   }
#endif

} // End ScrollDownLine

/*---------------------------------------------------------------
 *  Scroll the window down one page
 */

void
MimeRichTextP::ActScrollDownPage(Widget w, XKeyEvent*, String*, Cardinal*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);
   This->ScrollDownPage();
}

void
MimeRichTextC::ScrollDownPage()
{
   if ( priv->textVSB ) {
      int	val, ss, i, pi;
      XmScrollBarGetValues(priv->textVSB, &val, &ss, &i, &pi);
      int	max = priv->vsbMax - ss;
      if ( priv->vsbVal < max ) {
	 val = priv->vsbVal + pi;
	 if ( val > max ) val = max;
	 XmScrollBarSetValues(priv->textVSB, val, ss, i, pi, True);
      }
   }

} // End ScrollDownPage

/*---------------------------------------------------------------
 *  Scroll the window to the top
 */

void
MimeRichTextP::ActScrollTop(Widget w, XKeyEvent*, String*, Cardinal*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);
   This->ScrollTop();
}

void
MimeRichTextC::ScrollTop()
{
   if ( priv->textVSB ) {
      int	val, ss, i, pi;
      XmScrollBarGetValues(priv->textVSB, &val, &ss, &i, &pi);
      XmScrollBarSetValues(priv->textVSB, 0, ss, i, pi, True);
   }

#if 0
//   if ( !priv->topSoftLine ) return;

//
// Make the top position visible
//
   ScreenPosC	spos(priv->topSoftLine, 0);
   priv->ScrollToPosition(spos);

//
// If the cursor went off the screen, move it to the top position
//
   spos = priv->cursorPos;
   SoftLineC*	line = spos.softLine;
   if ( line && !priv->LineFullyVisible(line) && priv->topTextLine ) {
      priv->HideCursor();
      priv->cursorPos.Set(priv->topTextLine, 0, 0);
      priv->ScrollToCursor();
      priv->desiredCursorX = 0;
      priv->ShowCursor();
   }
#endif

} // End ScrollTop

/*---------------------------------------------------------------
 *  Scroll the window to the bottom
 */

void
MimeRichTextP::ActScrollBottom(Widget w, XKeyEvent*, String*, Cardinal*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);
   This->ScrollBottom();
}

void
MimeRichTextC::ScrollBottom()
{
   if ( priv->textVSB ) {
      int	val, ss, i, pi;
      XmScrollBarGetValues(priv->textVSB, &val, &ss, &i, &pi);
      int	max = priv->vsbMax - ss;
      XmScrollBarSetValues(priv->textVSB, max, ss, i, pi, True);
   }

#if 0
//   if ( !priv->botSoftLine ) return;

//
// Make the bottom position visible
//
   ScreenPosC	spos(priv->botSoftLine, 0);
   priv->ScrollToPosition(spos);

//
// If the cursor went off the screen, move it to the bottom position
//
   spos = priv->cursorPos;
   SoftLineC	*line = spos.softLine;
   if ( line && !priv->LineFullyVisible(line) && priv->botTextLine ) {

      TextLineC	*tline   = priv->botTextLine;
      unsigned	cmdCount = tline->cmdList.size();
      RichCmdC	*cmd     = tline->Cmd(cmdCount-1);
      unsigned	strSize  = cmd->LastPos();

      priv->HideCursor();
      priv->cursorPos.Set(tline, cmdCount-1, strSize);
      priv->ScrollToCursor();
      ScreenPosC	spos = priv->cursorPos;
      priv->desiredCursorX = spos.x;
      priv->ShowCursor();
   }
#endif

} // End ScrollBottom

/*---------------------------------------------------------------
 *  Scroll the current line to the top of the window
 */

void
MimeRichTextP::ActLineToTop(Widget w, XKeyEvent*, String*, Cardinal*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);
   This->LineToTop();
}

void
MimeRichTextC::LineToTop()
{
   priv->HideCursor();

   priv->ShowCursor();

} // End LineToTop

/*---------------------------------------------------------------
 *  Scroll the current line to the center of the window
 */

void
MimeRichTextP::ActLineToCenter(Widget w, XKeyEvent*, String*, Cardinal*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);
   This->LineToCenter();
}

void
MimeRichTextC::LineToCenter()
{
   priv->HideCursor();

   priv->ShowCursor();

} // End LineToCenter

/*---------------------------------------------------------------
 *  Scroll the current line to the bottom of the window
 */

void
MimeRichTextP::ActLineToBottom(Widget w, XKeyEvent*, String*, Cardinal*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);
   This->LineToBottom();
}

void
MimeRichTextC::LineToBottom()
{
   priv->HideCursor();

   priv->ShowCursor();

} // End LineToBottom

/*---------------------------------------------------------------
 *  Make the specified font change to the current selection or at the cursor
 *  If fcmd is FC_PLAIN, remove all markup.
 */

void
MimeRichTextC::ChangeFont(FontCmdT fcmd)
{
   priv->ChangeFont(fcmd);
}

void
MimeRichTextP::ChangeFont(FontCmdT fcmd)
{
   if ( !editable ) {
      XBell(halApp->display, 0);
      return;
   }

   HideCursor();

   TextPosC	newPos;

//
// If there is a current selection, make the change to that.  Otherwise, insert
//    the command at the cursor position.
//
   if ( selectOn ) {

//
// Save copies of the selection positions.
//
      TextPosC	begPos;
      TextPosC	endPos;
      if ( selectBegPos > selectEndPos ) {
	 begPos = selectEndPos;
	 endPos = selectBegPos;
      }
      else {
	 begPos = selectBegPos;
	 endPos = selectEndPos;
      }

      Boolean	correct;

//
// If we're making the text bigger or smaller, we do so regardless of the
//    current state
//
      if ( fcmd != FC_BIGGER && fcmd != FC_SMALLER ) {

//
// If the beginning state is already correct, advance the beginning state until
//    it's not
//
	 if ( fcmd == FC_PLAIN )
	    correct = (begPos.Cmd()->state.FontStack().size() == 0 &&
	               begPos.Cmd()->state.Underline() == 0);
	 else if ( fcmd == FC_UNDERLINE )
	    correct = (begPos.Cmd()->state.Underline() > 0);
	 else
	    correct = (begPos.Cmd()->state.CmdCount(fcmd) > 0);

	 while ( correct && begPos < endPos &&
		 FindPosNextCmd(begPos, &newPos) ) {

	    begPos = newPos;

	    if ( fcmd == FC_PLAIN )
	       correct = (begPos.Cmd()->state.FontStack().size() == 0 &&
			  begPos.Cmd()->state.Underline() == 0);
	    else if ( fcmd == FC_UNDERLINE )
	       correct = (begPos.Cmd()->state.Underline() > 0);
	    else
	       correct = (begPos.Cmd()->state.CmdCount(fcmd) > 0);
	 }

      } // End if current state is important

//
// If the beginning position is at the end of a non-blank string, we can
//    move to the start of the next one
//
      if ( begPos.strPos > 0 && begPos.strPos >= begPos.Cmd()->LastPos() &&
	   FindPosNextCmd(begPos, &newPos) ) {
	 begPos = newPos;
      }

      if ( fcmd != FC_BIGGER && fcmd != FC_SMALLER ) {

//
// If the end state is already correct, move the end state until
//    it's not
//
	 if ( fcmd == FC_PLAIN )
	    correct = (endPos.Cmd()->state.FontStack().size() == 0 &&
	               endPos.Cmd()->state.Underline() == 0);
	 else if ( fcmd == FC_UNDERLINE )
	    correct = (endPos.Cmd()->state.Underline() > 0);
	 else
	    correct = (endPos.Cmd()->state.CmdCount(fcmd) > 0);

	 while ( correct && endPos > begPos &&
		 FindPosPrevCmd(endPos, &newPos) ) {

	    endPos = newPos;

	    if ( fcmd == FC_PLAIN )
	       correct = (endPos.Cmd()->state.FontStack().size() == 0 &&
			  endPos.Cmd()->state.Underline() == 0);
	    else if ( fcmd == FC_UNDERLINE )
	       correct = (endPos.Cmd()->state.Underline() > 0);
	    else
	       correct = (endPos.Cmd()->state.CmdCount(fcmd) > 0);
	 }

      } // End if current state is important

//
// If the end position is at the beginning of a non-blank string, we can
//    move to the end of the previous one
//
      if ( endPos.strPos == 0 && endPos.Cmd()->LastPos() > 0 &&
	   FindPosPrevCmd(endPos, &newPos) ) {
	 endPos = newPos;
      }

//
// If there's no text in-between, we're done
//
      if ( begPos >= endPos ) {
	 ShowCursor();
	 return;
      }

//
// Clear the selection
//
      DrawSelection();	// To clear it
      selectOn = False;

//
// If the beginning position is not the beginning of a command, split
//    the command.
//
      if ( begPos.strPos != 0 ) {
	 SplitCommand(begPos, &newPos);
	 FixPosAfterSplit(&endPos, begPos);
	 begPos = newPos;
      }

//
// If the end position is not the end of a command, split the command.
//
      if ( endPos.strPos < endPos.Cmd()->LastPos() )
	 SplitCommand(endPos, &newPos);

//
// For each command between the beginning and end (inclusive), make the
//    font change
//
      TextPosC	curPos = begPos;
      Boolean	done = False;
      while ( !done && curPos <= endPos ) {

	 RichCmdC	*cmd = curPos.Cmd();

	 if ( fcmd == FC_PLAIN ) {
	    cmd->state.FontStack().Clear();
	    cmd->state.Underline(0);
	 }

	 else if ( fcmd == FC_UNDERLINE )
	    cmd->state.UnderlineMore();

	 else {

//
// If we want "bigger" and the current command is "smaller", or if we want
//    "smaller" and the current command is "bigger", just pop it.
// Otherwise push the new command
//
	    if ( (fcmd == FC_BIGGER && cmd->state.CurFontCmd() == FC_SMALLER) ||
		 (fcmd == FC_SMALLER && cmd->state.CurFontCmd() == FC_BIGGER) )
	       cmd->state.PopFont();

	    else
	       cmd->state.PushFont(fcmd);

	 } // End if font is not plain

	 done = !FindPosNextCmd(curPos, &newPos);
	 curPos = newPos;

      } // End for each command changing

//
// Update all affected lines
//
      LinesChanged(begPos.textLine, endPos.textLine);

//
// Turn the selection back on
//
      CompactSelection();
      DrawSelection();
      selectOn = True;

   } // End if selectOn

//
// Since there is no current selection, insert a command at the cursor
//    position.
//
   else {

//
// See what the text state is at the cursor position
//
      RichCmdC	*cmd = cursorPos.Cmd();

//
// If it's not already correct, add a blank string with the desired state
//
      Boolean	correct;
      if ( fcmd == FC_BIGGER || fcmd == FC_SMALLER )
	 correct = False;
      else if ( fcmd == FC_PLAIN )
	 correct = (cmd->state.FontStack().size() == 0 &&
		    cmd->state.Underline() == 0);
      else if ( fcmd == FC_UNDERLINE )
	 correct = (cmd->state.Underline() > 0);
      else
	 correct = (cmd->state.CmdCount(fcmd) > 0);

      if ( !correct ) {

//
// If the cursor is not in a blank text string, create one.
//
	 if ( cmd->IsText() && cmd->LastPos() > 0 ) {

	    TextPosC	oldPos = cursorPos;
	    SplitCommand(cursorPos, &newPos);

//
// Splitting may have left one of the two parts blank.  If so, use that part.
//
	    if ( cursorPos.Cmd()->LastPos() == 0 ) {
	       selectBegPos =
	       selectEndPos = cursorPos;
	    }

	    else if ( newPos.Cmd()->LastPos() == 0 ) {
	       cursorPos    =
	       selectBegPos =
	       selectEndPos = newPos;
	    }

	    else if ( oldPos.Cmd()->LastPos() == 0 ) {
	       cursorPos    =
	       selectBegPos =
	       selectEndPos = oldPos;
	    }

//
// If no part is blank, insert a blank command
//
	    else { 

	       InsertCommand(RC_TEXT, True/*after*/, oldPos, &newPos);

	       cursorPos    =
	       selectBegPos =
	       selectEndPos = newPos;
	    }

	    cmd = cursorPos.Cmd();

	 } // End if a new command is needed

//
// The cursor should now be in a blank text command
//
	 if ( fcmd == FC_PLAIN ) {
	    cmd->state.FontStack().Clear();
	    cmd->state.Underline(0);
	 }
	 else if ( fcmd == FC_UNDERLINE )
	    cmd->state.UnderlineMore();
	 else if ((fcmd == FC_BIGGER  && cmd->state.CurFontCmd() == FC_SMALLER)
	       || (fcmd == FC_SMALLER && cmd->state.CurFontCmd() == FC_BIGGER))
	    cmd->state.PopFont();
	 else
	    cmd->state.PushFont(fcmd);

//
// Update the current line
//
	 LineChanged(cursorPos.textLine);

      } // End if not already correct

   } // End if no current selection

//
// Keep the cursor in the window
//
   ScrollToCursor();
   ScreenPosC        spos = cursorPos;
   desiredCursorX = spos.x;

   ShowCursor();

} // End ChangeFont

/*---------------------------------------------------------------
 *  Make the specified character set change to the current selection
 *     or at the cursor.
 */

void
MimeRichTextC::ChangeCharset(CharC cs)
{
   priv->ChangeCharset(cs);
}

void
MimeRichTextP::ChangeCharset(CharC cs)
{
   int	num = pub->AddCharset(cs);

   switch (num) {
      case 1:	ChangeFont(FC_CHARSET_1);	break;
      case 2:	ChangeFont(FC_CHARSET_2);	break;
      case 3:	ChangeFont(FC_CHARSET_3);	break;
      case 4:	ChangeFont(FC_CHARSET_4);	break;
      case 5:	ChangeFont(FC_CHARSET_5);	break;
      case 6:	ChangeFont(FC_CHARSET_6);	break;
      case 7:	ChangeFont(FC_CHARSET_7);	break;
      case 8:	ChangeFont(FC_CHARSET_8);	break;
      case 9:	ChangeFont(FC_CHARSET_9);	break;
   }

} // End ChangeCharset

/*---------------------------------------------------------------
 *  Make the specified color change to the current selection or at the cursor
 *  If the color is the same as the foreground color, remove all color markup.
 */

void
MimeRichTextC::ChangeColor(StringC name)
{
   priv->ChangeColor(name);
}

void
MimeRichTextP::ChangeColor(StringC name)
{
   if ( !editable ) {
      XBell(halApp->display, 0);
      return;
   }

   HideCursor();

   TextPosC	newPos;
   Pixel	value = 0;
   PixelValue(textDA, name, &value);

//
// If there is a current selection, make the change to that.  Otherwise, insert
//    the command at the cursor position.
//
   if ( selectOn ) {

//
// Save copies of the selection positions.
//
      TextPosC	begPos;
      TextPosC	endPos;
      if ( selectBegPos > selectEndPos ) {
	 begPos = selectEndPos;
	 endPos = selectBegPos;
      }
      else {
	 begPos = selectBegPos;
	 endPos = selectEndPos;
      }

      Boolean	correct;

//
// If the beginning state is already correct, advance the beginning state until
//    it's not
//
      if ( name.Equals("none", IGNORE_CASE) )
	 correct = (begPos.Cmd()->state.ColorStack().size() == 0);
      else
	 correct = (begPos.Cmd()->state.ColorCount(value) > 0);

      while ( correct && begPos < endPos &&
	      FindPosNextCmd(begPos, &newPos) ) {

	 begPos = newPos;

	 if ( name.Equals("none", IGNORE_CASE) )
	    correct = (begPos.Cmd()->state.ColorStack().size() == 0);
	 else
	    correct = (begPos.Cmd()->state.ColorCount(value) > 0);
      }

//
// If the beginning position is at the end of a non-blank string, we can
//    move to the start of the next one
//
      if ( begPos.strPos > 0 && begPos.strPos >= begPos.Cmd()->LastPos() &&
	   FindPosNextCmd(begPos, &newPos) ) {
	 begPos = newPos;
      }

//
// If the end state is already correct, move the end state until
//    it's not
//
      if ( name.Equals("none", IGNORE_CASE) )
	 correct = (endPos.Cmd()->state.ColorStack().size() == 0);
      else
	 correct = (endPos.Cmd()->state.ColorCount(value) > 0);

      while ( correct && endPos > begPos &&
	      FindPosPrevCmd(endPos, &newPos) ) {

	 endPos = newPos;

	 if ( name.Equals("none", IGNORE_CASE) )
	    correct = (endPos.Cmd()->state.ColorStack().size() == 0);
	 else
	    correct = (endPos.Cmd()->state.ColorCount(value) > 0);
      }

//
// If the end position is at the beginning of a non-blank string, we can
//    move to the end of the previous one
//
      if ( endPos.strPos == 0 && endPos.Cmd()->LastPos() > 0 &&
	   FindPosPrevCmd(endPos, &newPos) ) {
	 endPos = newPos;
      }

//
// If there's no text in-between, we're done
//
      if ( begPos >= endPos ) {
	 ShowCursor();
	 return;
      }

//
// Clear the selection
//
      DrawSelection();	// To clear it
      selectOn = False;

//
// If the beginning position is not the beginning of a command, split
//    the command.
//
      if ( begPos.strPos != 0 ) {
	 SplitCommand(begPos, &newPos);
	 FixPosAfterSplit(&endPos, begPos);
	 begPos = newPos;
      }

//
// If the end position is not the end of a command, split the command.
//
      if ( endPos.strPos < endPos.Cmd()->LastPos() )
	 SplitCommand(endPos, &newPos);

//
// For each command between the beginning and end (inclusive), make the
//    color change
//
      TextPosC	curPos = begPos;
      Boolean	done = False;
      while ( !done && curPos <= endPos ) {

	 RichCmdC	*cmd = curPos.Cmd();

	 if ( name.Equals("none", IGNORE_CASE) )
	    cmd->state.ColorStack().removeAll();
	 else
	    cmd->state.PushColor(value);

	 done = !FindPosNextCmd(curPos, &newPos);
	 curPos = newPos;

      } // End for each command changing

//
// Update all affected lines
//
      LinesChanged(begPos.textLine, endPos.textLine);

//
// Turn the selection back on
//
      CompactSelection();
      DrawSelection();
      selectOn = True;

   } // End if selectOn

//
// Since there is no current selection, insert a command at the cursor
//    position.
//
   else {

//
// See what the text state is at the cursor position
//
      RichCmdC	*cmd = cursorPos.Cmd();

//
// If it's not already correct, add a blank string with the desired state
//
      Boolean	correct;
      if ( name.Equals("none", IGNORE_CASE) )
	 correct = (cmd->state.ColorStack().size() == 0);
      else
	 correct = (cmd->state.ColorCount(value) > 0);

      if ( !correct ) {

//
// If the cursor is not in a blank text string, create one.
//
	 if ( cmd->IsText() && cmd->LastPos() > 0 ) {

	    TextPosC	oldPos = cursorPos;
	    SplitCommand(cursorPos, &newPos);

//
// Splitting may have left one of the two parts blank.  If so, use that part.
//
	    if ( cursorPos.Cmd()->LastPos() == 0 ) {
	       selectBegPos =
	       selectEndPos = cursorPos;
	    }

	    else if ( newPos.Cmd()->LastPos() == 0 ) {
	       cursorPos    =
	       selectBegPos =
	       selectEndPos = newPos;
	    }

	    else if ( oldPos.Cmd()->LastPos() == 0 ) {
	       cursorPos    =
	       selectBegPos =
	       selectEndPos = oldPos;
	    }

//
// If no part is blank, insert a blank command
//
	    else { 

	       InsertCommand(RC_TEXT, True/*after*/, oldPos, &newPos);

	       cursorPos    =
	       selectBegPos =
	       selectEndPos = newPos;
	    }

	    cmd = cursorPos.Cmd();

	 } // End if a new command is needed

//
// The cursor should now be in a blank text command
//
	 if ( name.Equals("none", IGNORE_CASE) )
	    cmd->state.ColorStack().removeAll();
	 else
	    cmd->state.PushColor(value);

//
// Update the current line
//
	 LineChanged(cursorPos.textLine);

      } // End if not already correct

   } // End if no current selection

//
// Keep the cursor in the window
//
   ScrollToCursor();
   ScreenPosC        spos = cursorPos;
   desiredCursorX = spos.x;

   ShowCursor();

} // End ChangeColor

/*---------------------------------------------------------------
 *  Remove all markup from the current selection or at the cursor
 */

void
MimeRichTextP::ActPlain(Widget w, XKeyEvent*, String*, Cardinal*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);
   This->ChangeFont(FC_PLAIN);
}

/*---------------------------------------------------------------
 *  Make the current selection Bold or start Bold at the cursor
 */

void
MimeRichTextP::ActBold(Widget w, XKeyEvent*, String*, Cardinal*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);
   This->ChangeFont(FC_BOLD);
}

/*---------------------------------------------------------------
 *  Make the current selection italic or start italic at the cursor
 */

void
MimeRichTextP::ActItalic(Widget w, XKeyEvent*, String*, Cardinal*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);
   This->ChangeFont(FC_ITALIC);
}

/*---------------------------------------------------------------
 *  Make the current selection Fixed or start Fixed at the cursor
 */

void
MimeRichTextP::ActFixed(Widget w, XKeyEvent*, String*, Cardinal*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);
   This->ChangeFont(FC_FIXED);
}

/*---------------------------------------------------------------
 *  Make the current selection Smaller or start Smaller at the cursor
 */

void
MimeRichTextP::ActSmaller(Widget w, XKeyEvent*, String*, Cardinal*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);
   This->ChangeFont(FC_SMALLER);
}

/*---------------------------------------------------------------
 *  Make the current selection Bigger or start Bigger at the cursor
 */

void
MimeRichTextP::ActBigger(Widget w, XKeyEvent*, String*, Cardinal*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);
   This->ChangeFont(FC_BIGGER);
}

/*---------------------------------------------------------------
 *  Make the current selection Underline or start Underline at the cursor
 */

void
MimeRichTextP::ActUnderline(Widget w, XKeyEvent*, String*, Cardinal*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);
   This->ChangeFont(FC_UNDERLINE);
}

/*---------------------------------------------------------------
 *  Make the specified justification change to the current selection or
 *     at the cursor.
 */

void
MimeRichTextC::ChangeJust(JustCmdT jcmd)
{
   priv->ChangeJust(jcmd);
}

void
MimeRichTextP::ChangeJust(JustCmdT jcmd)
{
   if ( !editable ) {
      XBell(halApp->display, 0);
      return;
   }

   HideCursor();

   TextPosC	newPos;

//
// If there is a current selection, make the change to that.  Otherwise, change
//    the line containing the cursor.
//
   TextPosC	begPos;
   TextPosC	endPos;
   if ( selectOn ) {

//
// Save copies of the selection positions.
//
      if ( selectBegPos > selectEndPos ) {
	 begPos = selectEndPos;
	 endPos = selectBegPos;
      }
      else {
	 begPos = selectBegPos;
	 endPos = selectEndPos;
      }

   } // End if there is a selection

   else {

//
// Determine the text positions at the beginning and end of the current line
//
      ScreenPosC	spos = cursorPos;

      ScreenPosC	bpos(spos.softLine, 0);
      ScreenPosC	epos(spos.softLine, spos.softLine->bounds.xmax);

      begPos = bpos;
      endPos = epos;

   } // End if there is no selection

   Boolean		correct;

//
// If we're excerpting more, we do so regardless of the current state.
//
   if ( jcmd != JC_EXCERPT_MORE ) {

//
// If the beginning state is already correct, advance the beginning state until
//    it's not
//
      if ( jcmd == JC_EXCERPT_LESS )
	 correct = (begPos.Cmd()->state.Excerpt() == 0);
      else
	 correct = (begPos.Cmd()->state.CmdCount(jcmd) > 0);

      while ( correct && begPos < endPos &&
	      FindPosNextCmd(begPos, &newPos) ) {

	 begPos = newPos;

	 if ( jcmd == JC_EXCERPT_LESS )
	    correct = (begPos.Cmd()->state.Excerpt() == 0);
	 else
	    correct = (begPos.Cmd()->state.CmdCount(jcmd) > 0);
      }

   } // End if current state is important

//
// If the beginning position is at the end of a non-blank string, we can
//    move to the start of the next one
//
   if ( begPos.strPos > 0 && begPos.strPos >= begPos.Cmd()->LastPos() &&
	FindPosNextCmd(begPos, &newPos) ) {
      begPos = newPos;
   }

   if ( jcmd != JC_EXCERPT_MORE ) {

//
// If the end state is already correct, move the end state until
//    it's not
//
      if ( jcmd == JC_EXCERPT_LESS )
	 correct = (endPos.Cmd()->state.Excerpt() == 0);
      else
	 correct = (endPos.Cmd()->state.CmdCount(jcmd) > 0);

      while ( correct && endPos > begPos &&
	      FindPosPrevCmd(endPos, &newPos) ) {

	 endPos = newPos;

	 if ( jcmd == JC_EXCERPT_LESS )
	    correct = (endPos.Cmd()->state.Excerpt() == 0);
	 else
	    correct = (endPos.Cmd()->state.CmdCount(jcmd) > 0);
      }

   } // End if current state is important

//
// If the end position is at the beginning of a non-blank string, we can
//    move to the end of the previous one
//
   if ( endPos.strPos == 0 && endPos.Cmd()->LastPos() > 0 &&
	FindPosPrevCmd(endPos, &newPos) ) {
      endPos = newPos;
   }

//
// If there's no text in-between, we're done
//
   if ( begPos > endPos ) {
      ShowCursor();
      return;
   }

//
// Clear the selection
//
   Boolean	needSelect = selectOn;
   if ( selectOn ) {
      DrawSelection();	// To clear it
      selectOn = False;
   }

//
// If the beginning position is not the beginning of a text command, split
//    the command.
//
   if ( begPos.strPos != 0 ) {
      SplitCommand(begPos, &newPos);
      FixPosAfterSplit(&endPos, begPos);
      begPos = newPos;
   }

//
// If the end position is not the end of a text command, split the command.
//
   if ( endPos.strPos < endPos.Cmd()->LastPos() )
      SplitCommand(endPos, &newPos);

   TextLineC	*bline = begPos.textLine;
   TextLineC	*eline = endPos.textLine;

//
// If the beginning position is not the beginning of a line, insert a line
//    break
//
   if ( begPos.cmdPos != 0 ) {
      InsertLineBreak(begPos, &newPos);
      FixPosAfterBreak(&endPos, begPos, newPos.textLine);
      begPos = newPos;
      eline = endPos.textLine;
   }

//
// If the end position is not the end of a line, insert a line break
//
   if ( endPos.cmdPos != endPos.textLine->cmdList.size()-1 ) {
      InsertLineBreak(endPos, &newPos);
      eline = newPos.textLine;
   }

//
// For each command between the beginning and end (inclusive), make the
//    change
//
   TextPosC	curPos = begPos;
   Boolean	done = False;
   while ( !done && curPos <= endPos ) {

      RichCmdC	*cmd  = curPos.Cmd();

      if ( jcmd == JC_EXCERPT_MORE )
	 cmd->state.ExcerptMore();
      else if ( jcmd == JC_EXCERPT_LESS ) {
	 cmd->state.ExcerptLess();
      }
      else {
	 cmd->state.JustStack().Clear();
	 cmd->state.PushJust(jcmd);
      }

      done = !FindPosNextCmd(curPos, &newPos);
      curPos = newPos;
   }

//
// Update all affected lines
//
   LinesChanged(bline, eline, True/*reposition*/);

//
// Turn the selection back on
//
   if ( needSelect ) {

      selectBegPos = begPos;
      selectEndPos = endPos;

      CompactSelection();
      DrawSelection();
      selectOn = True;
   }

#if 0
//
// Since there is no current selection, insert a command at the cursor
//    position.
//
   else {

//
// See what the text state is at the cursor position
//
      RichCmdC	*cmd = cursorPos.Cmd();

//
// If it's not already correct, add a blank line with the desired state
//
      Boolean	correct = False;
      if ( jcmd == JC_EXCERPT_LESS )
	 correct = (cmd->state.Excerpt() == 0);
      else
	 correct = (cmd->state.CmdCount(jcmd) > 0);

      if ( !correct ) {

         TextLineC	*bline = cursorPos.textLine;

//
// If the cursor is at the beginning or end of the current line, we only need
//    to make one break.
//
	 Boolean atBeg = (cursorPos.cmdPos == 0 && cursorPos.strPos == 0);
	 Boolean atEnd =
	    (cursorPos.cmdPos == cursorPos.textLine->cmdList.size()-1 &&
	     cursorPos.strPos >= cursorPos.Cmd()->LastPos()-1);

//
// Add a line break
//
	 TextPosC	curPos = cursorPos;
	 InsertLineBreak(cursorPos, &newPos);

//
// If the cursor isn't already at the blank line, move it there
//
	 if ( !atBeg ) curPos = newPos;

//
// If we started in the middle add another line break
//
	 if ( !(atBeg || atEnd) )
	    InsertLineBreak(curPos, &newPos);

	 cursorPos = curPos;

         TextLineC	*eline = newPos.textLine;

//
// Make the change in the blank line
//
	 cmd = cursorPos.Cmd();

	 if ( jcmd == JC_EXCERPT_MORE )
	    cmd->state.ExcerptMore();
	 else if ( jcmd == JC_EXCERPT_LESS ) {
	    cmd->state.ExcerptLess();
	 }
	 else {
	    cmd->state.JustStack().Clear();
	    cmd->state.PushJust(jcmd);
	 }

	 selectBegPos =
	 selectEndPos = cursorPos;

//
// Update the new line(s)
//
	 LinesChanged(bline, eline, True/*reposition*/);

      } // End if not already correct

   } // End if no current selection
#endif

//
// Keep the cursor in the window
//
   ScrollToCursor();
   ScreenPosC        spos = cursorPos;
   desiredCursorX = spos.x;

   ShowCursor();

} // End ChangeJust

/*---------------------------------------------------------------
 *  Center all the lines in the selection or the line under the cursor
 */

void
MimeRichTextP::ActCenter(Widget w, XKeyEvent*, String*, Cardinal*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);
   This->ChangeJust(JC_CENTER);
}

/*---------------------------------------------------------------
 *  Left justify all the lines in the selection or the line under the cursor
 */

void
MimeRichTextP::ActFlushLeft(Widget w, XKeyEvent*, String*, Cardinal*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);
   This->ChangeJust(JC_LEFT);
}

/*---------------------------------------------------------------
 *  Right justify all the lines in the selection or the line under the cursor
 */

void
MimeRichTextP::ActFlushRight(Widget w, XKeyEvent*, String*, Cardinal*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);
   This->ChangeJust(JC_RIGHT);
}

/*---------------------------------------------------------------
 *  Fully justify all the lines in the selection or the line under the cursor
 */

void
MimeRichTextP::ActFlushBoth(Widget w, XKeyEvent*, String*, Cardinal*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);
   This->ChangeJust(JC_BOTH);
}

/*---------------------------------------------------------------
 *  Don't format the lines in the selection or the line under the cursor
 */

void
MimeRichTextP::ActNoFill(Widget w, XKeyEvent*, String*, Cardinal*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);
   This->ChangeJust(JC_NOFILL);
}

/*---------------------------------------------------------------
 *  Make the specified margin change to the current selection or
 *     at the cursor.
 */

void
MimeRichTextC::ChangeMargin(MarginCmdT mcmd)
{
   priv->ChangeMargin(mcmd);
}

void
MimeRichTextP::ChangeMargin(MarginCmdT mcmd)
{
   if ( !editable ) {
      XBell(halApp->display, 0);
      return;
   }

   HideCursor();

   TextPosC	newPos;

//
// If there is a current selection, make the change to that.  Otherwise, insert
//    the command at the cursor position.
//
   if ( selectOn ) {

//
// Save copies of the selection positions.
//
      TextPosC	begPos;
      TextPosC	endPos;
      if ( selectBegPos > selectEndPos ) {
	 begPos = selectEndPos;
	 endPos = selectBegPos;
      }
      else {
	 begPos = selectBegPos;
	 endPos = selectEndPos;
      }

      Boolean		correct;

//
// If we're moving the margins in, we do so regardless of the current state
//
      if ( mcmd != MC_LEFT_IN && mcmd != MC_RIGHT_IN ) {

//
// If the beginning state is already correct, advance the beginning state until
//    it's not
//
	 if ( mcmd == MC_LEFT_OUT )
	    correct = (begPos.Cmd()->state.LIndent() == 0);
	 else if ( mcmd == MC_RIGHT_OUT )
	    correct = (begPos.Cmd()->state.RIndent() == 0);

	 while ( correct && begPos < endPos &&
		 FindPosNextCmd(begPos, &newPos) ) {

	    begPos = newPos;

	    if ( mcmd == MC_LEFT_OUT )
	       correct = (begPos.Cmd()->state.LIndent() == 0);
	    else if ( mcmd == MC_RIGHT_OUT )
	       correct = (begPos.Cmd()->state.RIndent() == 0);
	 }

      } // End if current state is important

//
// If the beginning position is at the end of a non-blank string, we can
//    move to the start of the next one
//
      if ( begPos.strPos > 0 && begPos.strPos >= begPos.Cmd()->LastPos() &&
	   FindPosNextCmd(begPos, &newPos) ) {
	 begPos = newPos;
      }

      if ( mcmd != MC_LEFT_IN && mcmd != MC_RIGHT_IN ) {

//
// If the end state is already correct, move the end state until
//    it's not
//
	 if ( mcmd == MC_LEFT_OUT )
	    correct = (endPos.Cmd()->state.LIndent() == 0);
	 else if ( mcmd == MC_RIGHT_OUT )
	    correct = (endPos.Cmd()->state.RIndent() == 0);

	 while ( correct && endPos > begPos &&
		 FindPosPrevCmd(endPos, &newPos) ) {

	    endPos = newPos;

	    if ( mcmd == MC_LEFT_OUT )
	       correct = (endPos.Cmd()->state.LIndent() == 0);
	    else if ( mcmd == MC_RIGHT_OUT )
	       correct = (endPos.Cmd()->state.RIndent() == 0);
	 }

      } // End if current state is important

//
// If the end position is at the beginning of a non-blank string, we can
//    move to the end of the previous one
//
      if ( endPos.strPos == 0 && endPos.Cmd()->LastPos() > 0 &&
	   FindPosPrevCmd(endPos, &newPos) ) {
	 endPos = newPos;
      }

//
// If there's no text in-between, we're done
//
      if ( begPos >= endPos ) {
	 ShowCursor();
	 return;
      }

//
// Clear the selection
//
      DrawSelection();	// To clear it
      selectOn = False;

//
// If the beginning position is not the beginning of a text command, split
//    the command
//
      if ( begPos.strPos != 0 ) {
	 SplitCommand(begPos, &newPos);
	 FixPosAfterSplit(&endPos, begPos);
	 begPos = newPos;
      }

//
// If the end position is not the end of a text command, split the command.
//
      if ( endPos.strPos < endPos.Cmd()->LastPos() )
	 SplitCommand(endPos, &newPos);

//
// For each command between the beginning and end (inclusive), make the
//    margin change
//
      TextPosC	curPos = begPos;
      Boolean	done = False;
      while ( !done && curPos <= endPos ) {

	 RichCmdC	*cmd  = curPos.Cmd();

	 if      ( mcmd == MC_LEFT_IN   ) cmd->state.LIndentMore();
	 else if ( mcmd == MC_LEFT_OUT  ) cmd->state.LIndentLess();
	 else if ( mcmd == MC_RIGHT_IN  ) cmd->state.RIndentMore();
	 else if ( mcmd == MC_RIGHT_OUT ) cmd->state.RIndentLess();

	 done = !FindPosNextCmd(curPos, &newPos);
	 curPos = newPos;
      }

//
// Update all affected lines
//
      LinesChanged(begPos.textLine, endPos.textLine);

//
// Turn the selection back on
//
      CompactSelection();
      DrawSelection();
      selectOn = True;

   } // End if selectOn

//
// Since there is no current selection, insert a command at the cursor
//    position.
//
   else {

//
// See what the text state is at the cursor position
//
      RichCmdC	*cmd = cursorPos.Cmd();

//
// If the cursor position is at the end of a non-blank string, we can
//    move to the start of the next one
//
      if ( cursorPos.strPos > 0 && cursorPos.strPos >= cmd->LastPos() &&
	   FindPosNextCmd(cursorPos, &newPos) ) {
	 cursorPos = newPos;
	 cmd = cursorPos.Cmd();
      }

//
// If it's not already correct, we might need to split the string
//
      Boolean	correct = False;
      if      ( mcmd == MC_LEFT_OUT  ) correct = (cmd->state.LIndent() == 0);
      else if ( mcmd == MC_RIGHT_OUT ) correct = (cmd->state.RIndent() == 0);

      if ( !correct ) {

//
// If the cursor is at the beginning of a command, change that one.
//    Otherwise, split it.
//
	 Boolean	cmdAdded = False;
	 if ( cursorPos.strPos > 0 ) {	// Must be text

	    SplitCommand(cursorPos, &newPos);
	    cursorPos = newPos;

	    selectBegPos =
	    selectEndPos = cursorPos;

	    cmd = cursorPos.Cmd();
	    cmdAdded = True;

	 } // End if a new command was added

	 Boolean	marginChanged = True;
	 if      ( mcmd == MC_LEFT_IN   ) cmd->state.LIndentMore();
	 else if ( mcmd == MC_RIGHT_IN  ) cmd->state.RIndentMore();
	 else if ( mcmd == MC_LEFT_OUT  ) {
	    if ( cmd->state.LIndent() > 0 ) cmd->state.LIndentLess();
	    else			    marginChanged = False;
	 }
	 else if ( mcmd == MC_RIGHT_OUT ) {
	    if ( cmd->state.RIndent() > 0 ) cmd->state.RIndentLess();
	    else			  marginChanged = False;
	 }

//
// Update the current line
//
	 if ( marginChanged || cmdAdded ) LineChanged(cursorPos.textLine);

      } // End if not already correct

   } // End if no current selection

//
// Keep the cursor in the window
//
   ScrollToCursor();
   ScreenPosC        spos = cursorPos;
   desiredCursorX = spos.x;

   ShowCursor();

} // End ChangeMargin

/*---------------------------------------------------------------
 *  Left indent the current selection or the line under the cursor
 */

void
MimeRichTextP::ActLeftMarginIn(Widget w, XKeyEvent*, String*, Cardinal*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);
   This->ChangeMargin(MC_LEFT_IN);
}

/*---------------------------------------------------------------
 *  Remove one level of left indent for the current selection or the
 *     line under the cursor
 */

void
MimeRichTextP::ActLeftMarginOut(Widget w, XKeyEvent*, String*, Cardinal*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);
   This->ChangeMargin(MC_LEFT_OUT);
}

/*---------------------------------------------------------------
 *  Right indent the current selection or the line under the cursor
 */

void
MimeRichTextP::ActRightMarginIn(Widget w, XKeyEvent*, String*, Cardinal*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);
   This->ChangeMargin(MC_RIGHT_IN);
}

/*---------------------------------------------------------------
 *  Remove one level of Right indent for the current selection or the
 *     line under the cursor
 */

void
MimeRichTextP::ActRightMarginOut(Widget w, XKeyEvent*, String*, Cardinal*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);
   This->ChangeMargin(MC_RIGHT_OUT);
}

/*---------------------------------------------------------------
 *  Excerpt the current selection or the line under the cursor
 */

void
MimeRichTextP::ActExcerptMore(Widget w, XKeyEvent*, String*, Cardinal*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);
   This->ChangeJust(JC_EXCERPT_MORE);
}

/*---------------------------------------------------------------
 *  Remove one level of excerpt for the current selection or the
 *     line under the cursor
 */

void
MimeRichTextP::ActExcerptLess(Widget w, XKeyEvent*, String*, Cardinal*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);
   This->ChangeJust(JC_EXCERPT_LESS);
}

/*---------------------------------------------------------------
 *  NULL action
 */

void
MimeRichTextP::ActIgnore(Widget, XKeyEvent*, String*, Cardinal*)
{
}

/*-----------------------------------------------------------------------
 *  Add a graphic object at the cursor position
 */

void
MimeRichTextC::InsertGraphic(RichGraphicC *rg)
{
   if ( !priv->editable ) {
      XBell(halApp->display, 0);
      return;
   }

   priv->HideCursor();

   TextLineC	*tline = priv->cursorPos.textLine;

//
// See if the cursor is in the middle of a text command
//
   RichCmdC	*cmd = tline->Cmd(priv->cursorPos.cmdPos);
   Boolean	atBeg = (priv->cursorPos.strPos == 0);
   Boolean	atEnd = (priv->cursorPos.strPos == cmd->LastPos());
   if ( cmd->IsText() && !atBeg && !atEnd ) {
   
//
// Split the current command.  The new command will be after the current.
//
      RichCmdC	*newCmd = new RichCmdC(RC_TEXT);
      tline->AddCommand(newCmd, priv->cursorPos.cmdPos+1);

      newCmd->state = cmd->state;
      *newCmd->text = *cmd->text;
      (*newCmd->text)(0,priv->cursorPos.strPos) = "";
      cmd->text->Clear(priv->cursorPos.strPos);

//
// Move the cursor to the new command
//
      priv->cursorPos.cmdPos++;
      priv->cursorPos.strPos = 0;
      atBeg = True;

   } // End if cursor in middle

//
// Add a graphic command at the cursor position
//
   RichCmdC	*newCmd = new RichCmdC(RC_GRAPHIC);
   newCmd->state   = cmd->state;
   newCmd->graphic = rg;

   rg->owner = priv;
   void	*tmp = (void*)rg;
   priv->graphicList.add(tmp);

   if ( atEnd ) priv->cursorPos.cmdPos++;
   tline->AddCommand(newCmd, priv->cursorPos.cmdPos);

//
// Move the cursor to the end of the graphic
//
   priv->cursorPos.strPos = 1;

//
// Update the current line
//
   priv->LineChanged(tline);

//
// Keep the cursor in the window
//
   priv->ScrollToCursor();
   ScreenPosC        spos = priv->cursorPos;
   priv->desiredCursorX = spos.x;

   priv->ShowCursor();

} // End AddGraphic

/*-----------------------------------------------------------------------
 *  Remove the specified graphic object
 */

void
MimeRichTextC::RemoveGraphic(RichGraphicC *graphic)
{
   TextLineC	*tline = priv->topTextLine;
   while ( tline ) {

      unsigned	ccount = tline->cmdList.size();
      int c=0; for (c=0; c<ccount; c++) {

	 RichCmdC	*cmd = tline->Cmd(c);
	 if ( cmd->IsGraphic() && cmd->graphic == graphic ) {
	    TextPosC	pos1(tline, c, 0);
	    TextPosC	pos2(tline, c, 1);
	    priv->DeleteRange(pos1, pos2);
	    return;
	 }
      }

      tline = tline->next;
   }

} // End RemoveGraphic

/*-----------------------------------------------------------------------
 *  Reformat the line containing the specified graphic object
 */

void
MimeRichTextC::GraphicChanged(RichGraphicC *graphic, Boolean sizeChanged)
{
   TextLineC	*tline = priv->topTextLine;
   Boolean	found = False;
   while ( !found && tline ) {

      unsigned	ccount = tline->cmdList.size();
      int c=0; for (c=0; !found && c<ccount; c++) {

	 RichCmdC	*cmd = tline->Cmd(c);
	 if ( cmd->IsGraphic() && cmd->graphic == graphic ) {

//
// Reformat the line
//
	    if ( sizeChanged )
	       priv->LineChanged(tline);

//
// Find the soft line that contains this graphic and just redraw it
//
	    else {

	       SoftLineC	*sline = tline->softLine;
	       while ( !found && sline && sline->textLine == tline ) {

		  RichDrawDataC	*data = sline->drawData;
		  while ( !found && data ) {

		     if ( data->graphic == graphic ) {
			priv->DrawLines(sline, sline);
			found = True;
		     }

		     data = data->next;
		  }

		  sline = sline->next;

	       } // End for each soft line

	    } // End if size didn't change

	    found = True;

	 } // End if graphic found

      } // End for each command

      tline = tline->next;

   } // End for each line

} // End GraphicChanged

/*---------------------------------------------------------------
 *  Display the search dialog
 */

void
MimeRichTextP::ActSearch(Widget w, XKeyEvent*, String*, Cardinal*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);
   This->priv->Search();
}

void
MimeRichTextP::Search()
{
   static RichSearchWinC	*searchWin = NULL;
   if ( !searchWin )
      searchWin = new RichSearchWinC(pub->MainWidget(), "richSearchWin", 0,0);

   searchWin->Show(this);

} // End Search
