/*
 * $Id: HalShellC.h,v 1.1.1.1 2000/04/25 13:49:01 fnevgeny Exp $
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

#ifndef _HalShellC_h_
#define _HalShellC_h_

#include "CallbackListC.h"

#include <X11/Intrinsic.h>
#include <Xm/Xm.h>

class HalShellC {

protected:

   Widget		shell;
   Widget		mainWindow;
   Widget		topForm;
   Widget		appFrame;
   Widget		appForm;
   Widget		buttonSep;
   Widget		buttonFrame;
   Widget		buttonRC;
   Widget		msgDialog;
   Widget		msgWin;
   Widget		quickHelp;
   Widget		infoMsg;

   Boolean		shown;
   Boolean		mapped;
   int			buttonGravity;

   CallbackListC	hideCalls;
   static void		DoHide(Widget, HalShellC*, XtPointer);

//
// Focus event handlers (for quick-help)
//
   static void		EnterFocus (Widget, HalShellC*, XEvent*, Boolean*);
   static void		LeaveFocus (Widget, HalShellC*, XEvent*, Boolean*);
   static void		ChangeFocus(Widget, HalShellC*, XEvent*, Boolean*);
   static void		DoContextHelp(Widget, HalShellC*, XtPointer);

   CallbackListC	mapCalls;
   CallbackListC	unmapCalls;
   static void          HandleMapChange(Widget, HalShellC*, XEvent*, Boolean*);

public:

   HalShellC();
   virtual ~HalShellC();

//
// Add components
//
   void			AddButtonBox();
   void			SetButtonGravity(int);

//
// Display and clear string in QuickHelp footer
//
   void			QuickHelp(const char *msg) const;
   inline void		ClearQuickHelp() const		{ QuickHelp(" "); }
   inline void		Feedback(const char *msg) const { QuickHelp(msg); }
   inline void		ClearFeedback() const		{ QuickHelp(" "); }

//
// Display string in popup message window
//
   void			PopupMessage(const char*,
				     unsigned char type=XmDIALOG_ERROR);

//
// Display and clear string in infoMsg
//
   void			Message(const char *msg) const;
   inline void		ClearMessage() const { Message(" "); }

//
// Display and remove the application window
//
   virtual void		Show();
   virtual void		Hide();

   inline Boolean	IsShown() const { return shown; }
   inline Boolean	IsIconified() const { return shown && !mapped; }

//
// Turn the waiting cursor on or off
//
   void 		BusyCursor(Boolean);

//
// Add or remove quick-help handlers for the specified widget
//
   void			HandleFocus  (Widget);
   void			UnhandleFocus(Widget);

//
// Add or remove help handlers for the specified widget
//
   void			HandleHelp  (Widget, Boolean doChildren=False);
   void			UnhandleHelp(Widget, Boolean doChildren=False);

//
// Add or remove help handlers for all widgets in shell
//
   void			HandleHelp();
   void			UnhandleHelp();

//
// Turn fields on and off
//
   void			ShowQuickHelp();
   void			HideQuickHelp();
   void			ShowInfoMsg();
   void			HideInfoMsg();

//
// Manage exit callbacks
//
   inline void	AddHideCallback(CallbackFn *fn, void *data) {
      AddCallback(hideCalls, fn, data);
   }
   inline void	CallHideCallbacks() {
      CallCallbacks(hideCalls, this);
   }
   inline void	RemoveHideCallback(CallbackFn *fn, void *data) {
      RemoveCallback(hideCalls, fn, data);
   }

//
// Manage map and unmap callbacks
//
   inline void	AddMapCallback(CallbackFn *fn, void *data) {
      AddCallback(mapCalls, fn, data);
   }
   inline void	CallMapCallbacks() {
      CallCallbacks(mapCalls, this);
   }
   inline void	RemoveMapCallback(CallbackFn *fn, void *data) {
      RemoveCallback(mapCalls, fn, data);
   }

   inline void	AddUnmapCallback(CallbackFn *fn, void *data) {
      AddCallback(unmapCalls, fn, data);
   }
   inline void	CallUnmapCallbacks() {
      CallCallbacks(unmapCalls, this);
   }
   inline void	RemoveUnmapCallback(CallbackFn *fn, void *data) {
      RemoveCallback(unmapCalls, fn, data);
   }

//
// Casts
//
   inline operator	Widget() const		{ return shell; }
   inline operator	Window() const		{ return XtWindow(shell); }

//
// Widget return
//
   MEMBER_QUERY(Widget,	AppForm,	appForm)
   MEMBER_QUERY(Widget,	AppFrame,	appFrame)
   MEMBER_QUERY(Widget, ButtonBox,	buttonRC)
   MEMBER_QUERY(Widget, ButtonFrame,	buttonFrame)
   MEMBER_QUERY(int, 	ButtonGravity,	buttonGravity)
   MEMBER_QUERY(Widget, ButtonSep,	buttonSep)
   MEMBER_QUERY(Widget, InfoMsg,	infoMsg)
   MEMBER_QUERY(Widget,	MainWindow,	mainWindow)
   MEMBER_QUERY(Widget,	MsgWin,		msgWin)
   MEMBER_QUERY(Widget, QuickHelp,	quickHelp)
   MEMBER_QUERY(Widget,	Shell,		shell)
   MEMBER_QUERY(Widget, TopForm,	topForm)
};

#endif // _HalShellC_h_
