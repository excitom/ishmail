/*
 * $Id: PDictC.h,v 1.1.1.1 2000/04/25 13:49:01 fnevgeny Exp $
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

#ifndef	_PDictC_h_
#define	_PDictC_h_

#include "PListC.h"

/*
 *  PDictC.h -- declarations for a set of relations
 *
 *   This type of dictionary stores only pointers to the keys and values
 *   ODictC.h stores the actual keys and values
 */

/* Create a class, CLASS, which is a dictionary of KEYTYPE/VALTYPE relations */

#define  DEFINE_PDictC_Class(CLASS,KEYTYPE,VALTYPE) \
 \
/* Define the dictionary entry structure */ \
 \
class name2(CLASS,PEntryC) { \
 \
public: \
 \
   KEYTYPE	*key; \
   VALTYPE	*val; \
 \
   inline name2(CLASS,PEntryC) () { key = NULL; val = NULL; } \
   inline name2(CLASS,PEntryC) (const KEYTYPE* kp, const VALTYPE* vp) \
				 { key = (KEYTYPE *)kp; val = (VALTYPE *)vp; } \
   inline name2(CLASS,PEntryC) (const name2(CLASS,PEntryC)& e) { *this = e; } \
 \
   inline name2(CLASS,PEntryC)&	operator= (const name2(CLASS,PEntryC)& e) { \
      if ( this != &e ) { \
	 key = e.key; \
	 val = e.val; \
      } \
      return (*this); \
   } \
 \
   inline int	operator== (const name2(CLASS,PEntryC)& e) const { \
   /* Only need to check pointers here */ \
      return (key == e.key && val == e.val); \
   } \
   inline int	operator!= (const name2(CLASS,PEntryC)& e) const { \
      return (!(*this==e)); \
   } \
   inline int	compare  (const name2(CLASS,PEntryC)&) const { return 0; } \
   inline int	operator<(const name2(CLASS,PEntryC)&) const { return 0; } \
   inline int	operator>(const name2(CLASS,PEntryC)&) const { return 0; } \
 \
   inline ostream&  printOn(ostream& strm=cout) const { \
      strm <<key <<" -> " <<val; \
      return strm; \
   } \
}; \
 \
inline ostream& operator<<(ostream& strm, const name2(CLASS,PEntryC)& e) \
{ \
   return e.printOn(strm); \
} \
 \
/* Create a class, CLASSPList, which is a list of CLASSPEntryCs */ \
 \
DEFINE_PListC_Class(name2(CLASS,PList),name2(CLASS,PEntryC)) \
 \
/* Create CLASS as a derived class of CLASSPList */ \
 \
