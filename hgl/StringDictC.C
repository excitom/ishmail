//
// StringDictC.C
//

#include <config.h>

#include "StringDictC.h"

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
 * StringDictCEntryList constructors
 */

StringDictCEntryList::StringDictCEntryList() : GROWTH_AMOUNT(4), NULL_INDEX(-1), MAX_GROWTH(32768)
{
   _list		= (StringDictCEntry*)NULL;
   _count		= 0;
   _space		= 0;
   allow_duplicates	= FALSE;
   sorted		= FALSE;
   adjustGrowth		= TRUE;
   autoShrink		= TRUE;
}

StringDictCEntryList::StringDictCEntryList(unsigned ga) : GROWTH_AMOUNT(ga), NULL_INDEX(-1), MAX_GROWTH(ga)
{
   _list		= (StringDictCEntry*)NULL;
   _count		= 0;
   _space		= 0;
   allow_duplicates	= FALSE;
   sorted		= FALSE;
   adjustGrowth		= FALSE;
   autoShrink		= TRUE;
}

StringDictCEntryList::StringDictCEntryList(const StringDictCEntryList& l):GROWTH_AMOUNT(4), NULL_INDEX(-1), MAX_GROWTH(32768)
{
   _list		= (StringDictCEntry*)NULL;
   _count		= 0;
   _space		= 0;
   allow_duplicates	= FALSE;
   sorted		= FALSE;
   adjustGrowth		= TRUE;
   autoShrink		= TRUE;
   *this		= l;
}

/*------------------------------------------------------------------------
 * StringDictCEntryList destructor
 */

StringDictCEntryList::~StringDictCEntryList()
{
   if (_list) delete [] _list;
}

/*------------------------------------------------------------------------
 * Append another list to this one
 */

StringDictCEntryList&
StringDictCEntryList::operator+=(const StringDictCEntryList& l)
{
   if ( this != &l ) {
      register int count = l.size();
      register int i;
      for (i=0; i<count; i++) {
	 StringDictCEntry *item = l[i];
	 add(*item);
      }
   }
   return *this;
}

/*------------------------------------------------------------------------
 * Copy another list to this one
 */

StringDictCEntryList&
StringDictCEntryList::operator=(const StringDictCEntryList& l)
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
StringDictCEntryList::includes(const StringDictCEntry& obj) const
{
   return (indexOf(obj) != NULL_INDEX);
}

/*------------------------------------------------------------------------
 * Add the specified object to the list in the requested position
 *    The position is ignored if this list is sorted
 */

StringDictCEntry*
StringDictCEntryList::append(const StringDictCEntry& obj)
{
   return insert(obj, _count);
}

StringDictCEntry*
StringDictCEntryList::prepend(const StringDictCEntry& obj)
{
   return insert(obj, 0);
}

StringDictCEntry*
StringDictCEntryList::add(const StringDictCEntry& obj)
{
   return append(obj);
}

/*------------------------------------------------------------------------
 * Remove the specified object or index from the list
 */

void
StringDictCEntryList::removeAll()
{
   _count = 0;
   if (autoShrink) shrink();
}

void
StringDictCEntryList::removeLast()
{
   if ( _count > 0 ) remove(_count-1);
}

/*------------------------------------------------------------------------
 * Functions to support sorting
 */

void
StringDictCEntryList::SetSorted(char val)
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
StringDictCEntryList::AllowDuplicates(char val)
{
   allow_duplicates = val;
}

void	
StringDictCEntryList::AutoShrink(char val)
{
   autoShrink = val;
   if ( autoShrink ) shrink();
}

void	
StringDictCEntryList::SetCapacity(unsigned count)
{
   grow(count*sizeof(StringDictCEntry));
}

/*------------------------------------------------------------------------
 * Indexing functions
 */

StringDictCEntry*
StringDictCEntryList::operator[](unsigned i) const
{
   StringDictCEntry	*tp = (StringDictCEntry*)NULL;
   if (i < _count)
      tp = &_list[i];
   else {
      cerr << "StringDictCEntryList[" << i << "]: reference out of bounds.";
      if ( _count > 0 ) cerr << " Valid range is 0:" << _count-1 <<".";
      else		cerr << " List is empty.";
      cerr <<endl;
      abort();
   }
   return tp;
}

StringDictCEntry*
StringDictCEntryList::First() const
{
   StringDictCEntry	*tp = (StringDictCEntry*)NULL;
   if ( _count > 0 ) tp = &_list[0];
   return tp;
}

StringDictCEntry*
StringDictCEntryList::Last() const
{
   StringDictCEntry	*tp = (StringDictCEntry*)NULL;
   if ( _count > 0 ) tp = &_list[_count-1];
   return tp;
}

/*------------------------------------------------------------------------
 * Printing
 */

ostream&
StringDictCEntryList::printOn(ostream& strm) const
{
   for (int i=0; i<_count; i++) {
      strm << _list[i] <<endl;
   }
   return strm;
}

/*-------------------------------------------------------------*/

