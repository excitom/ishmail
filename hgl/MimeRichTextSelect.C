/*
 *  $Id: MimeRichTextSelect.C,v 1.4 2000/12/25 15:05:52 evgeny Exp $
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
#include "rsrc.h"
#include "System.h"
#include "CharC.h"

#include <Xm/AtomMgr.h>
#include <Xm/CutPaste.h>

#include <X11/Xatom.h>
#include <X11/Xmu/Atoms.h>
#include <X11/Xmu/StdSel.h>
#include <X11/Xmu/CharSet.h>
#include <X11/IntrinsicP.h>
#include <X11/CoreP.h>

#include <math.h>
#include <values.h>	// For MAXINT

extern int	debuglev;

/*----------------------------------------------------------------------
 * Callback to handle loss of primary selection ownership
 */

void
MimeRichTextP::LoseSelection(Widget w, Atom*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);

//
// Clear current selection since we no longer own it.
//
   if ( This->priv->selectOn ) This->priv->DrawSelection();
   This->priv->selectOn = False;
}

/*----------------------------------------------------------------------
 * Callback to handle request for primary selection
 */

Boolean
MimeRichTextP::SendSelection(Widget w, Atom *selection, Atom *target,
			     Atom *type, XtPointer *val, unsigned long *len,
			     int *format)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);

//
// Handle required atoms
//
   if ( *target == XA_TARGETS(halApp->display) ) {

      *len = 4;

      *val = XtMalloc((Cardinal)(sizeof(Atom) * (*len)));
      Atom	*targetP = *(Atom**)val;
      *targetP++ = XA_STRING;
      *targetP++ = XA_TEXT(halApp->display);
      *targetP++ = XA_COMPOUND_TEXT(halApp->display);
      *targetP++ = mimeRichAtom;

      *type = XA_ATOM;
      *format = sizeof(Atom) * 8;

      return True;

   } // End if someone wants to know what types we can send

   else if (    *target != XA_STRING
	     && *target != XA_TEXT(halApp->display)
	     && *target != XA_COMPOUND_TEXT(halApp->display)
	     && *target != mimeRichAtom ) {
      return XmuConvertStandardSelection(w, CurrentTime, selection, target,
				         type, (caddr_t*)val, len, format);
   }

//
// Now handle the targets we can supply
//

   if ( *target == XA_TEXT(halApp->display) ) *type = XA_STRING;
   else					      *type = *target;

   *format = sizeof(char) * 8;

//
// If there's a current selection, try that
//
   if ( This->priv->selectOn ) {

//
// Load up the output
//
      StringC	outbuf;
      This->priv->GetSelectionData(outbuf, (*target == mimeRichAtom)
					   ? TT_ENRICHED : TT_PLAIN);

//
// Get the length of the selection
//
      *len = outbuf.size();

//
// Allocate memory for selection data.  This memory will be freed by the
//    Intrinsics since we didn't register an XtSelectionDoneProc
//
      *val = XtMalloc((Cardinal)*len);

//
// Copy current selection
//
      register char	*dst = (char*)(*val);
      register char	*src = (char*)outbuf;

      for (int i=0; i<*len; i++) *dst++ = *src++;

   } // End if there's a current selection

//
// Try the clipboard if there's no selection
//
   else {

      int	status;
      u_long	clipLen;
      Window	win = XtWindow(This->priv->textDA);
      char	*format = (char *) ((*target == mimeRichAtom) ?
                                    MIME_ENRICHED_ATOM_NAME : "STRING");
      do {
	 status = XmClipboardInquireLength(halApp->display, win, format,
	 				   &clipLen);
      } while ( status == ClipboardLocked );

      if ( clipLen == 0 || status == ClipboardNoData )
	 return False;

//
// Allocate memory for selection data.  This memory will be freed by the
//    Intrinsics since we didn't register an XtSelectionDoneProc
//
      *val = XtMalloc((Cardinal)clipLen+1);
      char	*out = (char*)*val;
      long	privId;

      do {
	 status = XmClipboardRetrieve(halApp->display, win, format, out,
				      clipLen, len, &privId);
      } while ( status == ClipboardLocked );

      out[*len] = 0;
      *len = strlen(out);

   } // End if there's no current selection

   return True;

} // End SendSelection

/*----------------------------------------------------------------------
 * Method to build a string from the current selection
 */

void
MimeRichTextP::GetSelectionData(StringC& outbuf, TextTypeT type)
{
   if ( !selectBegPos.textLine || !selectEndPos.textLine ||
        selectBegPos == selectEndPos )
      return;

   TextPosC	*begPos = &selectBegPos;
   TextPosC	*endPos = &selectEndPos;

//
// Swap points if they are reversed
//
   if ( *begPos > *endPos ) {
      begPos = &selectEndPos;
      endPos = &selectBegPos;
   }

   sendExcerpt = False;	// We don't send this with selections

//
// Add commands to bring us up to the initial state
//
   if ( type != TT_PLAIN ) {
      TextPosC		prevPos;
      if ( FindPosPrevCmd(*begPos, &prevPos) ) {
	 TextStateC	*begState = prevPos.State();
	 TextStateC	nullState;
	 GetStateCommands(nullState, *begState, outbuf, type);
      }
   }

//
// Add command in the selected range
//
   GetRangeData(begPos, endPos, outbuf, type);

//
// Add commands to reset the state
//
   if ( type != TT_PLAIN ) {
      TextStateC	*endState = endPos->State();
      TextStateC	nullState;
      GetStateCommands(*endState, nullState, outbuf, type);
   }

   sendExcerpt = True;

   if ( debuglev > 1 )
      cout <<"Selection buffer is: [" <<outbuf <<"]" <<endl;

} // End GetSelectionData

/*----------------------------------------------------------------------
 * Method to build a string from the given range
 */

void
MimeRichTextP::GetRangeData(TextPosC *pos1, TextPosC *pos2, StringC& outbuf,
			    TextTypeT type, Boolean closeState, int lineSize)
{
   TextPosC	*begPos = pos1;
   TextPosC	*endPos = pos2;

//
// Swap points if they are reversed
//
   if ( *begPos > *endPos ) {
      begPos = pos2;
      endPos = pos1;
   }

//
// See if there's just one line involved
//
   if ( begPos->textLine == endPos->textLine ) {
      GetLineText(begPos, endPos, outbuf, type, lineSize);
   }

   else {

//
// Read to the end of the start line
//
      GetLineText(begPos, NULL, outbuf, type, lineSize);

//
// Add a newline
//
      if      ( type == TT_ENRICHED ) outbuf += '\n';
      else if ( type == TT_RICH     ) outbuf += "<nl>";
      outbuf += '\n';

//
// Read all lines in-between start and end line
//
      TextLineC	*begLine = begPos->textLine;
      TextLineC	*endLine = endPos->textLine;
      TextLineC	*tline   = begLine->next;
      while ( tline && tline != endLine ) {

	 int	oldLen = outbuf.size();
	 GetLineText(tline, outbuf, type, lineSize);
	 int	added = outbuf.size() - oldLen;

//
// Add a newline
//
	 if      ( type == TT_ENRICHED && added ) outbuf += '\n';
	 else if ( type == TT_RICH              ) outbuf += "<nl>";
	 outbuf += '\n';

	 tline = tline->next;
      }

//
// Read from the beginning of the end line
//
      GetLineText(NULL, endPos, outbuf, type, lineSize);

   } // End if more than one line selected

//
// Get closing commands
//
   if ( closeState && type != TT_PLAIN ) {
      TextStateC	nullState;
      GetStateCommands(*endPos->State(), nullState, outbuf, type);
   }

} // End GetRangeData

/*----------------------------------------------------------------------
 * Method to append the first text to the second, doubling any '<'s in
 *    the process if the type is enriched.
 */

void
MimeRichTextP::CopyText(StringC& src, StringC& dst, TextTypeT type,
			TextStateC *state, int lineSize)
{
//
// lineSize of 0 means we're not wrapping
//
   if ( lineSize == 0 ) {

//
// Once folded, plain cannot be restored.
// Enriched can always be restored so we can use 72 regardless
//
      if ( type == TT_PLAIN ) lineSize = MAXINT;
      else		      lineSize = 72;
   }

//
// See how much we're trying to add
//
   int		remaining = src.size();

//
// See how much room is left on the current line
//
   int		pos     = dst.RevPosOf('\n');
   int		wrapLen = lineSize - (dst.size() - pos - 1);

//
// If this line is full, see if there's a space where a break can be inserted
//
   if ( wrapLen <= 2 ) {

      pos     = dst.RevPosOf(' ');
      wrapLen = lineSize - (dst.size() - pos - 1);
      if ( wrapLen <= 2 ) {	// There is no space
	 //dst += "\n";
	 wrapLen = lineSize;
      }
      else {

//
// Insert break
//
	 char	*cs = dst;
	 cs[pos] = '\n';

//
// Add excerpting to plain text
//
	 if ( type == TT_PLAIN ) {
	    int	i;
	    pos++;
	    if ( sendExcerpt ) {
	       for (i=0; i<state->Excerpt(); i++) {
		  dst(pos,0) = excerptStr;
		  pos       += excerptStr.size();
		  wrapLen   -= excerptStr.size();
	       }
	    }
	    for (i=0; i<state->LIndent(); i++) {
	       dst(pos,0) = "   ";
	       pos       += 3;
	       wrapLen   -= 3;
	    }
	 }
      }
   }
   
   u_int	sstart = 0;
   CharC	line;

//
// Wrap lines at lineSize columns
//
   while ( remaining > 0 ) {

      Boolean	split = False;
      if ( remaining > wrapLen ) {

//
// Look backward from position lineSize and see if we can find a space
//
	 pos = sstart + wrapLen;
	 while ( pos > sstart && !isspace(src[pos]) ) pos--;

//
// If we found a space, end the line before it.
//
	 if ( pos > sstart ) {
	    line      = src(sstart, pos-sstart);
	    sstart    = pos + 1;
	    remaining = src.size() - sstart;
	    split     = True;
	 }

//
// If we didn't find a space to the left, look forward
//
	 else {

	    pos = sstart + wrapLen + 1;
	    while ( pos < src.size() && !isspace(src[pos]) ) pos++;

//
// If we found a space, insert a newline before the space
//
	    if ( pos < src.size() ) {
	       line      = src(sstart, pos-sstart);
	       sstart    = pos + 1;
	       remaining = src.size() - sstart;
	       split     = True;
	    }
	 }

      } // End if there are more than lineSize characters remaining

//
// If we didn't or couldn't split, just use what's left
//
      if ( !split ) {
	 line = src(sstart, remaining);
	 remaining = 0;
      }

//
// Look for "<" and replace with "<<"
//
      u_int	lstart = 0;
      if ( type == TT_ENRICHED ) {

	 pos = line.PosOf('<');
	 while ( pos >= 0 ) {

//
// Copy from start pos to current pos and add two less-than's
//
	    dst += line(lstart, pos-lstart);
	    dst += "<<";

	    lstart = pos + 1;
	    pos    = line.PosOf('<', lstart);

	 } // End for each '<'

      } // End if enriched

//
// Copy to end.
//
      int	len = line.Length() - lstart;
      if ( len > 0 )
	 dst += line(lstart, len);

//
// Add newline if text remains
//
      if ( remaining > 0 ) {

	 dst += "\n";
	 wrapLen = lineSize;	// Use this for all but the first line

	 if ( type == TT_PLAIN ) {

	    int	i;
	    if ( sendExcerpt ) {
	       for (i=0; i<state->Excerpt(); i++) {
		  dst     += excerptStr;
		  wrapLen -= excerptStr.size();
	       }
	    }
	    for (i=0; i<state->LIndent(); i++) {
	       dst     += "   ";
	       wrapLen -= 3;
	    }
	 }

      } // End more text to be added

   } // End while more source remains

} // End CopyText

/*----------------------------------------------------------------------
 * Method to extract the text between the given positions.  The positions
 *    are assumed to be on the same line.  If the start position is NULL,
 *    the beginning of the line is used.  If the end position is NULL,
 *    the end of the line is used.  The character at the end pos is not
 *    returned.  The type determines whether markup is included
 */

