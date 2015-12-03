//
// PartialMsgDictC.C
//

#include <config.h>

#include "PartialMsgDictC.h"

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
 * PartialMsgDictCEntryList constructors
 */

PartialMsgDictCEntryList::PartialMsgDictCEntryList() : GROWTH_AMOUNT(4), NULL_INDEX(-1), MAX_GROWTH(32768)
{
   _list		= (PartialMsgDictCEntry*)NULL;
   _count		= 0;
   _space		= 0;
   allow_duplicates	= FALSE;
   sorted		= FALSE;
   adjustGrowth		= TRUE;
   autoShrink		= TRUE;
}

PartialMsgDictCEntryList::PartialMsgDictCEntryList(unsigned ga) : GROWTH_AMOUNT(ga), NULL_INDEX(-1), MAX_GROWTH(ga)
{
   _list		= (PartialMsgDictCEntry*)NULL;
   _count		= 0;
   _space		= 0;
   allow_duplicates	= FALSE;
   sorted		= FALSE;
   adjustGrowth		= FALSE;
   autoShrink		= TRUE;
}

PartialMsgDictCEntryList::PartialMsgDictCEntryList(const PartialMsgDictCEntryList& l):GROWTH_AMOUNT(4), NULL_INDEX(-1), MAX_GROWTH(32768)
{
   _list		= (PartialMsgDictCEntry*)NULL;
   _count		= 0;
   _space		= 0;
   allow_duplicates	= FALSE;
   sorted		= FALSE;
   adjustGrowth		= TRUE;
   autoShrink		= TRUE;
   *this		= l;
}

/*------------------------------------------------------------------------
 * PartialMsgDictCEntryList destructor
 */

PartialMsgDictCEntryList::~PartialMsgDictCEntryList()
{
   if (_list) delete [] _list;
}

/*------------------------------------------------------------------------
 * Append another list to this one
 */

PartialMsgDictCEntryList&
PartialMsgDictCEntryList::operator+=(const PartialMsgDictCEntryList& l)
{
   if ( this != &l ) {
      register int count = l.size();
      register int i;
      for (i=0; i<count; i++) {
	 PartialMsgDictCEntry *item = l[i];
	 add(*item);
      }
   }
   return *this;
}

/*------------------------------------------------------------------------
 * Copy another list to this one
 */

PartialMsgDictCEntryList&
PartialMsgDictCEntryList::operator=(const PartialMsgDictCEntryList& l)
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
PartialMsgDictCEntryList::includes(const PartialMsgDictCEntry& obj) const
{
   return (indexOf(obj) != NULL_INDEX);
}

/*------------------------------------------------------------------------
 * Add the specified object to the list in the requested position
 *    The position is ignored if this list is sorted
 */

PartialMsgDictCEntry*
PartialMsgDictCEntryList::append(const PartialMsgDictCEntry& obj)
{
   return insert(obj, _count);
}

PartialMsgDictCEntry*
PartialMsgDictCEntryList::prepend(const PartialMsgDictCEntry& obj)
{
   return insert(obj, 0);
}

PartialMsgDictCEntry*
PartialMsgDictCEntryList::add(const PartialMsgDictCEntry& obj)
{
   return append(obj);
}

/*------------------------------------------------------------------------
 * Remove the specified object or index from the list
 */

void
PartialMsgDictCEntryList::removeAll()
{
   _count = 0;
   if (autoShrink) shrink();
}

void
PartialMsgDictCEntryList::removeLast()
{
   if ( _count > 0 ) remove(_count-1);
}

/*------------------------------------------------------------------------
 * Functions to support sorting
 */

void
PartialMsgDictCEntryList::SetSorted(char val)
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
PartialMsgDictCEntryList::AllowDuplicates(char val)
{
   allow_duplicates = val;
}

void	
PartialMsgDictCEntryList::AutoShrink(char val)
{
   autoShrink = val;
   if ( autoShrink ) shrink();
}

void	
PartialMsgDictCEntryList::SetCapacity(unsigned count)
{
   grow(count*sizeof(PartialMsgDictCEntry));
}

/*------------------------------------------------------------------------
 * Indexing functions
 */

