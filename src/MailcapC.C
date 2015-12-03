/*
 *  $Id: MailcapC.C,v 1.2 2000/05/07 12:26:12 fnevgeny Exp $
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
#include "MailcapC.h"

#include <hgl/CharC.h>

static void
StripQuotes(StringC& string)
{
   if ( string.StartsWith('"') && string.EndsWith('"') )
      string.CutBoth(1);
}

/*-----------------------------------------------------------------------
 * MailcapC constructor
 */

MailcapC::MailcapC(CharC line)
{
   needsterminal = False;
   copiousoutput = False;
   isText        = False;

//
// Loop through tokens
//
   u_int	loff  = 0;
   CharC	token = line.NextWord(loff, ';');
   while ( token.Length() > 0 ) {

      loff = token.Addr() - line.Addr() + token.Length();
      token.Trim();

//
// type/subtype and display command are required.  Others are optional
//
      if ( fullType.size() == 0 ) {

	 fullType = token;

//
// Decompose type into content-type and subtype
//
	 CharC	full = fullType;
	 int	pos = full.PosOf('/');
	 if ( pos >= 0 ) {
	    conType = full(0,pos);
	    subType = full(pos+1,full.Length()-pos-1);
	 }
	 else {
	    conType = full;
	    subType = '*';
	 }

	 isText = conType.Equals("text", IGNORE_CASE);

      } // End if found type token

      else if ( present.size() == 0 ) {
	 present = token;
	 StripQuotes(present);
      }
      else if ( token.StartsWith("compose=", IGNORE_CASE) ) {
	 token.CutBeg(8);
	 token.Trim();
	 compose = token;
	 StripQuotes(compose);
      }
      else if ( token.StartsWith("composetyped=", IGNORE_CASE) ) {
	 token.CutBeg(12);
	 token.Trim();
	 composetyped = token;
	 StripQuotes(composetyped);
      }
      else if ( token.StartsWith("print=", IGNORE_CASE) ) {
	 token.CutBeg(6);
	 token.Trim();
	 print = token;
	 StripQuotes(print);
      }
      else if ( token.StartsWith("edit=", IGNORE_CASE) ) {
	 token.CutBeg(5);
	 token.Trim();
	 edit = token;
	 StripQuotes(edit);
      }
      else if ( token.StartsWith("test=", IGNORE_CASE) ) {
	 token.CutBeg(5);
	 token.Trim();
	 test = token;
	 StripQuotes(test);
      }
      else if ( token.StartsWith("x11-bitmap=", IGNORE_CASE) ) {
	 token.CutBeg(11);
	 token.Trim();
	 bitmap = token;
	 StripQuotes(bitmap);
      }
      else if ( token.StartsWith("description=", IGNORE_CASE) ) {
	 token.CutBeg(12);
	 token.Trim();
	 desc = token;
	 StripQuotes(desc);
      }
      else if ( token.StartsWith("needsterminal", IGNORE_CASE) )
	 needsterminal = True;
      else if ( token.StartsWith("copiousoutput", IGNORE_CASE) )
	 copiousoutput = True;

      token = line.NextWord(loff, ';');

   } // End for each token

   fullType.toLower();
   conType.toLower();
   subType.toLower();
   if ( conType.size() == 0 ) conType = "*";
   if ( subType.size() == 0 ) subType = "*";

} // End MailcapC constructor

/*-----------------------------------------------------------------------
 * MailcapC destructor
 */

MailcapC::~MailcapC()
{
}

