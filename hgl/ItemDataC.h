/*
 *  $Id: ItemDataC.h,v 1.3 2000/08/13 13:25:32 evgeny Exp $
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
#ifndef _ItemDataC_h_
#define _ItemDataC_h_

#include "RectC.h"
#include "FieldListC.h"

#include <Xm/Xm.h>

/*-----------------------------------------------------------------------
 *  View specific data for each view item
 */

class VItemC;
class FieldViewC;
class PixmapC;

class ItemDataC {

public:

   VItemC	*item;
   FieldViewC	*view;
   RectC	bounds;
   FieldListC	fieldList;
   int		itemHt;
   PixmapC	*pixmap;

   inline void	operator=(const ItemDataC& d) {
      item       = d.item;
      view       = d.view;
      bounds     = d.bounds;
      itemHt     = d.itemHt;
      fieldList  = d.fieldList;
      pixmap     = d.pixmap;
   }
   inline int  	operator==(const ItemDataC& d) const {return(item == d.item);}
   inline int  	operator!=(const ItemDataC& d) const { return !(*this==d); }

   ItemDataC() {
      item     = NULL;
      view     = NULL;
      fieldList.AllowDuplicates(TRUE);
      itemHt   = 0;
      pixmap   = NULL;
   }
   ItemDataC(const VItemC& i, FieldViewC* fv) {
      item     = (VItemC *)&i;
      view     = fv;
      fieldList.AllowDuplicates(TRUE);
      itemHt   = 0;
      pixmap   = NULL;
   }
   ItemDataC(const ItemDataC& d) { *this = d; }

   ~ItemDataC() {
      unsigned	count = fieldList.size();
      for (int i=0; i<count; i++) delete fieldList[i];
   }
};

//
// Method to allow printing of ItemDataC
//
inline ostream& operator<<(ostream& strm, const ItemDataC&) { return(strm); }

#endif // _ItemDataC_h_
