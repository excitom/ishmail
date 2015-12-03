/*
 *  $Id: PtrList2.C,v 1.3 2000/05/07 12:26:11 fnevgeny Exp $
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

#include <config.h>
//
// PtrList2.C
//

#include "PtrList2.h"

#include <stdlib.h>

/*
 * List of generic pointers
 */

#ifndef FALSE
#define FALSE	0
#define TRUE	(!FALSE)
#endif

PtrList2::PtrList2()
: GROWTH_AMOUNT(4), NULL_INDEX(-1), MAX_GROWTH(32768)
{
   _list		= NULL;
   _count		= 0;
   _space		= 0;
   allow_duplicates	= FALSE;
   adjustGrowth		= TRUE;
   autoShrink		= TRUE;
}

PtrList2::PtrList2(unsigned ga)
: GROWTH_AMOUNT(ga), NULL_INDEX(-1), MAX_GROWTH(ga)
{
   _list		= NULL;
   _count		= 0;
   _space		= 0;
   allow_duplicates	= FALSE;
   adjustGrowth		= FALSE;
   autoShrink		= TRUE;
}

PtrList2::PtrList2(const PtrList2& l)
: GROWTH_AMOUNT(4), NULL_INDEX(-1), MAX_GROWTH(32768)
{
   _list		= NULL;
   _count		= 0;
   _space		= 0;
   allow_duplicates	= FALSE;
   adjustGrowth		= TRUE;
   autoShrink		= TRUE;
   *this		= l;
}

PtrList2::~PtrList2()
{
   if (_list) delete [] _list;
}

/* Append another list to this one */
PtrList2&
PtrList2::operator+=(const PtrList2& l)
{
   if ( this != &l ) {
      register int count = l.size();
      for (register int i=0; i<count; i++) append(l[i]);
   }
   return *this;
}

/* Copy another list to this one */
PtrList2&
PtrList2::operator=(const PtrList2& l)
{
   if ( this != &l ) {
      removeAll();
      *this += l;
   }
   return *this;
}

/* Return non-zero if the specified object is in the list */

int
PtrList2::includes(const void* op) const
{
   return (indexOf(op) != NULL_INDEX);
}

/* Add the specified object to the list in the requested position */
/*    The position is ignored if this list is sorted */

void*
PtrList2::append(const void* op)
{
   return insert(op, _count);
}

void*
PtrList2::prepend(const void* op)
{
   return insert(op, 0); 
}

void*
PtrList2::add(const void* op)
{
   return append(op); 
}

/* Remove the specified object or index from the list */

void
PtrList2::removeAll()
{
   _count = 0;
   if (autoShrink) shrink();
}

void
PtrList2::removeLast()
{
   if ( _count > 0 ) remove(_count-1);
}

void
PtrList2::AllowDuplicates(char val)
{
   allow_duplicates = val;
}

void
PtrList2::AutoShrink(char val)
{
   autoShrink = val;
   if ( autoShrink ) shrink();
}

void
PtrList2::SetCapacity(unsigned count)
{
   grow(count*sizeof(void*));
}

void*
PtrList2::operator[](unsigned i) const
{
   void	*tp = NULL;
   if (i < _count)
      tp = _list[i];
   else {
      cerr << "PtrList2[" << i << "]: reference out of bounds.";
      if ( _count > 0 ) cerr << " Valid range is 0:" << _count-1 <<".";
      else		cerr << " List is empty.";
      cerr <<endl;
      abort();
   }
   return tp;
}

void*
PtrList2::First() const
{
   void	*tp = NULL;
   if ( _count > 0 ) tp = _list[0];
   return tp;
}

void*
PtrList2::Last() const
{
   void	*tp = NULL;
   if ( _count > 0 ) tp = _list[_count-1];
   return tp;
}

ostream&
PtrList2::printOn(ostream& strm) const
{
   for (int i=0; i<_count; i++)
      strm <<dec(i,5) <<":" <<_list[i] <<endl;
   return strm;
}

/*-------------------------------------------------------------*/

void
PtrList2::grow(unsigned newSize)
{
/* Increase space */

   if ( newSize == 0 ) {
      _space += GROWTH_AMOUNT;
      if ( adjustGrowth && GROWTH_AMOUNT < MAX_GROWTH ) {
	 GROWTH_AMOUNT *= 2;
	 if ( GROWTH_AMOUNT > MAX_GROWTH ) GROWTH_AMOUNT = MAX_GROWTH;
      }
   }
   else if ( newSize == _space ) {
      return;
   }
   else if ( newSize < _space ) {
      if ( !autoShrink ) return;
      _space = newSize;
      if ( newSize < _count ) _count = newSize; /* Only copy this many */
   }
   else {
      _space = newSize;
   }

/* Allocate memory for new list */
   void	**old_list = _list;
   _list = new void*[_space];
   if ( !_list ) {
      cerr << "Could not allocate memory for PListC of size: " << _space <<endl;
      exit(1);
   }

/* Copy list to new memory */
   for (register int i=0; i<_count; i++) {
      _list[i] = old_list[i]; /* Just copying pointers here */
   }

/* Free up old memory */
   if ( old_list ) delete [] old_list;
}

