/*
 *  $Id: CompPrefC.C,v 1.4 2000/09/12 17:19:05 evgeny Exp $
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
#include "CompPrefC.h"
#include "ReplyPrefC.h"
#include "AppPrefC.h"
#include "CompPrefWinC.h"
#include "ShellExp.h"
#include "Query.h"

#include <hgl/rsrc.h>
#include <hgl/StringListC.h>

// Default command line templates for helper wrappers around pgp
#define DIGSIGNCMD_DEFAULT \
    "xtermexec 'Sign Message' ishencrypt -s -i %i -o %o -u '\"%u\"'"
#define ENCRYPTCMD_DEFAULT \
    "xtermexec 'Sign Message' ishencrypt -e -i %i -o %o -u '\"%u\"' -r '\"%r\"'"
#define ENCRYPTSIGNCMD_DEFAULT \
    "xtermexec 'Sign Message' ishencrypt -s -e -i %i -o %o -u '\"%u\"' -r '\"%r\"'"
#define MIMEDIGSIGNCMD_DEFAULT \
    "xtermexec 'Sign Message' ishencrypt -m -b %b -s -i %i -o %o -u '\"%u\"'"
#define MIMEENCRYPTCMD_DEFAULT \
    "xtermexec 'Sign Message' ishencrypt -m -b %b -e -i %i -o %o -u '\"%u\"' -r '\"%r\"'"
#define MIMEENCRYPTSIGNCMD_DEFAULT \
    "xtermexec 'Sign Message' ishencrypt -m -b %b -s -e -i %i -o %o -u '\"%u\"' -r '\"%r\"'"


/*---------------------------------------------------------------
 *  Constructor
 */

CompPrefC::CompPrefC() : PrefC()
{
   prefWin	= NULL;

//
// Read resources
//
   bodyCols	= get_int    (*halApp, "composeColumns",		72);
   bodyRows	= get_int    (*halApp, "composeRows",			20);
   headRows	= get_int    (*halApp, "composeHeaderRows",		1);
   maxFieldsPerLine = get_int(*halApp, "maxHeaderFieldsPerLine",	1);
   wrap		= get_boolean(*halApp, "composeWrapText",		False);
   showCc	= get_boolean(*halApp, "composeShowCc",			False);
   showBcc	= get_boolean(*halApp, "composeShowBcc",		False);
   showFcc	= get_boolean(*halApp, "composeShowFcc",		False);
   showOther	= get_boolean(*halApp, "composeShowOther",		False);
   showSettings	= get_boolean(*halApp, "composeShowSettings",		True);
   emacsMode	= get_boolean(*halApp, "emacsMode",			False);
   delMeansBs	= get_boolean(*halApp, "deleteMeansBackspace",		False);
   spaceEndsAddr= get_boolean(*halApp, "composeSpaceEndsAddress",	False);
   editCmd	= get_string (*halApp, "editCommand",
				    "xtermexec Editor vi $FILE");
   spellCmd	= get_string (*halApp, "spellCommand",
				    "xtermexec 'Check Spelling' ispell $FILE");
   digSignCmd	= get_string (*halApp, "digitalSignCommand", DIGSIGNCMD_DEFAULT);
   encryptCmd	= get_string (*halApp, "encryptCommand", ENCRYPTCMD_DEFAULT);
   encryptSignCmd = get_string (*halApp, "encryptAndSignCommand", ENCRYPTSIGNCMD_DEFAULT);
   mimeDigSignCmd = get_string (*halApp, "digitalSignMimeCommand", MIMEDIGSIGNCMD_DEFAULT);
   mimeEncryptCmd = get_string (*halApp, "encryptMimeCommand", MIMEENCRYPTCMD_DEFAULT);
   mimeEncryptSignCmd = get_string (*halApp, "encryptAndSignMimeCommand", MIMEENCRYPTSIGNCMD_DEFAULT);
   autoSave	= get_boolean(*halApp, "composeAutoSave",		True);
   autoSaveRate	= get_int    (*halApp, "composeAutoSaveRate",		30);

   orig.autoSaveDir = ishApp->appPrefs->OrigFolderDir();
   orig.autoSaveDir += "/.ishcomp";
   orig.autoSaveDir = get_string (*halApp, "composeAutoSaveDir",
					   orig.autoSaveDir);
   autoSaveDir = orig.autoSaveDir;
   ishApp->ExpandFolderName(autoSaveDir);

   if ( bodyCols < 1 ) bodyCols = 72;
   if ( bodyRows < 1 ) bodyRows = 20;
   if ( headRows < 1 ) headRows = 1;

   if      ( maxFieldsPerLine < 1 ) maxFieldsPerLine = 1;
   else if ( maxFieldsPerLine > 5 ) maxFieldsPerLine = 5;
   
   if ( digSignCmd.Contains("%s")     ||
        encryptCmd.Contains("%s")     ||
        encryptSignCmd.Contains("%s") ||
        mimeDigSignCmd.Contains("%s") ||
        mimeEncryptCmd.Contains("%s") ||
        mimeEncryptSignCmd.Contains("%s") ) {
        
      StringC s = "In Ishmail 2.0, the command line options";
      s += " of the encryption scripts have changed.";
      s += "\nShall I apply new defaults for this session?";
      if ( Query(s, *ishApp, False) == QUERY_YES ) {
          digSignCmd		= DIGSIGNCMD_DEFAULT;
          encryptCmd		= ENCRYPTCMD_DEFAULT;
          encryptSignCmd	= ENCRYPTSIGNCMD_DEFAULT;
          mimeDigSignCmd	= MIMEDIGSIGNCMD_DEFAULT;
          mimeEncryptCmd	= MIMEENCRYPTCMD_DEFAULT;
          mimeEncryptSignCmd	= MIMEENCRYPTSIGNCMD_DEFAULT;
          
          s = "Good. You can save the new settings permanently using";
          s += " \"Options/Composition\"";
          
          ishApp->PopupMessage(s, XmDIALOG_INFORMATION);
      }
   }
} // End constructor

