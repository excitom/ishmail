/*
 * $Id: Misc.C,v 1.3 2000/05/31 15:26:56 evgeny Exp $
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

#include "IshAppC.h"
#include "Base64.h"

#include <hgl/HalAppC.h>
#include <hgl/CharC.h>
#include <hgl/StringListC.h>
#include <hgl/RegexC.h>

#include <Xm/Xm.h>
#include <X11/IntrinsicP.h>
#include <X11/CoreP.h>

#include <time.h>
#include <unistd.h>

/*---------------------------------------------------------------
 *  Function to extract tokens from a string and put them in a list.
 */

void
ExtractList(CharC str, StringListC& list, const char *delim)
{
   if ( !delim ) delim = " \t\n,";

   static StringC	*tmp = NULL;
   if ( !tmp ) tmp = new StringC;

   u_int	offset = 0;
   CharC	entry = str.NextWord(offset, delim);
   while ( entry.Length() > 0 ) {

      *tmp = entry;
      list.add(*tmp);

      offset = entry.Addr() - str.Addr() + entry.Length();
      entry = str.NextWord(offset, delim);
   }

} // End ExtractList

/*-----------------------------------------------------------------------
 *  Function to set a dialog's parent so it pops up in the desired place
 */

void
PopupOver(Widget w, Widget parent)
{
//
// Set the dialog's parent so it pops up in the desired place
//
   Widget       shell = XtParent(w);
   shell->core.parent = parent;
   XtVaSetValues(shell, XmNtransientFor, parent, NULL);

   XtManageChild(w);
   XMapRaised(halApp->display, XtWindow(shell));
}

/*-----------------------------------------------------------------------
 *  Function to convert a text string to lower case
 */

void
ToLower(char *cs)
{
   if ( !cs ) return;

   while ( *cs ) {
      *cs = to_lower(*cs);
      cs++;
   }
}

/*-------------------------------------------------------------------
 *  Determine if there are any 8-bit characters in a string
 */

Boolean
Contains8Bit(CharC str)
{
//
// Loop through the string
//
   const char	*cp = str.Addr();
   const char	*ep = cp + str.Length();
   while ( cp < ep ) {
#ifdef __CHAR_UNSIGNED__
      if ( *cp > 127 ) return True;
#else
      if ( *cp < 0 ) return True;
#endif
      cp++;
   }

   return False;

} // End Contains8Bit

/*---------------------------------------------------------------
 *  Function to check for valid enriched text commands
 */

static Boolean
IsEnrichedCommand(CharC cmd)
{
//
// Check for optional <>
//
   if ( cmd.Length() > 2 && cmd[0] == '<' )
      cmd.Set(cmd.Addr()+1, cmd.Length()-2);

//
// Remove leading /
//
   if ( cmd.Length() > 1 && cmd[0] == '/' )
      cmd.Set(cmd.Addr()+1, cmd.Length()-1);

//
// Check size of string
//
   if ( cmd.Length() > 40 ) return False;

//
// Make sure there are only alphanumeric and - characters
//
   unsigned	count = cmd.Length();
   for (int i=0; i<count; i++) {
      char	c = cmd[i];
      if ( !isalnum(c) && c != '-' ) return False;
   }

//
// Now check the known commands
//
   return ( cmd.Equals("bold",		IGNORE_CASE) ||
	    cmd.Equals("italic",	IGNORE_CASE) ||
	    cmd.Equals("fixed",		IGNORE_CASE) ||
	    cmd.Equals("smaller",	IGNORE_CASE) ||
	    cmd.Equals("bigger",	IGNORE_CASE) ||
	    cmd.Equals("underline",	IGNORE_CASE) ||
	    cmd.Equals("center",	IGNORE_CASE) ||
	    cmd.Equals("flushleft",	IGNORE_CASE) ||
	    cmd.Equals("flushright",	IGNORE_CASE) ||
	    cmd.Equals("flushboth",	IGNORE_CASE) ||
	    cmd.Equals("nofill",	IGNORE_CASE) ||
	    cmd.Equals("indent",	IGNORE_CASE) ||
	    cmd.Equals("indentright",	IGNORE_CASE) ||
	    cmd.Equals("excerpt",	IGNORE_CASE) ||
	    cmd.Equals("param",		IGNORE_CASE) );

} // End IsEnrichedCommand

