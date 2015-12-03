/*
 *  $Id: ButtPrefWinC.h,v 1.1.1.1 2000/04/25 13:49:02 fnevgeny Exp $
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
#ifndef _ButtPrefWinC_h_
#define _ButtPrefWinC_h_

#include "OptWinC.h"
#include "ButtonEntryListC.h"

#include <hgl/StringC.h>
#include <hgl/PtrListC.h>

#include <Xm/DragDrop.h>

class ButtonMgrC;
class RowColC;
class ButtonEditWinC;
class IntListC;

class ButtPrefWinC : public OptWinC {

protected:

   ButtonMgrC		*buttMgr;	// First one
   PtrListC             buttMgrList;	// List of all

   Widget		availListW;
   Widget		arrowForm;
   Widget		addAB;
   Widget		remAB;
   Widget		usedListW;
   Widget		placeForm;
   RowColC		*gravRC;
   Widget		northTB;
   Widget		southTB;
   Widget		eastTB;
   Widget		westTB;
   Widget		sameSizeTB;

   ButtonEntryListC	availList;
   ButtonEntryListC	usedList;
   int			dropY;
   ButtonEditWinC	*editWin;
   ButtonEntryC		*editEntry;
   ButtonEntryC		saveEntry;

//
// Callbacks
//
   static void		ApplyEdit  (ButtonEditWinC*, ButtPrefWinC*);
   static void		DoActivate (Widget, Widget, XtPointer);
   static void		DoNorth    (Widget, ButtPrefWinC*,
					    XmToggleButtonCallbackStruct*);
   static void		DoSouth    (Widget, ButtPrefWinC*,
					    XmToggleButtonCallbackStruct*);
   static void		DoEast     (Widget, ButtPrefWinC*,
					    XmToggleButtonCallbackStruct*);
   static void		DoWest     (Widget, ButtPrefWinC*,
					    XmToggleButtonCallbackStruct*);
   static void		DoPopup    (Widget, ButtPrefWinC*, XtPointer);
   static void		SelectAvail(Widget, ButtPrefWinC*, XmListCallbackStruct*);
   static void		HandleAdd  (Widget, ButtPrefWinC*, XtPointer);
   static void		HandleRem  (Widget, ButtPrefWinC*, XtPointer);
   static void		SelectUsed (Widget, ButtPrefWinC*, XmListCallbackStruct*);
   static void		HandleEdit (Widget, ButtPrefWinC*, XmListCallbackStruct*);
   static void		FinishDrop(Widget, Widget, Atom*, Atom*, XtPointer,
				   unsigned long*, int);
   static void		HandleDrop(Widget,XtPointer, XmDropProcCallbackStruct*);

//
// Private methods
//
   Boolean		Apply();
   void			AvailToUsed(int*, int, int);
   int			EntryIndex(ButtonEntryListC&, const char*);
   void			GetDragPositions(XtPointer, u_long, Widget*, IntListC&);
   void			ProcessCascade(Widget cb, StringC label);
   void			ProcessButtonString(StringC);
   void			UsedToAvail(int*, int);
   void			UsedToUsed(int*, int, int);
   virtual void		Write() = 0;

public:

// Methods

   ButtPrefWinC(Widget, ButtonMgrC*);
   virtual ~ButtPrefWinC();

   void			AddManager(ButtonMgrC*);
   void			RemoveManager(ButtonMgrC*);
   void			EnableButtons();
   void			Show(Widget);
   void			Show();
};

#endif // _ButtPrefWinC_h_
