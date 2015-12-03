/*
 *  $Id: ReplyPrefC.C,v 1.2 2000/05/07 12:26:12 fnevgeny Exp $
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
#include "ReplyPrefC.h"
#include "CompPrefC.h"
#include "ReplyPrefWinC.h"

#include <hgl/rsrc.h>
#include <hgl/StringListC.h>
#include <hgl/RegexC.h>
#include <hgl/CharC.h>

/*---------------------------------------------------------------
 *  Constructor
 */

ReplyPrefC::ReplyPrefC() : PrefC()
{
   prefWin = NULL;

//
// Read resources
//
   removeMe	   = get_boolean(*halApp, "removeMeFromReply",	False);
   numberReplies   = get_boolean(*halApp, "numberReplies",	True);
   stripComments   = get_boolean(*halApp, "stripComments",	True);
   indentPrefix	   = get_string (*halApp, "indentPrefix",	"> ");
   attribStr	   = get_string (*halApp, "attributionString","$from: wrote:");
   beginForwardStr = get_string (*halApp, "beginForwardString",
				 ">>>>> Forwarded message from $from:");
   endForwardStr   = get_string (*halApp, "endForwardString",
				 "<<<<< End forwarded message");

   StringC forStr  = get_string(*halApp, "forwardHeaders", "None");
   if ( forStr.Equals("none", IGNORE_CASE) ) {
      forwardAllHeaders = False;
      forwardNoHeaders  = True;
   }
   else if ( forStr.StartsWith("disp", IGNORE_CASE) ) {
      forwardAllHeaders = False;
      forwardNoHeaders  = False;
   }
   else {
      forwardAllHeaders = True;
      forwardNoHeaders  = False;
   }

//
// Update variable format in attrib and forward strings
//
   UpdateVarPat(attribStr);
   UpdateVarPat(beginForwardStr);
   UpdateVarPat(endForwardStr);

} // End constructor

/*---------------------------------------------------------------
 *  destructor
 */

ReplyPrefC::~ReplyPrefC()
{
   delete prefWin;
}

/*---------------------------------------------------------------
 *  Method to write resources to resource database
 */

Boolean
ReplyPrefC::WriteDatabase()
{
   Store("removeMeFromReply",		removeMe);
   Store("numberReplies",		numberReplies);
   Store("stripComments",		stripComments);
   Store("composeSpaceEndsAddress",	ishApp->compPrefs->spaceEndsAddr);
   Store("indentPrefix", 		indentPrefix);
   Store("attributionString",		attribStr);
   Store("beginForwardString",		beginForwardStr);
   Store("endForwardString",		endForwardStr);

   if      ( forwardAllHeaders ) Store("forwardHeaders", "All");
   else if ( forwardNoHeaders  ) Store("forwardHeaders", "None");
   else				 Store("forwardHeaders", "Displayed");

   return True;

} // End WriteDatabase

/*---------------------------------------------------------------
 *  Method to write resources to a file
 */

Boolean
ReplyPrefC::WriteFile()
{
   StringListC	lineList;
   ReadResFile(lineList);

   Update(lineList, "removeMeFromReply",	removeMe);
   Update(lineList, "numberReplies",		numberReplies);
   Update(lineList, "stripComments",		stripComments);
   Update(lineList, "composeSpaceEndsAddress",ishApp->compPrefs->spaceEndsAddr);
   Update(lineList, "indentPrefix", 		indentPrefix);
   Update(lineList, "attributionString",	attribStr);
   Update(lineList, "beginForwardString",	beginForwardStr);
   Update(lineList, "endForwardString",		endForwardStr);

   if      (forwardAllHeaders) Update(lineList, "forwardHeaders", "All");
   else if (forwardNoHeaders ) Update(lineList, "forwardHeaders", "None");
   else			       Update(lineList, "forwardHeaders", "Displayed");

//
// Write the information back to the file
//
   return WriteResFile(lineList);

} // End WriteFile

/*---------------------------------------------------------------
 *  Function to change $header: form of variables to %header
 */

void
ReplyPrefC::UpdateVarPat(StringC& str)
{
   static RegexC	*oldVarPat = NULL;
   if ( !oldVarPat ) oldVarPat = new RegexC("\\$\\([^: \t]+\\):");

//
// If there are no old style patterns, we can quit
//
   if ( oldVarPat->search(str) < 0 ) return;

//
// Change % to %%
//
   int		pos;
   u_int	offset = 0;
   while ( (pos=str.PosOf('%', offset)) >= 0 ) {
      str(pos+1,0) = '%';
      offset = pos + 2;
   }

//
// Change $header: to %header
//
   StringC	key;
   pos = 0;
   while ( (pos=oldVarPat->search(str, pos)) >= 0 ) {
      key = str((*oldVarPat)[1]);
      key = "%" + key;
      str((*oldVarPat)[0]) = key;
   }

} // End UpdateVarPat

/*------------------------------------------------------------------------
 * Method to edit the preferences
 */

void
ReplyPrefC::Edit(Widget parent)
{
   halApp->BusyCursor(True);

   if ( !prefWin ) prefWin = new ReplyPrefWinC(parent);
   prefWin->Show(parent);

   halApp->BusyCursor(False);
}

