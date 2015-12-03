/*
 * $Id: CallbackC.h,v 1.1.1.1 2000/04/25 13:49:01 fnevgeny Exp $
 *
 * Copyright (c) 1991 HaL Computer Systems, Inc.  All rights reserved.
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

#ifndef	_CallbackC_h_
#define	_CallbackC_h_

#include "Base.h"

typedef void CallbackFn(void *, void *);

class CallbackC {

public:

// Store a pointer to a function returning void and expecting an item pointer
//   and a data pointer

   CallbackFn	*function;
   void*	data;
   CallbackC	*next;	// For use in list

// ASSIGNMENT FROM CallbackC

   CallbackC&	operator=(const CallbackC&);
   void		Set(CallbackFn*, void*);

// CONSTRUCTORS

   CallbackC();
   CallbackC(const CallbackC& c);
   CallbackC(CallbackFn*, void*);

// Calling the function

   void		operator() (void*) const;

// COMPARISON TO CallbackC

   inline int		operator== (const CallbackC& s) const {
      return (data == s.data && function == s.function);
   }
   inline int	operator!= (const CallbackC& s) const { return (!(*this==s)); }
   int	compare(const CallbackC&) const;
   inline int	operator<(const CallbackC& c) const { return (compare(c) < 0); }
   inline int	operator>(const CallbackC& c) const { return (compare(c) > 0); }

// PRINTING

   inline void	printOn(ostream& strm=cout) const {
      strm << "function @ " << (long)function << ", data @ " << (long)data;
   }
};

// PRINTING

inline ostream&
operator<<(ostream& strm, const CallbackC& c)
{
   c.printOn(strm);
   return(strm);
}

class CallbackListC;

//
// Callback list convenience functions
//
extern void CallCallbacks(CallbackListC&, void*);
extern void AddCallback(CallbackListC&, CallbackFn*, void*);
extern void RemoveCallback(CallbackListC&, CallbackFn*, void*);
extern void DeleteCallbacks(CallbackListC&);

extern void CallCallbacks(CallbackC**, void*);
extern void AddCallback(CallbackC**, CallbackFn*, void*);
extern void RemoveCallback(CallbackC**, CallbackFn*, void*);
extern void DeleteCallbacks(CallbackC**);

#endif // _CallbackC_h_