/*-------------------------------------------------------------*/

void
PtrList2::shrink()
{
   if ( !autoShrink ) return;

/* Decrease space if necessary */

   int  needed = _count * sizeof(void*);
   if ( needed > (_space>>1) ) return;

/* Start from scratch and get just as much space as necessary */

   if ( adjustGrowth ) GROWTH_AMOUNT = 4;
   _space = GROWTH_AMOUNT;
   if ( adjustGrowth && GROWTH_AMOUNT < MAX_GROWTH ) {
      GROWTH_AMOUNT *= 2;
      if ( GROWTH_AMOUNT > MAX_GROWTH ) GROWTH_AMOUNT = MAX_GROWTH;
   }

   while ( _space < needed ) {
      _space += GROWTH_AMOUNT;
      if ( adjustGrowth && GROWTH_AMOUNT < MAX_GROWTH ) {
	 GROWTH_AMOUNT *= 2;
	 if ( GROWTH_AMOUNT > MAX_GROWTH ) GROWTH_AMOUNT = MAX_GROWTH;
      }
   }

/* Allocate memory for new list */
   void	**old_list = _list;
   _list = new void*[_space];
   if ( !_list ) {
      cerr << "Could not allocate memory for PListC of size: " << _space <<endl;
      exit(1);
   }

/* Copy list to new memory */
   for (register int i=0; i<_count; i++) {
      _list[i] = old_list[i]; /* Just copying pointers here */
   }

/* Free up old memory */
   if ( old_list ) delete [] old_list;
}

/*-------------------------------------------------------------*/

int
PtrList2::indexOf(const void* op) const
{
/* Loop through the list until the entry is found */
/* Compare pointers to objects */

   register int	i;
   for (i=0; (i<_count && _list[i] != op); i++);
   if (i == _count) return NULL_INDEX;
   return i;
}

/*-------------------------------------------------------------*/

void*
PtrList2::insert(const void* op, unsigned i)
{
/* See if index is valid */
   if (i > _count) return NULL;

/* Don't allow same address in list twice */
   if ( !allow_duplicates ) {
      int	index = indexOf(op);
      if ( index != NULL_INDEX ) return _list[index];
   }

/* See if there is enough space for another entry */
   if ( _count >= _space ) grow();

/* Insert entry in position i */
/* Make a space for the new entry by moving all others back */

   register int	j;
   for (j=_count; j>i; j--) {
      _list[j] = _list[j-1]; /* Just copying pointers here */
   }

   _count++;
   _list[i] = (void *)op;

   return _list[i];
}

/*-------------------------------------------------------------*/

void*
PtrList2::replace(const void* op, unsigned i)
{
/* See if index is valid */
   if (i > _count) return NULL;

/* Don't allow same address in list twice */
   if ( op != NULL && !allow_duplicates ) {
      int	index = indexOf(op);
      if ( index != NULL_INDEX ) return _list[index];
   }

/* Insert entry in position i */
   _list[i] = (void *)op;

   return _list[i];
}

/*-------------------------------------------------------------*/

void
PtrList2::remove(unsigned index)
{
   if (index < _count) {

      /* Move entries forward in list and overwrite this one */
      for (register int i=index+1; i<_count; i++) {
	 _list[i-1] = _list[i]; /* Just copying pointers here */
      }
      _count--;
      if ( autoShrink ) shrink();
   }
}

void
PtrList2::remove(const void* op)
{
/* Look up this entry */

   register int i = indexOf(op);
   if ( i != NULL_INDEX ) remove(i);
}

/*-------------------------------------------------------------*/

void
PtrList2::removeNulls()
{
/* Collapse list */
   register int	j = 0;
   for (register int i=0; i<_count; i++) {
      if ( _list[i] != NULL ) {
	 if ( j != i ) _list[j] = _list[i]; /* Just copying pointers here */
	 j++;
      }
   }
   if ( j != _count ) {
      _count = j;
      if ( autoShrink ) shrink();
   }
}

/*-------------------------------------------------------------*/
/* Return a pointer to the list entry just before this one */

void*
PtrList2::before(const void* op) const
{
/* Look up this entry */

   register int i = indexOf(op);
   if ( i == NULL_INDEX || i == 0 ) return NULL;
   return _list[i-1];
}
