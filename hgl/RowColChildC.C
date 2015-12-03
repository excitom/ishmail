/*
 *  $Id: RowColChildC.C,v 1.2 2000/05/07 12:26:11 fnevgeny Exp $
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
#include "RowColChildC.h"

#include <X11/IntrinsicP.h>
#include <X11/CoreP.h>

extern int	debug1, debug2;

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//----------------------------------------------------------------------------
// RowColChildC constructor
//

RowColChildC::RowColChildC(Widget widget)
{
   w = widget;

   prefWd = 0;
   prefHt = 0;
   curX   = 0;
   curY   = 0;
   curWd  = 0;
   curHt  = 0;

   row = NULL;
   col = NULL;
}

//----------------------------------------------------------------------------
// Method to query for preferred size
//

void
RowColChildC::GetPrefSize()
{
   if ( !w ) return;

   XtWidgetGeometry	geo;
   XtQueryGeometry(w, NULL, &geo);

   prefWd = geo.width;
   prefHt = geo.height;

   if ( debug2 ) {
      cout <<"   pref size for " <<w->core.name <<"\tis "
           <<dec(prefWd,4) <<" " << dec(prefHt,4) <<endl;
      if ( w->core.width != prefWd || w->core.height != prefHt )
	 cout <<"   *** core size is " <<w->core.width
	      <<" " << w->core.height <<endl;
   }
}

//----------------------------------------------------------------------------
// Method to return preferred width
//

int
RowColChildC::PrefWidth()
{
   if ( !w ) return 0;

   if ( prefWd == 0 ) GetPrefSize();

   return prefWd;
}

//----------------------------------------------------------------------------
// Method to return preferred height
//

int
RowColChildC::PrefHeight()
{
   if ( !w ) return 0;

   if ( prefHt == 0 ) GetPrefSize();

   return prefHt;
}

//----------------------------------------------------------------------------
// Method to compare 2 children
//

int
RowColChildC::compare(const RowColChildC& child) const
{
   return (w - child.w);
}

