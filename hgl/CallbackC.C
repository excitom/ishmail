/*
 * $Id: CallbackC.C,v 1.2 2000/05/07 12:26:10 fnevgeny Exp $
 *
 * Copyright (c) 1993 HAL Computer Systems International, Ltd.
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

#include "CallbackC.h"
#include "CallbackListC.h"

CallbackC::CallbackC()
{
   function = (CallbackFn*)NULL;
   data = NULL;
   next = NULL;
}

CallbackC::CallbackC(const CallbackC& c)
{
   *this = c;
   next  = NULL;
}

CallbackC::CallbackC(CallbackFn *fn, void *d)
{
   function = fn;
   data     = d;
   next     = NULL;
}

// Calling the function

void
CallbackC::operator()(void *item) const
{
   if ( function ) (*function) (item, data);
}

int
CallbackC::compare(const CallbackC& c) const
{
   int	val;
   if      ( (unsigned long)function < (unsigned long)c.function ) val = -1;
   else if ( (unsigned long)function > (unsigned long)c.function ) val =  1;
   else {
      if      ( (unsigned long)data < (unsigned long)c.data ) val = -1;
      else if ( (unsigned long)data > (unsigned long)c.data ) val =  1;
      else					              val =  0;
   }
   return val;
}

CallbackC&
CallbackC::operator=(const CallbackC& c)
{
   if ( this != &c ) {
      function = c.function;
      data     = c.data;
   }
   return (*this);
}

void
CallbackC::Set(CallbackFn *f, void* d)
{
   function = f;
   data     = d;
}

//
// Callback list operations
//
void
CallCallbacks(CallbackListC& list, void *data)
{
//
// Call from a temporary copy of the list in case any callback removes itself.
//
   CallbackListC	tmpList = list;
   unsigned	count = list.size();
   for (int i=0; i<count; i++) (*tmpList[i])(data);
}

void
AddCallback(CallbackListC& list, CallbackFn *fn, void *data)
{
   list.append(new CallbackC(fn, data));
}

void
RemoveCallback(CallbackListC& list, CallbackFn *fn, void *data)
{
   CallbackC sample(fn, data);

//
// Loop from the end
//
   unsigned	count = list.size();
   for (int i=count-1; i>=0; i--) {
      CallbackC	*cb = list[i];
      if ( sample == *cb ) {
	 list.remove(i);
	 delete cb;
      }
   }
}

void
DeleteCallbacks(CallbackListC& list)
{
   unsigned	count = list.size();
   for (int i=0; i<count; i++) delete list[i];
}

void
CallCallbacks(CallbackC **list, void *data)
{
//
// Loop through callbacks
//
   CallbackC	*cb = *list;
   while ( cb ) {
      CallbackC	*next = cb->next;	// In case callback removes itself.
      (*cb)(data);			// Call it
      cb = next;
   }

} // End CallCallbacks

void
AddCallback(CallbackC **list, CallbackFn *fn, void *data)
{
   CallbackC	*newCb = new CallbackC(fn, data);
   if ( !*list ) {
      *list = newCb;
   }
   else {
      CallbackC	*cb = *list;
      while ( cb->next ) cb = cb->next;
      cb->next = newCb;
   }
}

void
RemoveCallback(CallbackC **list, CallbackFn *fn, void *data)
{
   CallbackC sample(fn, data);

//
// Loop through list
//
   CallbackC	*cb   = *list;
   CallbackC	*prev = NULL;
   while ( cb ) {

      CallbackC	*next = cb->next;
      if ( *cb == sample ) {
	 delete cb;
	 if ( prev ) prev->next = next;
	 else	     *list      = next;
      }
      else
	 prev = cb;

      cb = next;
   }
}

void
DeleteCallbacks(CallbackC **list)
{
//
// Loop through callbacks
//
   CallbackC	*cb = *list;
   while ( cb ) {
      CallbackC	*next = cb->next;	// In case callback removes itself.
      delete cb;
      cb = next;
   }

   *list = NULL;
}