/*-----------------------------------------------------------------------
 *  Function to determine if a character array contains any enriched text
 *     commands
 */

Boolean
IsEnriched(CharC string)
{
//
// Loop through string and look for <command>
//
   static RegexC	*cmdPat = NULL;
   if ( !cmdPat ) cmdPat = new RegexC("<\\([^>]+\\)>");

   CharC	cmdStr;
   RangeC	cmdRange;

   int	startPos = 0;
   int	pos;
   while ( (pos=cmdPat->search(string,startPos)) >= 0 ) {

      cmdRange = (*cmdPat)[1];
      cmdStr = string(cmdRange);

      if ( IsEnrichedCommand(cmdStr) ) return True;

      startPos = pos + cmdRange.length();

   } // End for each command

   return False;

} // End IsEnriched

/*-----------------------------------------------------------------------
 *  Function to determine if the first charset can be used where the
 *     second is supported.
 */

Boolean
CharsetOk(CharC cs1, CharC cs2)
{
//
// If the first one is ascii, it can be used anywhere
//
   cs1.Trim();
   if ( cs1.Length() == 0 || cs1.Equals("us-ascii", IGNORE_CASE) ) return True;

//
// If the second one is ascii, the first one must be ascii.
// 
   cs2.Trim();
   if ( cs2.Length() == 0 || cs2.Equals("us-ascii", IGNORE_CASE) ) return False;

//
// Since neither font is ascii, they must match.
// 
   if      ( cs1.StartsWith("iso-", IGNORE_CASE) ) cs1.CutBeg(4);
   else if ( cs1.StartsWith("iso",  IGNORE_CASE) ) cs1.CutBeg(3);

   if      ( cs2.StartsWith("iso-", IGNORE_CASE) ) cs2.CutBeg(4);
   else if ( cs2.StartsWith("iso",  IGNORE_CASE) ) cs2.CutBeg(3);

   return cs1.Equals(cs2, IGNORE_CASE);

} // End CharsetOk

/*---------------------------------------------------------------
 *  Function to generate a unique id
 */

void
GenId(StringC& str)
{
   static int	count = 1;
   StringC	user;

   TextToText64(ishApp->userId, &user, True/*IsText*/, False/*BreakLines*/);

   while ( user.EndsWith('=') ) user.CutEnd(1);
   user.toLower();

   str.Clear();
   str += count++;
   str += ".";
   str += (int)time(0);
   str += ".";
   str += (int)getpid();
   str += ".";
   str += user;
   str += "@";
   str += ishApp->domain;
}

/*---------------------------------------------------------------
 *  Function to generate a unique boundary label for mime messages
 */

void
GenBoundary(StringC& bound)
{
   StringC	bound1;
   GenId(bound1);
   bound.Clear();

//
// Encode the part after the count
//
   CharC	range = bound1;
   int		pos = range.PosOf('.');
   if ( pos > 0 ) range.CutBeg(pos);

   TextToText64(range, &bound, True/*IsText*/, False/*BreakLines*/);
   while ( bound.EndsWith('=') ) bound.CutEnd(1);

//
// Add the count back on
//
   bound1.Clear(pos);
   bound1 += bound;

//
// Restrict the length and convert to lower case so it can't be decoded.
//   A length of 50 allows the "boundary=" part of the content-type header
//   to fit on one header line with wrapping.
//
   if ( bound1.size() > 50 ) bound1.Clear(50);
   bound1.toLower();

//
// Make it pretty
//
   bound  = "----------";
   bound += bound1;

} // End GenBoundary

