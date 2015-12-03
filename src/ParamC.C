/*
 *  $Id: ParamC.C,v 1.2 2000/05/07 12:26:12 fnevgeny Exp $
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
#include "ParamC.h"

#include <hgl/StringC.h>

extern int	debuglev;

/*---------------------------------------------------------------
 *  ParamC constructor with unparsed string
 */

ParamC::ParamC(CharC src)
{
   src.Trim();

   full = NULL;
   next = NULL;

//
// Find the end of the first parameter in the (possible) list
//
   int	epos = src.PosOf('=');
   if ( epos <= 0 ) return;

//
// See if value is quoted
//
   int	pos = epos+1;
   if ( src[pos] == '"' ) {

//
// Look for an end quote.  Skip escaped ones.
//
      pos = src.PosOf('"', (u_int)(pos+1));
      while ( pos > 0 && src[pos-1] == '\\' )
	 pos = src.PosOf('"', (u_int)(pos+1));

   } // End if value is quoted

   else
      pos = -1;	// to look for semicolon

//
// If no quote was found, look for a semicolon.  Skip escaped ones.
//
   if ( pos < 0 ) {
      pos = src.PosOf(';');
      while ( pos > 0 && src[pos-1] == '\\' )
	 pos = src.PosOf(';', (u_int)(pos+1));
      if ( pos > 0 ) pos--;
   }

//
// If no quote or semicolon was found, use the whole string
//
   CharC	tmp;
   if ( pos < 0 ) {
      tmp = src;
   }
   else {
      pos++;
      tmp = src(0,pos);
      tmp.Trim();
   }

   full = new char[tmp.Length()+1];
   strncpy(full, tmp.Addr(), tmp.Length());
   full[tmp.Length()] = 0;

//
// Get the key and value parts
//
   epos = tmp.PosOf('=');
   key.Set(full, epos);
   key.Trim();

   val.Set(full+epos+1, strlen(full)-epos-1);
   if ( val[0] == '"' ) val.CutBoth(1);
   val.Trim();

//
// Convert key to lower case
//
   char	*cp = full;
   for (int i=0; i<epos; i++, cp++) *cp = to_lower(*cp);

//
// See if there are any more entries
//
   if ( pos > 0 ) {
      src.CutBeg(pos);
      if ( src[0] == ';' ) src.CutBeg(1);
      src.Trim();
      if ( src.Length() > 0 )
	 next = new ParamC(src);
   }

} // End constructor

/*---------------------------------------------------------------
 *  ParamC constructor with key/value pair
 */

ParamC::ParamC(CharC k, CharC v)
{
   full = NULL;
   next = NULL;
   Set(k, v);
}

/*---------------------------------------------------------------
 *  ParamC destructor
 */

ParamC::~ParamC()
{
   delete full;
   delete next;
}

/*---------------------------------------------------------------
 *  Method to update the value
 */

void
ParamC::Set(CharC k, CharC v)
{
   delete full;

   int	len = k.Length() + 1 + v.Length();
   full = new char[len+1];
   strncpy(full, k.Addr(), k.Length());
   full[k.Length()] = 0;
   strcat(full, "=");
   strncat(full, v.Addr(), v.Length());
   full[len] = 0;

   key.Set(full, k.Length());
   val.Set(full+k.Length()+1, v.Length());

} // End Set

/*---------------------------------------------------------------
 *  Method to update the value
 */

void
ParamC::SetValue(CharC v)
{
   StringC	k = key;
   Set(k, v);
}

/*---------------------------------------------------------------
 *  Print method
 */

void
ParamC::Print(ostream& strm) const
{
   if ( full ) {
      strm <<"Parameter [" <<full <<"]" <<endl;
      if ( debuglev > 2 ) {
	 strm <<"      Key [" <<key <<"]" <<endl;
	 strm <<"    Value [" <<val <<"]" <<endl;
      }
   }
}
