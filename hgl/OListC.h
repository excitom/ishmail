/*
 * $Id: OListC.h,v 1.1.1.1 2000/04/25 13:49:01 fnevgeny Exp $
 *
 * Copyright (c) 1992 HaL Computer Systems, Inc.  All rights reserved.
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

#ifndef	_OListC_h_
#define	_OListC_h_

#include "Base.h"
#include <generic.h>

/*
 * OListC.h -- header file for class OListC
 *
 *   This type of list store the members themselves
 *   PListC stores only pointers to the members
 */

#define  DEFINE_OListC_Class(CLASS,TYPE) \
 \
class CLASS { \
 \
protected: \
 \
   TYPE		*_list; \
   unsigned	_count; \
   unsigned	_space; \
   char		allow_duplicates; \
   char		sorted; \
   char		adjustGrowth; \
   char		autoShrink; \
   void		grow(unsigned size=0); \
   void		shrink(); \
 \
public: \
 \
   unsigned		GROWTH_AMOUNT; \
   const int		NULL_INDEX; \
   const unsigned	MAX_GROWTH; \
 \
   inline CLASS() : GROWTH_AMOUNT(4), NULL_INDEX(-1), MAX_GROWTH(32768) { \
      _list = NULL; \
      _count = _space = 0; \
      allow_duplicates = sorted = FALSE; \
      adjustGrowth = TRUE; \
      autoShrink = TRUE; \
   } \
   inline CLASS(unsigned ga) : GROWTH_AMOUNT(ga), NULL_INDEX(-1), MAX_GROWTH(ga) { \
      _list = NULL; \
      _count = _space = 0; \
      allow_duplicates = sorted = FALSE; \
      adjustGrowth = FALSE; \
      autoShrink = TRUE; \
   } \
   inline CLASS(const CLASS& l) : GROWTH_AMOUNT(4), NULL_INDEX(-1), MAX_GROWTH(32768) { \
      _list = NULL; \
      _count = _space = 0; \
      allow_duplicates = sorted = FALSE; \
      adjustGrowth = TRUE; \
      autoShrink = TRUE; \
      *this = l; \
   } \
   virtual inline ~CLASS()	{ if (_list) delete [] _list; } \
 \
/* Append another list to this one */ \
   inline CLASS& operator+=(const CLASS& l) { \
      if ( this != &l ) { \
	 register int count = l.size(); \
	 for (register int i=0; i<count; i++) { \
	    TYPE *item = l[i]; \
	    add(*item); \
	 } \
      } \
      return (*this); \
   } \
 \
/* Copy another list to this one */ \
   inline CLASS& operator=(const CLASS& l) { \
      if ( this != &l ) { \
	 removeAll(); \
	 *this += l; \
      } \
      return (*this); \
   } \
 \
/* Return the position of the specified object.	 Object must be in list */ \
 \
   int		indexOf(const TYPE& obj) const; \
 \
/* Return the sorted position of the specified object.	Object does not */ \
/*    have to be in the list */ \
 \
   unsigned	orderOf(const TYPE& obj) const; \
 \
/* Return non-zero if the specified object is in the list */ \
 \
   inline int	includes(const TYPE& obj) const \
      { return (indexOf(obj) != NULL_INDEX); } \
 \
/* Add the specified object to the list in the requested position */ \
/*    The position is ignored if this list is sorted */ \
 \
   TYPE		*insert(const TYPE& obj, unsigned index); \
   inline TYPE	*append(const TYPE& obj) { return(insert(obj, _count));} \
   inline TYPE	*prepend(const TYPE& obj) { return(insert(obj, 0)); } \
   inline TYPE	*add(const TYPE& obj) { return append(obj); } \
 \
/* Remove the specified object or index from the list */ \
 \
   void		remove(unsigned index); \
   void		remove(const TYPE& obj); \
   inline void	removeAll()	{ _count = 0; if (autoShrink) shrink(); } \
   inline void	removeLast()	{ if (_count>0) remove(_count-1); } \
 \
/* Return a pointer to the previous object in the list */ \
 \
   TYPE		*before(const TYPE& obj) const; \
 \
/* Functions to support sorting */ \
 \
   static int	compare(const void*, const void*); \
   void		sort(); \
   void		sort(int (*)(const void*, const void*)); \
   inline void	SetSorted(char val) { \
      if ( sorted != val ) { \
	 sorted = val; \
	 if ( sorted ) sort(); \
      } \
   } \
 \
   inline unsigned	size() const	{ return (_count); } \
   inline unsigned	capacity() const{ return (_space); } \
   inline TYPE		*start() const	{ return (_list); } \
   inline void		AllowDuplicates(char val) { allow_duplicates = val; } \
   inline void		AutoShrink(char val) { \
      autoShrink = val; \
      if ( autoShrink ) shrink(); \
   } \
   inline void		SetCapacity(unsigned count) { \
      grow(count*sizeof(TYPE)); \
   } \
 \
   inline TYPE	*operator[](unsigned i) const { \
      TYPE	*tp = NULL; \
      if (i < _count) \
	 tp = &_list[i]; \
      else \
	 cerr << "OListC[" << i << "]: reference out of bounds." \
	      << " Valid range is 0:" << _count-1 NL; \
      return tp; \
   } \
\
   inline TYPE		*First() const { \
      TYPE	*tp = NULL; \
      if ( _count > 0 ) tp = &_list[0]; \
      return tp; \
   } \
\
   inline TYPE		*Last() const { \
      TYPE	*tp = NULL; \
      if ( _count > 0 ) tp = &_list[_count-1]; \
      return tp; \
   } \
\
   inline ostream&	printOn(ostream& strm=cout) const { \
      for (int i=0; i<_count; i++) { \
	 strm << _list[i] NL; \
      } \
      return strm; \
   } \
}; \
 \
