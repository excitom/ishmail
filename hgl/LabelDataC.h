/*
 *  $Id: LabelDataC.h,v 1.1.1.1 2000/04/25 13:49:01 fnevgeny Exp $
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
#ifndef _LabelDataC_h_
#define _LabelDataC_h_

#include "StringC.h"

#include <X11/Intrinsic.h>
#include <Xm/Xm.h>

/*-----------------------------------------------------------------------
 *  Data for each component of a label
 */

class LabelDataC {

public:

   StringC	string;
   char		*tag;
   int		width;
   int		height;

   inline void	operator=(LabelDataC& l) {
      string = l.string;
      tag    = l.tag;
      width  = l.width;
      height = l.height;
   }
   inline int  	operator==(const LabelDataC& l) const
      { return( string == l.string); }
   inline int  	operator!=(const LabelDataC& l) const { return !(*this==l); }
   inline int	compare(const LabelDataC& l) const
	{ return string.compare(l.string); }
   inline int	operator<(const LabelDataC& l) const
	{ return (compare(l) < 0); }
   inline int	operator>(const LabelDataC& l) const
	{ return (compare(l) > 0); }

   LabelDataC() { tag = XmFONTLIST_DEFAULT_TAG; }
   LabelDataC(LabelDataC& l) {
      string = l.string;
      tag    = XmFONTLIST_DEFAULT_TAG;
      width  = l.width;
      height = l.height;
   }

   LabelDataC(XFontStruct* font, const StringC& label, unsigned start,
	      unsigned len) {

      string = label(start, len);

      int		dir, asc, dsc;
      XCharStruct	size;
      XTextExtents(font, string, string.size(), &dir, &asc, &dsc, &size);

      width  = size.width;
      height = font->ascent + font->descent;
   }

   LabelDataC(XmFontList fontList, const StringC& label, unsigned start,
	      unsigned len, char *t=XmFONTLIST_DEFAULT_TAG) {

      string = label(start, len);
      tag    = t;

      Dimension	w, h;
      XmString	xmstr = XmStringCreateLtoR(string, tag);
      XmStringExtent(fontList, xmstr, &w, &h); 

      width  = w;
      height = h;
   }
};

//
// Method to allow printing of LabelDataC
//
inline ostream& operator<<(ostream& strm, const LabelDataC&) { return(strm); }

#endif // _LabelDataC_h_
