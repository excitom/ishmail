/*
 *  $Id: HeadPrefC.h,v 1.1.1.1 2000/04/25 13:49:02 fnevgeny Exp $
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
#ifndef _HeadPrefC_h
#define _HeadPrefC_h

#include "PrefC.h"

#include <hgl/StringListC.h>

class CharC;

class HeadPrefWinC;

class HeadPrefC : public PrefC {

   HeadPrefWinC		*prefWin;

public:

//
// Public data
//
   StringListC		showHeadList;	// Display these headers
   StringListC		hideHeadList;	// Ignore these headers
   Boolean		useShowHeadList;	// Which has priority?

//
// Public methods
//
    HeadPrefC();
   ~HeadPrefC();

   void		Edit(Widget);
   Boolean	HeaderShown(CharC);
   Boolean	WriteDatabase();
   Boolean	WriteFile();
};

#endif // _HeadPrefC_h
