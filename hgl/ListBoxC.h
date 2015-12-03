/*
 * $Id: ListBoxC.h,v 1.1.1.1 2000/04/25 13:49:01 fnevgeny Exp $
 *
 * Copyright (c) 1992 HaL Computer Systems, Inc.  All rights reserved.
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

#ifndef _ListBoxC_h_
#define _ListBoxC_h_

#include "WArgList.h"
#include "StringListC.h"
#include "CallbackC.h"

class HalShellC;
class Help;

class ListBoxC {

   Widget		mainForm;
   Widget		buttonBox;
   Widget		itemText;
   Widget		addButton;
   Widget		remButton;
   Widget		repButton;
   Widget		clrButton;
   Widget		itemWin;
   Widget		itemList;
   Widget		removeWin;
   Widget		clearWin;

   StringListC		items;
   int			itemCount;	// Number of items in list
   Boolean		sorted;
   Boolean		addEnabled;
   Boolean		dupOk;		// Allow duplicates
   int			selectCount;	// Number of selected items
   int			textLen;	// Length of text in field
   Boolean		textUnique;	// Text in field not already in list

//
// Defer callbacks so they don't get called too often.
//
   Boolean		selectChange;
   Boolean		itemChange;
   int			deferCallbacks;
   void			DeferCallbacks(Boolean);
   CallbackListC	selectChangeCalls;
   CallbackListC	itemChangeCalls;
   CallbackListC	verifyCalls;

//
// Callback functions
//
   static void		DoClear      (Widget, ListBoxC*, XtPointer);
   static void		DoItemAdd    (Widget, ListBoxC*, XtPointer);
   static void		DoItemOpen   (Widget, ListBoxC*, XmListCallbackStruct*);
   static void		DoItemReplace(Widget, ListBoxC*, XtPointer);
   static void		DoRemove     (Widget, ListBoxC*, XtPointer);
   static void		DoSelect     (Widget, ListBoxC*, XmListCallbackStruct*);
   static void		DoTextChanged(Widget, ListBoxC*, XtPointer);
   static void		DoTextReturn (Widget, ListBoxC*, XtPointer);
   static void		FinishClear  (Widget, ListBoxC*, XtPointer);
   static void		FinishRemove (Widget, ListBoxC*, XtPointer);

//
// Private methods
//
   void			AddItem(Boolean);
   void			MakeVisible(int);
   void			UpdateButtons();	// Set sensitivities
   Boolean		VerifyClear();
   Boolean		VerifyRemove();

public:

// Methods

   ListBoxC(Widget parent, const char *name, ArgList argv=NULL,Cardinal argc=0);

//
// Cast to widget
//
   inline operator	Widget() const		{ return mainForm;	}

//
// User callbacks
//
   inline void	AddSelectChangeCallback(CallbackFn *fn, void *data) {
      AddCallback(selectChangeCalls, fn, data);
   }
   inline void	CallSelectChangeCallbacks() {
      CallCallbacks(selectChangeCalls, this);
   }
   inline void	RemoveSelectChangeCallback(CallbackFn *fn, void *data) {
      RemoveCallback(selectChangeCalls, fn, data);
   }
   inline void	AddItemChangeCallback(CallbackFn *fn, void *data) {
      AddCallback(itemChangeCalls, fn, data);
   }
   inline void	CallItemChangeCallbacks() {
      CallCallbacks(itemChangeCalls, this);
   }
   inline void	RemoveItemChangeCallback(CallbackFn *fn, void *data) {
      RemoveCallback(itemChangeCalls, fn, data);
   }
   inline void	AddVerifyCallback(CallbackFn *fn, void *data) {
      AddCallback(verifyCalls, fn, data);
   }
   inline void	RemoveVerifyCallback(CallbackFn *fn, void *data) {
      RemoveCallback(verifyCalls, fn, data);
   }

   void			AddItem(StringC);
   void			AddItems(const StringListC&);
   inline void		AllowDuplicates(Boolean val)
	{ dupOk = val; items.AllowDuplicates(val); }
   void			Clear();
   StringListC&		GetItems(StringListC&);
   StringListC&		GetSelectedItems(StringListC&);
   void			HandleHelp(HalShellC*, Help*);
   void			RemoveItem(StringC);
   void			RemoveItems(const StringListC&);
   void			RemoveSelected();
   void			SetItems(const StringListC&);
   inline void		SetSorted(Boolean val)
	{ sorted = val; items.SetSorted(val); }

   MEMBER_QUERY(Widget,		MainForm,	mainForm)
   MEMBER_QUERY(Widget,		ButtonBox,	buttonBox)
   MEMBER_QUERY(Widget,		ItemText,	itemText)
   MEMBER_QUERY(Widget,		AddButton,	addButton)
   MEMBER_QUERY(Widget,		RemButton,	remButton)
   MEMBER_QUERY(Widget,		RepButton,	repButton)
   MEMBER_QUERY(Widget,		ClearButton,	clrButton)
   MEMBER_QUERY(Widget,		ItemWin,	itemWin)
   MEMBER_QUERY(Widget,		ItemList,	itemList)
      PTR_QUERY(StringListC&,	Items,		items)
   MEMBER_QUERY(int,		SelectCount,	selectCount)
};

//
// Data type used in verify callbacks.  Used to verify item added to list
//
typedef struct {

   ListBoxC	*listBox;
   StringC	*text;
   Boolean	ok;

} ListBoxVerifyT;

#endif // _ListBoxC_h_
