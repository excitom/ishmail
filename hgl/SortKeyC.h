/*
 * $Id: SortKeyC.h,v 1.2 2000/06/05 17:00:41 evgeny Exp $
 *
 * Copyright (c) 1993 HAL Computer Systems International, Ltd.
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

#ifndef _SortKeyC_h_
#define _SortKeyC_h_

#include <stream.h>

class SortKeyC {

public:

   enum SortDirT {
      ASCENDING,
      DESCENDING
   };

protected:

   SortDirT	dir;

public:

   inline SortKeyC(SortDirT d=ASCENDING) { dir = d; }
   inline SortKeyC(const SortKeyC& k) { dir = k.dir; }

   inline  int	operator==(const SortKeyC& s) const { return this == &s; }
   inline  int	operator!=(const SortKeyC& s) const { return this != &s; }
   inline int	compare(const SortKeyC&) const { return 0; }
   inline int	operator<(const SortKeyC& s) const { return (compare(s) < 0); }
   inline int	operator>(const SortKeyC& s) const { return (compare(s) > 0); }

   inline SortDirT		Dir(void)	{ return dir; }
   inline const SortDirT	Dir(void) const	{ return dir; }
};

//
// Method to allow printing of SortKeyC
//

inline ostream&
operator<<(ostream& strm, const SortKeyC&)
{
   return(strm);
}

#endif // _SortKeyC_h_
