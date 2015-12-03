/*
 * $Id: MsgFindWinC.h,v 1.1.1.1 2000/04/25 13:49:02 fnevgeny Exp $
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

#ifndef _MsgFindWinC_h_
#define _MsgFindWinC_h_

#include <hgl/HalDialogC.h>
#include <hgl/IntListC.h>

class CharC;
class MsgC;
class RowColC;
class MsgItemC;
class ComplexMsgFindWinC;
class ComplexImapFindWinC;

/*--------------------------------------------------------------------------
 * Find window class
 */

class MsgFindWinC : public HalDialogC {

   int			findIndex;
   Widget		patternTF;
   RowColC		*searchRC;
   Widget		searchHeadTB;
   Widget		searchBodyTB;
   Widget		searchCaseTB;
   ComplexMsgFindWinC	*complexWin;
   ComplexImapFindWinC	*complexImapWin;

   Boolean		imapOn;
   Boolean		stateChanged;
   IntListC		numList;	// Result of IMAP search

   static void	DoFind      (Widget, MsgFindWinC*, XtPointer);
   static void	DoFindPrev  (Widget, MsgFindWinC*, XtPointer);
   static void	DoFindAll   (Widget, MsgFindWinC*, XtPointer);
   static void	DoComplex   (Widget, MsgFindWinC*, XtPointer);
   static void	StateChanged(Widget, MsgFindWinC*, XtPointer);

   Boolean	MsgContains(MsgC*, CharC);
   void		EnableWidgets();
   void		FindAll(const char*);
   void		FindAllImap(const char*);
   MsgItemC	*FindNext(const char*);
   MsgItemC	*FindNextImap(const char*);
   MsgItemC	*FindPrev(const char*);
   MsgItemC	*FindPrevImap(const char*);
   void		GetImapNumList(const char*, Boolean);

public:

// Methods

   MsgFindWinC(Widget);
   ~MsgFindWinC();

   void		FolderChanged();
   void		Show();
};

#endif // _MsgFindWinC_h_
