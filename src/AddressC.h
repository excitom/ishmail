/*
 *  $Id: AddressC.h,v 1.1.1.1 2000/04/25 13:49:02 fnevgeny Exp $
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
#ifndef _AddressC_h_
#define _AddressC_h_

#include <hgl/CharC.h>

class HeaderValC;
class RegexC;

class AddressC {

   CharC	FirstEntry(CharC, u_int*);
   void		Parse(CharC);

public:

   char		*full;
   HeaderValC	*name;
   CharC	 addr;
   CharC	 mailbox;
   CharC	 site;
   AddressC	*next;

   static RegexC	*anglePat;
   static RegexC	*parenPat;
   static RegexC	*quotePat;

   AddressC(CharC);
   ~AddressC();

   void			Print(ostream& strm) const;
};

//
// Method to allow printing of AddressC
//

inline ostream& operator<<(ostream& strm, AddressC& a)
{
   a.Print(strm);
   return(strm);
}

#endif // _AddressC_h_

