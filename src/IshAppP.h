/*
 *  $Id: IshAppP.h,v 1.2 2000/04/25 15:46:46 fnevgeny Exp $
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
#ifndef _IshAppP_h
#define _IshAppP_h

#include <hgl/StringListC.h>

#include <X11/Intrinsic.h>

#include <stdio.h>
#include <time.h>

class IshAppC;
class CharC;
class StringC;
class PixmapC;
class ReadWinC;

class IshAppP {

   friend class IshAppC;

   IshAppC	*pub;	// Public data and methods

//
// Private data
//
   PixmapC	*messagePM;
   PixmapC	*warningPM;
   PixmapC	*questionPM;
   PixmapC	*errorPM;
   XtIntervalId	checkTimer;
   time_t	lastMailCheck;
   StringListC	compFileList;

//
// Callbacks
//
   static void	CheckForNewMail    (IshAppP*, XtIntervalId*);
   static void	PreventClose       (Widget, Widget,   XtPointer);
   static void	FinishEditSavedComp(Widget, IshAppP*, XtPointer);
   static void	CancelEditSavedComp(Widget, IshAppP*, XtPointer);
   static void	DeleteSavedComp    (Widget, IshAppP*, XtPointer);

//
// Private methods
//
   StringC	*FindLine(StringListC&, CharC);
   Boolean	HasDuplicateLines(CharC);

public:

//
// Public methods
//
    IshAppP(IshAppC*);
   ~IshAppP();

   void		CreatePixmaps();
   void		FixIshmailrc();
   ReadWinC	*GetReadWin();
   void		GetResLines(CharC, StringListC&, Boolean removeDups=False);
   void		QueryImap();
   void		QueryPop();
   void		QueryFolderType();
   Boolean	WriteResFile(StringListC&);
   Boolean	WriteRules(FILE*, StringC&);
   void		EditSavedCompositions();
};

#endif // _IshAppP_h
