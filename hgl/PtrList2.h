/*
 *  $Id: PtrList2.h,v 1.1.1.1 2000/04/25 13:49:01 fnevgeny Exp $
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
// PtrList2.h
//

#ifndef _PtrList2_h_
#define _PtrList2_h_

#include <stream.h>

/*
 * List of generic pointers
 */

class PtrList2 {

protected:

   void			**_list;
   unsigned		_count;
   unsigned		_space;
   char         	allow_duplicates;
   char         	adjustGrowth;
   char         	autoShrink;
   void			grow(unsigned size=0);
   void			shrink();

public:

   unsigned		GROWTH_AMOUNT;
   const int		NULL_INDEX;
   const unsigned	MAX_GROWTH;

   PtrList2();
   PtrList2(unsigned ga);
   PtrList2(const PtrList2&);
   ~PtrList2();

/* Append another list to this one */
   PtrList2& operator+=(const PtrList2&);

/* Copy another list to this one */
   PtrList2& operator=(const PtrList2&);

/* Return the position of the specified object.  Object must be in list */

   int		indexOf(const void*) const;

/* Return non-zero if the specified object is in the list */

   int		includes(const void*) const;

/* Add the specified object to the list in the requested position */
/*    The position is ignored if this list is sorted */

   void		*insert (const void*, unsigned);
   void		*replace(const void*, unsigned);
   void		*append (const void*);
   void		*prepend(const void*);
   void		*add    (const void*);

/* Remove the specified object or index from the list */

   void		remove(unsigned);
   void		remove(const void*);
   void		removeNulls();
   void		removeAll();
   void		removeLast();

/* Return a pointer to the previous object in the list */

   void		*before(const void*) const;

   inline unsigned	size()		const { return _count; }
   inline unsigned	capacity()	const { return _space; }
   inline void		**start()	const { return _list; }
   void			AllowDuplicates(char);
   void			AutoShrink(char);
   void			SetCapacity(unsigned);

   void			*operator[](unsigned) const;
   void			*First() const;
   void			*Last() const;

   ostream&		printOn(ostream& strm=cout) const;
};

/*-------------------------------------------------------------*/
/* PRINTING */

inline ostream&
operator<<(ostream& strm, const PtrList2& c)
{
   return c.printOn(strm);
}

#endif // _PtrList2_h_
