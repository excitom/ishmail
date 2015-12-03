/*
 * $Id: DictEditWinC.h,v 1.1.1.1 2000/04/25 13:49:02 fnevgeny Exp $
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

#ifndef _DictEditWinC_h_
#define _DictEditWinC_h_

#include "OptWinC.h"

#include <hgl/VItemListC.h>

class FieldViewC;
class TBoxC;
class VBoxC;
class EntryEditWinC;

class DictEditWinC : public OptWinC {

protected:

   enum EditAction {
      NONE,
      ADD,
      MODIFY,
      INSERT
   };

//
// Widgets
//
   TBoxC		*taskBox;
   VBoxC		*entryBox;
   FieldViewC		*fieldView;
   Widget		addPB;
   Widget		removePB;
   Widget		modifyPB;
   Widget		insertPB;
   EntryEditWinC	*entryEditWin;

//
// Data
//
   int			viewType;
   VItemListC		itemList;
   VItemListC		addList;
   VItemListC		modList;
   VItemListC		remList;
   StringListC		keyList;
   VItemC		*selectedItem;
   EditAction		lastAction;

//
// Callbacks
//
   static void		AnswerDelete(Widget, int*, XmAnyCallbackStruct*);
   static void		ChangeSelection(VBoxC*, DictEditWinC*);
   static void		OpenEntry(VItemC*, DictEditWinC*);

   static void		DoAdd      (Widget, DictEditWinC*, XtPointer);
   static void		DoInsert   (Widget, DictEditWinC*, XtPointer);
   static void		DoModify   (Widget, DictEditWinC*, XtPointer);
   static void		DoRemove   (Widget, DictEditWinC*, XtPointer);
   static void		FinishAdd   (void*, DictEditWinC*);
   static void		FinishInsert(void*, DictEditWinC*);
   static void		FinishModify(void*, DictEditWinC*);

//
// Private methods
//
   void			AddItem(int pos);
   virtual Boolean	Apply() = 0;
   virtual void		Cancel() = 0;
   Boolean		ConfirmDelete(Widget);
   void			UpdateButtons();

public:

// Methods

   DictEditWinC(Widget, const char*);
   ~DictEditWinC();

   MEMBER_QUERY(VBoxC&,	EntryBox,	*entryBox);
   MEMBER_QUERY(TBoxC&,	TaskBox,	*taskBox);
};

#endif // _DictEditWinC_h_
