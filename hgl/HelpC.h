/*
 * $Id: HelpC.h,v 1.2 2000/05/03 15:35:26 fnevgeny Exp $
 *
 * Copyright (c) 1994 HAL Computer Systems International, Ltd.
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

#ifndef _HelpC_h_
#define _HelpC_h_

#ifndef EDIT_OK
#define EDIT_OK 1
#endif

#include "PtrListC.h"

#include <X11/Intrinsic.h>

/*
 * HaL On-Line Help support for GUI applications.
 *
 * This class allows GUI applications to access helpcards from a simple
 * (ascii, flat-file) database, and have them displayed in a Help Dialog.
 *
 * The GUI app should only create a single instance of this class.
 *
 */

class HelpCardC;
class MimeRichTextC;
class HelpDbC;
class HelpResWinC;

class HelpC {
  
public:

   Boolean	isActive;

//
// Constructor and destructor
//
   HelpC(Boolean editOk=False);
   ~HelpC();

//
// Methods
//
   void		ShowIndex(Widget parent=NULL);
   void		ShowCard(const Widget, const char *res=NULL, Widget par=NULL);
   void		ShowCard(const HelpCardC*, Widget);
  
   void		removeOliasButton();
   void		ReparentWindows(Widget);
   void		WriteNewDbFormat();
   Boolean	WriteDbFile(HelpDbC*, char*);
   HelpCardC	*FindCard(StringC&);
   HelpCardC	*FindCard(const Widget, const char* res=NULL);
   Boolean	Editable() const;
   HelpResWinC	*HelpResWin();

private:

   PtrListC		dbList;
   Widget		helpWin;
#if EDIT_OK
   Widget		nameTF;
   Widget		locTF;
   Widget		modLabel;
   PtrListC		modList;
   PtrListC		addList;
#endif
   Widget		helpTitle;
   Widget		helpText;
   Widget		glossWin;
   Widget		glossTitle;
   Widget		glossText;
   Widget		indexWin;
   Widget		indexList;
   Widget		viewPB;
   Widget		nextPB;
   Widget		prevPB;
   MimeRichTextC	*richText;
   MimeRichTextC	*glossRichText;
   HelpResWinC		*helpResWin;

   Boolean	showOlias;
   Boolean	enriched;
   Boolean	edit;
   Boolean	changed;
   HelpCardC	*curCard;
   int		indexPos;

   StringC	CardName(const Widget, const char *res);
   void		CreateHelpWin();
   void		PopupHelpWin(Widget);
   void		ShowCardFromIndex(int);
   void		ShowGlossary(const HelpCardC*, Widget);
   void		ShowMessage(const char*, Widget);
   void		SetChanged(Boolean);

   static void	DoGlossaryCard(void*, HelpC*);
   static void	DoHelpDone   (Widget, HelpC*, XtPointer);
   static void	DoGlossaryDone(Widget, HelpC*, XtPointer);
   static void	DoHelpHelp   (Widget, HelpC*, XtPointer);
   static void	DoGlossaryHelp(Widget, HelpC*, XtPointer);
   static void	DoHelpIndex  (Widget, HelpC*, XtPointer);
   static void	DoGlossaryIndex(Widget, HelpC*, XtPointer);
   static void	DoHelpOlias  (Widget, HelpC*, XtPointer);
   static void	DoIndexDone  (Widget, HelpC*, XtPointer);
   static void	DoIndexHelp  (Widget, HelpC*, XtPointer);
   static void	DoIndexNext  (Widget, HelpC*, XtPointer);
   static void	DoIndexOpen  (Widget, HelpC*, XmListCallbackStruct*);
   static void	DoIndexPrev  (Widget, HelpC*, XtPointer);
   static void	DoIndexSelect(Widget, HelpC*, XmListCallbackStruct*);
   static void	DoIndexView  (Widget, HelpC*, XtPointer);

#if EDIT_OK
   Boolean	CheckChanges(Boolean cancelOk=True);
   Boolean	GetDbFileName(StringC&);

   static void	DoFileNew (Widget, HelpC*, XtPointer);
   static void	DoFileDel (Widget, HelpC*, XtPointer);
   static void	DoFileSave(Widget, HelpC*, XtPointer);

   static void	DoEditPlain (Widget, HelpC*, XtPointer);
   static void	DoEditBold  (Widget, HelpC*, XtPointer);
   static void	DoEditItalic(Widget, HelpC*, XtPointer);
   static void	DoEditFixed (Widget, HelpC*, XtPointer);
   static void	DoEditUnder (Widget, HelpC*, XtPointer);
   static void	DoEditBig   (Widget, HelpC*, XtPointer);
   static void	DoEditSmall (Widget, HelpC*, XtPointer);
   static void	DoEditUndel (Widget, HelpC*, XtPointer);

   static void	DoJustLeft  (Widget, HelpC*, XtPointer);
   static void	DoJustRight (Widget, HelpC*, XtPointer);
   static void	DoJustCenter(Widget, HelpC*, XtPointer);

   static void	DoIndentLeftMore (Widget, HelpC*, XtPointer);
   static void	DoIndentLeftLess (Widget, HelpC*, XtPointer);
   static void	DoIndentRightMore(Widget, HelpC*, XtPointer);
   static void	DoIndentRightLess(Widget, HelpC*, XtPointer);

   static void	DoColorRed    (Widget, HelpC*, XtPointer);
   static void	DoColorGreen  (Widget, HelpC*, XtPointer);
   static void	DoColorBlue   (Widget, HelpC*, XtPointer);
   static void	DoColorYellow (Widget, HelpC*, XtPointer);
   static void	DoColorMagenta(Widget, HelpC*, XtPointer);
   static void	DoColorCyan   (Widget, HelpC*, XtPointer);
   static void	DoColorBlack  (Widget, HelpC*, XtPointer);
   static void	DoColorWhite  (Widget, HelpC*, XtPointer);
   static void	DoColorNone   (Widget, HelpC*, XtPointer);

   static void	EnterName      (Widget, HelpC*, XtPointer);
   static void	ModifyName     (Widget, HelpC*, XmTextVerifyCallbackStruct*);
   static void	TextChanged    (Widget, HelpC*, XtPointer);
   static void	RichTextChanged(void*,  HelpC*);
#endif
};

#endif // _HelpC_h_
