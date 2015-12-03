/*
 *  $Id: HeaderC.C,v 1.2 2000/05/07 12:26:11 fnevgeny Exp $
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
#include "HeaderC.h"
#include "HeaderValC.h"

#include <hgl/StringC.h>
#include <hgl/RegexC.h>

extern int	debuglev;

/*---------------------------------------------------------------
 *  HeaderC constructor
 */

HeaderC::HeaderC(CharC src)
{
   value = NULL;
   next  = NULL;

//
// Remove trailing newlines
//
   while ( src.EndsWith('\n') ) src.CutEnd(1);

//
// Allocate space for full header
//
   full = new char[src.Length()+1];
   strncpy(full, src.Addr(), src.Length());
   full[src.Length()] = 0;

   StringC	tmp;
   int		len;

//
// Strip newlines for value
//
   u_int	offset = 0;
   int		pos = src.PosOf('\n', offset);
   while ( offset < src.Length() && pos >= 0 ) {

//
// Add text up to this point
//
      len = pos - (int)offset;
      if ( len > 0 )
	 tmp += src(offset, len);

//
// Skip newline and following whitespace
//
      offset = pos + 2;
      if ( offset < src.Length() )
	 pos = src.PosOf('\n', offset);
   }

//
// Add remaining text
//
   len = src.Length() - (int)offset;
   if ( len > 0 )
      tmp += src(offset, len);

//
// Get key and value text
//
   pos = src.PosOf(':');
   key.Set(full, (pos >= 0) ? pos : 0);
   key.Trim();

   tmp.CutBeg(key.Length() + 1);
   tmp.Trim();

//
// Massage subject header to make "re:'s" look nicer
//
   if ( key.Equals("Subject", IGNORE_CASE) )
      FixSubjectRe(tmp);

   value = new HeaderValC(tmp);

} // End HeaderC constructor

/*---------------------------------------------------------------
 *  HeaderC destructor
 */

HeaderC::~HeaderC()
{
   delete value;
   delete full;
   delete next;
}

/*---------------------------------------------------------------
 *  Print method
 */

void
HeaderC::Print(ostream& strm) const
{
   if ( full ) {
      strm <<"Header [" <<full <<"]" <<endl;
      if ( debuglev > 2 )
	 strm <<"   Key [" <<key <<"]" <<endl;
   }
   if ( value && debuglev > 2 )
      strm <<" Value [" <<*value <<"]" <<endl;
}

/*------------------------------------------------------------------------
 * Combine multiple Re:'s in subject line
 */

void
HeaderC::FixSubjectRe(StringC& subjStr)
{
//
// Build a pattern that will match Re[x]: or Re(x):
//
   static RegexC	*rePat = NULL;
   if ( !rePat ) rePat = new RegexC("[Rr][Ee][[(]\\([0-9]+\\)[])]:");

//
// Count the number of Re expressions in the subject line
//
   int		count = 0;
   int		level = 0;
   Boolean	done = False;
   char		*cp = subjStr;
   while ( !done ) {

      CharC	sub = cp;
      if ( sub.StartsWith("re:", IGNORE_CASE) ) {

	 count++;
	 level++;
	 cp += 3;
	 while ( (*cp != 0) && (*cp == ' ' || *cp == '\t') ) cp++;
      }

      else if ( rePat->match(cp) ) {

	 int	pos = (*rePat)[1].firstIndex();
	 int	len = (*rePat)[1].length();
	 int	end = pos+len;
	 char	save = cp[end];
	 cp[end] = 0;
	 int	num = atoi(&cp[pos]);
	 cp[end] = save;
	 count++;
	 level += num;
	 cp += (*rePat)[0].length();
	 while ( (*cp != 0) && (*cp == ' ' || *cp == '\t') ) cp++;
      }

      else {
	 done = True;
      }

   } // End while more Re's

//
// If there is more than one, combine them
//
   if ( count > 1 ) {

      done = False;
      while ( !done ) {

	 if ( subjStr.StartsWith("re:", IGNORE_CASE) ) {
	    subjStr.CutBeg(3);
	    subjStr.Trim();
	 }

	 else if ( rePat->match(subjStr) ) {
	    subjStr((*rePat)[0]) = "";
	    subjStr.Trim();
	 }

	 else {
	    done = True;
	 }

      } // End while more Re's

//
// Put back just one Re
//
      if ( level > 1 ) {

	 StringC	numStr("Re[");
	 numStr += level;
	 numStr += "]: ";
	 subjStr = numStr + subjStr;
      }

      else {
	 subjStr = "Re: " + subjStr;
      }

   } // End if multiple Re's found

} // End FixSubjectRe

/*------------------------------------------------------------------------
 * Method to return value text
 */

void
HeaderC::GetValueText(StringC& text)
{
   value->GetValueText(text);
}

/*---------------------------------------------------------------
 *  Function to build a list of headers from a header string
 */

HeaderC*
ExtractHeaders(CharC data)
{
   data.Trim();
   if ( data.Length() == 0 ) return NULL;

//
// Read a header line
//
   StringC	headStr = data.NextWord(0, '\n');
   data.CutBeg(headStr.size()+1);

   if ( headStr.size() == 0 ) return NULL;

//
// Read continuation lines
//
   CharC	line = data.NextWord(0, '\n');
   while ( isspace(line[0]) ) {
      headStr += line;
      data.CutBeg(line.Length()+1);
      line = data.NextWord(0, '\n');
   }

//
// Create a new header object
//
   HeaderC	*newHead = new HeaderC(headStr);

//
// Read and set links to additional headers
//
   newHead->next = ExtractHeaders(data);

   return newHead;

} // End ExtractHeaders