void
MimeRichTextP::GetLineText(TextPosC *begPos, TextPosC *endPos, StringC& outbuf,
			   TextTypeT type, int lineSize)
{
   if ( !begPos && !endPos ) return;

   TextLineC	*tl = begPos ? begPos->textLine : endPos->textLine;

   int	begCmdPos = 0;
   int	begStrPos = 0;
   if ( begPos ) {
      begCmdPos = begPos->cmdPos;
      begStrPos = begPos->strPos;
   }

   int	endCmdPos;
   int	endStrPos;
   if ( endPos ) {
      endCmdPos = endPos->cmdPos;
      endStrPos = endPos->strPos;
   }
   else {
      endCmdPos = tl->cmdList.size() - 1;
      endStrPos = tl->Cmd(endCmdPos)->LastPos();
   }

   RichCmdC	*cmd      = tl->Cmd(begCmdPos);
   TextStateC	*curState = &cmd->state;
   int		i;

//
// Get opening commands
//
   if ( type != TT_PLAIN ) {

      if ( begPos )
	 GetStateCommands(*begPos, outbuf, type);
      else {
	 TextPosC	pos(tl, begCmdPos, begStrPos);
	 GetStateCommands(pos, outbuf, type);
      }
   }

   else {
      if ( sendExcerpt )
	 for (i=0; i<curState->Excerpt(); i++) outbuf += excerptStr;
      for (i=0; i<curState->LIndent(); i++) outbuf += "   ";
   }

//
// Get text
//
   StringC	tmpbuf;
   if ( begCmdPos == endCmdPos ) {

      if ( endStrPos > begStrPos ) {

	 if ( cmd->IsText() )
	    tmpbuf = (*cmd->text)(begStrPos, endStrPos-begStrPos);
	 else {
	    tmpbuf.Clear();
	    cmd->graphic->GetText(tmpbuf);
	 }

	 CopyText(tmpbuf, outbuf, type, curState, lineSize);
      }
   }

   else {

//
// Grab the part from the start command
//
      int	len = cmd->LastPos() - begStrPos;
      if ( len > 0 ) {

	 if ( cmd->IsText() ) tmpbuf = (*cmd->text)(begStrPos, len);
	 else {
	    tmpbuf.Clear();
	    cmd->graphic->GetText(tmpbuf);
	 }

	 CopyText(tmpbuf, outbuf, type, curState, lineSize);
      }

//
// Grab the text in between
//
      curState = &cmd->state;
      for (i=begCmdPos+1; i<endCmdPos; i++) {

	 cmd = tl->Cmd(i);

	 if ( type != TT_PLAIN )
	    GetStateCommands(*curState, cmd->state, outbuf, type);
	 curState = &cmd->state;

	 if ( cmd->IsText() ) tmpbuf = *cmd->text;
	 else {
	    tmpbuf.Clear();
	    cmd->graphic->GetText(tmpbuf);
	 }

	 CopyText(tmpbuf, outbuf, type, curState, lineSize);
      }

//
// Grab the part from the end command
//
      cmd = tl->Cmd(endCmdPos);
      if ( type != TT_PLAIN )
	 GetStateCommands(*curState, cmd->state, outbuf, type);
      curState = &cmd->state;

      if ( endStrPos > 0 ) {

	 if ( cmd->IsText() ) tmpbuf = (*cmd->text)(0, endStrPos);
	 else {
	    tmpbuf.Clear();
	    cmd->graphic->GetText(tmpbuf);
	 }

	 CopyText(tmpbuf, outbuf, type, curState, lineSize);
      }

   } // End if there is more than one data segment

} // End GetLineText

/*----------------------------------------------------------------------
 * Method to extract the text for the given line.
 */

void
MimeRichTextP::GetLineText(TextLineC *line, StringC& outbuf, TextTypeT type,
			   int lineSize)
{
//
// Add text segments
//
   RichCmdC	*cmd;
   TextStateC	*curState;
   int		i;

   if ( type != TT_PLAIN ) {
      TextPosC	thisPos(line, 0, 0);
      GetStateCommands(thisPos, outbuf, type);
   }
   else {
      cmd      = line->Cmd(0);
      curState = &cmd->state;
      if ( sendExcerpt )
	 for (i=0; i<curState->Excerpt(); i++) outbuf += excerptStr;
      for (i=0; i<curState->LIndent(); i++) outbuf += "   ";
   }


   StringC	tmpbuf;
   TextStateC	*prevState = NULL;
   unsigned	count = line->cmdList.size();
   for (i=0; i<count; i++) {

      cmd = line->Cmd(i);

//
// See if we need any commands
//
      curState = &cmd->state;
      if ( prevState && type != TT_PLAIN )
	 GetStateCommands(*prevState, *curState, outbuf, type);

      if ( cmd->IsText() ) tmpbuf = *cmd->text;
      else {
	 tmpbuf.Clear();
	 cmd->graphic->GetText(tmpbuf);
      }

      CopyText(tmpbuf, outbuf, type, curState, lineSize);

      prevState = curState;

   } // End for each command

} // End GetLineText

/*----------------------------------------------------------------------
 * Method to produce the commands required to change to the state at the
 *    specified position.
 */

void
MimeRichTextP::GetStateCommands(TextPosC& pos, StringC& outbuf, TextTypeT type)
{
//
// See if the state has changed since the last state
//
   TextPosC	prevPos;
   TextStateC	*curState = pos.State();
   if ( FindPosPrevCmd(pos, &prevPos) ) {
      TextStateC	*prevState = prevPos.State();
      GetStateCommands(*prevState, *curState, outbuf, type);
   }

   else {
      TextStateC	nullState;
      GetStateCommands(nullState, *curState, outbuf, type);
   }

} // End GetStateCommands

#if 0
/*----------------------------------------------------------------------
 * Function to compare the colors in two states
 */

static Boolean
ColorsEqual(TextStateC& state1, TextStateC& state2)
{
   PixelListC&	list1 = state1.ColorStack();
   PixelListC&	list2 = state2.ColorStack();
   u_int	len1 = list1.size();
   u_int	len2 = list2.size();
   if ( len1 != len2 ) return False;

   for (int i=0; i<len1; i++)
      if ( *list1[i] != *list2[i] ) return False;

   return True;
}
#endif

/*----------------------------------------------------------------------
 * Method to produce the commands required to change between the 2 states
 */

void
MimeRichTextP::GetStateCommands(TextStateC& state1, TextStateC& state2,
				StringC& outbuf, TextTypeT type)
{
   if ( state1 == state2 ) return;

//
// We will turn off any fonts in 1 but not in 2
// We will turn on  any fonts in 2 but not in 1
//
   StringC	fonts1, fonts2;
   StringC	jus1, jus2;
//   PixelListC	col1, col2;

   StringC	*link1 = state1.PLink();
   StringC	*link2 = state2.PLink();
   if ( link1 && link2 && *link1 == *link2 ) {
      link1 = link2 = NULL;
   }

   if ( state1.FontStack() != state2.FontStack() ) {

      fonts1 = state1.FontStack();
      fonts2 = state2.FontStack();
      int	len1 = fonts1.size();
      int	len2 = fonts2.size();
      int	len  = MIN(len1, len2);
      int	fpos = 0;

//
// Remove common elements
//
      while (fpos<len && fonts1[fpos] == fonts2[fpos]) fpos++;
      if ( fpos > 0 ) {
	 fonts1(0,fpos) = "";
	 fonts2(0,fpos) = "";
      }
   }

//
// We will turn off any justification in 1 but not in 2
// We will turn on  any justification in 2 but not in 1
//
   if ( state1.JustStack() != state2.JustStack() ) {

      jus1 = state1.JustStack();
      jus2 = state2.JustStack();
      int	len1 = jus1.size();
      int	len2 = jus2.size();
      int	len  = MIN(len1, len2);
      int	jpos = 0;

//
// Skip over common elements
//
      while (jpos<len && jus1[jpos] == jus2[jpos]) jpos++;
      if ( jpos > 0 ) {
	 jus1(0,jpos) = "";
	 jus2(0,jpos) = "";
      }
   }

//
// Turn off any fonts, starting at the end and working forward
//
   if ( fonts1.size() > 0 ) {

      int	size = fonts1.size();
      for (int i=size-1; i>=0; i--) {
	 switch (fonts1[i]) {
	    case (FC_BOLD):		outbuf += "</Bold>";	break;
	    case (FC_ITALIC):	outbuf += "</Italic>";	break;
	    case (FC_FIXED):		outbuf += "</Fixed>";	break;
	    case (FC_SMALLER):	outbuf += "</Smaller>";	break;
	    case (FC_BIGGER):	outbuf += "</Bigger>";	break;
	 }
      }
   }

//
// Turn off current color if it changes any colors
//
   Pixel	*col1 = NULL;
   Pixel	*col2 = NULL;
   Pixel	p1, p2;
   if ( state1.ColorCmdCount() > 0 ) {
      p1 = state1.CurColor();
      col1 = &p1;
   }
   if ( state2.ColorCmdCount() > 0 ) {
      p2 = state2.CurColor();
      col2 = &p2;
   }
   Boolean	colorChange = ((col1 && col2 && (*col1 != *col2)) ||
			       (col1 && !col2) || (col2 && !col1));
   if ( colorChange && col1 )
      outbuf += "</X-Color>";

//
// Turn off any links
//
   if ( link1 )
      outbuf += "</X-Link>";

//
// Turn off any underlining
//
   if ( state2.Underline() < state1.Underline() ) {
      int	diff = state1.Underline() - state2.Underline();
      for (int i=0; i<diff; i++) outbuf += "</Underline>";
   }

//
// Turn off any justification
//
   if ( jus1.size() > 0 ) {
      int	size = jus1.size();
      for (int i=size-1; i>=0; i--) {
	 switch (jus1[i]) {
	    case (JC_LEFT):		outbuf += "</FlushLeft>";	break;
	    case (JC_RIGHT):		outbuf += "</FlushRight>";	break;
	    case (JC_BOTH):		outbuf += "</FlushBoth>";	break;
	    case (JC_CENTER):	outbuf += "</Center>";		break;
	    case (JC_NOFILL):	outbuf += "</NoFill>";		break;
	 }
      }
   }

//
// Turn off any indenting
//
   if ( state2.RIndent() < state1.RIndent() ) {
      int	diff = state1.RIndent() - state2.RIndent();
      for (int i=0; i<diff; i++) outbuf += "</IndentRight>";
   }

   if ( state2.LIndent() < state1.LIndent() ) {
      int	diff = state1.LIndent() - state2.LIndent();
      for (int i=0; i<diff; i++) outbuf += "</Indent>";
   }

   if ( sendExcerpt && (state2.Excerpt() < state1.Excerpt()) ) {
      int	diff = state1.Excerpt() - state2.Excerpt();
      for (int i=0; i<diff; i++) outbuf += "</Excerpt>";
   }

//
// Look for things turning on
//
   if ( sendExcerpt && (state2.Excerpt() > state1.Excerpt()) ) {
      int	diff = state2.Excerpt() - state1.Excerpt();
      for (int i=0; i<diff; i++) outbuf += "<Excerpt>";
   }

   if ( state2.LIndent() > state1.LIndent() ) {
      int	diff = state2.LIndent() - state1.LIndent();
      for (int i=0; i<diff; i++) outbuf += "<Indent>";
   }

   if ( state2.RIndent() > state1.RIndent() ) {
      int	diff = state2.RIndent() - state1.RIndent();
      for (int i=0; i<diff; i++) outbuf += "<IndentRight>";
   }

//
// Turn on any justification
//
   if ( jus2.size() > 0 ) {
      int	size = jus2.size();
      for (int i=0; i<size; i++) {
	 switch (jus2[i]) {
	    case (JC_LEFT):	outbuf += "<FlushLeft>";	break;
	    case (JC_RIGHT):	outbuf += "<FlushRight>";	break;
	    case (JC_BOTH):	outbuf += "<FlushBoth>";	break;
	    case (JC_CENTER):	outbuf += "<Center>";		break;
	    case (JC_NOFILL):	outbuf += "<NoFill>";		break;
	 }
      }
   }

//
// Turn on any underlining
//
   if ( state2.Underline() > state1.Underline() ) {
      int	diff = state2.Underline() - state1.Underline();
      for (int i=0; i<diff; i++) outbuf += "<Underline>";
   }

//
// Turn on any links
//
   if ( link2 ) {
      outbuf += "<X-Link><Param>";
      outbuf += *link2;
      outbuf += "</Param>";
   }

//
// Turn on any colors
//
   if ( colorChange && col2 ) {
      outbuf += "<X-Color><Param>";
      outbuf += ColorName(textDA, *col2);
      outbuf += "</Param>";
   }

//
// Turn on any fonts
//
   if ( fonts2.size() > 0 ) {
      int	size = fonts2.size();
      for (int i=0; i<size; i++) {
	 switch (fonts2[i]) {
	    case (FC_BOLD):		outbuf += "<Bold>";	break;
	    case (FC_ITALIC):	outbuf += "<Italic>";	break;
	    case (FC_FIXED):		outbuf += "<Fixed>";	break;
	    case (FC_SMALLER):	outbuf += "<Smaller>";	break;
	    case (FC_BIGGER):	outbuf += "<Bigger>";	break;
	 }
      }
   }

} // End GetStateCommands

/*----------------------------------------------------------------------
 * Action proc to copy of selection to clipboard.  This is a noop since
 *    the text is copied when the selection takes place.
 */

void
MimeRichTextP::ActCopySelection(Widget, XKeyEvent*, String*, Cardinal*)
{
} // End ActCopySelection

