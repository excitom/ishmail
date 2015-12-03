/*
 *  $Id: ReplyPrefWinC.h,v 1.2 2000/06/05 14:46:48 evgeny Exp $
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
#ifndef _ReplyPrefWinC_h_
#define _ReplyPrefWinC_h_

#include "OptWinC.h"

class RowColC;

class ReplyPrefWinC : public OptWinC {

//
// Widgets
//
   RowColC		*checkRC;
   Widget		removeMeTB;
   Widget		numberTB;
   Widget		stripTB;
   RowColC		*fieldRC;
   Widget		indentTF;
   Widget		attribText;
   Widget		begForwardText;
   Widget		endForwardText;
   Widget		forwardAllTB;
   Widget		forwardDispTB;
   Widget		forwardNoneTB;

   static void		DoPopup(Widget, ReplyPrefWinC*, XtPointer);

//
// Private methods
//
   Boolean	Apply();

public:

// Methods

   ReplyPrefWinC(Widget);
   ~ReplyPrefWinC();

   void		Show(Widget);
   void		Show();
};

#endif // _ReplyPrefWinC_h_
