/*
 * $Id: PrefC.h,v 1.1.1.1 2000/04/25 13:49:02 fnevgeny Exp $
 *
 * Copyright (c) 1993 HAL Computer Systems International, Ltd.
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

#ifndef _PrefC_h_
#define _PrefC_h_

#include <hgl/StringC.h>

class StringListC;
class RuleDictC;

class PrefC {

   StringC	resStr;
   StringC	valStr;

   void		AddEscapes(StringC&);
   StringC	*Find(StringListC&, const char*);
   void		Store(const char*);
   void		Update(StringListC&, const char*);

protected:

   void		ExtractRules(StringC, RuleDictC&);
   void		ReadResFile(StringListC&);
   void		Store(const char*, const char*);
   void		Store(const char*, Boolean);
   void		Store(const char*, int);
   void		Store(const char*, float);
   void		Store(const char*, StringListC&);
   void		Store(const char*, RuleDictC&);
   void		StoreGravity(const char*, int);
   void		Update(StringListC&, const char*, const char*);
   void		Update(StringListC&, const char*, Boolean);
   void		Update(StringListC&, const char*, int);
   void		Update(StringListC&, const char*, float);
   void		Update(StringListC&, const char*, StringListC&);
   void		Update(StringListC&, const char*, RuleDictC&);
   void		UpdateGravity(StringListC&, const char*, int);
   Boolean	WriteResFile(StringListC&);

public:

   PrefC();
   virtual ~PrefC();

   virtual Boolean	WriteDatabase() = 0;
   virtual Boolean	WriteFile()     = 0;
};

#endif // _PrefC_h_