/*----------------------------------------------------------------------
 * Action proc to handle release of paste selection button
 */

void
MimeRichTextP::ActPaste(Widget w, XEvent *ev, String*, Cardinal*)
{
   if ( debuglev > 1 ) cout <<"Paste" <<endl;

   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);

   if ( !This->priv->editable ) {
      XBell(halApp->display, 0);
      return;
   }

//
// See what type of data is available.  ReceiveTargets will complete
//    the paste.
//
   This->priv->mousePaste =
      (ev->type == ButtonPress || ev->type == ButtonRelease);

   XButtonEvent	*bev = new XButtonEvent;
   *bev = *(XButtonEvent*)ev;

   // ev goes away; we need a copy
   XtGetSelectionValue(w, XA_PRIMARY, XA_TARGETS(halApp->display),
		       (XtSelectionCallbackProc)ReceiveTargets, bev, bev->time);

} // End ActPaste

/*----------------------------------------------------------------------
 * Callback to handle return of target types
 */

void
MimeRichTextP::ReceiveTargets(Widget w, XtPointer clientData, Atom*, Atom *type,
			      XtPointer val, unsigned long *len, int*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);

   XButtonEvent	*ev = (XButtonEvent*)clientData;

//
// Check for valid data
//
   if ( val == NULL || *len == 0 || *type == XT_CONVERT_FAIL ||
        *type != XA_ATOM ) {
      delete ev;
      return;
   }

//
// We have been sent a list of atoms that can be sent by the selection owner.
//    See if enriched is supported.
//
   Atom*	targetP = (Atom*)val;
   unsigned	atomCount = (unsigned)*len;
   Boolean	enrichedOk = False;
   if ( This->priv->textType == TT_ENRICHED ) {
      while ( atomCount > 0 ) {
	 if ( *targetP == mimeRichAtom ) enrichedOk = True;
	 atomCount--;
	 targetP++;
      }
   }

//
// Use the clipboard if we own the selection but it is not active.
//
   Window	swin = XGetSelectionOwner(halApp->display, XA_PRIMARY);
   if ( swin == XtWindow(This->priv->textDA) && !This->priv->selectOn ) {

      int	status;
      u_long	clipLen;
      char	*format = (char *) ((enrichedOk) ?
                                    MIME_ENRICHED_ATOM_NAME : "STRING");
      do {
	 status = XmClipboardInquireLength(halApp->display, swin, format,
	 				   &clipLen);
      } while ( status == ClipboardLocked );

      if ( clipLen > 0 && status != ClipboardNoData ) {

//
// Allocate memory for selection data.  This memory will be freed by the
//    Intrinsics since we didn't register an XtSelectionDoneProc
//
	 char	*out = XtMalloc((Cardinal)clipLen+1);
	 long	privId;
	 u_long	len;

	 do {
	    status = XmClipboardRetrieve(halApp->display, swin, format, out,
					 clipLen, &len, &privId);
	 } while ( status == ClipboardLocked );

	 out[len] = 0;
	 len = strlen(out);

	 CharC	buffer(out, (u_int)len);
	 This->priv->PasteText(w, ev, enrichedOk ? TT_ENRICHED : TT_PLAIN,
			       buffer);
	 delete ev;
	 return;

      } // End if clipboard has data

   } // End if getting data from clipboard

//
// If we get here, request the selection
//
   XtGetSelectionValue(w, XA_PRIMARY, enrichedOk ? mimeRichAtom : XA_STRING,
		       (XtSelectionCallbackProc)ReceiveSelection, ev, ev->time);

} // End ReceiveTargets

/*----------------------------------------------------------------------
 * Callback to handle paste of primary selection
 */

void
MimeRichTextP::ReceiveSelection(Widget w, XtPointer clientData, Atom*,
				Atom *type, XtPointer val, unsigned long *len,
				int*)
{
   //cout <<"ReceiveSelection " << *selection << " of type " << *type NL;

   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);

   XButtonEvent	*ev = (XButtonEvent*)clientData;

//
// Check for valid data
//
   if ( val == NULL || *len == 0 || *type == XT_CONVERT_FAIL ||
	(*type != XA_STRING && *type != mimeRichAtom) ) {
      delete ev;
      return;
   }

   CharC	buffer((char*)val, (unsigned)*len);
   This->priv->PasteText(w, ev,
   			 (*type == mimeRichAtom) ? TT_ENRICHED : TT_PLAIN,
   			 buffer);
   delete ev;

} // End ReceiveSelection

/*----------------------------------------------------------------------
 * Method to paste text at specified position
 */

void
MimeRichTextP::PasteText(Widget w, XButtonEvent *ev, TextTypeT type, CharC data)
{
   halApp->BusyCursor(True);

   if ( selectOn ) DrawSelection();	// turn it off

   if ( mousePaste )
      pub->MoveCursor(w, ev->x, ev->y);

//
// If there is a selection and the click is inside it, reset the selection
//    range
//
   if ( selectOn ) {

      TextPosC	*begPos = &selectBegPos;
      TextPosC	*endPos = &selectEndPos;
      if ( *begPos > *endPos ) {
	 begPos = &selectEndPos;
	 endPos = &selectBegPos;
      }

      if ( cursorPos >= *begPos && cursorPos <= *endPos ) {
	 selectBegPos = cursorPos;
	 selectEndPos = cursorPos;
	 selectOn = False;
      }
   }

//
// Add the data
//
   TextTypeT	saveType = textType;
   textType = type;

   pub->InsertString(data);

//
// Move cursor to end of string
//
   if ( mousePaste ) {

      HideCursor();

      u_int	count = inputLine->cmdList.size();
      for (int i=0; i<count; i++) {
	 RichCmdC	*cmd = inputLine->Cmd(i);
	 if ( cmd == inputCmd ) {
	    cursorPos.Set(inputLine, i, cmd->LastPos());
	    i = count;
	 }
      }

//
// Keep the cursor visible
//
      ScrollToCursor();
      ScreenPosC        spos = cursorPos;
      desiredCursorX = spos.x;

      ShowCursor();

   } // End if this was a mouse paste

   textType = saveType;

//
// Redraw the selection
//
   if ( selectOn ) DrawSelection();	// turn it on

   halApp->BusyCursor(False);

} // End PasteText

/*-----------------------------------------------------------------------
 *  Reset the click count after this timeout
 */

void
MimeRichTextP::ClickReset(MimeRichTextP *This, XtIntervalId*)
{
   //if ( debuglev > 1 ) cout <<"Click timed out" <<endl;

   This->clickCount = 0;
   This->clickTimer = (XtIntervalId)NULL;

//
// A multi-click effectively ends the selection
//
   if ( This->okToEndSelection )
      ActSelectEnd(This->clickWidget, &This->clickEvent, NULL, NULL);
}

/*---------------------------------------------------------------
 *  Draws the selection region using XOR mode.  May turn it on, may turn it off.
 *  Depends on whether it was on or off previously.
 */

void
MimeRichTextP::DrawSelection()
{
   DrawSelectionRange(&selectBegPos, &selectEndPos);
}

void
MimeRichTextP::DrawSelectionRange(TextPosC *begPos, TextPosC *endPos)
{
   if ( !begPos->textLine || !endPos->textLine || *begPos == *endPos )
      return;

//
// Swap points if they are reversed
//
   if ( *begPos > *endPos ) {
      TextPosC	*tmpPos = begPos;
      begPos = endPos;
      endPos = tmpPos;
   }

   ScreenPosC	begScr = *begPos;
   ScreenPosC	endScr = *endPos;

   int		begX     = begScr.x + 1;
   int		endX     = endScr.x;
   SoftLineC	*begLine = begScr.softLine;
   SoftLineC	*endLine = endScr.softLine;

   if ( !begLine || !endLine ) return;

   HideCursor();

   Drawable	drawto = textPm ? textPm : textWin;
   RectC	visArea(hsbVal, vsbVal, drawWd, drawHt);

   if ( begLine == endLine ) {

      int	wd = endX - begX + 1;
      if ( wd > 0 ) {

	 int	y  = begLine->bounds.ymin - lineSpacing;
	 int	ht = begLine->bounds.ht + lineSpacing;
	 if (y < 0) {
	    ht += y;
	    y = 0;
	 }
	 RectC	lineArea(begX, y, wd, ht);

	 if ( visArea.OverlapsY(lineArea) ) {

	    XSetFunction(halApp->display, textGC, GXxor);
	    XSetForeground(halApp->display, textGC, cursorColor);

	    if ( debuglev > 1 )
	       cout <<"Drawing selection: " <<begX <<" " <<y <<" "
					    <<wd <<" " <<ht <<endl;
	    XFillRectangle(halApp->display, drawto, textGC,
	    		   begX-hsbVal, y-vsbVal, wd, ht);

	    XSetFunction(halApp->display, textGC, GXcopy);
	    XSetForeground(halApp->display, textGC, fgColor);
	 }
      }

   } // End if just one line

   else {

      XSetFunction(halApp->display, textGC, GXxor);
      XSetForeground(halApp->display, textGC, cursorColor);

//      int	rightX = drawWd - 1;

//
// Select to the end of the start line
//
//      int	wd = rightX - begX + 1;
      int	wd = drawWd - (begX - hsbVal);
      int	y;
      int	ht;
      if ( wd > 0 && visArea.OverlapsY(begLine->bounds) ) {
	 y  = begLine->bounds.ymin - lineSpacing;
	 ht = begLine->bounds.ht   + lineSpacing;
	 if (y < 0) {
	    ht += y;
	    y = 0;
	 }

	 if ( debuglev > 1 )
	    cout <<"Drawing selection: " <<begX <<" " <<y <<" "
					 <<wd <<" " <<ht <<endl;
	 XFillRectangle(halApp->display, drawto, textGC, begX-hsbVal, y-vsbVal,
	 		wd, ht);
      }

//
// Select all lines between start and end line
//
      SoftLineC	*sline = begScr.softLine->next;
      while ( sline && sline != endScr.softLine ) {

	 int	x  = sline->bounds.xmin;
//	 wd = rightX - x + 1;
	 wd = drawWd - (x - hsbVal);

	 if ( wd > 0 && visArea.OverlapsY(sline->bounds) ) {

	    y  = sline->bounds.ymin - lineSpacing;
	    ht = sline->bounds.ht   + lineSpacing;
	    if (y < 0) {
	       ht += y;
	       y = 0;
	    }

	    if ( debuglev > 1 ) cout <<"Drawing selection: " <<x <<" " <<y <<" "
						       <<wd <<" " <<ht <<endl;
	    XFillRectangle(halApp->display, drawto, textGC, x-hsbVal, y-vsbVal,
	    		   wd, ht);
	 }

	 sline = sline->next;

      } // End for each intermediate line

//
// Select from the beginning of the end line
//
      wd = endX - endLine->bounds.xmin + 1;
      if ( wd > 0 && visArea.OverlapsY(endLine->bounds) ) {

	 y  = endLine->bounds.ymin - lineSpacing;
	 ht = endLine->bounds.ht + lineSpacing;
	 if (y < 0) {
	    ht += y;
	    y = 0;
	 }

	 if ( debuglev > 1 )
	    cout <<"Drawing selection: " <<endLine->bounds.xmin <<" " <<y <<" "
					 <<wd <<" " <<ht <<endl;
	 XFillRectangle(halApp->display, drawto, textGC,
	 		endLine->bounds.xmin-hsbVal, y-vsbVal, wd, ht);
      }

      XSetFunction(halApp->display, textGC, GXcopy);
      XSetForeground(halApp->display, textGC, fgColor);

   } // End if more than one line selected

   if ( drawto != textWin )
      XCopyArea(halApp->display, drawto, textWin, textGC, 0, 0, drawWd, drawHt,
      		0, 0);

   ShowCursor();

} // End DrawSelectionRange

/*---------------------------------------------------------------
 *  Method to find the line containing the specified y position
 */

SoftLineC
*MimeRichTextP::PickLine(int y)
{
//
// Loop through the line list and look for one containing the position.
//
   SoftLineC	*pickLine = NULL;
   TextLineC	*tline = topTextLine;
   while ( tline ) {

//
// Loop until we hit the correct text line
//
      if ( tline->bounds.ymax >= y ) {

	 SoftLineC	*sline = tline->softLine;
	 while ( sline && sline->textLine == tline && !pickLine ) {

//
// Loop until we hit the correct soft line
//
	    if ( sline->bounds.ymax >= y ) {

//
// Back up if we've gone past.
//
	       if ( sline->bounds.ymin > y && sline->prev ) sline = sline->prev;

//
// This should be the one
//
	       pickLine = sline;
	    }

	    sline = sline->next;

	 } // End for each soft line

	 if ( !pickLine && tline->softLine ) {
	    sline = tline->softLine;
	    if ( y <= sline->bounds.ymin ) pickLine = sline;
	    else			   pickLine = NULL;
	 }

      } // End if we hit the correct text line

      tline = tline->next;

   } // End for each text line

   if ( !pickLine && topSoftLine ) {
      SoftLineC	*sline = topSoftLine;
      if ( y <= sline->bounds.ymin ) pickLine = sline;
      else			     pickLine = NULL;
   }

   return pickLine;

} // End PickLine

