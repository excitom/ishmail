/*
 *  $Id: AddressC.C,v 1.2 2000/05/07 12:26:11 fnevgeny Exp $
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
#include "AddressC.h"
#include "HeaderValC.h"

#include <hgl/RegexC.h>
#include <hgl/StringC.h>

RegexC	*AddressC::anglePat = NULL;
RegexC	*AddressC::parenPat = NULL;
RegexC	*AddressC::quotePat = NULL;

extern int	debuglev;

/*---------------------------------------------------------------
 *  AddressC constructor
 */

AddressC::AddressC(CharC src)
{
   full = NULL;
   name = NULL;
   next = NULL;

//
// Get the first entry in the (possible) list
//
   u_int	nextPos;
   CharC	entry = FirstEntry(src, &nextPos);
   entry.Trim();
   if ( entry.Length() > 0 ) {

      Parse(entry);

//
// See if there are any more entries
//
      src.CutBeg(nextPos);
      src.Trim();
      if ( src.Length() > 0 )
	 next = new AddressC(src);
   }

} // End constructor

/*---------------------------------------------------------------
 *  AddressC destructor
 */

AddressC::~AddressC()
{
   delete full;
   delete name;
   delete next;
}

/*---------------------------------------------------------------
 *  Function to return the first address entry in a (possible) list of
 *     addresses
 */

CharC
AddressC::FirstEntry(CharC str, u_int *nextPos)
{
   *nextPos = 0;

//
// Skip initial whitespace and commas
//
   char	c = str[0];
   while ( c && (c == ',' || c == ' ' || c == '\t' || c == '\n') ) {
      str.CutBeg(1);
      c = str[0];
      (*nextPos)++;
   }

//
// First look for a comma or newline.  If there's none, the address goes all the
//    way to the end.
//
   int	cpos = str.PosOf(',');
   int	npos = str.PosOf('\n');
   int	pos  = (cpos < 0 ? npos : (npos < 0 ? cpos : MIN(cpos, npos)));
   while ( pos > 0 ) {

//
// See if the separator is escaped
//
      if ( str[pos-1] == '\\' ) {
	 cpos = str.PosOf(',',  (u_int)pos+1);
	 npos = str.PosOf('\n', (u_int)pos+1);
	 pos  = (cpos < 0 ? npos : (npos < 0 ? cpos : MIN(cpos, npos)));
	 continue;
      }

//
// See if the separator is between parens or quotes.  To do this, start at the
//    beginning and see if either is unbalanced.
//
      int	parenLevel = 0;
      Boolean	quoted = False;
      for (int i=0; i<pos; i++ ) {

	 c = str[i];
	 char	b = (i>0) ? str[i-1] : 0;

	 if ( c == '(' ) {
	    if ( b != '\\' ) parenLevel++;
	 }
	 else if ( c == ')' ) {
	    if ( parenLevel>0 && b != '\\' ) parenLevel--;
	 }
	 else if ( c == '"' ) {
	    if ( b != '\\' ) quoted = !quoted;
	 }
      }

      if ( quoted || parenLevel > 0 ) {
	 cpos = str.PosOf(',',  (u_int)pos+1);
	 npos = str.PosOf('\n', (u_int)pos+1);
	 pos  = (cpos < 0 ? npos : (npos < 0 ? cpos : MIN(cpos, npos)));
      }
      else {
	 *nextPos += pos+1;
	 return str(0, pos);			// Comma was legit
      }

   } // End for each comma

//
// We never found any real separators.
//
   *nextPos += str.Length();
   return str;

} // FirstEntry

/*---------------------------------------------------------------
 *  Function to parse the addres string into it's component pieces
 */

void
AddressC::Parse(CharC str)
{
   if ( !anglePat ) {
      anglePat = new RegexC("<\\([^>]*\\)>");
      parenPat = new RegexC("(\\(.*\\))");
      quotePat = new RegexC("\"\\([^\"]*\\)\"");
   }

   StringC	cmt;
   StringC	tmpAddr;
   int		pos;

   full = new char[str.Length()+1];
   strncpy(full, str.Addr(), str.Length());
   full[str.Length()] = 0;
   addr = full;

//
// Look for one of the known formats
//
   if ( anglePat->search(full) >= 0 ) {

//
// Get the address from within the angle brackets
//
      addr = addr((*anglePat)[1]);

//
// The comment will be whatever's left
//
      cmt = full;
      cmt((*anglePat)[0]) = "";

   } // End if <address> found

   else if ( parenPat->search(full) >= 0 ) {

//
// Get the comment from within the parenthesis
//
      cmt = str((*parenPat)[1]);

//
// The address will be whatever's left
//
      tmpAddr = full;
      tmpAddr((*parenPat)[0]) = "";
      tmpAddr.Trim();

      pos = addr.PosOf(tmpAddr, IGNORE_CASE);
      if ( pos >= 0 )
	 addr = addr(pos, tmpAddr.size());

   } // End if (comment) found

   else if ( quotePat->search(full) >= 0 ) {

//
// Get the comment from within the quotes
//
      cmt = str((*quotePat)[1]);

//
// The address will be whatever's left
//
      tmpAddr = full;
      tmpAddr((*quotePat)[0]) = "";
      tmpAddr.Trim();

      pos = addr.PosOf(tmpAddr, IGNORE_CASE);
      if ( pos >= 0 )
	 addr = addr(pos, tmpAddr.size());

   } // End if "comment" found

   addr.Trim();

//
// Extract mailbox and site from address
//
   pos = addr.PosOf('@');
   if ( pos >= 0 ) {
      mailbox = addr(0,pos);
      site    = addr(pos+1,addr.Length());
   }

   else {
      mailbox = addr;
      site    = addr(0,0);
   }

   cmt.Trim();

//
// Remove any remaining quotes from the comment
//
   if ( cmt.size() > 0 ) {

      char	*cp = strchr((char *)cmt, '"');
      while ( cp ) {
	 int	pos = cp - (char*)cmt;
	 cmt(pos,1) = "";
	 cp = strchr((char *)cmt, '"');
      }

      cmt.Trim();
   }

//
// Remove any surrounding parens from the comment if there are not other
//    parens in the middle
//
   if ( cmt.size() > 0 && cmt[0] == '(' && cmt[cmt.size()-1] == ')' &&
	!strchr(((char*)cmt)+1,'(') ) {
      cmt = cmt(1,cmt.size()-2);
      cmt.Trim();
   }

   if ( cmt.size() > 0 )
      name = new HeaderValC(cmt);

} // End Parse

/*---------------------------------------------------------------
 *  Print method
 */

void
AddressC::Print(ostream& strm) const
{
   if ( full ) {
      strm <<"[" <<full <<"]" <<flush;
      if ( debuglev > 2 ) {
	 strm <<endl;
	 strm <<"        Addr [" <<addr <<"]" <<endl;
	 strm <<"     Mailbox [" <<mailbox <<"]" <<endl;
	 strm <<"        Site [" <<site <<"]" <<endl;
      }
   }
   if ( name && debuglev > 2 )
      strm <<"        Name [" <<*name <<"]" <<endl;
   if ( next ) strm <<*next;
}
