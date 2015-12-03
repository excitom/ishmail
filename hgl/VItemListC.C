//
// VItemListC.C
//

#include <config.h>

#include "VItemListC.h"

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
 * PListC.meth -- method definition file for class PListC
 *
 *   This type of list stores only pointers to its members
 *   OListC stores the members themselves
 */

#include <stdlib.h>	// For qsort

#ifndef FALSE
#define FALSE	0
#define TRUE	(!FALSE)
#endif

VItemListC::VItemListC() : GROWTH_AMOUNT(4), NULL_INDEX(-1), MAX_GROWTH(32768)
{
   _list		= (VItemC**)NULL;
   _count		= 0;
   _space		= 0;
   allow_duplicates	= FALSE;
   sorted		= FALSE;
   adjustGrowth		= TRUE;
   autoShrink		= TRUE;
}

VItemListC::VItemListC(unsigned ga) : GROWTH_AMOUNT(ga), NULL_INDEX(-1), MAX_GROWTH(ga)
{
   _list		= (VItemC**)NULL;
   _count		= 0;
   _space		= 0;
   allow_duplicates	= FALSE;
   sorted		= FALSE;
   adjustGrowth		= FALSE;
   autoShrink		= TRUE;
}

VItemListC::VItemListC(const VItemListC& l):GROWTH_AMOUNT(4), NULL_INDEX(-1), MAX_GROWTH(32768)
{
   _list		= (VItemC**)NULL;
   _count		= 0;
   _space		= 0;
   allow_duplicates	= FALSE;
   sorted		= FALSE;
   adjustGrowth		= TRUE;
   autoShrink		= TRUE;
   *this		= l;
}

VItemListC::~VItemListC()
{
   if (_list) delete [] _list;
}

/* Append another list to this one */
VItemListC&
VItemListC::operator+=(const VItemListC& l)
{
   if ( this != &l ) {
      register int count = l.size();
      register int i;
      for (i=0; i<count; i++) append(l[i]);
   }
   return *this;
}

/* Copy another list to this one */
VItemListC&
VItemListC::operator=(const VItemListC& l)
{
   if ( this != &l ) {
      removeAll();
      *this += l;
   }
   return *this;
}

/* Return the sorted position of the specified object.  Object does not */
/*    have to be in the list */

unsigned
VItemListC::orderOf(const VItemC* op) const
{
   return orderOf(*op);
}

/* Return non-zero if the specified object is in the list */

int
VItemListC::includes(const VItemC& obj) const
{
   return (indexOf(obj) != NULL_INDEX);
}

int
VItemListC::includes(const VItemC* op) const
{
   return (indexOf(op) != NULL_INDEX);
}

/* Add the specified object to the list in the requested position */
/*    The position is ignored if this list is sorted */

VItemC*
VItemListC::append(const VItemC* op)
{
   return insert(op, _count);
}

VItemC*
VItemListC::prepend(const VItemC* op)
{
   return insert(op, 0); 
}

VItemC*
VItemListC::add(const VItemC* op)
{
   return append(op); 
}

/* Remove the specified object or index from the list */

void
VItemListC::removeAll()
{
   _count = 0;
   if (autoShrink) shrink();
}

void
VItemListC::removeLast()
{
   if ( _count > 0 ) remove(_count-1);
}

/* Functions to support sorting */

void
VItemListC::SetSorted(char val)
{
   if ( sorted != val ) {
      sorted = val;
      if ( sorted ) sort();
   }
}

void
VItemListC::AllowDuplicates(char val)
{
   allow_duplicates = val;
}

void
VItemListC::AutoShrink(char val)
{
   autoShrink = val;
   if ( autoShrink ) shrink();
}

void
VItemListC::SetCapacity(unsigned count)
{
   grow(count*sizeof(VItemC*));
}

VItemC*
VItemListC::operator[](unsigned i) const
{
   VItemC	*tp = (VItemC*)NULL;
   if (i < _count)
      tp = _list[i];
   else {
      cerr << "VItemListC[" << i << "]: reference out of bounds.";
      if ( _count > 0 ) cerr << " Valid range is 0:" << _count-1 <<".";
      else		cerr << " List is empty.";
      cerr <<endl;
      abort();
   }
   return tp;
}

VItemC*
VItemListC::First() const
{
   VItemC	*tp = (VItemC*)NULL;
   if ( _count > 0 ) tp = _list[0];
   return tp;
}

VItemC*
VItemListC::Last() const
{
   VItemC	*tp = (VItemC*)NULL;
   if ( _count > 0 ) tp = _list[_count-1];
   return tp;
}

ostream&
VItemListC::printOn(ostream& strm) const
{
   for (int i=0; i<_count; i++)
      strm <<dec(i,5) <<":" <<*_list[i] <<endl;
   return strm;
}

/*-------------------------------------------------------------*/

void
VItemListC::grow(unsigned newSize)
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
   VItemC	**old_list = _list;
   _list = new VItemC*[_space];
   if ( !_list ) {
      cerr << "Could not allocate memory for PListC of size: " << _space <<endl;
      exit(1);
   }

/* Copy list to new memory */
   register int i;
   for (i=0; i<_count; i++) {
      _list[i] = old_list[i]; /* Just copying pointers here */
   }

/* Free up old memory */
   if ( old_list ) delete [] old_list;
}

/*-------------------------------------------------------------*/

