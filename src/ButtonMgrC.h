/*
 * $Id: ButtonMgrC.h,v 1.1.1.1 2000/04/25 13:49:02 fnevgeny Exp $
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

#ifndef _ButtonMgrC_h_
#define _ButtonMgrC_h_

#include <hgl/StringC.h>

#include <X11/Intrinsic.h>

class HalShellC;
class TBoxC;
class WArgList;

class ButtonMgrC {

protected:

   HalShellC		*shell;
   TBoxC		*taskBox;
   Widget		menuBar;
   Widget		buttonBox;
   StringC		resPrefix;

//
// Callbacks
//
   static void		DoActivate(Widget, Widget, XtPointer);

public:

// Methods

   ButtonMgrC(HalShellC*, Widget, Widget, char*, TBoxC *tb=NULL);
   ~ButtonMgrC();

   Widget		AddButton(char*, WArgList&);
   Widget		ButtonFor(Widget);
   void			EnableButtons();
   int			Gravity();
   void			ManageButtons();
   void			RemoveButtons(WidgetList, Cardinal);
   void			SensitivityChanged(Widget);
   void			SetGravity(int);

   MEMBER_QUERY(Widget,		ButtonBox,	buttonBox);
   MEMBER_QUERY(Widget,		MenuBar,	menuBar);
      PTR_QUERY(StringC&,	ResPrefix,	resPrefix);
      PTR_QUERY(HalShellC&,	Shell,		*shell);
};

#endif // _ButtonMgrC_h_
