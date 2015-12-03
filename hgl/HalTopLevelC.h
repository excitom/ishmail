/*
 * $Id: HalTopLevelC.h,v 1.1.1.1 2000/04/25 13:49:01 fnevgeny Exp $
 *
 * Copyright (c) 1993 HAL Computer Systems International, Ltd.
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

#ifndef _HalTopLevelC_h_
#define _HalTopLevelC_h_

#include "HalShellC.h"

class HalTopLevelC : public HalShellC {

protected:

   Widget	topLevel;
   Widget	menuBar;
   Widget	appFrame;

   Widget	fileCB;
   Widget	filePD;
   Widget	fileExitPB;

   Widget	helpCB;
   Widget	helpPD;
   Widget	helpContextPB;
   Widget	helpWindowPB;
   Widget	helpAppPB;
   Widget	helpIndexPB;
   Widget	helpHelpPB;
   Widget	helpKeysPB;
   Widget	helpVerPB;

   Widget	exitWin;

   static void	FinishExit(Widget, HalTopLevelC*, XtPointer);

public:

   HalTopLevelC(const char *name, Widget parent);
   ~HalTopLevelC();

//
// Add components
//
   void		AddMenuBar();
   void		AddHelpMenu();
   void		AddExitButton();

//
// Public callbacks.
//
   static void	DoExit    (Widget, HalTopLevelC*, XtPointer);

//
// Return widgets
//

   MEMBER_QUERY(Widget, AppFrame,		appFrame)
   MEMBER_QUERY(Widget, FileButton,		fileCB)
   MEMBER_QUERY(Widget, FilePulldown,		filePD)
   MEMBER_QUERY(Widget, FileExitButton,		fileExitPB)
   MEMBER_QUERY(Widget, HelpButton,		helpCB)
   MEMBER_QUERY(Widget, HelpPulldown,		helpPD)
   MEMBER_QUERY(Widget, HelpContextButton,	helpContextPB)
   MEMBER_QUERY(Widget, HelpWindowButton,	helpWindowPB)
   MEMBER_QUERY(Widget, HelpAppButton,		helpAppPB)
   MEMBER_QUERY(Widget, HelpIndexButton,	helpIndexPB)
   MEMBER_QUERY(Widget, HelpHelpButton,		helpHelpPB)
   MEMBER_QUERY(Widget, HelpKeysButton,		helpKeysPB)
   MEMBER_QUERY(Widget, HelpVerButton,		helpVerPB)
   MEMBER_QUERY(Widget, MenuBar,		menuBar)
   MEMBER_QUERY(Widget, TopLevel,		topLevel)
};

#endif // _HalTopLevelC_h_
