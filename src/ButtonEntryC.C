/*
 * $Id: ButtonEntryC.C,v 1.2 2000/05/07 12:26:11 fnevgeny Exp $
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

#include <config.h>

#include "ButtonEntryC.h"

//-----------------------------------------------------------------------
// Constructors
//

ButtonEntryC::ButtonEntryC()
{
   name     = "";
   label    = "";
   abbrev   = "";
   posIndex = 1;
}

ButtonEntryC::ButtonEntryC(const ButtonEntryC& be)
{
   if ( &be != this )
      *this = be;
}

ButtonEntryC::ButtonEntryC(const char *nam, const char *lab, const char *ab)
{
   name     = nam;
   label    = lab;
   abbrev   = ab;
   posIndex = 1;
}

//-----------------------------------------------------------------------
// assignment operator
//

ButtonEntryC&
ButtonEntryC::operator=(const ButtonEntryC& b)
{
   name     = b.name;
   label    = b.label;
   abbrev   = b.abbrev;
   posIndex = b.posIndex;

   return *this;
}

//-----------------------------------------------------------------------
// compare method
//

int
ButtonEntryC::compare(const ButtonEntryC& b) const
{
   if ( posIndex >  b.posIndex ) return  1;
   if ( posIndex <  b.posIndex ) return -1;
   return 0;
}
