/*
 * $Id: IconDataC.C,v 1.2 2000/05/07 12:26:10 fnevgeny Exp $
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

#include <config.h>

#include "IconDataC.h"
#include "VItemC.h"

void
IconDataC::GetLabelSize(XFontStruct *font, int spacing)
{
//
// Clear list of components
//
   labelList.removeAll();
   labelWd =
   labelHt = 0;

//
// Loop to end of data
//
   StringC&	label = item->Label();
   char	*cs = (char *)label;
   unsigned	start = 0;
   if (cs != NULL)
   while ( *cs != 0 ) {

//
// Loop to end of component
//
      unsigned	len = 0;
      while ( *cs != '\n' && *cs != 0 ) {
	 cs++;
	 len++;
      }

//
// Create label data object
//
      LabelDataC	ldata(font, label, start, len);

//
// Add sizes to total
//
      labelHt += ldata.height;
      if ( ldata.width > labelWd ) labelWd = ldata.width;

//
// Add component to dictionary
//
      labelList.append(ldata);

//
// Look for another component
//
      if ( *cs == '\n' ) {
	 cs++;
	 start += len + 1;
      }

   } // End while more data

//
// Add spacing betewen components
//
   labelHt += (labelList.size()-1) * spacing;

} // End IconDataC GetLabelSize

void
IconDataC::GetLabelSize(XmFontList fontList, int spacing)
{
//
// Clear list of components
//
   labelList.removeAll();
   labelWd =
   labelHt = 0;

//
// Loop to end of data
//
   StringC&	label = item->Label();
   char		*tag  = item->LabelTag() ? item->LabelTag()
   					 : XmFONTLIST_DEFAULT_TAG;
   char		*cs   = (char *)label;
   unsigned	start = 0;

   while ( cs && *cs != 0 ) {

//
// Loop to end of component
//
      unsigned	len = 0;
      while ( *cs != '\n' && *cs != 0 ) {
	 cs++;
	 len++;
      }

//
// Create label data object
//
      LabelDataC	ldata(fontList, label, start, len, tag);

//
// Add sizes to total
//
      labelHt += ldata.height;
      if ( ldata.width > labelWd ) labelWd = ldata.width;

//
// Add component to dictionary
//
      labelList.append(ldata);

//
// Look for another component
//
      if ( *cs == '\n' ) {
	 cs++;
	 start += len + 1;
      }

   } // End while more data

//
// Add spacing betewen components
//
   labelHt += (labelList.size()-1) * spacing;

} // End IconDataC GetLabelSize
