/*
 *  $Id: PtrDictC.C,v 1.2 2000/05/07 12:26:11 fnevgeny Exp $
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
#include "PtrDictC.h"

#include <stdlib.h>	// For qsort

#ifndef FALSE
#define FALSE	0
#define TRUE	(!FALSE)
#endif

/* Copy one dictionary to another */

PtrDictC&
PtrDictC::operator=(const PtrDictC& d)
{
   int	count = d.size();
   /* Add each entry in source to destination */
   for (int i=0; i<count; i++) {
      PtrDictCEntry	*e = (PtrDictCEntry*)d[i];
      add(e->key, e->val);
   }
   return *this;
}

PtrDictCEntry*
PtrDictC::operator[](unsigned i) const
{
   void	*tp = NULL;
   if (i < _count)
      tp = _list[i];
   else {
      cerr << "PtrDictC[" << i << "]: reference out of bounds.";
      if ( _count > 0 ) cerr << " Valid range is 0:" << _count-1 <<".";
      else		cerr << " List is empty.";
      cerr <<endl;
      abort();
   }
   return (PtrDictCEntry*)tp;
}

int
PtrDictC::indexOf(PtrDictCEntry* entry) const
{
   return PtrList2::indexOf(entry);
}

int
PtrDictC::includes(void* kp) const
{
   return (indexOf(kp) != NULL_INDEX);
}

int
PtrDictC::includes(PtrDictCEntry* entry) const
{
   return PtrList2::includes(entry);
}

void
PtrDictC::remove(unsigned index)
{
   if ( index < _count ) {
      PtrDictCEntry	*entry = (PtrDictCEntry*)_list[index];
      if (entry) {
	 PtrList2::remove(index);
	 delete entry;
      }
   }
}

void
PtrDictC::remove(PtrDictCEntry* entry)
{
   if ( entry ) {
      PtrList2::remove(entry);
      delete entry;
   }
}

/* Printing */

void
PtrDictC::printOn(ostream& strm) const
{
   PtrList2::printOn(strm);
}

/*-------------------------------------------------------------*/

int
PtrDictC::indexOf(void* kp) const
{
   if ( !kp ) return NULL_INDEX;

/* Loop through list, checking key pointers for a match */
   for (register int i=0; i<_count; i++) {

/* If this one matches, return the index */
      if ( ((PtrDictCEntry*)_list[i])->key == (void *)kp ) {
	 return i;
      }
   }
   return NULL_INDEX;
}

/*-------------------------------------------------------------*/

void*
PtrDictC::definitionOf(void* kp) const
{
   int	i = indexOf(kp); /* Look up this entry */
   return ((i==NULL_INDEX) ? NULL : ((PtrDictCEntry*)_list[i])->val);
}

/*-------------------------------------------------------------*/

void*
PtrDictC::valOf(int index) const
{
   return ((PtrDictCEntry*)_list[index])->val;
}

/*-------------------------------------------------------------*/

void*
PtrDictC::keyOf(int index) const
{
    return ((PtrDictCEntry*)_list[index])->key;
}

/*-------------------------------------------------------------*/

PtrDictCEntry*
PtrDictC::entryOf(void* kp) const
{
   int	i = indexOf(kp); /* Get index of key */
   return ((i==NULL_INDEX) ? (PtrDictCEntry*)NULL : (PtrDictCEntry*)_list[i]);
}

/*-------------------------------------------------------------*/

PtrDictCEntry*
PtrDictC::add(void* kp, void* vp)
{
/* Check if this entry is already present */

   if ( includes(kp) ) {
      if ( definitionOf(kp) != vp )
	 return NULL; /* Same key with different value */
      else
	 return entryOf(kp);
   }
   else { /* Add it */
      /* Allocate a new entry */
      PtrDictCEntry	*ent = new PtrDictCEntry(kp, vp);
      append(ent);
      return ent;
   }
}

/*-------------------------------------------------------------*/

PtrDictCEntry*
PtrDictC::modify(void* kp, void* vp)
{
   int	i = indexOf(kp); /* Look up this entry */

   if ( i == NULL_INDEX )
      return NULL;

   else {
      ((PtrDictCEntry*)_list[i])->val = vp; /* Change it */
      return (PtrDictCEntry*)_list[i];
   }
}

/*-------------------------------------------------------------*/

PtrDictCEntry*
PtrDictC::modify(unsigned index, void* vp)
{
   if (index < _count) {
      ((PtrDictCEntry*)_list[index])->val = vp; /* Change it */
      return (PtrDictCEntry*)_list[index];
   }
   else
      return NULL;
}

/*-------------------------------------------------------------*/

void
PtrDictC::remove(void* kp)
{
   int	i = indexOf(kp); /* Look up this entry */

   if ( i != NULL_INDEX ) {
      PtrDictCEntry	*entry = (PtrDictCEntry*)_list[i];
      if (entry) {
	 PtrList2::remove(i);
	 delete entry;
      }
   }
}