/*---------------------------------------------------------------
 *  Method to move the cursor as close as possible to the specified x,y
 */

void
MimeRichTextC::MoveCursor(Widget w, int x, int y)
{
   priv->HideCursor();

//
// Take scrolling into account
//
   x += priv->hsbVal;
   y += priv->vsbVal;

//
// Find the line under the cursor
//
   SoftLineC	*pline = priv->PickLine(y);
   if ( !pline ) {
      pline = priv->botSoftLine;
      if ( pline ) x = pline->bounds.xmax;
   }
   ScreenPosC	spos(pline, x);
   priv->cursorPos = spos;

//
// Update the cursor
//
   priv->ScrollToCursor();
   priv->desiredCursorX = spos.x;

   priv->ShowCursor();

} // End MoveCursor(int,int)

/*-----------------------------------------------------------------------
 *  distance between 2 screen positions
 */

int
MimeRichTextP::Distance(ScreenPosC& sp1, ScreenPosC& sp2)
{
   if ( !sp1.softLine || !sp2.softLine ) return 0;

   if ( sp1.softLine == sp2.softLine ) return abs(sp1.x - sp2.x);

   ScreenPosC	*pos1, *pos2;
   if ( sp1.softLine->bounds.ymin < sp2.softLine->bounds.ymin ) {
      pos1 = &sp1;
      pos2 = &sp2;
   }
   else {
      pos1 = &sp2;
      pos2 = &sp1;
   }

//
// Find the distance to the end of line1
//
   int	sum = drawWd - pos1->x;

//
// Add the distance from the beginning of line2
//
   sum += pos2->x;

//
// Count the lines in between
//
   SoftLineC	*sline = pos1->softLine->next;
   while ( sline && sline != pos2->softLine ) {
      sum += drawWd;
      sline = sline->next;
   }

   return sum;

} // End Distance

/*---------------------------------------------------------------
 *  Method to return the position in the data block nearest to the specified x
 */

int
RichDrawDataC::Snap(int snapX)
{
   if ( snapX < x )
      snapX = x;

   else if ( snapX >= x + width )
      snapX = x + width - 1;

   else {	// Somewhere in the middle

//
// Add characters until we reach the x position
//
      int		dir, asc, dsc;
      XCharStruct	size;
      int		len = 1;
      char		*cs = string;
      int		curX = x;
      int		origX = snapX;
      int		dist = abs(origX - curX);
      int		minDist = dist;

      snapX = curX;
      while ( dist <= minDist && len < string.size() ) {

	 XTextExtents(font->xfont, cs, len, &dir, &asc, &dsc, &size);

	 curX = x + size.width - 1;
	 dist = abs(origX - curX);
	 if ( dist < minDist ) {
	    minDist = dist;
	    snapX   = curX;
	 }

	 len++;
      }

   } // End if position in the middle

   return snapX;

} // End RichDrawDataC::Snap

/*---------------------------------------------------------------
 *  Method to return the position in the data block nearest to the specified x
 */

int
RichDrawDataC::CharPos(int charX)
{
   int	charPos;

   if ( charX <= x )
      charPos = 0;

   else if ( charX >= x + width )
      charPos = string.size();

   else {	// Somewhere in the middle

      RichCmdC	*cmd = softLine->textLine->Cmd(cmdPos);
      if ( cmd->IsText() ) {

//
// Add characters until we reach the x position.  The position should already
//    be snapped.
//
	 int		dir, asc, dsc;
	 XCharStruct	size;
	 int		len = 1;
	 char		*cs = string;
	 int		curX = x;

	 XTextExtents(font->xfont, cs, len, &dir, &asc, &dsc, &size);
	 curX = x + size.width - 1;
	 while ( curX < charX && len < string.size() ) {
	    len++;
	    XTextExtents(font->xfont, cs, len, &dir, &asc, &dsc, &size);
	    curX = x + size.width - 1;
	 }
	 charPos = len;

      } // End if we're in a text command

      else {
	 int	minDist = charX - x;
	 int	maxDist = x + width - charX;
	 if ( minDist < maxDist ) charPos = 0;
	 else			  charPos = 1;
      }

   } // End if position in the middle

   return charPos;

} // End CharPos

/*-----------------------------------------------------------------------
 *  Find the leftmost character that belongs to the current character class.
 */

Boolean
MimeRichTextP::FindPosBegClass(TextPosC& curPos, TextPosC *newPos)
{
   TextLineC	*tl  = curPos.textLine;
   RichCmdC	*cmd = tl->Cmd(curPos.cmdPos);

   *newPos = curPos;

//
// If we're on a graphic command, just use that one
//
   if ( cmd->IsGraphic() ) {
      newPos->Set(tl, curPos.cmdPos, 0);
      return True;
   }

//
// We must be on a text command
//
   StringC	*str    = tl->Text(curPos.cmdPos);
   unsigned	strSize = str->size();
   if ( strSize == 0 ) return True;

   int		charPos = curPos.strPos;
   if ( charPos == strSize ) charPos--;

//
// Find the character class of the current character
//
   char	curChar = (*str)[charPos];
   int	curClass = charClasses[curChar];

//
// Look to the left
//
   int	strPos = charPos - 1;
   while ( strPos >= 0 && charClasses[(*str)[strPos]] == curClass )
      strPos--;

//
// If we're still good, look in other text strings
//
   int	cmdPos = curPos.cmdPos;
   Boolean	done = False;
   while ( !done && strPos < 0 && cmdPos > 0 ) {

      cmdPos--;
      RichCmdC	*cmd = tl->Cmd(cmdPos);
      if ( cmd->IsText() ) {

	 strPos = cmd->text->size() - 1;
	 while ( strPos >= 0 && charClasses[(*cmd->text)[strPos]] == curClass )
	    strPos--;
      }

      else {
	 cmdPos++;
	 strPos = -1;
	 done = True;
      }
   }

   if ( strPos < 0 ) strPos = 0;
   else		     strPos++;	// Since last one didn't pass
   newPos->Set(tl, cmdPos, strPos);

   return True;

} // End FindPosBegClass

/*-----------------------------------------------------------------------
 *  Find the rightmost character that belongs to the current character class.
 */

Boolean
MimeRichTextP::FindPosEndClass(TextPosC& curPos, TextPosC *newPos)
{
   TextLineC	*tl  = curPos.textLine;
   RichCmdC	*cmd = tl->Cmd(curPos.cmdPos);

   *newPos = curPos;

//
// If we're on a graphic command, just use that one
//
   if ( cmd->IsGraphic() ) {
      newPos->Set(tl, curPos.cmdPos, 1);
      return True;
   }

//
// We must be on a text command
//
   StringC	*str    = tl->Text(curPos.cmdPos);
   unsigned	strSize = str->size();
   if ( strSize == 0 ) return True;

   int		charPos = curPos.strPos;
   if ( charPos == strSize ) charPos--;

//
// Find the character class of the current character
//
   char	curChar = (*str)[charPos];
   int	curClass = charClasses[curChar];

//
// Look to the right
//
   int	strPos      = curPos.strPos;
   int	checkStrPos = charPos + 1;
   while ( checkStrPos < strSize &&
           charClasses[(*str)[checkStrPos]] == curClass ) {
      strPos = checkStrPos;
      checkStrPos++;
   }

//
// If we're still good, look in other text strings
//
   int		cmdPos       = curPos.cmdPos;
   int		checkCmdPos  = cmdPos + 1;
   unsigned	cmdCount     = tl->cmdList.size();
   unsigned	checkStrSize = strSize;
   Boolean	done         = False;
   while ( !done && checkStrPos >= checkStrSize && checkCmdPos < cmdCount ) {

      RichCmdC	*cmd = tl->Cmd(checkCmdPos);
      if ( cmd->IsText() ) {

	 checkStrSize = cmd->text->size();
	 checkStrPos = 0;

	 while ( checkStrPos < checkStrSize &&
		 charClasses[(*cmd->text)[checkStrPos]] == curClass ) {
	    cmdPos  = checkCmdPos;
	    strPos  = checkStrPos;
	    strSize = checkStrSize;
	    checkStrPos++;
	 }
      }

      else {
	 done = True;
      }

      checkCmdPos++;
   }

//
// Now, move to the right of the last matching character
//
   if ( strPos < strSize ) strPos++;
   newPos->Set(tl, cmdPos, strPos);

   return True;

} // End FindPosEndClass

/*-----------------------------------------------------------------------
 *  Find the position one character to the left of the specified position
 */

Boolean
MimeRichTextP::FindPosPrevChar(TextPosC& curPos, TextPosC *newPos)
{
   *newPos = curPos;
   if ( !newPos->textLine ) return False;

//
// See if we still have room in the current command
//
   if ( newPos->strPos > 0 ) {

      newPos->strPos--;

//
// Make sure we're not pointing to a clipped position
//
      ScreenPosC	spos = *newPos;
      while ( !spos.softLine && newPos->strPos > 0 ) {
	 newPos->strPos--;
	 spos = *newPos;
      }

      if ( spos.softLine ) 
	 return True;

   } // End if there is more room in the string

//
// See if there is another command
//
   if ( newPos->cmdPos > 0 ) {

      newPos->cmdPos--;
      RichCmdC	*cmd = newPos->Cmd();
      newPos->strPos = cmd->LastPos();

//
// If we're in the middle of a line, we may not have actually moved.  Check
//    this by seeing if we're in the same soft line.
//
      ScreenPosC	pos0 = curPos;
      ScreenPosC	pos1 = *newPos;

//
// Make sure we're not pointing to a clipped position
//
      while ( !pos1.softLine && newPos->strPos > 0 ) {
	 newPos->strPos--;
	 pos1 = *newPos;
      }

      if ( pos1.softLine ) {
	 if ( pos0.softLine == pos1.softLine ) newPos->strPos--;
	 if ( newPos->strPos < 0 ) newPos->strPos = 0;
	 return True;
      }

   } // End if there are more commands

//
// See if there is another line
//
   TextLineC	*prevLine = newPos->textLine->prev;
   if ( prevLine ) {
      newPos->textLine = prevLine;
      newPos->cmdPos = newPos->textLine->cmdList.size() - 1;
      RichCmdC	*cmd = newPos->Cmd();
      newPos->strPos = cmd->LastPos();

//
// Make sure we're not pointing to a clipped position
//
      ScreenPosC	spos = *newPos;
      while ( !spos.softLine && newPos->strPos > 0 ) {
	 newPos->strPos--;
	 spos = *newPos;
      }

      if ( spos.softLine ) 
	 return True;

   } // End if more lines above

//
// Nothing to the left
//
   return False;

} // End FindPosPrevChar

/*-----------------------------------------------------------------------
 *  Find the position one character to the right of the specified position
 */

Boolean
MimeRichTextP::FindPosNextChar(TextPosC& curPos, TextPosC *newPos)
{
   *newPos = curPos;
   if ( !newPos->textLine ) return False;

   RichCmdC	*cmd = newPos->Cmd();

//
// See if we still have room in the current command
//
   if ( newPos->strPos < cmd->LastPos() ) {

      newPos->strPos++;

//
// Make sure we're not pointing to a clipped position
//
      ScreenPosC	spos = *newPos;
      while ( !spos.softLine && newPos->strPos < cmd->LastPos() ) {
	 newPos->strPos++;
	 spos = *newPos;
      }

      if ( spos.softLine ) 
	 return True;

   } // End if we're not at the end of the string

//
// See if there is another command
//
   unsigned	cmdCount = newPos->textLine->cmdList.size();
   if ( newPos->cmdPos+1 < cmdCount ) {

      newPos->cmdPos++;
      newPos->strPos = 0;

//
// If we're in the middle of a line, we may not have actually moved.  Check
//    this by seeing if we're in the same soft line.
//
      ScreenPosC	pos0 = curPos;
      ScreenPosC	pos1 = *newPos;
      cmd = newPos->Cmd();

//
// Make sure we're not pointing to a clipped position
//
      while ( !pos1.softLine && newPos->strPos < cmd->LastPos() ) {
	 newPos->strPos++;
	 pos1 = *newPos;
      }

      if ( pos1.softLine ) {
	 if ( pos0.softLine == pos1.softLine ) newPos->strPos++;
	 if ( newPos->strPos > cmd->LastPos() ) newPos->strPos = cmd->LastPos();
	 return True;
      }

   } // End if there are more commands

//
// See if there is another line
//
   TextLineC	*nextLine = newPos->textLine->next;
   if ( nextLine ) {

      newPos->textLine = nextLine;

      newPos->cmdPos = 0;
      newPos->strPos = 0;
      cmd = newPos->Cmd();

//
// Make sure we're not pointing to a clipped position
//
      ScreenPosC	spos = *newPos;
      while ( !spos.softLine && newPos->strPos < cmd->LastPos() ) {
	 newPos->strPos++;
	 spos = *newPos;
      }

      if ( spos.softLine ) 
	 return True;

   } // End if more lines below

//
// Nothing to the right
//
   return False;

} // End FindPosNextChar

