/*
 *  $Id: ConfPrefC.C,v 1.2 2000/05/07 12:26:11 fnevgeny Exp $
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
#include "ConfPrefC.h"
#include "ConfPrefWinC.h"

#include <hgl/HalAppC.h>
#include <hgl/rsrc.h>
#include <hgl/StringListC.h>

/*---------------------------------------------------------------
 *  Constructor
 */

ConfPrefC::ConfPrefC() : PrefC()
{
   prefWin = NULL;

//
// Read resources
//
   confirmExit          = get_boolean(*halApp, "confirmExit",		True);
   confirmSaveOnExit    = get_boolean(*halApp, "confirmSaveOnExit",	True);
   confirmFolderType    = get_boolean(*halApp, "confirmFolderType",	True);
   confirmSendNoSubject = get_boolean(*halApp, "confirmSendNoSubject",	True);
   confirmSendNoBody    = get_boolean(*halApp, "confirmSendNoBody",	True);
   confirmSendPlain     = get_boolean(*halApp, "confirmSendPlain",	True);
   confirmClearSend     = get_boolean(*halApp, "confirmClearSend",	True);
   confirmCloseSend     = get_boolean(*halApp, "confirmCloseSend",	True);
   confirm8BitPlain     = get_boolean(*halApp, "confirm8BitPlain",	True);
   confirmDeleteGraphic = get_boolean(*halApp, "confirmDeleteGraphic",	True);
   confirmDeleteFolder  = get_boolean(*halApp, "confirmDeleteFolder",	True);

} // End constructor

/*---------------------------------------------------------------
 *  destructor
 */

ConfPrefC::~ConfPrefC()
{
   delete prefWin;
}

/*---------------------------------------------------------------
 *  Method to write resources to resource database
 */

Boolean
ConfPrefC::WriteDatabase()
{
   Store("confirmExit",			confirmExit);
   Store("confirmSaveOnExit",		confirmSaveOnExit);
   Store("confirmFolderType",		confirmFolderType);
   Store("confirmSendNoSubject",	confirmSendNoSubject);
   Store("confirmSendNoBody",		confirmSendNoBody);
   Store("confirmSendPlain",		confirmSendPlain);
   Store("confirmClearSend",		confirmClearSend);
   Store("confirmCloseSend",		confirmCloseSend);
   Store("confirm8BitPlain",		confirm8BitPlain);
   Store("confirmDeleteGraphic",	confirmDeleteGraphic);
   Store("confirmDeleteFolder",		confirmDeleteFolder);

   return True;

} // End WriteDatabase

/*---------------------------------------------------------------
 *  Method to write resources to a file
 */

Boolean
ConfPrefC::WriteFile()
{
   StringListC	lineList;
   ReadResFile(lineList);

   Update(lineList, "confirmExit",		confirmExit);
   Update(lineList, "confirmSaveOnExit",	confirmSaveOnExit);
   Update(lineList, "confirmFolderType",	confirmFolderType);
   Update(lineList, "confirmSendNoSubject",	confirmSendNoSubject);
   Update(lineList, "confirmSendNoBody",	confirmSendNoBody);
   Update(lineList, "confirmSendPlain",		confirmSendPlain);
   Update(lineList, "confirmClearSend",		confirmClearSend);
   Update(lineList, "confirmCloseSend",		confirmCloseSend);
   Update(lineList, "confirm8BitPlain",		confirm8BitPlain);
   Update(lineList, "confirmDeleteGraphic",	confirmDeleteGraphic);
   Update(lineList, "confirmDeleteFolder",	confirmDeleteFolder);

//
// Write the information back to the file
//
   return WriteResFile(lineList);

} // End WriteFile

/*------------------------------------------------------------------------
 * Method to edit the preferences
 */

void
ConfPrefC::Edit(Widget parent)
{
   halApp->BusyCursor(True);

   if ( !prefWin ) prefWin = new ConfPrefWinC(parent);
   prefWin->Show();

   halApp->BusyCursor(False);
}

