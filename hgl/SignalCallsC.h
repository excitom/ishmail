/*
 *  $Id: SignalCallsC.h,v 1.2 2000/04/27 14:38:28 fnevgeny Exp $
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
#ifndef _SignalCallsC_h_
#define _SignalCallsC_h_

#include "CallbackListC.h"

#include <signal.h>
#include <stream.h>

class SignalCallsC {

public:

   SIG_PF		builtIn;
   CallbackListC	cbList;

   SignalCallsC() { builtIn = SIG_DFL; }
   SignalCallsC& operator=(const SignalCallsC& sg) {
      if ( this != &sg ) {
	 builtIn = sg.builtIn;
	 cbList  = sg.cbList;
      }
      return *this;
   }
   SignalCallsC(const SignalCallsC& sg) { *this = sg; }
   int	operator==(const SignalCallsC&) const { return 0; }
   int	operator!=(const SignalCallsC&) const { return 1; }
};

//
// Method to allow <<
//
inline ostream& operator<<(ostream& strm, const SignalCallsC&) { return(strm); }

#endif // _SignalCallsC_h_