/*-----------------------------------------------------------------------
 *  Find the beginning of this word or if we're at the beginning, the
 *     beginning of the previous word.  This means find the first whitespace
 *     character to the left of the next non-white character.
 */

Boolean
MimeRichTextP::FindPosPrevWord(TextPosC& curPos, TextPosC *newPos)
{
   *newPos = curPos;
   if ( !newPos->textLine ) return False;

//
// See if we're at the beginning of the line
//
   if ( newPos->cmdPos == 0 && newPos->strPos == 0 ) {

      int	tindex = newPos->textLine->index;
      if ( tindex == 0 ) return False;

//
// Move to the previous line
//
      newPos->textLine = newPos->textLine->prev;
      unsigned	cmdCount = newPos->textLine->cmdList.size();
      newPos->cmdPos = cmdCount - 1;
      RichCmdC	*cmd = newPos->Cmd();
      newPos->strPos = cmd->LastPos();

   } // End if we're at the beginning of a line

   Boolean	needWhite    = True;
   Boolean	needNonWhite = True;

//
// Look backwards through data
//
   unsigned	cmdCount = newPos->textLine->cmdList.size();
   int		strPos   = newPos->strPos;
   int		cmdPos   = newPos->cmdPos;
   RichCmdC	*cmd = newPos->Cmd();
   Boolean	found = False;
   while ( !found && cmdPos >= 0 ) {

//
// Look backwards through the characters
//
      strPos--;
      while ( !found && strPos >= 0 ) {

	 if ( cmd->IsText() ) {
	    char	c = (*cmd->text)[strPos];
	    if ( needNonWhite ) {
	       if ( !isspace(c) ) needNonWhite = False;
	    }
	    else if ( needWhite ) {
	       if ( isspace(c) ) {
		  strPos++;
		  found = True;
	       }
	    }
	 }

	 else {
	    strPos = 0;
	    found = True;
	 }

	 if ( !found ) strPos--;

      } // End for each character

//
// Move to previous command
//
      if ( !found ) {

	 cmdPos--;

//
// See if we can possibly stop here.  We have to have found some characters.
//
	 if ( !needNonWhite ) {	// Means we found some

	    if ( cmdPos < 0 ) {
	       cmdPos++;
	       strPos = 0;
	       found = True;
	    }

	    else {

	       cmd = newPos->textLine->Cmd(cmdPos);
	       if ( !cmd->IsText() ) {
		  cmdPos++;
		  strPos = 0;
		  found = True;
	       }
	       else
		  strPos = cmd->LastPos();
	    }

	 } // End if characters have been found

	 else if ( cmdPos >= 0 ) {

	    cmd = newPos->textLine->Cmd(cmdPos);
	    strPos = cmd->LastPos();

	 } // End if no characters found yet

      } // End if not yet found

   } // End for each data block

   if ( found ) {
      newPos->cmdPos = cmdPos;
      newPos->strPos = strPos;
   }
   else {
      newPos->cmdPos = 0;
      newPos->strPos = 0;
   }

//
// Make sure we're not pointing to a clipped position
//
   ScreenPosC	spos = *newPos;
   while ( !spos.softLine && newPos->strPos > 0 ) {
      newPos->strPos--;
      spos = *newPos;
   }

   if ( spos.softLine )
      return True;
   else
      return False;

} // End FindPosPrevWord

/*-----------------------------------------------------------------------
 *  Find the beginning of this word.  This means find the first whitespace
 *     character to the left of the next non-white character.
 */

Boolean
MimeRichTextP::FindPosBegWord(TextPosC& curPos, TextPosC *newPos)
{
   *newPos = curPos;
   if ( !newPos->textLine ) return False;

//
// See if we're at the beginning of the line
//
   if ( newPos->cmdPos == 0 && newPos->strPos == 0 ) return True;

//
// Look backwards through data
//
   unsigned	cmdCount = newPos->textLine->cmdList.size();
   int		strPos   = newPos->strPos;
   int		cmdPos   = newPos->cmdPos;
   RichCmdC	*cmd = newPos->Cmd();
   Boolean	found = False;
   while ( !found && cmdPos >= 0 ) {

//
// Look backwards through the characters
//
      strPos--;
      while ( !found && strPos >= 0 ) {

	 if ( cmd->IsText() ) {
	    char	c = (*cmd->text)[strPos];
	    if ( isspace(c) ) {
	       strPos++;
	       found = True;
	    }
	 }

	 else {
	    strPos = 0;
	    found = True;
	 }

	 if ( !found ) strPos--;

      } // End for each character

//
// Move to previous command
//
      if ( !found ) {

	 cmdPos--;
	 if ( cmdPos < 0 ) {
	    cmdPos++;
	    strPos = 0;
	    found = True;
	 }

	 else {

	    cmd = newPos->textLine->Cmd(cmdPos);
	    if ( !cmd->IsText() ) {
	       cmdPos++;
	       strPos = 0;
	       found = True;
	    }
	    else
	       strPos = cmd->LastPos();
	 }

      } // End if not yet found

   } // End for each data block

   if ( found ) {
      newPos->cmdPos = cmdPos;
      newPos->strPos = strPos;
   }
   else {
      newPos->cmdPos = 0;
      newPos->strPos = 0;
   }

//
// Make sure we're not pointing to a clipped position
//
   ScreenPosC	spos = *newPos;
   while ( !spos.softLine && newPos->strPos > 0 ) {
      newPos->strPos--;
      spos = *newPos;
   }

   if ( spos.softLine )
      return True;
   else
      return False;

} // End FindPosBegWord

/*-----------------------------------------------------------------------
 *  Find the beginning of the next word.  This means find the first non-white
 *     character to the right of the next white character.
 */

Boolean
MimeRichTextP::FindPosNextWord(TextPosC& curPos, TextPosC *newPos)
{
   *newPos = curPos;
   if ( !newPos->textLine ) return False;

//
// Look forward through data
//
   Boolean	needWhite    = True;
   Boolean	needNonWhite = True;
   unsigned	cmdCount = newPos->textLine->cmdList.size();
   int		strPos = newPos->strPos;
   int		cmdPos = newPos->cmdPos;
   RichCmdC	*cmd = newPos->Cmd();
   Boolean	found = False;
   Boolean	gotText = False;

   while ( !found && cmdPos < cmdCount ) {

//
// Look forward through the characters
//
      unsigned	strSize = cmd->LastPos();
      while ( !found && strPos < strSize ) {

	 if ( cmd->IsText() ) {

	    gotText = True;
	    char	c = (*cmd->text)[strPos];
	    if ( needWhite ) {
	       if ( isspace(c) ) needWhite = False;
	    }
	    else if ( needNonWhite ) {
	       if ( !isspace(c) ) found = True;
	    }
	 }

//
// If we found any characters, we can stop here
//
	 else {
	    if ( gotText ) found = True;
	    else	   needWhite = False;
	 }

	 if ( !found ) strPos++;

      } // End for each character

//
// Move to next command
//
      if ( !found ) {
	 cmdPos++;
	 if ( cmdPos < cmdCount ) {
	    cmd = newPos->textLine->Cmd(cmdPos);
	    strPos = 0;
	 }
      }

   } // End for each data block

//
// If we're at the end of the line, move to the beginning of the next
//
   if ( !found && cmdPos == cmdCount && strPos == cmd->LastPos() ) {

      TextLineC	*nextLine = newPos->textLine->next;
      if ( nextLine ) {

//
// Move to the next line
//
	 newPos->textLine = nextLine;
	 newPos->cmdPos = 0;
	 newPos->strPos = 0;
      }

      else {
//
// Move to the end of this line
//
	 newPos->cmdPos = cmdCount - 1;
	 newPos->strPos = cmd->LastPos();
      }

   } // End if at end of line

   else {
      newPos->cmdPos = cmdPos;
      newPos->strPos = strPos;
   }

//
// Make sure we're not pointing to a clipped position
//
   ScreenPosC	spos = *newPos;
   while ( !spos.softLine && newPos->strPos < cmd->LastPos() ) {
      newPos->strPos++;
      spos = *newPos;
   }

   if ( spos.softLine )
      return True;
   else
      return False;

} // End FindPosNextWord

/*-----------------------------------------------------------------------
 *  Find the end of the current word.  This means find the next white
 *     character.
 */

Boolean
MimeRichTextP::FindPosEndWord(TextPosC& curPos, TextPosC *newPos)
{
   *newPos = curPos;
   if ( !newPos->textLine ) return False;

//
// Look forward through data
//
   unsigned	cmdCount = newPos->textLine->cmdList.size();
   int		strPos = newPos->strPos;
   int		cmdPos = newPos->cmdPos;
   RichCmdC	*cmd = newPos->Cmd();
   Boolean	found = False;

   while ( !found && cmdPos < cmdCount ) {

//
// Look forward through the characters
//
      unsigned	strSize = cmd->LastPos();
      while ( !found && strPos < strSize ) {

	 if ( cmd->IsText() ) {
	    char	c = (*cmd->text)[strPos];
	    if ( isspace(c) ) found = True;
	 }

	 else
	    found = True;

	 if ( !found ) strPos++;

      } // End for each character

//
// Move to next command
//
      if ( !found ) {
	 cmdPos++;
	 if ( cmdPos < cmdCount ) {
	    cmd = newPos->textLine->Cmd(cmdPos);
	    strPos = 0;
	 }
      }

   } // End for each data block

   if ( !found ) {

//
// Move to the end of this line
//
      newPos->cmdPos = cmdCount - 1;
      newPos->strPos = cmd->LastPos();

   } // End if at end of line

   else {
      newPos->cmdPos = cmdPos;
      newPos->strPos = strPos;
   }

//
// Make sure we're not pointing to a clipped position
//
   ScreenPosC	spos = *newPos;
   while ( !spos.softLine && newPos->strPos < cmd->LastPos() ) {
      newPos->strPos++;
      spos = *newPos;
   }

   if ( spos.softLine )
      return True;
   else
      return False;

} // End FindPosEndWord

/*---------------------------------------------------------------
 *  Method to find the draw data containing the specified x position
 */

RichDrawDataC
*SoftLineC::PickData(int x)
{
   RichDrawDataC	*pickData = NULL;
   RichDrawDataC	*dd = drawData;
   while ( !pickData && dd ) {

//
// Loop until we hit the correct data block
//
      if ( (dd->x + dd->width) > x ) {

//
// Back up if we've gone past.
//
	 if ( dd->x > x && dd->prev ) dd = dd->prev;

//
// This should be the one
//
	 pickData = dd;
      }

      dd = dd->next;

   } // End for each data block

   if ( !pickData && drawData ) {
      if ( x <= bounds.xmin ) pickData = FirstData();
      else		      pickData = LastData();
   }

   return pickData;

} // End PickData

/*---------------------------------------------------------------
 *  Change the selection range
 */

void
MimeRichTextP::UpdateSelection(TextPosC& begPos, TextPosC& endPos)
{
   if ( begPos == selectBegPos && endPos == selectEndPos ) return;

//
// If the positions are the same, turn off the selection
//
   if ( begPos == endPos ) {
      if ( selectOn ) DrawSelection(); 	// To turn it off
      selectOn = False;
      selectBegPos = begPos;
      selectEndPos = endPos;
      return;
   }

//
// If there is no current selection, just use the specified positions
//
   if ( !selectOn ) {
      selectBegPos = begPos;
      selectEndPos = endPos;
      DrawSelection();
      selectOn = True;
      return;
   }

//
// If the beginning position has changed, draw to the new position
//
   if ( begPos < selectBegPos ) {
      DrawSelectionRange(&begPos, &selectBegPos);	// Draw section
   }
   else if ( begPos > selectBegPos ) {
      DrawSelectionRange(&selectBegPos, &begPos);	// Clear section
   }

//
// If the end position has changed, draw to the new position
//
   if ( endPos < selectEndPos ) {
      DrawSelectionRange(&endPos, &selectEndPos);	// Clear section
   }
   else if ( endPos > selectEndPos ) {
      DrawSelectionRange(&selectEndPos, &endPos);	// Draw section
   }

   selectBegPos = begPos;
   selectEndPos = endPos;

   //CompactSelection();

} // End UpdateSelection

/*---------------------------------------------------------------
 *  Handle start of the a selection
 */

void
MimeRichTextP::ActSelectBegin(Widget w, XButtonEvent *ev, String*, Cardinal*)
{
   //if ( debuglev > 1 ) cout <<"Button down" <<endl;

   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);