void
StringDictCEntryList::grow(unsigned newSize)
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
   StringDictCEntry	*old_list = _list;
   _list = new StringDictCEntry[_space];
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
StringDictCEntryList::shrink()
{
   if ( !autoShrink ) return;

/* Decrease space if necessary */

   int  needed = _count * sizeof(StringDictCEntry);
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
   StringDictCEntry	*old_list = _list;
   _list = new StringDictCEntry[_space];

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
StringDictCEntryList::indexOf(const StringDictCEntry& obj) const
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

StringDictCEntry*
StringDictCEntryList::insert(const StringDictCEntry& obj, unsigned i)
{
/* If this list is sorted, ignore the given index and compute the real one */
   if ( sorted ) i = orderOf(obj);

/* Otherwise, see if index is valid */
   if (i > _count) return (StringDictCEntry*)NULL;

/* Don't allow same address in list twice */
   if ( !allow_duplicates && includes(obj) ) return (StringDictCEntry*)NULL;

/* See if there is enough space for another entry */

   if ( _count >= _space ) grow();

/* Insert entry in position i */
/* Make a space for the new entry by moving all others back */

   register int	j;
   for (j=_count; j>i; j--) {
      _list[j] = _list[j-1]; /* Copying objects here */
   }

   _count++;
   StringDictCEntry	*tp = (StringDictCEntry *)&obj;
/* Copy the object to the list object */
   _list[i] = *tp;

   return &_list[i];
}

/*-------------------------------------------------------------*/

void
StringDictCEntryList::remove(unsigned index)
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
StringDictCEntryList::remove(const StringDictCEntry& obj)
{
/* Look up this entry */

   register unsigned i = indexOf(obj);

   if ( i != NULL_INDEX ) remove(i);
}

/*-------------------------------------------------------------*/
/* Return a pointer to the list entry just before this one */

StringDictCEntry*
StringDictCEntryList::before(const StringDictCEntry& obj) const
{
/* Look up this entry */

   register int i = indexOf(obj);

   if ( i == NULL_INDEX || i == 0 ) return (StringDictCEntry*)NULL;
   return &_list[i-1];
}

/*-------------------------------------------------------------*/

unsigned
StringDictCEntryList::orderOf(const StringDictCEntry& obj) const
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

} /* End StringDictCEntryList::orderOf */

/*-------------------------------------------------------------*/

int
StringDictCEntryList::compare(const void *a, const void *b)
{
   StringDictCEntry *ta = (StringDictCEntry *)a;
   StringDictCEntry *tb = (StringDictCEntry *)b;

   /* return ta->compare(*tb); */
   return (*ta < *tb) ? -1 : ((*ta > *tb) ? 1 : 0);
}

/*-------------------------------------------------------------*/

void
StringDictCEntryList::sort()
{
   qsort((void *)_list, (size_t)_count, (size_t)sizeof(StringDictCEntry),
       	 (int (*)(const void*, const void*))compare);
}

/*-------------------------------------------------------------*/

void
StringDictCEntryList::sort(int (*compar)(const void*, const void*))
{
   if ( !compar ) return;
   qsort((void *)_list, (size_t)_count, (size_t)sizeof(StringDictCEntry), compar);
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

StringDictC&
StringDictC::operator=(const StringDictC& d)
{
   if ( this != &d ) {

      _count = 0;
      register int	count = d.size();
//
// Add each entry in source to destination
//
      register int i;
      for (i=0; i<count; i++) {
	 StringDictCEntry	*e = d[i];
	 add(e->key, e->val);
      }
   }
   return *this;
}

/* Lookup up entries */

int
StringDictC::indexOf(const StringDictCEntry& entry) const
{
   return StringDictCEntryList::indexOf(entry);
}

int
StringDictC::includes(const StringC& key) const
{
   return (indexOf(key) != NULL_INDEX);
}

int
StringDictC::includes(const StringDictCEntry& entry) const
{
   return StringDictCEntryList::includes(entry);
}

/* Deleting entries */

void
StringDictC::remove(unsigned index)
{
   StringDictCEntryList::remove(index);
}

void
StringDictC::remove(StringDictCEntry& entry)
{
   StringDictCEntryList::remove(entry);
}

/*-------------------------------------------------------------*/

int
StringDictC::indexOf(const StringC& key) const
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

StringC*
StringDictC::definitionOf(const StringC& key) const
{
   int	i = indexOf(key); /* Look up this entry */
   return ((i==NULL_INDEX) ? (StringC*)NULL : &_list[i].val);
}

/*-------------------------------------------------------------*/

StringC&
StringDictC::keyOf(const int index) const
{
    return _list[index].key;
}

/*-------------------------------------------------------------*/

StringC&
StringDictC::valOf(const int index) const
{
    return _list[index].val;
}

/*-------------------------------------------------------------*/

StringDictCEntry*
StringDictC::entryOf(const StringC& key) const
{
   int	i = indexOf(key); /* Get index of key */
   return ((i==NULL_INDEX) ? (StringDictCEntry*)NULL : &_list[i]);
}

/*-------------------------------------------------------------*/

StringDictCEntry*
StringDictC::add(const StringC& key, const StringC& val)
{
/* Check if this entry is already present */

   if ( includes(key) ) {
      if ( *definitionOf(key) != val ) {
         return (StringDictCEntry*)NULL; /* Same key with different value */
      } else {
         return entryOf(key);
      }
   } else { /* Add it */
      StringDictCEntry      ent(key, val);
      return append(ent);
   }
}

/*-------------------------------------------------------------*/
/* Change the definition for this key */

StringDictCEntry*
StringDictC::modify(const StringC& key, const StringC& val)
{
   int	i = indexOf(key); /* Look up this entry */

   if ( i == NULL_INDEX ) {
      return (StringDictCEntry*)NULL;
   } else {
      _list[i].val = val; /* Change it */
      return &_list[i];
   }
}

/*-------------------------------------------------------------*/

StringDictCEntry*
StringDictC::modify(unsigned index, const StringC& val)
{
   if (index < _count) {
      _list[index].val = val; /* Change it */
      return &_list[index];
   } else {
      return (StringDictCEntry*)NULL;
   }
}

/*-------------------------------------------------------------*/

void
StringDictC::remove( StringC& key)
{
   int	i = indexOf(key); /* Look up this entry */

   if ( i != NULL_INDEX ) {
      StringDictCEntryList::remove(i);
   }
}

