/*
 *  $Id: IconPrefC.C,v 1.2 2000/05/07 12:26:11 fnevgeny Exp $
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
#include "IconPrefC.h"
#include "IconPrefWinC.h"

#include <hgl/HalAppC.h>
#include <hgl/rsrc.h>
#include <hgl/StringListC.h>

/*---------------------------------------------------------------
 *  Constructor
 */

IconPrefC::IconPrefC() : PrefC()
{
   prefWin = NULL;

//
// Read resources
//
   iconRules.AllowDuplicates(TRUE);
   StringC	tmpStr = get_string(*halApp, "icons");
   ExtractRules(tmpStr, iconRules);

} // End constructor

/*---------------------------------------------------------------
 *  destructor
 */

IconPrefC::~IconPrefC()
{
   delete prefWin;
}

/*---------------------------------------------------------------
 *  Method to write resources to resource database
 */

Boolean
IconPrefC::WriteDatabase()
{
   Store("icons", iconRules);

   return True;

} // End WriteDatabase

/*---------------------------------------------------------------
 *  Method to write resources to a file
 */

Boolean
IconPrefC::WriteFile()
{
   StringListC	lineList;
   ReadResFile(lineList);

   Update(lineList, "icons", iconRules);

//
// Write the information back to the file
//
   return WriteResFile(lineList);

} // End WriteFile

/*------------------------------------------------------------------------
 * Method to edit the preferences
 */

void
IconPrefC::Edit(Widget parent)
{
   halApp->BusyCursor(True);

   if ( !prefWin ) prefWin = new IconPrefWinC(parent);
   prefWin->Show();

   halApp->BusyCursor(False);
}