//
// Remove any existing click timer
//
   if ( This->priv->clickTimer ) {
      XtRemoveTimeOut(This->priv->clickTimer);
      This->priv->clickTimer = (XtIntervalId)NULL;
   }
   This->priv->okToEndSelection = False;

   XmProcessTraversal(This->priv->textDA, XmTRAVERSE_CURRENT);

//
// Count multi-clicks ourself if a timer can't be used
//
   if ( !This->priv->timerOk ) {
      u_long	diff = ev->time - This->priv->selectTime;
      if ( diff > (u_long)XtGetMultiClickTime(halApp->display) )
	 This->priv->clickCount = 0;
   }

   This->priv->clickCount++;
   This->priv->selectTime = ev->time;

//
// See if this click is in a graphic
//
   RichGraphicC *graphic = NULL;
   if ( This->priv->clickCount <= 2 )
      graphic = This->priv->PickGraphic(ev->x, ev->y);

   TextPosC	newBegPos, newEndPos;

   switch ( This->priv->clickCount ) {

      case 1:

         if ( graphic ) {
	    if ( debuglev > 1 ) cout <<"Single click in graphic" <<endl;
	    graphic->SingleClick(ev);
	    This->MoveCursor(graphic, 1);
	 }
	 else {
	    if ( debuglev > 1 ) cout <<"Single click" <<endl;
	    This->MoveCursor(w, ev->x, ev->y);
	 }

	 newBegPos = This->priv->cursorPos;
	 newEndPos = This->priv->cursorPos;
	 This->priv->selectType = SELECT_CHAR;
	 break;

      case 2:	// Select word

	 if ( graphic ) {
	    if ( debuglev > 1 ) cout <<"Double click in graphic" <<endl;
	    graphic->DoubleClick(ev);
	    This->MoveCursor(graphic, 1);
	    newBegPos = This->priv->cursorPos;
	    newEndPos = This->priv->cursorPos;
	 }
	 else {
	    if ( debuglev > 1 ) cout <<"Double click" <<endl;
	    This->priv->FindPosBegClass(This->priv->cursorPos, &newBegPos);
	    This->priv->FindPosEndClass(This->priv->cursorPos, &newEndPos);
	 }

	 This->priv->selectType = SELECT_WORD;
         break;

      case 3:	// Select line
      {
         if ( debuglev > 1 ) cout <<"Got triple click" <<endl;
	 ScreenPosC	begPos(This->priv->cursorPos);
	 ScreenPosC	endPos(This->priv->cursorPos);
	 begPos.Set(begPos.softLine, begPos.softLine->bounds.xmin);
	 endPos.Set(endPos.softLine, endPos.softLine->bounds.xmax);
	 newBegPos = begPos;
	 newEndPos = endPos;
	 This->priv->selectType = SELECT_LINE;
      } break;

      case 4:	// Select page
      {
         if ( debuglev > 1 ) cout <<"Got quadruple click" <<endl;
	 if ( This->priv->botTextLine ) {
	    TextLineC	*tline = This->priv->botTextLine;
	    unsigned	cmdCount = tline->cmdList.size();
	    RichCmdC	*cmd     = tline->Cmd(cmdCount-1);
	    newBegPos.Set(This->priv->topTextLine, 0, 0);
	    newEndPos.Set(tline, cmdCount-1, cmd->LastPos());
	    This->priv->selectType = SELECT_PAGE;
	 }
	 This->priv->clickCount = 0;
      } break;
   }

//
// Keep the beginning position to the left of the end position
//
   if ( newBegPos <= newEndPos )
      This->priv->UpdateSelection(newBegPos, newEndPos);
   else
      This->priv->UpdateSelection(newEndPos, newBegPos);

   This->priv->baseSelectBegPos = This->priv->selectBegPos;
   This->priv->baseSelectEndPos = This->priv->selectEndPos;

   if ( This->priv->selectOn ) {

//
// Move the cursor to the nearest end of the selection
//
      This->priv->HideCursor();
      ScreenPosC	cpos = This->priv->cursorPos;
      ScreenPosC	bpos = This->priv->selectBegPos;
      ScreenPosC	epos = This->priv->selectEndPos;
      int begDist = This->priv->Distance(cpos, bpos);
      int endDist = This->priv->Distance(cpos, epos);
      if ( begDist < endDist )
	 This->priv->cursorPos = This->priv->selectBegPos;
      else
	 This->priv->cursorPos = This->priv->selectEndPos;
      This->priv->ShowCursor();
   }

//
// Add a timer to look for multi-clicks
//
   This->priv->clickWidget = w;
   This->priv->clickEvent  = *ev;

   if ( This->priv->timerOk )
      This->priv->clickTimer  =
	 XtAppAddTimeOut(halApp->context, XtGetMultiClickTime(halApp->display),
			 (XtTimerCallbackProc)ClickReset,
			 (XtPointer)This->priv);

} // End ActSelectBegin

/*---------------------------------------------------------------
 *  Handle extend of the selection
 */

void
MimeRichTextP::ActSelectMotion(Widget w, XMotionEvent *ev, String*,Cardinal*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);

   This->priv->selectTime = ev->time;

   if ( !This->priv->baseSelectBegPos.textLine ) return;

//
// Find the position of the pointer in both da and form coords
//
   int	dx = ev->x;
   int	dy = ev->y;

//
// See if the pointer is inside or outside the window
//
   Dimension    winWd, winHt;
   XtVaGetValues(This->priv->textDA, XmNwidth, &winWd, XmNheight, &winHt, 0);

   Boolean	scrollU = (This->priv->vsbOn &&
			   This->priv->vsbVal > 0 &&
			   dy <= (int)0);
   Boolean	scrollD = (This->priv->vsbOn &&
   			   This->priv->vsbVal < This->priv->vsbMax &&
			   dy >= (int)winHt);
   Boolean	scrollL = (This->priv->hsbOn &&
			   This->priv->hsbVal > 0 &&
			   dx <= (int)0);
   Boolean	scrollR = (This->priv->hsbOn &&
   			   This->priv->hsbVal < This->priv->hsbMax &&
			   dx >= (int)winWd);
   Boolean	offScreen = (scrollU || scrollD || scrollL || scrollR);
   if ( offScreen ) {
      This->priv->scrollEvent = *ev;
      This->priv->scrollEvent.x = dx;
      This->priv->scrollEvent.y = dy;
      This->priv->vScrollAction = (char *) (scrollU ? "IncrementUpOrLeft"
				          : scrollD ? "IncrementDownOrRight"
					            : NULL);
      This->priv->hScrollAction = (char *) (scrollL ? "IncrementUpOrLeft"
				          : scrollR ? "IncrementDownOrRight"
					           : NULL);
#if 0
//      if ( debuglev > 1 ) {
	 cout <<"Event off-screen, scrolling ";
	 if      ( scrollU ) cout <<"up ";
	 else if ( scrollD ) cout <<"down ";
	 else		     cout <<"none ";
	 cout <<"and ";
	 if      ( scrollL ) cout <<"left.";
	 else if ( scrollR ) cout <<"right.";
	 else		     cout <<"none.";
	 cout <<endl;
//      }
#endif
   }

//
// If there is a scroll timer running, see if we moved back on the screen
//
   if ( This->priv->scrollTimer && !offScreen ) {

//      cout <<"Stopping auto-scroll timer." <<endl;
      XtRemoveTimeOut(This->priv->scrollTimer);
      This->priv->scrollTimer = (XtIntervalId)NULL;

   } // End if there is a scroll timer

//
// If there's no scroll timer, see if we need one
//
   else if ( This->priv->timerOk && !This->priv->scrollTimer && offScreen ) {

//      if ( debuglev > 1 )
//	 cout <<"Starting auto-scroll timer" <<endl;

      This->priv->scrollTimer = XtAppAddTimeOut(halApp->context,
				    This->priv->autoScrollInterval,
				    (XtTimerCallbackProc)HandleAutoScroll,
				    (XtPointer)This->priv);

   } // End if there is no scroll timer

//
// If we're on the screen, process this event
//
   if ( !offScreen )
      This->priv->UpdateMotionSelection(dx + This->priv->hsbVal,
					dy + This->priv->vsbVal,
					True/*scroll*/);

} // End ActSelectMotion

/*---------------------------------------------------------------
 *  Update the selection after a motion event
 */

void
MimeRichTextP::UpdateMotionSelection(int x, int y, Boolean scroll)
{
//
// Find the position of the cursor
//
   SoftLineC	*pline = PickLine(y);
   if ( !pline ) {
      pline = botSoftLine;
      if ( pline ) x = pline->bounds.xmax;
   }
   ScreenPosC	spos(pline, x);
   TextPosC	newPos = spos;

   HideCursor();

//
// See if we're selecting a word at a time
//
   if ( selectType == SELECT_WORD ) {

//
// If the new position is to the left of the base beginning position, select
//    to the beginning of the word under the cursor.
//
      if ( newPos < baseSelectBegPos ) {
	 TextPosC	wordPos;
	 if ( FindPosBegClass(newPos, &wordPos) )
	    UpdateSelection(wordPos, baseSelectEndPos);
      }

//
// If the new position is to the right of the base ending position, select
//    to the end of the word under the cursor.
//
      else if ( newPos > baseSelectEndPos ) {
	 TextPosC	wordPos;
	 if ( FindPosEndClass(newPos, &wordPos) )
	    UpdateSelection(baseSelectBegPos, wordPos);
      }

//
// Restore the base selection if the new position is inside it.
//
      else {
	 UpdateSelection(baseSelectBegPos, baseSelectEndPos);
      }

   } // End if we're selecting a word at a time

//
// See if we're selecting a line at a time
//
   else if ( selectType == SELECT_LINE ) {

//
// If the new position is to the left of the base beginning position, select
//    to the beginning of the line under the cursor.
//
      if ( newPos < baseSelectBegPos ) {
	 TextPosC	linePos(newPos.textLine, 0, 0);
	 UpdateSelection(linePos, baseSelectEndPos);
      }

//
// If the new position is to the right of the base ending position, select
//    to the end of the line under the cursor.
//
      else if ( newPos > baseSelectEndPos ) {
	 ScreenPosC	endPos(newPos);
	 endPos.Set(endPos.softLine, endPos.softLine->bounds.xmax);
	 TextPosC	linePos(endPos);
	 UpdateSelection(baseSelectBegPos, linePos);
      }

//
// Restore the base selection if the new position is inside it.
//
      else {
	 UpdateSelection(baseSelectBegPos, baseSelectEndPos);
      }

   } // End if we're selecting a line at a time

//
// See if we're selecting a character at a time
//
   else if ( selectType == SELECT_CHAR ) {

//
// If the new position is to the left of the base position, select from the
//    new position to the base position.
//
      if ( newPos <= baseSelectBegPos )
	 UpdateSelection(newPos, baseSelectBegPos);

//
// If the new position is to the right of the base position, select from the
//    base position to the new position.
//
      else if ( newPos > baseSelectBegPos )
	 UpdateSelection(baseSelectBegPos, newPos);

   } // End if selecting a character at a time

   cursorPos = newPos;
   if ( scroll ) ScrollToCursor();
   desiredCursorX = spos.x;

   ShowCursor();

} // End UpdateMotionSelection

/*-----------------------------------------------------------------------
 *  Handle automatic scrolling
 */

void
MimeRichTextP::HandleAutoScroll(MimeRichTextP *This, XtIntervalId*)
{
//
// Call the action proc to scroll
//
   char	*parms;
   if ( This->vScrollAction ) {
//      if ( debuglev > 1 )
//	 cout <<"Auto-scroll vertical: " <<This->vScrollAction <<endl;
      parms = "0";	// Up or Down
      XtCallActionProc(This->textVSB, This->vScrollAction,
		       (XEvent*)&This->scrollEvent, &parms, 1);
   }

   if ( This->hScrollAction ) {
//      if ( debuglev > 1 )
//	 cout <<"Auto-scroll horizontal: " <<This->hScrollAction <<endl;
      parms = "1";	// Left or Right
      XtCallActionProc(This->textHSB, This->hScrollAction,
		       (XEvent*)&This->scrollEvent, &parms, 1);
   }

   This->UpdateMotionSelection(This->scrollEvent.x + This->hsbVal,
			       This->scrollEvent.y + This->vsbVal,
			       False/*don't scroll*/);

//
// Repeat the cycle
//
   This->scrollTimer = XtAppAddTimeOut(halApp->context,
				       This->autoScrollInterval,
				       (XtTimerCallbackProc)HandleAutoScroll,
				       (XtPointer)This);

} // End HandleAutoScroll

/*---------------------------------------------------------------
 *  Handle end of the selection
 */

void
MimeRichTextP::ActSelectEnd(Widget w, XButtonEvent *ev, String*, Cardinal*)
{
   //if ( debuglev > 1 ) cout <<"Button up" <<endl;

   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);
   This->priv->selectTime = ev->time;

