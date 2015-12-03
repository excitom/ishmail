/*
 *  $Id: PickAliasWinC.h,v 1.1.1.1 2000/04/25 13:49:02 fnevgeny Exp $
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

#ifndef _PickAliasWinC_h_
#define _PickAliasWinC_h_

#include <hgl/HalDialogC.h>
#include <hgl/StringListC.h>
#include <hgl/CallbackC.h>
#include <hgl/VItemListC.h>

class VBoxC;
class FieldViewC;
class StringDictC;

class PickAliasWinC : public HalDialogC {

//
// Widgets
//
   Widget		titleLabel;
   VBoxC		*aliasBox;
   FieldViewC		*fieldView;
   Widget		findTF;
   Widget		okPB;
   Widget		applyPB;
   Widget		expandTB;

//
// Data
//
   int			viewType;
   StringListC		aliasList;
   VItemListC		itemList;
   CallbackC		applyCall;
   int			findIndex;

//
// Callbacks
//
   static void		ChangeSelection(VBoxC*,  PickAliasWinC*);
   static void		PickAlias      (VItemC*, PickAliasWinC*);
   static int		AliasCompare   (const void*, const void*);

   static void		DoApply   (Widget, PickAliasWinC*, XtPointer);
   static void		DoOk      (Widget, PickAliasWinC*, XtPointer);
   static void		DoPopup   (Widget, PickAliasWinC*, XtPointer);
   static void		DoFindNext(Widget, PickAliasWinC*, XtPointer);
   static void		DoFindPrev(Widget, PickAliasWinC*, XtPointer);
   static void		DoFindAll (Widget, PickAliasWinC*, XtPointer);

//
// Private methods
//
   void			ProcessDict(StringDictC&, int);

public:

//
// Methods
//
   PickAliasWinC(Widget);
   ~PickAliasWinC();

   const StringListC&	AliasList() const { return aliasList; }
   void			Show(const char*);
   void			Show() { HalDialogC::Show(); }
   void			SetApplyCallback(CallbackFn*, void*);
};

#endif // _PickAliasWinC_h_