class CLASS : public name2(CLASS,PList) { \
 \
public: \
 \
   inline CLASS() {} \
   virtual inline ~CLASS() {} \
 \
/* Build this dictionary from another */ \
   inline CLASS(const CLASS& d) { \
      *this = d; \
   } \
 \
/* Copy one dictionary to another */ \
 \
   inline CLASS&	operator= (const CLASS& d) { \
      int	count = d.size(); \
      /* Add each entry in source to destination */ \
      for (int i=0; i<count; i++) { \
	 name2(CLASS,PEntryC)	*e = d[i]; \
	 add(e->key, e->val); \
      } \
      return (*this); \
   } \
 \
/* Lookup up entries */ \
 \
   int			indexOf(const KEYTYPE& key) const; \
   int			indexOf(const KEYTYPE* kp) const; \
   inline int		indexOf(const name2(CLASS,PEntryC)& entry) const { \
      return (name2(CLASS,PList)::indexOf(entry)); \
   } \
   inline int		indexOf(const name2(CLASS,PEntryC)* entry) const { \
      return (name2(CLASS,PList)::indexOf(entry)); \
   } \
 \
   inline int		includes(const KEYTYPE& key) const { \
      return (indexOf(key) != NULL_INDEX); \
   } \
   inline int		includes(const KEYTYPE* kp) const { \
      return (indexOf(kp) != NULL_INDEX); \
   } \
   inline int		includes(const name2(CLASS,PEntryC)& entry) const { \
      return (name2(CLASS,PList)::includes(entry)); \
   } \
   inline int		includes(const name2(CLASS,PEntryC)* entry) const { \
      return (name2(CLASS,PList)::includes(entry)); \
   } \
 \
   VALTYPE		*definitionOf(const KEYTYPE& key) const; \
   VALTYPE		*definitionOf(const KEYTYPE* kp) const; \
 \
   VALTYPE		*valOf(const int index) const; \
 \
   KEYTYPE       *keyOf(const int index) const; \
 \
   name2(CLASS,PEntryC)	*entryOf(const KEYTYPE& key) const; \
   name2(CLASS,PEntryC)	*entryOf(const KEYTYPE* kp) const; \
 \
/* Changing entries */ \
 \
   name2(CLASS,PEntryC)	*add(const KEYTYPE* kp, const VALTYPE* vp); \
   name2(CLASS,PEntryC)	*modify(const KEYTYPE& key, const VALTYPE* vp); \
   name2(CLASS,PEntryC)	*modify(const KEYTYPE* kp, const VALTYPE* vp); \
   name2(CLASS,PEntryC)	*modify(unsigned index, const VALTYPE* vp); \
 \
/* Deleting entries */ \
 \
   void		remove(const KEYTYPE& key); \
   void		remove(const KEYTYPE* kp); \
   void		remove(unsigned index) { \
      if ( index < _count ) { \
	 name2(CLASS,PEntryC)	*entry = _list[index]; \
	 if (entry) { \
	    name2(CLASS,PList)::remove(index); \
	    delete entry; \
	 } \
      } \
   } \
   inline void		remove(const name2(CLASS,PEntryC)& entry) { \
      name2(CLASS,PList)::remove(entry); \
      delete (name2(CLASS,PEntryC) *)&entry; \
   } \
   inline void		remove(const name2(CLASS,PEntryC)* entry) { \
      if (entry) { \
	 name2(CLASS,PList)::remove(entry); \
	 delete (name2(CLASS,PEntryC) *)entry; \
      } \
   } \
 \
/* Printing */ \
 \
   inline void  printOn(ostream& strm=cout) const { \
      name2(CLASS,PList)::printOn(strm); \
   } \
}; \
 \
inline ostream& operator<<(ostream& strm, const CLASS& d) \
{ \
   d.printOn(strm); \
   return(strm); \
}

//*********************************************************************
//*********************************************************************

#define  DEFINE_PDictC_Methods(CLASS,KEYTYPE,VALTYPE) \
 \
DEFINE_PListC_Methods(name2(CLASS,PList),name2(CLASS,PEntryC)) \
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
      if ( *(_list[i]->key) == key ) { \
	 return (i); \
      } \
   } \
   return (NULL_INDEX); \
} \
 \
/*-------------------------------------------------------------*/ \
 \
