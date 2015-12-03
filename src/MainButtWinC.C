/*
 *  $Id: MainButtWinC.C,v 1.2 2000/05/07 12:26:12 fnevgeny Exp $
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
#include "MainButtWinC.h"
#include "IshAppC.h"
#include "MainWinC.h"
#include "MainButtPrefC.h"
#include "ButtonMgrC.h"
#include "ButtonEntryC.h"

/*---------------------------------------------------------------
 *  Main window constructor
 */

MainButtWinC::MainButtWinC(Widget par)
: ButtPrefWinC(par, ishApp->mainWin->buttMgr)
{
}

/*---------------------------------------------------------------
 *  Main window destructor
 */

MainButtWinC::~MainButtWinC()
{
}

/*---------------------------------------------------------------
 *  Method to write resources
 */

void
MainButtWinC::Write()
{
   MainButtPrefC	*prefs    = ishApp->mainButtPrefs;
   StringC		*buttStr  = &prefs->buttonStr;
   StringDictC		*buttDict = &prefs->buttonResDict;

//
// Reset resources
//
   buttStr->Clear();
   buttDict->removeAll();

   StringC	resStr, valStr;

//
// Loop through buttons
//
   unsigned	count = usedList.size();
   for (int i=0; i<count; i++) {

      ButtonEntryC	*be = (ButtonEntryC *)usedList[i];

      if ( i>0 ) *buttStr += " ";
      *buttStr += be->name;

//
// Get the user's desired abbreviation
//
      resStr = buttMgr->ResPrefix() + be->name + ".labelString";
      buttDict->add(resStr, be->abbrev);

   } // End for each possible button

   prefs->WriteDatabase();

//
// Write to file if necessary
//
   if ( applyAll ) prefs->WriteFile();

} // End Write