void
VItemListC::shrink()
{
   if ( !autoShrink ) return;

/* Decrease space if necessary */

   int  needed = _count * sizeof(VItemC*);
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
   VItemC	**old_list = _list;
   _list = new VItemC*[_space];
   if ( !_list ) {
      cerr << "Could not allocate memory for PListC of size: " << _space <<endl;
      exit(1);
   }

/* Copy list to new memory */
   register int i;
   for (i=0; i<_count; i++) {
      _list[i] = old_list[i]; /* Just copying pointers here */
   }

/* Free up old memory */
   if ( old_list ) delete [] old_list;
}

/*-------------------------------------------------------------*/

int
VItemListC::indexOf(const VItemC& obj) const
{
/* Use the orderOf call if the list is sorted.  This should increase speed */
/* for very large lists (djl) */

   if (sorted) {
     int i = orderOf(obj);
     if (i == _count) return NULL_INDEX;
     if (*_list[i] == obj) return i;
     else return NULL_INDEX;
   }

/* Loop through the list until the entry is found */
/* Compare actual objects */

   register int i;
   for (i=0; (i<_count && (*_list[i] != obj)); i++);
   if (i == _count) return NULL_INDEX;
   return i;
}

/*-------------------------------------------------------------*/

int
VItemListC::indexOf(const VItemC* op) const
{
/* Loop through the list until the entry is found */
/* Compare pointers to objects */

   register int i;
   for (i=0; (i<_count && _list[i] != op); i++);
   if (i == _count) return NULL_INDEX;
   return i;
}

/*-------------------------------------------------------------*/

VItemC*
VItemListC::insert(const VItemC* op, unsigned i)
{
/* If this list is sorted, ignore the given index and compute the real one */
   if ( sorted ) i = orderOf(*op);

/* Otherwise, see if index is valid */
   else if (i > _count) return (VItemC*)NULL;

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
   _list[i] = (VItemC *)op;

   return _list[i];
}

/*-------------------------------------------------------------*/

VItemC*
VItemListC::replace(const VItemC* op, unsigned i)
{
/* See if index is valid */
   if (i > _count) return (VItemC*)NULL;

/* Don't allow same address in list twice */
   if ( op != (VItemC*)NULL && !allow_duplicates ) {
      int	index = indexOf(op);
      if ( index != NULL_INDEX ) return _list[index];
   }

/* Insert entry in position i */
   _list[i] = (VItemC *)op;
   if ( sorted ) sort();

   return _list[i];
}

/*-------------------------------------------------------------*/

void
VItemListC::remove(unsigned index)
{
   if (index < _count) {

      /* Move entries forward in list and overwrite this one */
      register int i;
      for (i=index+1; i<_count; i++) {
	 _list[i-1] = _list[i]; /* Just copying pointers here */
      }
      _count--;
      if ( autoShrink ) shrink();
   }
}

/*-------------------------------------------------------------*/

void
VItemListC::remove(const VItemC& obj)
{
/* Look up this entry */

   register int i = indexOf(obj);
   if ( i != NULL_INDEX ) remove(i);
}

/*-------------------------------------------------------------*/

void
VItemListC::remove(const VItemC* op)
{
/* Look up this entry */

   register int i = indexOf(op);
   if ( i != NULL_INDEX ) remove(i);
}

/*-------------------------------------------------------------*/

void
VItemListC::removeNulls()
{
/* Collapse list */
   register int	j = 0;
   register int i;
   for (i=0; i<_count; i++) {
      if ( _list[i] != (VItemC*)NULL ) {
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

VItemC*
VItemListC::before(const VItemC* op) const
{
/* Look up this entry */

   register int i = indexOf(op);
   if ( i == NULL_INDEX || i == 0 ) return (VItemC*)NULL;
   return _list[i-1];
}

/*-------------------------------------------------------------*/

unsigned
VItemListC::orderOf(const VItemC& obj) const
{
/*
 *  This algorithm is a simple binary search
 *  If the object is in the list, the index of the object is returned.
 *  If the object is not in the list, the index where the object would fit
 *     in the sorting order is returned.
 */

/* Check special cases first */
   if ( _count == 0 || obj < *_list[0] ) return 0;
   if ( obj > *_list[_count-1] ) return _count;

   int low = 0;
   int high = _count - 1;
   int mid = 0;

   while ( low <= high ) {

      mid = (low + high) / 2;
      /* int comp = obj.compare(*_list[mid]); */
      int comp = (obj < *_list[mid]) ? -1 : ((obj > *_list[mid]) ? 1 : 0);

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

} /* End VItemListC::orderOf */

/*-------------------------------------------------------------*/

int
VItemListC::compare(const void *a, const void *b)
{
   VItemC	*ta = *(VItemC **)a;
   VItemC	*tb = *(VItemC **)b;

   /* return ta->compare(*tb); */
   return (*ta < *tb) ? -1 : ((*ta > *tb) ? 1 : 0);
}

/*-------------------------------------------------------------*/

void
VItemListC::sort()
{
   qsort((void *)_list, (size_t)_count, (size_t)sizeof(VItemC*),
	 (int (*)(const void*, const void*))compare);
}

/*-------------------------------------------------------------*/

void
VItemListC::sort(int (*compar)(const void*, const void*))
{
   if ( !compar ) return;
   qsort((void *)_list, (size_t)_count, (size_t)sizeof(VItemC*), compar);
}

