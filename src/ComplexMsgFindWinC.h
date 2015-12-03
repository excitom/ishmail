/*
 * $Id: ComplexMsgFindWinC.h,v 1.1.1.1 2000/04/25 13:49:02 fnevgeny Exp $
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

#ifndef _ComplexMsgFindWinC_h_
#define _ComplexMsgFindWinC_h_

#include "MsgFindExp.h"

#include <hgl/BoolExpWinC.h>

/*--------------------------------------------------------------------------
 * Find window class
 */

class ComplexMsgFindWinC : public BoolExpWinC {

   int		findIndex;

   Widget	fromTF;
   Widget	toTF;
   Widget	subjTF;
   Widget	headTF;
   Widget	bodyTF;
   Widget	msgTF;
   Widget	dateTF;
   Widget	numTF;
   Widget	lineTF;
   Widget	byteTF;

   MsgNumExpC::MsgNumOp		numOp;
   MsgDateExpC::MsgDateOp	dateOp;
   MsgLineExpC::MsgLineOp	lineOp;
   MsgByteExpC::MsgByteOp	byteOp;
   MsgStatusT			stat;

   static void	AddFrom     (Widget, ComplexMsgFindWinC*, XtPointer);
   static void	AddTo       (Widget, ComplexMsgFindWinC*, XtPointer);
   static void	AddSubject  (Widget, ComplexMsgFindWinC*, XtPointer);
   static void	AddHead     (Widget, ComplexMsgFindWinC*, XtPointer);
   static void	AddBody     (Widget, ComplexMsgFindWinC*, XtPointer);
   static void	AddMsg      (Widget, ComplexMsgFindWinC*, XtPointer);
   static void	AddDate     (Widget, ComplexMsgFindWinC*, XtPointer);
   static void	AddStatus   (Widget, ComplexMsgFindWinC*, XtPointer);
   static void	AddNumber   (Widget, ComplexMsgFindWinC*, XtPointer);
   static void	AddLine     (Widget, ComplexMsgFindWinC*, XtPointer);
   static void	AddByte     (Widget, ComplexMsgFindWinC*, XtPointer);
   static void	ChangeNumOp (Widget, ComplexMsgFindWinC*, XtPointer);
   static void	ChangeDateOp(Widget, ComplexMsgFindWinC*, XtPointer);
   static void	ChangeLineOp(Widget, ComplexMsgFindWinC*, XtPointer);
   static void	ChangeByteOp(Widget, ComplexMsgFindWinC*, XtPointer);
   static void	ChangeStatus(Widget, ComplexMsgFindWinC*, XtPointer);
   static void	EnablePB    (Widget, Widget,       XtPointer);
   static void	DoFind      (BoolExpC*, ComplexMsgFindWinC*);
   static void	DoFindPrev  (Widget, ComplexMsgFindWinC*, XtPointer);
   static void	DoFindPrev2 (BoolExpC*, ComplexMsgFindWinC*);
   static void	DoFindAll   (Widget, ComplexMsgFindWinC*, XtPointer);
   static void	DoFindAll2  (BoolExpC*, ComplexMsgFindWinC*);

public:

// Methods

   ComplexMsgFindWinC(Widget);

   void		Show();
};

#endif // _ComplexMsgFindWinC_h_
