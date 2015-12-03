/*
 *  $Id: SendMisc.C,v 1.2 2000/05/07 12:26:13 fnevgeny Exp $
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
#include "SendMisc.h"
#include "Misc.h"
#include "IshAppC.h"
#include "MimeTypes.h"
#include "QuotedP.h"
#include "Base64.h"
#include "MailPrefC.h"

#include <hgl/CharC.h>
#include <hgl/StringListC.h>

/*---------------------------------------------------------------
 *  Method to add a single header to the list
 */

Boolean
AddHeader(StringC headStr, StringListC& headList, Boolean dupOk)
{
//
// Remove any existing copies of this header
//
   if ( !dupOk ) {
      CharC	newKey = headStr.NextWord(0, ':');
      u_int	count = headList.size();
      for (int i=count-1; i>=0; i--) {
	 StringC	*oldStr = headList[i];
	 CharC		oldKey = oldStr->NextWord(0, ':');
	 if ( newKey == oldKey ) headList.remove(i);
      }
   }

   if ( !headStr.EndsWith('\n') ) headStr += '\n';

//
// Encode this header if necessary
//
   if ( Contains8Bit(headStr) && ishApp->mailPrefs->headEncType != ET_NONE ) {
      if ( !EncodeHeader(headStr) ) return False;
   }

   headList.add(headStr);
   return True;

} // End AddHeader

/*---------------------------------------------------------------
 *  Method to encode header lines that contains 8-bit characters
 */

Boolean
EncodeHeader(StringC& headStr)
{
   if ( headStr.EndsWith('\n') ) headStr.CutEnd(1);

//
// Skip the header name
//
   CharC	val;
   int	pos = headStr.PosOf(':');
   if ( pos >= 0 ) {
      pos++;
      if ( isspace(headStr[pos]) ) pos++;
      val = headStr(pos, headStr.size());
   }
   else
      val = headStr;

//
// Encode the value
//
   StringC	result;
   Boolean	success;
   if ( ishApp->mailPrefs->headEncType == ET_BASE_64 )
      success = TextToText1522B(val, ishApp->mailPrefs->charset, &result);
   else
      success = TextToText1522Q(val, ishApp->mailPrefs->charset, &result);

   if ( success ) {
      headStr(pos, headStr.size()) = result;
      headStr += '\n';
   }

   return success;

} // End EncodeHeader

