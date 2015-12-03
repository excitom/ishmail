/*
 * $Id: LoginWinC.C,v 1.4 2000/12/13 10:06:42 evgeny Exp $
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

#include <config.h>

#include "IshAppC.h"
#include "AppPrefC.h"
#include "LoginWinC.h"

#include <hgl/WArgList.h>
#include <hgl/rsrc.h>
#include <hgl/WXmString.h>
#include <hgl/RowColC.h>
#include <hgl/CharC.h>
#include <hgl/TextMisc.h>
#include <hgl/PtrList2.h>

#include <Xm/Label.h>
#include <Xm/TextF.h>
#include <Xm/PushB.h>
#include <Xm/Protocols.h>
#include <Xm/AtomMgr.h>

static void
WmClose(Widget, int *answer, XtPointer)
{
   *answer = XmCR_CANCEL;
}

/*-----------------------------------------------------------------------
 *  LoginWinC constructor
 */

LoginWinC::LoginWinC(Widget parent, char *name, ArgList argv, Cardinal argc)
 : HalDialogC(name, parent, argv, argc)
{
   WArgList	args;
   Widget	wlist[4];

   args.Reset();
   args.DialogStyle(XmDIALOG_FULL_APPLICATION_MODAL);
   XtSetValues(mainBoard, ARGS);

//
// Create appForm hierarchy
//
// appForm
//    Label	loginLabel
//    Label	hostLabel
//    RowColC	inputRC
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_NONE);
   Widget	loginLabel = XmCreateLabel(appForm, "loginLabel", ARGS);

   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_WIDGET, loginLabel);
   args.RightAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_NONE);
   hostLabel = XmCreateLabel(appForm, "hostLabel", ARGS);

   args.Reset();
   args.TopAttachment(XmATTACH_WIDGET, loginLabel);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_NONE);
   inputRC = new RowColC(appForm, "inputRC", ARGS);
   inputRC->Defer(True);

//
// Set up 2 Columns
//
   inputRC->SetOrientation(RcROW_MAJOR);
   inputRC->SetColCount(2);
   inputRC->SetColAlignment(0, XmALIGNMENT_END);
   inputRC->SetColAlignment(1, XmALIGNMENT_CENTER);
   inputRC->SetColWidthAdjust(0, RcADJUST_NONE);
   inputRC->SetColWidthAdjust(1, RcADJUST_ATTACH);
   inputRC->SetColResize(0, False);
   inputRC->SetColResize(1, True);

//
// Create inputRC hierarchy
//
// inputRC
//    Label		nameLabel
//    TextField		nameTF
//    Label		passLabel
//    TextField		passTF
//
   Widget nameLabel = XmCreateLabel    (*inputRC, "nameLabel", 0,0);
   nameTF           = CreateTextField(*inputRC, "nameTF",    0,0);
   Widget passLabel = XmCreateLabel    (*inputRC, "passLabel", 0,0);
   passTF           = CreateTextField(*inputRC, "passTF",    0,0);

   XtAddCallback(nameTF, XmNactivateCallback, (XtCallbackProc)NextField,
		 (XtPointer)this);
   XtAddCallback(passTF, XmNmodifyVerifyCallback,
		 (XtCallbackProc)ProcessPassword, (XtPointer)this);
   XtAddCallback(passTF, XmNactivateCallback, (XtCallbackProc)DoOk,
		 (XtPointer)this);

//
// Trap window manager close function
//
   XmAddWMProtocolCallback(*this, halApp->delWinAtom, (XtCallbackProc)WmClose,
			   (XtPointer)&reason);

//
// It would be nice if the password field receives initial focus in the
// window, but we haven't found a way to make this work. A kludge is to
// show the password field first, so that it gets the initial focus. Some
// people don't like re-ordering the fields, so there is a settable
// preference to determine the order of the fields.

   if (ishApp->appPrefs->pwFieldFirst) {
      wlist[2] = nameLabel;
      wlist[3] = nameTF;
      wlist[0] = passLabel;
      wlist[1] = passTF;
   } else {
      wlist[0] = nameLabel;
      wlist[1] = nameTF;
      wlist[2] = passLabel;
      wlist[3] = passTF;
   }
   XtManageChildren(wlist, 4);	// *inputRC children
   inputRC->SetChildren(wlist, 4);

   wlist[0] = loginLabel;
   wlist[1] = hostLabel;
   wlist[2] = *inputRC;
   XtManageChildren(wlist, 3);	// appForm children

   inputRC->Defer(False);

