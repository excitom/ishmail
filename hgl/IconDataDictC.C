//
// IconDataDictC.C
//

#include <config.h>

#include "IconDataDictC.h"

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

IconDataDictCEntryList::IconDataDictCEntryList() : GROWTH_AMOUNT(4), NULL_INDEX(-1), MAX_GROWTH(32768)
{
   _list		= (IconDataDictCEntry**)NULL;
   _count		= 0;
   _space		= 0;
   allow_duplicates	= FALSE;
   sorted		= FALSE;
   adjustGrowth		= TRUE;
   autoShrink		= TRUE;
}

IconDataDictCEntryList::IconDataDictCEntryList(unsigned ga) : GROWTH_AMOUNT(ga), NULL_INDEX(-1), MAX_GROWTH(ga)
{
   _list		= (IconDataDictCEntry**)NULL;
   _count		= 0;
   _space		= 0;
   allow_duplicates	= FALSE;
   sorted		= FALSE;
   adjustGrowth		= FALSE;
   autoShrink		= TRUE;
}

IconDataDictCEntryList::IconDataDictCEntryList(const IconDataDictCEntryList& l):GROWTH_AMOUNT(4), NULL_INDEX(-1), MAX_GROWTH(32768)
{
   _list		= (IconDataDictCEntry**)NULL;
   _count		= 0;
   _space		= 0;
   allow_duplicates	= FALSE;
   sorted		= FALSE;
   adjustGrowth		= TRUE;
   autoShrink		= TRUE;
   *this		= l;
}

IconDataDictCEntryList::~IconDataDictCEntryList()
{
   if (_list) delete [] _list;
}

/* Append another list to this one */
IconDataDictCEntryList&
IconDataDictCEntryList::operator+=(const IconDataDictCEntryList& l)
{
   if ( this != &l ) {
      register int count = l.size();
      register int i;
      for (i=0; i<count; i++) append(l[i]);
   }
   return *this;
}

/* Copy another list to this one */
IconDataDictCEntryList&
IconDataDictCEntryList::operator=(const IconDataDictCEntryList& l)
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
IconDataDictCEntryList::orderOf(const IconDataDictCEntry* op) const
{
   return orderOf(*op);
}

/* Return non-zero if the specified object is in the list */

int
IconDataDictCEntryList::includes(const IconDataDictCEntry& obj) const
{
   return (indexOf(obj) != NULL_INDEX);
}

int
IconDataDictCEntryList::includes(const IconDataDictCEntry* op) const
{
   return (indexOf(op) != NULL_INDEX);
}

/* Add the specified object to the list in the requested position */
/*    The position is ignored if this list is sorted */

IconDataDictCEntry*
IconDataDictCEntryList::append(const IconDataDictCEntry* op)
{
   return insert(op, _count);
}

IconDataDictCEntry*
IconDataDictCEntryList::prepend(const IconDataDictCEntry* op)
{
   return insert(op, 0); 
}

IconDataDictCEntry*
IconDataDictCEntryList::add(const IconDataDictCEntry* op)
{
   return append(op); 
}

/* Remove the specified object or index from the list */

void
IconDataDictCEntryList::removeAll()
{
   _count = 0;
   if (autoShrink) shrink();
}

void
IconDataDictCEntryList::removeLast()
{
   if ( _count > 0 ) remove(_count-1);
}

/* Functions to support sorting */

void
IconDataDictCEntryList::SetSorted(char val)
{
   if ( sorted != val ) {
      sorted = val;
      if ( sorted ) sort();
   }
}

void
IconDataDictCEntryList::AllowDuplicates(char val)
{
   allow_duplicates = val;
}

void
IconDataDictCEntryList::AutoShrink(char val)
{
   autoShrink = val;
   if ( autoShrink ) shrink();
}

void
IconDataDictCEntryList::SetCapacity(unsigned count)
{
   grow(count*sizeof(IconDataDictCEntry*));
}

