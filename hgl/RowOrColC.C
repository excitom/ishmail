/*
 *  $Id: RowOrColC.C,v 1.2 2000/05/07 12:26:11 fnevgeny Exp $
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
#include "RowOrColC.h"
#include "RowColC.h"

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//----------------------------------------------------------------------------
// RowOrColC constructor
//

RowOrColC::RowOrColC(RowColC *parent)
{
   par = parent;

   childList.AllowDuplicates(FALSE);
   pos   = 0;
   prefSize = 0;
   realSize = 0;
   visible = True;

} // End RowOrColC constructor

//----------------------------------------------------------------------------
// Method to reset variables
//

void
RowOrColC::Reset()
{
   pos      = 0;
   prefSize = 0;
   realSize = 0;
   childList.removeAll();
}

//----------------------------------------------------------------------------
// Method to compare 2 rows or columns
//

int
RowOrColC::compare(const RowOrColC& roc) const
{
   return (pos - roc.pos);
}

//----------------------------------------------------------------------------
// Method to add a new child to this row or column
//

void
RowOrColC::AddChild(RowColChildC *child)
{
   childList.add(child);

   if ( !par->deferred ) {
      CalcPrefSize();
      SizeChildren();
      PlaceChildren();
   }
   else
      par->changed = True;

} // End RowOrColC AddChild

//----------------------------------------------------------------------------
// Method to set height of row or width of column
//

void
RowOrColC::SetSize(int sz)
{
   if ( realSize == sz ) return;

   realSize = sz;

   if ( !par->deferred ) {
      SizeChildren();
      PlaceChildren();
   }
   else
      par->changed = True;
}

//----------------------------------------------------------------------------
// Method to set position of row or column
//

void
RowOrColC::SetPosition(int p)
{
   if ( pos == p ) return;

   pos = p;

   if ( !par->deferred )
      PlaceChildren();
   else
      par->changed = True;
}

//----------------------------------------------------------------------------
// Method to set the row or column alignment
//

void
RowOrColC::SetAlignment(RcAlignT a)
{
   if ( alignment == a ) return;

   alignment = a;

//
// Alignment has no effect if adjust is ADJUST_ATTACH
//
   if ( adjust != RcADJUST_ATTACH ) {
      if ( !par->deferred )
	 PlaceChildren();
      else
	 par->changed = True;
   }
}

//----------------------------------------------------------------------------
// Method to set resize flag
//

void
RowOrColC::SetResize(Boolean val)
{
   if ( (resizeOk && val) || (!resizeOk && !val) ) return;

   resizeOk = val;

   par->changed = True;
}

//----------------------------------------------------------------------------
// Method to set visibility flag
//

void
RowOrColC::SetVisible(Boolean val)
{
   if ( (visible && val) || (!visible && !val) ) return;

   visible = val;

   par->changed = True;
}

//----------------------------------------------------------------------------
// Method to set the size adjust mode
//

void
RowOrColC::SetAdjust(RcAdjustT adj)
{
   if ( adjust == adj ) return;

   adjust = adj;

   if ( !par->deferred ) {
      SizeChildren();
      PlaceChildren();
   }
   else
      par->changed = True;
}

//----------------------------------------------------------------------------
// Method to recalculate position and size
//

void
RowOrColC::Refresh()
{
   SizeChildren();
   PlaceChildren();
}

