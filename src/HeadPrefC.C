/*
 *  $Id: HeadPrefC.C,v 1.2 2000/05/07 12:26:11 fnevgeny Exp $
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
#include "HeadPrefC.h"
#include "HeadPrefWinC.h"
#include "Misc.h"

#include <hgl/HalAppC.h>
#include <hgl/rsrc.h>
#include <hgl/CharC.h>

/*---------------------------------------------------------------
 *  Constructor
 */

HeadPrefC::HeadPrefC() : PrefC()
{
   prefWin = NULL;

//
// Read resources
//
   StringC defaultShow("From Date To Cc Subject");
   StringC showHeadStr = get_string (*halApp, "displayedHeaders", defaultShow);
   ExtractList(showHeadStr, showHeadList);

   StringC	defaultHide(300);
   defaultHide += "Bcc ";
   defaultHide += "Comments ";
   defaultHide += "Content-Length ";
   defaultHide += "Content-Transfer-Encoding ";
   defaultHide += "Content-Type ";
   defaultHide += "Encrypted ";
   defaultHide += "In-Reply-To ";
   defaultHide += "Keywords ";
   defaultHide += "Mailer ";
   defaultHide += "Message-Id ";
   defaultHide += "Mime-Version ";
   defaultHide += "Return-Path ";
   defaultHide += "Received ";
   defaultHide += "References ";
   defaultHide += "Reply-To ";
   defaultHide += "Resent-Bcc ";
   defaultHide += "Resent-Cc ";
   defaultHide += "Resent-Date ";
   defaultHide += "Resent-From ";
   defaultHide += "Resent-Message-Id ";
   defaultHide += "Resent-Reply-To ";
   defaultHide += "Resent-Sender ";
   defaultHide += "Resent-To ";
   defaultHide += "Sender ";
   defaultHide += "Status ";
   StringC hideHeadStr = get_string (*halApp, "ignoredHeaders", defaultHide);
   ExtractList(hideHeadStr, hideHeadList);

   useShowHeadList = get_boolean(*halApp, "suppressUsingDisplayedHeaders",True);

//
// Remove any shown headers in the ignore list
//
   if ( useShowHeadList ) {

      unsigned	count = showHeadList.size();
      for (int i=0; i<count; i++) {
	 StringC	*head = showHeadList[i];
	 if ( hideHeadList.includes(*head) ) hideHeadList.remove(*head);
      }
   }

//
// Remove any ignored headers in the show list
//
   else {

      unsigned	count = hideHeadList.size();
      for (int i=0; i<count; i++) {
	 StringC	*head = hideHeadList[i];
	 if ( showHeadList.includes(*head) ) showHeadList.remove(*head);
      }
   }

} // End constructor

/*---------------------------------------------------------------
 *  destructor
 */

HeadPrefC::~HeadPrefC()
{
   delete prefWin;
}

/*---------------------------------------------------------------
 *  Method to write resources to resource database
 */

Boolean
HeadPrefC::WriteDatabase()
{
   Store("displayedHeaders",			showHeadList);
   Store("ignoredHeaders",			hideHeadList);
   Store("suppressUsingDisplayedHeaders",	useShowHeadList);

   return True;

} // End WriteDatabase

/*---------------------------------------------------------------
 *  Method to write resources to a file
 */

Boolean
HeadPrefC::WriteFile()
{
   StringListC	lineList;
   ReadResFile(lineList);

   Update(lineList, "displayedHeaders",			showHeadList);
   Update(lineList, "ignoredHeaders",			hideHeadList);
   Update(lineList, "suppressUsingDisplayedHeaders",	useShowHeadList);

//
// Write the information back to the file
//
   return WriteResFile(lineList);

} // End WriteFile

/*------------------------------------------------------------------------
 * Method to determine if a message header is to be displayed
 */

Boolean
HeadPrefC::HeaderShown(CharC head)
{
//
// This header is ok if it's in the show list or not in the hide list
//
   StringListC*	list;
   if ( useShowHeadList ) list = &showHeadList;
   else			  list = &hideHeadList;

   unsigned	count = list->size();
   Boolean	found = False;
   for (int i=0; !found && i<count; i++) {
      StringC	*entry = (*list)[i];
      found = head.Equals(*entry, IGNORE_CASE);
   }

   return (useShowHeadList ? found : !found);

} // End HeaderShown

/*------------------------------------------------------------------------
 * Method to edit the preferences
 */

void
HeadPrefC::Edit(Widget parent)
{
   halApp->BusyCursor(True);

   if ( !prefWin ) prefWin = new HeadPrefWinC(parent);
   prefWin->Show();

   halApp->BusyCursor(False);
}

