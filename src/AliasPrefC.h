/*
 *  $Id: AliasPrefC.h,v 1.1.1.1 2000/04/25 13:49:02 fnevgeny Exp $
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
#ifndef _AliasPrefC_h
#define _AliasPrefC_h

#include "PrefC.h"

#include <hgl/StringListC.h>
#include <hgl/StringDictC.h>

class RegexC;
class StringDictC;

class AliasPrefWinC;

class AliasPrefC : public PrefC {

//
// Private data.  We don't want anyone setting these directly.
//
   RegexC		*aliasPat;	// pattern for mailrc aliases
   RegexC		*elmAliasPat;	// pattern for elm aliases
   RegexC		*pineAliasPat;	// pattern for pine aliases
   RegexC		*namePat;	// For mailrc format conversion
   AliasPrefWinC	*aliasWin;
   AliasPrefWinC	*groupWin;

//
// As typed by the user
//
   struct {
      StringC		mailrcFile;		// Name of .mailrc file
      StringC		groupMailrcFile;	// Name of group mailrc file
      StringC		otherAliasStr;		// extra places to look
   } orig;

//
// Fully expanded
//
   StringC		mailrcFile;		// Name of .mailrc file
   StringC		groupMailrcFile;	// Name of group mailrc file
   StringListC		otherAliasFiles;	// extra places to look

//
// Private methods
//
   void			AddAliasCommas(StringC&);
   void			ConvertAliasToMailrc(StringC&);
   void			ConvertMailrcToAlias(StringC&);
   void			ReadAliasFile(const char*, StringDictC&, Boolean);
   void			UpdateAliases (StringListC&, StringDictC&);
   Boolean		WriteAliases(const char*, StringDictC&);

public:

//
// Public data
//
   StringDictC		aliasDict;	// alias -> expansion
   StringDictC		groupAliasDict;	// alias -> expansion
   StringDictC		otherAliasDict;	// alias -> expansion
   Boolean		sortAliases;	// If sorted alphabetically
   StringListC		hideAddrList;	// Not to be expanded

//
// Public methods
//
    AliasPrefC();
   ~AliasPrefC();

   void		SetMailrcFile(const char*);
   void		SetGroupMailrcFile(const char*);
   void		SetOtherAliasFiles(const char*);

   StringC&	MailrcFile()		{ return mailrcFile; }
   StringC&	GroupMailrcFile()	{ return groupMailrcFile; }
   StringListC&	OtherAliasFiles()	{ return otherAliasFiles; }

   StringC&	OrigMailrcFile()	{ return orig.mailrcFile; }
   StringC&	OrigGroupMailrcFile()	{ return orig.groupMailrcFile; }
   StringC&	OrigOtherAliasFiles()	{ return orig.otherAliasStr; }

   StringC	ExpandAddress(StringC&, int, Boolean expandHidden=True);
   void		Edit(Widget);
   void		EditGroup(Widget);
   Boolean	WriteDatabase();
   Boolean	WriteFile();
};

#endif // _AliasPrefC_h
