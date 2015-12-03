/*
 *  $Id: ReadButtPrefC.C,v 1.2 2000/05/07 12:26:12 fnevgeny Exp $
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
#include "IshAppC.h"
#include "ReadButtPrefC.h"
#include "ReadButtWinC.h"
#include "ReadWinC.h"
#include "ButtonMgrC.h"

#include <hgl/rsrc.h>
#include <hgl/StringListC.h>
#include <hgl/ButtonBox.h>

/*---------------------------------------------------------------
 *  Constructor
 */

ReadButtPrefC::ReadButtPrefC() : PrefC()
{
   prefWin = NULL;

//
// Read resources
//
   buttonStr = get_string(*halApp, "readingButtons");

} // End constructor

/*---------------------------------------------------------------
 *  destructor
 */

ReadButtPrefC::~ReadButtPrefC()
{
   delete prefWin;
}

/*---------------------------------------------------------------
 *  Method to write resources to resource database
 */

Boolean
ReadButtPrefC::WriteDatabase()
{
   Store("readingButtons", buttonStr);
   unsigned	count = buttonResDict.size();
   for (int i=0; i<count; i++)
      Store(buttonResDict[i]->key, buttonResDict[i]->val);

   if ( ishApp->readWinList.size() > 0 ) {

      ReadWinC		*readWin = (ReadWinC*)*ishApp->readWinList[0];
      ButtonMgrC	*bm = readWin->buttMgr;
      short		cols;
      Boolean		sameSize;
      XtVaGetValues(bm->ButtonBox(), XmNnumColumns, &cols,
				     BbNuniformCols, &sameSize, NULL);
      Store("readWin.mainWindow*buttonRC.numColumns", (int)cols);
      Store("readWin.mainWindow*buttonRC.uniformRows", sameSize);
      Store("readWin.mainWindow*buttonRC.uniformCols", sameSize);
      StoreGravity("readWin.buttonGravity",	bm->Gravity());
   }

   return True;

} // End WriteDatabase

/*---------------------------------------------------------------
 *  Method to write resources to a file
 */

Boolean
ReadButtPrefC::WriteFile()
{
   StringListC	lineList;
   ReadResFile(lineList);

   Update(lineList, "readingButtons", buttonStr);
   unsigned	count = buttonResDict.size();
   for (int i=0; i<count; i++)
      Update(lineList, buttonResDict[i]->key, buttonResDict[i]->val);

   if ( ishApp->readWinList.size() > 0 ) {

      ReadWinC	*readWin = (ReadWinC*)*ishApp->readWinList[0];

      ButtonMgrC	*bm = readWin->buttMgr;
      short		cols;
      Boolean		sameSize;
      XtVaGetValues(bm->ButtonBox(), XmNnumColumns, &cols,
				     BbNuniformCols, &sameSize, NULL);
      Update(lineList, "readWin.mainWindow*buttonRC.numColumns", (int)cols);
      Update(lineList, "readWin.mainWindow*buttonRC.uniformRows", sameSize);
      Update(lineList, "readWin.mainWindow*buttonRC.uniformCols", sameSize);
      UpdateGravity(lineList, "readWin.buttonGravity", bm->Gravity());
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
ReadButtPrefC::Edit(Widget parent, ButtonMgrC *mgr)
{
   halApp->BusyCursor(True);

   if ( !prefWin ) prefWin = new ReadButtWinC(parent, mgr);
   else		   prefWin->AddManager(mgr);

   prefWin->Show();

   halApp->BusyCursor(False);
}

