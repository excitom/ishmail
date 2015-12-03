/*
 * $Id: ButtonEntryC.h,v 1.1.1.1 2000/04/25 13:49:02 fnevgeny Exp $
 *
 * Copyright (c) 1994 HAL Computer Systems International, Ltd.
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

#ifndef _ButtonEntryC_h_
#define _ButtonEntryC_h_

#include <hgl/StringC.h>

#include <X11/Intrinsic.h>

class ButtonEntryC {

public:

   StringC	name;
   StringC	label;
   StringC	abbrev;
   short	posIndex;

//
// Constructors
//
   ButtonEntryC();
   ButtonEntryC(const char*, const char*, const char*);
   ButtonEntryC(const ButtonEntryC&);

//
// Operators
//
   ButtonEntryC& operator=(const ButtonEntryC&);
   int	compare(const ButtonEntryC&) const;
   inline int operator==(const ButtonEntryC& b) const { return(compare(b)==0); }
   inline int operator!=(const ButtonEntryC& b) const { return(compare(b)!=0); }
   inline int operator< (const ButtonEntryC& b) const { return(compare(b)< 0); }
   inline int operator> (const ButtonEntryC& b) const { return(compare(b)> 0); }
   inline int operator<=(const ButtonEntryC& b) const { return(compare(b)<=0); }
   inline int operator>=(const ButtonEntryC& b) const { return(compare(b)>=0); }
};

//
// Method to allow <<
//
inline ostream& operator<<(ostream& strm, const ButtonEntryC&) { return(strm); }

#endif // _ButtonEntryC_h_
