/*
 * $Id: SortExpWinC.h,v 1.1.1.1 2000/04/25 13:49:01 fnevgeny Exp $
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

#ifndef _SortExpWinC_h_
#define _SortExpWinC_h_

#include "HalDialogC.h"
#include "CallbackListC.h"
#include "SortElemListC.h"
#include "SortKeyListC.h"

class SortKeyC;

//----------------------------------------------------------------------

class SortExpWinC : public HalDialogC {

protected:

   Widget		expForm;
   Widget		expDA;
   Widget		expWin;
   Widget		clipWin;
   Widget		cornerDA;
   Widget		cursorLabel;
   Widget		applyQueryWin;

   SortElemListC	expList;
   SortKeyListC		keyList;
   int			insertPos;
   Boolean		resizeOk;
   CallbackListC	applyCalls;
   Boolean		applyAll;
   void			(*writeFunc)(SortExpWinC*);

   int			blinkInterval;
   XtIntervalId		blinkTimer;
   Boolean		cursorOn;
   Dimension		cursorWd,  cursorHt;
   Dimension		borWd;
   Dimension		marWd,  marHt;
   Dimension		clipWd, clipHt;

   static XtTranslations	translations;
   static XtActionsRec		actions[5];

   static void	BlinkProc(SortExpWinC*, XtIntervalId*);

//
// Callbacks
//
   static void	DoApply (Widget, SortExpWinC*, XtPointer);
   static void	DoCancel(Widget, SortExpWinC*, XtPointer);
   static void	DoClear (Widget, SortExpWinC*, XtPointer);
   static void	DoOk    (Widget, SortExpWinC*, XtPointer);

//
// Event handlers
//
   static void	HandleFocusChange(Widget, SortExpWinC*, XEvent*, Boolean*);
   static void	HandleResize     (Widget, SortExpWinC*, XEvent*, Boolean*);
   static void	WaitForAnswer    (Widget, int*, XmAnyCallbackStruct*);
   static void	SetApplyAll      (Widget, SortExpWinC*,
				  XmToggleButtonCallbackStruct*);
   static void	SetApplyCurrent  (Widget, SortExpWinC*,
				  XmToggleButtonCallbackStruct*);

//
// Action procs
//
   static void	HandleDeleteLeft  (Widget, XKeyEvent*,    String*, Cardinal*);
   static void	HandleDeleteRight (Widget, XKeyEvent*,    String*, Cardinal*);
   static void	HandleMoveLeft    (Widget, XKeyEvent*,    String*, Cardinal*);
   static void	HandleMoveRight   (Widget, XKeyEvent*,    String*, Cardinal*);
   static void	HandleMovePointer (Widget, XButtonEvent*, String*, Cardinal*);


   Boolean	Apply();
   Boolean	ApplyQuery();
   void		DrawExp();

public:

   Widget	AddSortKey(SortKeyC*, const char*);
   void		Clear();
   void		RemoveSortKey(SortKeyC*);

   SortExpWinC(Widget, const char *name="sortExpWin");
   ~SortExpWinC();

//
// New Callbacks
//
   inline void	AddApplyCallback(CallbackFn *fn, void *data) {
      AddCallback(applyCalls, fn, data);
   }
   inline void	CallApplyCallbacks()	{ CallCallbacks(applyCalls, this); }
   inline void	RemoveApplyCallback(CallbackFn *fn, void *data) {
      RemoveCallback(applyCalls, fn, data);
   }

//
// Query data
//
   MEMBER_QUERY(Widget,		ExpForm,	expForm)
   MEMBER_QUERY(Widget,		ExpWin,		expWin)
   MEMBER_QUERY(Widget,		ClipWin,	clipWin)
   MEMBER_QUERY(Widget,		ExpDA,		expDA)
      PTR_QUERY(SortKeyListC&,	KeyList,	keyList)

//
// Set data
//
   inline void	SetWriteFunction(void (*func)(SortExpWinC*)) { writeFunc = func; }
};

#endif // _SortExpWinC_h_
