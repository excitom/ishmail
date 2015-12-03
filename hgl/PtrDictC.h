/*
 *  $Id: PtrDictC.h,v 1.1.1.1 2000/04/25 13:49:01 fnevgeny Exp $
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
//
// PtrDictC.h
//

#ifndef _PtrDictC_h_
#define _PtrDictC_h_

#include "PtrList2.h"

/*------------------------------------------------------------------------
 * A dictionary entry containing two void pointers
 */

class PtrDictCEntry {

public:

   void	*key;
   void	*val;

   inline PtrDictCEntry () { key = NULL; val = NULL; }
   inline PtrDictCEntry (void* kp, void* vp) { key = kp; val = vp; }
   inline PtrDictCEntry (const PtrDictCEntry& e) { *this = e; }

   inline PtrDictCEntry&	operator= (const PtrDictCEntry& e) {
      if ( this != &e ) {
	 key = e.key;
	 val = e.val;
      }
      return (*this);
   }

   inline int	operator== (const PtrDictCEntry& e) const {
      return (key == e.key && val == e.val);
   }
   inline int	operator!= (const PtrDictCEntry& e) const {
      return (!(*this==e));
   }
   inline int	compare  (const PtrDictCEntry&) const { return 0; }
   inline int	operator<(const PtrDictCEntry&) const { return 0; }
   inline int	operator>(const PtrDictCEntry&) const { return 0; }

   inline ostream&  printOn(ostream& strm=cout) const {
      strm <<key <<" -> " <<val;
      return strm;
   }
};

inline ostream& operator<<(ostream& strm, const PtrDictCEntry& e)
{
   return e.printOn(strm);
}

/*------------------------------------------------------------------------
 * A dictionary containing void pointer keys and values
 */

class PtrDictC : public PtrList2 {

public:

   inline PtrDictC() {}
   virtual inline ~PtrDictC() {}

/* Build this dictionary from another */
   inline PtrDictC(const PtrDictC& d) { *this = d; }

/* Copy one dictionary to another */

   PtrDictC&	operator=(const PtrDictC& d);

/* Lookup up entries */

   PtrDictCEntry	*operator[](unsigned) const;

   int			indexOf (void* kp) const;
   int			indexOf (PtrDictCEntry* entry) const;
   int			includes(void* kp) const;
   int			includes(PtrDictCEntry* entry) const;

   void			*definitionOf(void* kp) const;
   void			*valOf(int index) const;
   void			*keyOf(int index) const;
   PtrDictCEntry	*entryOf(void* kp) const;

/* Changing entries */

   PtrDictCEntry		*add   (void* kp, void* vp);
   PtrDictCEntry		*modify(void* kp, void* vp);
   PtrDictCEntry		*modify(unsigned index, void* vp);

/* Deleting entries */

   void			remove(void* kp);
   void			remove(unsigned index);
   void			remove(PtrDictCEntry* entry);

/* Printing */

   void			printOn(ostream& strm=cout) const;
};

inline ostream& operator<<(ostream& strm, const PtrDictC& d)
{
   d.printOn(strm);
   return(strm);
}

#endif // _PtrDictC_h_
