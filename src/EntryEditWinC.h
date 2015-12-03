/*
 * $Id: EntryEditWinC.h,v 1.2 2000/06/05 13:28:02 evgeny Exp $
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

#ifndef _EntryEditWinC_h_
#define _EntryEditWinC_h_

#include <Xm/Xm.h>

#include <hgl/HalDialogC.h>
#include <hgl/StringC.h>
#include <hgl/CallbackC.h>

class RowColC;

class EntryEditWinC : public HalDialogC {

protected:

//
// Widgets
//
   Widget		keyTF;
   Widget		valueText;
   RowColC		*entryRC;

   StringC		keyString;
   StringC		valueString;
   CallbackC		applyCallback;
   Boolean		aliasMode;

   static void		DoApply      (Widget, EntryEditWinC*, XtPointer);
   static void		DoOk         (Widget, EntryEditWinC*, XtPointer);

//
// Private methods
//
   Boolean		Apply();

public:

// Methods

   EntryEditWinC(Widget, const char*);
   ~EntryEditWinC();

   void		EnableAliases();
   void		SetKey(const char*);
   void		SetValue(const char*);
   inline void	Show() { HalDialogC::Show(); }
   void		Show(CallbackFn*, void*);

      PTR_QUERY(StringC&,	KeyString,	keyString);
      PTR_QUERY(StringC&,	ValueString,	valueString);
      MEMBER_QUERY(Widget,	KeyTF,		keyTF);
      PTR_QUERY(Widget,		ValueText,	valueText);
};

#endif // _EntryEditWinC_h_