PartialMsgDictCEntry*
PartialMsgDictCEntryList::operator[](unsigned i) const
{
   PartialMsgDictCEntry	*tp = (PartialMsgDictCEntry*)NULL;
   if (i < _count)
      tp = &_list[i];
   else {
      cerr << "PartialMsgDictCEntryList[" << i << "]: reference out of bounds.";
      if ( _count > 0 ) cerr << " Valid range is 0:" << _count-1 <<".";
      else		cerr << " List is empty.";
      cerr <<endl;
      abort();
   }
   return tp;
}

PartialMsgDictCEntry*
PartialMsgDictCEntryList::First() const
{
   PartialMsgDictCEntry	*tp = (PartialMsgDictCEntry*)NULL;
   if ( _count > 0 ) tp = &_list[0];
   return tp;
}

PartialMsgDictCEntry*
PartialMsgDictCEntryList::Last() const
{
   PartialMsgDictCEntry	*tp = (PartialMsgDictCEntry*)NULL;
   if ( _count > 0 ) tp = &_list[_count-1];
   return tp;
}

/*------------------------------------------------------------------------
 * Printing
 */

ostream&
PartialMsgDictCEntryList::printOn(ostream& strm) const
{
   for (int i=0; i<_count; i++) {
      strm << _list[i] <<endl;
   }
   return strm;
}

/*-------------------------------------------------------------*/

void
PartialMsgDictCEntryList::grow(unsigned newSize)
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
   PartialMsgDictCEntry	*old_list = _list;
   _list = new PartialMsgDictCEntry[_space];
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
PartialMsgDictCEntryList::shrink()
{
   if ( !autoShrink ) return;

/* Decrease space if necessary */

   int  needed = _count * sizeof(PartialMsgDictCEntry);
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
   PartialMsgDictCEntry	*old_list = _list;
   _list = new PartialMsgDictCEntry[_space];

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
PartialMsgDictCEntryList::indexOf(const PartialMsgDictCEntry& obj) const
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

PartialMsgDictCEntry*
PartialMsgDictCEntryList::insert(const PartialMsgDictCEntry& obj, unsigned i)
{
/* If this list is sorted, ignore the given index and compute the real one */
   if ( sorted ) i = orderOf(obj);

/* Otherwise, see if index is valid */
   if (i > _count) return (PartialMsgDictCEntry*)NULL;

/* Don't allow same address in list twice */
   if ( !allow_duplicates && includes(obj) ) return (PartialMsgDictCEntry*)NULL;

/* See if there is enough space for another entry */

   if ( _count >= _space ) grow();

/* Insert entry in position i */
/* Make a space for the new entry by moving all others back */

   register int	j;
   for (j=_count; j>i; j--) {
      _list[j] = _list[j-1]; /* Copying objects here */
   }

   _count++;
   PartialMsgDictCEntry	*tp = (PartialMsgDictCEntry *)&obj;
/* Copy the object to the list object */
   _list[i] = *tp;

   return &_list[i];
}

/*-------------------------------------------------------------*/

void
PartialMsgDictCEntryList::remove(unsigned index)
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
PartialMsgDictCEntryList::remove(const PartialMsgDictCEntry& obj)
{
/* Look up this entry */

   register unsigned i = indexOf(obj);

   if ( i != NULL_INDEX ) remove(i);
}

/*-------------------------------------------------------------*/
/* Return a pointer to the list entry just before this one */

PartialMsgDictCEntry*
PartialMsgDictCEntryList::before(const PartialMsgDictCEntry& obj) const
{
/* Look up this entry */

   register int i = indexOf(obj);

   if ( i == NULL_INDEX || i == 0 ) return (PartialMsgDictCEntry*)NULL;
   return &_list[i-1];
}

/*-------------------------------------------------------------*/

unsigned
PartialMsgDictCEntryList::orderOf(const PartialMsgDictCEntry& obj) const
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

} /* End PartialMsgDictCEntryList::orderOf */

/*-------------------------------------------------------------*/

int
PartialMsgDictCEntryList::compare(const void *a, const void *b)
{
   PartialMsgDictCEntry *ta = (PartialMsgDictCEntry *)a;
   PartialMsgDictCEntry *tb = (PartialMsgDictCEntry *)b;

   /* return ta->compare(*tb); */
   return (*ta < *tb) ? -1 : ((*ta > *tb) ? 1 : 0);
}

/*-------------------------------------------------------------*/

void
PartialMsgDictCEntryList::sort()
{
   qsort((void *)_list, (size_t)_count, (size_t)sizeof(PartialMsgDictCEntry),
       	 (int (*)(const void*, const void*))compare);
}

/*-------------------------------------------------------------*/

