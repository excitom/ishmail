/*
 *  $Id: ReadPrefC.C,v 1.2 2000/05/07 12:26:12 fnevgeny Exp $
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
#include "ReadPrefC.h"
#include "ReadPrefWinC.h"

#include <hgl/HalAppC.h>
#include <hgl/rsrc.h>
#include <hgl/CharC.h>
#include <hgl/StringListC.h>

/*---------------------------------------------------------------
 *  Constructor
 */

ReadPrefC::ReadPrefC() : PrefC()
{
   prefWin = NULL;

//
// Read resources
//
   visCols     = get_int    (*halApp, "readingColumns",		80);
   visHeadRows = get_int    (*halApp, "readingHeaderRows",	6);
   visBodyRows = get_int    (*halApp, "readingBodyRows",	30);
   wrap	       = get_boolean(*halApp, "readingWrapText",	True);
   divideText  = get_boolean(*halApp, "readingTextDividers",	False);
   webCmd      = get_string (*halApp, "webBrowserCommand",	"Mosaic %s");
   decryptCmd  = get_string (*halApp, "decryptCommand",		"xtermexec Decrypt ishdecrypt d %s");
   authCmd     = get_string (*halApp, "authenticationCommand",	"xtermexec Authenticate ishdecrypt a %s");

   StringC	tmpStr = get_string(*halApp, "readingViewType",	"flat");
   if      ( tmpStr.Equals("outline",  IGNORE_CASE) )
      viewType = READ_VIEW_OUTLINE;
   else if ( tmpStr.Equals("source",   IGNORE_CASE) )
      viewType = READ_VIEW_SOURCE;
   else if ( tmpStr.StartsWith("cont", IGNORE_CASE) )
      viewType = READ_VIEW_CONTAINER;
   else
      viewType = READ_VIEW_FLAT;

} // End constructor

/*---------------------------------------------------------------
 *  destructor
 */

ReadPrefC::~ReadPrefC()
{
   delete prefWin;
}

/*---------------------------------------------------------------
 *  Method to write resources to resource database
 */

Boolean
ReadPrefC::WriteDatabase()
{
   if ( visCols     > 0 ) Store("readingColumns",	visCols);
   if ( visHeadRows > 0 ) Store("readingHeaderRows",	visHeadRows);
   if ( visBodyRows > 0 ) Store("readingBodyRows",	visBodyRows);

   Store("readingWrapText",		wrap);
   Store("readingDivideText",		divideText);
   Store("webBrowserCommand",		webCmd);
   Store("decryptCommand",		decryptCmd);
   Store("authenticationCommand",	authCmd);

   switch (viewType) {
      case (READ_VIEW_FLAT):	  Store("readingViewType", "Flat");	 break;
      case (READ_VIEW_OUTLINE):   Store("readingViewType", "Outline");	 break;
      case (READ_VIEW_CONTAINER): Store("readingViewType", "Container"); break;
      case (READ_VIEW_SOURCE):    Store("readingViewType", "Source");	 break;
   }

   return True;

} // End WriteDatabase

/*---------------------------------------------------------------
 *  Method to write resources to a file
 */

Boolean
ReadPrefC::WriteFile()
{
   StringListC	lineList;
   ReadResFile(lineList);

   if ( visCols     > 0 ) Update(lineList, "readingColumns",    visCols);
   if ( visHeadRows > 0 ) Update(lineList, "readingHeaderRows", visHeadRows);
   if ( visBodyRows > 0 ) Update(lineList, "readingBodyRows",   visBodyRows);

   Update(lineList, "readingWrapText",		wrap);
   Update(lineList, "readingDivideText",	divideText);
   Update(lineList, "webBrowserCommand",	webCmd);
   Update(lineList, "decryptCommand",		decryptCmd);
   Update(lineList, "authenticationCommand",	authCmd);

   switch (viewType) {
      case (READ_VIEW_FLAT):
	 Update(lineList, "readingViewType",	"Flat");
	 break;
      case (READ_VIEW_OUTLINE):
	 Update(lineList, "readingViewType",	"Outline");
	 break;
      case (READ_VIEW_CONTAINER):
	 Update(lineList, "readingViewType",	"Container");
	 break;
      case (READ_VIEW_SOURCE):
	 Update(lineList, "readingViewType",	"Source");
	 break;
   }

//
// Write the information back to the file
//
   return WriteResFile(lineList);

} // End WriteFile

/*------------------------------------------------------------------------
 * Method to edit the preferences
 */

void
ReadPrefC::Edit(Widget parent)
{
   halApp->BusyCursor(True);

   if ( !prefWin ) prefWin = new ReadPrefWinC(parent);
   prefWin->Show();

   halApp->BusyCursor(False);
}