//
// If we're in a multi-click, return
//
   if ( This->priv->clickTimer ) {
      This->priv->okToEndSelection = True;
      return;
   }
   This->priv->okToEndSelection = False;

   if ( debuglev > 1 ) cout <<"End selection" <<endl;

//
// Stop any auto scrolling
//
   if ( This->priv->scrollTimer ) {

//      cout <<"Stopping auto-scroll timer." <<endl;
      XtRemoveTimeOut(This->priv->scrollTimer);
      This->priv->scrollTimer = (XtIntervalId)NULL;
   }

//
// If there's no selection, return
//
   if ( This->priv->selectBegPos == This->priv->selectEndPos ) return;

//
// Claim ownership of the primary selection
//
   This->priv->SelectionChanged();

   return;

} // End ActSelectEnd

/*---------------------------------------------------------------
 *  Handle extend of the selection
 */

void
MimeRichTextP::ActSelectExtend(Widget w, XEvent *ev, String*, Cardinal*)
{
   if ( debuglev > 1 ) cout <<"Select extend" <<endl;

   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);

   int	x, y;
   if ( ev->type == MotionNotify ) {
      XMotionEvent	*mev = (XMotionEvent*)ev;
      x = mev->x + This->priv->hsbVal;
      y = mev->y + This->priv->vsbVal;
      This->priv->selectTime = mev->time;
   }
   else if ( ev->type == KeyPress || ev->type == KeyRelease ) {
      XKeyEvent	*kev = (XKeyEvent*)ev;
      x = kev->x + This->priv->hsbVal;
      y = kev->y + This->priv->vsbVal;
      This->priv->selectTime = kev->time;
   }
   else {	// Button
      XButtonEvent	*bev = (XButtonEvent*)ev;
      x = bev->x + This->priv->hsbVal;
      y = bev->y + This->priv->vsbVal;
      This->priv->selectTime = bev->time;
   }

   SoftLineC	*pline = This->priv->PickLine(y);
   if ( !pline ) {
      pline = This->priv->botSoftLine;
      if ( pline ) x = pline->bounds.xmax;
   }
   ScreenPosC	spos(pline, x);
   TextPosC	newPos = spos;

   TextPosC	*begPos = &This->priv->selectBegPos;
   TextPosC	*endPos = &This->priv->selectEndPos;

//
// Swap points if they are reversed
//
   if ( *begPos > *endPos ) {
      begPos = &This->priv->selectEndPos;
      endPos = &This->priv->selectBegPos;
   }

   This->priv->HideCursor();

//
// See if we've moved the starting position
//
   if ( newPos < *begPos )
      This->priv->UpdateSelection(newPos, *endPos);

//
// See if we've moved the ending position
//
   else if ( newPos > *endPos )
      This->priv->UpdateSelection(*begPos, newPos);

//
// See if we've inside the selection
//
   else if ( newPos > *begPos && newPos < *endPos ) {

//
// See which one we're closer to
//
      ScreenPosC	begSpos = *begPos;
      ScreenPosC	endSpos = *endPos;
      int		begDist = This->priv->Distance(spos, begSpos);
      int		endDist = This->priv->Distance(spos, endSpos);

//
// See if we're closer to the start
//
      if ( begDist < endDist )
	 This->priv->UpdateSelection(newPos, *endPos);

//
// See if we're closer to the end
//
      else
	 This->priv->UpdateSelection(*begPos, newPos);

   } // End if shrinking the area

   This->priv->cursorPos = newPos;
   This->priv->ScrollToCursor();
   This->priv->desiredCursorX = spos.x;

   This->priv->ShowCursor();

} // End ActSelectExtend

/*---------------------------------------------------------------
 *  Extend the selection one character to the left
 */

void
MimeRichTextP::ActSelectLeftChar(Widget w, XKeyEvent *ev, String*, Cardinal*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);
   This->priv->selectTime = ev->time;
   This->SelectLeftChar();
}

void
MimeRichTextC::SelectLeftChar()
{
   priv->HideCursor();

   TextPosC	newPos;
   if ( priv->FindPosPrevChar(priv->cursorPos, &newPos) ) {

      if ( priv->selectOn ) {

	 TextPosC	*begPos = &priv->selectBegPos;
	 TextPosC	*endPos = &priv->selectEndPos;
	 if ( *begPos > *endPos ) {
	    begPos = &priv->selectEndPos;
	    endPos = &priv->selectBegPos;
	 }

	 if ( priv->cursorPos == *begPos ) {
	    priv->DrawSelectionRange(&newPos, begPos);
	    *begPos = newPos;
	 }
	 else {
	    priv->DrawSelectionRange(&newPos, endPos);
	    *endPos = newPos;
	 }

      } // End if there is a current selection

      else {
	 priv->selectBegPos = newPos;
	 priv->selectEndPos = priv->cursorPos;
	 priv->DrawSelection();
	 priv->selectOn = True;
      }

      priv->cursorPos = newPos;
      priv->ScrollToCursor();
      ScreenPosC	spos = priv->cursorPos;
      priv->desiredCursorX = spos.x;

      priv->SelectionChanged();

   } // End if there is a position to the left

   priv->ShowCursor();

} // End SelectLeftChar

/*---------------------------------------------------------------
 *  Extend the selection one word to the left
 */

void
MimeRichTextP::ActSelectLeftWord(Widget w, XKeyEvent *ev, String*, Cardinal*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);
   This->priv->selectTime = ev->time;
   This->SelectLeftWord();
}

void
MimeRichTextC::SelectLeftWord()
{
   priv->HideCursor();

   TextPosC	newPos;
   if ( priv->FindPosPrevWord(priv->cursorPos, &newPos) ) {

      if ( priv->selectOn ) {

	 TextPosC	*begPos = &priv->selectBegPos;
	 TextPosC	*endPos = &priv->selectEndPos;
	 if ( *begPos > *endPos ) {
	    begPos = &priv->selectEndPos;
	    endPos = &priv->selectBegPos;
	 }

	 if ( priv->cursorPos == *begPos ) {
	    priv->DrawSelectionRange(&newPos, begPos);
	    *begPos = newPos;
	 }
	 else {
	    priv->DrawSelectionRange(&newPos, endPos);
	    *endPos = newPos;
	 }

      } // End if there is a current selection

      else {
	 priv->selectBegPos = newPos;
	 priv->selectEndPos = priv->cursorPos;
	 priv->DrawSelection();
	 priv->selectOn = True;
      }

      priv->cursorPos = newPos;
      priv->ScrollToCursor();
      ScreenPosC	spos = priv->cursorPos;
      priv->desiredCursorX = spos.x;

      priv->SelectionChanged();

   } // End if there is a position to the left

   priv->ShowCursor();

} // End SelectLeftWord

/*---------------------------------------------------------------
 *  Extend the selection one character to the right
 */

void
MimeRichTextP::ActSelectRightChar(Widget w, XKeyEvent *ev, String*, Cardinal*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);
   This->priv->selectTime = ev->time;
   This->SelectRightChar();
}

void
MimeRichTextC::SelectRightChar()
{
   priv->HideCursor();

   TextPosC	newPos;
   if ( priv->FindPosNextChar(priv->cursorPos, &newPos) ) {

      if ( priv->selectOn ) {

	 TextPosC	*begPos = &priv->selectBegPos;
	 TextPosC	*endPos = &priv->selectEndPos;
	 if ( *begPos > *endPos ) {
	    begPos = &priv->selectEndPos;
	    endPos = &priv->selectBegPos;
	 }

	 if ( priv->cursorPos == *begPos ) {
	    priv->DrawSelectionRange(begPos, &newPos);
	    *begPos = newPos;
	 }
	 else {
	    priv->DrawSelectionRange(endPos, &newPos);
	    *endPos = newPos;
	 }

      } // End if there is a current selection

      else {
	 priv->selectBegPos = priv->cursorPos;
	 priv->selectEndPos = newPos;
	 priv->DrawSelection();
	 priv->selectOn = True;
      }

      priv->cursorPos = newPos;
      priv->ScrollToCursor();
      ScreenPosC	spos = priv->cursorPos;
      priv->desiredCursorX = spos.x;

      priv->SelectionChanged();

   } // End if there is a position to the left

   priv->ShowCursor();

} // End SelectRightChar

/*---------------------------------------------------------------
 *  Extend the selection one word to the right
 */

void
MimeRichTextP::ActSelectRightWord(Widget w, XKeyEvent *ev, String*, Cardinal*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);
   This->priv->selectTime = ev->time;
   This->SelectRightWord();
}

void
MimeRichTextC::SelectRightWord()
{
   priv->HideCursor();

   TextPosC	newPos;
   if ( priv->FindPosNextWord(priv->cursorPos, &newPos) ) {

      if ( priv->selectOn ) {

	 TextPosC	*begPos = &priv->selectBegPos;
	 TextPosC	*endPos = &priv->selectEndPos;
	 if ( *begPos > *endPos ) {
	    begPos = &priv->selectEndPos;
	    endPos = &priv->selectBegPos;
	 }

	 if ( priv->cursorPos == *begPos ) {
	    priv->DrawSelectionRange(begPos, &newPos);
	    *begPos = newPos;
	 }
	 else {
	    priv->DrawSelectionRange(endPos, &newPos);
	    *endPos = newPos;
	 }

      } // End if there is a current selection

      else {
	 priv->selectBegPos = priv->cursorPos;
	 priv->selectEndPos = newPos;
	 priv->DrawSelection();
	 priv->selectOn = True;
      }

      priv->cursorPos = newPos;
      priv->ScrollToCursor();
      ScreenPosC	spos = priv->cursorPos;
      priv->desiredCursorX = spos.x;

      priv->SelectionChanged();

   } // End if there is a position to the left

   priv->ShowCursor();

} // End SelectRightWord

/*---------------------------------------------------------------
 *  Extend the selection to the start of the line
 */

void
MimeRichTextP::ActSelectLineBeg(Widget w, XKeyEvent *ev, String*, Cardinal*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);
   This->priv->selectTime = ev->time;
   This->SelectLineBeg();
}

void
MimeRichTextC::SelectLineBeg()
{
   priv->HideCursor();

   ScreenPosC	newPos;
   Boolean	success = False;

   if ( priv->selectOn ) {

      TextPosC	*begPos;
      if ( priv->selectBegPos > priv->selectEndPos )
	 begPos = &priv->selectEndPos;
      else
	 begPos = &priv->selectBegPos;

      ScreenPosC	spos = *begPos;
      newPos.Set(spos.softLine, 0);
      if ( newPos != spos ) {
	 TextPosC	tpos = newPos;
	 priv->DrawSelectionRange(&tpos, begPos);
	 *begPos = tpos;
	 success = True;
      }

   } // End if there's a current selection

   else if ( priv->cursorPos.textLine ) {

      ScreenPosC	spos = priv->cursorPos;
      newPos.Set(spos.softLine, 0);
      if ( newPos != spos ) {
	 priv->selectBegPos = newPos;
	 priv->selectEndPos = priv->cursorPos;
	 priv->DrawSelection();
	 priv->selectOn = True;
	 success = True;
      }

   } // End if no current selection

   if ( success ) {
      priv->cursorPos = newPos;
      priv->ScrollToCursor();
      ScreenPosC	spos = priv->cursorPos;
      priv->desiredCursorX = spos.x;
      priv->SelectionChanged();
   }

   priv->ShowCursor();

} // End SelectLineBeg

/*---------------------------------------------------------------
 *  Extend the selection to the end of the line
 */

void
MimeRichTextP::ActSelectLineEnd(Widget w, XKeyEvent *ev, String*, Cardinal*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);
   This->priv->selectTime = ev->time;
   This->SelectLineEnd();
}

void
MimeRichTextC::SelectLineEnd()
{
   priv->HideCursor();

   ScreenPosC	newPos;
   Boolean	success = False;

   if ( priv->selectOn ) {

      TextPosC	*endPos;
      if ( priv->selectBegPos > priv->selectEndPos )
	 endPos = &priv->selectBegPos;
      else
	 endPos = &priv->selectEndPos;

      ScreenPosC	spos = *endPos;
      newPos.Set(spos.softLine, spos.softLine->bounds.xmax);
      if ( newPos != spos ) {
	 TextPosC	tpos = newPos;
	 priv->DrawSelectionRange(endPos, &tpos);
	 *endPos = tpos;
	 success = True;
      }

   } // End if there's a current selection

   else if ( priv->cursorPos.textLine ) {

      ScreenPosC	spos = priv->cursorPos;
      newPos.Set(spos.softLine, spos.softLine->bounds.xmax);
      if ( newPos != spos ) {
	 priv->selectBegPos = priv->cursorPos;
	 priv->selectEndPos = newPos;
	 priv->DrawSelection();
	 priv->selectOn = True;
	 success = True;
      }

   } // End if no current selection

   if ( success ) {
      priv->cursorPos = newPos;
      priv->ScrollToCursor();
      ScreenPosC	spos = priv->cursorPos;
      priv->desiredCursorX = spos.x;
      priv->SelectionChanged();
   }

   priv->ShowCursor();

} // End SelectLineEnd

