//
// PtrListC.C
//

#include <config.h>

#include "PtrListC.h"

/* 
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

/*
 * OListC.meth -- method definition file for class OListC
 *
 *   This type of list store the members themselves
 *   PListC stores only pointers to the members
 */

#include <stdlib.h>	// For qsort

#ifndef FALSE
#define FALSE	0
#define TRUE	(!FALSE)
#endif

/*------------------------------------------------------------------------
 * PtrListC constructors
 */

PtrListC::PtrListC() : GROWTH_AMOUNT(4), NULL_INDEX(-1), MAX_GROWTH(32768)
{
   _list		= (PtrT*)NULL;
   _count		= 0;
   _space		= 0;
   allow_duplicates	= FALSE;
   sorted		= FALSE;
   adjustGrowth		= TRUE;
   autoShrink		= TRUE;
}

PtrListC::PtrListC(unsigned ga) : GROWTH_AMOUNT(ga), NULL_INDEX(-1), MAX_GROWTH(ga)
{
   _list		= (PtrT*)NULL;
   _count		= 0;
   _space		= 0;
   allow_duplicates	= FALSE;
   sorted		= FALSE;
   adjustGrowth		= FALSE;
   autoShrink		= TRUE;
}

PtrListC::PtrListC(const PtrListC& l):GROWTH_AMOUNT(4), NULL_INDEX(-1), MAX_GROWTH(32768)
{
   _list		= (PtrT*)NULL;
   _count		= 0;
   _space		= 0;
   allow_duplicates	= FALSE;
   sorted		= FALSE;
   adjustGrowth		= TRUE;
   autoShrink		= TRUE;
   *this		= l;
}

/*------------------------------------------------------------------------
 * PtrListC destructor
 */

PtrListC::~PtrListC()
{
   if (_list) delete [] _list;
}

/*------------------------------------------------------------------------
 * Append another list to this one
 */

PtrListC&
PtrListC::operator+=(const PtrListC& l)
{
   if ( this != &l ) {
      register int count = l.size();
      register int i;
      for (i=0; i<count; i++) {
	 PtrT *item = l[i];
	 add(*item);
      }
   }
   return *this;
}

/*------------------------------------------------------------------------
 * Copy another list to this one
 */

PtrListC&
PtrListC::operator=(const PtrListC& l)
{
   if ( this != &l ) {
      removeAll();
      *this += l;
   }
   return *this;
}

/*------------------------------------------------------------------------
 * Return non-zero if the specified object is in the list
 */

int
PtrListC::includes(const PtrT& obj) const
{
   return (indexOf(obj) != NULL_INDEX);
}

/*------------------------------------------------------------------------
 * Add the specified object to the list in the requested position
 *    The position is ignored if this list is sorted
 */

PtrT*
PtrListC::append(const PtrT& obj)
{
   return insert(obj, _count);
}

PtrT*
PtrListC::prepend(const PtrT& obj)
{
   return insert(obj, 0);
}

PtrT*
PtrListC::add(const PtrT& obj)
{
   return append(obj);
}

/*------------------------------------------------------------------------
 * Remove the specified object or index from the list
 */

void
PtrListC::removeAll()
{
   _count = 0;
   if (autoShrink) shrink();
}

void
PtrListC::removeLast()
{
   if ( _count > 0 ) remove(_count-1);
}

/*------------------------------------------------------------------------
 * Functions to support sorting
 */

void
PtrListC::SetSorted(char val)
{
   if ( sorted != val ) {
      sorted = val;
      if ( sorted ) sort();
   }
}

/*------------------------------------------------------------------------
 * Misc
 */

void	
PtrListC::AllowDuplicates(char val)
{
   allow_duplicates = val;
}

void	
PtrListC::AutoShrink(char val)
{
   autoShrink = val;
   if ( autoShrink ) shrink();
}

void	
PtrListC::SetCapacity(unsigned count)
{
   grow(count*sizeof(PtrT));
}

/*------------------------------------------------------------------------
 * Indexing functions
 */

PtrT*
PtrListC::operator[](unsigned i) const
{
   PtrT	*tp = (PtrT*)NULL;
   if (i < _count)
      tp = &_list[i];
   else {
      cerr << "PtrListC[" << i << "]: reference out of bounds.";
      if ( _count > 0 ) cerr << " Valid range is 0:" << _count-1 <<".";
      else		cerr << " List is empty.";
      cerr <<endl;
      abort();
   }
   return tp;
}

PtrT*
PtrListC::First() const
{
   PtrT	*tp = (PtrT*)NULL;
   if ( _count > 0 ) tp = &_list[0];
   return tp;
}

PtrT*
PtrListC::Last() const
{
   PtrT	*tp = (PtrT*)NULL;
   if ( _count > 0 ) tp = &_list[_count-1];
   return tp;
}

/*------------------------------------------------------------------------
 * Printing
 */

ostream&
PtrListC::printOn(ostream& strm) const
{
   for (int i=0; i<_count; i++) {
      strm << _list[i] <<endl;
   }
   return strm;
}

/*-------------------------------------------------------------*/

void
PtrListC::grow(unsigned newSize)
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
   PtrT	*old_list = _list;
   _list = new PtrT[_space];
   if ( !_list ) {
      cerr << "Could not allocate memory for OListC of size: " << _space <<endl;
      exit(1);
   }

