/*
 *  $Id: HeadPrefWinC.h,v 1.1.1.1 2000/04/25 13:49:02 fnevgeny Exp $
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
#ifndef _HeadPrefWinC_h_
#define _HeadPrefWinC_h_

#include "OptWinC.h"

#include <hgl/ListBoxC.h>

class HeadPrefWinC : public OptWinC {

//
// Widgets
//
   Widget		listForm;
   Widget		choiceFrame;
   Widget		arrowForm;
   Widget		dListFrame;
   Widget		iListFrame;
   Widget		choiceDisplayTB;
   Widget		choiceIgnoreTB;
   ListBoxC		*dListBox;
   ListBoxC		*iListBox;
   Widget		dtoiAB;
   Widget		itodAB;

//
// Callbacks
//
   static void		ChangeDList(ListBoxC*, HeadPrefWinC*);
   static void		ChangeIList(ListBoxC*, HeadPrefWinC*);
   static void		ChangeListSelection(ListBoxC*, Widget);
   static void		DoPopup (Widget, HeadPrefWinC*, XtPointer);
   static void		HandleDrop(Widget,XtPointer, XmDropProcCallbackStruct*);
   static void		MoveDToI(Widget, HeadPrefWinC*, XtPointer);
   static void		MoveIToD(Widget, HeadPrefWinC*, XtPointer);
   static void		TransferText(Widget, Widget, Atom*, Atom*,
				     XtPointer, unsigned long*, int*);
   static void		VerifyHeader(ListBoxVerifyT*, HeadPrefWinC*);

//
// Private methods
//
   Boolean		Apply();

public:

// Methods

   HeadPrefWinC(Widget);
   ~HeadPrefWinC();
};

#endif // _HeadPrefWinC_h_
