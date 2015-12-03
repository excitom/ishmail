/*
 *  $Id: MailPrefWinC.h,v 1.4 2000/06/19 13:32:18 evgeny Exp $
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
#ifndef _MailPrefWinC_h_
#define _MailPrefWinC_h_

#include "OptWinC.h"

class RowColC;

class MailPrefWinC : public OptWinC {

//
// Widgets
//
   RowColC	*checkRC;
   Widget	checkAddrTB;
   Widget	splitTB;
   Widget	splitTF;
   Widget	noFccTB;
   Widget	fccFolderTB;
   Widget	fccFolderTF;
   Widget	fccPatternTB;
   Widget	fccPatternTF;
   Widget	fccPatternOM;
   Widget	fccUserPB;
   Widget	fccAddrPB;
   Widget	fccYearPB;
   Widget	fccMonthPB;
   Widget	fccWeekPB;
   Widget	fccDayPB;
   RowColC	*fieldRC;
   Widget	fromHeadTF;
   Widget	mailPlainTB;
   Widget	mailMimeTB;
   Widget	mailAltTB;
   Widget	textPlainTB;
   Widget	textRichTB;
   Widget	charTF;
   Widget	charAsciiPB;
   Widget	charIso1PB;
   Widget	charIso2PB;
   Widget	charIso3PB;
   Widget	charIso4PB;
   Widget	charIso5PB;
   Widget	charIso6PB;
   Widget	charIso7PB;
   Widget	charIso8PB;
   Widget	charIso9PB;
   Widget	charIso13PB;
   Widget	bodyEnc8bitTB;
   Widget	bodyEncQpTB;
   Widget	headEncNoneTB;
   Widget	headEncQTB;
   Widget	headEncBTB;
   Widget	deadTB;
   Widget	deadTF;
   Widget	sendmailTF;
   Widget	confirmAddrTB;
   Widget	confirmAddrText;
   Widget	otherHeadText;

   static void	AutoSelectFolder (Widget, MailPrefWinC*, XtPointer);
   static void	AutoSelectPattern(Widget, MailPrefWinC*, XtPointer);
   static void	DoCharset        (Widget, MailPrefWinC*, XtPointer);
   static void	DoPopup          (Widget, MailPrefWinC*, XtPointer);
   static void	FccChanged       (Widget, MailPrefWinC*,
   				  XmToggleButtonCallbackStruct*);

//
// Private methods
//
   Boolean	Apply();

public:

// Methods

   MailPrefWinC(Widget);
   ~MailPrefWinC();

   void		Show(Widget);
   void		Show();
};

#endif // _MailPrefWinC_h_
