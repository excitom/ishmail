/*
 * $Id: ODictC.h,v 1.1.1.1 2000/04/25 13:49:01 fnevgeny Exp $
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

#ifndef	_ODictC_h_
#define	_ODictC_h_

#include "OListC.h"

/*
 *  ODictC.h -- declarations for a set relations
 *
 *  This type of dictionary stores the actual keys and values
 *  PDictC.h stores only pointers to the keys and values
 */

/* Create a class, CLASS, which is a dictionary of KEYTYPE/VALTYPE relations */

#define  DEFINE_ODictC_Class(CLASS,KEYTYPE,VALTYPE) \
 \
/* Define the dictionary entry structure */ \
 \
class name2(CLASS,OEntryC) { \
 \
public: \
 \
   KEYTYPE	key; \
   VALTYPE	val; \
 \
   inline name2(CLASS,OEntryC) () {} \
   inline name2(CLASS,OEntryC) (const KEYTYPE& kobj, const VALTYPE& vobj) \
			: key(kobj), val(vobj) {} \
   inline name2(CLASS,OEntryC) (const name2(CLASS,OEntryC)& e) { *this = e; } \
 \
   inline name2(CLASS,OEntryC)&	operator= (const name2(CLASS,OEntryC)& e) { \
      if ( this != &e ) { \
	 key = e.key; \
	 val = e.val; \
      } \
      return (*this); \
   } \
 \
   inline int	operator== (const name2(CLASS,OEntryC)& e) const { \
      return (key == e.key && val == e.val); \
   } \
   inline int	operator!= (const name2(CLASS,OEntryC)& e) const { \
      return (!(*this==e)); \
   } \
   inline int	compare(const name2(CLASS,OEntryC)&) const { return 0; } \
   inline int	operator<(const name2(CLASS,OEntryC)&) const { return 0; } \
   inline int	operator>(const name2(CLASS,OEntryC)&) const { return 0; } \
 \
   inline ostream&  printOn(ostream& strm=cout) const { \
      strm << key << " = " << val; \
      return strm; \
   } \
}; \
 \
inline ostream& operator<<(ostream& strm, name2(CLASS,OEntryC)& e) \
{ \
   return e.printOn(strm); \
} \
 \
/* Create a class, CLASSOList, which is a list of CLASSOEntryCs */ \
 \
DEFINE_OListC_Class(name2(CLASS,OList),name2(CLASS,OEntryC)) \
 \
/* Create CLASS as a derived class of CLASSOList */ \
 \
class CLASS : public name2(CLASS,OList) { \
 \
public: \
 \
/* Copy one dictionary to another */ \
 \
   inline CLASS&	operator= (const CLASS& d) { \
      if ( this != &d ) { \
	 _count = 0; \
	 register int	count = d.size(); \
/* Add each entry in source to destination */ \
	 for (register int i=0; i<count; i++) { \
	    name2(CLASS,OEntryC)	*e = d[i]; \
	    add(e->key, e->val); \
	 } \
      } \
      return (*this); \
   } \
 \
/* Lookup up entries */ \
 \
   int		indexOf(const KEYTYPE& key) const; \
 \
   inline int	indexOf(const name2(CLASS,OEntryC)& entry) const { \
      return (name2(CLASS,OList)::indexOf(entry)); \
   } \
 \
   inline int	includes(const KEYTYPE& key) const { \
      return (indexOf(key) != NULL_INDEX); \
   } \
   inline int	includes(const name2(CLASS,OEntryC)& entry) const { \
      return (name2(CLASS,OList)::includes(entry)); \
   } \
 \
   VALTYPE	*definitionOf(const KEYTYPE& key) const; \
 \
   KEYTYPE	&keyOf(const int index) const; \
   VALTYPE	&valOf(const int index) const; \
 \
   name2(CLASS,OEntryC)	*entryOf(const KEYTYPE& key) const; \
 \
/* Changing entries */ \
 \
   name2(CLASS,OEntryC)	*add(const KEYTYPE& key, const VALTYPE& val); \
   name2(CLASS,OEntryC)	*modify(const KEYTYPE& key, const VALTYPE& val); \
   name2(CLASS,OEntryC)	*modify(unsigned index, const VALTYPE& val); \
 \
/* Deleting entries */ \
 \
   void		remove( KEYTYPE& key); \
   void		remove(unsigned index) { \
      name2(CLASS,OList)::remove(index); \
   } \
   inline void	remove( name2(CLASS,OEntryC)& entry) { \
      name2(CLASS,OList)::remove(entry); \
   } \
 \
}; \
 \