//
// Create buttonRC hierarchy
//
//   buttonRC
//	PushButton	okPB
//	PushButton	cancelPB
//	PushButton	helpPB
//
   AddButtonBox();
   Widget okPB     = XmCreatePushButton(buttonRC, "okPB",     0,0);
   Widget cancelPB = XmCreatePushButton(buttonRC, "cancelPB", 0,0);
   Widget helpPB   = XmCreatePushButton(buttonRC, "helpPB",   0,0);

   XtAddCallback(okPB, XmNactivateCallback, (XtCallbackProc)DoOk,
   		 (XtPointer)this);
   XtAddCallback(cancelPB, XmNactivateCallback, (XtCallbackProc)DoCancel,
   		 (XtPointer)this);
   XtAddCallback(helpPB, XmNactivateCallback, (XtCallbackProc)HalAppC::DoHelp,
   		 (char *) "helpcard");

   wlist[0] = okPB;
   wlist[1] = cancelPB;
   wlist[2] = helpPB;
   XtManageChildren(wlist, 3);	// buttonRC children

   HandleHelp();

} // End constructor

/*-----------------------------------------------------------------------
 *  LoginWinC destructor
 */

LoginWinC::~LoginWinC()
{
   delete inputRC;
}

/*-----------------------------------------------------------------------
 *  Prompt for user name and password
 */

Boolean
LoginWinC::GetLogin(const CharC& host, StringC& name, StringC& password)
{
   char	*cs = new char[host.Length() + 1];
   strncpy(cs, host.Addr(), host.Length());
   cs[host.Length()] = 0;

   WXmString	wstr(cs);
   XtVaSetValues(hostLabel, XmNlabelString, (XmString)wstr, NULL);
   delete cs;

   shadow.Clear();
   XmTextFieldSetString(nameTF, name);
   XmTextFieldSetString(passTF, "");
   XtVaSetValues(appForm, XmNinitialFocus, passTF, NULL);

   Show();

   XUndefineCursor(halApp->display, *this);

//
// Simulate the main event loop and wait for the answer
//
   reason = XmCR_NONE;
   while ( reason == XmCR_NONE ) {
      XtAppProcessEvent(halApp->context, XtIMXEvent);
      XSync(halApp->display, False);
   }

   Hide();
   XSync(halApp->display, False);
   XmUpdateDisplay(*this);

   if ( reason == XmCR_OK ) {
      name     = login;
      password = shadow;
   }

   return (reason == XmCR_OK);

} // End GetLogin

/*-----------------------------------------------------------------------
 *  Prompt for password
 */

Boolean
LoginWinC::GetPassword(StringC& password)
{
   inputRC->SetRowVisible(0, False);

   shadow.Clear();
   XmTextFieldSetString(passTF, "");

   Show();

   XUndefineCursor(halApp->display, *this);

//
// Simulate the main event loop and wait for the answer
//
   reason = XmCR_NONE;
   while ( reason == XmCR_NONE ) {
      XtAppProcessEvent(halApp->context, XtIMXEvent);
      XSync(halApp->display, False);
   }

   Hide();
   XSync(halApp->display, False);
   XmUpdateDisplay(*this);

   if ( reason == XmCR_OK )
      password = shadow;

   return (reason == XmCR_OK);

} // End GetPassword

/*-----------------------------------------------------------------------
 *  Handle press of ok button
 */

void
LoginWinC::DoOk(Widget, LoginWinC *lw, XtPointer)
{
//
// Check text fields
//
   char	*cs = XmTextFieldGetString(lw->nameTF);
   lw->login = cs;
   XtFree(cs);

   if ( lw->login.size() <= 0 ) {
      StringC	msg = "Please enter a login id";
      set_invalid(lw->nameTF, True, True);
      lw->PopupMessage(msg);
      return;
   }

   if ( lw->shadow.size() <= 0 ) {
      StringC	msg = "Please enter a password";
      set_invalid(lw->passTF, True, True);
      lw->PopupMessage(msg);
      return;
   }

   lw->reason = XmCR_OK;

} // End DoOk

