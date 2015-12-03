/*
 * $Id: HelpDbC.h,v 1.1.1.1 2000/04/25 13:49:01 fnevgeny Exp $
 *
 * Copyright (c) 1994 HAL Computer Systems International, Ltd.
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

#ifndef _HelpDbC_h_
#define _HelpDbC_h_

#include "CharC.h"
#include "StringC.h"
#include "PtrListC.h"

#include <X11/Intrinsic.h>

class HelpDbC;
class MappedFileC;

class HelpCardC {

public:

   HelpDbC	*db;
   StringC	name;
   StringC	title;
   StringC	locator;
   long		offset;
   long		length;

   Boolean	GetText(StringC&) const;
   Boolean	IsGlossary() const { return name.EndsWith("glo"); }
};

class HelpDbC {

   void		DeleteCards();
   void		ReadNewDb();
   void		ReadOldDb();

   static int	CompareTitles(const void*, const void*);

public:

//
// Data
//
   Boolean	loaded;
   StringC	file;
   MappedFileC	*dbMap;
   PtrListC	cardList;

//
// Constructor and destructor
//
   HelpDbC(const char*);
   ~HelpDbC();

//
// Methods
//
   HelpCardC	*FindCard(const CharC) const;
};

#endif // _HelpDbC_h_