/*---------------------------------------------------------------
 *  Extend the selection to the previous line
 */

void
MimeRichTextP::ActSelectUpLine(Widget w, XKeyEvent *ev, String*, Cardinal*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);
   This->priv->selectTime = ev->time;
   This->SelectUpLine();
}

void
MimeRichTextC::SelectUpLine()
{
   priv->HideCursor();

//
// Find the position of the previous line
//
   SoftLineC	*prev = NULL;
   ScreenPosC	spos;
   if ( priv->selectOn ) {

      TextPosC	*begPos;
      if ( priv->selectBegPos > priv->selectEndPos )
	 begPos = &priv->selectEndPos;
      else
	 begPos = &priv->selectBegPos;

      spos = *begPos;
      prev = spos.softLine->prev;
      if ( prev ) {
	 spos.Set(prev, priv->desiredCursorX);
	 TextPosC	newPos = spos;
	 priv->DrawSelectionRange(&newPos, begPos);
	 *begPos = newPos;
      }
   }

   else {

      spos = priv->cursorPos;
      prev = spos.softLine->prev;
      if ( prev ) {
	 spos.Set(prev, priv->desiredCursorX);
	 TextPosC	newPos = spos;
	 priv->selectBegPos = newPos;
	 priv->selectEndPos = priv->cursorPos;
	 priv->DrawSelection();
	 priv->selectOn = True;
      }
   }

   if ( prev ) {
      priv->cursorPos = spos;
      priv->ScrollToCursor();
      priv->SelectionChanged();
   }

   priv->ShowCursor();

} // End SelectUpLine

/*---------------------------------------------------------------
 *  Extend the selection to the next line
 */

void
MimeRichTextP::ActSelectDownLine(Widget w, XKeyEvent *ev, String*, Cardinal*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);
   This->priv->selectTime = ev->time;
   This->SelectDownLine();
}

void
MimeRichTextC::SelectDownLine()
{
   priv->HideCursor();

//
// Find the position of the next line
//
   SoftLineC	*next = NULL;
   ScreenPosC	spos;
   if ( priv->selectOn ) {

      TextPosC	*endPos;
      if ( priv->selectBegPos > priv->selectEndPos )
	 endPos = &priv->selectBegPos;
      else
	 endPos = &priv->selectEndPos;

      spos = *endPos;
      next = spos.softLine->next;
      if ( next ) {
	 spos.Set(next, priv->desiredCursorX);
	 TextPosC	newPos = spos;
	 priv->DrawSelectionRange(endPos, &newPos);
	 *endPos = newPos;
      }
   }

   else {

      spos = priv->cursorPos;
      next = spos.softLine->next;
      if ( next ) {
	 spos.Set(next, priv->desiredCursorX);
	 TextPosC	newPos = spos;
	 priv->selectBegPos = priv->cursorPos;
	 priv->selectEndPos = newPos;
	 priv->DrawSelection();
	 priv->selectOn = True;
      }
   }

   if ( next ) {
      priv->cursorPos = spos;
      priv->ScrollToCursor();
      priv->SelectionChanged();
   }

   priv->ShowCursor();

} // End SelectDownLine

/*---------------------------------------------------------------
 *  Extend the selection to the previous paragraph
 */

void
MimeRichTextP::ActSelectUpPara(Widget w, XKeyEvent *ev, String*, Cardinal*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);
   This->priv->selectTime = ev->time;
   This->SelectUpPara();
}

void
MimeRichTextC::SelectUpPara()
{
#if 0
   priv->HideCursor();

   priv->ShowCursor();
#else
   SelectUpLine();
#endif

} // End SelectUpPara

/*---------------------------------------------------------------
 *  Extend the selection to the next paragraph
 */

void
MimeRichTextP::ActSelectDownPara(Widget w, XKeyEvent *ev, String*, Cardinal*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);
   This->priv->selectTime = ev->time;
   This->SelectDownPara();
}

void
MimeRichTextC::SelectDownPara()
{
#if 0
   priv->HideCursor();

   priv->ShowCursor();
#else
   SelectDownLine();
#endif
} // End SelectDownPara

/*---------------------------------------------------------------
 *  Extend the selection to the start of the file
 */

void
MimeRichTextP::ActSelectFileBeg(Widget w, XKeyEvent *ev, String*, Cardinal*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);
   This->priv->selectTime = ev->time;
   This->SelectFileBeg();
}

void
MimeRichTextC::SelectFileBeg()
{
   if ( !priv->topTextLine ) return;

   priv->HideCursor();

   TextPosC	newPos(priv->topTextLine, 0, 0);
   Boolean	success = False;

   if ( priv->selectOn ) {

      TextPosC	*begPos;
      if ( priv->selectBegPos > priv->selectEndPos )
	 begPos = &priv->selectEndPos;
      else
	 begPos = &priv->selectBegPos;

      if ( newPos != *begPos ) {
	 priv->DrawSelectionRange(&newPos, begPos);
	 *begPos = newPos;
	 success = True;
      }

   } // End if there's a current selection

   else if ( priv->cursorPos.textLine ) {

      if ( newPos != priv->cursorPos ) {
	 priv->selectBegPos = newPos;
	 priv->selectEndPos = priv->cursorPos;
	 priv->DrawSelection();
	 priv->selectOn = True;
	 success = True;
      }

   } // End if no current selection

   if ( success ) {
      priv->cursorPos = newPos;
      priv->ScrollToCursor();
      ScreenPosC	spos = priv->cursorPos;
      priv->desiredCursorX = spos.x;
      priv->SelectionChanged();
   }

   priv->ShowCursor();

} // End SelectFileBeg

/*---------------------------------------------------------------
 *  Extend the selection to the end of the file
 */

void
MimeRichTextP::ActSelectFileEnd(Widget w, XKeyEvent *ev, String*, Cardinal*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);
   This->priv->selectTime = ev->time;
   This->SelectFileEnd();
}

void
MimeRichTextC::SelectFileEnd()
{
   if ( !priv->botTextLine ) return;

   priv->HideCursor();

   TextLineC	*tline   = priv->botTextLine;
   unsigned	cmdCount = tline->cmdList.size();
   RichCmdC	*cmd     = tline->Cmd(cmdCount-1);
   unsigned	strSize  = cmd->LastPos();

   TextPosC	newPos(tline, cmdCount-1, strSize);
   Boolean	success = False;

   if ( priv->selectOn ) {

      TextPosC	*endPos;
      if ( priv->selectBegPos > priv->selectEndPos )
	 endPos = &priv->selectBegPos;
      else
	 endPos = &priv->selectEndPos;

      if ( newPos != *endPos ) {
	 priv->DrawSelectionRange(endPos, &newPos);
	 *endPos = newPos;
	 success = True;
      }

   } // End if there's a current selection

   else if ( priv->cursorPos.textLine ) {

      if ( newPos != priv->cursorPos ) {
	 priv->selectBegPos = priv->cursorPos;
	 priv->selectEndPos = newPos;
	 priv->DrawSelection();
	 priv->selectOn = True;
	 success = True;
      }

   } // End if no current selection

   if ( success ) {
      priv->cursorPos = newPos;
      priv->ScrollToCursor();
      ScreenPosC	spos = priv->cursorPos;
      priv->desiredCursorX = spos.x;
      priv->SelectionChanged();
   }

   priv->ShowCursor();

} // End SelectFileEnd

/*---------------------------------------------------------------
 *  Invoke the web browser to view the URL under the cursor
 */

//
// This routine has been changed to handle two cases:  URL click or a
// x-link click.
//

void
MimeRichTextP::ActFollowURL(Widget w, XButtonEvent *ev, String*, Cardinal*)
{
   if ( debuglev > 0 ) cout <<"Follow URL" <<endl;

   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);

   int	x = ev->x + This->priv->hsbVal;
   int	y = ev->y + This->priv->vsbVal;

//
// Check the command under the cursor and verify that it is a URL
//
   SoftLineC	*pline = This->priv->PickLine(y);
   if ( !pline ) {
      pline = This->priv->botSoftLine;
      if ( pline ) x = pline->bounds.xmax;
   }
   ScreenPosC	urlScreenPos(pline, x);
   TextPosC	urlTextPos = urlScreenPos;

//
// If there is a selection and the click is in the selection, use that instead
//
   StringC	urlText;
   StringC	*linkText = NULL;
   Boolean	isURL = False;
   if ( This->priv->selectOn && urlTextPos >= This->priv->selectBegPos &&
      				urlTextPos <= This->priv->selectEndPos ) {
      This->priv->GetSelectionData(urlText, TT_PLAIN);
   }
   else {
      RichCmdC	*urlCmd = urlTextPos.Cmd();
      if ( !urlCmd || !urlCmd->text ) {
	 XBell(halApp->display, 0);
	 return;
      } else if (urlCmd->state.URL()) {
	 isURL = True;
 	 urlText = *urlCmd->text;
      } else if (urlCmd->state.PLinkCount()) {
	 linkText = urlCmd->state.PLink();
      } else {
	 XBell(halApp->display, 0);
	 return;
      }
   }

//
// Add the URL text to the web browser command
//
   if (isURL) {
      StringC	cmd = This->priv->webCommand;
      CharC	range = cmd;
      int	pos = range.PosOf("%s");
      while ( pos >= 0 ) {
	 cmd(pos,2) = urlText;
	 range = cmd;
	 pos = range.PosOf("%s");
      }

//
// Run this command in the background
//
      if ( debuglev > 0 ) cout <<"Executing command: " <<cmd <<endl;
      if ( !cmd.EndsWith("&") ) cmd += " &";

      halApp->BusyCursor(True);
      System(cmd);
      halApp->BusyCursor(False);

//
// If not a URL, call the link callbacks
   } else {
     CallCallbacks(This->priv->linkCalls, linkText);
   }

} // End ActFollowURL

/*---------------------------------------------------------------
 *  Claim ownership of the selection and copy the text to the clipboard
 */

void
MimeRichTextP::SelectionChanged()
{
   if ( !selectOn ) return;

//
// Claim ownership of the primary selection
//
   Window	swin = XGetSelectionOwner(halApp->display, XA_PRIMARY);
   if ( swin != XtWindow(textDA) &&
        !XtOwnSelection(textDA, XA_PRIMARY, selectTime,
			(XtConvertSelectionProc)SendSelection,
			(XtLoseSelectionProc)LoseSelection, NULL) ) {

//
// Clear current selection since we can't own it.
//
      DrawSelection();
      selectOn = False;
      return;
   }

//
// Copy selected text to clipboard
//
   int			status;
   long			clipId;
   Window		win   = XtWindow(textDA);
   static XmString	label = (XmString)NULL;
   if ( label == (XmString)NULL ) {
      label = XmStringCreateSimple("MimeRichTextSelection");
      XmClipboardRegisterFormat(halApp->display, "STRING", 8);
      XmClipboardRegisterFormat(halApp->display, MIME_ENRICHED_ATOM_NAME, 8);
   }

   halApp->BusyCursor(True);

//
// Initiate the copy
//
   do {
      status = XmClipboardStartCopy(halApp->display, win, label,
				    selectTime, NULL, NULL, &clipId);
   } while ( status == ClipboardLocked );

//
// Copy the plaintext version
//
   StringC	data;
   GetSelectionData(data, TT_PLAIN);

   do {
      status = XmClipboardCopy(halApp->display, win, clipId, "STRING",
			       (char*)data, data.size()+1, 0, NULL);
   } while ( status == ClipboardLocked );

//
// Copy the enriched version
//
   data.Clear();
   GetSelectionData(data, TT_ENRICHED);

   do {
      status = XmClipboardCopy(halApp->display, win, clipId,
			       MIME_ENRICHED_ATOM_NAME, (char*)data,
			       data.size()+1, 0, NULL);
   } while ( status == ClipboardLocked );

//
// End the copy
//
   do {
      status = XmClipboardEndCopy(halApp->display, win, clipId);
   } while ( status == ClipboardLocked );

   halApp->BusyCursor(False);

} // End SelectionChanged

/*---------------------------------------------------------------
 *  Post a popup menu for the text widget or one of the graphic
 *     attachments.
 */

void
MimeRichTextP::ActPostMenu(Widget w, XButtonEvent *ev, String*, Cardinal*)
{
   MimeRichTextC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);

//
// See if this click is in a graphic
//
   RichGraphicC	*rg = This->priv->PickGraphic(ev->x, ev->y);
   if ( rg ) {
      if ( debuglev > 0 ) cout <<"Post menu in graphic" <<endl;
      rg->PostMenu(ev);
      return;
   }

   if ( debuglev > 0 ) cout <<"Post menu" <<endl;

} // End ActPostMenu
