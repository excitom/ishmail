/*
 * $Id: SignalRegistry.C,v 1.3 2000/05/07 12:26:11 fnevgeny Exp $
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

#include "SignalRegistry.h"
#include "SignalDictC.h"

static SignalDictC	*sigToCallbackDict = NULL;

extern int	debuglev;

//------------------------------------------------------------------------
// general purpose signal handler.  Looks up real callback for signal
//

static void
HandleSignal(int signum)
{
   if ( debuglev > 0 ) cout <<"HandleSignal got signal " <<signum <<endl;

//
// Look up the callback list for the given signal
//
   SignalCallsC	*calls = sigToCallbackDict->definitionOf(signum);

//
// Get the id of the signalling process and the status of the signal
//
   SignalDataT	data;
   data.signum = signum;

   if ( signum == SIGCHLD ) {
      data.pid = wait(&data.status);
      if ( debuglev > 1 ) cout <<"   pid is " <<data.pid <<endl;
   }

//
// Call the registered callbacks
//
   if ( calls->cbList.size() > 0 ) {
      if ( debuglev > 1 ) cout <<"   Calling callbacks" <<endl;
      CallCallbacks(calls->cbList, &data);
   }

//
// Reregister the signal handler.  Unlike on BSD systems, signals
// in ANSI C are reset to their default behavior when  raised
//
   calls->builtIn = signal(signum, HandleSignal);

} // End HandleSignal

//------------------------------------------------------------------------
// Method to register a callback on signal detection
//

void
AddSignalCallback(int signum, CallbackFn *fn, void *data)
{
   if ( debuglev > 1 )
      cout <<"registering function " << (unsigned int)fn << " with data " <<data
	   <<" for signal " <<signum <<endl;

//
// See if a callback dictionary exists
//
   if ( !sigToCallbackDict ) sigToCallbackDict = new SignalDictC;

//
// See if a callback list exists for the specified signal
//
   if ( !sigToCallbackDict->includes(signum) ) {

//
// Create a new callback list
//
      SignalCallsC	sg;
      sg.builtIn = SIG_DFL;
      sigToCallbackDict->add(signum, sg);

   } // End if callback list not found

   SignalCallsC	*calls = sigToCallbackDict->definitionOf(signum);

//
// Add the callback to the list
//
   AddCallback(calls->cbList, fn, data);

//
// Register the signal handler for this signal if this is the first callback
//    in the list
//
   if ( calls->cbList.size() == 1 )
      calls->builtIn = signal(signum, HandleSignal);

} // End AddSignalCallback

//------------------------------------------------------------------------
// Method to deregister a callback on signal detection
//

void
RemoveSignalCallback(int signum, CallbackFn *fn, void *data)
{
   if ( debuglev > 1 )
      cout <<"deregistering function " << (unsigned int)fn << " with data " <<data
	   <<" for signal " <<signum <<endl;

   if ( !sigToCallbackDict ) return;

//
// Look up the callback list for the given signal
//
   SignalCallsC	*calls = sigToCallbackDict->definitionOf(signum);

//
// Remove the callback from the list
//
   RemoveCallback(calls->cbList, fn, data);

//
// Restore the default signal handler if no more callbacks are registered
//
   if ( calls->cbList.size() == 0 )
      signal(signum, calls->builtIn);

} // End RemoveSignalCallback