/* Copy list to new memory */
   register int i;
   for (i=0; i<_count; i++) {
      _list[i] = old_list[i]; /* Copying objects here */
   }

/* Free up old memory */
   if ( old_list ) delete [] old_list;
}

/*-------------------------------------------------------------*/

void
PtrListC::shrink()
{
   if ( !autoShrink ) return;

/* Decrease space if necessary */

   int  needed = _count * sizeof(PtrT);
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
   PtrT	*old_list = _list;
   _list = new PtrT[_space];

/* Copy list to new memory */
   register int i;
   for (i=0; i<_count; i++) {
      _list[i] = old_list[i]; /* Copying objects here */
   }

/* Free up old memory */
   if ( old_list ) delete [] old_list;
}

/*-------------------------------------------------------------*/

int
PtrListC::indexOf(const PtrT& obj) const
{
/* Use the orderOf call if the list is sorted.  This should increase speed */
/* for very large lists (djl) */

   if (sorted) {
     int i = orderOf(obj);
     if (i == _count)	return NULL_INDEX;
     if (_list[i] == obj)	return i;
     else		        return NULL_INDEX;
   }

/* Loop through the list until the entry is found */
/* Compare actual objects */

   register int i;
   for (i=0; (i<_count && (_list[i] != obj)); i++);

   if (i == _count) {
      return NULL_INDEX;
   }

   return i;
}

/*-------------------------------------------------------------*/

PtrT*
PtrListC::insert(const PtrT& obj, unsigned i)
{
/* If this list is sorted, ignore the given index and compute the real one */
   if ( sorted ) i = orderOf(obj);

/* Otherwise, see if index is valid */
   if (i > _count) return (PtrT*)NULL;

/* Don't allow same address in list twice */
   if ( !allow_duplicates && includes(obj) ) return (PtrT*)NULL;

/* See if there is enough space for another entry */

   if ( _count >= _space ) grow();

/* Insert entry in position i */
/* Make a space for the new entry by moving all others back */

   register int	j;
   for (j=_count; j>i; j--) {
      _list[j] = _list[j-1]; /* Copying objects here */
   }

   _count++;
   PtrT	*tp = (PtrT *)&obj;
/* Copy the object to the list object */
   _list[i] = *tp;

   return &_list[i];
}

/*-------------------------------------------------------------*/

void
PtrListC::remove(unsigned index)
{
   if (index < _count) {

      /* Move entries forward in list and overwrite this one */
      register int i;
      for (i=index+1; i<_count; i++) {
	 _list[i-1] = _list[i]; /* Copying objects here */
      }
      _count--;
      if ( autoShrink ) shrink();
   }
}

/*-------------------------------------------------------------*/

void
PtrListC::remove(const PtrT& obj)
{
/* Look up this entry */

   register unsigned i = indexOf(obj);

   if ( i != NULL_INDEX ) remove(i);
}

/*-------------------------------------------------------------*/
/* Return a pointer to the list entry just before this one */

PtrT*
PtrListC::before(const PtrT& obj) const
{
/* Look up this entry */

   register int i = indexOf(obj);

   if ( i == NULL_INDEX || i == 0 ) return (PtrT*)NULL;
   return &_list[i-1];
}

/*-------------------------------------------------------------*/

unsigned
PtrListC::orderOf(const PtrT& obj) const
{
/*
 *  This algorithm is a simple binary search
 *  If the object is in the list, the index of the object is returned.
 *  If the object is not in the list, the index where the object would fit
 *     iN THE SOrting order is returned.
 */

/* Check special cases first */
   if ( _count == 0 || obj < _list[0] ) return 0;
   if ( obj > _list[_count-1] ) return _count;

   int low = 0;
   int high = _count - 1;
   int mid = 0;

   while ( low <= high ) {

      mid = (low + high) / 2;
      /*int comp = obj.compare(_list[mid]); */
      int comp = (obj < _list[mid]) ? -1 : ((obj > _list[mid]) ? 1 : 0);

      if ( comp > 0 )		low = mid + 1;
      else if (comp == 0)	return mid;
      else 			high = mid - 1;
   }

/*
 * If the last comparison said the value was greater than the mid,
 *    then we want to return an index 1 greater than the mid because
 *    the entry should get inserted after the mid.
 */

   if (low > mid) mid++;

   return mid;

} /* End PtrListC::orderOf */

/*-------------------------------------------------------------*/

int
PtrListC::compare(const void *a, const void *b)
{
   PtrT *ta = (PtrT *)a;
   PtrT *tb = (PtrT *)b;

   /* return ta->compare(*tb); */
   return (*ta < *tb) ? -1 : ((*ta > *tb) ? 1 : 0);
}

/*-------------------------------------------------------------*/

void
PtrListC::sort()
{
   qsort((void *)_list, (size_t)_count, (size_t)sizeof(PtrT),
       	 (int (*)(const void*, const void*))compare);
}

/*-------------------------------------------------------------*/

void
PtrListC::sort(int (*compar)(const void*, const void*))
{
   if ( !compar ) return;
   qsort((void *)_list, (size_t)_count, (size_t)sizeof(PtrT), compar);
}

