/*
 *  $Id: HeaderValC.C,v 1.2 2000/05/07 12:26:11 fnevgeny Exp $
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
#include "HeaderValC.h"
#include "QuotedP.h"
#include "Base64.h"

#include <hgl/RegexC.h>
#include <hgl/CharC.h>
#include <hgl/StringC.h>

RegexC	*HeaderValC::pat1522 = NULL;

/*---------------------------------------------------------------
 *  HeaderValC constructors
 */

HeaderValC::HeaderValC()
{
   full    = NULL;
   text    = NULL;
   charset = NULL;
   next    = NULL;
}

HeaderValC::HeaderValC(CharC src)
{
   text    = NULL;
   charset = NULL;
   next    = NULL;

   src.Trim();

//
// Allocate space for full header
//
   full = new char[src.Length()+1];
   strncpy(full, src.Addr(), src.Length());
   full[src.Length()] = 0;

//
// Build a regular expression to match the following patterns:
//
//  "=?charset?Q?encoded-text?="
//  "=?charset?B?encoded-text?="
//
   if ( !pat1522 )
      pat1522 =
	 new RegexC("=\\?\\([^ \t\n?]+\\)\\?\\([BbQq]\\)\\?\\([^?]+\\)\\?=");

//
// Check for RFC1522 encoded headers
//
   int	pos = pat1522->search(src);
   if ( pos >= 0 ) {

      char		enc;
      CharC		tmpStr;
      int		start = 0;
      HeaderValC	*curVal = this;
      while ( pos >= 0 ) {

//
// Add any non-encoded text
//
	 u_int	len = pos - start;
	 if ( len > 0 ) {

//
// Single whitespace characters between two encoded patterns are ignored.
//    newline characters don't count.
//
	    if ( start > 0 && ((len == 1 && isspace(src[start])) ||
	      (len == 2 && src[start] == '\n' && isspace(src[start+1]))) )
	       ; // ignore
	    else {

	       if ( curVal->text ) {
		  curVal->next = new HeaderValC;
		  curVal = curVal->next;
	       }

	       tmpStr = src(start, pos-start);
	       curVal->text = new char[tmpStr.Length()+1];
	       strncpy(curVal->text, tmpStr.Addr(), tmpStr.Length());
	       curVal->text[tmpStr.Length()] = 0;
	    }

	 } // End if there is non-encoded text

//
// Add encoded text
//
	 if ( curVal->text ) {
	    curVal->next = new HeaderValC;
	    curVal = curVal->next;
	 }

//
// Store the charset
//
	 tmpStr = src((*pat1522)[1]);
	 curVal->charset = new char[tmpStr.Length()+1];
	 strncpy(curVal->charset, tmpStr.Addr(), tmpStr.Length());
	 curVal->charset[tmpStr.Length()] = 0;

//
// Decode the word
//
	 StringC	decStr;
	 enc = src[(*pat1522)[2].firstIndex()];
	 if ( enc == 'q' || enc == 'Q' )
	    Text1522QToText(src((*pat1522)[3]), &decStr);
	 else if ( enc == 'b' || enc == 'B' )
	    Text64ToText(src((*pat1522)[3]), &decStr);
	 else
	    decStr = src((*pat1522)[3]);

//
// Store the decoded word
//
	 curVal->text = new char[decStr.size()+1];
	 strcpy(curVal->text, decStr);

//
// Look for more text
//
	 start = pos + (*pat1522)[0].length();
	 pos = pat1522->search(src, start);

      } // End for each encoded word

//
// Store any remaining header text
//
      if ( start < src.Length() ) {

	 if ( curVal->text ) {
	    curVal->next = new HeaderValC;
	    curVal = curVal->next;
	 }

	 tmpStr = src(start, src.Length()-start);
	 curVal->text = new char[tmpStr.Length()+1];
	 strncpy(curVal->text, tmpStr.Addr(), tmpStr.Length());
	 curVal->text[tmpStr.Length()] = 0;
      }

   } // End if RFC1522 encoding is present

   else {
      text = full;
   }

} // End HeaderValC constructor

/*---------------------------------------------------------------
 *  HeaderValC destructor
 */

HeaderValC::~HeaderValC()
{
   delete full;
   if ( text != full ) delete text;
   delete charset;
   delete next;
}

/*---------------------------------------------------------------
 *  Print method
 */

void
HeaderValC::Print(ostream& strm) const
{
   if ( charset ) strm <<"(" <<charset <<")";
   if ( text    ) strm <<text;
   if ( next    ) strm <<*next;
}

/*---------------------------------------------------------------
 *  Method to return text representation of value
 */

void
HeaderValC::GetValueText(StringC& val) const
{
   if ( text ) val += text;
   if ( next ) next->GetValueText(val);
}
