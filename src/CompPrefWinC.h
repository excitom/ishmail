/*
 *  $Id: CompPrefWinC.h,v 1.1.1.1 2000/04/25 13:49:02 fnevgeny Exp $
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
#ifndef _CompPrefWinC_h_
#define _CompPrefWinC_h_

#include "OptWinC.h"

class RowColC;
class SendWinC;

class CompPrefWinC : public OptWinC {

//
// Widgets
//
   RowColC	*sizeRC;
   Widget	bodyColTF;
   Widget	bodyRowTF;
   Widget	headRowTF;
   Widget	wrapTB;
   Widget	showCcTB;
   Widget	showBccTB;
   Widget	showFccTB;
   Widget	showOtherTB;
   Widget	emacsTB;
   Widget	deleteTB;
   Widget	spaceTB;
   Widget	maxFields1TB;
   Widget	maxFields2TB;
   Widget	maxFields3TB;
   Widget	maxFields4TB;
   Widget	maxFields5TB;
   RowColC	*fieldRC;
   Widget	editorTF;
   Widget	spellTF;
   Widget	digSignTF;
   Widget	encryptTF;
   Widget	encryptSignTF;
   Widget	mimeDigSignTF;
   Widget	mimeEncryptTF;
   Widget	mimeEncryptSignTF;
   Widget	autoSaveTB;
   Widget	autoSaveRateTF;
   Widget	autoSaveDirTF;

//
// Data
//
   SendWinC	*sendWin;

//
// Private methods
//
   Boolean	Apply();

public:

// Methods

   CompPrefWinC(Widget);
   ~CompPrefWinC();

   void		Show(Widget);
   void		Show();

   MEMBER_QUERY(Widget, ColTF, bodyColTF);
   MEMBER_QUERY(Widget, RowTF, bodyRowTF);
};

#endif // _CompPrefWinC_h_