/*-----------------------------------------------------------------------
 *  Handle press of cancel button
 */

void
LoginWinC::DoCancel(Widget, LoginWinC *lw, XtPointer)
{
   lw->reason = XmCR_CANCEL;
}

/*-----------------------------------------------------------------------
 *  Hide password text
 */

void
LoginWinC::ProcessPassword(Widget w, LoginWinC *lw,
			   XmTextVerifyCallbackStruct *cbs)
{
//
// mostly from the Motif Programming Manual, p.517-518,
// except for the activation section
//
   if (cbs->text->length > 1) {
      cbs->doit = False;		// don't allow "paste" operations
      return;
   }

//
// Modifying selected text doesn't work well so we don't allow any changes
//    if there is a selection.  How do they know what they selected anyway?
//    All they can see is a bunch of *'s
//
   XmTextPosition	left, right;
   if ( XmTextFieldGetSelectionPosition(w, &left, &right) ) {
      XmTextFieldClearSelection(w, cbs->event->xbutton.time);
      cbs->doit = False;
      return;
   }

//
// Handle backspace
//
   if ( !cbs->text->ptr ) {
      //cout <<"NULL text" NL;
      lw->shadow((unsigned)cbs->startPos,1) = "";
   }

//
// Substitute new text
//
   else {
      //cout <<"new text is <" <<cbs->text->ptr <<">" NL;
      if ( cbs->startPos < lw->shadow.size() ) {
	 unsigned	len = (unsigned)(cbs->endPos - cbs->startPos);
	 lw->shadow((unsigned)cbs->startPos, len) = cbs->text->ptr;
      } else
	 lw->shadow += cbs->text->ptr;
   }
   //cout <<"Shadow: <" <<lw->shadow <<">" NL;

//
// Replace real text with stars
//
   for (int i=0; i<cbs->text->length; i++) cbs->text->ptr[i] = '*';

} // End ProcessPassword

/*---------------------------------------------------------------
 *  Callback to handle return key in name field
 */

void
LoginWinC::NextField(Widget, LoginWinC *lw, XtPointer)
{
   XmProcessTraversal(lw->passTF, XmTRAVERSE_CURRENT);
}

class LoginDataC {

public:

   StringC	host;
   StringC	user;
   StringC	pass;

    LoginDataC() {}
   ~LoginDataC() {}
};

static PtrList2		*loginList = NULL;
static LoginWinC	*loginWin  = NULL;

/*------------------------------------------------------------------------
 * Function to get user login id and password for the specified host
 */

Boolean
GetLogin(CharC host, StringC& user, StringC& pass)
{
//
// See if there is a known login for this host
//
   if ( !loginList ) loginList = new PtrList2;

   u_int	count = loginList->size();
   for (int i=0; i<count; i++) {

      LoginDataC	*data = (LoginDataC*)(*loginList)[i];
      if ( data->host == host ) {
	 user = data->user;
	 pass = data->pass;
	 return True;
      }
   }

//
// If we get to this point, we need to prompt the user to log in
//
   if ( !loginWin ) loginWin = new LoginWinC(*halApp);

   if ( loginWin->GetLogin(host, user, pass) ) {

      LoginDataC	*data = new LoginDataC;
      data->host = host;
      data->user = user;
      data->pass = pass;

      loginList->add(data);

      return True;
   }

   return False;

} // End GetLogin

/*------------------------------------------------------------------------
 * Function to invalidate a stored user login id and password
 */

void
InvalidateLogin(CharC host)
{
   if ( !loginList ) return;

//
// See if there is a known login for this host
//
   u_int	count = loginList->size();
   for (int i=0; i<count; i++) {

      LoginDataC	*data = (LoginDataC*)(*loginList)[i];
      if ( data->host == host ) {
	 loginList->remove(data);
	 return;
      }
   }

} // End InvalidateLogin

