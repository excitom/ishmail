/*
 * $Id: ButtonEditWinC.h,v 1.1.1.1 2000/04/25 13:49:02 fnevgeny Exp $
 *
 * Copyright (c) 1994 HAL Computer Systems International, Ltd.
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

#ifndef _ButtonEditWinC_h_
#define _ButtonEditWinC_h_

#include <hgl/HalDialogC.h>
#include <hgl/StringC.h>
#include <hgl/CallbackC.h>

#include <Xm/Xm.h>

class ButtonEntryC;

class ButtonEditWinC : public HalDialogC {

protected:

//
// Widgets
//
   Widget		labelTF;
   Widget		abbrevTF;
   Widget		orderTF;

   ButtonEntryC		*entry;
   CallbackListC	applyCalls;

   static void		DoApply      (Widget, ButtonEditWinC*, XtPointer);
   static void		DoOk         (Widget, ButtonEditWinC*, XtPointer);

//
// Private methods
//
   Boolean		Apply();

public:

// Methods

   ButtonEditWinC(Widget);

//
// Register callbacks
//
   inline void  AddApplyCallback(CallbackFn *fn, void *data=NULL) {
      AddCallback(applyCalls, fn, data);
   }
   inline void  RemoveApplyCallback(CallbackFn *fn, void *data=NULL) {
      RemoveCallback(applyCalls, fn, data);
   }
   inline void  CallApplyCallbacks() { CallCallbacks(applyCalls, this); }

   inline void	Show() { HalDialogC::Show(); }
   void		Show(ButtonEntryC*);

   MEMBER_QUERY(ButtonEntryC*, Entry, entry)
};

#endif // _ButtonEditWinC_h_
