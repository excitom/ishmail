/*
 * $Id: LoginWinC.h,v 1.1.1.1 2000/04/25 13:49:02 fnevgeny Exp $
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

#ifndef _LoginWinC_h_
#define _LoginWinC_h_

#include <hgl/HalDialogC.h>
#include <hgl/StringC.h>

class CharC;
class RowColC;

class LoginWinC : public HalDialogC {

//
// Widgets
//
   Widget		hostLabel;
   Widget		nameTF;
   Widget		passTF;
   RowColC		*inputRC;

//
// Data
//
   StringC		login;
   StringC		shadow;
   int			reason;

//
// Callbacks
//
   static void		DoCancel (Widget, LoginWinC*, XtPointer);
   static void		DoOk     (Widget, LoginWinC*, XtPointer);
   static void		NextField(Widget, LoginWinC*, XtPointer);
   static void		ProcessPassword(Widget, LoginWinC*,
					XmTextVerifyCallbackStruct*);

public:

// Methods

   LoginWinC(Widget, char *name="loginWin", ArgList argv=NULL, Cardinal argc=0);
   ~LoginWinC();

   Boolean		GetLogin   (const CharC& host, StringC&, StringC&);
   Boolean		GetPassword(StringC&);
};

extern Boolean		GetLogin       (CharC, StringC&, StringC&);
extern void		InvalidateLogin(CharC);

#endif // _LoginWinC_h_
