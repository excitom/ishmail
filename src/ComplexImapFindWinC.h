/*
 *  $Id: ComplexImapFindWinC.h,v 1.1.1.1 2000/04/25 13:49:02 fnevgeny Exp $
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
#ifndef _ComplexImapFindWinC_h_
#define _ComplexImapFindWinC_h_

#include <hgl/HalDialogC.h>
#include <hgl/IntListC.h>

class StringC;

/*--------------------------------------------------------------------------
 * Find window class
 */

class ComplexImapFindWinC : public HalDialogC {

   int		findIndex;
   Boolean	stateChanged;
   IntListC	numList;

   Widget	toTB;
   Widget	ccTB;
   Widget	bccTB;
   Widget	fromTB;
   Widget	subjTB;
   Widget	bodyTB;
   Widget	msgTB;

   Widget	toTF;
   Widget	ccTF;
   Widget	bccTF;
   Widget	fromTF;
   Widget	subjTF;
   Widget	bodyTF;
   Widget	msgTF;

   Widget	befDateTB;
   Widget	onDateTB;
   Widget	aftDateTB;

   Widget	befDateTF;
   Widget	onDateTF;
   Widget	aftDateTF;

   Widget	recTB;
   Widget	notRecTB;
   Widget	seenTB;
   Widget	notSeenTB;
   Widget	repTB;
   Widget	notRepTB;
   Widget	delTB;
   Widget	notDelTB;

   static void	AutoSelect  (Widget, Widget, XtPointer);
   static void	AutoDeselect(Widget, Widget, XmToggleButtonCallbackStruct*);
   static void	DoFindNext  (Widget, ComplexImapFindWinC*, XtPointer);
   static void	DoFindPrev  (Widget, ComplexImapFindWinC*, XtPointer);
   static void	DoFindAll   (Widget, ComplexImapFindWinC*, XtPointer);
   static void	DoClear     (Widget, ComplexImapFindWinC*, XtPointer);
   static void	StateChanged(Widget, ComplexImapFindWinC*, XtPointer);

   Boolean	AddTerm(StringC&, const char*, Widget, Widget);
   Boolean	AddTerm(StringC&, const char*, Widget);
   Boolean	GetNumList();

public:

// Methods

   ComplexImapFindWinC(Widget);

   void		Show();
};

#endif // _ComplexImapFindWinC_h_
