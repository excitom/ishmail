/*
 *  $Id: CompPrefC.h,v 1.1.1.1 2000/04/25 13:49:02 fnevgeny Exp $
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
#ifndef _CompPrefC_h
#define _CompPrefC_h

#include "PrefC.h"

class CompPrefWinC;

class CompPrefC : public PrefC {

   CompPrefWinC		*prefWin;

//
// As typed by the user
//
   struct {
      StringC	autoSaveDir;
   } orig;

//
// Fully expanded
//
   StringC	autoSaveDir;

public:

//
// Public data
//
   int			bodyCols;
   int			bodyRows;
   int			headRows;
   int			maxFieldsPerLine;
   Boolean		wrap;
   Boolean		showCc;		// Show Cc header
   Boolean		showBcc;	// Show Bcc header
   Boolean		showFcc;	// Show Fcc header
   Boolean		showOther;	// Show Other headers
   Boolean		showSettings;	// Show Settings panel
   Boolean		emacsMode;
   Boolean		delMeansBs;
   Boolean		spaceEndsAddr;
   StringC		editCmd;
   StringC		spellCmd;
   StringC		digSignCmd;
   StringC		encryptCmd;
   StringC		encryptSignCmd;
   StringC		mimeDigSignCmd;
   StringC		mimeEncryptCmd;
   StringC		mimeEncryptSignCmd;
   Boolean		autoSave;
   int			autoSaveRate;

//
// Public methods
//
    CompPrefC();
   ~CompPrefC();

   void		SetAutoSaveDir(const char*);
   StringC&	AutoSaveDir()		{ return autoSaveDir; }
   StringC&	OrigAutoSaveDir()	{ return orig.autoSaveDir; }

   void		Edit(Widget);
   Boolean	WriteDatabase();
   Boolean	WriteFile();
};

#endif // _CompPrefC_h