IconDataDictCEntry*
IconDataDictCEntryList::operator[](unsigned i) const
{
   IconDataDictCEntry	*tp = (IconDataDictCEntry*)NULL;
   if (i < _count)
      tp = _list[i];
   else {
      cerr << "IconDataDictCEntryList[" << i << "]: reference out of bounds.";
      if ( _count > 0 ) cerr << " Valid range is 0:" << _count-1 <<".";
      else		cerr << " List is empty.";
      cerr <<endl;
      abort();
   }
   return tp;
}

IconDataDictCEntry*
IconDataDictCEntryList::First() const
{
   IconDataDictCEntry	*tp = (IconDataDictCEntry*)NULL;
   if ( _count > 0 ) tp = _list[0];
   return tp;
}

IconDataDictCEntry*
IconDataDictCEntryList::Last() const
{
   IconDataDictCEntry	*tp = (IconDataDictCEntry*)NULL;
   if ( _count > 0 ) tp = _list[_count-1];
   return tp;
}

ostream&
IconDataDictCEntryList::printOn(ostream& strm) const
{
   for (int i=0; i<_count; i++)
      strm <<dec(i,5) <<":" <<*_list[i] <<endl;
   return strm;
}

/*-------------------------------------------------------------*/

void
IconDataDictCEntryList::grow(unsigned newSize)
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
   IconDataDictCEntry	**old_list = _list;
   _list = new IconDataDictCEntry*[_space];
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
IconDataDictCEntryList::shrink()
{
   if ( !autoShrink ) return;

/* Decrease space if necessary */

   int  needed = _count * sizeof(IconDataDictCEntry*);
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
   IconDataDictCEntry	**old_list = _list;
   _list = new IconDataDictCEntry*[_space];
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
IconDataDictCEntryList::indexOf(const IconDataDictCEntry& obj) const
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
IconDataDictCEntryList::indexOf(const IconDataDictCEntry* op) const
{
/* Loop through the list until the entry is found */
/* Compare pointers to objects */

   register int i;
   for (i=0; (i<_count && _list[i] != op); i++);
   if (i == _count) return NULL_INDEX;
   return i;
}

/*-------------------------------------------------------------*/

IconDataDictCEntry*
IconDataDictCEntryList::insert(const IconDataDictCEntry* op, unsigned i)
{
/* If this list is sorted, ignore the given index and compute the real one */
   if ( sorted ) i = orderOf(*op);

/* Otherwise, see if index is valid */
   else if (i > _count) return (IconDataDictCEntry*)NULL;

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
   _list[i] = (IconDataDictCEntry *)op;

   return _list[i];
}

/*-------------------------------------------------------------*/

IconDataDictCEntry*
IconDataDictCEntryList::replace(const IconDataDictCEntry* op, unsigned i)
{
/* See if index is valid */
   if (i > _count) return (IconDataDictCEntry*)NULL;

/* Don't allow same address in list twice */
   if ( op != (IconDataDictCEntry*)NULL && !allow_duplicates ) {
      int	index = indexOf(op);
      if ( index != NULL_INDEX ) return _list[index];
   }

/* Insert entry in position i */
   _list[i] = (IconDataDictCEntry *)op;
   if ( sorted ) sort();

   return _list[i];
}

/*-------------------------------------------------------------*/

void
IconDataDictCEntryList::remove(unsigned index)
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
IconDataDictCEntryList::remove(const IconDataDictCEntry& obj)
{
/* Look up this entry */

   register int i = indexOf(obj);
   if ( i != NULL_INDEX ) remove(i);
}

/*-------------------------------------------------------------*/

void
IconDataDictCEntryList::remove(const IconDataDictCEntry* op)
{
/* Look up this entry */

   register int i = indexOf(op);
   if ( i != NULL_INDEX ) remove(i);
}

/*-------------------------------------------------------------*/

void
IconDataDictCEntryList::removeNulls()
{
/* Collapse list */
   register int	j = 0;
   register int i;
   for (i=0; i<_count; i++) {
      if ( _list[i] != (IconDataDictCEntry*)NULL ) {
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

IconDataDictCEntry*
IconDataDictCEntryList::before(const IconDataDictCEntry* op) const
{
/* Look up this entry */

   register int i = indexOf(op);
   if ( i == NULL_INDEX || i == 0 ) return (IconDataDictCEntry*)NULL;
   return _list[i-1];
}

/*-------------------------------------------------------------*/

unsigned
IconDataDictCEntryList::orderOf(const IconDataDictCEntry& obj) const
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

} /* End IconDataDictCEntryList::orderOf */

/*-------------------------------------------------------------*/

int
IconDataDictCEntryList::compare(const void *a, const void *b)
{
   IconDataDictCEntry	*ta = *(IconDataDictCEntry **)a;
   IconDataDictCEntry	*tb = *(IconDataDictCEntry **)b;

   /* return ta->compare(*tb); */
   return (*ta < *tb) ? -1 : ((*ta > *tb) ? 1 : 0);
}

/*-------------------------------------------------------------*/

void
IconDataDictCEntryList::sort()
{
   qsort((void *)_list, (size_t)_count, (size_t)sizeof(IconDataDictCEntry*),
	 (int (*)(const void*, const void*))compare);
}

/*-------------------------------------------------------------*/

void
IconDataDictCEntryList::sort(int (*compar)(const void*, const void*))
{
   if ( !compar ) return;
   qsort((void *)_list, (size_t)_count, (size_t)sizeof(IconDataDictCEntry*), compar);
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
 *  PDictC.meth -- method definition file for class PDictC
 *
 *   This type of dictionary stores only pointers to the keys and values
 *   ODictC.h stores the actual keys and values
 */

/* Copy one dictionary to another */

IconDataDictC&
IconDataDictC::operator=(const IconDataDictC& d)
{
   int	count = d.size();
   /* Add each entry in source to destination */
   for (int i=0; i<count; i++) {
      IconDataDictCEntry	*e = d[i];
      add(e->key, e->val);
   }
   return *this;
}

int
IconDataDictC::indexOf(const IconDataDictCEntry& entry) const
{
   return IconDataDictCEntryList::indexOf(entry);
}

int
IconDataDictC::indexOf(const IconDataDictCEntry* entry) const
{
   return IconDataDictCEntryList::indexOf(entry);
}

int
IconDataDictC::includes(const VItemC& key) const
{
   return (indexOf(key) != NULL_INDEX);
}

int
IconDataDictC::includes(const VItemC* kp) const
{
   return (indexOf(kp) != NULL_INDEX);
}

int
IconDataDictC::includes(const IconDataDictCEntry& entry) const
{
   return IconDataDictCEntryList::includes(entry);
}

int
IconDataDictC::includes(const IconDataDictCEntry* entry) const
{
   return IconDataDictCEntryList::includes(entry);
}

void
IconDataDictC::remove(unsigned index)
{
   if ( index < _count ) {
      IconDataDictCEntry	*entry = _list[index];
      if (entry) {
	 IconDataDictCEntryList::remove(index);
	 delete entry;
      }
   }
}

void
IconDataDictC::remove(const IconDataDictCEntry& entry)
{
   IconDataDictCEntryList::remove(entry);
   delete (IconDataDictCEntry *)&entry;
}

void
IconDataDictC::remove(const IconDataDictCEntry* entry)
{
   if (entry) {
      IconDataDictCEntryList::remove(entry);
      delete (IconDataDictCEntry *)entry;
   }
}

/* Printing */

void
IconDataDictC::printOn(ostream& strm) const
{
   IconDataDictCEntryList::printOn(strm);
}

/*-------------------------------------------------------------*/

int
IconDataDictC::indexOf(const VItemC& key) const
{
/* Loop through list, checking keys for a match */
   register int i;
   for (i=0; i<_count; i++) {

/* If this one matches, return the index */
      if ( *(_list[i]->key) == key ) {
	 return i;
      }
   }
   return NULL_INDEX;
}

/*-------------------------------------------------------------*/

int
IconDataDictC::indexOf(const VItemC* kp) const
{
   if ( !kp ) return NULL_INDEX;

/* Loop through list, checking key pointers for a match */
   register int i;
   for (i=0; i<_count; i++) {

/* If this one matches, return the index */
      if ( _list[i]->key == (VItemC *)kp ) {
	 return i;
      }
   }
   return NULL_INDEX;
}

/*-------------------------------------------------------------*/

IconDataC*
IconDataDictC::definitionOf(const VItemC& key) const
{
   int	i = indexOf(key); /* Look up this entry */
   return ((i==NULL_INDEX) ? (IconDataC*)NULL : _list[i]->val);
}

/*-------------------------------------------------------------*/

IconDataC*
IconDataDictC::definitionOf(const VItemC* kp) const
{
   int	i = indexOf(kp); /* Look up this entry */
   return ((i==NULL_INDEX) ? (IconDataC*)NULL : _list[i]->val);
}

/*-------------------------------------------------------------*/

IconDataC*
IconDataDictC::valOf(const int index) const
{
   return _list[index]->val;
}

/*-------------------------------------------------------------*/

VItemC*
IconDataDictC::keyOf(const int index) const
{
    return _list[index]->key;
}

/*-------------------------------------------------------------*/

IconDataDictCEntry*
IconDataDictC::entryOf(const VItemC& key) const
{
   int	i = indexOf(key); /* Get index of key */
   return ((i==NULL_INDEX) ? (IconDataDictCEntry*)NULL : _list[i]);
}

/*-------------------------------------------------------------*/

IconDataDictCEntry*
IconDataDictC::entryOf(const VItemC* kp) const
{
   int	i = indexOf(kp); /* Get index of key */
   return ((i==NULL_INDEX) ? (IconDataDictCEntry*)NULL : _list[i]);
}

/*-------------------------------------------------------------*/

IconDataDictCEntry*
IconDataDictC::add(const VItemC* kp, const IconDataC* vp)
{
/* Check if this entry is already present */

   if ( includes(kp) ) {
      if ( definitionOf(kp) != vp ) {
	 return (IconDataDictCEntry*)NULL; /* Same key with different value */
      } else {
	 return entryOf(kp);
      }
   } else { /* Add it */
      /* Allocate a new entry */
      IconDataDictCEntry	*ent = new IconDataDictCEntry(kp, vp);
      append(ent);
      return ent;
   }
}

/*-------------------------------------------------------------*/
/* Change the definition for this key */

IconDataDictCEntry*
IconDataDictC::modify(const VItemC& key, const IconDataC* vp)
{
   int	i = indexOf(key); /* Look up this entry */

   if ( i == NULL_INDEX ) {
      return (IconDataDictCEntry*)NULL;
   } else {
      _list[i]->val = (IconDataC *)vp; /* Change it */
      return _list[i];
   }
}

/*-------------------------------------------------------------*/

IconDataDictCEntry*
IconDataDictC::modify(const VItemC* kp, const IconDataC* vp)
{
   int	i = indexOf(kp); /* Look up this entry */

   if ( i == NULL_INDEX ) {
      return (IconDataDictCEntry*)NULL;
   } else {
      _list[i]->val = (IconDataC *)vp; /* Change it */
      return _list[i];
   }
}

/*-------------------------------------------------------------*/

IconDataDictCEntry*
IconDataDictC::modify(unsigned index, const IconDataC* vp)
{
   if (index < _count) {
      _list[index]->val = (IconDataC *)vp; /* Change it */
      return _list[index];
   } else {
      return (IconDataDictCEntry*)NULL;
   }
}

/*-------------------------------------------------------------*/

void
IconDataDictC::remove(const VItemC& key)
{
   int	i = indexOf(key); /* Look up this entry */

   if ( i != NULL_INDEX ) {
      IconDataDictCEntry	*entry = _list[i];
      if (entry) {
	 IconDataDictCEntryList::remove(i);
	 delete entry;
      }
   }
}

/*-------------------------------------------------------------*/

void
IconDataDictC::remove(const VItemC* kp)
{
   int	i = indexOf(kp); /* Look up this entry */

   if ( i != NULL_INDEX ) {
      IconDataDictCEntry	*entry = _list[i];
      if (entry) {
	 IconDataDictCEntryList::remove(i);
	 delete entry;
      }
   }
}