void
PartialMsgDictCEntryList::sort(int (*compar)(const void*, const void*))
{
   if ( !compar ) return;
   qsort((void *)_list, (size_t)_count, (size_t)sizeof(PartialMsgDictCEntry), compar);
}


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
 * ODictC.meth -- method definition file for class ODictC
 *
 *   This type of list store the members themselves
 *   PDictC stores only pointers to the members
 */

/* Copy one dictionary to another */

PartialMsgDictC&
PartialMsgDictC::operator=(const PartialMsgDictC& d)
{
   if ( this != &d ) {

      _count = 0;
      register int	count = d.size();
//
// Add each entry in source to destination
//
      register int i;
      for (i=0; i<count; i++) {
	 PartialMsgDictCEntry	*e = d[i];
	 add(e->key, e->val);
      }
   }
   return *this;
}

/* Lookup up entries */

int
PartialMsgDictC::indexOf(const PartialMsgDictCEntry& entry) const
{
   return PartialMsgDictCEntryList::indexOf(entry);
}

int
PartialMsgDictC::includes(const StringC& key) const
{
   return (indexOf(key) != NULL_INDEX);
}

int
PartialMsgDictC::includes(const PartialMsgDictCEntry& entry) const
{
   return PartialMsgDictCEntryList::includes(entry);
}

/* Deleting entries */

void
PartialMsgDictC::remove(unsigned index)
{
   PartialMsgDictCEntryList::remove(index);
}

void
PartialMsgDictC::remove(PartialMsgDictCEntry& entry)
{
   PartialMsgDictCEntryList::remove(entry);
}

/*-------------------------------------------------------------*/

int
PartialMsgDictC::indexOf(const StringC& key) const
{
/* Loop through list, checking keys for a match */
   register int i;
   for (i=0; i<_count; i++) {

/* If this one matches, return the index */
      if ( _list[i].key == key ) {
	 return i;
      }
   }
   return NULL_INDEX;
}

/*-------------------------------------------------------------*/

PartialMsgPtr*
PartialMsgDictC::definitionOf(const StringC& key) const
{
   int	i = indexOf(key); /* Look up this entry */
   return ((i==NULL_INDEX) ? (PartialMsgPtr*)NULL : &_list[i].val);
}

/*-------------------------------------------------------------*/

StringC&
PartialMsgDictC::keyOf(const int index) const
{
    return _list[index].key;
}

/*-------------------------------------------------------------*/

PartialMsgPtr&
PartialMsgDictC::valOf(const int index) const
{
    return _list[index].val;
}

/*-------------------------------------------------------------*/

PartialMsgDictCEntry*
PartialMsgDictC::entryOf(const StringC& key) const
{
   int	i = indexOf(key); /* Get index of key */
   return ((i==NULL_INDEX) ? (PartialMsgDictCEntry*)NULL : &_list[i]);
}

/*-------------------------------------------------------------*/

PartialMsgDictCEntry*
PartialMsgDictC::add(const StringC& key, const PartialMsgPtr& val)
{
/* Check if this entry is already present */

   if ( includes(key) ) {
      if ( *definitionOf(key) != val ) {
         return (PartialMsgDictCEntry*)NULL; /* Same key with different value */
      } else {
         return entryOf(key);
      }
   } else { /* Add it */
      PartialMsgDictCEntry      ent(key, val);
      return append(ent);
   }
}

/*-------------------------------------------------------------*/
/* Change the definition for this key */

PartialMsgDictCEntry*
PartialMsgDictC::modify(const StringC& key, const PartialMsgPtr& val)
{
   int	i = indexOf(key); /* Look up this entry */

   if ( i == NULL_INDEX ) {
      return (PartialMsgDictCEntry*)NULL;
   } else {
      _list[i].val = val; /* Change it */
      return &_list[i];
   }
}

/*-------------------------------------------------------------*/

PartialMsgDictCEntry*
PartialMsgDictC::modify(unsigned index, const PartialMsgPtr& val)
{
   if (index < _count) {
      _list[index].val = val; /* Change it */
      return &_list[index];
   } else {
      return (PartialMsgDictCEntry*)NULL;
   }
}

/*-------------------------------------------------------------*/

void
PartialMsgDictC::remove( StringC& key)
{
   int	i = indexOf(key); /* Look up this entry */

   if ( i != NULL_INDEX ) {
      PartialMsgDictCEntryList::remove(i);
   }
}