/* PRINTING */ \
 \
inline ostream& \
operator<<(ostream& strm, const CLASS& c) \
{ \
   return c.printOn(strm); \
}

//***************************************************************************
//***************************************************************************

#define  DEFINE_OListC_Methods(CLASS,TYPE) \
 \
/*-------------------------------------------------------------*/ \
 \
void \
CLASS::grow(unsigned newSize) \
{ \
/* Increase space */ \
 \
   if ( newSize == 0 ) { \
      _space += GROWTH_AMOUNT; \
      if ( adjustGrowth && GROWTH_AMOUNT < MAX_GROWTH ) { \
	 GROWTH_AMOUNT *= 2; \
         if ( GROWTH_AMOUNT > MAX_GROWTH ) GROWTH_AMOUNT = MAX_GROWTH; \
      } \
   } \
   else if ( newSize == _space ) { \
      return; \
   } \
   else if ( newSize < _space ) { \
      if ( !autoShrink ) return; \
      _space = newSize; \
      if ( newSize < _count ) _count = newSize; /* Only copy this many */ \
   } \
   else { \
      _space = newSize; \
   } \
 \
/* Allocate memory for new list */ \
   TYPE	*old_list = _list; \
   _list = new TYPE[_space]; \
   if ( !_list ) { \
      cerr << "Could not allocate memory for OListC of size: " << _space NL; \
      exit(1); \
   } \
 \
/* Copy list to new memory */ \
   for (register int i=0; i<_count; i++) { \
      _list[i] = old_list[i]; /* Copying objects here */ \
   } \
 \
/* Free up old memory */ \
   if ( old_list ) delete [] old_list; \
} \
 \
/*-------------------------------------------------------------*/ \
 \
void \
CLASS::shrink() \
{ \
   if ( !autoShrink ) return; \
 \
/* Decrease space if necessary */ \
 \
   int  needed = _count * sizeof(TYPE); \
   if ( needed > (_space>>1) ) return; \
 \
/* Start from scratch and get just as much space as necessary */ \
 \
   if ( adjustGrowth ) GROWTH_AMOUNT = 4; \
   _space = GROWTH_AMOUNT; \
   if ( adjustGrowth && GROWTH_AMOUNT < MAX_GROWTH ) { \
      GROWTH_AMOUNT *= 2; \
      if ( GROWTH_AMOUNT > MAX_GROWTH ) GROWTH_AMOUNT = MAX_GROWTH; \
   } \
 \
   while ( _space < needed ) { \
      _space += GROWTH_AMOUNT; \
      if ( adjustGrowth && GROWTH_AMOUNT < MAX_GROWTH ) { \
	 GROWTH_AMOUNT *= 2; \
	 if ( GROWTH_AMOUNT > MAX_GROWTH ) GROWTH_AMOUNT = MAX_GROWTH; \
      } \
   } \
 \
/* Allocate memory for new list */ \
   TYPE	*old_list = _list; \
   _list = new TYPE[_space]; \
 \
/* Copy list to new memory */ \
   for (register int i=0; i<_count; i++) { \
      _list[i] = old_list[i]; /* Copying objects here */ \
   } \
 \
/* Free up old memory */ \
   if ( old_list ) delete [] old_list; \
} \
 \
/*-------------------------------------------------------------*/ \
 \
int \
CLASS::indexOf(const TYPE& obj) const \
{ \
/* Use the orderOf call if the list is sorted.  This should increase speed */ \
/* for very large lists (djl) */ \
\
   if (sorted) { \
     int i = orderOf(obj); \
     if (i == _count) return (NULL_INDEX); \
     if (_list[i] == obj) return (i); \
     else return (NULL_INDEX); \
   } \
 \
/* Loop through the list until the entry is found */ \
/* Compare actual objects */ \
 \
   for (register int i=0; (i<_count && (_list[i] != obj)); i++); \
 \
   if (i == _count) { \
      return (NULL_INDEX); \
   } \
 \
   return (i); \
} \
 \
