/*
 *  $Id: RichSearchWinC.h,v 1.1.1.1 2000/04/25 13:49:01 fnevgeny Exp $
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
#ifndef _RichSearchWinC_h_
#define _RichSearchWinC_h_

#include "HalDialogC.h"

/*--------------------------------------------------------------------------
 * Text search window class
 */

class MimeRichTextP;
class TextLineC;
class TextPosC;
class RegexC;
class CharC;

class RichSearchWinC : public HalDialogC {

   Widget			stringTF;
   Widget			caseTB;
   Widget			backTB;
   Widget			wildTB;

   MimeRichTextP		*rich;

   static void	DoFind    (Widget, RichSearchWinC*, XtPointer);
   static void	ToggleCase(Widget, RichSearchWinC*, XtPointer);
   static void	ToggleWild(Widget, RichSearchWinC*, XtPointer);

//
// Private methods
//
   Boolean	SearchLine   (TextLineC*, RegexC&, Boolean);
   Boolean	SearchLine   (TextLineC*, CharC&,  Boolean, Boolean);
   Boolean	RevSearchLine(TextLineC*, RegexC&, Boolean);
   Boolean	RevSearchLine(TextLineC*, CharC&,  Boolean, Boolean);
   void		UpdateSelection(TextPosC&, TextPosC&, TextPosC&);

public:

// Methods

   RichSearchWinC(Widget, const char*, ArgList argv=NULL, Cardinal argc=0);

   void		Show(MimeRichTextP*);
   void		Show(Widget);
   void		Show();
};

#endif // _RichSearchWinC_h_