inline ostream& operator<<(ostream& strm, const CLASS& d) \
{ \
   return d.printOn(strm); \
}

//*********************************************************************
//*********************************************************************

#define  DEFINE_ODictC_Methods(CLASS,KEYTYPE,VALTYPE) \
 \
DEFINE_OListC_Methods(name2(CLASS,OList),name2(CLASS,OEntryC)) \
 \
/*-------------------------------------------------------------*/ \
 \
int \
CLASS::indexOf(const KEYTYPE& key) const \
{ \
/* Loop through list, checking keys for a match */ \
   for (register int i=0; i<_count; i++) { \
 \
/* If this one matches, return the index */ \
      if ( _list[i].key == key ) { \
	 return (i); \
      } \
   } \
   return (NULL_INDEX); \
} \
 \
/*-------------------------------------------------------------*/ \
 \
VALTYPE* \
CLASS::definitionOf(const KEYTYPE& key) const \
{ \
   int	i = indexOf(key); /* Look up this entry */ \
   return ((i==NULL_INDEX) ? NULL : &_list[i].val); \
} \
 \
/*-------------------------------------------------------------*/ \
 \
KEYTYPE& \
CLASS::keyOf(const int index) const \
{ \
    return (_list[index].key); \
} \
 \
/*-------------------------------------------------------------*/ \
 \
VALTYPE& \
CLASS::valOf(const int index) const \
{ \
    return (_list[index].val); \
} \
 \
/*-------------------------------------------------------------*/ \
 \
name2(CLASS,OEntryC)* \
CLASS::entryOf(const KEYTYPE& key) const \
{ \
   int	i = indexOf(key); /* Get index of key */ \
   return ((i==NULL_INDEX) ? NULL : &_list[i]); \
} \
 \
/*-------------------------------------------------------------*/ \
 \
name2(CLASS,OEntryC)* \
CLASS::add(const KEYTYPE& key, const VALTYPE& val) \
{ \
/* Check if this entry is already present */ \
 \
   if ( includes(key) ) { \
      if ( *definitionOf(key) != val ) { \
         return (NULL); /* Same key with different value */ \
      } else { \
         return (entryOf(key)); \
      } \
   } else { /* Add it */ \
      name2(CLASS,OEntryC)      ent(key, val); \
      return (append(ent)); \
   } \
} \
 \
/*-------------------------------------------------------------*/ \
/* Change the definition for this key */ \
 \
name2(CLASS,OEntryC)* \
CLASS::modify(const KEYTYPE& key, const VALTYPE& val) \
{ \
   int	i = indexOf(key); /* Look up this entry */ \
 \
   if ( i == NULL_INDEX ) { \
      return (NULL); \
   } else { \
      _list[i].val = val; /* Change it */ \
      return (&_list[i]); \
   } \
} \
 \
/*-------------------------------------------------------------*/ \
 \
name2(CLASS,OEntryC)* \
CLASS::modify(unsigned index, const VALTYPE& val) \
{ \
   if (index < _count) { \
      _list[index].val = val; /* Change it */ \
      return (&_list[index]); \
   } else { \
      return (NULL); \
   } \
} \
 \
/*-------------------------------------------------------------*/ \
 \
void \
CLASS::remove( KEYTYPE& key) \
{ \
   int	i = indexOf(key); /* Look up this entry */ \
 \
   if ( i != NULL_INDEX ) { \
      name2(CLASS,OList)::remove(i); \
   } \
} \

#endif // _CLASS_h_