/*-------------------------------------------------------------*/ \
 \
TYPE* \
CLASS::insert(const TYPE& obj, unsigned i) \
{ \
/* If this list is sorted, ignore the given index and compute the real one */ \
   if ( sorted ) i = orderOf(obj); \
 \
/* Otherwise, see if index is valid */ \
   if (i > _count) return (NULL); \
 \
/* Don't allow same address in list twice */ \
   if ( !allow_duplicates && includes(obj) ) return (NULL); \
 \
/* See if there is enough space for another entry */ \
 \
   if ( _count >= _space ) grow(); \
 \
/* Insert entry in position i */ \
/* Make a space for the new entry by moving all others back */ \
 \
   register int	j; \
   for (j=_count; j>i; j--) { \
      _list[j] = _list[j-1]; /* Copying objects here */ \
   } \
 \
   _count++; \
   TYPE	*tp = (TYPE *)&obj; \
/* Copy the object to the list object */ \
   _list[i] = *tp; \
 \
   return(&_list[i]); \
} \
 \
/*-------------------------------------------------------------*/ \
 \
void \
CLASS::remove(unsigned index) \
{ \
   if (index < _count) { \
 \
      /* Move entries forward in list and overwrite this one */ \
      for (register int i=index+1; i<_count; i++) { \
	 _list[i-1] = _list[i]; /* Copying objects here */ \
      } \
      _count--; \
      if ( autoShrink ) shrink(); \
   } \
} \
 \
/*-------------------------------------------------------------*/ \
 \
void \
CLASS::remove(const TYPE& obj) \
{ \
/* Look up this entry */ \
 \
   register unsigned i = indexOf(obj); \
 \
   if ( i != NULL_INDEX ) remove(i); \
} \
 \
/*-------------------------------------------------------------*/ \
/* Return a pointer to the list entry just before this one */ \
 \
TYPE* \
CLASS::before(const TYPE& obj) const \
{ \
/* Look up this entry */ \
 \
   register int i = indexOf(obj); \
 \
   if ( i == NULL_INDEX || i == 0 ) return (NULL); \
   return(&_list[i-1]); \
} \
 \
/*-------------------------------------------------------------*/ \
 \
unsigned \
CLASS::orderOf(const TYPE& obj) const \
{ \
/* \
 *  This algorithm is a simple binary search \
 *  If the object is in the list, the index of the object is returned. \
 *  If the object is not in the list, the index where the object would fit \
 *     iN THE SOrting order is returned.  \
 */ \
 \
/* Check special cases first */ \
   if ( _count == 0 || obj < _list[0] ) return (0); \
   if ( obj > _list[_count-1] ) return (_count); \
 \
   int low = 0; \
   int high = _count - 1; \
   int mid = 0; \
 \
   while ( low <= high ) { \
 \
      mid = (low + high) / 2; \
      /*int comp = obj.compare(_list[mid]); */ \
      int comp = (obj < _list[mid]) ? -1 : ((obj > _list[mid]) ? 1 : 0); \
 \
      if ( comp > 0 )		low = mid + 1; \
      else if (comp == 0)	return (mid); \
      else 			high = mid - 1; \
   } \
 \
/* \
 * If the last comparison said the value was greater than the mid, \
 *    then we want to return an index 1 greater than the mid because \
 *    the entry should get inserted after the mid. \
 */ \
 \
   if (low > mid) mid++; \
 \
   return (mid); \
 \
} /* End CLASS::orderOf */ \
 \
/*-------------------------------------------------------------*/ \
 \
int \
CLASS::compare(const void *a, const void *b) \
{ \
   TYPE *ta = (TYPE *)a; \
   TYPE *tb = (TYPE *)b; \
 \
   /* return ta->compare(*tb); */ \
   return (*ta < *tb) ? -1 : ((*ta > *tb) ? 1 : 0); \
} \
 \
/*-------------------------------------------------------------*/ \
 \
void \
CLASS::sort() \
{ \
   qsort((void *)_list, (size_t)_count, (size_t)sizeof(TYPE), \
       	 (int (*)(const void*, const void*))compare); \
} \
 \
/*-------------------------------------------------------------*/ \
 \
void \
CLASS::sort(int (*compar)(const void*, const void*)) \
{ \
   if ( !compar ) return; \
   qsort((void *)_list, (size_t)_count, (size_t)sizeof(TYPE), compar); \
}

#endif // _OListC_h_
