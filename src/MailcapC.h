/*
 *  $Id: MailcapC.h,v 1.1.1.1 2000/04/25 13:49:02 fnevgeny Exp $
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
#ifndef _MailcapC_h_
#define _MailcapC_h_

#include <hgl/StringC.h>
#include <X11/Intrinsic.h>

/* 
   This structure defines a 3-way tree for finding commands associated
   with the type and subtype of a body part.
*/

class MailcapC {

public:

   StringC	fullType;
   StringC	conType;
   StringC	subType;
   StringC	present;	// command to "present (display, play, etc)
			   	//    the body part
   StringC	compose;	// compose body part of this type
   StringC	composetyped;	// compose body part of this type and include
			   	//    Content-Type: and
				//    Content-Transfer-Encoding: with it
   StringC	print;		// command to send body part to printer
   StringC	edit;		// command to edit body part with
   StringC	test;		// shell command for additional test to
			   	//    make sure this is the entry to use
			   	//    for the specified body part
   StringC	bitmap;		// X11-bitmap file to be used to id this part
   StringC	desc;		// description

   Boolean	needsterminal;
   Boolean	copiousoutput;
   Boolean	isText;		// important for base64 encoding of newlines

//
// Methods
//
   MailcapC(CharC);
   ~MailcapC();

   inline int	operator==(const MailcapC& m) const
   	{ return fullType == m.fullType; }
   inline int	operator!=(const MailcapC& m) const { return !(*this==m); }
   inline int	compare(const MailcapC& m) const
   	{ return fullType.compare(m.fullType); }
   inline int	operator<(const MailcapC& m) const { return (compare(m) < 0); }
   inline int	operator>(const MailcapC& m) const { return (compare(m) > 0); }
};

//
// Method to allow <<
//
inline ostream& operator<<(ostream& strm, const MailcapC&) { return(strm); }

#endif // _MailcapC_h_