/*---------------------------------------------------------------
 *  destructor
 */

CompPrefC::~CompPrefC()
{
   delete prefWin;
}

/*---------------------------------------------------------------
 *  Method to write resources to resource database
 */

Boolean
CompPrefC::WriteDatabase()
{
   if (bodyCols         > 0) Store("composeColumns",	     bodyCols);
   if (bodyRows         > 0) Store("composeRows",	     bodyRows);
   if (headRows         > 0) Store("composeHeaderRows",	     headRows);
   if (maxFieldsPerLine > 0) Store("maxHeaderFieldsPerLine", maxFieldsPerLine);

   Store("composeWrapText",		wrap);
   Store("composeShowCc",		showCc);
   Store("composeShowBcc",		showBcc);
   Store("composeShowFcc",		showFcc);
   Store("composeShowOther",		showOther);
   Store("composeShowSettings",		showSettings);
   Store("emacsMode",			emacsMode);
   Store("deleteMeansBackspace",	delMeansBs);
   Store("composeSpaceEndsAddress",	spaceEndsAddr);
   Store("stripComments",		ishApp->replyPrefs->stripComments);
   Store("editCommand",			editCmd);
   Store("spellCommand",		spellCmd);
   Store("digitalSignCommand",		digSignCmd);
   Store("encryptCommand",		encryptCmd);
   Store("encryptAndSignCommand",	encryptSignCmd);
   Store("digitalSignMimeCommand",	mimeDigSignCmd);
   Store("encryptMimeCommand",		mimeEncryptCmd);
   Store("encryptAndSignMimeCommand",	mimeEncryptSignCmd);
   Store("composeAutoSave",		autoSave);
   Store("composeAutoSaveRate",		autoSaveRate);
   Store("composeAutoSaveDir",		orig.autoSaveDir);

   return True;

} // End WriteDatabase

/*---------------------------------------------------------------
 *  Method to write resources to a file
 */

Boolean
CompPrefC::WriteFile()
{
   StringListC	lineList;
   ReadResFile(lineList);

   if ( bodyCols > 0 ) Update(lineList, "composeColumns",	bodyCols);
   if ( bodyRows > 0 ) Update(lineList, "composeRows",		bodyRows);
   if ( headRows > 0 ) Update(lineList, "composeHeaderRows",	headRows);
   if ( maxFieldsPerLine > 0 )
      Update(lineList, "maxHeaderFieldsPerLine", maxFieldsPerLine);

   Update(lineList, "composeWrapText",		wrap);
   Update(lineList, "composeShowCc",		showCc);
   Update(lineList, "composeShowBcc",		showBcc);
   Update(lineList, "composeShowFcc",		showFcc);
   Update(lineList, "composeShowOther",		showOther);
   Update(lineList, "composeShowSettings",	showSettings);
   Update(lineList, "emacsMode",		emacsMode);
   Update(lineList, "deleteMeansBackspace",	delMeansBs);
   Update(lineList, "composeSpaceEndsAddress",	spaceEndsAddr);
   Update(lineList, "stripComments",	ishApp->replyPrefs->stripComments);
   Update(lineList, "editCommand",		editCmd);
   Update(lineList, "spellCommand",		spellCmd);
   Update(lineList, "digitalSignCommand",	digSignCmd);
   Update(lineList, "encryptCommand",		encryptCmd);
   Update(lineList, "encryptAndSignCommand",	encryptSignCmd);
   Update(lineList, "digitalSignMimeCommand",	mimeDigSignCmd);
   Update(lineList, "encryptMimeCommand",	mimeEncryptCmd);
   Update(lineList, "encryptAndSignMimeCommand",mimeEncryptSignCmd);
   Update(lineList, "composeAutoSave",		autoSave);
   Update(lineList, "composeAutoSaveRate",	autoSaveRate);
   Update(lineList, "composeAutoSaveDir",	orig.autoSaveDir);

//
// Write the information back to the file
//
   return WriteResFile(lineList);

} // End WriteFile

/*------------------------------------------------------------------------
 * Method to edit the preferences
 */

void
CompPrefC::Edit(Widget parent)
{
   halApp->BusyCursor(True);

   if ( !prefWin ) prefWin = new CompPrefWinC(parent);
   prefWin->Show(parent);

   halApp->BusyCursor(False);
}

/*------------------------------------------------------------------------
 * Method to set the auto-save directory name
 */

void
CompPrefC::SetAutoSaveDir(const char *name)
{
   if ( orig.autoSaveDir == name ) return;

   autoSaveDir = orig.autoSaveDir = name;
   ishApp->ExpandFolderName(autoSaveDir);
}