int \
CLASS::indexOf(const KEYTYPE* kp) const \
{ \
   if ( !kp ) return (NULL_INDEX); \
 \
/* Loop through list, checking key pointers for a match */ \
   for (register int i=0; i<_count; i++) { \
 \
/* If this one matches, return the index */ \
      if ( _list[i]->key == (KEYTYPE *)kp ) { \
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
   return ((i==NULL_INDEX) ? NULL : _list[i]->val); \
} \
 \
/*-------------------------------------------------------------*/ \
 \
VALTYPE* \
CLASS::definitionOf(const KEYTYPE* kp) const \
{ \
   int	i = indexOf(kp); /* Look up this entry */ \
   return ((i==NULL_INDEX) ? NULL : _list[i]->val); \
} \
 \
/*-------------------------------------------------------------*/ \
 \
VALTYPE* \
CLASS::valOf(const int index) const \
{ \
   return (_list[index]->val); \
} \
 \
/*-------------------------------------------------------------*/ \
 \
KEYTYPE*  \
CLASS::keyOf(const int index) const \
{ \
    return (_list[index]->key); \
} \
 \
/*-------------------------------------------------------------*/ \
 \
name2(CLASS,PEntryC)* \
CLASS::entryOf(const KEYTYPE& key) const \
{ \
   int	i = indexOf(key); /* Get index of key */ \
   return ((i==NULL_INDEX) ? NULL : _list[i]); \
} \
 \
/*-------------------------------------------------------------*/ \
 \
name2(CLASS,PEntryC)* \
CLASS::entryOf(const KEYTYPE* kp) const \
{ \
   int	i = indexOf(kp); /* Get index of key */ \
   return ((i==NULL_INDEX) ? NULL : _list[i]); \
} \
 \
/*-------------------------------------------------------------*/ \
 \
name2(CLASS,PEntryC)* \
CLASS::add(const KEYTYPE* kp, const VALTYPE* vp) \
{ \
/* Check if this entry is already present */ \
 \
   if ( includes(kp) ) { \
      if ( definitionOf(kp) != vp ) { \
	 return (NULL); /* Same key with different value */ \
      } else { \
	 return (entryOf(kp)); \
      } \
   } else { /* Add it */ \
      /* Allocate a new entry */ \
      name2(CLASS,PEntryC)	*ent = new name2(CLASS,PEntryC)(kp, vp); \
      append(ent); \
      return (ent); \
   } \
} \
 \
/*-------------------------------------------------------------*/ \
/* Change the definition for this key */ \
 \
name2(CLASS,PEntryC)* \
CLASS::modify(const KEYTYPE& key, const VALTYPE* vp) \
{ \
   int	i = indexOf(key); /* Look up this entry */ \
 \
   if ( i == NULL_INDEX ) { \
      return (NULL); \
   } else { \
      _list[i]->val = (VALTYPE *)vp; /* Change it */ \
      return (_list[i]); \
   } \
} \
 \
/*-------------------------------------------------------------*/ \
 \
name2(CLASS,PEntryC)* \
CLASS::modify(const KEYTYPE* kp, const VALTYPE* vp) \
{ \
   int	i = indexOf(kp); /* Look up this entry */ \
 \
   if ( i == NULL_INDEX ) { \
      return (NULL); \
   } else { \
      _list[i]->val = (VALTYPE *)vp; /* Change it */ \
      return (_list[i]); \
   } \
} \
 \
/*-------------------------------------------------------------*/ \
 \
name2(CLASS,PEntryC)* \
CLASS::modify(unsigned index, const VALTYPE* vp) \
{ \
   if (index < _count) { \
      _list[index]->val = (VALTYPE *)vp; /* Change it */ \
      return (_list[index]); \
   } else { \
      return (NULL); \
   } \
} \
 \
/*-------------------------------------------------------------*/ \
 \
void \
CLASS::remove(const KEYTYPE& key) \
{ \
   int	i = indexOf(key); /* Look up this entry */ \
 \
   if ( i != NULL_INDEX ) { \
      name2(CLASS,PEntryC)	*entry = _list[i]; \
      if (entry) { \
	 name2(CLASS,PList)::remove(i); \
	 delete entry; \
      } \
   } \
} \
 \
/*-------------------------------------------------------------*/ \
 \
void \
CLASS::remove(const KEYTYPE* kp) \
{ \
   int	i = indexOf(kp); /* Look up this entry */ \
 \
   if ( i != NULL_INDEX ) { \
      name2(CLASS,PEntryC)	*entry = _list[i]; \
      if (entry) { \
	 name2(CLASS,PList)::remove(i); \
	 delete entry; \
      } \
   } \
}

#endif // _CLASS_h_
