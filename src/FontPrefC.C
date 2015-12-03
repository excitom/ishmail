/*
 *  $Id: FontPrefC.C,v 1.2 2000/05/07 12:26:11 fnevgeny Exp $
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
#include "FontPrefC.h"
#include "FontPrefWinC.h"

#include <hgl/rsrc.h>
#include <hgl/FontDataC.h>

#include <hgl/StringListC.h>

/*---------------------------------------------------------------
 *  Constructor
 */

FontPrefC::FontPrefC() : PrefC()
{
   prefWin = NULL;

//
// Read resources
//
   buttonFont = get_string(*halApp, "buttonFontList");
   labelFont  = get_string(*halApp, "labelFontList");
   textFont   = get_string(*halApp, "textFontList");
   listFont   = get_string(*halApp, "listFontList");

   char		*resType;
   XrmValue	resVal;
   XrmGetResource(ishApp->resdb, "Ishmail.MimeRichTextC.font",
		  "Ishmail.MimeRichTextC.font", &resType, &resVal);
   richFont   = (char*)resVal.addr;

} // End constructor

/*---------------------------------------------------------------
 *  destructor
 */

FontPrefC::~FontPrefC()
{
   delete prefWin;
}

/*---------------------------------------------------------------
 *  Method to write resources to resource database
 */

Boolean
FontPrefC::WriteDatabase()
{
   if ( buttonFont.size() > 0 ) {
      Store("buttonFontList",		buttonFont);
      Store("XmPushButton.fontList",	buttonFont);
      Store("XmCascadeButton.fontList",	buttonFont);
   }

   if ( labelFont.size() > 0 ) {
      Store("labelFontList",		labelFont);
      Store("XmLabel.fontList",		labelFont);
      Store("XmToggleButton.fontList",	labelFont);
   }

   if ( textFont.size() > 0 ) {
      Store("textFontList",		textFont);
      Store("XmText.fontList",		textFont);
      Store("XmTextField.fontList",	textFont);
      Store("MimeRichTextC.fixedFont",	textFont);
   }

   if ( richFont.size() > 0 ) {
      Store("MimeRichTextC.font",	richFont);
   }

   if ( listFont.size() > 0 ) {

      Store("listFontList",		listFont);
      Store("XmList.fontList",		listFont);

      FontDataC	*listData = new FontDataC(listFont);
      StringC	viewStr;
      FontDataC	*pData;
      FontDataC	*bData;
      FontDataC	*iData;
      FontDataC	*biData;
      if ( listData->IsBold() ) {
	 bData  = listData;
	 pData  = bData->NonBold();
	 iData  = pData->Italic();
	 biData = bData->Italic();
      }
      else if ( listData->IsItalic() ) {
	 iData  = listData;
	 pData  = bData->NonItalic();
	 bData  = pData->Bold();
	 biData = bData->Italic();
      }
      else {
	 pData  = listData;
	 bData  = pData->Bold();
	 iData  = pData->Italic();
	 biData = bData->Italic();
      }

      viewStr  = pData->Name();  viewStr += "=plain,";
      viewStr += bData->Name();  viewStr += "=bold,";
      viewStr += iData->Name();  viewStr += "=italic,";
      viewStr += biData->Name(); viewStr += "=bold-italic";

      delete listData;

      Store("viewDA.fontList", viewStr);
   }

   return True;

} // End WriteDatabase

/*---------------------------------------------------------------
 *  Method to write resources to a file
 */

Boolean
FontPrefC::WriteFile()
{
   StringListC	lineList;
   ReadResFile(lineList);

   if ( buttonFont.size() > 0 ) {
      Update(lineList, "buttonFontList",		buttonFont);
      Update(lineList, "XmPushButton.fontList",	buttonFont);
      Update(lineList, "XmCascadeButton.fontList",	buttonFont);
   }

   if ( labelFont.size() > 0 ) {
      Update(lineList, "labelFontList",		labelFont);
      Update(lineList, "XmLabel.fontList",		labelFont);
      Update(lineList, "XmToggleButton.fontList",	labelFont);
   }

   if ( textFont.size() > 0 ) {
      Update(lineList, "textFontList",		textFont);
      Update(lineList, "XmText.fontList",		textFont);
      Update(lineList, "XmTextField.fontList",	textFont);
      Update(lineList, "MimeRichTextC.fixedFont",	textFont);
   }

   if ( richFont.size() > 0 ) {
      Update(lineList, "MimeRichTextC.font",	richFont);
   }

   if ( listFont.size() > 0 ) {
      Update(lineList, "listFontList",		listFont);
      Update(lineList, "XmList.fontList",	listFont);

      FontDataC	*listData = new FontDataC(listFont);
      StringC	viewStr;
      FontDataC	*pData;
      FontDataC	*bData;
      FontDataC	*iData;
      FontDataC	*biData;
      if ( listData->IsBold() ) {
	 bData  = listData;
	 pData  = bData->NonBold();
	 iData  = pData->Italic();
	 biData = bData->Italic();
      }
      else if ( listData->IsItalic() ) {
	 iData  = listData;
	 pData  = bData->NonItalic();
	 bData  = pData->Bold();
	 biData = bData->Italic();
      }
      else {
	 pData  = listData;
	 bData  = pData->Bold();
	 iData  = pData->Italic();
	 biData = bData->Italic();
      }

      viewStr  = pData->Name();  viewStr += "=plain,";
      viewStr += bData->Name();  viewStr += "=bold,";
      viewStr += iData->Name();  viewStr += "=italic,";
      viewStr += biData->Name(); viewStr += "=bold-italic";

      delete listData;

      Update(lineList, "viewDA.fontList", viewStr);
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
FontPrefC::Edit(Widget parent)
{
   halApp->BusyCursor(True);

   if ( !prefWin ) prefWin = new FontPrefWinC(parent);
   prefWin->Show();

   halApp->BusyCursor(False);
}

