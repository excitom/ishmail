/*
 *  $Id: AppPrefWinC.h,v 1.2 2000/06/29 10:53:29 evgeny Exp $
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
#ifndef _AppPrefWinC_h_
#define _AppPrefWinC_h_

#include "OptWinC.h"

class StringC;
class RowColC;

class AppPrefWinC : public OptWinC {

//
// Widgets
//
   RowColC	*checkRC;
   RowColC	*fieldRC;
   Widget	inBoxTF;
   Widget	checkTF;
   Widget	alertTB;
   Widget	showNewTB;
   Widget	delSaveTB;
   Widget	hideDelTB;
   Widget	quickHelpTB;
   Widget	windowPosTB;
   Widget	newUnreadTB;
   Widget	archiveTB;
   Widget	archiveTF;
   Widget	recentTF;
   Widget	folderTF;
   Widget	imapTB;
   Widget	imapTF;
   Widget	popTB;
   Widget	popTF;
   Widget	popCmdTF;
   Widget	saveFolderTB;
   Widget	saveFolderTF;
   Widget	savePatternTB;
   Widget	savePatternOM;
   Widget	savePatternTF;
   Widget	saveUserPB;
   Widget	saveAddrPB;
   Widget	savePatternPB;
   Widget	savePatUserPB;
   Widget	savePatAddrPB;
   Widget	trashTF;
   Widget	autoTF;
   Widget	printTF;
   Widget	bellScale;

//
// Callbacks
//
   static void	DoPopup          (Widget, AppPrefWinC*, XtPointer);
   static void	AutoSelectFolder (Widget, AppPrefWinC*, XtPointer);
   static void	AutoSelectPattern(Widget, AppPrefWinC*, XtPointer);
   static void	EditPatterns     (Widget, AppPrefWinC*, XtPointer);

   static void	SaveChanged(Widget, AppPrefWinC*,
				    XmToggleButtonCallbackStruct*);

//
// Private methods
//
   Boolean		Apply();

public:

// Methods

   AppPrefWinC(Widget);
   ~AppPrefWinC();

   void			Show();
};

#endif // _AppPrefWinC_h_
