/*
 *  $Id: ColumnC.h,v 1.1.1.1 2000/04/25 13:49:01 fnevgeny Exp $
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
#ifndef _ColumnC_h_
#define _ColumnC_h_

#include "FieldC.h"

#include <X11/Intrinsic.h>

/*-----------------------------------------------------------------------
 *  Column justifications
 */

typedef enum {

   JUSTIFY_LEFT,
   JUSTIFY_RIGHT,
   JUSTIFY_CENTER

} ColumnJustifyT;

/*-----------------------------------------------------------------------
 *  View specific data for each column
 */

class ColumnC {

public:

   FieldC		title;
   int			minWd;
   int			maxWd;
   int			curWd;
   Boolean		visible;
   ColumnJustifyT	justify;

   inline void	operator=(const ColumnC& c) {
      title   = c.title;
      minWd   = c.minWd;
      maxWd   = c.maxWd;
      curWd   = c.curWd;
      visible = c.visible;
      justify = c.justify;
   }
   inline int  	operator==(const ColumnC& c) const {return(title == c.title);}
   inline int  	operator!=(const ColumnC& c) const { return !(*this==c); }
   inline int	compare(const ColumnC& c) const
      { return title.compare(c.title); }
   inline int	operator<(const ColumnC& c) const { return (compare(c) < 0); }
   inline int	operator>(const ColumnC& c) const { return (compare(c) > 0); }

   inline ColumnC() {
      minWd   = 0;
      maxWd   = 0;
      curWd   = 0;
      visible = True;
      justify = JUSTIFY_LEFT;
   }
   inline ColumnC(const ColumnC& c) { *this = c; }

   inline int	Width() {
      int	wd = curWd;
      if ( maxWd > 0 && wd > maxWd ) wd = maxWd;
      if ( minWd > 0 && wd < minWd ) wd = minWd;
      return wd;
   }
};

//
// Method to allow printing of ColumnC
//
inline ostream& operator<<(ostream& strm, const ColumnC&) { return(strm); }

#endif // _ColumnC_h_
