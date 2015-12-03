/*
 * $Id: BoolExpWinC.h,v 1.3 2000/08/13 13:25:32 evgeny Exp $
 *
 * Copyright (c) 1992 HAL Computer Systems International, Ltd.
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

#ifndef _BoolExpWinC_h_
#define _BoolExpWinC_h_

#include "HalDialogC.h"
#include "CallbackListC.h"
#include "IntListC.h"
#include "ExpElemListC.h"
#include "BoolExpC.h"

//----------------------------------------------------------------------

class BoolExpWinC : public HalDialogC {

protected:

   Widget		termForm;
   Widget		okPB;
   Widget		applyPB;
   Widget		clearPB;
   Widget		cancelPB;
   Widget		helpPB;

   Boolean		Apply();

private:

   Widget		opRC;
   Widget		lparPB;
   Widget		rparPB;
   Widget		andPB;
   Widget		orPB;
   Widget		notPB;
   Widget		expWin;
   Widget		clipWin;
   Widget		expDA;
   Widget		cornerDA;
   Widget		cursorLabel;

   ExpElemListC		expList;
   int			insertPos;
   BoolExpC		*exp;
   Boolean		resizeOk;
   CallbackListC	applyCalls;

   int			blinkInterval;
   XtIntervalId		blinkTimer;
   Boolean		cursorOn;
   Dimension		cursorWd,  cursorHt;
   Dimension		rowHt;
   Dimension		borWd;
   Dimension		marWd,  marHt;
   Dimension		clipWd, clipHt;
   IntListC		rowList;		// List of indexes of first
						// element in each row
   TrueExpC		trueExp;		// Default expression

   static XtTranslations	translations;
   static XtActionsRec		actions[10];

   static void	BlinkProc(BoolExpWinC*, XtIntervalId*);

//
// Callbacks
//
   static void	DoAnd   (Widget, BoolExpWinC*, XtPointer);
   static void	DoApply (Widget, BoolExpWinC*, XtPointer);
   static void	DoClear (Widget, BoolExpWinC*, XtPointer);
   static void	DoLParen(Widget, BoolExpWinC*, XtPointer);
   static void	DoNot   (Widget, BoolExpWinC*, XtPointer);
   static void	DoOk    (Widget, BoolExpWinC*, XtPointer);
   static void	DoOr    (Widget, BoolExpWinC*, XtPointer);
   static void	DoPopup (Widget, BoolExpWinC*, XtPointer);
   static void	DoRParen(Widget, BoolExpWinC*, XtPointer);

//
// Event handlers
//
   static void	HandleFocusChange(Widget, BoolExpWinC*, XEvent*, Boolean*);
   static void	HandleResize     (Widget, BoolExpWinC*, XEvent*, Boolean*);

//
// Action procs
//
   static void	HandleInsertLParen(Widget, XKeyEvent*,    String*, Cardinal*);
   static void	HandleInsertRParen(Widget, XKeyEvent*,    String*, Cardinal*);
   static void	HandleInsertAnd   (Widget, XKeyEvent*,    String*, Cardinal*);
   static void	HandleInsertOr    (Widget, XKeyEvent*,    String*, Cardinal*);
   static void	HandleInsertNot   (Widget, XKeyEvent*,    String*, Cardinal*);
   static void	HandleDeleteLeft  (Widget, XKeyEvent*,    String*, Cardinal*);
   static void	HandleDeleteRight (Widget, XKeyEvent*,    String*, Cardinal*);
   static void	HandleMoveLeft    (Widget, XKeyEvent*,    String*, Cardinal*);
   static void	HandleMoveRight   (Widget, XKeyEvent*,    String*, Cardinal*);
   static void	HandleMovePointer (Widget, XButtonEvent*, String*, Cardinal*);

   void		DrawExp();
   BoolExpC	*ParseExpList(int first, int last);

public:

   void		AddTermExp(TermExpC*);

   BoolExpWinC(Widget);
   ~BoolExpWinC();

//
// Add callbacks
//
   inline void	AddApplyCallback(CallbackFn *fn, void *data) {
      AddCallback(applyCalls, fn, data);
   }

//
// Call callbacks
//
   inline void	CallApplyCallbacks() { CallCallbacks(applyCalls, exp); }

//
// Remove callbacks
//
   inline void	RemoveApplyCallback(CallbackFn *fn, void *data) {
      RemoveCallback(applyCalls, fn, data);
   }

//
// Query data
//
   MEMBER_QUERY(Widget,	OpRC,		opRC)
   MEMBER_QUERY(Widget,	LparPB,		lparPB)
   MEMBER_QUERY(Widget,	RparPB,		rparPB)
   MEMBER_QUERY(Widget,	AndPB,		andPB)
   MEMBER_QUERY(Widget,	OrPB,		orPB)
   MEMBER_QUERY(Widget,	NotPB,		notPB)
   MEMBER_QUERY(Widget,	TermForm,	termForm)
   MEMBER_QUERY(Widget,	ExpWin,		expWin)
   MEMBER_QUERY(Widget,	ClipWin,	clipWin)
   MEMBER_QUERY(Widget,	ExpDA,		expDA)
};

#endif // _BoolExpWinC_h_
