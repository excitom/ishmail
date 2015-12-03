/*
 *  $Id: HelpResWinC.h,v 1.1.1.1 2000/04/25 13:49:01 fnevgeny Exp $
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
#ifndef _HelpResWinC_h_
#define _HelpResWinC_h_

#include "HalTopLevelC.h"
#include "VBoxC.h"
#include "PtrList2.h"

#include <stdio.h>

class FieldViewC;
class WidgetRecC;
class EntryDataC;
class ResEntryC;

class HelpResWinC : public HalTopLevelC {

   VBoxC	*widgetBox;
   FieldViewC	*widgetView;
   VTypeT	widgetViewType;

   Widget	shortNameTF;
   Widget	longNameTF;
   Widget	quickHelpTF;
   Widget	quickNameTF;
   Widget	quickLoadPB;
   Widget	contCardTF;
   Widget	contCardPB;
   Widget	contNameTF;
   Widget	contLoadPB;
   Widget	helpCardTF;
   Widget	helpCardPB;
   Widget	helpNameTF;
   Widget	helpLoadPB;
   Widget	modLabel1;
   Widget	modLabel2;

   Cursor	pickCursor;
   WidgetRecC	*curWidget;
   Pixel	modBg;
   Pixel	regBg;
   Boolean	changed;
   Boolean	selfMod;

   static void	DoWidgetPick    (Widget, HelpResWinC*, XtPointer);
   static void	DoWidgetNext    (Widget, HelpResWinC*, XtPointer);
   static void	DoWidgetPrev    (Widget, HelpResWinC*, XtPointer);
   static void	DoWidgetReset   (Widget, HelpResWinC*, XtPointer);
   static void	DoWidgetDel     (Widget, HelpResWinC*, XtPointer);
   static void	DoParentLoad    (Widget, HelpResWinC*, XtPointer);
   static void	DoQuickLoad     (Widget, HelpResWinC*, XtPointer);
   static void	DoContLoad      (Widget, HelpResWinC*, XtPointer);
   static void	DoHelpLoad      (Widget, HelpResWinC*, XtPointer);
   static void	DoContCard      (Widget, HelpResWinC*, XtPointer);
   static void	DoHelpCard      (Widget, HelpResWinC*, XtPointer);
   static void	DoLoadAll       (Widget, HelpResWinC*, XtPointer);
   static void	DoLoadSome      (Widget, HelpResWinC*, XtPointer);
   static void	DoSave          (Widget, HelpResWinC*, XtPointer);
   static void	DoDone          (Widget, HelpResWinC*, XtPointer);
   static void	DoPopup         (Widget, HelpResWinC*, XtPointer);
   static void	QuickTextChanged(Widget, HelpResWinC*, XtPointer);
   static void	ContTextChanged (Widget, HelpResWinC*, XtPointer);
   static void	HelpTextChanged (Widget, HelpResWinC*, XtPointer);

   static void	FinishInit(HelpResWinC*, XtIntervalId*);

   static void	OpenWidget(VItemC*, HelpResWinC*);

   void		CheckResVal(WidgetRecC*, CharC, CharC, EntryDataC*, EntryDataC*,
   			    PtrList2&);
   void		Clear();
   int		EntryIndex(ResEntryC*, PtrList2&);
   VItemC	*FindItem(Widget);
   void		GetResLines(CharC);
   WidgetRecC	*GetWidgetRec(Widget);
   void		InitWidgetRec(WidgetRecC*, Widget);
   Boolean	LineMatches(CharC, Widget, CharC, Widget*, int*);
   void		LoadWidget(Widget, Boolean);
   void		MergeLists(PtrList2&, PtrList2&);
   Boolean	QuerySave(Boolean*, Boolean);
   void		SelectWidget(Widget);
   void		ShowWidgetInfo(Widget);
   void		ShowWidgetInfo(WidgetRecC*);
   void		UpdateWidget(Widget);
   Boolean	WriteResources(FILE*, PtrList2&, PtrList2&);

public:

   PtrList2	quickList, quickList2;
   PtrList2	contList, contList2;
   PtrList2	helpList, helpList2;
   PtrList2	widgetList;

   HelpResWinC(Widget);
   ~HelpResWinC();
};

#endif

