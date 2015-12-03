/*
 * $Id: IconDataC.h,v 1.1.1.1 2000/04/25 13:49:01 fnevgeny Exp $
 *
 * Copyright (c) 1992 HaL Computer Systems, Inc.  All rights reserved.
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

#ifndef _IconDataC_h_
#define _IconDataC_h_

#include "RectC.h"
#include "LabelDataListC.h"

#include <X11/Intrinsic.h>

class VItemC;
class PixmapC;
class IconViewC;

/*-----------------------------------------------------------------------
 *  View specific data for each view item
 */

class IconDataC {

public:

   VItemC		*item;
   IconViewC		*view;
   RectC		bounds;
   int			row;
   int			col;
   PixmapC		*pixmap;
   int			labelWd;
   int			labelHt;
   LabelDataListC	labelList;

   inline void	operator=(const IconDataC& d) {
      item      = d.item;
      view      = d.view;
      row       = d.row;
      col       = d.col;
      bounds    = d.bounds;
      pixmap    = d.pixmap;
      labelWd   = d.labelWd;
      labelHt   = d.labelHt;
      labelList = d.labelList;
   }
   inline int  	operator==(const IconDataC& d) const {return(item == d.item);}
   inline int  	operator!=(const IconDataC& d) const { return !(*this==d); }

   IconDataC() {
      item    = NULL;
      view    = NULL;
      row     = 0;
      col     = 0;
      pixmap  = NULL;
      labelWd =
      labelHt = 0;
      labelList.AllowDuplicates(TRUE);
   }
   IconDataC(const VItemC& i, IconViewC *iv=NULL) {
      item    = (VItemC *)&i;
      view    = iv;
      row     = 0;
      col     = 0;
      pixmap  = NULL;
      labelWd =
      labelHt = 0;
      labelList.AllowDuplicates(TRUE);
   }
   IconDataC(const IconDataC& d) { *this = d; }

   void		GetLabelSize(XmFontList,   int);
   void		GetLabelSize(XFontStruct*, int);
};

//
// Method to allow printing of IconDataC
//
inline ostream& operator<<(ostream& strm, const IconDataC&) { return(strm); }

#endif // _IconDataC_h_
