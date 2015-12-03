/*
 *  $Id: SavePrefC.h,v 1.1.1.1 2000/04/25 13:49:02 fnevgeny Exp $
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
#ifndef _SavePrefC_h
#define _SavePrefC_h

#include "PrefC.h"
#include "RuleDictC.h"

class SavePrefWinC;

class SavePrefC : public PrefC {

   SavePrefWinC		*prefWin;

public:

//
// Public data
//
   RuleDictC		saveRules;	// pattern -> command

//
// Public methods
//
    SavePrefC();
   ~SavePrefC();

   void		Edit(Widget);
   Boolean	WriteDatabase();
   Boolean	WriteFile();
};

#endif // _SavePrefC_h
