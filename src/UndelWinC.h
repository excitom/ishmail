/*
 * $Id: UndelWinC.h,v 1.2 2000/08/07 11:05:17 evgeny Exp $
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

#ifndef _UndelWinC_h_
#define _UndelWinC_h_

#include <hgl/HalDialogC.h>

class TBoxC;
class FieldViewC;
class VBoxC;
class VItemC;
class VItemListC;

class UndelWinC : public HalDialogC {

public:

//
// Widgets
//
   Widget		selectPB;
   Widget		deselectPB;
   Widget		okPB;
   Widget		applyPB;

   TBoxC		*msgBox;
   FieldViewC		*fieldView;

//
// Data
//
   int			viewType;

//
// Callbacks
//
   static void		ChangeSelection(VBoxC*, UndelWinC*);

   static void		DoApply   (Widget, UndelWinC*, XtPointer);
   static void		DoCancel  (Widget, UndelWinC*, XtPointer);
   static void		DoDeselect(Widget, UndelWinC*, XtPointer);
   static void		DoOk      (Widget, UndelWinC*, XtPointer);
   static void		DoPopup   (Widget, UndelWinC*, XtPointer);
   static void		DoSelect  (Widget, UndelWinC*, XtPointer);

//
// Methods
//
   UndelWinC(Widget);
   ~UndelWinC();

   void		AddItem(VItemC&);
   void		Clear();
   void		SetItems(VItemListC&);
   void		UpdateFields();
};

#endif // _UndelWinC_h_
