/*
 *  $Id: FieldC.h,v 1.1.1.1 2000/04/25 13:49:01 fnevgeny Exp $
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
#ifndef _FieldC_h_
#define _FieldC_h_

#include "StringC.h"

#include <Xm/XmStrDefs.h>

/*-----------------------------------------------------------------------
 *  View specific data for each field
 */

class FieldC {

public:

   StringC	string;
   char		*tag;
   int		width;
   int		height;

   inline void	operator=(const FieldC& f) {
      string = f.string;
      width  = f.width;
      height = f.height;
   }
   inline int  	operator==(const FieldC& f) const {return(string == f.string);}
   inline int  	operator!=(const FieldC& f) const { return !(*this==f); }
   inline int	compare(const FieldC& f) const
      { return string.compare(f.string); }
   inline int	operator<(const FieldC& f) const { return (compare(f) < 0); }
   inline int	operator>(const FieldC& f) const { return (compare(f) > 0); }

   FieldC() {
      tag    = XmFONTLIST_DEFAULT_TAG;
      width  = 0;
      height = 0;
   }
   FieldC(const FieldC& f) { *this = f; }
   ~FieldC() { if ( tag != XmFONTLIST_DEFAULT_TAG ) delete tag; }

   void	SetString(StringC& s, char *t=XmFONTLIST_DEFAULT_TAG) {

      if ( string != s ) string = s;

      if ( tag != t && strcmp(tag, t) != 0 ) {

	 if ( tag != XmFONTLIST_DEFAULT_TAG ) delete tag;

	 if ( t == XmFONTLIST_DEFAULT_TAG )
	    tag = XmFONTLIST_DEFAULT_TAG;
	 else {
	    tag = new char[strlen(t)+1];
	    strcpy(tag, t);
	 }
      }
   }
};

//
// Method to allow printing of FieldC
//
inline ostream& operator<<(ostream& strm, const FieldC&) { return(strm); }

#endif // _FieldC_h_
